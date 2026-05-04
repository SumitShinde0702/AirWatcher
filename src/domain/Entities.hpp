#pragma once

#include <map>
#include <optional>
#include <string>
#include <ctime>
#include <vector>

namespace airwatcher {

enum class Role {
    Agency,
    Provider,
    PrivateUser,
    Admin
};

struct Sensor {
    std::string id;
    double latitude {};
    double longitude {};
};

struct Attribute {
    std::string id;
    std::string unit;
    std::string description;
};

struct Measurement {
    std::string timestamp;
    std::time_t epoch {};
    std::string sensorId;
    std::string attributeId;
    double value {};
};

struct Provider {
    std::string providerId;
    std::string cleanerId;
};

struct Cleaner {
    std::string cleanerId;
    double latitude {};
    double longitude {};
    std::string start;
    std::string stop;
    std::time_t startEpoch {};
    std::time_t stopEpoch {};
};

struct PrivateUserSensor {
    std::string userId;
    std::string sensorId;
};

struct UserAccount {
    std::string username;
    std::string passwordHash;
    Role role {Role::Agency};
};

struct SensorAqiPoint {
    std::string sensorId;
    std::time_t epoch {};
    double aqi {};
};

struct SensorSimilarity {
    std::string sensorId;
    double score {};
};

struct ReliabilityReport {
    std::string sensorId;
    bool reliable {true};
    double anomalyScore {};
    std::string details;
};

struct CleanerImpactReport {
    std::string cleanerId;
    double beforeMeanAqi {};
    double duringMeanAqi {};
    double afterMeanAqi {};
    double improvement {};
};

} // namespace airwatcher
