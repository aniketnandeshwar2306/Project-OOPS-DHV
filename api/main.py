import os
import joblib
import pandas as pd
import numpy as np
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Optional

# Initialize FastAPI App
app = FastAPI(
    title="CitySync Civic Analytics & Anomaly Detection API",
    description="Backend API serving municipal KPI metrics, cross-city benchmarking, and ML fake-closure prediction for Smart India Hackathon PS11.",
    version="1.0.0"
)

# Paths
MODEL_PATH = "ml/models/fake_closure_detector.joblib"
BBMP_PARQUET = "data/processed/bbmp_grievances_processed.parquet"
NYC_PARQUET = "data/processed/nyc_311_2025_processed.parquet"

# Load ML artifacts globally
artifacts = None
if os.path.exists(MODEL_PATH):
    artifacts = joblib.load(MODEL_PATH)
    print("Loaded ML Anomaly Detector successfully.")

from ml.predict import predict_civic_complaint

class ComplaintPredictionRequest(BaseModel):
    standard_category: str
    channel: str
    staff_remarks: str
    created_hour: Optional[int] = 12
    is_weekend: Optional[int] = 0

class PredictionResponse(BaseModel):
    is_fake_closure: int
    anomaly_probability: float
    risk_level: str
    flagged_reason: str

class CivicComplaintInput(BaseModel):
    description: str

class OfficerResponseInfo(BaseModel):
    recommended_action: str
    verification_status: str
    fake_closure_risk: float

class CivicComplaintPrediction(BaseModel):
    status: str
    category: str
    confidence: float
    estimated_resolution_hours: float
    estimated_resolution_days: float
    sla_tier: str
    sla_compliance_probability: Optional[float] = 0.85
    recommended_priority: str
    officer_response: Optional[OfficerResponseInfo] = None
    message: Optional[str] = None

@app.get("/")
def root():
    return {
        "project": "CitySync — Civic Issue Reporting & Resolution Analytics",
        "hackathon": "Smart India Hackathon (SIH 2025) — Problem Statement 11",
        "status": "Online",
        "docs_url": "/docs"
    }

@app.get("/api/kpi/summary")
def get_kpi_summary():
    """
    Returns global high-level KPIs across Bengaluru BBMP and NYC 311.
    """
    return {
        "bengaluru_bbmp": {
            "total_complaints": 1207746,
            "fake_closure_count": 120710,
            "fake_closure_rate_pct": 9.99,
            "primary_anomaly_modes": {
                "lazy_canned_brush_off": 93562,
                "citizen_dissatisfied_low_rating": 14758,
                "future_promise_while_closed": 8406,
                "reopened_by_citizen": 3984
            }
        },
        "nyc_311_benchmark": {
            "total_complaints": 3654125,
            "median_resolution_hours": 6.61,
            "mean_resolution_hours": 236.35,
            "fast_closures_15m_count": 255910,
            "fast_closures_15m_pct": 7.12,
            "chronic_backlog_30d_count": 261274,
            "chronic_backlog_30d_pct": 7.27
        }
    }

@app.get("/api/categories")
def get_category_distribution():
    """
    Returns breakdown across the Unified Master Civic Taxonomy.
    """
    return {
        "categories": [
            {"category": "Solid Waste Management", "share_pct": 28.5},
            {"category": "Street Lighting & Electrical", "share_pct": 22.4},
            {"category": "Roads & Infrastructure", "share_pct": 18.2},
            {"category": "Horticulture & Greenery", "share_pct": 6.8},
            {"category": "Animal Husbandry & Stray Animals", "share_pct": 5.2},
            {"category": "Public Health & Sanitation", "share_pct": 4.5},
            {"category": "Drainage & Water Supply", "share_pct": 4.1},
            {"category": "Revenue & Urban Governance", "share_pct": 3.8},
            {"category": "General / Others", "share_pct": 6.5}
        ]
    }

@app.get("/api/wards/top-anomalies")
def get_top_anomaly_zones():
    """
    Returns zones with highest fake-closure rates for administrative intervention.
    """
    return {
        "top_zones": [
            {"zone": "East", "fake_closure_rate_pct": 11.4, "status": "High Alert"},
            {"zone": "Mahadevapura", "fake_closure_rate_pct": 10.8, "status": "High Alert"},
            {"zone": "South", "fake_closure_rate_pct": 10.2, "status": "Warning"},
            {"zone": "Bommanahalli", "fake_closure_rate_pct": 9.7, "status": "Warning"},
            {"zone": "West", "fake_closure_rate_pct": 9.1, "status": "Moderate"},
            {"zone": "RRNagar", "fake_closure_rate_pct": 8.9, "status": "Moderate"},
            {"zone": "Yelahanka", "fake_closure_rate_pct": 7.8, "status": "Normal"}
        ]
    }

@app.post("/api/predict/complaint", response_model=CivicComplaintPrediction)
def predict_complaint_endpoint(req: CivicComplaintInput):
    """
    Real-time inference endpoint for citizen complaints:
    Predicts Civic Master Category, Confidence, Estimated Resolution Time (Hours & Days),
    SLA tier, and Recommended Priority.
    """
    return predict_civic_complaint(req.description)

@app.post("/api/predict/fake-closure", response_model=PredictionResponse)
def predict_fake_closure(req: ComplaintPredictionRequest):
    """
    Real-time inference endpoint: evaluates incoming closure remarks & ticket attributes
    and predicts whether the closure is suspicious / premature.
    """
    if artifacts is None:
        raise HTTPException(status_code=500, detail="ML model artifacts not loaded.")
    
    clf = artifacts["model"]
    cat_freq_map = artifacts["cat_freq"]
    chan_freq_map = artifacts["channel_freq"]
    
    remarks = str(req.staff_remarks).strip().lower()
    remarks_len = len(remarks)
    word_count = len(remarks.split())
    has_canned = 1 if remarks in [".", "-", "close", "closed", "attend", "attended", "clear"] else 0
    has_promise = 1 if any(w in remarks for w in ["action", "shortly", "process", "taken"]) else 0
    
    cat_f = cat_freq_map.get(req.standard_category, 0.05)
    chan_f = chan_freq_map.get(req.channel, 0.05)
    
    X = pd.DataFrame([{
        "created_hour": req.created_hour,
        "is_weekend": req.is_weekend,
        "remarks_len": remarks_len,
        "word_count": word_count,
        "has_canned_word": has_canned,
        "has_promise_word": has_promise,
        "cat_freq": cat_f,
        "channel_freq": chan_f
    }])
    
    pred = int(clf.predict(X)[0])
    prob = float(clf.predict_proba(X)[0][1])
    
    if prob >= 0.70:
        risk = "HIGH RISK"
        reason = "Canned brush-off or premature promise detected"
    elif prob >= 0.40:
        risk = "MODERATE RISK"
        reason = "Awaiting citizen confirmation / rating"
    else:
        risk = "LOW RISK"
        reason = "Genuine operational resolution"
        
    return PredictionResponse(
        is_fake_closure=pred,
        anomaly_probability=round(prob, 4),
        risk_level=risk,
        flagged_reason=reason
    )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("api.main:app", host="127.0.0.1", port=8000, reload=True)
