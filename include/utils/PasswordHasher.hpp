#ifndef CIVICCARE_UTILS_PASSWORDHASHER_HPP
#define CIVICCARE_UTILS_PASSWORDHASHER_HPP

#include <string>

namespace civiccare {

/**
 * @brief Utility class for secure password hashing and salt generation.
 * 
 * Uses SHA-256 with cryptographically random per-user salt.
 * Ensures passwords are never stored or compared as plaintext.
 */
class PasswordHasher {
public:
    /**
     * @brief Generates a cryptographically random hexadecimal salt.
     * @param length Number of random bytes (default 16 bytes = 32 hex chars).
     */
    static std::string generateSalt(size_t length = 16);

    /**
     * @brief Computes SHA-256 hash of (salt + password).
     * @param password Plaintext password.
     * @param salt Unique per-user salt string.
     * @return 64-character lowercase hexadecimal hash.
     */
    static std::string hashPassword(const std::string& password, const std::string& salt);

    /**
     * @brief Verifies a plaintext password against stored salt and expected hash.
     * @param password Plaintext candidate password.
     * @param salt Stored user salt.
     * @param expectedHash Stored hash to compare against.
     * @return True if matches, false otherwise.
     */
    static bool verifyPassword(const std::string& password, 
                              const std::string& salt, 
                              const std::string& expectedHash);

    /**
     * @brief Computes raw SHA-256 hash of an input string.
     */
    static std::string sha256(const std::string& input);
};

} // namespace civiccare

#endif // CIVICCARE_UTILS_PASSWORDHASHER_HPP
