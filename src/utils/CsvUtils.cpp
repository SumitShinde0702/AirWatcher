#include "utils/CsvUtils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace airwatcher::utils {

std::vector<std::string> splitSemicolonLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ';')) {
        tokens.push_back(trim(token));
    }
    if (!tokens.empty() && tokens.back().empty()) {
        tokens.pop_back();
    }
    return tokens;
}

std::string trim(const std::string& value) {
    std::string out = value;
    out.erase(out.begin(), std::find_if(out.begin(), out.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    out.erase(std::find_if(out.rbegin(), out.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), out.end());
    return out;
}

} // namespace airwatcher::utils
