#ifndef CIVICCARE_MODELS_ADMINISTRATOR_HPP
#define CIVICCARE_MODELS_ADMINISTRATOR_HPP

#include "models/User.hpp"

namespace civiccare {

/**
 * @brief Represents a System Administrator in CivicCare.
 * 
 * Administrators possess superuser authority:
 * - Register and oversee municipal staff and departments.
 * - View all complaints across all wards.
 * - Access system analytics, reports, database backups, and restore operations.
 */
class Administrator : public User {
private:
    int m_adminLevel; // 1 = Ward Supervisor, 2 = Zonal Admin, 3 = Central Municipal Commissioner
    std::string m_officeLocation;

public:
    /**
     * @brief Default constructor.
     */
    Administrator();

    /**
     * @brief Parameterized constructor.
     */
    Administrator(int id,
                  const std::string& name,
                  const std::string& email,
                  const std::string& passwordHash,
                  const std::string& salt,
                  const std::string& phoneNumber,
                  int adminLevel = 1,
                  const std::string& officeLocation = "Headquarters",
                  const std::string& createdAt = "",
                  const std::string& updatedAt = "");

    ~Administrator() override = default;

    // ==========================================
    // Polymorphic Overrides
    // ==========================================

    UserRole getRole() const override { return UserRole::ADMINISTRATOR; }
    std::string getRoleName() const override { return "Administrator"; }

    void displayProfile(std::ostream& os = std::cout) const override;

    bool hasPermission(Permission permission) const override;

    bool canAccessComplaint(int complaintCitizenId, int assignedStaffId = -1) const override;

    std::unique_ptr<User> clone() const override;

    // ==========================================
    // Admin-Specific Getters & Setters
    // ==========================================

    int getAdminLevel() const noexcept { return m_adminLevel; }
    void setAdminLevel(int level) noexcept { m_adminLevel = level; }

    const std::string& getOfficeLocation() const noexcept { return m_officeLocation; }
    void setOfficeLocation(const std::string& office) { m_officeLocation = office; }
};

} // namespace civiccare

#endif // CIVICCARE_MODELS_ADMINISTRATOR_HPP
