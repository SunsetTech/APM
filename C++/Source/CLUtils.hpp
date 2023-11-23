#include <cstddef>
#include <CL/cl.h>

#pragma once

namespace CLUtils {
	struct ArgumentDefintion {
		const size_t Size;
		const void* Value;
	};
	
	cl_uint SetKernelArguments(cl_kernel Kernel, cl_uint Count, const ArgumentDefintion* Definitions);
	const char* GetErrorName(cl_int Err);
	void PrintAndHaltIfError(cl_int Err);
}
