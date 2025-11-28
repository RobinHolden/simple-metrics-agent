#pragma once

#include "config.hpp"

class Agent {
public:
    explicit Agent(AgentConfig config);

    // Runs the main loop until a stop signal is received.
    // Returns 0 on clean shutdown.
    int run() const;

private:
    AgentConfig config_;
};
