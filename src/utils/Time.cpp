#include "utils/Time.hpp"

#include <iomanip>
#include <sstream>

namespace airwatcher::utils {

std::time_t parseTimestamp(const std::string& ts) {
    std::tm tmStruct {};
    std::istringstream ss(ts);
    ss >> std::get_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return 0;
    }
    tmStruct.tm_isdst = -1;
    return std::mktime(&tmStruct);
}

std::string formatTimestamp(std::time_t epoch) {
    std::tm* tmStruct = std::localtime(&epoch);
    if (tmStruct == nullptr) {
        return {};
    }
    std::ostringstream out;
    out << std::put_time(tmStruct, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

} // namespace airwatcher::utils
