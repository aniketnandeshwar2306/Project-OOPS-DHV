#include "services/AuthenticationService.hpp"
#include "exceptions/CivicCareExceptions.hpp"
#include "utils/ValidationUtils.hpp"
#include "utils/PasswordHasher.hpp"
#include "utils/DateTimeUtils.hpp"

#include <random>
#include <sstream>
#include <iomanip>

namespace civiccare {

AuthenticationService::AuthenticationService(std::shared_ptr<UserRepository> userRepository)
    : m_userRepository(userRepository) {
    if (!m_userRepository) {
        throw ValidationException("AuthenticationService requires a valid non-null UserRepository.");
    }
}

std::string AuthenticationService::generateSessionToken() const {
    static const char hexChars[] = "0123456789abcdef";
    static std::random_device rd;
    static std::mt19937 gen([]() {
        uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        try {
            seed ^= rd();
        } catch (...) {}
        return static_cast<unsigned long>(seed);
    }());
    std::uniform_int_distribution<size_t> dis(0, 15);

    std::string token = "sess_";
    for (int i = 0; i < 32; ++i) {
        token += hexChars[dis(gen)];
    }
    return token;
}

std::shared_ptr<Citizen> AuthenticationService::registerCitizen(
    const std::string& name,
    const std::string& email,
    const std::string& password,
    const std::string& phone,
    int wardNumber,
    const std::string& address,
    const std::string& civicId) {

    // 1. Input sanitization and validation
    std::string trimmedName = ValidationUtils::trim(name);
    if (trimmedName.empty()) {
        throw ValidationException("Name cannot be empty.", "name");
    }

    std::string trimmedEmail = ValidationUtils::trim(email);
    if (!ValidationUtils::isValidEmail(trimmedEmail)) {
        throw ValidationException("Invalid email format: '" + email + "'.", "email");
    }

    std::string pwdError;
    if (!ValidationUtils::isValidPassword(password, pwdError)) {
        throw ValidationException(pwdError, "password");
    }

    std::string trimmedPhone = ValidationUtils::trim(phone);
    if (!trimmedPhone.empty() && !ValidationUtils::isValidPhone(trimmedPhone)) {
        throw ValidationException("Invalid phone number format.", "phone");
    }

    if (wardNumber < 1 || wardNumber > 300) {
        throw ValidationException("Ward number must be between 1 and 300.", "wardNumber");
    }

    // 2. Uniqueness check
    if (m_userRepository->existsByEmail(trimmedEmail)) {
        throw DuplicateEmailException(trimmedEmail);
    }

    // 3. Cryptographic salt and hash generation
    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword(password, salt);
    std::string now = DateTimeUtils::getCurrentTimestamp();

    // 4. Instantiate Citizen model
    auto citizen = std::make_shared<Citizen>(
        0, trimmedName, trimmedEmail, hash, salt, trimmedPhone,
        wardNumber, address, civicId, now, now
    );

    // 5. Persist through repository
    m_userRepository->save(citizen);
    return citizen;
}

std::shared_ptr<MunicipalStaff> AuthenticationService::registerStaff(
    const std::string& adminSessionToken,
    const std::string& name,
    const std::string& email,
    const std::string& password,
    const std::string& phone,
    int departmentId,
    const std::string& departmentName,
    const std::string& employeeId,
    const std::string& designation) {

    // 1. RBAC authorization check: Only Administrator can register staff
    requirePermission(adminSessionToken, Permission::MANAGE_STAFF);

    // 2. Input validation
    std::string trimmedName = ValidationUtils::trim(name);
    if (trimmedName.empty()) {
        throw ValidationException("Staff name cannot be empty.", "name");
    }

    std::string trimmedEmail = ValidationUtils::trim(email);
    if (!ValidationUtils::isValidEmail(trimmedEmail)) {
        throw ValidationException("Invalid staff email format: '" + email + "'.", "email");
    }

    std::string pwdError;
    if (!ValidationUtils::isValidPassword(password, pwdError)) {
        throw ValidationException(pwdError, "password");
    }

    if (departmentId <= 0) {
        throw ValidationException("Department ID must be a positive integer.", "departmentId");
    }

    std::string trimmedEmpId = ValidationUtils::trim(employeeId);
    if (trimmedEmpId.empty()) {
        throw ValidationException("Employee ID cannot be empty.", "employeeId");
    }

    // 3. Check duplicate email
    if (m_userRepository->existsByEmail(trimmedEmail)) {
        throw DuplicateEmailException(trimmedEmail);
    }

    // 4. Hash and persist
    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword(password, salt);
    std::string now = DateTimeUtils::getCurrentTimestamp();

    auto staff = std::make_shared<MunicipalStaff>(
        0, trimmedName, trimmedEmail, hash, salt, phone,
        departmentId, departmentName, trimmedEmpId, designation, now, now
    );

    m_userRepository->save(staff);
    return staff;
}

std::shared_ptr<Administrator> AuthenticationService::registerAdmin(
    const std::string& adminSessionToken,
    const std::string& name,
    const std::string& email,
    const std::string& password,
    const std::string& phone,
    int adminLevel,
    const std::string& officeLocation) {

    // 1. RBAC check: Only an existing Administrator can create another Admin
    requirePermission(adminSessionToken, Permission::MANAGE_USERS);

    // 2. Validate
    std::string trimmedName = ValidationUtils::trim(name);
    if (trimmedName.empty()) throw ValidationException("Admin name cannot be empty.", "name");

    std::string trimmedEmail = ValidationUtils::trim(email);
    if (!ValidationUtils::isValidEmail(trimmedEmail)) {
        throw ValidationException("Invalid admin email format.", "email");
    }

    std::string pwdError;
    if (!ValidationUtils::isValidPassword(password, pwdError)) {
        throw ValidationException(pwdError, "password");
    }

    if (m_userRepository->existsByEmail(trimmedEmail)) {
        throw DuplicateEmailException(trimmedEmail);
    }

    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword(password, salt);
    std::string now = DateTimeUtils::getCurrentTimestamp();

    auto admin = std::make_shared<Administrator>(
        0, trimmedName, trimmedEmail, hash, salt, phone,
        adminLevel, officeLocation, now, now
    );

    m_userRepository->save(admin);
    return admin;
}

std::shared_ptr<Administrator> AuthenticationService::seedInitialAdmin(
    const std::string& name,
    const std::string& email,
    const std::string& password,
    const std::string& phone,
    const std::string& officeLocation) {

    // Only allowed if no admin exists in the system
    auto existingAdmins = m_userRepository->findByRole(UserRole::ADMINISTRATOR);
    if (!existingAdmins.empty()) {
        throw UnauthorizedException("Initial admin already seeded. Additional admins must be registered by an existing admin.");
    }

    std::string trimmedName = ValidationUtils::trim(name);
    std::string trimmedEmail = ValidationUtils::trim(email);
    if (!ValidationUtils::isValidEmail(trimmedEmail)) {
        throw ValidationException("Invalid email format.", "email");
    }

    std::string pwdError;
    if (!ValidationUtils::isValidPassword(password, pwdError)) {
        throw ValidationException(pwdError, "password");
    }

    if (m_userRepository->existsByEmail(trimmedEmail)) {
        throw DuplicateEmailException(trimmedEmail);
    }

    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword(password, salt);
    std::string now = DateTimeUtils::getCurrentTimestamp();

    auto admin = std::make_shared<Administrator>(
        0, trimmedName, trimmedEmail, hash, salt, phone,
        3, officeLocation, now, now
    );

    m_userRepository->save(admin);
    return admin;
}

Session AuthenticationService::login(const std::string& email, const std::string& password) {
    std::string trimmedEmail = ValidationUtils::trim(email);
    if (trimmedEmail.empty() || password.empty()) {
        throw AuthenticationException("Email and password cannot be empty.");
    }

    auto user = m_userRepository->findByEmail(trimmedEmail);
    if (!user) {
        throw AuthenticationException("Invalid email or password.");
    }

    if (!user->verifyPassword(password)) {
        throw AuthenticationException("Invalid email or password.");
    }

    std::string token = generateSessionToken();
    Session session(token, user);

    m_activeSessions[token] = session;
    return session;
}

void AuthenticationService::logout(const std::string& sessionToken) {
    auto it = m_activeSessions.find(sessionToken);
    if (it != m_activeSessions.end()) {
        it->second.invalidate();
        m_activeSessions.erase(it);
    }
}

bool AuthenticationService::isSessionValid(const std::string& sessionToken) const {
    auto it = m_activeSessions.find(sessionToken);
    if (it == m_activeSessions.end()) return false;
    return it->second.isValid();
}

Session AuthenticationService::getSession(const std::string& sessionToken) const {
    auto it = m_activeSessions.find(sessionToken);
    if (it == m_activeSessions.end() || !it->second.isValid()) {
        throw SessionExpiredException();
    }
    return it->second;
}

std::shared_ptr<const User> AuthenticationService::getAuthenticatedUser(const std::string& sessionToken) const {
    return getSession(sessionToken).getUser();
}

bool AuthenticationService::checkPermission(const std::string& sessionToken, Permission permission) const {
    auto it = m_activeSessions.find(sessionToken);
    if (it == m_activeSessions.end() || !it->second.isValid()) {
        return false;
    }
    return it->second.hasPermission(permission);
}

void AuthenticationService::requirePermission(const std::string& sessionToken, Permission permission) const {
    auto it = m_activeSessions.find(sessionToken);
    if (it == m_activeSessions.end() || !it->second.isValid()) {
        throw SessionExpiredException();
    }

    if (!it->second.hasPermission(permission)) {
        throw UnauthorizedException(
            "Access Denied: User role '" + roleToString(it->second.getUserRole()) + 
            "' lacks permission '" + permissionToString(permission) + "'.",
            permissionToString(permission)
        );
    }
}

bool AuthenticationService::canAccessComplaint(const std::string& sessionToken, 
                                              int complaintCitizenId, 
                                              int assignedStaffId) const {
    auto it = m_activeSessions.find(sessionToken);
    if (it == m_activeSessions.end() || !it->second.isValid()) {
        return false;
    }
    return it->second.canAccessComplaint(complaintCitizenId, assignedStaffId);
}

void AuthenticationService::authorizeComplaintAccess(const std::string& sessionToken, 
                                                    int complaintCitizenId, 
                                                    int assignedStaffId) const {
    auto it = m_activeSessions.find(sessionToken);
    if (it == m_activeSessions.end() || !it->second.isValid()) {
        throw SessionExpiredException();
    }

    if (!it->second.canAccessComplaint(complaintCitizenId, assignedStaffId)) {
        throw UnauthorizedException(
            "Access Denied: User '" + it->second.getUserEmail() + "' (" + 
            roleToString(it->second.getUserRole()) + 
            ") is not authorized to access complaint owned by citizen ID " + 
            std::to_string(complaintCitizenId) + "."
        );
    }
}

void AuthenticationService::changePassword(const std::string& sessionToken,
                                           const std::string& oldPassword,
                                           const std::string& newPassword) {
    Session session = getSession(sessionToken);
    int userId = session.getUserId();

    auto user = m_userRepository->findById(userId);
    if (!user) {
        throw UserNotFoundException(std::to_string(userId));
    }

    if (!user->verifyPassword(oldPassword)) {
        throw AuthenticationException("Incorrect existing password.");
    }

    std::string pwdError;
    if (!ValidationUtils::isValidPassword(newPassword, pwdError)) {
        throw ValidationException(pwdError, "newPassword");
    }

    user->updatePassword(newPassword);
    m_userRepository->update(user);
}

void AuthenticationService::updateProfile(const std::string& sessionToken,
                                          const std::string& newName,
                                          const std::string& newPhone) {
    Session session = getSession(sessionToken);
    int userId = session.getUserId();

    auto user = m_userRepository->findById(userId);
    if (!user) {
        throw UserNotFoundException(std::to_string(userId));
    }

    std::string trimmedName = ValidationUtils::trim(newName);
    if (!trimmedName.empty()) {
        user->setName(trimmedName);
    }

    std::string trimmedPhone = ValidationUtils::trim(newPhone);
    if (!trimmedPhone.empty()) {
        if (!ValidationUtils::isValidPhone(trimmedPhone)) {
            throw ValidationException("Invalid phone format.", "phone");
        }
        user->setPhoneNumber(trimmedPhone);
    }

    user->setUpdatedAt(DateTimeUtils::getCurrentTimestamp());
    m_userRepository->update(user);
}

} // namespace civiccare
