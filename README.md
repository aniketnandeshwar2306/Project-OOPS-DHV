# Project-OOPS-DHV

## CitySync / CivicCare: Crowdsourced Civic Issue Reporting and Resolution Analytics Dashboard
**Smart India Hackathon (SIH 2025) — Problem Statement 11**  
**Coursework:** Object-Oriented Programming (OOP) & Data Handling & Visualization (DHV)  
**Team Role Allocation:** Member 1 — Machine Learning & Analytics Pipeline

---

## 1. Executive Summary

Municipal administrations across rapidly growing Indian cities struggle to track, prioritize, and resolve citizen-reported civic issues (potholes, garbage dumps, broken streetlights, waterlogging, encroachments) due to multi-channel data fragmentation (helplines, mobile apps, social media, walk-in ward visits). 

**CivicCare** solves this challenge through an integrated two-tier architecture:
1. **High-Performance C++ Core Application:** Object-oriented complaint lifecycle tracking, role-based access control, priority queuing, and SQLite storage.
2. **Tri-Model Machine Learning Suite:** NLP civic category classification, Citizen Charter SLA resolution turnaround estimation, and officer closure remark anomaly verification.
3. **Power BI Business Intelligence Dashboard:** Interactive executive analytics, ward-level bottleneck identification, lifecycle funnel analysis, and temporal inflow heatmaps.

---

## 2. Machine Learning Architecture (Tri-Model Suite)

To achieve production-grade performance ($\ge 80\%$ benchmark across all metrics) without data leakage, overfitting, or underfitting, the system deploys three complementary models:

| Model | Target Variable | Core Architecture | Benchmark on Held-Out Test Data | Independent Web Audit Score |
| :--- | :--- | :--- | :--- | :---: |
| **1. Category Classification Ensemble** | 9 Master Civic Categories | 4-Model Soft-Voting Ensemble (Calibrated LinearSVC, LogisticRegression, ComplementNB, Calibrated Ridge) with Word+Char `FeatureUnion` | **Test Acc: 99.33%**<br>**Macro F1: 0.9947**<br>**5-Fold CV: 99.38%** | **98.00%** (49/50 on live CPGRAMS/X/Reddit complaints) |
| **2. Resolution Time & SLA Engine** | Citizen Charter SLA Compliance (On-Time vs Overdue) + Continuous Hours | HistGradientBoosting & Logistic Regression Ensemble + Log-Normal Regressor | **SLA Acc: 87.12%**<br>**ROC-AUC: 0.9274**<br>**Macro F1: 0.8120** | **Median AE: 6.87 hrs** (Resilient to multi-month tail delays) |
| **3. Officer Response & Anomaly Verifier** | Action Routing & Fake-Closure Detection | HistGradientBoosting Classifier on Officer Remarks (`staff_remarks`) | **Test Acc: 92.93%**<br>**ROC-AUC: 0.9629**<br>**Macro F1: 0.8355** | **86.88% recall on premature/canned closures** |

---

## 3. Academic Notebook Suite

The repository includes a 3-part academic notebook series pre-rendered with executed code, visual figures, and mathematical explanations:

- **[`notebooks/01_EDA_and_Data_Preprocessing.ipynb`](notebooks/01_EDA_and_Data_Preprocessing.ipynb):**
  - Dataset auditing of NYC 311 (3.65M records) and Bengaluru BBMP (1.21M records).
  - Skewness analysis of resolution turnaround (Median 6.61h vs Mean 236.35h) justifying log-transformation.
  - Identification of BBMP's 130 fixed dropdown descriptors and resolving label noise via statistical majority voting.
- **[`notebooks/02_Feature_Engineering_and_Leakage_Audit.ipynb`](notebooks/02_Feature_Engineering_and_Leakage_Audit.ipynb):**
  - Demonstration of Target Label Leakage elimination (restricting features strictly to pure citizen description text).
  - Subword text engineering (`FeatureUnion` of Word $1-3$ n-grams + Char $3-5$ subwords).
  - Discovery and resolution of the `"road"` spurious location correlation via location-invariance augmentation.
  - Dual-head SLA target engineering and officer remark linguistic feature extraction.
- **[`notebooks/03_Model_Selection_Ensemble_and_Evaluation.ipynb`](notebooks/03_Model_Selection_Ensemble_and_Evaluation.ipynb):**
  - Multi-algorithm comparative selection (Naive Bayes, Logistic Regression, LinearSVC, Ridge, Gradient Boosting).
  - Soft-Voting Ensemble probability weighting formula: $\hat{y} = \arg\max_c \sum w_m P_m(y=c \mid \mathbf{x})$.
  - Validation against 50 unseen real-world citizen complaints from the web.
  - Interactive prediction widget.

---

## 4. Power BI Dashboard & Business Intelligence

Curated Star-Schema tables, DAX measures, and pre-rendered executive dashboards are available in **[`powerbi/`](powerbi/)**:

- **Star-Schema Data Model:** Fact table (`fact_grievances.csv`) linked to Dimension tables (`dim_category.csv`, `dim_ward.csv`, `dim_date.csv`).
- **Comprehensive DAX Library:** Volume, Resolution Rate %, Median Turnaround, SLA Compliance %, Fake Closure Anomaly Rate %, and Ward Density per 10k population.
- **Pre-rendered Visual Previews:**
  - `powerbi/visuals/01_executive_kpi_overview.png` — Real-Time Complaint Inflow & Multi-Channel Fragmentation.
  - `powerbi/visuals/02_ward_geographic_performance.png` — Bottleneck Quadrant (Density vs Turnaround) & Zone SLA Breakdown.
  - `powerbi/visuals/03_complaint_lifecycle_funnel.png` — 6-Stage Redressal Funnel & Turnaround vs SLA Target.
  - `powerbi/visuals/04_accountability_fake_closure.png` — Zone Fake Closure Alert Ranking & Anomaly Modes.
  - `powerbi/visuals/05_temporal_inflow_heatmap.png` — 7-Day $\times$ 24-Hour Matrix Heatmap.

---

## 5. C++ Application & API Integration Contract

### CLI Inference (Subprocess Contract for C++ Application)
```bash
python ml/predict.py --text "Open manhole and overflowing drain near the bus stop" --remarks "attended"
```

**JSON Output Contract:**
```json
{
  "status": "success",
  "category": "Drainage & Water Supply",
  "confidence": 0.928,
  "estimated_resolution_hours": 27.2,
  "estimated_resolution_days": 1.13,
  "sla_tier": "Urgent (24 to 48 Hours)",
  "sla_compliance_probability": 0.726,
  "recommended_priority": "High",
  "officer_response": {
    "recommended_action": "Field Crew: Stormwater Desilting & Pipe Repair",
    "verification_status": "Verified: Genuine Resolution",
    "fake_closure_risk": 0.098
  }
}
```

### FastAPI REST Endpoints
```bash
uvicorn api.main:app --host 127.0.0.1 --port 8000
```
- `POST /api/predict/complaint` — Returns category, resolution time, priority, and officer action.
- `POST /api/predict/fake-closure` — Audits officer remarks and predicts premature closure risk.
- `GET /api/kpi/summary` — Returns high-level municipal KPIs across NYC and Bengaluru.

---

## 6. Repository Organization

```text
CitySync/
├── .gitignore                               (Protects Git push from files > 100MB)
├── README.md                                (Project documentation)
├── api/
│   └── main.py                              (FastAPI REST endpoints)
├── data/
│   ├── raw/                                 (Large raw datasets excluded by .gitignore)
│   └── processed/                           (Preprocessed Parquets & sample CSVs)
├── Datasets/
│   └── citizen-grievances.csv               (Raw BBMP dataset excluded by .gitignore)
├── figures/                                 (Analytical and confusion matrix plots)
├── ml/
│   ├── 01_train_category_ensemble.py        (Category ensemble training pipeline)
│   ├── 02_train_resolution_time_model.py    (Resolution time & SLA engine training)
│   ├── 03_train_officer_response_model.py   (Officer response & fake closure training)
│   ├── config.py                            (Taxonomy mappings & Citizen Charter SLAs)
│   ├── CPP_INTEGRATION_GUIDE.md             (C++ subprocess header & integration guide)
│   ├── MODEL_EVALUATION_REPORT.md           (Scientific evaluation report)
│   ├── predict.py                           (Unified CLI predictor)
│   ├── verify_new_web_data.py               (Independent web data audit suite)
│   └── models/                              (Trained production model artifacts)
├── notebooks/
│   ├── 01_EDA_and_Data_Preprocessing.ipynb
│   ├── 02_Feature_Engineering_and_Leakage_Audit.ipynb
│   └── 03_Model_Selection_Ensemble_and_Evaluation.ipynb
└── powerbi/
    ├── DAX_MEASURES.dax                     (Ready-to-use DAX measures)
    ├── DAX_MEASURES.md                      (DAX formulas reference guide)
    ├── export_powerbi_data.py               (Star-schema data export pipeline)
    ├── generate_dashboard_visuals.py        (Python dashboard visual generator)
    ├── POWER_BI_DASHBOARD_GUIDE.md          (Multi-page Power BI dashboard guide)
    ├── data/                                (Star-schema Fact and Dimension CSVs)
    └── visuals/                             (5 High-res dashboard visual exports)
```

---

## 7. License & Academic Attribution
Developed for Smart India Hackathon (SIH 2025) Problem Statement 11. All datasets are public open data under the Open Government Data (OGD) Platform India and NYC Open Data.
