#ifndef CIVICCARE_MODELS_USER_HPP
#define CIVICCARE_MODELS_USER_HPP

#include "models/Role.hpp"
#include "models/Permission.hpp"
#include "utils/PasswordHasher.hpp"

#include <string>
#include <memory>
#include <iostream>

namespace civiccare {

/**
 * @brief Abstract Base Class representing a User in the CivicCare system.
 * 
 * Demonstrates:
 * - Abstraction: Exposes high-level contracts while hiding implementation details.
 * - Encapsulation: Private member variables with strictly validated getters/setters.
 * - Polymorphism: Pure virtual methods for role resolution, permission validation,
 *   complaint authorization, and formatted profile inspection.
 */
class User {
private:
    int m_id;
    std::string m_name;
    std::string m_email;
    std::string m_passwordHash;
    std::string m_salt;
    std::string m_phoneNumber;
    std::string m_createdAt;
    std::string m_updatedAt;

public:
    /**
     * @brief Default constructor for deserialization and ORM purposes.
     */
    User();

    /**
     * @brief Parameterized constructor for creating a new user instance.
     * @param id Unique user ID (0 for unregistered/unpersisted records).
     * @param name Full name of the user.
     * @param email Unique email address (login credential).
     * @param passwordHash Hexadecimal SHA-256 hash of (salt + password).
     * @param salt Unique cryptographic salt.
     * @param phoneNumber Contact telephone number.
     * @param createdAt Creation timestamp (ISO format).
     * @param updatedAt Last update timestamp (ISO format).
     */
    User(int id,
         const std::string& name,
         const std::string& email,
         const std::string& passwordHash,
         const std::string& salt,
         const std::string& phoneNumber,
         const std::string& createdAt = "",
         const std::string& updatedAt = "");

    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction.
     */
    virtual ~User() = default;

    // ==========================================
    // Pure Virtual Interface (Polymorphism)
    // ==========================================

    /**
     * @brief Returns the discrete UserRole enum value.
     */
    virtual UserRole getRole() const = 0;

    /**
     * @brief Returns a human-readable role name (e.g. "Citizen", "Administrator").
     */
    virtual std::string getRoleName() const = 0;

    /**
     * @brief Polymorphically outputs the detailed user profile to an output stream.
     */
    virtual void displayProfile(std::ostream& os = std::cout) const = 0;

    /**
     * @brief Evaluates whether this user has a specific RBAC permission.
     */
    virtual bool hasPermission(Permission permission) const = 0;

    /**
     * @brief Verifies if this user has access rights to view or process a specific complaint.
     * @param complaintCitizenId ID of the citizen who registered the complaint.
     * @param assignedStaffId ID of the municipal staff member assigned to the complaint (-1 if unassigned).
     */
    virtual bool canAccessComplaint(int complaintCitizenId, int assignedStaffId = -1) const = 0;

    /**
     * @brief Virtual copy constructor (Prototype pattern) for deep copying polymorphic objects.
     */
    virtual std::unique_ptr<User> clone() const = 0;

    // ==========================================
    // Concrete Domain Methods
    // ==========================================

    /**
     * @brief Verifies a plaintext password candidate against this user's stored salt and hash.
     * @param plaintextPassword The password to test.
     * @return True if matches, false otherwise.
     */
    bool verifyPassword(const std::string& plaintextPassword) const;

    /**
     * @brief Updates the user's password with a newly generated salt and SHA-256 hash.
     * @param newPlaintextPassword The new password to set.
     */
    void updatePassword(const std::string& newPlaintextPassword);

    // ==========================================
    // Encapsulated Getters & Setters
    // ==========================================

    int getId() const noexcept { return m_id; }
    void setId(int id) noexcept { m_id = id; }

    const std::string& getName() const noexcept { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    const std::string& getEmail() const noexcept { return m_email; }
    void setEmail(const std::string& email) { m_email = email; }

    const std::string& getPasswordHash() const noexcept { return m_passwordHash; }
    void setPasswordHash(const std::string& hash) { m_passwordHash = hash; }

    const std::string& getSalt() const noexcept { return m_salt; }
    void setSalt(const std::string& salt) { m_salt = salt; }

    const std::string& getPhoneNumber() const noexcept { return m_phoneNumber; }
    void setPhoneNumber(const std::string& phone) { m_phoneNumber = phone; }

    const std::string& getCreatedAt() const noexcept { return m_createdAt; }
    void setCreatedAt(const std::string& timestamp) { m_createdAt = timestamp; }

    const std::string& getUpdatedAt() const noexcept { return m_updatedAt; }
    void setUpdatedAt(const std::string& timestamp) { m_updatedAt = timestamp; }

    /**
     * @brief Overloaded stream insertion operator for polymorphically printing users.
     */
    friend std::ostream& operator<<(std::ostream& os, const User& user);
};

} // namespace civiccare

#endif // CIVICCARE_MODELS_USER_HPP
