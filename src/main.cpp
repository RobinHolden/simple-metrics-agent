#include <iostream>

#include "config.hpp"

int main(int argc, char **argv) {
	const ParseResult res = parse_args(argc, argv);

	switch (res.status) {
	case ParseStatus::Help:
		print_usage(std::cout);
		return 0;

	case ParseStatus::Error:
		std::cerr << "Error: " << res.error << '\n';
		print_usage(std::cerr);
		return 1;

	case ParseStatus::Ok:
		break;
	}

	const AgentConfig &cfg = res.config;

	std::cout << "simple-metrics-agent starting\n";
	std::cout << "  endpoint: " << cfg.endpoint << '\n';
	std::cout << "  interval: " << cfg.interval.count() << "s\n";
	if (!cfg.hostname.empty()) {
		std::cout << "  hostname: " << cfg.hostname << '\n';
	} else {
		std::cout << "  hostname: (auto-detect later)\n";
	}

	// TODO:
	// - auto-detect hostname if empty
	// - main loop with sleep and SIGINT/SIGTERM handling
	// - metrics collection and TCP send

	return 0;
}
