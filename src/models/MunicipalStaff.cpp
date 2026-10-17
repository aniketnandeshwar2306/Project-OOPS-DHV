#include "models/MunicipalStaff.hpp"

namespace civiccare {

MunicipalStaff::MunicipalStaff()
    : User(),
      m_departmentId(0),
      m_departmentName(""),
      m_employeeId(""),
      m_designation("") {}

MunicipalStaff::MunicipalStaff(int id,
                               const std::string& name,
                               const std::string& email,
                               const std::string& passwordHash,
                               const std::string& salt,
                               const std::string& phoneNumber,
                               int departmentId,
                               const std::string& departmentName,
                               const std::string& employeeId,
                               const std::string& designation,
                               const std::string& createdAt,
                               const std::string& updatedAt)
    : User(id, name, email, passwordHash, salt, phoneNumber, createdAt, updatedAt),
      m_departmentId(departmentId),
      m_departmentName(departmentName),
      m_employeeId(employeeId),
      m_designation(designation) {}

void MunicipalStaff::displayProfile(std::ostream& os) const {
    os << "============= MUNICIPAL STAFF PROFILE ============\n"
       << " Staff ID      : " << getId() << "\n"
       << " Name          : " << getName() << "\n"
       << " Email         : " << getEmail() << "\n"
       << " Role          : " << getRoleName() << "\n"
       << " Phone         : " << getPhoneNumber() << "\n"
       << " Employee ID   : " << m_employeeId << "\n"
       << " Department    : " << m_departmentName << " (ID: " << m_departmentId << ")\n"
       << " Designation   : " << m_designation << "\n"
       << " Member Since  : " << getCreatedAt() << "\n"
       << "=================================================";
}

bool MunicipalStaff::hasPermission(Permission permission) const {
    switch (permission) {
        case Permission::VIEW_ASSIGNED_COMPLAINTS:
        case Permission::UPDATE_COMPLAINT_STATUS:
        case Permission::ADD_RESOLUTION_NOTES:
        case Permission::REASSIGN_COMPLAINT:
            return true;
        default:
            return false;
    }
}

bool MunicipalStaff::canAccessComplaint(int /* complaintCitizenId */, int assignedStaffId) const {
    // Staff can access a complaint if they are the designated staff assigned to it.
    return (assignedStaffId == getId());
}

std::unique_ptr<User> MunicipalStaff::clone() const {
    return std::unique_ptr<User>(new MunicipalStaff(*this));
}

} // namespace civiccare
