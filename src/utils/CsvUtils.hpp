#pragma once

#include <string>
#include <vector>

namespace airwatcher::utils {

std::vector<std::string> splitSemicolonLine(const std::string& line);
std::string trim(const std::string& value);

} // namespace airwatcher::utils
