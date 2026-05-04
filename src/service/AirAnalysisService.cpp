#include "service/AirAnalysisService.hpp"

#include "utils/Geo.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

namespace airwatcher {

AirAnalysisService::AirAnalysisService(const DataStore& dataStore) : dataStore_(dataStore) {}

void AirAnalysisService::setExcludedUsers(const std::set<std::string>& excludedUsers) {
    excludedUsers_ = excludedUsers;
}

std::set<std::string> AirAnalysisService::excludedUsers() const {
    return excludedUsers_;
}

std::map<std::string, int> AirAnalysisService::pointsByUser() const {
    return pointsByUser_;
}

std::string AirAnalysisService::privateOwner(const std::string& sensorId) const {
    for (const auto& entry : dataStore_.privateUserSensors) {
        if (entry.sensorId == sensorId) {
            return entry.userId;
        }
    }
    return {};
}

bool AirAnalysisService::sensorAllowed(const std::string& sensorId) const {
    const auto owner = privateOwner(sensorId);
    if (owner.empty()) return true;
    return excludedUsers_.find(owner) == excludedUsers_.end();
}

std::optional<double> AirAnalysisService::sensorAqiAt(const std::string& sensorId, std::time_t epoch) {
    std::map<std::string, double> attributes;
    for (const auto& m : dataStore_.measurements) {
        if (m.sensorId == sensorId && m.epoch == epoch && sensorAllowed(sensorId)) {
            attributes[m.attributeId] = m.value;
        }
    }
    if (attributes.empty()) {
        return std::nullopt;
    }

    const auto owner = privateOwner(sensorId);
    if (!owner.empty()) {
        ++pointsByUser_[owner];
    }
    return aqiService_.computeAqiFromAttributes(attributes);
}

std::vector<SensorAqiPoint> AirAnalysisService::sensorSeries(const std::string& sensorId,
    std::time_t start, std::time_t stop) {
    std::map<std::time_t, std::map<std::string, double>> byTime;
    for (const auto& m : dataStore_.measurements) {
        if (m.sensorId == sensorId && m.epoch >= start && m.epoch <= stop && sensorAllowed(sensorId)) {
            byTime[m.epoch][m.attributeId] = m.value;
        }
    }

    std::vector<SensorAqiPoint> series;
    series.reserve(byTime.size());
    for (const auto& [epoch, attrs] : byTime) {
        const auto owner = privateOwner(sensorId);
        if (!owner.empty()) {
            ++pointsByUser_[owner];
        }
        series.push_back({sensorId, epoch, aqiService_.computeAqiFromAttributes(attrs)});
    }
    return series;
}

std::optional<double> AirAnalysisService::meanAqiInArea(double latitude, double longitude,
    double radiusKm, std::time_t start, std::time_t stop) {
    auto stats = meanAqiInAreaWithStats(latitude, longitude, radiusKm, start, stop);
    if (!stats.has_value()) {
        return std::nullopt;
    }
    return stats->meanAqi;
}

std::optional<MeanAqiStats> AirAnalysisService::meanAqiInAreaWithStats(double latitude, double longitude,
    double radiusKm, std::time_t start, std::time_t stop) {
    std::vector<double> aqiValues;
    std::size_t sensorsContributing = 0;

    for (const auto& sensor : dataStore_.sensors) {
        if (!sensorAllowed(sensor.id)) continue;
        const double distance = utils::haversineKm(latitude, longitude, sensor.latitude, sensor.longitude);
        if (distance > radiusKm) continue;

        const auto series = sensorSeries(sensor.id, start, stop);
        if (!series.empty()) {
            ++sensorsContributing;
        }
        for (const auto& point : series) {
            aqiValues.push_back(point.aqi);
        }
    }

    if (aqiValues.empty()) {
        return std::nullopt;
    }
    const double sum = std::accumulate(aqiValues.begin(), aqiValues.end(), 0.0);
    return MeanAqiStats {
        sum / static_cast<double>(aqiValues.size()),
        sum,
        aqiValues.size(),
        sensorsContributing
    };
}

std::vector<SensorSimilarity> AirAnalysisService::rankSensorsBySimilarity(const std::string& referenceSensor,
    std::time_t start, std::time_t stop) {
    std::vector<SensorSimilarity> result;
    const auto referenceSeries = sensorSeries(referenceSensor, start, stop);
    if (referenceSeries.empty()) return result;

    std::map<std::time_t, double> refByTime;
    for (const auto& p : referenceSeries) refByTime[p.epoch] = p.aqi;

    for (const auto& sensor : dataStore_.sensors) {
        if (sensor.id == referenceSensor || !sensorAllowed(sensor.id)) continue;
        const auto otherSeries = sensorSeries(sensor.id, start, stop);
        if (otherSeries.empty()) continue;

        double dot = 0.0;
        double normA = 0.0;
        double normB = 0.0;
        for (const auto& p : otherSeries) {
            auto it = refByTime.find(p.epoch);
            if (it == refByTime.end()) continue;
            dot += it->second * p.aqi;
            normA += it->second * it->second;
            normB += p.aqi * p.aqi;
        }
        if (normA == 0.0 || normB == 0.0) continue;

        const double cosine = dot / (std::sqrt(normA) * std::sqrt(normB));
        result.push_back({sensor.id, cosine});
    }

    std::sort(result.begin(), result.end(), [](const SensorSimilarity& a, const SensorSimilarity& b) {
        return a.score > b.score;
    });
    return result;
}

std::optional<SimilarityDebug> AirAnalysisService::explainSimilarity(const std::string& referenceSensor,
    const std::string& comparedSensor, std::time_t start, std::time_t stop) {
    if (referenceSensor == comparedSensor || !sensorAllowed(referenceSensor) || !sensorAllowed(comparedSensor)) {
        return std::nullopt;
    }

    const auto referenceSeries = sensorSeries(referenceSensor, start, stop);
    const auto comparedSeries = sensorSeries(comparedSensor, start, stop);
    if (referenceSeries.empty() || comparedSeries.empty()) {
        return std::nullopt;
    }

    std::map<std::time_t, double> refByTime;
    for (const auto& point : referenceSeries) {
        refByTime[point.epoch] = point.aqi;
    }

    double dot = 0.0;
    double normRef = 0.0;
    double normCmp = 0.0;
    std::size_t overlap = 0;
    for (const auto& point : comparedSeries) {
        auto it = refByTime.find(point.epoch);
        if (it == refByTime.end()) {
            continue;
        }
        dot += it->second * point.aqi;
        normRef += it->second * it->second;
        normCmp += point.aqi * point.aqi;
        ++overlap;
    }
    if (normRef == 0.0 || normCmp == 0.0 || overlap == 0) {
        return std::nullopt;
    }

    return SimilarityDebug {
        referenceSensor,
        comparedSensor,
        dot / (std::sqrt(normRef) * std::sqrt(normCmp)),
        dot,
        normRef,
        normCmp,
        overlap
    };
}

std::optional<double> AirAnalysisService::estimateAqiAtPosition(double latitude, double longitude,
    std::time_t epoch, std::size_t kNearest) {
    for (const auto& sensor : dataStore_.sensors) {
        if (sensor.latitude == latitude && sensor.longitude == longitude && sensorAllowed(sensor.id)) {
            return sensorAqiAt(sensor.id, epoch);
        }
    }

    struct Candidate {
        std::string sensorId;
        double distance;
        double aqi;
    };
    std::vector<Candidate> candidates;

    for (const auto& sensor : dataStore_.sensors) {
        if (!sensorAllowed(sensor.id)) continue;
        auto aqi = sensorAqiAt(sensor.id, epoch);
        if (!aqi.has_value()) continue;

        const double dist = utils::haversineKm(latitude, longitude, sensor.latitude, sensor.longitude);
        if (dist < 0.001) return aqi;
        candidates.push_back({sensor.id, dist, *aqi});
    }

    if (candidates.empty()) return std::nullopt;
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return a.distance < b.distance;
    });
    if (candidates.size() > kNearest) candidates.resize(kNearest);

    double weightedSum = 0.0;
    double totalWeight = 0.0;
    for (const auto& c : candidates) {
        const double w = 1.0 / c.distance;
        weightedSum += w * c.aqi;
        totalWeight += w;
    }
    return weightedSum / totalWeight;
}

ReliabilityReport AirAnalysisService::analyzeSensorReliability(const std::string& sensorId,
    std::time_t start, std::time_t stop) {
    int suspicious = 0;
    int total = 0;

    std::map<std::time_t, std::vector<double>> neighborAqi;
    for (const auto& sensor : dataStore_.sensors) {
        if (sensor.id == sensorId || !sensorAllowed(sensor.id)) continue;
        auto series = sensorSeries(sensor.id, start, stop);
        for (const auto& p : series) {
            neighborAqi[p.epoch].push_back(p.aqi);
        }
    }

    auto targetSeries = sensorSeries(sensorId, start, stop);
    for (const auto& p : targetSeries) {
        ++total;
        if (p.aqi < 1.0 || p.aqi > 10.0) {
            ++suspicious;
            continue;
        }

        auto it = neighborAqi.find(p.epoch);
        if (it == neighborAqi.end() || it->second.empty()) continue;

        const double mean = std::accumulate(it->second.begin(), it->second.end(), 0.0)
            / static_cast<double>(it->second.size());
        if (std::abs(p.aqi - mean) >= 4.0) {
            ++suspicious;
        }
    }

    double anomaly = total == 0 ? 0.0 : static_cast<double>(suspicious) / static_cast<double>(total);
    const bool reliable = anomaly < 0.35;

    ReliabilityReport report;
    report.sensorId = sensorId;
    report.reliable = reliable;
    report.anomalyScore = anomaly;
    report.details = reliable ? "Sensor behavior is consistent." : "Sensor diverges from neighborhood profile.";
    return report;
}

std::optional<CleanerImpactReport> AirAnalysisService::evaluateCleanerImpact(const std::string& cleanerId,
    double radiusKm) {
    auto cleanerIt = std::find_if(dataStore_.cleaners.begin(), dataStore_.cleaners.end(),
        [&cleanerId](const Cleaner& c) { return c.cleanerId == cleanerId; });
    if (cleanerIt == dataStore_.cleaners.end()) return std::nullopt;

    const auto& cleaner = *cleanerIt;
    const std::time_t duration = cleaner.stopEpoch - cleaner.startEpoch;
    if (duration <= 0) return std::nullopt;

    auto before = meanAqiInArea(cleaner.latitude, cleaner.longitude, radiusKm,
        cleaner.startEpoch - duration, cleaner.startEpoch - 1);
    auto during = meanAqiInArea(cleaner.latitude, cleaner.longitude, radiusKm,
        cleaner.startEpoch, cleaner.stopEpoch);
    auto after = meanAqiInArea(cleaner.latitude, cleaner.longitude, radiusKm,
        cleaner.stopEpoch + 1, cleaner.stopEpoch + duration);

    if (!before.has_value() || !during.has_value() || !after.has_value()) {
        return std::nullopt;
    }

    CleanerImpactReport report;
    report.cleanerId = cleanerId;
    report.beforeMeanAqi = *before;
    report.duringMeanAqi = *during;
    report.afterMeanAqi = *after;
    report.improvement = report.beforeMeanAqi - report.duringMeanAqi;
    return report;
}

} // namespace airwatcher
