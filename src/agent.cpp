#include "agent.hpp"

#include <chrono>
#include <csignal>
#include <format>
#include <iostream>
#include <thread>
#include <utility>

#include "config.hpp"
#include "metrics.hpp"
#include "net.hpp"

namespace {

volatile std::sig_atomic_t g_signal_status = 0;

void handle_signal(int signum) {
	g_signal_status = signum;
}

void install_signal_handlers() {
	(void)std::signal(SIGINT, handle_signal);
	(void)std::signal(SIGTERM, handle_signal);
#ifdef SIGPIPE
	(void)std::signal(SIGPIPE, SIG_IGN); // send() returns EPIPE instead
#endif
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

	Endpoint endpoint{};

	std::string error;
	if (!parse_endpoint(config_.endpoint, endpoint, error)) {
		std::cerr << "Invalid endpoint '" << config_.endpoint << "': " << error
				  << '\n';
		return 1;
	}

	TcpClient client{endpoint};

	install_signal_handlers();
	std::cout << "Press Ctrl+C to stop (SIGINT) or send SIGTERM.\n";

	const auto interval = config_.interval;
	while (!should_stop()) {
		HostMetrics metrics{};
		if (!collect_host_metrics(metrics, error)) {
			std::cerr << "[metrics] collect_host_metrics failed: " << error
					  << '\n';
		} else {
			const auto now = std::chrono::system_clock::now();

			const std::string json_line = std::format(
				R"({{"hostname":"{}","load1":{:.2f},"load5":{:.2f},"load15":{:.2f},"mem_total":{},"mem_free":{}}})",
				config_.hostname, metrics.load1, metrics.load5, metrics.load15,
				metrics.mem_total_bytes, metrics.mem_free_bytes);

			std::cout << std::format("[metrics] t={:%F %T} {}\n", now,
									 json_line);

			// Lazy reconnect: if not connected, try to connect before sending
			if (!client.connect(error)) {
				std::cerr << "[tcp] connect failed: " << error << '\n';
			} else {
				if (!client.send_line(json_line, error)) {
					std::cerr << "[tcp] send_line failed: " << error << '\n';
				}
			}
		}

		std::this_thread::sleep_for(interval);
	}

	std::cout << "Shutting down due to signal " << g_signal_status << '\n';
	return 0;
}
