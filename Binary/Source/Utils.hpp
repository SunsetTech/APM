#pragma once

#include <cstddef>

namespace Utils {
	char* ReadFile(const char* Filename, bool AddNullTerminator, size_t* Length);
	void WriteWAV_File(const char* Filename, unsigned int SampleRate, unsigned char ChannelCount, size_t Length, float** Channels);
	namespace Time {
		long long Nanoseconds();
		long long Milliseconds();
		long long Microseconds();
	}
}
