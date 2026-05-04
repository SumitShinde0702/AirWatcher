#include "service/AqiService.hpp"

#include <algorithm>
#include <vector>

namespace airwatcher {

namespace {
int mapThreshold(double value, const std::vector<double>& thresholds) {
    for (std::size_t i = 0; i < thresholds.size(); ++i) {
        if (value <= thresholds[i]) {
            return static_cast<int>(i + 1);
        }
    }
    return 10;
}
}

int AqiService::pollutantSubIndex(const std::string& attributeId, double value) const {
    if (attributeId == "O3") {
        return mapThreshold(value, {29, 54, 79, 104, 129, 149, 179, 209, 239});
    }
    if (attributeId == "NO2") {
        return mapThreshold(value, {29, 54, 84, 109, 134, 164, 199, 274, 399});
    }
    if (attributeId == "SO2") {
        return mapThreshold(value, {39, 79, 119, 159, 199, 249, 299, 399, 499});
    }
    if (attributeId == "PM10") {
        return mapThreshold(value, {6, 13, 20, 27, 34, 41, 49, 64, 79});
    }
    return 1;
}

double AqiService::computeAqiFromAttributes(const std::map<std::string, double>& values) const {
    int maxSubIndex = 1;
    for (const auto& [attribute, value] : values) {
        maxSubIndex = std::max(maxSubIndex, pollutantSubIndex(attribute, value));
    }
    return static_cast<double>(maxSubIndex);
}

} // namespace airwatcher
