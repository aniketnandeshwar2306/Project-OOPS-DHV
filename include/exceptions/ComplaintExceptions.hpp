#pragma once

#include <stdexcept>
#include <string>

namespace CivicCare {

/**
 * @brief Base exception class for all CivicCare domain and business logic errors.
 */
class CivicCareException : public std::runtime_error {
public:
    explicit CivicCareException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Thrown when input data for a complaint fails domain validation rules.
 */
class InvalidComplaintDataException : public CivicCareException {
public:
    explicit InvalidComplaintDataException(const std::string& message)
        : CivicCareException("Validation Error: " + message) {}
};

/**
 * @brief Thrown when an illegal lifecycle state transition is attempted
 *        (e.g., trying to transition from REGISTERED directly to CLOSED).
 */
class InvalidStatusTransitionException : public CivicCareException {
public:
    InvalidStatusTransitionException(const std::string& fromStatus, const std::string& toStatus, const std::string& reason = "")
        : CivicCareException("Invalid Status Transition: Cannot move from '" + fromStatus + 
                             "' to '" + toStatus + "'" + (reason.empty() ? "" : (" (" + reason + ")"))) {}
};

/**
 * @brief Thrown when a complaint or related entity cannot be located.
 */
class ComplaintNotFoundException : public CivicCareException {
public:
    explicit ComplaintNotFoundException(int complaintId)
        : CivicCareException("Complaint with ID " + std::to_string(complaintId) + " was not found.") {}
};

/**
 * @brief Thrown when an operation is unauthorized for the given user role/context.
 */
class UnauthorizedOperationException : public CivicCareException {
public:
    explicit UnauthorizedOperationException(const std::string& message)
        : CivicCareException("Unauthorized Operation: " + message) {}
};

} // namespace CivicCare
