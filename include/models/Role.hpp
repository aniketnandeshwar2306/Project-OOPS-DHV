#ifndef CIVICCARE_MODELS_ROLE_HPP
#define CIVICCARE_MODELS_ROLE_HPP

#include <string>
#include <stdexcept>

namespace civiccare {

/**
 * @brief Enumeration representing the user roles in CivicCare.
 * 
 * Strict role-based categorization:
 * - CITIZEN: Regular citizens who can lodge and track their civic complaints.
 * - MUNICIPAL_STAFF: Ward/Department personnel assigned to inspect and resolve complaints.
 * - ADMINISTRATOR: System supervisors with full authority over users, staff, and reports.
 */
enum class UserRole {
    CITIZEN,
    MUNICIPAL_STAFF,
    ADMINISTRATOR
};

inline std::string roleToString(UserRole role) {
    switch (role) {
        case UserRole::CITIZEN:
            return "CITIZEN";
        case UserRole::MUNICIPAL_STAFF:
            return "MUNICIPAL_STAFF";
        case UserRole::ADMINISTRATOR:
            return "ADMINISTRATOR";
        default:
            return "UNKNOWN";
    }
}

inline UserRole stringToRole(const std::string& str) {
    if (str == "CITIZEN" || str == "Citizen" || str == "citizen") {
        return UserRole::CITIZEN;
    }
    if (str == "MUNICIPAL_STAFF" || str == "MunicipalStaff" || str == "STAFF" || str == "Staff" || str == "staff") {
        return UserRole::MUNICIPAL_STAFF;
    }
    if (str == "ADMINISTRATOR" || str == "Administrator" || str == "ADMIN" || str == "Admin" || str == "admin") {
        return UserRole::ADMINISTRATOR;
    }
    throw std::invalid_argument("Invalid user role string: " + str);
}

} // namespace civiccare

#endif // CIVICCARE_MODELS_ROLE_HPP
