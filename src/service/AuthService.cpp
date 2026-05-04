#include "service/AuthService.hpp"

#include "utils/CsvUtils.hpp"

#include <fstream>
#include <functional>
#include <sstream>
#include <utility>

namespace airwatcher {

AuthService::AuthService(std::string accountsFilePath) : accountsFilePath_(std::move(accountsFilePath)) {}

bool AuthService::load() {
    accounts_.clear();
    unreliableUsers_.clear();

    std::ifstream in(accountsFilePath_);
    if (!in.is_open()) {
        registerAccount("admin", "admin", Role::Admin);
        return persist();
    }

    std::string line;
    while (std::getline(in, line)) {
        auto cols = utils::splitSemicolonLine(line);
        if (cols.size() < 3) continue;
        if (cols[0] == "UNRELIABLE" && cols.size() >= 2) {
            unreliableUsers_.insert(cols[1]);
            continue;
        }
        accounts_.push_back({cols[0], cols[1], stringToRole(cols[2])});
    }
    return true;
}

bool AuthService::persist() const {
    std::ofstream out(accountsFilePath_, std::ios::trunc);
    if (!out.is_open()) return false;

    for (const auto& account : accounts_) {
        out << account.username << ";" << account.passwordHash << ";" << roleToString(account.role) << ";\n";
    }
    for (const auto& userId : unreliableUsers_) {
        out << "UNRELIABLE;" << userId << ";;\n";
    }
    return true;
}

bool AuthService::registerAccount(const std::string& username, const std::string& password, Role role) {
    for (const auto& account : accounts_) {
        if (account.username == username) return false;
    }
    accounts_.push_back({username, hashPassword(password), role});
    return persist();
}

std::optional<UserAccount> AuthService::login(const std::string& username, const std::string& password) const {
    const auto passwordHash = hashPassword(password);
    for (const auto& account : accounts_) {
        if (account.username == username && account.passwordHash == passwordHash) {
            return account;
        }
    }
    return std::nullopt;
}

void AuthService::markUserUnreliable(const std::string& userId) {
    unreliableUsers_.insert(userId);
}

std::set<std::string> AuthService::unreliableUsers() const {
    return unreliableUsers_;
}

std::string AuthService::hashPassword(const std::string& password) {
    return std::to_string(std::hash<std::string> {}(password));
}

std::string AuthService::roleToString(Role role) {
    switch (role) {
    case Role::Agency: return "AGENCY";
    case Role::Provider: return "PROVIDER";
    case Role::PrivateUser: return "PRIVATE";
    case Role::Admin: return "ADMIN";
    }
    return "AGENCY";
}

Role AuthService::stringToRole(const std::string& role) {
    if (role == "PROVIDER") return Role::Provider;
    if (role == "PRIVATE") return Role::PrivateUser;
    if (role == "ADMIN") return Role::Admin;
    return Role::Agency;
}

} // namespace airwatcher
