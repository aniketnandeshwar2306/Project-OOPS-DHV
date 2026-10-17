#pragma once

#include <queue>
#include <vector>
#include <functional>
#include "models/Complaint.hpp"
#include "exceptions/ComplaintExceptions.hpp"
#include "utils/Optional.hpp"

namespace CivicCare {

/**
 * @brief Custom comparator for std::priority_queue.
 * Ordering rules:
 * 1. Higher Priority enum value comes first (CRITICAL > HIGH > MEDIUM > LOW).
 * 2. Deterministic Tie-Breaker: Earlier creation timestamp (createdAt) comes first (FIFO for same priority).
 */
struct ComplaintPriorityComparator {
    bool operator()(const Complaint& a, const Complaint& b) const noexcept {
        // Higher numeric priority value means more urgent
        int pA = static_cast<int>(a.getPriority());
        int pB = static_cast<int>(b.getPriority());

        if (pA != pB) {
            return pA < pB; // 'b' is higher priority, so 'b' comes first
        }

        // Tie-breaker: Earlier ISO timestamp comes first.
        // In max-heap, a is "less" if its creation date is later (newer).
        return a.getCreatedAt() > b.getCreatedAt();
    }
};

/**
 * @brief Engine that prioritizes and dispatches complaints for municipal response.
 * Uses an STL priority_queue with deterministic tie-breaking.
 */
class PriorityService {
private:
    std::priority_queue<Complaint, std::vector<Complaint>, ComplaintPriorityComparator> queue_;

public:
    PriorityService() = default;

    /**
     * @brief Insert a complaint into the priority queue.
     * Only pending/actionable complaints (REGISTERED, ASSIGNED, REOPENED) are typically queued.
     */
    void enqueue(const Complaint& complaint);

    /**
     * @brief Peek at the most urgent complaint without removing it.
     * @throws CivicCareException if the queue is empty.
     */
    Complaint peekNext() const;

    /**
     * @brief Extract and remove the most urgent complaint.
     * @throws CivicCareException if the queue is empty.
     */
    Complaint popNext();

    /**
     * @brief Rebuild the priority queue from a list of complaints (e.g., loaded from database).
     * Filters for active complaints (excluding RESOLVED and CLOSED).
     * @param complaints The full list of complaints from repository.
     * @param departmentId Optional filter to queue only for a specific department (or 0 for all).
     */
    void rebuildQueue(const std::vector<Complaint>& complaints, int departmentId = 0);

    /**
     * @brief Get all pending complaints in sorted priority order without emptying the internal queue.
     */
    std::vector<Complaint> getSortedPendingComplaints() const;

    /**
     * @brief Check if the queue is empty.
     */
    bool isEmpty() const noexcept { return queue_.empty(); }

    /**
     * @brief Get the count of complaints waiting in the queue.
     */
    size_t size() const noexcept { return queue_.size(); }

    /**
     * @brief Clear all entries in the queue.
     */
    void clear() noexcept;
};

} // namespace CivicCare
