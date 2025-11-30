#include "hostname.hpp"

#include <array>
#include <cstddef>
#include <unistd.h>

#include "posix_error.hpp"

namespace {

constexpr std::size_t kHostnameBufferSize = 256;

} // namespace

HostnameResult detect_hostname() {
    HostnameResult res{};

    std::array<char, kHostnameBufferSize> buf{};

    if (::gethostname(buf.data(), buf.size()) != 0) {
        res.error = errno_message("gethostname");
        return res;
    }

    // Ensure null termination if truncated
    buf[kHostnameBufferSize - 1] = '\0';

    res.hostname.assign(buf.data());
    if (res.hostname.empty()) {
        res.error = "gethostname returned empty name";
        return res;
    }

    res.ok = true;
    return res;
}
