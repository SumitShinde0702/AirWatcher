#include "repository/CsvRepository.hpp"

#include "utils/CsvUtils.hpp"
#include "utils/Time.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace airwatcher {

namespace {
std::string pathOf(const std::string& dir, const std::string& file) {
    return dir + "/" + file;
}

bool isHeaderLine(const std::vector<std::string>& cols, const std::string& expectedFirst) {
    return !cols.empty() && cols[0] == expectedFirst;
}
}

CsvRepository::CsvRepository(std::string dataDirectory) : dataDirectory_(std::move(dataDirectory)) {}

bool CsvRepository::loadAll() {
    dataStore_ = {};
    return loadSensors() && loadMeasurements() && loadAttributes() && loadProviders() && loadCleaners() && loadUsers();
}

bool CsvRepository::loadSensors() {
    std::ifstream in(pathOf(dataDirectory_, "sensors.csv"));
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 3) continue;
        if (isHeaderLine(cols, "SensorID")) continue;
        dataStore_.sensors.push_back({cols[0], std::stod(cols[1]), std::stod(cols[2])});
    }
    return true;
}

bool CsvRepository::loadMeasurements() {
    std::ifstream in(pathOf(dataDirectory_, "measurements.csv"));
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 4) continue;
        if (isHeaderLine(cols, "Timestamp")) continue;
        Measurement m;
        m.timestamp = cols[0];
        m.epoch = utils::parseTimestamp(cols[0]);
        m.sensorId = cols[1];
        m.attributeId = cols[2];
        m.value = std::stod(cols[3]);
        dataStore_.measurements.push_back(m);
    }
    return true;
}

bool CsvRepository::loadAttributes() {
    std::ifstream in(pathOf(dataDirectory_, "attributes.csv"));
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 3) continue;
        if (isHeaderLine(cols, "AttributeID")) continue;
        dataStore_.attributes.push_back({cols[0], cols[1], cols[2]});
    }
    return true;
}

bool CsvRepository::loadProviders() {
    std::ifstream in(pathOf(dataDirectory_, "providers.csv"));
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 2) continue;
        if (isHeaderLine(cols, "ProviderID")) continue;
        dataStore_.providers.push_back({cols[0], cols[1]});
    }
    return true;
}

bool CsvRepository::loadCleaners() {
    std::ifstream in(pathOf(dataDirectory_, "cleaners.csv"));
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 5) continue;
        if (isHeaderLine(cols, "CleanerID")) continue;
        Cleaner c;
        c.cleanerId = cols[0];
        c.latitude = std::stod(cols[1]);
        c.longitude = std::stod(cols[2]);
        c.start = cols[3];
        c.stop = cols[4];
        c.startEpoch = utils::parseTimestamp(cols[3]);
        c.stopEpoch = utils::parseTimestamp(cols[4]);
        dataStore_.cleaners.push_back(c);
    }
    return true;
}

bool CsvRepository::loadUsers() {
    std::ifstream in(pathOf(dataDirectory_, "users.csv"));
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 2) continue;
        if (isHeaderLine(cols, "UserID")) continue;
        dataStore_.privateUserSensors.push_back({cols[0], cols[1]});
    }
    return true;
}

bool CsvRepository::appendSensor(const Sensor& sensor) {
    std::ofstream out(pathOf(dataDirectory_, "sensors.csv"), std::ios::app);
    if (!out.is_open()) return false;
    out << sensor.id << ";" << sensor.latitude << ";" << sensor.longitude << ";\n";
    dataStore_.sensors.push_back(sensor);
    return true;
}

bool CsvRepository::appendMeasurement(const Measurement& measurement) {
    std::ofstream out(pathOf(dataDirectory_, "measurements.csv"), std::ios::app);
    if (!out.is_open()) return false;
    out << measurement.timestamp << ";" << measurement.sensorId << ";" << measurement.attributeId << ";"
        << measurement.value << ";\n";
    dataStore_.measurements.push_back(measurement);
    return true;
}

bool CsvRepository::appendProvider(const Provider& provider) {
    std::ofstream out(pathOf(dataDirectory_, "providers.csv"), std::ios::app);
    if (!out.is_open()) return false;
    out << provider.providerId << ";" << provider.cleanerId << ";\n";
    dataStore_.providers.push_back(provider);
    return true;
}

bool CsvRepository::appendCleaner(const Cleaner& cleaner) {
    std::ofstream out(pathOf(dataDirectory_, "cleaners.csv"), std::ios::app);
    if (!out.is_open()) return false;
    out << cleaner.cleanerId << ";" << cleaner.latitude << ";" << cleaner.longitude << ";"
        << cleaner.start << ";" << cleaner.stop << ";\n";
    dataStore_.cleaners.push_back(cleaner);
    return true;
}

bool CsvRepository::appendPrivateUser(const PrivateUserSensor& user) {
    std::ofstream out(pathOf(dataDirectory_, "users.csv"), std::ios::app);
    if (!out.is_open()) return false;
    out << user.userId << ";" << user.sensorId << ";\n";
    dataStore_.privateUserSensors.push_back(user);
    return true;
}

std::set<std::string> CsvRepository::validateReferences() const {
    std::set<std::string> issues;
    std::set<std::string> sensorIds;
    std::set<std::string> attributeIds;
    std::set<std::string> cleanerIds;

    for (const auto& s : dataStore_.sensors) sensorIds.insert(s.id);
    for (const auto& a : dataStore_.attributes) attributeIds.insert(a.id);
    for (const auto& c : dataStore_.cleaners) cleanerIds.insert(c.cleanerId);

    for (const auto& m : dataStore_.measurements) {
        if (sensorIds.find(m.sensorId) == sensorIds.end()) {
            issues.insert("Unknown sensor in measurements: " + m.sensorId);
        }
        if (attributeIds.find(m.attributeId) == attributeIds.end()) {
            issues.insert("Unknown attribute in measurements: " + m.attributeId);
        }
    }

    for (const auto& p : dataStore_.providers) {
        if (cleanerIds.find(p.cleanerId) == cleanerIds.end()) {
            issues.insert("Unknown cleaner in providers: " + p.cleanerId);
        }
    }

    for (const auto& u : dataStore_.privateUserSensors) {
        if (sensorIds.find(u.sensorId) == sensorIds.end()) {
            issues.insert("Unknown sensor in users: " + u.sensorId);
        }
    }

    return issues;
}

} // namespace airwatcher
