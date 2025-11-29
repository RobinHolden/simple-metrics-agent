#pragma once

#include <cstdint>
#include <string>

struct HostMetrics {
	double load1{0.0};
	double load5{0.0};
	double load15{0.0};

	std::uint64_t mem_total_bytes{0};
	std::uint64_t mem_free_bytes{0};
};

// Returns true on success. On failure, returns false and sets `error`
bool collect_host_metrics(HostMetrics &out, std::string &error);
