#pragma once

#include <string>
#include "exceptions/ComplaintExceptions.hpp"

namespace CivicCare {

/**
 * @brief Domain model representing a Municipal Department (e.g., Roads, Sanitation, Water).
 */
class Department {
private:
    int id_;
    std::string name_;
    std::string code_;
    std::string contactEmail_;
    std::string contactPhone_;
    bool active_;

public:
    Department()
        : id_(0), name_(""), code_(""), contactEmail_(""), contactPhone_(""), active_(true) {}

    Department(int id,
               const std::string& name,
               const std::string& code,
               const std::string& contactEmail = "",
               const std::string& contactPhone = "",
               bool active = true)
        : id_(id),
          name_(name),
          code_(code),
          contactEmail_(contactEmail),
          contactPhone_(contactPhone),
          active_(active) {
        validate();
    }

    void validate() const {
        if (id_ < 0) {
            throw InvalidComplaintDataException("Department id cannot be negative.");
        }
        if (name_.empty()) {
            throw InvalidComplaintDataException("Department name cannot be empty.");
        }
    }

    // Getters
    int getId() const noexcept { return id_; }
    const std::string& getName() const noexcept { return name_; }
    const std::string& getCode() const noexcept { return code_; }
    const std::string& getContactEmail() const noexcept { return contactEmail_; }
    const std::string& getContactPhone() const noexcept { return contactPhone_; }
    bool isActive() const noexcept { return active_; }

    // Setters
    void setId(int id) noexcept { id_ = id; }
    void setName(const std::string& name) {
        if (name.empty()) throw InvalidComplaintDataException("Department name cannot be empty.");
        name_ = name;
    }
    void setCode(const std::string& code) noexcept { code_ = code; }
    void setContactEmail(const std::string& email) noexcept { contactEmail_ = email; }
    void setContactPhone(const std::string& phone) noexcept { contactPhone_ = phone; }
    void setActive(bool active) noexcept { active_ = active; }
};

} // namespace CivicCare
