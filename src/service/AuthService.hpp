#pragma once

#include "domain/Entities.hpp"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace airwatcher {

class AuthService {
public:
    explicit AuthService(std::string accountsFilePath);

    bool load();
    bool persist() const;

    bool registerAccount(const std::string& username, const std::string& password, Role role);
    std::optional<UserAccount> login(const std::string& username, const std::string& password) const;

    void markUserUnreliable(const std::string& userId);
    std::set<std::string> unreliableUsers() const;

private:
    static std::string hashPassword(const std::string& password);
    static std::string roleToString(Role role);
    static Role stringToRole(const std::string& role);

    std::string accountsFilePath_;
    std::vector<UserAccount> accounts_;
    std::set<std::string> unreliableUsers_;
};

} // namespace airwatcher
