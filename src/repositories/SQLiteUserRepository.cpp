#include "repositories/SQLiteUserRepository.hpp"
#include "models/Citizen.hpp"
#include "models/MunicipalStaff.hpp"
#include "models/Administrator.hpp"
#include "exceptions/CivicCareExceptions.hpp"

#include <iostream>

#if defined(__has_include)
  #if __has_include(<sqlite3.h>)
    #define CIVICCARE_HAS_SQLITE3 1
    #include <sqlite3.h>
  #endif
#endif

namespace civiccare {

#ifdef CIVICCARE_HAS_SQLITE3

SQLiteUserRepository::SQLiteUserRepository(const std::string& dbPath)
    : m_dbPath(dbPath), m_dbHandle(nullptr) {
    int rc = sqlite3_open(m_dbPath.c_str(), &m_dbHandle);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(m_dbHandle);
        sqlite3_close(m_dbHandle);
        m_dbHandle = nullptr;
        throw RepositoryException("Cannot open SQLite database: " + err);
    }
    initializeSchema();
}

SQLiteUserRepository::~SQLiteUserRepository() {
    if (m_dbHandle) {
        sqlite3_close(m_dbHandle);
        m_dbHandle = nullptr;
    }
}

void SQLiteUserRepository::initializeSchema() {
    const char* schema = 
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  role TEXT NOT NULL,"
        "  name TEXT NOT NULL,"
        "  email TEXT UNIQUE NOT NULL,"
        "  password_hash TEXT NOT NULL,"
        "  salt TEXT NOT NULL,"
        "  phone TEXT,"
        "  created_at TEXT NOT NULL,"
        "  updated_at TEXT NOT NULL,"
        "  ward_number INTEGER DEFAULT 0,"
        "  residential_address TEXT DEFAULT '',"
        "  aadhar_or_voter_id TEXT DEFAULT '',"
        "  department_id INTEGER DEFAULT 0,"
        "  department_name TEXT DEFAULT '',"
        "  employee_id TEXT DEFAULT '',"
        "  designation TEXT DEFAULT '',"
        "  admin_level INTEGER DEFAULT 1,"
        "  office_location TEXT DEFAULT ''"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);"
        "CREATE INDEX IF NOT EXISTS idx_users_role ON users(role);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_dbHandle, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "Unknown schema creation error";
        sqlite3_free(errMsg);
        throw RepositoryException("Failed to initialize SQLite users schema: " + err);
    }
}

int SQLiteUserRepository::save(const std::shared_ptr<User>& user) {
    if (!user) throw ValidationException("Cannot save null user.");

    const char* sql = 
        "INSERT INTO users (role, name, email, password_hash, salt, phone, created_at, updated_at, "
        "ward_number, residential_address, aadhar_or_voter_id, "
        "department_id, department_name, employee_id, designation, "
        "admin_level, office_location) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw RepositoryException(sqlite3_errmsg(m_dbHandle));
    }

    sqlite3_bind_text(stmt, 1, roleToString(user->getRole()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user->getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user->getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user->getSalt().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, user->getPhoneNumber().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, user->getCreatedAt().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, user->getUpdatedAt().c_str(), -1, SQLITE_TRANSIENT);

    if (user->getRole() == UserRole::CITIZEN) {
        auto citizen = std::dynamic_pointer_cast<Citizen>(user);
        sqlite3_bind_int(stmt, 9, citizen ? citizen->getWardNumber() : 0);
        sqlite3_bind_text(stmt, 10, citizen ? citizen->getResidentialAddress().c_str() : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 11, citizen ? citizen->getAadharOrVoterId().c_str() : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_null(stmt, 12);
        sqlite3_bind_null(stmt, 13);
        sqlite3_bind_null(stmt, 14);
        sqlite3_bind_null(stmt, 15);
        sqlite3_bind_null(stmt, 16);
        sqlite3_bind_null(stmt, 17);
    } else if (user->getRole() == UserRole::MUNICIPAL_STAFF) {
        auto staff = std::dynamic_pointer_cast<MunicipalStaff>(user);
        sqlite3_bind_null(stmt, 9);
        sqlite3_bind_null(stmt, 10);
        sqlite3_bind_null(stmt, 11);
        sqlite3_bind_int(stmt, 12, staff ? staff->getDepartmentId() : 0);
        sqlite3_bind_text(stmt, 13, staff ? staff->getDepartmentName().c_str() : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 14, staff ? staff->getEmployeeId().c_str() : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 15, staff ? staff->getDesignation().c_str() : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_null(stmt, 16);
        sqlite3_bind_null(stmt, 17);
    } else if (user->getRole() == UserRole::ADMINISTRATOR) {
        auto admin = std::dynamic_pointer_cast<Administrator>(user);
        sqlite3_bind_null(stmt, 9);
        sqlite3_bind_null(stmt, 10);
        sqlite3_bind_null(stmt, 11);
        sqlite3_bind_null(stmt, 12);
        sqlite3_bind_null(stmt, 13);
        sqlite3_bind_null(stmt, 14);
        sqlite3_bind_null(stmt, 15);
        sqlite3_bind_int(stmt, 16, admin ? admin->getAdminLevel() : 1);
        sqlite3_bind_text(stmt, 17, admin ? admin->getOfficeLocation().c_str() : "", -1, SQLITE_TRANSIENT);
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_CONSTRAINT) {
        sqlite3_finalize(stmt);
        throw DuplicateEmailException(user->getEmail());
    } else if (rc != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(m_dbHandle);
        sqlite3_finalize(stmt);
        throw RepositoryException("Failed to insert user: " + err);
    }

    sqlite3_finalize(stmt);
    int assignedId = static_cast<int>(sqlite3_last_insert_rowid(m_dbHandle));
    user->setId(assignedId);
    return assignedId;
}

bool SQLiteUserRepository::update(const std::shared_ptr<User>& user) {
    if (!user) return false;
    const char* sql = "UPDATE users SET name=?, email=?, password_hash=?, salt=?, phone=?, updated_at=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, user->getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user->getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user->getSalt().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user->getPhoneNumber().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, user->getUpdatedAt().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, user->getId());

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool SQLiteUserRepository::deleteById(int id) {
    const char* sql = "DELETE FROM users WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

static std::shared_ptr<User> parseUserRow(sqlite3_stmt* stmt) {
    int id = sqlite3_column_int(stmt, 0);
    std::string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    std::string salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    const char* phoneRaw = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    std::string phone = phoneRaw ? phoneRaw : "";
    std::string createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    std::string updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

    UserRole role = stringToRole(roleStr);
    if (role == UserRole::CITIZEN) {
        int ward = sqlite3_column_int(stmt, 9);
        const char* addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* civic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        return std::make_shared<Citizen>(id, name, email, hash, salt, phone, ward,
                                         addr ? addr : "", civic ? civic : "", createdAt, updatedAt);
    } else if (role == UserRole::MUNICIPAL_STAFF) {
        int deptId = sqlite3_column_int(stmt, 12);
        const char* dName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        const char* empId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
        const char* desig = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));
        return std::make_shared<MunicipalStaff>(id, name, email, hash, salt, phone, deptId,
                                                dName ? dName : "", empId ? empId : "", desig ? desig : "",
                                                createdAt, updatedAt);
    } else if (role == UserRole::ADMINISTRATOR) {
        int lvl = sqlite3_column_int(stmt, 16);
        const char* off = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
        return std::make_shared<Administrator>(id, name, email, hash, salt, phone, lvl,
                                               off ? off : "Headquarters", createdAt, updatedAt);
    }
    return nullptr;
}

std::shared_ptr<User> SQLiteUserRepository::findById(int id) {
    const char* sql = "SELECT id, role, name, email, password_hash, salt, phone, created_at, updated_at, "
                      "ward_number, residential_address, aadhar_or_voter_id, "
                      "department_id, department_name, employee_id, designation, "
                      "admin_level, office_location FROM users WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return nullptr;
    sqlite3_bind_int(stmt, 1, id);

    std::shared_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = parseUserRow(stmt);
    }
    sqlite3_finalize(stmt);
    return user;
}

std::shared_ptr<User> SQLiteUserRepository::findByEmail(const std::string& email) {
    const char* sql = "SELECT id, role, name, email, password_hash, salt, phone, created_at, updated_at, "
                      "ward_number, residential_address, aadhar_or_voter_id, "
                      "department_id, department_name, employee_id, designation, "
                      "admin_level, office_location FROM users WHERE email=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return nullptr;
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    std::shared_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = parseUserRow(stmt);
    }
    sqlite3_finalize(stmt);
    return user;
}

std::vector<std::shared_ptr<User>> SQLiteUserRepository::findAll() {
    std::vector<std::shared_ptr<User>> list;
    const char* sql = "SELECT id, role, name, email, password_hash, salt, phone, created_at, updated_at, "
                      "ward_number, residential_address, aadhar_or_voter_id, "
                      "department_id, department_name, employee_id, designation, "
                      "admin_level, office_location FROM users ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto u = parseUserRow(stmt);
        if (u) list.push_back(u);
    }
    sqlite3_finalize(stmt);
    return list;
}

std::vector<std::shared_ptr<User>> SQLiteUserRepository::findByRole(UserRole role) {
    std::vector<std::shared_ptr<User>> list;
    const char* sql = "SELECT id, role, name, email, password_hash, salt, phone, created_at, updated_at, "
                      "ward_number, residential_address, aadhar_or_voter_id, "
                      "department_id, department_name, employee_id, designation, "
                      "admin_level, office_location FROM users WHERE role=? ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    sqlite3_bind_text(stmt, 1, roleToString(role).c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto u = parseUserRow(stmt);
        if (u) list.push_back(u);
    }
    sqlite3_finalize(stmt);
    return list;
}

bool SQLiteUserRepository::existsByEmail(const std::string& email) {
    return findByEmail(email) != nullptr;
}

size_t SQLiteUserRepository::count() {
    const char* sql = "SELECT COUNT(*) FROM users;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
    size_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return count;
}

#else

// Fallback stub when compiled without SQLite3 headers/library
SQLiteUserRepository::SQLiteUserRepository(const std::string& dbPath)
    : m_dbPath(dbPath), m_dbHandle(nullptr) {}

SQLiteUserRepository::~SQLiteUserRepository() {}

int SQLiteUserRepository::save(const std::shared_ptr<User>& /* user */) {
    throw RepositoryException("SQLite3 is not linked. Compile with -lsqlite3 to enable SQLiteUserRepository.");
}
bool SQLiteUserRepository::update(const std::shared_ptr<User>& /* user */) { return false; }
bool SQLiteUserRepository::deleteById(int /* id */) { return false; }
std::shared_ptr<User> SQLiteUserRepository::findById(int /* id */) { return nullptr; }
std::shared_ptr<User> SQLiteUserRepository::findByEmail(const std::string& /* email */) { return nullptr; }
std::vector<std::shared_ptr<User>> SQLiteUserRepository::findAll() { return {}; }
std::vector<std::shared_ptr<User>> SQLiteUserRepository::findByRole(UserRole /* role */) { return {}; }
bool SQLiteUserRepository::existsByEmail(const std::string& /* email */) { return false; }
size_t SQLiteUserRepository::count() { return 0; }

#endif

} // namespace civiccare
