#include "Utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace Utils {
	char* ReadFile(const char* Filename, bool AddNullTerminator, size_t* Length) { //thanks to whoever chatgpt stole this from
		FILE* File = fopen(Filename, "r");
		if (File == NULL) {
			fprintf(stderr, "Unable to open File %s\n", Filename);
			return NULL;
		}

		fseek(File, 0, SEEK_END);
		*Length = ftell(File);
		fseek(File, 0, SEEK_SET);

		char* Content = (char*)malloc(*Length + (AddNullTerminator ? 1 : 0)); // +1 for null terminator
		if (Content == NULL) {
			fclose(File);
			fprintf(stderr, "Memory allocation failed\n");
			return NULL;
		}

		size_t BytesRead = fread(Content, 1, *Length, File);
		if (BytesRead != *Length) {
			fclose(File);
			free(Content);
			fprintf(stderr, "Error reading File\n");
			return NULL;
		}
		
		if (AddNullTerminator) {
			Content[*Length] = '\0';
		}
		
		fclose(File);
		return Content;
	}
	
	namespace Time {
		long long Nanoseconds() {
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts); // CLOCK_MONOTONIC provides a monotonically increasing clock
			return ts.tv_sec * 1000000000LL + ts.tv_nsec; // Calculate the timestamp in nanoseconds
		}

		long long Milliseconds() {
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
		}

		long long Microseconds() {
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts); // CLOCK_MONOTONIC provides a monotonically increasing clock
			return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL; // Calculate the timestamp in microseconds
		}
	}
}
