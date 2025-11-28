#pragma once

#include <chrono>
#include <iosfwd>
#include <string>

struct AgentConfig {
	std::string endpoint = "127.0.0.1:9000";
	std::chrono::seconds interval{5};
	std::string hostname;
};

enum class ParseStatus {
	Ok,
	Help,
	Error,
};

struct ParseResult {
	ParseStatus status{ParseStatus::Ok};
	AgentConfig config{}; // meaningful only if status == Ok
	std::string error;	  // meaningful only if status == Error
};

ParseResult parse_args(int argc, char **argv);

void print_usage(std::ostream &os);