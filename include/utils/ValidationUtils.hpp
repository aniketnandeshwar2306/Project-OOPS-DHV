#ifndef CIVICCARE_UTILS_VALIDATIONUTILS_HPP
#define CIVICCARE_UTILS_VALIDATIONUTILS_HPP

#include <string>
#include <algorithm>
#include <cctype>

namespace civiccare {

class ValidationUtils {
public:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    static bool isValidEmail(const std::string& email) {
        std::string trimmed = trim(email);
        if (trimmed.empty()) return false;

        size_t atPos = trimmed.find('@');
        if (atPos == std::string::npos || atPos == 0 || atPos == trimmed.length() - 1) {
            return false;
        }

        // Must not contain multiple '@' symbols
        if (trimmed.find('@', atPos + 1) != std::string::npos) {
            return false;
        }

        size_t dotPos = trimmed.find('.', atPos + 1);
        if (dotPos == std::string::npos || dotPos == atPos + 1 || dotPos == trimmed.length() - 1) {
            return false;
        }

        // Check for disallowed characters (spaces, tabs)
        for (char c : trimmed) {
            if (std::isspace(static_cast<unsigned char>(c))) return false;
        }

        return true;
    }

    static bool isValidPassword(const std::string& password, std::string& reason) {
        if (password.length() < 6) {
            reason = "Password must be at least 6 characters long.";
            return false;
        }

        bool hasLetter = false;
        bool hasDigit = false;
        for (char c : password) {
            if (std::isalpha(static_cast<unsigned char>(c))) hasLetter = true;
            if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        }

        if (!hasLetter || !hasDigit) {
            reason = "Password must contain both letters and digits.";
            return false;
        }

        return true;
    }

    static bool isValidPhone(const std::string& phone) {
        std::string trimmed = trim(phone);
        if (trimmed.length() < 7 || trimmed.length() > 15) return false;
        for (char c : trimmed) {
            if (!std::isdigit(static_cast<unsigned char>(c)) && c != '+' && c != '-' && c != ' ') {
                return false;
            }
        }
        return true;
    }
};

} // namespace civiccare

#endif // CIVICCARE_UTILS_VALIDATIONUTILS_HPP
