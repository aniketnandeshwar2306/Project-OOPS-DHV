#ifndef CIVICCARE_REPOSITORIES_USERREPOSITORY_HPP
#define CIVICCARE_REPOSITORIES_USERREPOSITORY_HPP

#include "models/User.hpp"
#include "models/Role.hpp"

#include <memory>
#include <vector>
#include <string>

namespace civiccare {

/**
 * @brief Abstract Repository Interface for User persistence.
 * 
 * Agreed contract between Member 2 (User Management) and Member 4 (Database Lead).
 * Isolates business logic from the underlying storage technology (SQLite, Files, etc.).
 */
class UserRepository {
public:
    virtual ~UserRepository() = default;

    /**
     * @brief Persists a new user record.
     * @param user Polymorphic shared pointer to User instance.
     * @return Generated unique integer ID.
     * @throws DuplicateEmailException if a user with the same email already exists.
     * @throws RepositoryException on database / IO errors.
     */
    virtual int save(const std::shared_ptr<User>& user) = 0;

    /**
     * @brief Updates an existing user record.
     * @param user Pointer to User with updated values.
     * @return True if updated, false if user ID not found.
     */
    virtual bool update(const std::shared_ptr<User>& user) = 0;

    /**
     * @brief Deletes a user by unique ID.
     * @param id User ID to remove.
     * @return True if removed, false if not found.
     */
    virtual bool deleteById(int id) = 0;

    /**
     * @brief Finds a user by their unique ID.
     * @param id The ID to look up.
     * @return Polymorphic shared_ptr to User, or nullptr if not found.
     */
    virtual std::shared_ptr<User> findById(int id) = 0;

    /**
     * @brief Finds a user by their unique email address.
     * @param email The email to look up.
     * @return Polymorphic shared_ptr to User, or nullptr if not found.
     */
    virtual std::shared_ptr<User> findByEmail(const std::string& email) = 0;

    /**
     * @brief Retrieves all persisted users.
     */
    virtual std::vector<std::shared_ptr<User>> findAll() = 0;

    /**
     * @brief Retrieves all users belonging to a specific role.
     */
    virtual std::vector<std::shared_ptr<User>> findByRole(UserRole role) = 0;

    /**
     * @brief Quickly checks if an email address is already registered.
     */
    virtual bool existsByEmail(const std::string& email) = 0;

    /**
     * @brief Returns total number of registered users.
     */
    virtual size_t count() = 0;
};

} // namespace civiccare

#endif // CIVICCARE_REPOSITORIES_USERREPOSITORY_HPP
