# CitySync / CivicCare — Scientific Model Evaluation & Benchmark Report
**SIH 2025 Problem Statement 11: Crowdsourced Civic Issue Reporting and Resolution Analytics Dashboard**  
**Role:** Member 1 — Machine Learning & Data Pipeline  
**Architecture:** Tri-Model Municipal Intelligence Suite (Category Ensemble, Resolution Time Engine, Officer Response Verifier)  
**Date:** October 2026

---

## 1. Executive Summary & Architecture Overview

To achieve production-grade performance ($\ge 80\%$ benchmark across all metrics) without overfitting, underfitting, or data leakage, the Member 1 pipeline deploys **three complementary machine learning engines**:

| Model Engine | Target Variable | Core Algorithm | Test Dataset Size | Primary Benchmark (Holdout) | Target Met? |
| :--- | :--- | :--- | :--- | :--- | :---: |
| **1. Category Classification Ensemble** | 9 Master Civic Categories | 4-Model Soft-Voting Ensemble (Calibrated LinearSVC, LogisticRegression, ComplementNB, Calibrated Ridge) with Word+Char FeatureUnion | 1,490 held-out citizen complaints | **Accuracy: 99.06%**<br>**Macro F1: 0.9937**<br>**5-Fold CV: 99.38%** | **YES ($\ge 80\%$)** |
| **2. Resolution Time & SLA Engine** | SLA Compliance (On-Time vs Overdue) + Continuous Hours | HistGradientBoosting & Logistic Regression Ensemble + Log-Normal Regressor | 60,000 historical complaint records | **Accuracy: 86.71%**<br>**ROC-AUC: 0.9262**<br>**Macro F1: 0.8048**<br>**Median AE: 6.87h** | **YES ($\ge 80\%$)** |
| **3. Officer Response & Anomaly Engine** | Action Routing & Fake-Closure Verification | HistGradientBoosting & Balanced Classifier on Officer Remarks & Lifecycle | 241,550 municipal grievance records | **Accuracy: 92.95%**<br>**ROC-AUC: 0.9634**<br>**Macro F1: 0.8355**<br>**Recall (Fake): 86.88%** | **YES ($\ge 80\%$)** |

---

## 2. Model 1: Civic Category Classification Ensemble

### A. Architectural Design & Overfitting Prevention
- **Elimination of Target Leakage**: Input strictly uses citizen grievance descriptions (`descriptor`), completely eliminating parent department labels.
- **Subword Semantic Anchoring**: Uses `FeatureUnion` combining:
  1. Word n-grams $(1, 3)$ with sublinear term-frequency scaling.
  2. Character subword n-grams $(3, 5)$ with word-boundary awareness (`char_wb`). This prevents out-of-vocabulary failures on Indian municipal terms (e.g. *SWD*, *nalah*, *khata*, *pothole*, *transformer*).
- **Soft-Voting Ensemble**: Fuses posterior probability distributions across:
  - *Calibrated LinearSVC* (Hinge loss with Platt scaling calibration)
  - *Logistic Regression* (L2 regularization, tuned $C=6.0$, balanced class weights)
  - *Complement Naive Bayes* ($\alpha=0.1$, mathematically optimized for skewed text frequency)
  - *Calibrated Ridge Classifier* ($L2$ squared loss with calibration)

### B. 5-Fold Stratified Cross-Validation Results
- **Fold 1:** Accuracy: 99.06% | Macro F1: 0.9941
- **Fold 2:** Accuracy: 99.46% | Macro F1: 0.9966
- **Fold 3:** Accuracy: 99.73% | Macro F1: 0.9982
- **Fold 4:** Accuracy: 99.46% | Macro F1: 0.9966
- **Fold 5:** Accuracy: 99.19% | Macro F1: 0.9948
- **Mean Cross-Validation Accuracy:** **99.38% ($\pm 0.23\%$)**
- **Mean Cross-Validation Macro F1:** **0.9960 ($\pm 0.0015$)**

### C. Held-Out Test Set Per-Class Metrics (1,490 Samples)
| Master Civic Category | Precision | Recall | F1-Score | Support |
| :--- | :---: | :---: | :---: | :---: |
| **Animal Husbandry & Stray Animals** | 1.0000 | 1.0000 | **1.0000** | 141 |
| **Drainage & Water Supply** | 1.0000 | 0.9905 | **0.9952** | 211 |
| **Encroachments & Footpaths** | 1.0000 | 1.0000 | **1.0000** | 53 |
| **Horticulture & Greenery** | 1.0000 | 1.0000 | **1.0000** | 57 |
| **Public Health & Sanitation** | 0.9878 | 0.9918 | **0.9898** | 244 |
| **Revenue & Urban Governance** | 0.9813 | 0.9953 | **0.9882** | 211 |
| **Roads & Infrastructure** | 0.9770 | 1.0000 | **0.9884** | 170 |
| **Solid Waste Management** | 1.0000 | 1.0000 | **1.0000** | 73 |
| **Street Lighting & Electrical** | 0.9907 | 0.9727 | **0.9817** | 330 |
| **Overall Accuracy** | — | — | **99.06%** | 1,490 |
| **Macro Average** | 0.9930 | 0.9945 | **0.9937** | 1,490 |

---

## 3. Model 2: Resolution Time & SLA Compliance Engine

### A. Dual-Head Framework
Municipal resolution times exhibit extreme multi-month tail delays that corrupt ordinary linear regression. The engine solves this via a dual-head formulation:
1. **SLA Turnaround Classifier**: Predicts whether a complaint will be resolved within the Citizen Charter SLA window based on category, reporting channel, borough/ward, and temporal factors (creation hour, day of week, weekend).
2. **Continuous Turnaround Regressor**: Uses gradient boosting over log-transformed hours $\log(1 + \text{hours})$, blended with Indian Municipal Citizen Charter SLAs.

### B. SLA Compliance Holdout Results (60,000 Test Records)
- **Test Accuracy:** **86.71%** (Exceeds $\ge 80\%$ benchmark)
- **ROC-AUC Score:** **0.9262** (High discriminative power)
- **Macro F1-Score:** **0.8048**
- **Weighted F1-Score:** **0.8686**
- **Median Absolute Error:** **6.87 hours** (Resilient to tail delays)

### C. Indian Municipal Citizen Charter SLA Reference
| Master Category | Target Turnaround | Tolerable Max SLA | Priority Tier |
| :--- | :---: | :---: | :--- |
| **Street Lighting & Electrical** | 24.0h (1.0 day) | 48.0h | Immediate / High |
| **Solid Waste Management** | 24.0h (1.0 day) | 48.0h | Immediate / Medium |
| **Drainage & Water Supply** | 36.0h (1.5 days) | 72.0h | Urgent / High |
| **Animal Husbandry & Stray Animals** | 36.0h (1.5 days) | 72.0h | Standard / Medium |
| **Roads & Infrastructure** | 48.0h (2.0 days) | 96.0h | Standard / High |
| **Public Health & Sanitation** | 48.0h (2.0 days) | 96.0h | Standard / Medium |
| **Horticulture & Greenery** | 48.0h (2.0 days) | 120.0h | Standard / Low |
| **Encroachments & Footpaths** | 72.0h (3.0 days) | 168.0h | Extended / Medium |
| **Revenue & Urban Governance** | 120.0h (5.0 days) | 240.0h | Administrative / Low |

---

## 4. Model 3: Officer Response & Action Verification Engine

### A. Operational Action Routing
Maps complaints directly to field crews:
- **Roads & Infrastructure:** *Field Crew: Road Maintenance & Asphalt Paving*
- **Drainage & Water Supply:** *Field Crew: Stormwater Desilting & Pipe Repair*
- **Street Lighting & Electrical:** *Field Crew: Electrical Lineman & Luminaire Replacement*
- **Solid Waste Management:** *Sanitation Squad: Compact Truck & Debris Clearing*
- **Public Health & Sanitation:** *Health Inspector: Anti-Larval Spray & Sanitation Audit*
- **Animal Husbandry & Stray Animals:** *Veterinary Unit: Stray Animal Rescue & ABC Program*
- **Horticulture & Greenery:** *Forestry Squad: Tree Trimming & Branch Clearance*
- **Encroachments & Footpaths:** *Enforcement Team: Encroachment Eviction & Notice*
- **Revenue & Urban Governance:** *Revenue Officer: Cadastral / Khata Document Verification*

### B. Officer Response Quality & Fake Closure Detection
Evaluates officer remarks (`staff_remarks`), resolution speed, canned brush-offs (e.g. `"."`, `"-"`, `"closed"`, `"attended"`), and citizen ratings to flag premature closures.

**Evaluation on 241,550 Held-Out Records (from 1.2M BBMP Grievances):**
- **Test Accuracy:** **92.95%** (Exceeds $\ge 80\%$ benchmark)
- **ROC-AUC Score:** **0.9634** (Exceeds $\ge 0.85$ benchmark)
- **Macro F1-Score:** **0.8355**
- **Weighted F1-Score:** **0.9350**
- **Recall on Anomalous / Fake Closures:** **86.88%** (20,974 caught out of 24,142)

**Confusion Matrix (241,550 Test Records):**
$$\begin{pmatrix} 203,537 & 13,871 \\ 3,168 & 20,974 \end{pmatrix}$$

---

## 5. Unified Prediction Contract (C++ & API)

Run predictions directly from the command line:
```bash
python ml/predict.py --text "Open manhole and overflowing drain near the bus stop" --remarks "attended"
```

**JSON Output:**
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

---
*Report generated and validated on October 17, 2026 for CitySync / CivicCare SIH 2025.*
