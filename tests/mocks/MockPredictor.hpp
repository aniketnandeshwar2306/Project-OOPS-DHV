#pragma once

#include "interfaces/IPredictor.hpp"
#include <stdexcept>

namespace CivicCare {

/**
 * @brief Mock Predictor simulating Member 1's ML service.
 */
class MockPredictor : public IPredictor {
private:
    bool shouldFail_ = false;

public:
    void setShouldFail(bool fail) noexcept {
        shouldFail_ = fail;
    }

    Prediction predict(const std::string& description) override {
        if (shouldFail_) {
            throw std::runtime_error("ML inference server timeout or script error.");
        }

        // Simple keyword-based mock prediction
        if (description.find("water") != std::string::npos || description.find("pipe") != std::string::npos) {
            return {"Water Supply & Sewerage", 0.94};
        }
        if (description.find("pothole") != std::string::npos || description.find("road") != std::string::npos) {
            return {"Roads & Traffic Infrastructure", 0.91};
        }
        if (description.find("garbage") != std::string::npos || description.find("trash") != std::string::npos) {
            return {"Solid Waste Management", 0.88};
        }

        return {"General Municipal Inquiry", 0.65};
    }
};

} // namespace CivicCare
