#pragma once

#include <cstddef>

namespace Utils {
	char* ReadFile(const char* Filename, bool AddNullTerminator, size_t* Length);
	
	namespace Time {
		long long Nanoseconds();
		long long Milliseconds();
		long long Microseconds();
	}
}
