#include "models/Administrator.hpp"

namespace civiccare {

Administrator::Administrator()
    : User(),
      m_adminLevel(1),
      m_officeLocation("Headquarters") {}

Administrator::Administrator(int id,
                             const std::string& name,
                             const std::string& email,
                             const std::string& passwordHash,
                             const std::string& salt,
                             const std::string& phoneNumber,
                             int adminLevel,
                             const std::string& officeLocation,
                             const std::string& createdAt,
                             const std::string& updatedAt)
    : User(id, name, email, passwordHash, salt, phoneNumber, createdAt, updatedAt),
      m_adminLevel(adminLevel),
      m_officeLocation(officeLocation) {}

void Administrator::displayProfile(std::ostream& os) const {
    os << "============== ADMINISTRATOR PROFILE =============\n"
       << " Admin ID      : " << getId() << "\n"
       << " Name          : " << getName() << "\n"
       << " Email         : " << getEmail() << "\n"
       << " Role          : " << getRoleName() << "\n"
       << " Phone         : " << getPhoneNumber() << "\n"
       << " Admin Level   : Tier " << m_adminLevel << "\n"
       << " Office        : " << m_officeLocation << "\n"
       << " Member Since  : " << getCreatedAt() << "\n"
       << "=================================================";
}

bool Administrator::hasPermission(Permission /* permission */) const {
    // Administrator has all permissions across the system.
    return true;
}

bool Administrator::canAccessComplaint(int /* complaintCitizenId */, int /* assignedStaffId */) const {
    // Administrator has authority to view, inspect, and manage all complaints.
    return true;
}

std::unique_ptr<User> Administrator::clone() const {
    return std::unique_ptr<User>(new Administrator(*this));
}

} // namespace civiccare
