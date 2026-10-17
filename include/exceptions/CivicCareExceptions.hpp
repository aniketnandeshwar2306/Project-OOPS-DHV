#ifndef CIVICCARE_EXCEPTIONS_CIVICCAREEXCEPTIONS_HPP
#define CIVICCARE_EXCEPTIONS_CIVICCAREEXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace civiccare {

/**
 * @brief Base exception for all CivicCare domain and service errors.
 */
class CivicCareException : public std::runtime_error {
public:
    explicit CivicCareException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Thrown when user authentication fails (e.g., incorrect password, account locked).
 */
class AuthenticationException : public CivicCareException {
public:
    explicit AuthenticationException(const std::string& message = "Invalid email or password.")
        : CivicCareException(message) {}
};

/**
 * @brief Thrown when a user record is not found in the repository.
 */
class UserNotFoundException : public CivicCareException {
public:
    explicit UserNotFoundException(const std::string& identifier)
        : CivicCareException("User not found: " + identifier) {}
};

/**
 * @brief Thrown when attempting to register an email address that is already registered.
 */
class DuplicateEmailException : public CivicCareException {
private:
    std::string m_email;
public:
    explicit DuplicateEmailException(const std::string& email)
        : CivicCareException("A user with email '" + email + "' already exists."),
          m_email(email) {}

    const std::string& getEmail() const noexcept { return m_email; }
};

/**
 * @brief Thrown when input data fails domain validation rules.
 */
class ValidationException : public CivicCareException {
private:
    std::string m_fieldName;
public:
    explicit ValidationException(const std::string& message, const std::string& fieldName = "")
        : CivicCareException(message), m_fieldName(fieldName) {}

    const std::string& getFieldName() const noexcept { return m_fieldName; }
};

/**
 * @brief Thrown when an authenticated user attempts an action outside their role's permissions.
 */
class UnauthorizedException : public CivicCareException {
private:
    std::string m_requiredPermission;
public:
    explicit UnauthorizedException(const std::string& message, const std::string& permission = "")
        : CivicCareException(message), m_requiredPermission(permission) {}

    const std::string& getRequiredPermission() const noexcept { return m_requiredPermission; }
};

/**
 * @brief Thrown when a session token is expired, invalid, or unrecognized.
 */
class SessionExpiredException : public CivicCareException {
public:
    explicit SessionExpiredException(const std::string& message = "Session has expired or is invalid. Please log in again.")
        : CivicCareException(message) {}
};

/**
 * @brief Thrown when an error occurs during repository persistence (I/O, database constraints, corruption).
 */
class RepositoryException : public CivicCareException {
public:
    explicit RepositoryException(const std::string& message)
        : CivicCareException("Persistence error: " + message) {}
};

} // namespace civiccare

#endif // CIVICCARE_EXCEPTIONS_CIVICCAREEXCEPTIONS_HPP
