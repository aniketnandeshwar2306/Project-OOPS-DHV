#pragma once

#include <string>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "exceptions/ComplaintExceptions.hpp"
#include "utils/Optional.hpp"

namespace CivicCare {

/**
 * @brief Priority levels for complaints.
 * Higher integer values represent higher urgency in the priority queue.
 */
enum class Priority {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3,
    CRITICAL = 4
};

/**
 * @brief Lifecycle statuses of a civic complaint.
 */
enum class ComplaintStatus {
    REGISTERED,
    ASSIGNED,
    IN_PROGRESS,
    RESOLVED,
    CLOSED,
    REOPENED
};

// --- String Conversion Helpers ---

inline std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::LOW:      return "LOW";
        case Priority::MEDIUM:   return "MEDIUM";
        case Priority::HIGH:     return "HIGH";
        case Priority::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

inline Priority stringToPriority(const std::string& str) {
    if (str == "LOW" || str == "low")           return Priority::LOW;
    if (str == "MEDIUM" || str == "medium")     return Priority::MEDIUM;
    if (str == "HIGH" || str == "high")         return Priority::HIGH;
    if (str == "CRITICAL" || str == "critical") return Priority::CRITICAL;
    throw InvalidComplaintDataException("Unknown priority string: " + str);
}

inline std::string statusToString(ComplaintStatus s) {
    switch (s) {
        case ComplaintStatus::REGISTERED:  return "REGISTERED";
        case ComplaintStatus::ASSIGNED:    return "ASSIGNED";
        case ComplaintStatus::IN_PROGRESS: return "IN_PROGRESS";
        case ComplaintStatus::RESOLVED:    return "RESOLVED";
        case ComplaintStatus::CLOSED:      return "CLOSED";
        case ComplaintStatus::REOPENED:    return "REOPENED";
        default:                           return "UNKNOWN";
    }
}

inline ComplaintStatus stringToStatus(const std::string& str) {
    if (str == "REGISTERED" || str == "registered")   return ComplaintStatus::REGISTERED;
    if (str == "ASSIGNED" || str == "assigned")       return ComplaintStatus::ASSIGNED;
    if (str == "IN_PROGRESS" || str == "in_progress") return ComplaintStatus::IN_PROGRESS;
    if (str == "RESOLVED" || str == "resolved")       return ComplaintStatus::RESOLVED;
    if (str == "CLOSED" || str == "closed")           return ComplaintStatus::CLOSED;
    if (str == "REOPENED" || str == "reopened")       return ComplaintStatus::REOPENED;
    throw InvalidComplaintDataException("Unknown status string: " + str);
}

/**
 * @brief Helper to generate current UTC ISO-8601 timestamp string.
 */
inline std::string getCurrentTimestampString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_ptr = std::gmtime(&now_c);
    std::tm tm_buf = tm_ptr ? *tm_ptr : std::tm{};

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

/**
 * @brief Domain Model representing a civic complaint in CivicCare.
 * Enforces data encapsulation, validation rules, and lifecycle integrity.
 */
class Complaint {
private:
    int id_;
    int citizenId_;
    std::string description_;
    std::string category_;
    int wardId_;
    Priority priority_;
    ComplaintStatus status_;
    std::string createdAt_;
    std::string updatedAt_;
    std::string locationDetails_;
    Optional<int> assignedStaffId_;
    Optional<int> departmentId_;

public:
    // Default constructor
    Complaint()
        : id_(0),
          citizenId_(0),
          description_(""),
          category_("Unclassified"),
          wardId_(0),
          priority_(Priority::MEDIUM),
          status_(ComplaintStatus::REGISTERED),
          createdAt_(getCurrentTimestampString()),
          updatedAt_(createdAt_),
          locationDetails_(""),
          assignedStaffId_(NullOpt),
          departmentId_(NullOpt) {}

    // Parameterized constructor for registering new complaints
    Complaint(int citizenId,
              const std::string& description,
              const std::string& category,
              int wardId,
              Priority priority = Priority::MEDIUM,
              const std::string& locationDetails = "")
        : id_(0),
          citizenId_(citizenId),
          description_(description),
          category_(category.empty() ? "Unclassified" : category),
          wardId_(wardId),
          priority_(priority),
          status_(ComplaintStatus::REGISTERED),
          createdAt_(getCurrentTimestampString()),
          updatedAt_(createdAt_),
          locationDetails_(locationDetails),
          assignedStaffId_(NullOpt),
          departmentId_(NullOpt) {
        validate();
    }

    // Full constructor (typically used when loading from persistence/SQLite)
    Complaint(int id,
              int citizenId,
              const std::string& description,
              const std::string& category,
              int wardId,
              Priority priority,
              ComplaintStatus status,
              const std::string& createdAt,
              const std::string& updatedAt,
              const std::string& locationDetails = "",
              Optional<int> assignedStaffId = NullOpt,
              Optional<int> departmentId = NullOpt)
        : id_(id),
          citizenId_(citizenId),
          description_(description),
          category_(category.empty() ? "Unclassified" : category),
          wardId_(wardId),
          priority_(priority),
          status_(status),
          createdAt_(createdAt),
          updatedAt_(updatedAt),
          locationDetails_(locationDetails),
          assignedStaffId_(assignedStaffId),
          departmentId_(departmentId) {
        validate();
    }

    // --- Validation Rules ---
    void validate() const {
        if (citizenId_ <= 0) {
            throw InvalidComplaintDataException("citizenId must be a positive integer.");
        }
        if (description_.length() < 5) {
            throw InvalidComplaintDataException("Complaint description is too short (min 5 characters required).");
        }
        if (description_.length() > 3000) {
            throw InvalidComplaintDataException("Complaint description exceeds max length (3000 characters).");
        }
        if (wardId_ <= 0) {
            throw InvalidComplaintDataException("wardId must be a positive integer (valid municipality ward).");
        }
    }

    // --- Getters ---
    int getId() const noexcept { return id_; }
    int getCitizenId() const noexcept { return citizenId_; }
    const std::string& getDescription() const noexcept { return description_; }
    const std::string& getCategory() const noexcept { return category_; }
    int getWardId() const noexcept { return wardId_; }
    Priority getPriority() const noexcept { return priority_; }
    ComplaintStatus getStatus() const noexcept { return status_; }
    const std::string& getCreatedAt() const noexcept { return createdAt_; }
    const std::string& getUpdatedAt() const noexcept { return updatedAt_; }
    const std::string& getLocationDetails() const noexcept { return locationDetails_; }
    Optional<int> getAssignedStaffId() const noexcept { return assignedStaffId_; }
    Optional<int> getDepartmentId() const noexcept { return departmentId_; }

    // --- Setters / Mutators ---
    void setId(int id) noexcept { id_ = id; }

    void setDescription(const std::string& desc) {
        if (desc.length() < 5) {
            throw InvalidComplaintDataException("Description too short.");
        }
        description_ = desc;
        touch();
    }

    void setCategory(const std::string& cat) {
        category_ = cat.empty() ? "Unclassified" : cat;
        touch();
    }

    void setWardId(int wardId) {
        if (wardId <= 0) {
            throw InvalidComplaintDataException("wardId must be positive.");
        }
        wardId_ = wardId;
        touch();
    }

    void setPriority(Priority p) noexcept {
        priority_ = p;
        touch();
    }

    void setStatus(ComplaintStatus s) noexcept {
        status_ = s;
        touch();
    }

    void setLocationDetails(const std::string& loc) {
        locationDetails_ = loc;
        touch();
    }

    void assignStaff(int staffId, int departmentId) {
        if (staffId <= 0 || departmentId <= 0) {
            throw InvalidComplaintDataException("Staff ID and Department ID must be positive.");
        }
        assignedStaffId_ = staffId;
        departmentId_ = departmentId;
        touch();
    }

    void clearAssignment() noexcept {
        assignedStaffId_ = NullOpt;
        departmentId_ = NullOpt;
        touch();
    }

    void touch() {
        updatedAt_ = getCurrentTimestampString();
    }
};

} // namespace CivicCare
