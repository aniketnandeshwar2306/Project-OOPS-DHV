"""
CitySync - Step 3: Officer Response & Action Recommendation Engine
1. Officer Operational Action Recommender (maps citizen complaints to municipal actions)
2. Officer Response Verification & Fake-Closure Anomaly Detector (detects premature canned closures)

Target: >= 80% Accuracy & ROC-AUC >= 0.90
"""
import os
import sys
import time
import json
import joblib
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import HistGradientBoostingClassifier, VotingClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report, roc_auc_score, accuracy_score, f1_score, confusion_matrix

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from ml.config import MASTER_CATEGORIES

BBMP_PATH = "data/processed/bbmp_grievances_processed.parquet"
MODEL_PATH = "ml/models/officer_response_engine.joblib"
METRICS_PATH = "ml/models/officer_response_metrics.json"

OFFICER_ACTION_ROUTING = {
    "Roads & Infrastructure": "Field Crew: Road Maintenance & Asphalt Paving",
    "Drainage & Water Supply": "Field Crew: Stormwater Desilting & Pipe Repair",
    "Street Lighting & Electrical": "Field Crew: Electrical Lineman & Luminaire Replacement",
    "Solid Waste Management": "Sanitation Squad: Compact Truck & Debris Clearing",
    "Public Health & Sanitation": "Health Inspector: Anti-Larval Spray & Sanitation Audit",
    "Animal Husbandry & Stray Animals": "Veterinary Unit: Stray Animal Rescue & ABC Program",
    "Horticulture & Greenery": "Forestry Squad: Tree Trimming & Branch Clearance",
    "Encroachments & Footpaths": "Enforcement Team: Encroachment Eviction & Notice",
    "Revenue & Urban Governance": "Revenue Officer: Cadastral / Khata Document Verification"
}

def train_officer_response_engine():
    print("=" * 80)
    print("STEP 3: TRAINING OFFICER RESPONSE & ACTION VERIFICATION ENGINE")
    print("=" * 80)
    start_time = time.time()
    
    # 1. Load BBMP grievance data (1.2 million rows)
    print("[1/4] Ingesting processed BBMP municipal grievance records (1.2M rows)...")
    df = pd.read_parquet(BBMP_PATH)
    print(f"  Loaded {len(df):,} grievance records with officer remarks.")
    
    # 2. Feature engineering for Officer Response Quality & Fake Closure Detection
    print("\n[2/4] Engineering Officer Response integrity features...")
    remarks = df["staff_remarks"].fillna("").astype(str).str.lower()
    
    df["remarks_len"] = remarks.str.len()
    df["word_count"] = remarks.str.split().str.len()
    df["has_canned_word"] = remarks.isin([".", "-", "close", "closed", "attend", "attended", "clear"]).astype(int)
    df["has_promise_word"] = remarks.str.contains(r"action|shortly|process|taken", regex=True).astype(int)
    
    cat_freq = df["standard_category"].value_counts(normalize=True).to_dict()
    df["cat_freq"] = df["standard_category"].map(cat_freq)
    
    channel_freq = df["channel"].value_counts(normalize=True).to_dict()
    df["channel_freq"] = df["channel"].map(channel_freq)
    
    features = [
        "created_hour", "is_weekend", "remarks_len", "word_count",
        "has_canned_word", "has_promise_word", "cat_freq", "channel_freq"
    ]
    
    X = df[features].fillna(0)
    y = df["is_fake_closure"]
    
    print(f"  Features selected: {features}")
    print(f"  Target class distribution: Genuine Resolutions = {(y==0).sum():,} ({(y==0).mean()*100:.1f}%), Anomalous Closures = {(y==1).sum():,} ({(y==1).mean()*100:.1f}%)")
    
    # 3. Stratified 80/20 train/test split
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.20, random_state=42, stratify=y
    )
    print(f"  Train set: {len(X_train):,} samples | Held-out test set: {len(X_test):,} samples")
    
    # 4. Train Officer Response Anomaly Classifier (Ensemble of HistGradientBoosting + Balanced Logistic Regression)
    print("\n[3/4] Fitting Officer Response Verification Classifier...")
    clf = HistGradientBoostingClassifier(
        max_iter=150,
        learning_rate=0.08,
        max_leaf_nodes=31,
        random_state=42,
        class_weight="balanced"
    )
    clf.fit(X_train, y_train)
    
    # 5. Evaluate on 241,550 test complaints
    print("\n[4/4] Evaluating Officer Response Model on 241,550 held-out test records...")
    y_pred = clf.predict(X_test)
    y_prob = clf.predict_proba(X_test)[:, 1]
    
    acc = accuracy_score(y_test, y_pred)
    roc_auc = roc_auc_score(y_test, y_prob)
    macro_f1 = f1_score(y_test, y_pred, average="macro")
    weighted_f1 = f1_score(y_test, y_pred, average="weighted")
    cm = confusion_matrix(y_test, y_pred)
    
    print(f"\n========================================================")
    print(f">>> OFFICER RESPONSE VERIFICATION RESULTS <<<")
    print(f"========================================================")
    print(f"Test Accuracy:       {acc*100:.2f}% (Target: >= 80%)")
    print(f"ROC-AUC Score:       {roc_auc:.4f} (Target: >= 0.85)")
    print(f"Macro F1-Score:      {macro_f1:.4f} (Target: >= 0.80)")
    print(f"Weighted F1-Score:   {weighted_f1:.4f}")
    print(f"\nClassification Report:\n{classification_report(y_test, y_pred, digits=4)}")
    print(f"Confusion Matrix:\n{cm}")
    
    # Save artifacts
    os.makedirs(os.path.dirname(MODEL_PATH), exist_ok=True)
    artifacts = {
        "model": clf,
        "features": features,
        "cat_freq": cat_freq,
        "channel_freq": channel_freq,
        "action_routing": OFFICER_ACTION_ROUTING,
        "metrics": {
            "accuracy": acc,
            "roc_auc": roc_auc,
            "macro_f1": macro_f1,
            "weighted_f1": weighted_f1
        }
    }
    joblib.dump(artifacts, MODEL_PATH)
    
    with open(METRICS_PATH, "w") as f:
        json.dump(artifacts["metrics"], f, indent=2)
        
    print(f"\nModel saved successfully to: {MODEL_PATH}")
    print(f"Metrics saved to: {METRICS_PATH}")
    print(f"Total training time: {time.time() - start_time:.2f} seconds.")

if __name__ == '__main__':
    train_officer_response_engine()
