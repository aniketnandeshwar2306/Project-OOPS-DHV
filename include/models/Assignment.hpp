#pragma once

#include <string>
#include "exceptions/ComplaintExceptions.hpp"
#include "models/Complaint.hpp"

namespace CivicCare {

/**
 * @brief Domain model capturing the assignment of a complaint to municipal staff.
 */
class Assignment {
private:
    int id_;
    int complaintId_;
    int staffId_;
    int departmentId_;
    int assignedByUserId_;
    std::string assignedAt_;
    std::string notes_;
    bool isCurrent_;

public:
    Assignment()
        : id_(0),
          complaintId_(0),
          staffId_(0),
          departmentId_(0),
          assignedByUserId_(0),
          assignedAt_(getCurrentTimestampString()),
          notes_(""),
          isCurrent_(true) {}

    Assignment(int complaintId,
               int staffId,
               int departmentId,
               int assignedByUserId,
               const std::string& notes = "")
        : id_(0),
          complaintId_(complaintId),
          staffId_(staffId),
          departmentId_(departmentId),
          assignedByUserId_(assignedByUserId),
          assignedAt_(getCurrentTimestampString()),
          notes_(notes),
          isCurrent_(true) {
        validate();
    }

    Assignment(int id,
               int complaintId,
               int staffId,
               int departmentId,
               int assignedByUserId,
               const std::string& assignedAt,
               const std::string& notes,
               bool isCurrent)
        : id_(id),
          complaintId_(complaintId),
          staffId_(staffId),
          departmentId_(departmentId),
          assignedByUserId_(assignedByUserId),
          assignedAt_(assignedAt),
          notes_(notes),
          isCurrent_(isCurrent) {
        validate();
    }

    void validate() const {
        if (complaintId_ <= 0) throw InvalidComplaintDataException("Invalid complaintId in Assignment.");
        if (staffId_ <= 0) throw InvalidComplaintDataException("Invalid staffId in Assignment.");
        if (departmentId_ <= 0) throw InvalidComplaintDataException("Invalid departmentId in Assignment.");
    }

    // Getters
    int getId() const noexcept { return id_; }
    int getComplaintId() const noexcept { return complaintId_; }
    int getStaffId() const noexcept { return staffId_; }
    int getDepartmentId() const noexcept { return departmentId_; }
    int getAssignedByUserId() const noexcept { return assignedByUserId_; }
    const std::string& getAssignedAt() const noexcept { return assignedAt_; }
    const std::string& getNotes() const noexcept { return notes_; }
    bool isCurrent() const noexcept { return isCurrent_; }

    // Setters
    void setId(int id) noexcept { id_ = id; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setIsCurrent(bool current) noexcept { isCurrent_ = current; }
};

} // namespace CivicCare
