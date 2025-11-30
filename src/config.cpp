#include "config.hpp"

#include <charconv>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

namespace opts {
inline constexpr std::string_view help_long = "--help";
inline constexpr std::string_view help_short = "-h";
inline constexpr std::string_view endpoint = "--endpoint";
inline constexpr std::string_view interval = "--interval";
inline constexpr std::string_view hostname = "--hostname";
} // namespace opts

namespace {

// Helper for consistent error text
std::string missing_value_for(std::string_view flag) {
	std::string msg = "Missing value for ";
	msg += flag;
	return msg;
}

// Helper to parse integer seconds into std::chrono::seconds
bool parse_interval_seconds(std::string_view text, std::chrono::seconds &out,
							std::string &error) {
	long long value = 0;
	const char *first = text.data();
	const char *last = text.data() + text.size();

	auto result = std::from_chars(first, last, value);
	if (result.ec != std::errc{} || result.ptr != last) {
		error = "Invalid value for ";
		error += opts::interval;
		error += " (expected integer seconds)";
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

		if (arg == opts::help_long || arg == opts::help_short) {
			res.status = ParseStatus::Help;
			return res;
		}

		if (arg == opts::endpoint) {
			if (i + 1 >= argc) {
				res.status = ParseStatus::Error;
				res.error = missing_value_for(opts::endpoint);
				return res;
			}
			cfg.endpoint = argv[++i];
			continue;
		}

		if (arg == opts::interval) {
			if (i + 1 >= argc) {
				res.status = ParseStatus::Error;
				res.error = missing_value_for(opts::interval);
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

		if (arg == opts::hostname) {
			if (i + 1 >= argc) {
				res.status = ParseStatus::Error;
				res.error = missing_value_for(opts::hostname);
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
	const AgentConfig defaults{};

	os << "simple-metrics-agent\n"
	   << "Usage:\n"
	   << "  simple-metrics-agent [" << opts::endpoint << " HOST:PORT] ["
	   << opts::interval << " SECONDS] [" << opts::hostname << " NAME]\n"
	   << "\n"
	   << "Options:\n"
	   << "  " << opts::endpoint
	   << "   TCP endpoint to send metrics to (default " << defaults.endpoint
	   << ")\n"
	   << "  " << opts::interval
	   << "   Metrics collection interval in seconds (default "
	   << defaults.interval.count() << ")\n"
	   << "  " << opts::hostname
	   << "   Override hostname field in metrics payload\n"
	   << "  " << opts::help_long << ", " << opts::help_short
	   << "       Show this help and exit\n";
}
