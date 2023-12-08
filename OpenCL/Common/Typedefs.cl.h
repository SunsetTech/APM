#pragma once

#ifdef __OPENCL_C_VERSION__ //we use these to ensure portability across hosts of the host included header files
	typedef float cl_float;
	typedef int cl_int;
	typedef unsigned int cl_uint;
	typedef bool cl_bool;
#else
#endif
