#include "repositories/FileUserRepository.hpp"
#include "models/Citizen.hpp"
#include "models/MunicipalStaff.hpp"
#include "models/Administrator.hpp"
#include "exceptions/CivicCareExceptions.hpp"
#include "utils/ValidationUtils.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace civiccare {

namespace {

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string escape(const std::string& str) {
    std::string res;
    for (char c : str) {
        if (c == '|') res += "\\|";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else res += c;
    }
    return res;
}

std::string unescape(const std::string& str) {
    std::string res;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            if (str[i + 1] == '|') { res += '|'; ++i; }
            else if (str[i + 1] == '\\') { res += '\\'; ++i; }
            else if (str[i + 1] == 'n') { res += '\n'; ++i; }
            else { res += str[i]; }
        } else {
            res += str[i];
        }
    }
    return res;
}

} // anonymous namespace

FileUserRepository::FileUserRepository(const std::string& filePath)
    : m_filePath(filePath), m_nextId(1) {
    ensureFileExists();
    // Scan highest existing ID
    auto users = loadAllFromFile();
    for (const auto& u : users) {
        if (u->getId() >= m_nextId) {
            m_nextId = u->getId() + 1;
        }
    }
}

void FileUserRepository::ensureFileExists() {
    std::ifstream infile(m_filePath);
    if (!infile.good()) {
        std::ofstream outfile(m_filePath);
        if (!outfile.good()) {
            throw RepositoryException("Unable to create user storage file: " + m_filePath);
        }
        // Write header
        outfile << "# CIVICCARE USER STORAGE V1.0\n";
    }
}

std::string FileUserRepository::serializeUser(const std::shared_ptr<User>& user) const {
    std::ostringstream oss;
    oss << user->getId() << "|"
        << roleToString(user->getRole()) << "|"
        << escape(user->getName()) << "|"
        << escape(user->getEmail()) << "|"
        << user->getPasswordHash() << "|"
        << user->getSalt() << "|"
        << escape(user->getPhoneNumber()) << "|"
        << user->getCreatedAt() << "|"
        << user->getUpdatedAt() << "|";

    if (user->getRole() == UserRole::CITIZEN) {
        auto citizen = std::dynamic_pointer_cast<Citizen>(user);
        if (citizen) {
            oss << citizen->getWardNumber() << "|"
                << escape(citizen->getResidentialAddress()) << "|"
                << escape(citizen->getAadharOrVoterId());
        }
    } else if (user->getRole() == UserRole::MUNICIPAL_STAFF) {
        auto staff = std::dynamic_pointer_cast<MunicipalStaff>(user);
        if (staff) {
            oss << staff->getDepartmentId() << "|"
                << escape(staff->getDepartmentName()) << "|"
                << escape(staff->getEmployeeId()) << "|"
                << escape(staff->getDesignation());
        }
    } else if (user->getRole() == UserRole::ADMINISTRATOR) {
        auto admin = std::dynamic_pointer_cast<Administrator>(user);
        if (admin) {
            oss << admin->getAdminLevel() << "|"
                << escape(admin->getOfficeLocation());
        }
    }
    return oss.str();
}

std::shared_ptr<User> FileUserRepository::deserializeUser(const std::string& line) const {
    if (line.empty() || line[0] == '#') {
        return nullptr;
    }

    auto tokens = split(line, '|');
    if (tokens.size() < 9) {
        return nullptr;
    }

    try {
        int id = std::stoi(tokens[0]);
        UserRole role = stringToRole(tokens[1]);
        std::string name = unescape(tokens[2]);
        std::string email = unescape(tokens[3]);
        std::string passwordHash = tokens[4];
        std::string salt = tokens[5];
        std::string phone = unescape(tokens[6]);
        std::string createdAt = tokens[7];
        std::string updatedAt = tokens[8];

        if (role == UserRole::CITIZEN) {
            int wardNumber = (tokens.size() > 9 && !tokens[9].empty()) ? std::stoi(tokens[9]) : 0;
            std::string address = (tokens.size() > 10) ? unescape(tokens[10]) : "";
            std::string civicId = (tokens.size() > 11) ? unescape(tokens[11]) : "";
            return std::make_shared<Citizen>(id, name, email, passwordHash, salt, phone, 
                                             wardNumber, address, civicId, createdAt, updatedAt);
        } else if (role == UserRole::MUNICIPAL_STAFF) {
            int deptId = (tokens.size() > 9 && !tokens[9].empty()) ? std::stoi(tokens[9]) : 0;
            std::string deptName = (tokens.size() > 10) ? unescape(tokens[10]) : "";
            std::string empId = (tokens.size() > 11) ? unescape(tokens[11]) : "";
            std::string designation = (tokens.size() > 12) ? unescape(tokens[12]) : "";
            return std::make_shared<MunicipalStaff>(id, name, email, passwordHash, salt, phone,
                                                    deptId, deptName, empId, designation, createdAt, updatedAt);
        } else if (role == UserRole::ADMINISTRATOR) {
            int adminLevel = (tokens.size() > 9 && !tokens[9].empty()) ? std::stoi(tokens[9]) : 1;
            std::string office = (tokens.size() > 10) ? unescape(tokens[10]) : "Headquarters";
            return std::make_shared<Administrator>(id, name, email, passwordHash, salt, phone,
                                                   adminLevel, office, createdAt, updatedAt);
        }
    } catch (...) {
        return nullptr;
    }
    return nullptr;
}

std::vector<std::shared_ptr<User>> FileUserRepository::loadAllFromFile() {
    std::vector<std::shared_ptr<User>> users;
    std::ifstream infile(m_filePath);
    if (!infile.is_open()) {
        throw RepositoryException("Failed to open storage file: " + m_filePath);
    }

    std::string line;
    while (std::getline(infile, line)) {
        auto user = deserializeUser(line);
        if (user) {
            users.push_back(user);
        }
    }
    return users;
}

void FileUserRepository::writeAllToFile(const std::vector<std::shared_ptr<User>>& users) {
    std::ofstream outfile(m_filePath, std::ios::trunc);
    if (!outfile.is_open()) {
        throw RepositoryException("Failed to write to storage file: " + m_filePath);
    }

    outfile << "# CIVICCARE USER STORAGE V1.0\n";
    for (const auto& u : users) {
        outfile << serializeUser(u) << "\n";
    }
    outfile.flush();
}

int FileUserRepository::save(const std::shared_ptr<User>& user) {
    if (!user) {
        throw ValidationException("Cannot save null user pointer.");
    }

    auto users = loadAllFromFile();

    // Check duplicate email
    for (const auto& existing : users) {
        if (existing->getEmail() == user->getEmail()) {
            throw DuplicateEmailException(user->getEmail());
        }
    }

    int assignedId = m_nextId++;
    user->setId(assignedId);
    users.push_back(user);

    writeAllToFile(users);
    return assignedId;
}

bool FileUserRepository::update(const std::shared_ptr<User>& user) {
    if (!user) return false;

    auto users = loadAllFromFile();
    bool found = false;

    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i]->getId() == user->getId()) {
            // Check email uniqueness if email changed
            if (users[i]->getEmail() != user->getEmail()) {
                for (size_t j = 0; j < users.size(); ++j) {
                    if (j != i && users[j]->getEmail() == user->getEmail()) {
                        throw DuplicateEmailException(user->getEmail());
                    }
                }
            }
            users[i] = user;
            found = true;
            break;
        }
    }

    if (found) {
        writeAllToFile(users);
    }
    return found;
}

bool FileUserRepository::deleteById(int id) {
    auto users = loadAllFromFile();
    auto initialSize = users.size();
    users.erase(
        std::remove_if(users.begin(), users.end(), [id](const std::shared_ptr<User>& u) {
            return u->getId() == id;
        }),
        users.end()
    );

    if (users.size() != initialSize) {
        writeAllToFile(users);
        return true;
    }
    return false;
}

std::shared_ptr<User> FileUserRepository::findById(int id) {
    auto users = loadAllFromFile();
    for (const auto& u : users) {
        if (u->getId() == id) {
            return u;
        }
    }
    return nullptr;
}

std::shared_ptr<User> FileUserRepository::findByEmail(const std::string& email) {
    auto users = loadAllFromFile();
    for (const auto& u : users) {
        if (u->getEmail() == email) {
            return u;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<User>> FileUserRepository::findAll() {
    return loadAllFromFile();
}

std::vector<std::shared_ptr<User>> FileUserRepository::findByRole(UserRole role) {
    auto users = loadAllFromFile();
    std::vector<std::shared_ptr<User>> filtered;
    for (const auto& u : users) {
        if (u->getRole() == role) {
            filtered.push_back(u);
        }
    }
    return filtered;
}

bool FileUserRepository::existsByEmail(const std::string& email) {
    return findByEmail(email) != nullptr;
}

size_t FileUserRepository::count() {
    return loadAllFromFile().size();
}

} // namespace civiccare
