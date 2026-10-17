#ifndef CIVICCARE_MODELS_MUNICIPALSTAFF_HPP
#define CIVICCARE_MODELS_MUNICIPALSTAFF_HPP

#include "models/User.hpp"

namespace civiccare {

/**
 * @brief Represents a Municipal Staff member in CivicCare.
 * 
 * Municipal staff are assigned to departments (e.g. Roads, Water, Sanitation)
 * and can view and resolve complaints assigned directly to them.
 */
class MunicipalStaff : public User {
private:
    int m_departmentId;
    std::string m_departmentName;
    std::string m_employeeId;
    std::string m_designation;

public:
    /**
     * @brief Default constructor.
     */
    MunicipalStaff();

    /**
     * @brief Parameterized constructor.
     */
    MunicipalStaff(int id,
                   const std::string& name,
                   const std::string& email,
                   const std::string& passwordHash,
                   const std::string& salt,
                   const std::string& phoneNumber,
                   int departmentId,
                   const std::string& departmentName,
                   const std::string& employeeId,
                   const std::string& designation,
                   const std::string& createdAt = "",
                   const std::string& updatedAt = "");

    ~MunicipalStaff() override = default;

    // ==========================================
    // Polymorphic Overrides
    // ==========================================

    UserRole getRole() const override { return UserRole::MUNICIPAL_STAFF; }
    std::string getRoleName() const override { return "Municipal Staff"; }

    void displayProfile(std::ostream& os = std::cout) const override;

    bool hasPermission(Permission permission) const override;

    bool canAccessComplaint(int complaintCitizenId, int assignedStaffId = -1) const override;

    std::unique_ptr<User> clone() const override;

    // ==========================================
    // Staff-Specific Getters & Setters
    // ==========================================

    int getDepartmentId() const noexcept { return m_departmentId; }
    void setDepartmentId(int id) noexcept { m_departmentId = id; }

    const std::string& getDepartmentName() const noexcept { return m_departmentName; }
    void setDepartmentName(const std::string& name) { m_departmentName = name; }

    const std::string& getEmployeeId() const noexcept { return m_employeeId; }
    void setEmployeeId(const std::string& empId) { m_employeeId = empId; }

    const std::string& getDesignation() const noexcept { return m_designation; }
    void setDesignation(const std::string& designation) { m_designation = designation; }
};

} // namespace civiccare

#endif // CIVICCARE_MODELS_MUNICIPALSTAFF_HPP
