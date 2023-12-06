#define CL_TARGET_OPENCL_VERSION 200
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
	void PrintAndHaltIfError(const char* Activity, cl_int Err);
	cl_program CompileProgramFromFiles(cl_context Context, cl_device_id Device, unsigned int SourceCount, const char** SourcePaths, const char* BuildFlags);
}
