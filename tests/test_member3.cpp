#include <iostream>
#include <cassert>
#include <memory>

#include "models/Complaint.hpp"
#include "models/Department.hpp"
#include "models/Assignment.hpp"
#include "models/ComplaintHistory.hpp"
#include "services/ComplaintService.hpp"
#include "services/PriorityService.hpp"
#include "mocks/MockComplaintRepository.hpp"
#include "mocks/MockPredictor.hpp"

using namespace CivicCare;

// Helper assertion macro with detailed logging
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] TEST FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            std::exit(1); \
        } else { \
            std::cout << "[+] PASSED: " << msg << "\n"; \
        } \
    } while (0)

void testComplaintValidation() {
    std::cout << "\n=== Running Test: Complaint Model Validation ===\n";

    // Valid complaint should construct fine
    try {
        Complaint c(101, "Severe pipeline leakage causing road waterlogging", "Water Supply", 5, Priority::HIGH);
        TEST_ASSERT(c.getCitizenId() == 101, "Citizen ID is correctly initialized");
        TEST_ASSERT(c.getPriority() == Priority::HIGH, "Priority is HIGH");
        TEST_ASSERT(c.getStatus() == ComplaintStatus::REGISTERED, "Initial status is REGISTERED");
    } catch (...) {
        TEST_ASSERT(false, "Valid complaint threw unexpected exception");
    }

    // Invalid citizenId (0 or negative)
    try {
        Complaint c(-1, "Valid description text here", "Water", 1);
        TEST_ASSERT(false, "Failed to throw on invalid citizenId");
    } catch (const InvalidComplaintDataException& e) {
        TEST_ASSERT(true, "Properly rejected invalid citizenId");
    }

    // Invalid short description
    try {
        Complaint c(101, "bad", "Water", 1);
        TEST_ASSERT(false, "Failed to throw on short description");
    } catch (const InvalidComplaintDataException& e) {
        TEST_ASSERT(true, "Properly rejected short description");
    }

    // Invalid wardId
    try {
        Complaint c(101, "Valid description text here", "Water", 0);
        TEST_ASSERT(false, "Failed to throw on invalid wardId");
    } catch (const InvalidComplaintDataException& e) {
        TEST_ASSERT(true, "Properly rejected invalid wardId");
    }
}

void testRegistrationAndMLIntegration() {
    std::cout << "\n=== Running Test: Registration & ML Fallback ===\n";

    auto repo = std::make_shared<MockComplaintRepository>();
    auto predictor = std::make_shared<MockPredictor>();
    ComplaintService service(repo, predictor);

    // Case 1: ML Predicts correctly
    int c1Id = service.lodgeComplaint(
        201,
        "Deep pothole on SV road near Milan subway",
        3, // ward 3
        "Near Milan Subway Pillar 4",
        "", // no manual category -> ML should be used
        Priority::MEDIUM
    );
    Complaint c1 = service.getComplaintById(c1Id);
    TEST_ASSERT(c1.getCategory() == "Roads & Traffic Infrastructure", "ML predicted 'Roads & Traffic Infrastructure'");

    // Case 2: Citizen supplies explicit manual category -> ML should NOT override it
    int c2Id = service.lodgeComplaint(
        202,
        "Garbage pile accumulating on sidewalk",
        4,
        "Opposite municipal school",
        "Public Health Inspection",
        Priority::LOW
    );
    Complaint c2 = service.getComplaintById(c2Id);
    TEST_ASSERT(c2.getCategory() == "Public Health Inspection", "Explicit manual category respected");

    // Case 3: ML fails / crashes -> Graceful fallback to 'Unclassified'
    predictor->setShouldFail(true);
    int c3Id = service.lodgeComplaint(
        203,
        "Water supply disrupted for past 48 hours in building 12",
        2
    );
    Complaint c3 = service.getComplaintById(c3Id);
    TEST_ASSERT(c3.getCategory() == "Unclassified", "Graceful fallback to 'Unclassified' on ML failure");

    // Case 4: Duplicate submission prevention
    try {
        service.lodgeComplaint(
            201, // same citizen
            "Deep pothole on SV road near Milan subway", // identical text
            3 // same ward
        );
        TEST_ASSERT(false, "Allowed duplicate complaint submission");
    } catch (const InvalidComplaintDataException& e) {
        TEST_ASSERT(true, "Duplicate complaint submission successfully prevented");
    }
}

void testComplaintLifecycleAndStateTransitions() {
    std::cout << "\n=== Running Test: Lifecycle State Transitions ===\n";

    auto repo = std::make_shared<MockComplaintRepository>();
    ComplaintService service(repo);

    int cid = service.lodgeComplaint(301, "Fallen tree blocking arterial road", 7, "Sector 4", "Disaster Management", Priority::CRITICAL);
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::REGISTERED, "Status is REGISTERED");

    // Test 1: Illegal transition attempt (REGISTERED -> IN_PROGRESS or CLOSED directly)
    try {
        service.markInProgress(cid, 501);
        TEST_ASSERT(false, "Allowed illegal transition from REGISTERED to IN_PROGRESS");
    } catch (const InvalidStatusTransitionException& e) {
        TEST_ASSERT(true, "Rejected illegal transition from REGISTERED to IN_PROGRESS");
    }

    try {
        service.closeComplaint(cid, 301, "Closed prematurely");
        TEST_ASSERT(false, "Allowed illegal transition from REGISTERED to CLOSED");
    } catch (const InvalidStatusTransitionException& e) {
        TEST_ASSERT(true, "Rejected illegal transition from REGISTERED to CLOSED");
    }

    // Test 2: Valid assignment (REGISTERED -> ASSIGNED)
    service.assignComplaint(cid, 501, 10, 1, "Dispatched emergency tree clearing crew");
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::ASSIGNED, "Status transitioned to ASSIGNED");
    TEST_ASSERT(service.getComplaintById(cid).getAssignedStaffId().value() == 501, "Assigned to staff 501");
    TEST_ASSERT(service.getComplaintById(cid).getDepartmentId().value() == 10, "Assigned to department 10");

    // Test 3: Valid progress (ASSIGNED -> IN_PROGRESS)
    service.markInProgress(cid, 501, "Crew reached location, chainsaw operation underway");
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::IN_PROGRESS, "Status transitioned to IN_PROGRESS");

    // Test 4: Valid resolution (IN_PROGRESS -> RESOLVED)
    service.resolveComplaint(cid, 501, "Tree cleared and wood transported to depot. Road open.");
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::RESOLVED, "Status transitioned to RESOLVED");

    // Test 5: Valid closure (RESOLVED -> CLOSED)
    service.closeComplaint(cid, 301, "Road is clear. Thank you for prompt action.");
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::CLOSED, "Status transitioned to CLOSED");

    // Test 6: Reopening a complaint (CLOSED -> REOPENED)
    service.reopenComplaint(cid, 301, "Tree branches still obstructing bicycle lane.");
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::REOPENED, "Status transitioned to REOPENED");

    // Test 7: Re-assignment (REOPENED -> ASSIGNED)
    service.assignComplaint(cid, 502, 10, 1, "Assigned secondary team for brush cleanup");
    TEST_ASSERT(service.getComplaintById(cid).getStatus() == ComplaintStatus::ASSIGNED, "Reopened complaint transitioned to ASSIGNED");

    // Test 8: Verify complete audit history
    auto history = service.getComplaintHistory(cid);
    std::cout << "[i] Total status changes recorded: " << history.size() << "\n";
    TEST_ASSERT(history.size() >= 7, "All lifecycle changes logged in audit history");
}

void testPriorityQueueAndTieBreaker() {
    std::cout << "\n=== Running Test: Priority Queue & Tie-Breaker ===\n";

    PriorityService pqService;

    // Create complaints with explicit timestamps to test deterministic tie-breaker
    Complaint cLow(1, 101, "Low urgency street paint faded", "Roads", 1, Priority::LOW, ComplaintStatus::REGISTERED, "2026-10-17T09:00:00Z", "2026-10-17T09:00:00Z");
    Complaint cMed1(2, 102, "Medium streetlight flickering", "Electricity", 1, Priority::MEDIUM, ComplaintStatus::REGISTERED, "2026-10-17T10:00:00Z", "2026-10-17T10:00:00Z");
    Complaint cMed2(3, 103, "Medium park bench broken", "Gardens", 1, Priority::MEDIUM, ComplaintStatus::REGISTERED, "2026-10-17T10:30:00Z", "2026-10-17T10:30:00Z");
    Complaint cHigh(4, 104, "High water contamination reported", "Water", 1, Priority::HIGH, ComplaintStatus::REGISTERED, "2026-10-17T08:00:00Z", "2026-10-17T08:00:00Z");
    Complaint cCrit(5, 105, "Critical gas leak odor near school", "Disaster", 1, Priority::CRITICAL, ComplaintStatus::REGISTERED, "2026-10-17T11:00:00Z", "2026-10-17T11:00:00Z");

    // Enqueue in arbitrary order
    pqService.enqueue(cMed2);
    pqService.enqueue(cLow);
    pqService.enqueue(cCrit);
    pqService.enqueue(cMed1);
    pqService.enqueue(cHigh);

    TEST_ASSERT(pqService.size() == 5, "Priority queue contains 5 items");

    // Pop 1: Should be CRITICAL
    Complaint pop1 = pqService.popNext();
    TEST_ASSERT(pop1.getPriority() == Priority::CRITICAL, "1st popped is CRITICAL");
    TEST_ASSERT(pop1.getDescription().find("gas leak") != std::string::npos, "1st is gas leak complaint");

    // Pop 2: Should be HIGH
    Complaint pop2 = pqService.popNext();
    TEST_ASSERT(pop2.getPriority() == Priority::HIGH, "2nd popped is HIGH");

    // Pop 3 & 4: Both are MEDIUM. Earlier filed cMed1 (10:00:00Z) must pop before cMed2 (10:30:00Z) (FIFO tie-breaker)
    Complaint pop3 = pqService.popNext();
    Complaint pop4 = pqService.popNext();
    TEST_ASSERT(pop3.getPriority() == Priority::MEDIUM, "3rd popped is MEDIUM");
    TEST_ASSERT(pop4.getPriority() == Priority::MEDIUM, "4th popped is MEDIUM");
    TEST_ASSERT(pop3.getDescription().find("streetlight") != std::string::npos, "Tie-breaker: earlier complaint (10:00:00Z) popped first");
    TEST_ASSERT(pop4.getDescription().find("bench") != std::string::npos, "Tie-breaker: later complaint (10:30:00Z) popped second");

    // Pop 5: Should be LOW
    Complaint pop5 = pqService.popNext();
    TEST_ASSERT(pop5.getPriority() == Priority::LOW, "5th popped is LOW");
    TEST_ASSERT(pqService.isEmpty(), "Queue is empty after extracting all elements");
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "      CIVICCARE - MEMBER 3 UNIT TESTS & VERIFICATION     \n";
    std::cout << "========================================================\n";

    try {
        testComplaintValidation();
        testRegistrationAndMLIntegration();
        testComplaintLifecycleAndStateTransitions();
        testPriorityQueueAndTieBreaker();

        std::cout << "\n========================================================\n";
        std::cout << "  ALL MEMBER 3 TESTS PASSED SUCCESSFULLY! (100% DONE)   \n";
        std::cout << "========================================================\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "\n[-] FATAL EXCEPTION: " << ex.what() << "\n";
        return 1;
    }
}
