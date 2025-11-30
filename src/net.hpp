#pragma once

#include <string>
#include <string_view>

struct Endpoint {
	std::string host;
	std::string port;
};

// Parse "host:port". On success, fills `out` and returns true.
// On failure, returns false and sets `error`.
bool parse_endpoint(std::string_view text, Endpoint &out, std::string &error);

class TcpClient {
public:
	explicit TcpClient(Endpoint endpoint);
	~TcpClient();

	TcpClient(const TcpClient &) = delete;
	TcpClient &operator=(const TcpClient &) = delete;

	// Attempts to open a TCP connection to endpoint_. If already connected,
	// returns true immediately. On failure, closes any half-open socket,
	// sets `error`, and returns false.
	bool connect(std::string &error);

	// Closes the socket if open.
	void close() noexcept;

	bool is_connected() const noexcept {
		return sock_ >= 0;
	}

	// Sends the whole buffer. On error, closes the socket and sets `error`.
	bool send_all(std::string_view data, std::string &error);

	// Convenience helper for text protocols: sends `line` followed by '\n'.
	bool send_line(std::string_view line, std::string &error);

private:
	Endpoint endpoint_;
	int sock_;
};
