#pragma once

#include <unordered_map>
#include <algorithm>
#include "interfaces/IComplaintRepository.hpp"
#include "utils/Optional.hpp"

namespace CivicCare {

/**
 * @brief In-memory mock repository for isolated testing of Member 3's services.
 */
class MockComplaintRepository : public IComplaintRepository {
private:
    int nextComplaintId_ = 1;
    int nextAssignmentId_ = 1;
    int nextHistoryId_ = 1;

    std::unordered_map<int, Complaint> complaints_;
    std::vector<ComplaintHistory> historyRecords_;
    std::vector<Assignment> assignments_;

public:
    int save(const Complaint& complaint) override {
        Complaint copy = complaint;
        int id = nextComplaintId_++;
        copy.setId(id);
        complaints_[id] = copy;
        return id;
    }

    bool update(const Complaint& complaint) override {
        if (complaints_.find(complaint.getId()) == complaints_.end()) {
            return false;
        }
        complaints_[complaint.getId()] = complaint;
        return true;
    }

    Optional<Complaint> findById(int id) override {
        auto it = complaints_.find(id);
        if (it != complaints_.end()) {
            return it->second;
        }
        return NullOpt;
    }

    std::vector<Complaint> findAll() override {
        std::vector<Complaint> list;
        for (const auto& pair : complaints_) {
            list.push_back(pair.second);
        }
        return list;
    }

    std::vector<Complaint> findByCitizenId(int citizenId) override {
        std::vector<Complaint> list;
        for (const auto& pair : complaints_) {
            if (pair.second.getCitizenId() == citizenId) {
                list.push_back(pair.second);
            }
        }
        return list;
    }

    std::vector<Complaint> findByDepartmentId(int departmentId) override {
        std::vector<Complaint> list;
        for (const auto& pair : complaints_) {
            if (pair.second.getDepartmentId().has_value() &&
                pair.second.getDepartmentId().value() == departmentId) {
                list.push_back(pair.second);
            }
        }
        return list;
    }

    void recordHistory(const ComplaintHistory& history) override {
        ComplaintHistory copy = history;
        copy.setId(nextHistoryId_++);
        historyRecords_.push_back(copy);
    }

    std::vector<ComplaintHistory> findHistoryByComplaintId(int complaintId) override {
        std::vector<ComplaintHistory> results;
        for (const auto& h : historyRecords_) {
            if (h.getComplaintId() == complaintId) {
                results.push_back(h);
            }
        }
        return results;
    }

    void saveAssignment(const Assignment& assignment) override {
        Assignment copy = assignment;
        copy.setId(nextAssignmentId_++);
        // Mark previous assignments for this complaint as not current
        for (auto& a : assignments_) {
            if (a.getComplaintId() == assignment.getComplaintId()) {
                a.setIsCurrent(false);
            }
        }
        assignments_.push_back(copy);
    }

    Optional<Assignment> findCurrentAssignment(int complaintId) override {
        for (auto it = assignments_.rbegin(); it != assignments_.rend(); ++it) {
            if (it->getComplaintId() == complaintId && it->isCurrent()) {
                return *it;
            }
        }
        return NullOpt;
    }
};

} // namespace CivicCare
