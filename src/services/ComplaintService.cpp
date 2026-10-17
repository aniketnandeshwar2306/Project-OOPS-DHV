#include "services/ComplaintService.hpp"
#include <iostream>

namespace CivicCare {

ComplaintService::ComplaintService(std::shared_ptr<IComplaintRepository> repository,
                                   std::shared_ptr<IPredictor> predictor)
    : repository_(std::move(repository)),
      predictor_(std::move(predictor)) {
    if (!repository_) {
        throw CivicCareException("ComplaintService requires a valid non-null IComplaintRepository.");
    }
}

void ComplaintService::setPredictor(std::shared_ptr<IPredictor> predictor) noexcept {
    predictor_ = std::move(predictor);
}

bool ComplaintService::isValidTransition(ComplaintStatus from, ComplaintStatus to) const noexcept {
    switch (from) {
        case ComplaintStatus::REGISTERED:
            return (to == ComplaintStatus::ASSIGNED);

        case ComplaintStatus::ASSIGNED:
            return (to == ComplaintStatus::IN_PROGRESS || to == ComplaintStatus::ASSIGNED);

        case ComplaintStatus::IN_PROGRESS:
            return (to == ComplaintStatus::RESOLVED);

        case ComplaintStatus::RESOLVED:
            return (to == ComplaintStatus::CLOSED || to == ComplaintStatus::REOPENED);

        case ComplaintStatus::CLOSED:
            return (to == ComplaintStatus::REOPENED);

        case ComplaintStatus::REOPENED:
            return (to == ComplaintStatus::ASSIGNED);

        default:
            return false;
    }
}

Optional<int> ComplaintService::findActiveDuplicate(int citizenId, int wardId, const std::string& description) const {
    auto citizenComplaints = repository_->findByCitizenId(citizenId);
    for (const auto& c : citizenComplaints) {
        // If an active complaint has the exact same description and ward
        if (c.getWardId() == wardId && c.getDescription() == description) {
            if (c.getStatus() != ComplaintStatus::CLOSED && c.getStatus() != ComplaintStatus::RESOLVED) {
                return c.getId();
            }
        }
    }
    return NullOpt;
}

int ComplaintService::lodgeComplaint(int citizenId,
                                     const std::string& description,
                                     int wardId,
                                     const std::string& locationDetails,
                                     const std::string& preferredCategory,
                                     Priority initialPriority) {
    // Check for duplicate submission to prevent accidental double-reporting
    auto existingDuplicate = findActiveDuplicate(citizenId, wardId, description);
    if (existingDuplicate.has_value()) {
        throw InvalidComplaintDataException(
            "Duplicate complaint detected: An active complaint (#" + 
            std::to_string(existingDuplicate.value()) + 
            ") with identical description is already pending in this ward."
        );
    }

    std::string finalCategory = preferredCategory;

    // If citizen did not specify a category and an ML predictor is available, request prediction
    if (finalCategory.empty()) {
        if (predictor_) {
            try {
                Prediction pred = predictor_->predict(description);
                if (!pred.category.empty() && pred.confidence >= 0.20) {
                    finalCategory = pred.category;
                } else {
                    finalCategory = "Unclassified";
                }
            } catch (const std::exception& ex) {
                // Graceful degradation: never fail registration due to ML service failure
                std::cerr << "[CivicCare Warning] ML prediction failed: " << ex.what() 
                          << ". Falling back to 'Unclassified'." << std::endl;
                finalCategory = "Unclassified";
            }
        } else {
            finalCategory = "Unclassified";
        }
    }

    // Construct and validate complaint
    Complaint complaint(citizenId, description, finalCategory, wardId, initialPriority, locationDetails);

    // Save into persistence layer
    int generatedId = repository_->save(complaint);
    complaint.setId(generatedId);

    // Record initial history entry
    ComplaintHistory initialHistory(
        generatedId,
        ComplaintStatus::REGISTERED,
        ComplaintStatus::REGISTERED,
        citizenId,
        "Complaint lodged by citizen."
    );
    repository_->recordHistory(initialHistory);

    return generatedId;
}

void ComplaintService::assignComplaint(int complaintId,
                                       int staffId,
                                       int departmentId,
                                       int assignedByUserId,
                                       const std::string& notes) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    ComplaintStatus oldStatus = complaint.getStatus();
    ComplaintStatus newStatus = ComplaintStatus::ASSIGNED;

    // Check transition validity
    if (!isValidTransition(oldStatus, newStatus)) {
        throw InvalidStatusTransitionException(
            statusToString(oldStatus),
            statusToString(newStatus),
            "Complaint must be in REGISTERED or REOPENED status to be assigned."
        );
    }

    // Update assignment details
    complaint.assignStaff(staffId, departmentId);
    complaint.setStatus(newStatus);
    repository_->update(complaint);

    // Save assignment record
    Assignment assignment(complaintId, staffId, departmentId, assignedByUserId, notes);
    repository_->saveAssignment(assignment);

    // Record audit history
    ComplaintHistory history(
        complaintId,
        oldStatus,
        newStatus,
        assignedByUserId,
        "Assigned to Staff ID " + std::to_string(staffId) + " (Dept " + std::to_string(departmentId) + "). Notes: " + notes
    );
    repository_->recordHistory(history);
}

void ComplaintService::markInProgress(int complaintId,
                                      int staffId,
                                      const std::string& remarks) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    ComplaintStatus oldStatus = complaint.getStatus();
    ComplaintStatus newStatus = ComplaintStatus::IN_PROGRESS;

    if (!isValidTransition(oldStatus, newStatus)) {
        throw InvalidStatusTransitionException(
            statusToString(oldStatus),
            statusToString(newStatus),
            "Complaint must be ASSIGNED before marking IN_PROGRESS."
        );
    }

    complaint.setStatus(newStatus);
    repository_->update(complaint);

    ComplaintHistory history(
        complaintId,
        oldStatus,
        newStatus,
        staffId,
        remarks.empty() ? "Staff commenced work." : remarks
    );
    repository_->recordHistory(history);
}

void ComplaintService::resolveComplaint(int complaintId,
                                        int staffId,
                                        const std::string& resolutionDetails) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    ComplaintStatus oldStatus = complaint.getStatus();
    ComplaintStatus newStatus = ComplaintStatus::RESOLVED;

    if (!isValidTransition(oldStatus, newStatus)) {
        throw InvalidStatusTransitionException(
            statusToString(oldStatus),
            statusToString(newStatus),
            "Complaint must be IN_PROGRESS before marking RESOLVED."
        );
    }

    complaint.setStatus(newStatus);
    repository_->update(complaint);

    ComplaintHistory history(
        complaintId,
        oldStatus,
        newStatus,
        staffId,
        "Resolution reported: " + resolutionDetails
    );
    repository_->recordHistory(history);
}

void ComplaintService::closeComplaint(int complaintId,
                                      int userId,
                                      const std::string& feedback) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    ComplaintStatus oldStatus = complaint.getStatus();
    ComplaintStatus newStatus = ComplaintStatus::CLOSED;

    if (!isValidTransition(oldStatus, newStatus)) {
        throw InvalidStatusTransitionException(
            statusToString(oldStatus),
            statusToString(newStatus),
            "Only RESOLVED complaints can be officially CLOSED."
        );
    }

    complaint.setStatus(newStatus);
    repository_->update(complaint);

    ComplaintHistory history(
        complaintId,
        oldStatus,
        newStatus,
        userId,
        "Complaint closed. Feedback: " + feedback
    );
    repository_->recordHistory(history);
}

void ComplaintService::reopenComplaint(int complaintId,
                                       int userId,
                                       const std::string& reason) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    ComplaintStatus oldStatus = complaint.getStatus();
    ComplaintStatus newStatus = ComplaintStatus::REOPENED;

    if (!isValidTransition(oldStatus, newStatus)) {
        throw InvalidStatusTransitionException(
            statusToString(oldStatus),
            statusToString(newStatus),
            "Only RESOLVED or CLOSED complaints may be REOPENED."
        );
    }

    complaint.setStatus(newStatus);
    repository_->update(complaint);

    ComplaintHistory history(
        complaintId,
        oldStatus,
        newStatus,
        userId,
        "Complaint reopened by user. Reason: " + reason
    );
    repository_->recordHistory(history);
}

void ComplaintService::updateCategory(int complaintId,
                                      const std::string& newCategory,
                                      int staffId) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    std::string oldCategory = complaint.getCategory();
    complaint.setCategory(newCategory);
    repository_->update(complaint);

    ComplaintHistory history(
        complaintId,
        complaint.getStatus(),
        complaint.getStatus(),
        staffId,
        "Category adjusted from '" + oldCategory + "' to '" + newCategory + "'."
    );
    repository_->recordHistory(history);
}

void ComplaintService::updatePriority(int complaintId,
                                      Priority newPriority,
                                      int staffId) {
    auto optComplaint = repository_->findById(complaintId);
    if (!optComplaint.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }

    Complaint complaint = optComplaint.value();
    Priority oldPriority = complaint.getPriority();
    complaint.setPriority(newPriority);
    repository_->update(complaint);

    ComplaintHistory history(
        complaintId,
        complaint.getStatus(),
        complaint.getStatus(),
        staffId,
        "Priority adjusted from '" + priorityToString(oldPriority) + "' to '" + priorityToString(newPriority) + "'."
    );
    repository_->recordHistory(history);
}

// --- Query Implementations ---

Complaint ComplaintService::getComplaintById(int complaintId) const {
    auto opt = repository_->findById(complaintId);
    if (!opt.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }
    return opt.value();
}

std::vector<Complaint> ComplaintService::getComplaintsByCitizen(int citizenId) const {
    return repository_->findByCitizenId(citizenId);
}

std::vector<Complaint> ComplaintService::getComplaintsByDepartment(int departmentId) const {
    return repository_->findByDepartmentId(departmentId);
}

std::vector<Complaint> ComplaintService::getAllComplaints() const {
    return repository_->findAll();
}

std::vector<ComplaintHistory> ComplaintService::getComplaintHistory(int complaintId) const {
    // Check existence first
    auto opt = repository_->findById(complaintId);
    if (!opt.has_value()) {
        throw ComplaintNotFoundException(complaintId);
    }
    return repository_->findHistoryByComplaintId(complaintId);
}

Optional<Assignment> ComplaintService::getCurrentAssignment(int complaintId) const {
    return repository_->findCurrentAssignment(complaintId);
}

} // namespace CivicCare
