#ifndef CIVICCARE_REPOSITORIES_FILEUSERREPOSITORY_HPP
#define CIVICCARE_REPOSITORIES_FILEUSERREPOSITORY_HPP

#include "repositories/UserRepository.hpp"
#include <string>
#include <vector>
#include <memory>

namespace civiccare {

/**
 * @brief File-stream based implementation of UserRepository.
 * 
 * Uses C++ file streams (std::ifstream, std::ofstream) to persist polymorphic
 * user records across application restarts.
 * Ensures data durability, auto-incrementing ID generation, and unique email constraints.
 */
class FileUserRepository : public UserRepository {
private:
    std::string m_filePath;
    int m_nextId;

    void ensureFileExists();
    std::vector<std::shared_ptr<User>> loadAllFromFile();
    void writeAllToFile(const std::vector<std::shared_ptr<User>>& users);

    std::string serializeUser(const std::shared_ptr<User>& user) const;
    std::shared_ptr<User> deserializeUser(const std::string& line) const;

public:
    /**
     * @brief Constructor initializing file storage at the specified path.
     * @param filePath Path to the storage file (e.g. "data/users.csv" or "users_store.dat").
     */
    explicit FileUserRepository(const std::string& filePath = "civiccare_users.dat");

    ~FileUserRepository() override = default;

    int save(const std::shared_ptr<User>& user) override;
    bool update(const std::shared_ptr<User>& user) override;
    bool deleteById(int id) override;
    std::shared_ptr<User> findById(int id) override;
    std::shared_ptr<User> findByEmail(const std::string& email) override;
    std::vector<std::shared_ptr<User>> findAll() override;
    std::vector<std::shared_ptr<User>> findByRole(UserRole role) override;
    bool existsByEmail(const std::string& email) override;
    size_t count() override;

    const std::string& getFilePath() const noexcept { return m_filePath; }
};

} // namespace civiccare

#endif // CIVICCARE_REPOSITORIES_FILEUSERREPOSITORY_HPP
