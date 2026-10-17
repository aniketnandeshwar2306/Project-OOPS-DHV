#include "models/User.hpp"
#include "models/Citizen.hpp"
#include "models/MunicipalStaff.hpp"
#include "models/Administrator.hpp"
#include "models/Role.hpp"
#include "models/Permission.hpp"
#include "models/Session.hpp"
#include "repositories/FileUserRepository.hpp"
#include "services/AuthenticationService.hpp"
#include "exceptions/CivicCareExceptions.hpp"
#include "utils/PasswordHasher.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <cstdio>
#include <sstream>

using namespace civiccare;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] Assertion FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            std::exit(1); \
        } \
    } while (0)

#define RUN_TEST(func) \
    do { \
        std::cout << "[RUNNING] " << #func << "...\n"; \
        func(); \
        std::cout << "[PASSED]  " << #func << "\n\n"; \
        totalPassed++; \
    } while (0)

static int totalPassed = 0;
const std::string TEST_STORAGE_PATH = "test_civiccare_users.dat";

void cleanupTestFile() {
    std::remove(TEST_STORAGE_PATH.c_str());
}

// -------------------------------------------------------------
// Test 1: OOP Polymorphism & Class Hierarchy
// -------------------------------------------------------------
void testPolymorphismAndHierarchy() {
    std::vector<std::shared_ptr<User>> users;

    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword("SecurePass123", salt);

    users.push_back(std::make_shared<Citizen>(
        1, "Aarav Sharma", "aarav@gmail.com", hash, salt, "9876543210", 12, "Sector 4, Nerul", "VOTER-101"
    ));
    users.push_back(std::make_shared<MunicipalStaff>(
        2, "Rajesh Kadam", "rajesh.k@bmc.gov.in", hash, salt, "9822011223", 5, "Solid Waste Management", "BMC-STAFF-404", "Field Inspector"
    ));
    users.push_back(std::make_shared<Administrator>(
        3, "Dr. Sunita Patil", "sunita.admin@bmc.gov.in", hash, salt, "9820099881", 3, "Municipal Central HQ"
    ));

    // Polymorphic calls
    TEST_ASSERT(users[0]->getRole() == UserRole::CITIZEN, "User 0 must be CITIZEN");
    TEST_ASSERT(users[0]->getRoleName() == "Citizen", "User 0 role name must be Citizen");

    TEST_ASSERT(users[1]->getRole() == UserRole::MUNICIPAL_STAFF, "User 1 must be MUNICIPAL_STAFF");
    TEST_ASSERT(users[1]->getRoleName() == "Municipal Staff", "User 1 role name must be Municipal Staff");

    TEST_ASSERT(users[2]->getRole() == UserRole::ADMINISTRATOR, "User 2 must be ADMINISTRATOR");
    TEST_ASSERT(users[2]->getRoleName() == "Administrator", "User 2 role name must be Administrator");

    // Test stream insertion & displayProfile polymorphism
    for (const auto& u : users) {
        std::ostringstream oss;
        oss << *u;
        TEST_ASSERT(!oss.str().empty(), "displayProfile output should not be empty");
    }

    // Test Virtual Copy Constructor (Prototype / clone())
    auto clonedCitizen = users[0]->clone();
    TEST_ASSERT(clonedCitizen != nullptr, "Cloned pointer must not be null");
    TEST_ASSERT(clonedCitizen->getRole() == UserRole::CITIZEN, "Cloned role must match");
    TEST_ASSERT(clonedCitizen->getEmail() == "aarav@gmail.com", "Cloned email must match");
}

// -------------------------------------------------------------
// Test 2: Password Hashing & Security
// -------------------------------------------------------------
void testPasswordHashing() {
    std::string pwd = "MyStrongPassword2026";
    std::string salt1 = PasswordHasher::generateSalt();
    std::string salt2 = PasswordHasher::generateSalt();

    TEST_ASSERT(salt1 != salt2, "Two generated salts should be distinct");
    TEST_ASSERT(salt1.length() >= 32, "Salt length should be at least 32 hex chars");

    std::string hash1 = PasswordHasher::hashPassword(pwd, salt1);
    std::string hash2 = PasswordHasher::hashPassword(pwd, salt2);

    // Hashes must be different because salts are different
    TEST_ASSERT(hash1 != hash2, "Different salts must produce different hashes");
    TEST_ASSERT(hash1.length() == 64, "SHA-256 hash must be 64 hexadecimal characters");
    TEST_ASSERT(hash1.find(pwd) == std::string::npos, "Plaintext password must NOT appear in hash");

    // Verification
    TEST_ASSERT(PasswordHasher::verifyPassword(pwd, salt1, hash1), "Correct password must verify successfully");
    TEST_ASSERT(!PasswordHasher::verifyPassword("WrongPassword1", salt1, hash1), "Wrong password must fail");
}

// -------------------------------------------------------------
// Test 3: Input Validation & Sanitization
// -------------------------------------------------------------
void testInputValidation() {
    cleanupTestFile();
    auto repo = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
    AuthenticationService auth(repo);

    // 1. Invalid email
    bool caught = false;
    try {
        auth.registerCitizen("John Doe", "invalid-email-address", "Pass123", "9876543210", 5);
    } catch (const ValidationException& e) {
        caught = true;
    }
    TEST_ASSERT(caught, "Registering with invalid email must throw ValidationException");

    // 2. Weak password (too short)
    caught = false;
    try {
        auth.registerCitizen("John Doe", "john@example.com", "123", "9876543210", 5);
    } catch (const ValidationException& e) {
        caught = true;
    }
    TEST_ASSERT(caught, "Registering with short password must throw ValidationException");

    // 3. Weak password (no digits)
    caught = false;
    try {
        auth.registerCitizen("John Doe", "john@example.com", "OnlyLettersPass", "9876543210", 5);
    } catch (const ValidationException& e) {
        caught = true;
    }
    TEST_ASSERT(caught, "Registering with password lacking digits must throw ValidationException");

    // 4. Empty name
    caught = false;
    try {
        auth.registerCitizen("   ", "john@example.com", "Pass1234", "9876543210", 5);
    } catch (const ValidationException& e) {
        caught = true;
    }
    TEST_ASSERT(caught, "Registering with blank name must throw ValidationException");

    // 5. Invalid ward number
    caught = false;
    try {
        auth.registerCitizen("John Doe", "john@example.com", "Pass1234", "9876543210", 999);
    } catch (const ValidationException& e) {
        caught = true;
    }
    TEST_ASSERT(caught, "Registering with invalid ward number must throw ValidationException");

    cleanupTestFile();
}

// -------------------------------------------------------------
// Test 4: Registration, Duplicate Prevention & Seed Admin
// -------------------------------------------------------------
void testRegistrationAndDuplicates() {
    cleanupTestFile();
    auto repo = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
    AuthenticationService auth(repo);

    // Register a valid citizen
    auto citizen = auth.registerCitizen(
        "Kavita Deshmukh", "kavita@mumbai.in", "Password@123", "9819012345", 14, "Dadar West"
    );
    TEST_ASSERT(citizen != nullptr, "Citizen pointer must not be null");
    TEST_ASSERT(citizen->getId() > 0, "Assigned ID must be positive");
    TEST_ASSERT(citizen->getRole() == UserRole::CITIZEN, "Role must be CITIZEN");

    // Duplicate email registration should fail
    bool caughtDuplicate = false;
    try {
        auth.registerCitizen(
            "Another Person", "kavita@mumbai.in", "DifferentPass1", "9811111111", 14
        );
    } catch (const DuplicateEmailException& e) {
        caughtDuplicate = true;
        TEST_ASSERT(e.getEmail() == "kavita@mumbai.in", "Exception must report offending email");
    }
    TEST_ASSERT(caughtDuplicate, "Duplicate email registration must throw DuplicateEmailException");

    // Seed initial admin
    auto admin = auth.seedInitialAdmin(
        "Commissioner V. Rao", "commissioner@bmc.gov.in", "AdminMaster2026", "9820000001", "Central Municipal HQ"
    );
    TEST_ASSERT(admin != nullptr, "Initial admin must be created");
    TEST_ASSERT(admin->getRole() == UserRole::ADMINISTRATOR, "Role must be ADMINISTRATOR");

    // Second call to seedInitialAdmin should be rejected
    bool caughtSecondAdmin = false;
    try {
        auth.seedInitialAdmin("Rogue Admin", "rogue@admin.com", "HackedPass1", "9999999999");
    } catch (const UnauthorizedException&) {
        caughtSecondAdmin = true;
    }
    TEST_ASSERT(caughtSecondAdmin, "Seeding multiple initial admins must be prohibited");

    cleanupTestFile();
}

// -------------------------------------------------------------
// Test 5: Authentication, Login & Session Management
// -------------------------------------------------------------
void testAuthenticationAndSessions() {
    cleanupTestFile();
    auto repo = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
    AuthenticationService auth(repo);

    auth.registerCitizen("Pooja Nair", "pooja@gmail.com", "PoojaSecret9", "9988776655", 8);

    // 1. Successful Login
    Session session = auth.login("pooja@gmail.com", "PoojaSecret9");
    TEST_ASSERT(session.isValid(), "Session must be valid after successful login");
    TEST_ASSERT(!session.getToken().empty(), "Session token must be populated");
    TEST_ASSERT(session.getUserEmail() == "pooja@gmail.com", "Session email must match");
    TEST_ASSERT(session.getUserRole() == UserRole::CITIZEN, "Session user role must be CITIZEN");

    // 2. Failed Login: Wrong password
    bool caught = false;
    try {
        auth.login("pooja@gmail.com", "IncorrectPassword1");
    } catch (const AuthenticationException&) {
        caught = true;
    }
    TEST_ASSERT(caught, "Login with wrong password must throw AuthenticationException");

    // 3. Failed Login: Non-existent email
    caught = false;
    try {
        auth.login("nonexistent@domain.com", "SomePass123");
    } catch (const AuthenticationException&) {
        caught = true;
    }
    TEST_ASSERT(caught, "Login with non-existent email must throw AuthenticationException");

    // 4. Logout
    auth.logout(session.getToken());
    TEST_ASSERT(!auth.isSessionValid(session.getToken()), "Session must be invalid after logout");

    caught = false;
    try {
        auth.getSession(session.getToken());
    } catch (const SessionExpiredException&) {
        caught = true;
    }
    TEST_ASSERT(caught, "Accessing logged-out session must throw SessionExpiredException");

    cleanupTestFile();
}

// -------------------------------------------------------------
// Test 6: Role-Based Access Control (RBAC) & Privilege Separation
// -------------------------------------------------------------
void testRBACAndPrivileges() {
    cleanupTestFile();
    auto repo = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
    AuthenticationService auth(repo);

    // Setup: Admin and Citizen
    auto admin = auth.seedInitialAdmin("Head Admin", "head.admin@civic.gov", "AdminPass2026", "9990001111");
    auto citizen = auth.registerCitizen("Amit Verma", "amit@yahoo.com", "AmitPass123", "9870001122", 3);

    Session adminSession = auth.login("head.admin@civic.gov", "AdminPass2026");
    Session citizenSession = auth.login("amit@yahoo.com", "AmitPass123");

    // 1. Citizen cannot register staff (privilege escalation prevention)
    bool caught = false;
    try {
        auth.registerStaff(
            citizenSession.getToken(), "Fake Staff", "fake@staff.com", "StaffPass1", "9000000000",
            1, "Roads", "STF-01", "Officer"
        );
    } catch (const UnauthorizedException& e) {
        caught = true;
        TEST_ASSERT(e.getRequiredPermission() == "MANAGE_STAFF", "Required permission must be MANAGE_STAFF");
    }
    TEST_ASSERT(caught, "Citizen attempting to register staff must be rejected with UnauthorizedException");

    // 2. Admin CAN register staff
    auto staff = auth.registerStaff(
        adminSession.getToken(), "Suresh Shinde", "suresh@bmc.gov.in", "SureshPass1", "9123456789",
        2, "Water Dept", "BMC-W-10", "Junior Engineer"
    );
    TEST_ASSERT(staff != nullptr, "Admin must be able to register staff");

    Session staffSession = auth.login("suresh@bmc.gov.in", "SureshPass1");

    // 3. Permission checks
    TEST_ASSERT(auth.checkPermission(citizenSession.getToken(), Permission::LODGE_COMPLAINT), "Citizen can lodge complaints");
    TEST_ASSERT(!auth.checkPermission(citizenSession.getToken(), Permission::MANAGE_STAFF), "Citizen CANNOT manage staff");
    TEST_ASSERT(!auth.checkPermission(citizenSession.getToken(), Permission::SYSTEM_BACKUP), "Citizen CANNOT backup system");

    TEST_ASSERT(auth.checkPermission(staffSession.getToken(), Permission::VIEW_ASSIGNED_COMPLAINTS), "Staff can view assigned complaints");
    TEST_ASSERT(auth.checkPermission(staffSession.getToken(), Permission::UPDATE_COMPLAINT_STATUS), "Staff can update complaint status");
    TEST_ASSERT(!auth.checkPermission(staffSession.getToken(), Permission::MANAGE_USERS), "Staff CANNOT manage users");

    TEST_ASSERT(auth.checkPermission(adminSession.getToken(), Permission::MANAGE_USERS), "Admin can manage users");
    TEST_ASSERT(auth.checkPermission(adminSession.getToken(), Permission::SYSTEM_BACKUP), "Admin can perform system backup");
    TEST_ASSERT(auth.checkPermission(adminSession.getToken(), Permission::VIEW_ANALYTICS), "Admin can view analytics");

    // 4. requirePermission enforcement
    caught = false;
    try {
        auth.requirePermission(staffSession.getToken(), Permission::MANAGE_USERS);
    } catch (const UnauthorizedException&) {
        caught = true;
    }
    TEST_ASSERT(caught, "Staff executing requirePermission(MANAGE_USERS) must throw UnauthorizedException");

    cleanupTestFile();
}

// -------------------------------------------------------------
// Test 7: Complaint Ownership & Access Rules
// -------------------------------------------------------------
void testComplaintAccessRules() {
    cleanupTestFile();
    auto repo = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
    AuthenticationService auth(repo);

    auto admin = auth.seedInitialAdmin("Central Admin", "admin@bmc.org", "AdminPass999", "9000000001");
    auto citizen1 = auth.registerCitizen("Citizen One", "c1@test.com", "PassOne123", "9000000002", 1);
    auto citizen2 = auth.registerCitizen("Citizen Two", "c2@test.com", "PassTwo123", "9000000003", 2);

    Session adminSession = auth.login("admin@bmc.org", "AdminPass999");
    auto staff1 = auth.registerStaff(
        adminSession.getToken(), "Staff Alpha", "s1@bmc.org", "StaffPass123", "9000000004",
        10, "Sanitation", "SAN-01", "Inspector"
    );
    auto staff2 = auth.registerStaff(
        adminSession.getToken(), "Staff Beta", "s2@bmc.org", "StaffPass123", "9000000005",
        20, "Roads", "ROD-01", "Engineer"
    );

    Session c1Session = auth.login("c1@test.com", "PassOne123");
    Session c2Session = auth.login("c2@test.com", "PassTwo123");
    Session s1Session = auth.login("s1@bmc.org", "StaffPass123");
    Session s2Session = auth.login("s2@bmc.org", "StaffPass123");

    // Suppose Complaint #101 was lodged by Citizen 1 and assigned to Staff 1
    int complaint101_creator = citizen1->getId();
    int complaint101_assignedStaff = staff1->getId();

    // 1. Citizen 1 (the owner) can access complaint #101
    TEST_ASSERT(auth.canAccessComplaint(c1Session.getToken(), complaint101_creator, complaint101_assignedStaff),
                "Citizen 1 must have access to their own complaint");

    // 2. Citizen 2 CANNOT access complaint #101
    TEST_ASSERT(!auth.canAccessComplaint(c2Session.getToken(), complaint101_creator, complaint101_assignedStaff),
                "Citizen 2 must NOT have access to Citizen 1's complaint");

    bool caught = false;
    try {
        auth.authorizeComplaintAccess(c2Session.getToken(), complaint101_creator, complaint101_assignedStaff);
    } catch (const UnauthorizedException&) {
        caught = true;
    }
    TEST_ASSERT(caught, "authorizeComplaintAccess for Citizen 2 must throw UnauthorizedException");

    // 3. Assigned Staff 1 can access complaint #101
    TEST_ASSERT(auth.canAccessComplaint(s1Session.getToken(), complaint101_creator, complaint101_assignedStaff),
                "Staff 1 (assigned) must have access to complaint #101");

    // 4. Unassigned Staff 2 CANNOT access complaint #101
    TEST_ASSERT(!auth.canAccessComplaint(s2Session.getToken(), complaint101_creator, complaint101_assignedStaff),
                "Staff 2 (unassigned) must NOT have access to complaint #101");

    caught = false;
    try {
        auth.authorizeComplaintAccess(s2Session.getToken(), complaint101_creator, complaint101_assignedStaff);
    } catch (const UnauthorizedException&) {
        caught = true;
    }
    TEST_ASSERT(caught, "authorizeComplaintAccess for Staff 2 must throw UnauthorizedException");

    // 5. Administrator can access ANY complaint
    TEST_ASSERT(auth.canAccessComplaint(adminSession.getToken(), complaint101_creator, complaint101_assignedStaff),
                "Admin must have access to any complaint in the municipality");

    cleanupTestFile();
}

// -------------------------------------------------------------
// Test 8: Persistence Across Application Restarts
// -------------------------------------------------------------
void testPersistenceAcrossRestarts() {
    cleanupTestFile();

    // PHASE 1: Run application session 1, register users, and close
    {
        auto repo1 = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
        AuthenticationService auth1(repo1);

        auto admin = auth1.seedInitialAdmin("Restart Admin", "restart.admin@bmc.in", "AdminPass123", "9876543210");
        Session adminSess = auth1.login("restart.admin@bmc.in", "AdminPass123");

        auth1.registerCitizen("Resident Maya", "maya@residence.com", "MayaPass123", "9812345678", 7, "Apt 201, Chembur");
        auth1.registerStaff(
            adminSess.getToken(), "Officer Joshi", "joshi@bmc.in", "JoshiPass123", "9823456789",
            3, "Electricity", "ELE-99", "Lineman Chief"
        );

        TEST_ASSERT(repo1->count() == 3, "Phase 1: Must have 3 persisted users");
        // Simulated process termination: repo1 and auth1 fall out of scope and are destroyed
    }

    // PHASE 2: Reboot application fresh, recreate repository from file
    {
        auto repo2 = std::make_shared<FileUserRepository>(TEST_STORAGE_PATH);
        AuthenticationService auth2(repo2);

        TEST_ASSERT(repo2->count() == 3, "Phase 2: Reopened repository must still contain 3 users");

        // Verify users can log in using original credentials
        Session adminSess = auth2.login("restart.admin@bmc.in", "AdminPass123");
        TEST_ASSERT(adminSess.getUserRole() == UserRole::ADMINISTRATOR, "Admin role restored polymorphically");

        Session citizenSess = auth2.login("maya@residence.com", "MayaPass123");
        TEST_ASSERT(citizenSess.getUserRole() == UserRole::CITIZEN, "Citizen role restored polymorphically");
        auto loadedCitizen = std::dynamic_pointer_cast<const Citizen>(citizenSess.getUser());
        TEST_ASSERT(loadedCitizen != nullptr, "Dynamic cast to Citizen must succeed");
        TEST_ASSERT(loadedCitizen->getWardNumber() == 7, "Citizen ward number must persist");
        TEST_ASSERT(loadedCitizen->getResidentialAddress() == "Apt 201, Chembur", "Address must persist");

        Session staffSess = auth2.login("joshi@bmc.in", "JoshiPass123");
        TEST_ASSERT(staffSess.getUserRole() == UserRole::MUNICIPAL_STAFF, "Staff role restored polymorphically");
        auto loadedStaff = std::dynamic_pointer_cast<const MunicipalStaff>(staffSess.getUser());
        TEST_ASSERT(loadedStaff != nullptr, "Dynamic cast to MunicipalStaff must succeed");
        TEST_ASSERT(loadedStaff->getDepartmentId() == 3, "Staff department ID must persist");
        TEST_ASSERT(loadedStaff->getEmployeeId() == "ELE-99", "Staff employee ID must persist");
    }

    cleanupTestFile();
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "    CIVICCARE: MEMBER 2 COMPREHENSIVE TEST SUITE         \n";
    std::cout << "    User Management, Authentication & RBAC Verification  \n";
    std::cout << "=========================================================\n\n";

    RUN_TEST(testPolymorphismAndHierarchy);
    RUN_TEST(testPasswordHashing);
    RUN_TEST(testInputValidation);
    RUN_TEST(testRegistrationAndDuplicates);
    RUN_TEST(testAuthenticationAndSessions);
    RUN_TEST(testRBACAndPrivileges);
    RUN_TEST(testComplaintAccessRules);
    RUN_TEST(testPersistenceAcrossRestarts);

    std::cout << "=========================================================\n";
    std::cout << " ALL " << totalPassed << " TEST SUITES PASSED SUCCESSFULLY! (100% SUCCESS) \n";
    std::cout << "=========================================================\n";
    return 0;
}
