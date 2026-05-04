#pragma once

#include <map>
#include <string>

namespace airwatcher {

class AqiService {
public:
    int pollutantSubIndex(const std::string& attributeId, double value) const;
    double computeAqiFromAttributes(const std::map<std::string, double>& values) const;
};

} // namespace airwatcher
