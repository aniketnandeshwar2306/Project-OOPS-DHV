#ifndef CIVICCARE_MODELS_PERMISSION_HPP
#define CIVICCARE_MODELS_PERMISSION_HPP

#include <string>

namespace civiccare {

/**
 * @brief Discrete permissions in CivicCare for Role-Based Access Control (RBAC).
 * Enforced at the service layer, preventing unauthorized operations.
 */
enum class Permission {
    // Citizen Permissions
    LODGE_COMPLAINT,
    VIEW_OWN_COMPLAINTS,
    REOPEN_OWN_COMPLAINT,
    CANCEL_OWN_COMPLAINT,

    // Staff Permissions
    VIEW_ASSIGNED_COMPLAINTS,
    UPDATE_COMPLAINT_STATUS,
    ADD_RESOLUTION_NOTES,
    REASSIGN_COMPLAINT,

    // Administrator Permissions
    MANAGE_STAFF,
    MANAGE_DEPARTMENTS,
    VIEW_ALL_COMPLAINTS,
    VIEW_ANALYTICS,
    SYSTEM_BACKUP,
    SYSTEM_RESTORE,
    MANAGE_USERS
};

inline std::string permissionToString(Permission permission) {
    switch (permission) {
        case Permission::LODGE_COMPLAINT:
            return "LODGE_COMPLAINT";
        case Permission::VIEW_OWN_COMPLAINTS:
            return "VIEW_OWN_COMPLAINTS";
        case Permission::REOPEN_OWN_COMPLAINT:
            return "REOPEN_OWN_COMPLAINT";
        case Permission::CANCEL_OWN_COMPLAINT:
            return "CANCEL_OWN_COMPLAINT";
        case Permission::VIEW_ASSIGNED_COMPLAINTS:
            return "VIEW_ASSIGNED_COMPLAINTS";
        case Permission::UPDATE_COMPLAINT_STATUS:
            return "UPDATE_COMPLAINT_STATUS";
        case Permission::ADD_RESOLUTION_NOTES:
            return "ADD_RESOLUTION_NOTES";
        case Permission::REASSIGN_COMPLAINT:
            return "REASSIGN_COMPLAINT";
        case Permission::MANAGE_STAFF:
            return "MANAGE_STAFF";
        case Permission::MANAGE_DEPARTMENTS:
            return "MANAGE_DEPARTMENTS";
        case Permission::VIEW_ALL_COMPLAINTS:
            return "VIEW_ALL_COMPLAINTS";
        case Permission::VIEW_ANALYTICS:
            return "VIEW_ANALYTICS";
        case Permission::SYSTEM_BACKUP:
            return "SYSTEM_BACKUP";
        case Permission::SYSTEM_RESTORE:
            return "SYSTEM_RESTORE";
        case Permission::MANAGE_USERS:
            return "MANAGE_USERS";
        default:
            return "UNKNOWN_PERMISSION";
    }
}

} // namespace civiccare

#endif // CIVICCARE_MODELS_PERMISSION_HPP
