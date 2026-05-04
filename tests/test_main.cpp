#include "repository/CsvRepository.hpp"
#include "service/AirAnalysisService.hpp"
#include "service/AqiService.hpp"
#include "utils/Time.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace airwatcher;

    AqiService aqi;
    assert(aqi.pollutantSubIndex("O3", 20) == 1);
    assert(aqi.pollutantSubIndex("O3", 300) == 10);

    CsvRepository repo("csv_files");
    assert(repo.loadAll());
    assert(!repo.data().sensors.empty());
    assert(!repo.data().measurements.empty());

    AirAnalysisService analysis(repo.data());
    const auto start = utils::parseTimestamp("2019-01-01 12:00:00");
    const auto stop = utils::parseTimestamp("2019-01-10 12:00:00");

    auto mean = analysis.meanAqiInArea(45.0, 2.0, 500.0, start, stop);
    assert(mean.has_value());

    auto ranking = analysis.rankSensorsBySimilarity("Sensor0", start, stop);
    assert(!ranking.empty());

    auto estimate = analysis.estimateAqiAtPosition(44.0, -1.0, start);
    assert(estimate.has_value());

    auto reliability = analysis.analyzeSensorReliability("Sensor0", start, stop);
    assert(reliability.anomalyScore >= 0.0);

    auto cleaner = analysis.evaluateCleanerImpact("Cleaner0", 500.0);
    assert(cleaner.has_value());

    std::cout << "All tests passed.\n";
    return 0;
}
