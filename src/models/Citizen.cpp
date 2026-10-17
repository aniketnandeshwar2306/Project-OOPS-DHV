#include "models/Citizen.hpp"

namespace civiccare {

Citizen::Citizen()
    : User(),
      m_wardNumber(0),
      m_residentialAddress(""),
      m_aadharOrVoterId("") {}

Citizen::Citizen(int id,
                 const std::string& name,
                 const std::string& email,
                 const std::string& passwordHash,
                 const std::string& salt,
                 const std::string& phoneNumber,
                 int wardNumber,
                 const std::string& residentialAddress,
                 const std::string& aadharOrVoterId,
                 const std::string& createdAt,
                 const std::string& updatedAt)
    : User(id, name, email, passwordHash, salt, phoneNumber, createdAt, updatedAt),
      m_wardNumber(wardNumber),
      m_residentialAddress(residentialAddress),
      m_aadharOrVoterId(aadharOrVoterId) {}

void Citizen::displayProfile(std::ostream& os) const {
    os << "================ CITIZEN PROFILE ================\n"
       << " User ID       : " << getId() << "\n"
       << " Name          : " << getName() << "\n"
       << " Email         : " << getEmail() << "\n"
       << " Role          : " << getRoleName() << "\n"
       << " Phone         : " << getPhoneNumber() << "\n"
       << " Ward Number   : " << m_wardNumber << "\n"
       << " Address       : " << (m_residentialAddress.empty() ? "N/A" : m_residentialAddress) << "\n"
       << " Civic ID      : " << (m_aadharOrVoterId.empty() ? "N/A" : m_aadharOrVoterId) << "\n"
       << " Member Since  : " << getCreatedAt() << "\n"
       << "=================================================";
}

bool Citizen::hasPermission(Permission permission) const {
    switch (permission) {
        case Permission::LODGE_COMPLAINT:
        case Permission::VIEW_OWN_COMPLAINTS:
        case Permission::REOPEN_OWN_COMPLAINT:
        case Permission::CANCEL_OWN_COMPLAINT:
            return true;
        default:
            return false;
    }
}

bool Citizen::canAccessComplaint(int complaintCitizenId, int /* assignedStaffId */) const {
    // A citizen can ONLY access complaints they personally registered.
    return (complaintCitizenId == getId());
}

std::unique_ptr<User> Citizen::clone() const {
    return std::unique_ptr<User>(new Citizen(*this));
}

} // namespace civiccare
