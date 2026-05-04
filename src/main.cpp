#include "cli/CliApp.hpp"

int main() {
    airwatcher::CliApp app("csv_files", "csv_files/accounts.csv");
    return app.run();
}
