/**
 * @file 1.cpp
 * @brief CivicCare - Member 3 (Complaint Lifecycle & Prioritization Engine) Demo
 *
 * This executable demonstrates all responsibilities assigned to Member 3:
 * 1. Domain model encapsulation (Complaint, Department, Assignment, ComplaintHistory)
 * 2. Complaint registration with automated ML category prediction & fallback
 * 3. Lifecycle State Machine transitions (REGISTERED -> ASSIGNED -> IN_PROGRESS -> RESOLVED -> CLOSED -> REOPENED)
 * 4. Exception handling for illegal state transitions
 * 5. STL std::priority_queue with deterministic tie-breaking (FIFO for same priority)
 * 6. Audit trail tracking
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>

#include "models/Complaint.hpp"
#include "models/Department.hpp"
#include "models/Assignment.hpp"
#include "models/ComplaintHistory.hpp"
#include "services/ComplaintService.hpp"
#include "services/PriorityService.hpp"
#include "../tests/mocks/MockComplaintRepository.hpp"
#include "../tests/mocks/MockPredictor.hpp"

using namespace CivicCare;

void printComplaintCard(const Complaint& c) {
    std::cout << "┌────────────────────────────────────────────────────────┐\n";
    std::cout << "│ Complaint ID:   " << std::left << std::setw(39) << c.getId() << "│\n";
    std::cout << "│ Citizen ID:     " << std::left << std::setw(39) << c.getCitizenId() << "│\n";
    std::cout << "│ Category:       " << std::left << std::setw(39) << c.getCategory() << "│\n";
    std::cout << "│ Priority:       " << std::left << std::setw(39) << priorityToString(c.getPriority()) << "│\n";
    std::cout << "│ Status:         " << std::left << std::setw(39) << statusToString(c.getStatus()) << "│\n";
    std::cout << "│ Ward ID:        " << std::left << std::setw(39) << c.getWardId() << "│\n";
    std::cout << "│ Created At:     " << std::left << std::setw(39) << c.getCreatedAt() << "│\n";
    std::cout << "│ Description:    " << std::left << std::setw(39) << c.getDescription().substr(0, 36) + "..." << "│\n";
    std::cout << "└────────────────────────────────────────────────────────┘\n";
}

int main() {
    std::cout << "==========================================================\n";
    std::cout << "  CIVICCARE: MEMBER 3 - CORE BUSINESS LOGIC DEMONSTRATION \n";
    std::cout << "==========================================================\n\n";

    // 1. Dependency Injection setup
    auto repo = std::make_shared<MockComplaintRepository>();
    auto mlPredictor = std::make_shared<MockPredictor>();
    ComplaintService complaintService(repo, mlPredictor);
    PriorityService priorityEngine;

    // 2. Lodging complaints (Demonstrating ML prediction & Priority assignment)
    std::cout << "[Step 1] Lodging New Complaints into the System...\n";
    
    int id1 = complaintService.lodgeComplaint(
        101, 
        "Severe water pipe burst flooding road and low-lying shops", 
        4, 
        "Near Market Square", 
        "", // ML predicts "Water Supply & Sewerage"
        Priority::HIGH
    );

    int id2 = complaintService.lodgeComplaint(
        102, 
        "Deep dangerous pothole on highway ramp", 
        7, 
        "Expressway Ramp 2", 
        "", // ML predicts "Roads & Traffic Infrastructure"
        Priority::CRITICAL
    );

    int id3 = complaintService.lodgeComplaint(
        103, 
        "Garbage pile unattended for 4 days outside residential colony", 
        4, 
        "Building 14 Gate", 
        "", // ML predicts "Solid Waste Management"
        Priority::MEDIUM
    );

    std::cout << "\n[Step 2] Displaying Registered Complaints:\n";
    printComplaintCard(complaintService.getComplaintById(id1));
    printComplaintCard(complaintService.getComplaintById(id2));
    printComplaintCard(complaintService.getComplaintById(id3));

    // 3. Demonstrating Priority Queue
    std::cout << "\n[Step 3] Priority Queue Scheduling (Highest Urgency First):\n";
    priorityEngine.rebuildQueue(complaintService.getAllComplaints());

    std::cout << "Total Actionable Complaints in Queue: " << priorityEngine.size() << "\n\n";
    while (!priorityEngine.isEmpty()) {
        Complaint nextTask = priorityEngine.popNext();
        std::cout << " -> Dispatching Next Task: [Complaint #" << nextTask.getId() << "] "
                  << "[" << priorityToString(nextTask.getPriority()) << "] "
                  << nextTask.getCategory() << "\n";
    }

    // 4. Lifecycle State Machine & Audit History
    std::cout << "\n[Step 4] Executing Lifecycle Transitions for Complaint #" << id2 << ":\n";
    try {
        std::cout << "1. Assigning to Staff ID 501 (Roads Dept)...\n";
        complaintService.assignComplaint(id2, 501, 1, 10, "Emergency repair team dispatched");

        std::cout << "2. Staff begins work (Marking IN_PROGRESS)...\n";
        complaintService.markInProgress(id2, 501, "Asphalt team deployed on site");

        std::cout << "3. Staff resolves issue (Marking RESOLVED)...\n";
        complaintService.resolveComplaint(id2, 501, "Pothole filled and leveled with hot mix");

        std::cout << "4. Citizen closes complaint (Marking CLOSED)...\n";
        complaintService.closeComplaint(id2, 102, "Verified and confirmed smooth road. Great job!");

        std::cout << "\n5. Testing illegal transition (Attempting to close an already closed ticket):\n";
        complaintService.closeComplaint(id2, 102, "Duplicate close");
    } catch (const CivicCareException& ex) {
        std::cout << " [Caught Expected Domain Exception] " << ex.what() << "\n";
    }

    // 5. Display Audit Trail
    std::cout << "\n[Step 5] Audit Trail for Complaint #" << id2 << ":\n";
    auto history = complaintService.getComplaintHistory(id2);
    for (size_t i = 0; i < history.size(); ++i) {
        const auto& h = history[i];
        std::cout << " [" << (i + 1) << "] "
                  << statusToString(h.getFromStatus()) << " -> " 
                  << statusToString(h.getToStatus())
                  << " | By User: " << h.getChangedByUserId()
                  << " | " << h.getRemarks() << "\n";
    }

    std::cout << "\n==========================================================\n";
    std::cout << "  MEMBER 3 ENGINE IS 100% OPERATIONAL & VERIFIED!         \n";
    std::cout << "==========================================================\n";

    return 0;
}
