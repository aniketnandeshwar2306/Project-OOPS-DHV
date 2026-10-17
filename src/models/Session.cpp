#include "models/Session.hpp"
#include "exceptions/CivicCareExceptions.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>

namespace civiccare {

Session::Session()
    : m_sessionToken(""),
      m_user(nullptr),
      m_createdAt(std::chrono::system_clock::now()),
      m_expiresAt(std::chrono::system_clock::now()),
      m_active(false) {}

Session::Session(const std::string& token,
                 std::shared_ptr<const User> user,
                 int durationMinutes)
    : m_sessionToken(token),
      m_user(user),
      m_createdAt(std::chrono::system_clock::now()),
      m_expiresAt(m_createdAt + std::chrono::minutes(durationMinutes)),
      m_active(true) {}

bool Session::isValid() const {
    if (!m_active || !m_user) {
        return false;
    }
    return std::chrono::system_clock::now() < m_expiresAt;
}

void Session::invalidate() {
    m_active = false;
}

int Session::getUserId() const {
    if (!m_user) throw SessionExpiredException("No user attached to this session.");
    return m_user->getId();
}

UserRole Session::getUserRole() const {
    if (!m_user) throw SessionExpiredException("No user attached to this session.");
    return m_user->getRole();
}

const std::string& Session::getUserEmail() const {
    if (!m_user) throw SessionExpiredException("No user attached to this session.");
    return m_user->getEmail();
}

const std::string& Session::getUserName() const {
    if (!m_user) throw SessionExpiredException("No user attached to this session.");
    return m_user->getName();
}

bool Session::hasPermission(Permission permission) const {
    if (!isValid()) return false;
    return m_user->hasPermission(permission);
}

bool Session::canAccessComplaint(int complaintCitizenId, int assignedStaffId) const {
    if (!isValid()) return false;
    return m_user->canAccessComplaint(complaintCitizenId, assignedStaffId);
}

std::string Session::getExpirationTimeStr() const {
    std::time_t expTime = std::chrono::system_clock::to_time_t(m_expiresAt);
    std::tm tmBuffer;
    std::tm* tmPtr = std::localtime(&expTime);
    if (tmPtr) {
        tmBuffer = *tmPtr;
    } else {
        std::memset(&tmBuffer, 0, sizeof(tmBuffer));
    }
    std::ostringstream oss;
    oss << std::put_time(&tmBuffer, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace civiccare
