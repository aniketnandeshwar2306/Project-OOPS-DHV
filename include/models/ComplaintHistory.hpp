#pragma once

#include <string>
#include "exceptions/ComplaintExceptions.hpp"
#include "models/Complaint.hpp"

namespace CivicCare {

/**
 * @brief Domain model capturing an immutable audit trail entry of status changes.
 */
class ComplaintHistory {
private:
    int id_;
    int complaintId_;
    ComplaintStatus fromStatus_;
    ComplaintStatus toStatus_;
    int changedByUserId_;
    std::string changedAt_;
    std::string remarks_;

public:
    ComplaintHistory()
        : id_(0),
          complaintId_(0),
          fromStatus_(ComplaintStatus::REGISTERED),
          toStatus_(ComplaintStatus::REGISTERED),
          changedByUserId_(0),
          changedAt_(getCurrentTimestampString()),
          remarks_("") {}

    ComplaintHistory(int complaintId,
                     ComplaintStatus fromStatus,
                     ComplaintStatus toStatus,
                     int changedByUserId,
                     const std::string& remarks = "")
        : id_(0),
          complaintId_(complaintId),
          fromStatus_(fromStatus),
          toStatus_(toStatus),
          changedByUserId_(changedByUserId),
          changedAt_(getCurrentTimestampString()),
          remarks_(remarks) {
        validate();
    }

    ComplaintHistory(int id,
                     int complaintId,
                     ComplaintStatus fromStatus,
                     ComplaintStatus toStatus,
                     int changedByUserId,
                     const std::string& changedAt,
                     const std::string& remarks)
        : id_(id),
          complaintId_(complaintId),
          fromStatus_(fromStatus),
          toStatus_(toStatus),
          changedByUserId_(changedByUserId),
          changedAt_(changedAt),
          remarks_(remarks) {
        validate();
    }

    void validate() const {
        if (complaintId_ <= 0) {
            throw InvalidComplaintDataException("ComplaintHistory must refer to a valid complaintId > 0.");
        }
        if (changedByUserId_ <= 0) {
            throw InvalidComplaintDataException("ComplaintHistory must record who made the change (userId > 0).");
        }
    }

    // Getters
    int getId() const noexcept { return id_; }
    int getComplaintId() const noexcept { return complaintId_; }
    ComplaintStatus getFromStatus() const noexcept { return fromStatus_; }
    ComplaintStatus getToStatus() const noexcept { return toStatus_; }
    int getChangedByUserId() const noexcept { return changedByUserId_; }
    const std::string& getChangedAt() const noexcept { return changedAt_; }
    const std::string& getRemarks() const noexcept { return remarks_; }

    // Setters
    void setId(int id) noexcept { id_ = id; }
};

} // namespace CivicCare
