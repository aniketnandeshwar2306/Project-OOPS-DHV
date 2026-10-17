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
#include <string>

using namespace civiccare;

void printBanner() {
    std::cout << "\n=================================================================\n";
    std::cout << "                 CIVICCARE CIVIC PORTAL                          \n";
    std::cout << "        Member 2: User Management & Authentication               \n";
    std::cout << "=================================================================\n";
}

void demonstratePolymorphism() {
    std::cout << "\n>>> [DEMO 1: OOP Polymorphism & Inheritance]\n";
    std::vector<std::shared_ptr<User>> registry;

    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword("TestPass@2026", salt);

    registry.push_back(std::make_shared<Citizen>(
        1, "Aarav Sharma", "aarav@gmail.com", hash, salt, "9876543210", 12, "Sector 4, Nerul, Navi Mumbai", "VOTER-102938"
    ));
    registry.push_back(std::make_shared<MunicipalStaff>(
        2, "Rajesh Kadam", "rajesh.k@bmc.gov.in", hash, salt, "9822011223", 5, "Solid Waste Management", "BMC-STAFF-404", "Field Inspector"
    ));
    registry.push_back(std::make_shared<Administrator>(
        3, "Dr. Sunita Patil", "sunita.admin@bmc.gov.in", hash, salt, "9820099881", 3, "Central Municipal Headquarters"
    ));

    std::cout << "Iterating over std::vector<std::shared_ptr<User>> using dynamic polymorphism:\n\n";
    for (const auto& user : registry) {
        // Polymorphic call to operator<< which delegates to user->displayProfile()
        std::cout << *user << "\n\n";
    }
}

void demonstrateRBAC(AuthenticationService& authService) {
    std::cout << "\n>>> [DEMO 2: Role-Based Access Control (RBAC) & Service-Level Security]\n";

    try {
        // Step A: Seed initial administrator
        std::cout << "1. Bootstrapping Super-Administrator...\n";
        try {
            authService.seedInitialAdmin(
                "Super Admin", "admin@civiccare.gov.in", "AdminRootPass2026", "9800000001", "Central Municipal HQ"
            );
            std::cout << "   [+] Super Admin created successfully.\n";
        } catch (const UnauthorizedException&) {
            std::cout << "   [*] Super Admin already seeded.\n";
        }

        // Step B: Admin login
        Session adminSession = authService.login("admin@civiccare.gov.in", "AdminRootPass2026");
        std::cout << "   [+] Admin Logged In. Session Token: " << adminSession.getToken().substr(0, 16) << "...\n";

        // Step C: Public registration of a citizen
        std::cout << "\n2. Registering Citizen (Public Registration)...\n";
        std::string citizenEmail = "priya.citizen@gmail.com";
        std::shared_ptr<Citizen> citizen;
        if (!authService.getRepository()->existsByEmail(citizenEmail)) {
            citizen = authService.registerCitizen(
                "Priya Sharma", citizenEmail, "PriyaPass2026", "9819998877", 14, "Bandra West, Ward H/West"
            );
            std::cout << "   [+] Citizen registered with ID: " << citizen->getId() << "\n";
        }

        Session citizenSession = authService.login(citizenEmail, "PriyaPass2026");
        std::cout << "   [+] Citizen Logged In. Session Token: " << citizenSession.getToken().substr(0, 16) << "...\n";

        // Step D: Testing Privilege Escalation Prevention
        std::cout << "\n3. Testing Privilege Escalation Prevention:\n";
        std::cout << "   Attempting to register staff using Citizen session...\n";
        try {
            authService.registerStaff(
                citizenSession.getToken(), "Unauthorized Staff", "unauth@bmc.in", "StaffPass123", "9999999999",
                1, "Roads", "STF-999", "Hacker"
            );
            std::cerr << "   [-] ERROR: Security breach! Citizen was allowed to register staff!\n";
        } catch (const UnauthorizedException& e) {
            std::cout << "   [SUCCESS] Rejected with UnauthorizedException: " << e.what() << "\n";
        }

        // Step E: Authorized Staff Registration by Admin
        std::cout << "\n4. Admin registering Municipal Staff...\n";
        std::string staffEmail = "anil.deshmukh@bmc.gov.in";
        std::shared_ptr<MunicipalStaff> staff;
        if (!authService.getRepository()->existsByEmail(staffEmail)) {
            staff = authService.registerStaff(
                adminSession.getToken(), "Anil Deshmukh", staffEmail, "StaffPass2026", "9822334455",
                10, "Stormwater Drains", "BMC-SWD-042", "Assistant Drainage Engineer"
            );
            std::cout << "   [+] Staff registered with ID: " << staff->getId() << "\n";
        }
        Session staffSession = authService.login(staffEmail, "StaffPass2026");

        // Step F: Complaint Ownership and Authorization Test
        std::cout << "\n5. Testing Complaint Ownership & Access Rules:\n";
        int complaintId = 501;
        int citizenUserId = citizenSession.getUserId();
        int staffUserId = staffSession.getUserId();
        int otherCitizenUserId = citizenUserId + 999;

        std::cout << "   Complaint #501 registered by Citizen (ID: " << citizenUserId << ") assigned to Staff (ID: " << staffUserId << ")\n";

        // Check Citizen
        bool cCanAccess = authService.canAccessComplaint(citizenSession.getToken(), citizenUserId, staffUserId);
        std::cout << "   - Can Citizen access their own complaint?  -> " << (cCanAccess ? "YES (AUTHORIZED)" : "NO") << "\n";

        // Check Unauthorized Citizen access
        bool rogueAccess = authService.canAccessComplaint(citizenSession.getToken(), otherCitizenUserId, staffUserId);
        std::cout << "   - Can Citizen access someone else's complaint? -> " << (rogueAccess ? "YES" : "NO (STRICTLY BLOCKED)") << "\n";

        // Check Staff
        bool sCanAccess = authService.canAccessComplaint(staffSession.getToken(), citizenUserId, staffUserId);
        std::cout << "   - Can assigned Staff access complaint?     -> " << (sCanAccess ? "YES (AUTHORIZED)" : "NO") << "\n";

        // Check Admin
        bool aCanAccess = authService.canAccessComplaint(adminSession.getToken(), citizenUserId, staffUserId);
        std::cout << "   - Can Administrator access complaint?      -> " << (aCanAccess ? "YES (FULL SUPERUSER ACCESS)" : "NO") << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[-] Exception in demonstration: " << e.what() << "\n";
    }
}

void demonstratePersistence() {
    std::cout << "\n>>> [DEMO 3: Persistent File Storage & Restart Recovery]\n";
    std::string demoFile = "demo_civiccare_users.dat";

    std::cout << "Phase 1: Writing records and closing storage file...\n";
    {
        auto repo = std::make_shared<FileUserRepository>(demoFile);
        AuthenticationService auth(repo);

        std::string email = "test.restart@civiccare.org";
        if (!repo->existsByEmail(email)) {
            auth.registerCitizen("Restart Tester", email, "SecureRestart2026", "9876543210", 3, "Andheri East");
        }
        std::cout << "   Persisted total " << repo->count() << " users to '" << demoFile << "'.\n";
    }

    std::cout << "Phase 2: Re-opening fresh application session from '" << demoFile << "'...\n";
    {
        auto repo = std::make_shared<FileUserRepository>(demoFile);
        AuthenticationService auth(repo);

        std::cout << "   Loaded total " << repo->count() << " users from storage.\n";
        Session sess = auth.login("test.restart@civiccare.org", "SecureRestart2026");
        std::cout << "   [+] Login succeeded after restart! User: " << sess.getUserName()
                  << " | Role: " << roleToString(sess.getUserRole()) << "\n";
    }
}

int main() {
    printBanner();

    demonstratePolymorphism();

    auto repo = std::make_shared<FileUserRepository>("civiccare_demo_users.dat");
    AuthenticationService authService(repo);

    demonstrateRBAC(authService);
    demonstratePersistence();

    std::cout << "\n=================================================================\n";
    std::cout << "          MEMBER 2 DEMONSTRATION COMPLETE SUCCESSFULLY!          \n";
    std::cout << "=================================================================\n";
    return 0;
}
