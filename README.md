# simple-metrics-agent

Small C++ metrics agent that periodically collects host metrics and ships them over TCP as JSON lines.  
Designed to run on Linux and macOS and to talk to a log / metrics pipeline (for example, a [simple TCP log collector](https://github.com/RobinHolden/simple-log-pipeline)).

## Features

- POSIX-style daemon process (foreground binary) with clean shutdown on signals
- Periodic metrics collection:
  - 1 / 5 / 15 minute load average (`getloadavg`)
  - total and free memory (Linux: `sysinfo`, macOS: `sysctl` + `host_statistics64`)
- Simple, text-based protocol:
  - JSON line per sample, one per interval
  - includes hostname, load averages, and memory stats
- TCP client with lazy reconnect:
  - parses `host:port`
  - reconnects automatically when the connection drops
- Minimal CLI:
  - override endpoint, interval, and hostname from the command line
- Small test binary (no frameworks) covering argument parsing and endpoint parsing

The goal is to have a compact, readable example of a small POSIX agent written in modern C++.

## Requirements

- CMake 3.20+
- A C++20 compiler
  - Clang or GCC on Linux
  - Apple Clang on macOS

On Linux, the code uses `getloadavg` and `sysinfo`.  
On macOS, it uses `getloadavg`, `sysctlbyname("hw.memsize")`, and `host_statistics64`.

## Build

From the project root:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

This produces:

- `simple-metrics-agent` – main agent binary
- `unit_tests` – small test binary

Both are built with C++20 and, for Clang/GCC, a reasonably strict warning set (`-Wall -Wextra -Wpedantic` and others).

## Usage

Basic usage:

```sh
./simple-metrics-agent
```

Default configuration:

- endpoint: `127.0.0.1:9090`
- interval: `5` seconds
- hostname: auto-detected from `gethostname()`

You can override these with flags:

```sh
./simple-metrics-agent \
  --endpoint example.com:1234 \
  --interval 10 \
  --hostname my-host
```

Available options:

- `--endpoint HOST:PORT`  
  TCP endpoint to send JSON lines to (default `127.0.0.1:9090`).
- `--interval SECONDS`  
  Metrics collection interval in seconds (default `5`).
- `--hostname NAME`  
  Override auto-detected hostname.
- `--help`, `-h`  
  Print usage and exit.

### Output format

Every interval, the agent collects metrics and sends a single JSON line over TCP.  
It also logs the same line to stdout for visibility.

Example JSON payload:

```json
{"hostname":"my-host","load1":0.42,"load5":0.37,"load15":0.35,"mem_total":16777216000,"mem_free":5238784000}
```

Fields:

- `hostname` – detected or overridden hostname
- `load1`, `load5`, `load15` – load averages
- `mem_total` – total memory in bytes
- `mem_free` – free (or free+inactive) memory in bytes, depending on platform

### Example with a local TCP server

You can inspect the JSON output using `nc` as a simple listener:

Terminal 1:

```sh
nc -l 9090
```

Terminal 2 (from `build/`):

```sh
./simple-metrics-agent --endpoint 127.0.0.1:9090
```

You should see one JSON line per interval in the `nc` terminal.

### Shutdown

The agent installs simple POSIX signal handlers:

- `SIGINT` (Ctrl+C)
- `SIGTERM`
- `SIGPIPE` is ignored so that failed sends report `EPIPE` instead of terminating the process.

On `SIGINT` or `SIGTERM`, the main loop stops after the current sleep/iteration, and the TCP socket is closed.

## Tests

There is a small test binary that exercises the “edges” of configuration, not the OS-dependent metrics collection:

- argument parsing (`parse_args`)
- endpoint parsing (`parse_endpoint`)

From the `build/` directory:

```sh
ctest
```

or directly:

```sh
./unit_tests
```

The tests are written with plain `assert` in `tests/unit_tests.cpp`, no additional framework required.

## Architecture overview

The project is split into small, focused translation units:

- `src/main.cpp`  
  Parses arguments, prints usage on `--help` or errors, and hands off to `Agent`.

- `src/agent.*`  
  Core runtime loop:
  - installs signal handlers
  - resolves hostname via `detect_hostname()` (unless overridden)
  - parses the `host:port` endpoint
  - periodically:
  	- calls `collect_host_metrics`
    - formats a JSON line using `std::format`
    - logs to stdout
    - connects and sends to the TCP endpoint via `TcpClient`
  - exits cleanly on signal

- `src/config.*`  
  Holds `AgentConfig` and CLI parsing logic:
  - `--endpoint`, `--interval`, `--hostname`, `--help`
  - uses `std::from_chars` to parse the interval as seconds
  - validates integer, positive interval values
  - provides a `print_usage(std::ostream&)` helper

- `src/metrics.*`  
  Platform-specific metrics collection:
  - load average via `getloadavg`
  - memory stats via:
    - Linux: `sysinfo`
    - macOS: `sysctlbyname("hw.memsize")` + `host_statistics64` + `host_page_size`
  - returns a `HostMetrics` struct and an error string on failure

- `src/net.*`  
  Simple POSIX TCP client:
  - `parse_endpoint("host:port")` with basic validation
  - `TcpClient` with:
    - lazy `connect` using `getaddrinfo` and `connect`
    - `send_all` loop with `send`, handling `EINTR` and short writes
    - `send_line` helper for newline-terminated messages

- `src/hostname.*`  
  Thin wrapper around `gethostname` that:
  - uses a fixed-size `std::array<char, 256>`
  - guarantees null termination
  - returns a `HostnameResult` struct with `ok`, `hostname`, and `error`

- `src/posix_error.hpp`  
  Small helper to turn `errno` into a readable `std::string`.

Tests live in:

- `tests/unit_tests.cpp`  
  Self-contained, links directly against `config.cpp` and `net.cpp`.

## Limitations and possible extensions

This project is intentionally small and focused. Obvious directions for future work:

- Make the JSON payload extensible (more metrics, tags, or nested objects)
- Add simple retry/backoff strategy around TCP reconnection
- Make configuration pluggable (for example, parse a small TOML/YAML file)
- Support more platforms (for example, Windows) behind the same `HostMetrics` interface
- Add a structured logger instead of writing directly to `std::cout` / `std::cerr`

For now, the project is meant as a compact example of:

- modern C++20 used in a POSIX context
- small agent architecture:
  - config parsing
  - signal handling
  - OS metrics
  - TCP networking
- a testable design where non-OS parts are covered by small unit tests.
