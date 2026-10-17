# CitySync / CivicCare — Power BI DAX Measures Reference Guide
**Problem Statement 11: Crowdsourced Civic Issue Reporting and Resolution Analytics Dashboard**  
**Role:** Member 1 — Machine Learning & Analytics Pipeline

---

## 1. Overview & Data Model Relationships

To enable fast visual slicing and time-intelligence in Power BI Desktop, the following **Star-Schema Relationships** should be established:

| From Table (Fact) | Foreign Key | To Table (Dimension) | Primary Key | Cardinality | Cross Filter |
| :--- | :--- | :--- | :--- | :---: | :---: |
| `fact_grievances` | `standard_category` | `dim_category` | `category_name` | Many to One ($* : 1$) | Single |
| `fact_grievances` | `ward` | `dim_ward` | `ward_name` | Many to One ($* : 1$) | Single |
| `fact_grievances` | `created_date` | `dim_date` | `Date` | Many to One ($* : 1$) | Single |

---

## 2. Core Operational DAX Measures

### A. Volume & Resolution Redressal

#### 1. `Total Complaints`
```dax
Total Complaints = COUNTROWS(fact_grievances)
```
- **Description:** Returns the total volume of complaints based on current slicers (date, category, ward, or channel).
- **Format:** Whole Number with thousands separator (e.g. `200,000`).

#### 2. `Closed Complaints`
```dax
Closed Complaints = 
CALCULATE(
    COUNTROWS(fact_grievances),
    fact_grievances[status] IN {"Closed", "Resolved"}
)
```
- **Description:** Filters for tickets marked as closed or resolved by municipal staff.

#### 3. `Resolution Rate %`
```dax
Resolution Rate % = 
DIVIDE([Closed Complaints], [Total Complaints], 0)
```
- **Description:** Proportion of lodged grievances that reached closure.
- **Format:** Percentage (`0.0%`).

---

### B. Citizen Charter SLA & Turnaround Time

#### 4. `Average Resolution Hours`
```dax
Average Resolution Hours = AVERAGE(fact_grievances[resolution_time_hours])
```
- **Description:** The arithmetic mean of resolution turnaround time in hours.
- **Format:** Decimal (`0.0` hrs).

#### 5. `Median Resolution Hours`
```dax
Median Resolution Hours = MEDIAN(fact_grievances[resolution_time_hours])
```
- **Description:** The 50th percentile resolution time. **Critical for civic analytics** because municipal resolution times are heavily right-skewed by rare multi-month legal disputes; median provides the true representative citizen experience.
- **Format:** Decimal (`0.0` hrs).

#### 6. `SLA Compliance %`
```dax
SLA Compliance % = 
DIVIDE(
    CALCULATE(COUNTROWS(fact_grievances), fact_grievances[within_sla] = 1),
    [Total Complaints],
    0
)
```
- **Description:** Measures whether grievances met the Indian Municipal Citizen Charter turnaround target for their category (e.g. $\le 24$h for streetlights, $\le 36$h for water supply, $\le 48$h for road potholes).
- **Format:** Percentage (`0.0%`).

#### 7. `SLA Breach %`
```dax
SLA Breach % = 1 - [SLA Compliance %]
```
- **Description:** Identifies the failure rate where municipal departments violated citizen charter deadlines.

---

### C. Accountability & Fake-Closure Anomaly Detection

#### 8. `Fake Closures Count`
```dax
Fake Closures Count = 
CALCULATE(
    COUNTROWS(fact_grievances),
    fact_grievances[is_fake_closure] = 1
)
```
- **Description:** Counts complaints closed with suspicious fast turnaround times, canned brush-off remarks (e.g. `"."`, `"-"`, `"closed"`), or low citizen ratings.

#### 9. `Fake Closure Rate %`
```dax
Fake Closure Rate % = 
DIVIDE([Fake Closures Count], [Closed Complaints], 0)
```
- **Description:** Ratio of flagged premature closures relative to all closed tickets. Used to rank supervisor zones requiring administrative audit.
- **Format:** Percentage (`0.0%`).

#### 10. `Genuine Resolution Rate %`
```dax
Genuine Resolution Rate % = 
DIVIDE([Closed Complaints] - [Fake Closures Count], [Total Complaints], 0)
```
- **Description:** True operational effectiveness discounting superficial closures.

---

### D. Geographic Density & Ward Normalization

#### 11. `Complaint Density per 10k Pop`
```dax
Complaint Density per 10k Pop = 
DIVIDE([Total Complaints], SUM(dim_ward[population_in_10k]), 0)
```
- **Description:** Population-adjusted complaint density per 10,000 residents. Eliminates population bias when comparing massive suburban wards (like Mahadevapura) against smaller core urban wards.

---

### E. Time Intelligence & Growth

#### 12. `Complaints MTD`
```dax
Complaints MTD = TOTALMTD([Total Complaints], dim_date[Date])
```
- **Description:** Month-to-date cumulative grievance filings.

#### 13. `MoM Growth %`
```dax
MoM Growth % = 
VAR PrevMonth = CALCULATE([Total Complaints], DATEADD(dim_date[Date], -1, MONTH))
RETURN
DIVIDE([Total Complaints] - PrevMonth, PrevMonth, 0)
```
- **Description:** Month-over-month percentage change in grievance inflow.
