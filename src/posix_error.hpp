#pragma once

#include <cerrno>
#include <string>
#include <system_error>

inline std::string errno_message(const char *what) {
	const std::error_code ec(errno, std::generic_category());
	std::string msg = what;
	msg += ": ";
	msg += ec.message();
	return msg;
}
