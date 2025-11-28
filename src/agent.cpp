#include "agent.hpp"

#include <chrono>
#include <csignal>
#include <ctime>
#include <iostream>
#include <thread>
#include <utility>
#include <format>

#include "config.hpp"

namespace {

volatile std::sig_atomic_t g_signal_status = 0;

void handle_signal(int signum) {
	g_signal_status = signum;
}

void install_signal_handlers() {
	(void)std::signal(SIGINT, handle_signal);
	(void)std::signal(SIGTERM, handle_signal);
}

bool should_stop() {
	return g_signal_status != 0;
}

} // namespace

Agent::Agent(AgentConfig config) : config_{std::move(config)} {}

int Agent::run() const {
	std::cout << "simple-metrics-agent starting\n";
	std::cout << "  endpoint: " << config_.endpoint << '\n';
	std::cout << "  interval: " << config_.interval.count() << "s\n";
	if (!config_.hostname.empty()) {
		std::cout << "  hostname: " << config_.hostname << '\n';
	} else {
		std::cout << "  hostname: (auto-detect later)\n";
	}

	install_signal_handlers();
	std::cout << "Press Ctrl+C to stop (SIGINT) or send SIGTERM.\n";

	const auto interval = config_.interval;
	while (!should_stop()) {
		const auto now = std::chrono::system_clock::now();
		std::cout << std::format("[tick] interval elapsed at {:%F %T}\n", now);
		std::this_thread::sleep_for(interval);
	}

	std::cout << "Shutting down due to signal " << g_signal_status << '\n';

	return 0;
}
