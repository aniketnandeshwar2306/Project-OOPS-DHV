#pragma once

#include <string>

namespace CivicCare {

/**
 * @brief Prediction result supplied by Member 1's ML inference model.
 */
struct Prediction {
    std::string category;
    double confidence; // Value between 0.0 and 1.0
};

/**
 * @brief Contract interface for Category & Urgency prediction (Provided by Member 1).
 */
class IPredictor {
public:
    virtual ~IPredictor() = default;

    /**
     * @brief Predict the municipal complaint category and confidence for a description.
     * @param description The raw complaint text.
     * @return Prediction structure containing predicted category and confidence.
     */
    virtual Prediction predict(const std::string& description) = 0;
};

} // namespace CivicCare
