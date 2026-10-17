#ifndef CIVICCARE_MODELS_SESSION_HPP
#define CIVICCARE_MODELS_SESSION_HPP

#include "models/User.hpp"

#include <string>
#include <memory>
#include <chrono>

namespace civiccare {

/**
 * @brief Represents an authenticated user session in CivicCare.
 * Tracks session token, active user identity, and session expiration.
 */
class Session {
private:
    std::string m_sessionToken;
    std::shared_ptr<const User> m_user;
    std::chrono::system_clock::time_point m_createdAt;
    std::chrono::system_clock::time_point m_expiresAt;
    bool m_active;

public:
    /**
     * @brief Default constructor creating an empty/invalid session.
     */
    Session();

    /**
     * @brief Constructs an active session for an authenticated user.
     * @param token Unique session token.
     * @param user Pointer to authenticated User.
     * @param durationMinutes Session lifetime in minutes (default: 60 minutes).
     */
    Session(const std::string& token,
            std::shared_ptr<const User> user,
            int durationMinutes = 60);

    /**
     * @brief Checks if the session is active and not expired.
     */
    bool isValid() const;

    /**
     * @brief Explicitly invalidates / logs out this session.
     */
    void invalidate();

    const std::string& getToken() const noexcept { return m_sessionToken; }
    std::shared_ptr<const User> getUser() const noexcept { return m_user; }

    int getUserId() const;
    UserRole getUserRole() const;
    const std::string& getUserEmail() const;
    const std::string& getUserName() const;

    /**
     * @brief Delegates permission check to the underlying user.
     */
    bool hasPermission(Permission permission) const;

    /**
     * @brief Delegates complaint access verification to the underlying user.
     */
    bool canAccessComplaint(int complaintCitizenId, int assignedStaffId = -1) const;

    std::string getExpirationTimeStr() const;
};

} // namespace civiccare

#endif // CIVICCARE_MODELS_SESSION_HPP
