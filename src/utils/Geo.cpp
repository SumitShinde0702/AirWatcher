#include "utils/Geo.hpp"

#include <cmath>

namespace airwatcher::utils {

namespace {
constexpr double kEarthRadiusKm = 6371.0;
double toRadians(double value) {
    return value * 3.14159265358979323846 / 180.0;
}
}

double haversineKm(double lat1, double lon1, double lat2, double lon2) {
    const double dLat = toRadians(lat2 - lat1);
    const double dLon = toRadians(lon2 - lon1);
    const double a = std::pow(std::sin(dLat / 2.0), 2)
        + std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) * std::pow(std::sin(dLon / 2.0), 2);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return kEarthRadiusKm * c;
}

} // namespace airwatcher::utils
