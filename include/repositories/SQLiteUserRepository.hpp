#ifndef CIVICCARE_REPOSITORIES_SQLITEUSERREPOSITORY_HPP
#define CIVICCARE_REPOSITORIES_SQLITEUSERREPOSITORY_HPP

#include "repositories/UserRepository.hpp"
#include <string>
#include <memory>
#include <vector>

// Forward declaration of sqlite3 structures to avoid mandatory sqlite3 header
// if compiling in an environment where Member 4 hasn't installed SQLite yet.
struct sqlite3;

namespace civiccare {

/**
 * @brief SQLite3 implementation of the UserRepository interface.
 * 
 * Implements the user persistence schema coordinated with Member 4:
 * Table: `users`
 * Fields:
 * - id: INTEGER PRIMARY KEY AUTOINCREMENT
 * - role: TEXT NOT NULL ('CITIZEN', 'MUNICIPAL_STAFF', 'ADMINISTRATOR')
 * - name: TEXT NOT NULL
 * - email: TEXT UNIQUE NOT NULL
 * - password_hash: TEXT NOT NULL
 * - salt: TEXT NOT NULL
 * - phone: TEXT
 * - created_at: TEXT NOT NULL
 * - updated_at: TEXT NOT NULL
 * - ward_number: INTEGER DEFAULT 0 (Citizen)
 * - residential_address: TEXT DEFAULT '' (Citizen)
 * - aadhar_or_voter_id: TEXT DEFAULT '' (Citizen)
 * - department_id: INTEGER DEFAULT 0 (Staff)
 * - department_name: TEXT DEFAULT '' (Staff)
 * - employee_id: TEXT DEFAULT '' (Staff)
 * - designation: TEXT DEFAULT '' (Staff)
 * - admin_level: INTEGER DEFAULT 1 (Admin)
 * - office_location: TEXT DEFAULT '' (Admin)
 */
class SQLiteUserRepository : public UserRepository {
private:
    std::string m_dbPath;
    sqlite3* m_dbHandle;

    void initializeSchema();
    void executeQuery(const std::string& sql);

public:
    explicit SQLiteUserRepository(const std::string& dbPath = "civiccare.db");
    ~SQLiteUserRepository() override;

    int save(const std::shared_ptr<User>& user) override;
    bool update(const std::shared_ptr<User>& user) override;
    bool deleteById(int id) override;
    std::shared_ptr<User> findById(int id) override;
    std::shared_ptr<User> findByEmail(const std::string& email) override;
    std::vector<std::shared_ptr<User>> findAll() override;
    std::vector<std::shared_ptr<User>> findByRole(UserRole role) override;
    bool existsByEmail(const std::string& email) override;
    size_t count() override;

    const std::string& getDbPath() const noexcept { return m_dbPath; }
};

} // namespace civiccare

#endif // CIVICCARE_REPOSITORIES_SQLITEUSERREPOSITORY_HPP
