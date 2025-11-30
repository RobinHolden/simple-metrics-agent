#pragma once

#include <string>

struct HostnameResult {
    bool ok{false};
    std::string hostname;
    std::string error;
};

// Tries to detect the local hostname using POSIX gethostname().
// On success, returns { true, hostname, "" }.
// On failure, returns { false, "", error_message }.
HostnameResult detect_hostname();
