#pragma once

#include <ctime>
#include <string>

namespace airwatcher::utils {

std::time_t parseTimestamp(const std::string& ts);
std::string formatTimestamp(std::time_t epoch);

} // namespace airwatcher::utils
