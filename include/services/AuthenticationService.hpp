#ifndef CIVICCARE_SERVICES_AUTHENTICATIONSERVICE_HPP
#define CIVICCARE_SERVICES_AUTHENTICATIONSERVICE_HPP

#include "models/User.hpp"
#include "models/Citizen.hpp"
#include "models/MunicipalStaff.hpp"
#include "models/Administrator.hpp"
#include "models/Session.hpp"
#include "models/Permission.hpp"
#include "repositories/UserRepository.hpp"

#include <memory>
#include <string>
#include <unordered_map>
namespace civiccare {

/**
 * @brief Service responsible for User Authentication, Registration, Session Management,
 * and Role-Based Access Control (RBAC).
 * 
 * Implements Dependency Injection with UserRepository to ensure clean decoupling from
 * the storage layer (SQLite / File storage).
 */
class AuthenticationService {
private:
    std::shared_ptr<UserRepository> m_userRepository;
    std::unordered_map<std::string, Session> m_activeSessions;

    std::string generateSessionToken() const;

public:
    /**
     * @brief Constructs AuthenticationService with an injected UserRepository.
     * @param userRepository Abstract repository for user persistence.
     */
    explicit AuthenticationService(std::shared_ptr<UserRepository> userRepository);

    ~AuthenticationService() = default;

    // ==========================================
    // Registration Methods
    // ==========================================

    /**
     * @brief Public registration for citizens.
     * Prevents citizens from elevating privileges or selecting administrative/staff roles.
     * @param name Full name.
     * @param email Unique email address.
     * @param password Plaintext password (will be validated and hashed).
     * @param phone Contact number.
     * @param wardNumber Civic ward number.
     * @param address Residential address.
     * @param civicId Optional Aadhar/Voter ID.
     * @return Shared pointer to newly created Citizen.
     * @throws ValidationException on invalid input.
     * @throws DuplicateEmailException if email is already in use.
     */
    std::shared_ptr<Citizen> registerCitizen(
        const std::string& name,
        const std::string& email,
        const std::string& password,
        const std::string& phone,
        int wardNumber,
        const std::string& address = "",
        const std::string& civicId = "");

    /**
     * @brief Privileged registration for Municipal Staff.
     * Can ONLY be invoked by an authenticated Administrator.
     * @param adminSessionToken Valid session token belonging to an Administrator.
     * @param name Full name.
     * @param email Staff corporate email.
     * @param password Plaintext password.
     * @param phone Contact number.
     * @param departmentId Assigned municipal department ID.
     * @param departmentName Assigned department name (e.g., "Water Supply").
     * @param employeeId Unique municipal employee badge ID.
     * @param designation Job title (e.g., "Senior Field Inspector").
     * @return Shared pointer to newly created MunicipalStaff.
     * @throws UnauthorizedException if session is not an Administrator.
     */
    std::shared_ptr<MunicipalStaff> registerStaff(
        const std::string& adminSessionToken,
        const std::string& name,
        const std::string& email,
        const std::string& password,
        const std::string& phone,
        int departmentId,
        const std::string& departmentName,
        const std::string& employeeId,
        const std::string& designation);

    /**
     * @brief Privileged registration for an Administrator.
     * Can ONLY be invoked by an existing authenticated Administrator.
     * @param adminSessionToken Valid session token belonging to an Administrator.
     * @param name Full name.
     * @param email Administrative email.
     * @param password Plaintext password.
     * @param phone Contact phone.
     * @param adminLevel Admin tier (1 = Ward, 2 = Zonal, 3 = Central).
     * @param officeLocation Headquarters or regional office name.
     * @return Shared pointer to newly created Administrator.
     * @throws UnauthorizedException if caller is not an Administrator.
     */
    std::shared_ptr<Administrator> registerAdmin(
        const std::string& adminSessionToken,
        const std::string& name,
        const std::string& email,
        const std::string& password,
        const std::string& phone,
        int adminLevel = 1,
        const std::string& officeLocation = "Central Headquarters");

    /**
     * @brief System Bootstrap method: seeds the initial Super-Admin when the database is empty.
     * If an Administrator already exists, this method is locked and throws an exception.
     */
    std::shared_ptr<Administrator> seedInitialAdmin(
        const std::string& name,
        const std::string& email,
        const std::string& password,
        const std::string& phone,
        const std::string& officeLocation = "Central Headquarters");

    // ==========================================
    // Authentication & Session Management
    // ==========================================

    /**
     * @brief Authenticates a user by email and password, issuing a new session token.
     * @param email Login email.
     * @param password Plaintext password.
     * @return Active Session object containing token and user profile.
     * @throws AuthenticationException on invalid credentials.
     */
    Session login(const std::string& email, const std::string& password);

    /**
     * @brief Ends an active session.
     * @param sessionToken Active session token.
     */
    void logout(const std::string& sessionToken);

    /**
     * @brief Verifies whether a given session token is active and valid.
     */
    bool isSessionValid(const std::string& sessionToken) const;

    /**
     * @brief Retrieves the active Session object by token.
     * @throws SessionExpiredException if session is invalid.
     */
    Session getSession(const std::string& sessionToken) const;

    /**
     * @brief Retrieves the authenticated User attached to a session.
     * @throws SessionExpiredException if session is invalid.
     */
    std::shared_ptr<const User> getAuthenticatedUser(const std::string& sessionToken) const;

    // ==========================================
    // Role-Based Access Control (RBAC)
    // ==========================================

    /**
     * @brief Checks if the authenticated session has a specific permission.
     */
    bool checkPermission(const std::string& sessionToken, Permission permission) const;

    /**
     * @brief Enforces that an authenticated session possesses a specific permission.
     * Validates permission at the service method level.
     * @throws SessionExpiredException if session is invalid.
     * @throws UnauthorizedException if user lacks the required permission.
     */
    void requirePermission(const std::string& sessionToken, Permission permission) const;

    /**
     * @brief Evaluates whether the authenticated user has access rights to a specific complaint.
     * - Citizens can only view/access their own complaints.
     * - Municipal Staff can only view/access complaints assigned to them.
     * - Administrators can access all complaints.
     */
    bool canAccessComplaint(const std::string& sessionToken, int complaintCitizenId, int assignedStaffId = -1) const;

    /**
     * @brief Enforces access authorization for a specific complaint.
     * @throws UnauthorizedException if access is not permitted.
     */
    void authorizeComplaintAccess(const std::string& sessionToken, int complaintCitizenId, int assignedStaffId = -1) const;

    // ==========================================
    // Profile & Password Management
    // ==========================================

    /**
     * @brief Changes the password for the currently authenticated user.
     */
    void changePassword(const std::string& sessionToken,
                        const std::string& oldPassword,
                        const std::string& newPassword);

    /**
     * @brief Updates user's personal profile information (name and phone).
     */
    void updateProfile(const std::string& sessionToken,
                       const std::string& newName,
                       const std::string& newPhone);

    /**
     * @brief Returns reference to the injected repository.
     */
    std::shared_ptr<UserRepository> getRepository() const noexcept { return m_userRepository; }
};

} // namespace civiccare

#endif // CIVICCARE_SERVICES_AUTHENTICATIONSERVICE_HPP
