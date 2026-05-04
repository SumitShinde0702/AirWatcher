#pragma once

#include "domain/Entities.hpp"
#include "repository/CsvRepository.hpp"
#include "service/AqiService.hpp"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace airwatcher {

struct MeanAqiStats {
    double meanAqi {};
    double sumAqi {};
    std::size_t pointCount {};
    std::size_t sensorCount {};
};

struct SimilarityDebug {
    std::string referenceSensor;
    std::string comparedSensor;
    double score {};
    double dotProduct {};
    double normReference {};
    double normCompared {};
    std::size_t overlapCount {};
};

class AirAnalysisService {
public:
    explicit AirAnalysisService(const DataStore& dataStore);

    void setExcludedUsers(const std::set<std::string>& excludedUsers);
    std::set<std::string> excludedUsers() const;
    std::map<std::string, int> pointsByUser() const;

    std::optional<double> meanAqiInArea(double latitude, double longitude, double radiusKm,
        std::time_t start, std::time_t stop);
    std::optional<MeanAqiStats> meanAqiInAreaWithStats(double latitude, double longitude, double radiusKm,
        std::time_t start, std::time_t stop);

    std::vector<SensorSimilarity> rankSensorsBySimilarity(const std::string& referenceSensor,
        std::time_t start, std::time_t stop);
    std::optional<SimilarityDebug> explainSimilarity(const std::string& referenceSensor,
        const std::string& comparedSensor, std::time_t start, std::time_t stop);

    std::optional<double> estimateAqiAtPosition(double latitude, double longitude,
        std::time_t epoch, std::size_t kNearest = 5);

    ReliabilityReport analyzeSensorReliability(const std::string& sensorId,
        std::time_t start, std::time_t stop);

    std::optional<CleanerImpactReport> evaluateCleanerImpact(const std::string& cleanerId,
        double radiusKm);

private:
    std::string privateOwner(const std::string& sensorId) const;
    bool sensorAllowed(const std::string& sensorId) const;
    std::optional<double> sensorAqiAt(const std::string& sensorId, std::time_t epoch);
    std::vector<SensorAqiPoint> sensorSeries(const std::string& sensorId, std::time_t start, std::time_t stop);

    const DataStore& dataStore_;
    AqiService aqiService_;
    std::set<std::string> excludedUsers_;
    std::map<std::string, int> pointsByUser_;
};

} // namespace airwatcher
