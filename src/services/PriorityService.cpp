#include "services/PriorityService.hpp"

namespace CivicCare {

void PriorityService::enqueue(const Complaint& complaint) {
    queue_.push(complaint);
}

Complaint PriorityService::peekNext() const {
    if (queue_.empty()) {
        throw CivicCareException("Cannot peek: Priority queue is empty.");
    }
    return queue_.top();
}

Complaint PriorityService::popNext() {
    if (queue_.empty()) {
        throw CivicCareException("Cannot pop: Priority queue is empty.");
    }
    Complaint topComplaint = queue_.top();
    queue_.pop();
    return topComplaint;
}

void PriorityService::rebuildQueue(const std::vector<Complaint>& complaints, int departmentId) {
    clear();

    for (const auto& c : complaints) {
        // Only active, unresolved complaints enter the priority dispatch queue
        if (c.getStatus() == ComplaintStatus::RESOLVED || c.getStatus() == ComplaintStatus::CLOSED) {
            continue;
        }

        // Apply department filter if requested
        if (departmentId > 0) {
            if (!c.getDepartmentId().has_value() || c.getDepartmentId().value() != departmentId) {
                continue;
            }
        }

        queue_.push(c);
    }
}

std::vector<Complaint> PriorityService::getSortedPendingComplaints() const {
    // Copy the priority queue to extract elements in sorted order without modifying the original
    auto tempQueue = queue_;
    std::vector<Complaint> sortedList;
    sortedList.reserve(tempQueue.size());

    while (!tempQueue.empty()) {
        sortedList.push_back(tempQueue.top());
        tempQueue.pop();
    }

    return sortedList;
}

void PriorityService::clear() noexcept {
    queue_ = std::priority_queue<Complaint, std::vector<Complaint>, ComplaintPriorityComparator>();
}

} // namespace CivicCare
