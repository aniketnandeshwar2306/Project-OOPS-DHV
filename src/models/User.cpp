#include "models/User.hpp"
#include "utils/DateTimeUtils.hpp"

namespace civiccare {

User::User()
    : m_id(0),
      m_name(""),
      m_email(""),
      m_passwordHash(""),
      m_salt(""),
      m_phoneNumber(""),
      m_createdAt(""),
      m_updatedAt("") {}

User::User(int id,
           const std::string& name,
           const std::string& email,
           const std::string& passwordHash,
           const std::string& salt,
           const std::string& phoneNumber,
           const std::string& createdAt,
           const std::string& updatedAt)
    : m_id(id),
      m_name(name),
      m_email(email),
      m_passwordHash(passwordHash),
      m_salt(salt),
      m_phoneNumber(phoneNumber),
      m_createdAt(createdAt.empty() ? DateTimeUtils::getCurrentTimestamp() : createdAt),
      m_updatedAt(updatedAt.empty() ? m_createdAt : updatedAt) {}

bool User::verifyPassword(const std::string& plaintextPassword) const {
    if (m_passwordHash.empty() || m_salt.empty()) {
        return false;
    }
    return PasswordHasher::verifyPassword(plaintextPassword, m_salt, m_passwordHash);
}

void User::updatePassword(const std::string& newPlaintextPassword) {
    m_salt = PasswordHasher::generateSalt();
    m_passwordHash = PasswordHasher::hashPassword(newPlaintextPassword, m_salt);
    m_updatedAt = DateTimeUtils::getCurrentTimestamp();
}

std::ostream& operator<<(std::ostream& os, const User& user) {
    user.displayProfile(os);
    return os;
}

} // namespace civiccare
