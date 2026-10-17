#ifndef CIVICCARE_MODELS_CITIZEN_HPP
#define CIVICCARE_MODELS_CITIZEN_HPP

#include "models/User.hpp"

namespace civiccare {

/**
 * @brief Represents a Citizen user in CivicCare.
 * 
 * Citizens can lodge complaints, track progress of their own complaints,
 * and reopen or cancel their own complaints. Citizens CANNOT view other citizens'
 * complaints or perform administrative/staff actions.
 */
class Citizen : public User {
private:
    int m_wardNumber;
    std::string m_residentialAddress;
    std::string m_aadharOrVoterId;

public:
    /**
     * @brief Default constructor.
     */
    Citizen();

    /**
     * @brief Parameterized constructor.
     */
    Citizen(int id,
            const std::string& name,
            const std::string& email,
            const std::string& passwordHash,
            const std::string& salt,
            const std::string& phoneNumber,
            int wardNumber,
            const std::string& residentialAddress = "",
            const std::string& aadharOrVoterId = "",
            const std::string& createdAt = "",
            const std::string& updatedAt = "");

    ~Citizen() override = default;

    // ==========================================
    // Polymorphic Overrides
    // ==========================================

    UserRole getRole() const override { return UserRole::CITIZEN; }
    std::string getRoleName() const override { return "Citizen"; }

    void displayProfile(std::ostream& os = std::cout) const override;

    bool hasPermission(Permission permission) const override;

    bool canAccessComplaint(int complaintCitizenId, int assignedStaffId = -1) const override;

    std::unique_ptr<User> clone() const override;

    // ==========================================
    // Citizen-Specific Getters & Setters
    // ==========================================

    int getWardNumber() const noexcept { return m_wardNumber; }
    void setWardNumber(int ward) noexcept { m_wardNumber = ward; }

    const std::string& getResidentialAddress() const noexcept { return m_residentialAddress; }
    void setResidentialAddress(const std::string& address) { m_residentialAddress = address; }

    const std::string& getAadharOrVoterId() const noexcept { return m_aadharOrVoterId; }
    void setAadharOrVoterId(const std::string& id) { m_aadharOrVoterId = id; }
};

} // namespace civiccare

#endif // CIVICCARE_MODELS_CITIZEN_HPP
