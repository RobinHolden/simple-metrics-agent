#include "config.hpp"

#include <charconv>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

// Helper to parse integer seconds into std::chrono::seconds
namespace {
bool parse_interval_seconds(std::string_view text, std::chrono::seconds &out,
							std::string &error) {
	long long value = 0;
	const char *first = text.data();
	const char *last = text.data() + text.size();

	auto result = std::from_chars(first, last, value);
	if (result.ec != std::errc{} || result.ptr != last) {
		error = "Invalid value for --interval (expected integer seconds)";
		return false;
	}
	if (value <= 0) {
		error = "Interval must be positive seconds";
		return false;
	}

	out = std::chrono::seconds{value};
	return true;
}
} // namespace

ParseResult parse_args(int argc, char **argv) {
	ParseResult res{};
	AgentConfig cfg{};

	for (int i = 1; i < argc; ++i) {
		const std::string_view arg{argv[i]};

		if (arg == std::string_view{"--help"}
			|| arg == std::string_view{"-h"}) {
			res.status = ParseStatus::Help;
			return res;
		}

		if (arg == std::string_view{"--endpoint"}) {
			if (i + 1 >= argc) {
				res.status = ParseStatus::Error;
				res.error = "Missing value for --endpoint";
				return res;
			}
			cfg.endpoint = argv[++i];
			continue;
		}

		if (arg == std::string_view{"--interval"}) {
			if (i + 1 >= argc) {
				res.status = ParseStatus::Error;
				res.error = "Missing value for --interval";
				return res;
			}
			const std::string_view val{argv[++i]};
			std::chrono::seconds interval{};
			std::string error;
			if (!parse_interval_seconds(val, interval, error)) {
				res.status = ParseStatus::Error;
				res.error = error;
				return res;
			}
			cfg.interval = interval;
			continue;
		}

		if (arg == std::string_view{"--hostname"}) {
			if (i + 1 >= argc) {
				res.status = ParseStatus::Error;
				res.error = "Missing value for --hostname";
				return res;
			}
			cfg.hostname = argv[++i];
			continue;
		}

		// Unknown flag
		res.status = ParseStatus::Error;
		res.error = "Unknown argument: " + std::string(arg);
		return res;
	}

	res.status = ParseStatus::Ok;
	res.config = cfg;
	return res;
}

void print_usage(std::ostream &os) {
	os
		<< "simple-metrics-agent\n"
		<< "Usage:\n"
		<< "  simple-metrics-agent [--endpoint HOST:PORT] [--interval SECONDS] "
		   "[--hostname NAME]\n"
		<< "\n"
		<< "Options:\n"
		<< "  --endpoint   TCP endpoint to send metrics to (default "
		   "127.0.0.1:9000)\n"
		<< "  --interval   Metrics collection interval in seconds (default 5)\n"
		<< "  --hostname   Override hostname field in metrics payload\n"
		<< "  --help       Show this help and exit\n";
}
