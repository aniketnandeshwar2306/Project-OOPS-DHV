#pragma once

#include <memory>
#include <string>
#include <vector>
#include "models/Complaint.hpp"
#include "models/Assignment.hpp"
#include "models/ComplaintHistory.hpp"
#include "interfaces/IComplaintRepository.hpp"
#include "interfaces/IPredictor.hpp"
#include "exceptions/ComplaintExceptions.hpp"
#include "utils/Optional.hpp"

namespace CivicCare {

/**
 * @brief Core engine service managing complaint lifecycles, validation,
 * assignments, and ML prediction integration.
 */
class ComplaintService {
private:
    std::shared_ptr<IComplaintRepository> repository_;
    std::shared_ptr<IPredictor> predictor_; // Optional ML predictor

    // Validates whether transitioning from 'from' to 'to' is permitted by business rules
    bool isValidTransition(ComplaintStatus from, ComplaintStatus to) const noexcept;

public:
    /**
     * @brief Constructor with dependency injection.
     * @param repository Persistence repository instance (Contract with Member 4).
     * @param predictor ML prediction service instance (Contract with Member 1). Can be nullptr.
     */
    ComplaintService(std::shared_ptr<IComplaintRepository> repository,
                     std::shared_ptr<IPredictor> predictor = nullptr);

    /**
     * @brief Set or update the ML predictor service.
     */
    void setPredictor(std::shared_ptr<IPredictor> predictor) noexcept;

    // --- Core Operations ---

    /**
     * @brief Register a new complaint lodged by a citizen.
     * @param citizenId Authenticated citizen ID.
     * @param description Full textual description of the civic issue.
     * @param wardId Municipality ward identifier.
     * @param locationDetails Optional landmark or GPS coordinates.
     * @param preferredCategory Optional category manually specified by user. If empty, ML is invoked.
     * @param initialPriority Optional initial priority. Defaults to MEDIUM if not specified.
     * @return Newly assigned unique complaint ID.
     */
    int lodgeComplaint(int citizenId,
                       const std::string& description,
                       int wardId,
                       const std::string& locationDetails = "",
                       const std::string& preferredCategory = "",
                       Priority initialPriority = Priority::MEDIUM);

    /**
     * @brief Check if a citizen has already submitted an identical active complaint in the same ward.
     * Prevents accidental duplicate submissions.
     * @return Optional containing existing complaint ID if duplicate found, or NullOpt.
     */
    Optional<int> findActiveDuplicate(int citizenId, int wardId, const std::string& description) const;

    /**
     * @brief Assign a complaint to a department and specific staff member.
     * Transitions complaint status to ASSIGNED.
     */
    void assignComplaint(int complaintId,
                         int staffId,
                         int departmentId,
                         int assignedByUserId,
                         const std::string& notes = "");

    /**
     * @brief Staff member marks complaint as under active investigation/repair.
     * Transitions status from ASSIGNED to IN_PROGRESS.
     */
    void markInProgress(int complaintId,
                        int staffId,
                        const std::string& remarks = "Staff began work on the complaint.");

    /**
     * @brief Staff member marks complaint as resolved.
     * Transitions status from IN_PROGRESS to RESOLVED.
     */
    void resolveComplaint(int complaintId,
                          int staffId,
                          const std::string& resolutionDetails);

    /**
     * @brief Citizen or Admin accepts resolution and closes the complaint ticket.
     * Transitions status from RESOLVED to CLOSED.
     */
    void closeComplaint(int complaintId,
                        int userId,
                        const std::string& feedback = "Citizen confirmed resolution.");

    /**
     * @brief Citizen or Admin reopens an unresolved or unsatisfactory complaint.
     * Transitions status from RESOLVED or CLOSED to REOPENED.
     */
    void reopenComplaint(int complaintId,
                         int userId,
                         const std::string& reason);

    /**
     * @brief Staff or Administrator corrects/updates the complaint category.
     */
    void updateCategory(int complaintId,
                        const std::string& newCategory,
                        int staffId);

    /**
     * @brief Update the complaint priority.
     */
    void updatePriority(int complaintId,
                        Priority newPriority,
                        int staffId);

    // --- Query Operations ---

    Complaint getComplaintById(int complaintId) const;
    std::vector<Complaint> getComplaintsByCitizen(int citizenId) const;
    std::vector<Complaint> getComplaintsByDepartment(int departmentId) const;
    std::vector<Complaint> getAllComplaints() const;
    std::vector<ComplaintHistory> getComplaintHistory(int complaintId) const;
    Optional<Assignment> getCurrentAssignment(int complaintId) const;
};

} // namespace CivicCare
