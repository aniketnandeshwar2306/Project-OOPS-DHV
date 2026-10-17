#pragma once

#include <vector>
#include "models/Complaint.hpp"
#include "models/Assignment.hpp"
#include "models/ComplaintHistory.hpp"
#include "utils/Optional.hpp"

namespace CivicCare {

/**
 * @brief Persistence Contract for Complaints, Assignments, and Audit Trails (Provided by Member 4).
 * Decouples domain logic from SQLite or file systems.
 */
class IComplaintRepository {
public:
    virtual ~IComplaintRepository() = default;

    /**
     * @brief Persist a newly lodged complaint.
     * @param complaint The complaint object to save.
     * @return Generated primary key ID.
     */
    virtual int save(const Complaint& complaint) = 0;

    /**
     * @brief Update an existing complaint in storage.
     * @param complaint The complaint with updated fields.
     * @return true if updated successfully, false otherwise.
     */
    virtual bool update(const Complaint& complaint) = 0;

    /**
     * @brief Look up a complaint by its ID.
     */
    virtual Optional<Complaint> findById(int id) = 0;

    /**
     * @brief Retrieve all complaints.
     */
    virtual std::vector<Complaint> findAll() = 0;

    /**
     * @brief Retrieve complaints lodged by a specific citizen.
     */
    virtual std::vector<Complaint> findByCitizenId(int citizenId) = 0;

    /**
     * @brief Retrieve complaints assigned to a specific department.
     */
    virtual std::vector<Complaint> findByDepartmentId(int departmentId) = 0;

    /**
     * @brief Record an audit log entry for a status change.
     */
    virtual void recordHistory(const ComplaintHistory& history) = 0;

    /**
     * @brief Retrieve complete status change audit trail for a complaint.
     */
    virtual std::vector<ComplaintHistory> findHistoryByComplaintId(int complaintId) = 0;

    /**
     * @brief Save or update a staff assignment record.
     */
    virtual void saveAssignment(const Assignment& assignment) = 0;

    /**
     * @brief Find the current active assignment for a complaint.
     */
    virtual Optional<Assignment> findCurrentAssignment(int complaintId) = 0;
};

} // namespace CivicCare
