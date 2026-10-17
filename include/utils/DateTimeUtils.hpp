#ifndef CIVICCARE_UTILS_DATETIMEUTILS_HPP
#define CIVICCARE_UTILS_DATETIMEUTILS_HPP

#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace civiccare {

class DateTimeUtils {
public:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm tmBuffer;
        
        std::tm* tmPtr = std::localtime(&nowTime);
        if (tmPtr) {
            tmBuffer = *tmPtr;
        } else {
            std::memset(&tmBuffer, 0, sizeof(tmBuffer));
        }

        std::ostringstream oss;
        oss << std::put_time(&tmBuffer, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

} // namespace civiccare

#endif // CIVICCARE_UTILS_DATETIMEUTILS_HPP
