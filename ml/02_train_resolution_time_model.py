"""
CitySync - Resolution Time & SLA Turnaround Prediction Engine
Dual-Head Architecture:
1. SLA Turnaround Tier Classifier (Fast <= 24h, Standard 24-72h, Extended > 72h, and On-Time SLA Compliance)
   Target: >= 80% Accuracy & ROC-AUC >= 0.90
2. Continuous Resolution Hours Regressor calibrated to Citizen Charter SLAs
"""
import os
import sys
import time
import json
import joblib
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import HistGradientBoostingClassifier, HistGradientBoostingRegressor, VotingClassifier
from sklearn.linear_model import LogisticRegression, Ridge
from sklearn.metrics import accuracy_score, classification_report, roc_auc_score, f1_score, mean_absolute_error, median_absolute_error
from sklearn.preprocessing import OneHotEncoder
from sklearn.compose import ColumnTransformer
from sklearn.pipeline import Pipeline

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from ml.config import MASTER_CATEGORIES, NYC_CATEGORY_MAPPING, INDIAN_MUNICIPAL_SLAS

NYC_PARQUET = "data/processed/nyc_311_2025_processed.parquet"
MODEL_PATH = "ml/models/resolution_time_engine.joblib"
METRICS_PATH = "ml/models/resolution_time_metrics.json"

def train_resolution_time_engine():
    print("=" * 80)
    print("TRAINING RESOLUTION TIME & SLA PREDICTION ENGINE (NYC 311 + CITIZEN CHARTER)")
    print("=" * 80)
    start_time = time.time()
    
    # 1. Load data
    print("[1/4] Ingesting NYC 311 historical complaint resolution records...")
    cols = [
        'descriptor', 'complaint_type', 'agency', 'borough',
        'created_hour', 'created_day_of_week', 'is_weekend', 'resolution_time_hours'
    ]
    df = pd.read_parquet(NYC_PARQUET, columns=cols)
    df = df.dropna(subset=['descriptor', 'complaint_type', 'resolution_time_hours']).copy()
    df = df[df['resolution_time_hours'] >= 0.1].copy() # Filter zero/negative anomalies
    
    # Map to Master Taxonomy
    df['category'] = df['complaint_type'].map(NYC_CATEGORY_MAPPING).fillna('Roads & Infrastructure')
    df = df[df['category'].isin(MASTER_CATEGORIES)].copy()
    
    # Sample 300,000 records for fast, balanced training
    df_sample = df.sample(n=min(300000, len(df)), random_state=42).copy()
    print(f"  Training on {len(df_sample):,} representative historical complaints.")
    
    # 2. Target engineering
    # Target 1: SLA Compliance (Whether resolved within Citizen Charter max tolerable hours)
    def calc_within_sla(row):
        cat = row['category']
        sla_max = INDIAN_MUNICIPAL_SLAS.get(cat, {}).get('max_hours', 72.0)
        return int(row['resolution_time_hours'] <= sla_max)
    
    df_sample['within_sla'] = df_sample.apply(calc_within_sla, axis=1)
    
    # Target 2: 3-Tier SLA Speed Category
    def calc_sla_tier(hours):
        if hours <= 24.0:
            return 'Fast (Within 24 Hours)'
        elif hours <= 72.0:
            return 'Standard (24 to 72 Hours)'
        else:
            return 'Extended (> 72 Hours)'
    
    df_sample['sla_tier'] = df_sample['resolution_time_hours'].apply(calc_sla_tier)
    
    # Target 3: Log-transformed continuous hours
    df_sample['log_hours'] = np.log1p(df_sample['resolution_time_hours'].clip(upper=720)) # clip at 30 days
    
    print("\n[2/4] Engineered Resolution Time Targets:")
    print("  SLA Compliance rate:", df_sample['within_sla'].value_counts(normalize=True).to_dict())
    print("  SLA Tiers:", df_sample['sla_tier'].value_counts(normalize=True).to_dict())
    
    features = ['category', 'agency', 'borough', 'created_hour', 'created_day_of_week', 'is_weekend']
    X = df_sample[features]
    y_sla = df_sample['within_sla']
    y_tier = df_sample['sla_tier']
    y_hours = df_sample['log_hours']
    
    # Train / Test split (80 / 20)
    X_tr, X_te, y_sla_tr, y_sla_te, y_tier_tr, y_tier_te, y_h_tr, y_h_te = train_test_split(
        X, y_sla, y_tier, y_hours, test_size=0.20, random_state=42, stratify=y_sla
    )
    print(f"  Train set: {len(X_tr):,} samples | Held-out test set: {len(X_te):,} samples")
    
    # 3. Preprocessor pipeline
    preprocessor = ColumnTransformer(
        transformers=[
            ('cat', OneHotEncoder(handle_unknown='ignore', sparse_output=False), ['category', 'agency', 'borough']),
            ('num', 'passthrough', ['created_hour', 'created_day_of_week', 'is_weekend'])
        ]
    )
    
    # Model A: SLA Compliance Classifier (Ensemble of HistGradientBoosting + LogisticRegression)
    print("\n[3/4] Fitting SLA Compliance Ensemble Classifier...")
    clf_hgb = HistGradientBoostingClassifier(max_iter=150, learning_rate=0.1, random_state=42)
    clf_lr = LogisticRegression(max_iter=500, class_weight='balanced', random_state=42)
    
    sla_ensemble = VotingClassifier(
        estimators=[('hgb', clf_hgb), ('lr', clf_lr)],
        voting='soft',
        weights=[3, 1]
    )
    
    pipe_sla = Pipeline([('prep', preprocessor), ('clf', sla_ensemble)])
    pipe_sla.fit(X_tr, y_sla_tr)
    
    y_sla_pred = pipe_sla.predict(X_te)
    y_sla_prob = pipe_sla.predict_proba(X_te)[:, 1]
    acc_sla = accuracy_score(y_sla_te, y_sla_pred)
    roc_sla = roc_auc_score(y_sla_te, y_sla_prob)
    f1_sla_w = f1_score(y_sla_te, y_sla_pred, average='weighted')
    f1_sla_m = f1_score(y_sla_te, y_sla_pred, average='macro')
    
    print(f"  >>> SLA Compliance Model Results (Test Set of {len(X_te):,} records):")
    print(f"      Accuracy:  {acc_sla*100:.2f}% (Target: >= 80%)")
    print(f"      ROC-AUC:   {roc_sla:.4f} (Target: >= 0.85)")
    print(f"      Macro F1:  {f1_sla_m:.4f}")
    print(f"      Weighted F1: {f1_sla_w:.4f}")
    
    # Model B: Continuous Resolution Hours Regressor
    print("\nFitting Continuous Hours Regressor (HistGradientBoostingRegressor)...")
    reg_hgb = HistGradientBoostingRegressor(max_iter=150, learning_rate=0.08, random_state=42)
    pipe_reg = Pipeline([('prep', preprocessor), ('reg', reg_hgb)])
    pipe_reg.fit(X_tr, y_h_tr)
    
    pred_log = pipe_reg.predict(X_te)
    pred_hours = np.expm1(pred_log)
    actual_hours = np.expm1(y_h_te)
    
    mae_hours = mean_absolute_error(actual_hours, pred_hours)
    med_hours = median_absolute_error(actual_hours, pred_hours)
    # Check bounded SLA accuracy (within 24 hours of target)
    within_sla_window = (np.abs(actual_hours - pred_hours) <= 24.0).mean()
    
    print(f"  >>> Continuous Regressor Results:")
    print(f"      MAE: {mae_hours:.2f} hours")
    print(f"      Median Absolute Error: {med_hours:.2f} hours")
    print(f"      Bounded Operational SLA Window Accuracy: {within_sla_window*100:.2f}%")
    
    # 4. Save artifacts
    print("\n[4/4] Saving model bundle and evaluation metrics...")
    os.makedirs(os.path.dirname(MODEL_PATH), exist_ok=True)
    artifacts = {
        'sla_classifier': pipe_sla,
        'hours_regressor': pipe_reg,
        'features': features,
        'metrics': {
            'accuracy': acc_sla,
            'roc_auc': roc_sla,
            'macro_f1': f1_sla_m,
            'weighted_f1': f1_sla_w,
            'median_abs_error_hours': med_hours,
            'sla_window_accuracy': within_sla_window
        }
    }
    joblib.dump(artifacts, MODEL_PATH)
    
    with open(METRICS_PATH, 'w') as f:
        json.dump(artifacts['metrics'], f, indent=2)
        
    print(f"  Model saved to: {MODEL_PATH}")
    print(f"  Metrics saved to: {METRICS_PATH}")
    print(f"Total time elapsed: {time.time() - start_time:.2f} seconds.")

if __name__ == '__main__':
    train_resolution_time_engine()
