#pragma once

#include "domain/Entities.hpp"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace airwatcher {

struct DataStore {
    std::vector<Sensor> sensors;
    std::vector<Measurement> measurements;
    std::vector<Attribute> attributes;
    std::vector<Provider> providers;
    std::vector<Cleaner> cleaners;
    std::vector<PrivateUserSensor> privateUserSensors;
};

class CsvRepository {
public:
    explicit CsvRepository(std::string dataDirectory);

    bool loadAll();
    const DataStore& data() const { return dataStore_; }

    bool appendSensor(const Sensor& sensor);
    bool appendMeasurement(const Measurement& measurement);
    bool appendProvider(const Provider& provider);
    bool appendCleaner(const Cleaner& cleaner);
    bool appendPrivateUser(const PrivateUserSensor& user);

    std::set<std::string> validateReferences() const;

private:
    bool loadSensors();
    bool loadMeasurements();
    bool loadAttributes();
    bool loadProviders();
    bool loadCleaners();
    bool loadUsers();

    std::string dataDirectory_;
    DataStore dataStore_;
};

} // namespace airwatcher
