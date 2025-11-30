#include "net.hpp"

#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "posix_error.hpp"

bool parse_endpoint(std::string_view text, Endpoint &out, std::string &error) {
	const auto pos = text.rfind(':');
	if (pos == std::string_view::npos || pos == 0 || pos + 1 >= text.size()) {
		error = "expected endpoint in form host:port";
		return false;
	}

	out.host.assign(text.substr(0, pos));
	out.port.assign(text.substr(pos + 1));

	for (const char c : out.port) {
		if (c < '0' || c > '9') {
			error = "port must be a decimal number";
			return false;
		}
	}

	return true;
}

TcpClient::TcpClient(Endpoint endpoint)
	: endpoint_{std::move(endpoint)}, sock_{-1} {}

TcpClient::~TcpClient() {
	close();
}

void TcpClient::close() noexcept {
	if (sock_ >= 0) {
		::close(sock_);
		sock_ = -1;
	}
}

bool TcpClient::connect(std::string &error) {
	if (is_connected()) {
		return true;
	}

	struct addrinfo hints{};
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo *result = nullptr;
	const int rc = ::getaddrinfo(endpoint_.host.c_str(), endpoint_.port.c_str(),
								 &hints, &result);
	if (rc != 0) {
		error = "getaddrinfo failed: ";
		error += ::gai_strerror(rc);
		return false;
	}

	int fd = -1;
	for (const struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
		fd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0) {
			continue;
		}

		if (::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
			sock_ = fd;
			break;
		}

		::close(fd);
	}

	::freeaddrinfo(result);

	if (sock_ < 0) {
		error = errno_message("connect");
		return false;
	}

	return true;
}

bool TcpClient::send_all(std::string_view data, std::string &error) {
	if (!is_connected()) {
		error = "socket is not connected";
		return false;
	}

	const char *buf = data.data();
	std::size_t remaining = data.size();

	while (remaining > 0) {
		const ssize_t n = ::send(sock_, buf, remaining, 0);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			error = errno_message("send");
			close();
			return false;
		}
		if (n == 0) {
			error = "send returned 0 (connection closed)";
			close();
			return false;
		}
		buf += n;
		remaining -= static_cast<size_t>(n);
	}

	return true;
}

bool TcpClient::send_line(std::string_view line, std::string &error) {
	std::string buffer;
	buffer.reserve(line.size() + 1);
	buffer.append(line);
	buffer.push_back('\n');
	return send_all(buffer, error);
}
