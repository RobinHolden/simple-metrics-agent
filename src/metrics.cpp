#include "metrics.hpp"

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <system_error>

#include <stdlib.h> // getloadavg

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/sysctl.h>
#endif

#include "posix_error.hpp"

namespace {

bool collect_load(HostMetrics &out, std::string &error) {
	std::array<double, 3> loads{};

	const int n_samples = getloadavg(loads.data(), loads.size());
	if (n_samples < 0) {
		error = errno_message("getloadavg");
		return false;
	}
	if (n_samples >= 1) {
		out.load1 = loads[0];
	}
	if (n_samples >= 2) {
		out.load5 = loads[1];
	}
	if (n_samples >= 3) {
		out.load15 = loads[2];
	}

	return true;
}

#ifdef __linux__

bool collect_memory_linux(HostMetrics &out, std::string &error) {
	struct sysinfo info{};
	if (sysinfo(&info) != 0) {
		error = errno_message("sysinfo");
		return false;
	}

	const std::uint64_t unit = info.mem_unit ? info.mem_unit : 1;

	out.mem_total_bytes = info.totalram * unit;
	out.mem_free_bytes = info.freeram * unit;

	return true;
}

#endif // __linux__

#ifdef __APPLE__

bool collect_memory_macos(HostMetrics &out, std::string &error) {
	// Total memory
	std::uint64_t memsize = 0;
	std::size_t len = sizeof(memsize);

	if (sysctlbyname("hw.memsize", &memsize, &len, nullptr, 0) != 0) {
		error = errno_message("sysctlbyname(hw.memsize)");
		return false;
	}

	// Free memory (free + inactive)
	mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
	vm_statistics64_data_t vmstat{};
	host_info64_t info = reinterpret_cast<host_info64_t>(&vmstat);
	const kern_return_t kern_result =
		host_statistics64(mach_host_self(), HOST_VM_INFO64, info, &count);
	if (kern_result != KERN_SUCCESS) {
		error = "host_statistics64 failed";
		return false;
	}

	vm_size_t page_size = 0;
	if (host_page_size(mach_host_self(), &page_size) != KERN_SUCCESS) {
		error = "host_page_size failed";
		return false;
	}

	const std::uint64_t free_pages =
		static_cast<std::uint64_t>(vmstat.free_count) + vmstat.inactive_count;

	const std::uint64_t free_bytes = free_pages * page_size;

	out.mem_total_bytes = memsize;
	out.mem_free_bytes = free_bytes;

	return true;
}

#endif // __APPLE__

} // namespace

bool collect_host_metrics(HostMetrics &out, std::string &error) {
	if (!collect_load(out, error)) {
		return false;
	}

#ifdef __linux__
	if (!collect_memory_linux(out, error)) {
		return false;
	}
#elif defined(__APPLE__)
	if (!collect_memory_macos(out, error)) {
		return false;
	}
#endif

	return true;
}
