#pragma once

#include "repository/CsvRepository.hpp"
#include "service/AirAnalysisService.hpp"
#include "service/AuthService.hpp"

#include <string>

namespace airwatcher {

class CliApp {
public:
    CliApp(std::string dataDir, std::string accountsFile);
    int run();

private:
    void runSession(const UserAccount& account);
    void printMenu(Role role) const;

    CsvRepository repository_;
    AuthService authService_;
};

} // namespace airwatcher
