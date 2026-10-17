# CivicCare ML Interface — C++ Integration Guide
**Author:** Member 1 (ML & Data Pipeline Lead)  
**Target Audience:** Member 3 (Complaint Service) & Member 4 (Terminal & Integration Lead)  
**Contract Version:** 2.0 (Dual-Prediction: Category + Resolution Time + Priority)

---

## 1. Overview
The machine learning pipeline provides a **standalone CLI interface** (`ml/predict.py`) and a **local REST API** (`/api/predict/complaint`) that converts an incoming citizen complaint description into:
1. **Civic Master Category** (one of the 9 unified categories)
2. **Confidence Score** ($0.0$ to $1.0$)
3. **Estimated Resolution Time** in hours and days (trained on NYC empirical data and calibrated to Indian Citizen Charter SLAs)
4. **SLA Tier** (e.g., *"Immediate"*, *"Standard"*, *"Extended"*)
5. **Recommended Priority** (`Low`, `Medium`, `High`, `Critical`)

---

## 2. Command-Line Invocation (Subprocess / Pipe)

Member 4 can execute the python script directly using C++ `popen` or `std::system`:

```bash
python ml/predict.py "Severe pothole on main road causing vehicle damage"
```

### JSON Output Schema

```json
{
  "status": "success",
  "category": "Roads & Infrastructure",
  "confidence": 0.958,
  "estimated_resolution_hours": 34.3,
  "estimated_resolution_days": 1.43,
  "sla_tier": "Standard (24 to 72 Hours)",
  "recommended_priority": "Critical"
}
```

### Fallback Guarantee (Fault Tolerance)
If the description is empty, invalid, or the ML model file is missing, the script **never crashes**. It returns a safe municipal fallback:

```json
{
  "status": "fallback",
  "category": "Roads & Infrastructure",
  "confidence": 0.50,
  "estimated_resolution_hours": 48.0,
  "estimated_resolution_days": 2.0,
  "sla_tier": "Standard (24 to 72 Hours)",
  "recommended_priority": "Medium",
  "message": "Description too short or empty."
}
```

---

## 3. C++ Shared Interface Implementation (Header Example)

Below is the concrete C++ implementation matching the agreed project interface:

```cpp
#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <array>
#include <sstream>

struct Prediction {
    std::string category;
    double confidence;
    double estimatedResolutionHours;
    std::string priority;
    std::string slaTier;
    bool isSuccess;
};

class Predictor {
public:
    virtual Prediction predict(const std::string& description) = 0;
    virtual ~Predictor() = default;
};

// Member 4 Integration Class
class MLSubprocessPredictor : public Predictor {
private:
    std::string pythonPath;
    std::string scriptPath;

public:
    MLSubprocessPredictor(
        const std::string& py = "python", 
        const std::string& script = "ml/predict.py"
    ) : pythonPath(py), scriptPath(script) {}

    Prediction predict(const std::string& description) override {
        // Sanitize quotes for command line safety
        std::string sanitized = description;
        for (char &c : sanitized) {
            if (c == '"' || c == '\'') c = ' ';
        }

        std::string command = pythonPath + " " + scriptPath + " \"" + sanitized + "\"";
        
        std::array<char, 256> buffer;
        std::string resultJson;
        
        #ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
        #else
        FILE* pipe = popen(command.c_str(), "r");
        #endif

        if (!pipe) {
            // Safe fallback if subprocess fails
            return {"Roads & Infrastructure", 0.50, 48.0, "Medium", "Standard", false};
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            resultJson += buffer.data();
        }

        #ifdef _WIN32
        _pclose(pipe);
        #else
        pclose(pipe);
        #endif

        return parseJson(resultJson);
    }

private:
    Prediction parseJson(const std::string& json) {
        Prediction pred;
        pred.category = "Roads & Infrastructure";
        pred.confidence = 0.50;
        pred.estimatedResolutionHours = 48.0;
        pred.priority = "Medium";
        pred.slaTier = "Standard (24 to 72 Hours)";
        pred.isSuccess = false;

        // Simple string search parser (or use nlohmann/json if configured in CMake)
        auto extractStr = [&](const std::string& key) -> std::string {
            auto pos = json.find("\"" + key + "\":");
            if (pos == std::string::npos) return "";
            auto start = json.find("\"", pos + key.length() + 3);
            if (start == std::string::npos) return "";
            auto end = json.find("\"", start + 1);
            if (end == std::string::npos) return "";
            return json.substr(start + 1, end - start - 1);
        };

        std::string status = extractStr("status");
        if (status == "success" || status == "fallback") {
            pred.category = extractStr("category");
            pred.priority = extractStr("recommended_priority");
            pred.slaTier = extractStr("sla_tier");
            pred.isSuccess = (status == "success");

            // Extract numeric confidence
            auto confPos = json.find("\"confidence\":");
            if (confPos != std::string::npos) {
                pred.confidence = std::stod(json.substr(confPos + 13, 5));
            }
            
            // Extract numeric hours
            auto hoursPos = json.find("\"estimated_resolution_hours\":");
            if (hoursPos != std::string::npos) {
                pred.estimatedResolutionHours = std::stod(json.substr(hoursPos + 29, 5));
            }
        }
        return pred;
    }
};
```

---

## 4. Test Suite for Member 3 & Member 4 Validation

Use these 5 test complaints in your C++ tests to verify end-to-end integration:

| Test # | Input Description | Expected Category | Expected Priority | Expected SLA |
|---|---|---|---|---|
| **Test 1** | `"Massive pothole on main road causing severe accident danger"` | `Roads & Infrastructure` | `Critical` | ~34 hours |
| **Test 2** | `"Garbage vehicle not arrived and black spot dumping in market"` | `Solid Waste Management` | `Medium` | ~29 hours |
| **Test 3** | `"Street light pole sparking and wire broken on road"` | `Street Lighting & Electrical` | `Critical` | ~24 hours |
| **Test 4** | `"Stray dog biting children in community playground"` | `Animal Husbandry & Stray Animals` | `Critical` | ~33 hours |
| **Test 5** | `"Dangerous dead tree branches fallen across footpath"` | `Horticulture & Greenery` | `Critical` | ~74 hours |
