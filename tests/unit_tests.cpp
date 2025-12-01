// tests/unit_tests.cpp
#include "config.hpp"
#include "net.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {

// Small helper used in each test: build argc/argv from a vector<string>
// Returns the vector that backs argv.
[[nodiscard]] std::vector<char *> make_argv(std::vector<std::string> &args,
											int &argc_out, char **&argv_out) {
	std::vector<char *> argv_buffer;
	argv_buffer.reserve(args.size());
	for (auto &arg : args) {
		argv_buffer.push_back(arg.data());
	}
	argc_out = static_cast<int>(argv_buffer.size());
	argv_out = argv_buffer.data();
	return argv_buffer;
}

void test_default_config() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Ok);
	assert(res.config.endpoint == "127.0.0.1:9090");
	assert(res.config.interval.count() == 5);
	assert(res.config.hostname.empty());
}

void test_help_flag_long() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"--help",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Help);
}

void test_help_flag_short() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"-h",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Help);
}

void test_endpoint_override() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"--endpoint",
		"example.com:1234",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Ok);
	assert(res.config.endpoint == "example.com:1234");
}

void test_interval_override() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"--interval",
		"10",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Ok);
	assert(res.config.interval.count() == 10);
}

void test_bad_interval_non_numeric() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"--interval",
		"abc",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Error);
	assert(!res.error.empty());
}

void test_bad_interval_negative() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"--interval",
		"-5",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Error);
	assert(!res.error.empty());
}

void test_unknown_flag() {
	std::vector<std::string> args = {
		"simple-metrics-agent",
		"--does-not-exist",
	};
	int argc = 0;
	char **argv = nullptr;
	const auto argv_buffer = make_argv(args, argc, argv);

	const ParseResult res = parse_args(argc, argv);
	assert(res.status == ParseStatus::Error);
	assert(!res.error.empty());
}

void test_parse_endpoint_ok() {
	Endpoint ep{};
	std::string error;

	bool ok = parse_endpoint("localhost:8080", ep, error);
	assert(ok);
	assert(error.empty());
	assert(ep.host == "localhost");
	assert(ep.port == "8080");

	ok = parse_endpoint("example.com:443", ep, error);
	assert(ok);
	assert(error.empty());
	assert(ep.host == "example.com");
	assert(ep.port == "443");
}

void test_parse_endpoint_errors() {
	Endpoint ep{};
	std::string error;

	bool ok = parse_endpoint("noport", ep, error);
	assert(!ok);
	assert(!error.empty());

	ok = parse_endpoint("host:", ep, error);
	assert(!ok);

	ok = parse_endpoint(":8080", ep, error);
	assert(!ok);

	ok = parse_endpoint("host:abc", ep, error);
	assert(!ok);
}

} // namespace

int main() {
	try {
		test_default_config();
		test_help_flag_long();
		test_help_flag_short();
		test_endpoint_override();
		test_interval_override();
		test_bad_interval_non_numeric();
		test_bad_interval_negative();
		test_unknown_flag();
		test_parse_endpoint_ok();
		test_parse_endpoint_errors();

		std::cout << "All unit tests passed\n";
		return 0;
	} catch (const std::exception &ex) {
		std::cerr << "Unit tests threw exception: " << ex.what() << '\n';
	} catch (...) {
		std::cerr << "Unit tests threw non std::exception\n";
	}

	return 1;
}
