#include "cli/CliApp.hpp"

#include "utils/Time.hpp"
#include "utils/Timer.hpp"

#include <iostream>
#include <utility>

namespace airwatcher {

CliApp::CliApp(std::string dataDir, std::string accountsFile)
    : repository_(std::move(dataDir)), authService_(std::move(accountsFile)) {}

int CliApp::run() {
    if (!repository_.loadAll()) {
        std::cerr << "Failed to load CSV files.\n";
        return 1;
    }
    authService_.load();

    std::cout << "AirWatcher CLI\n";
    std::cout << "Username: ";
    std::string username;
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::string password;
    std::getline(std::cin, password);

    auto account = authService_.login(username, password);
    if (!account.has_value()) {
        std::cout << "Invalid credentials.\n";
        return 1;
    }

    runSession(*account);
    authService_.persist();
    return 0;
}

void CliApp::runSession(const UserAccount& account) {
    AirAnalysisService analysis(repository_.data());
    analysis.setExcludedUsers(authService_.unreliableUsers());

    bool running = true;
    while (running) {
        printMenu(account.role);
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "0") {
            running = false;
        } else if (choice == "1") {
            double lat, lon, radius;
            std::string startStr, stopStr;
            std::cout << "Center latitude: "; std::cin >> lat;
            std::cout << "Center longitude: "; std::cin >> lon;
            std::cout << "Radius km: "; std::cin >> radius;
            std::cin.ignore();
            std::cout << "Start (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, startStr);
            std::cout << "Stop  (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, stopStr);

            utils::ScopedTimer timer;
            auto mean = analysis.meanAqiInArea(lat, lon, radius,
                utils::parseTimestamp(startStr), utils::parseTimestamp(stopStr));
            std::cout << (mean.has_value() ? "Mean AQI: " + std::to_string(*mean) : "No data for area/time") << "\n";
            std::cout << "Duration: " << timer.elapsedMs() << " ms\n";
        } else if (choice == "2") {
            std::string ref, startStr, stopStr;
            std::cout << "Reference sensor: "; std::getline(std::cin, ref);
            std::cout << "Start (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, startStr);
            std::cout << "Stop  (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, stopStr);

            utils::ScopedTimer timer;
            auto ranking = analysis.rankSensorsBySimilarity(ref,
                utils::parseTimestamp(startStr), utils::parseTimestamp(stopStr));
            std::cout << "Top 5 similar sensors:\n";
            for (std::size_t i = 0; i < ranking.size() && i < 5; ++i) {
                std::cout << i + 1 << ". " << ranking[i].sensorId << " score=" << ranking[i].score << "\n";
            }
            std::cout << "Duration: " << timer.elapsedMs() << " ms\n";
        } else if (choice == "3") {
            double lat, lon;
            std::string timeStr;
            std::cout << "Latitude: "; std::cin >> lat;
            std::cout << "Longitude: "; std::cin >> lon;
            std::cin.ignore();
            std::cout << "Timestamp (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, timeStr);

            auto value = analysis.estimateAqiAtPosition(lat, lon, utils::parseTimestamp(timeStr));
            std::cout << (value.has_value() ? "Estimated AQI: " + std::to_string(*value) : "No estimate available") << "\n";
        } else if (choice == "4") {
            std::string sensor, startStr, stopStr;
            std::cout << "Sensor ID: "; std::getline(std::cin, sensor);
            std::cout << "Start (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, startStr);
            std::cout << "Stop  (YYYY-MM-DD HH:MM:SS): "; std::getline(std::cin, stopStr);

            auto report = analysis.analyzeSensorReliability(sensor,
                utils::parseTimestamp(startStr), utils::parseTimestamp(stopStr));
            std::cout << "Reliable: " << (report.reliable ? "yes" : "no")
                      << " anomaly=" << report.anomalyScore << "\n";
            std::cout << report.details << "\n";
        } else if (choice == "5") {
            std::string cleanerId;
            double radius;
            std::cout << "Cleaner ID: "; std::getline(std::cin, cleanerId);
            std::cout << "Radius km: "; std::cin >> radius; std::cin.ignore();

            auto report = analysis.evaluateCleanerImpact(cleanerId, radius);
            if (!report.has_value()) {
                std::cout << "Cleaner impact unavailable.\n";
            } else {
                std::cout << "Before=" << report->beforeMeanAqi << " During=" << report->duringMeanAqi
                          << " After=" << report->afterMeanAqi << " Improvement=" << report->improvement << "\n";
            }
        } else if (choice == "6" && account.role == Role::Admin) {
            std::string userId;
            std::cout << "Private user ID to flag unreliable: "; std::getline(std::cin, userId);
            authService_.markUserUnreliable(userId);
            analysis.setExcludedUsers(authService_.unreliableUsers());
            std::cout << "User flagged and excluded from future analytics.\n";
        } else if (choice == "7") {
            const auto points = analysis.pointsByUser();
            for (const auto& [user, score] : points) {
                std::cout << user << " -> " << score << " points\n";
            }
            if (points.empty()) {
                std::cout << "No private-user points yet.\n";
            }
        } else if (choice == "8" && account.role == Role::Admin) {
            std::string sensorId;
            double lat, lon;
            std::cout << "New sensor ID: "; std::getline(std::cin, sensorId);
            std::cout << "Latitude: "; std::cin >> lat;
            std::cout << "Longitude: "; std::cin >> lon; std::cin.ignore();
            if (repository_.appendSensor({sensorId, lat, lon})) {
                std::cout << "Sensor added.\n";
            } else {
                std::cout << "Failed to add sensor.\n";
            }
        } else if (choice == "9") {
            std::string startStr = "2019-01-01 12:00:00";
            std::string stopStr = "2019-01-15 12:00:00";
            utils::ScopedTimer timer;
            (void) analysis.meanAqiInArea(45.0, 2.0, 400.0, utils::parseTimestamp(startStr), utils::parseTimestamp(stopStr));
            (void) analysis.rankSensorsBySimilarity("Sensor0", utils::parseTimestamp(startStr), utils::parseTimestamp(stopStr));
            (void) analysis.estimateAqiAtPosition(45.0, 2.0, utils::parseTimestamp("2019-01-03 12:00:00"));
            std::cout << "Benchmark scenario duration: " << timer.elapsedMs() << " ms\n";
        } else {
            std::cout << "Unknown command.\n";
        }
    }
}

void CliApp::printMenu(Role role) const {
    std::cout << "\n--- Menu ---\n";
    std::cout << "1) Mean AQI in circular area/time range\n";
    std::cout << "2) Rank sensors by similarity\n";
    std::cout << "3) Estimate AQI at position/time\n";
    std::cout << "4) Analyze sensor reliability\n";
    std::cout << "5) Evaluate cleaner impact\n";
    if (role == Role::Admin) {
        std::cout << "6) Mark private user as unreliable\n";
        std::cout << "8) Add sensor entity\n";
    }
    std::cout << "7) Show private-user points\n";
    std::cout << "9) Run benchmark scenario\n";
    std::cout << "0) Exit\n";
    std::cout << "> ";
}

} // namespace airwatcher
