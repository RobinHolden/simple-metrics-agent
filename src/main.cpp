#include <iostream>

#include "agent.hpp"
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

	const Agent agent{res.config};
	return agent.run();
}
