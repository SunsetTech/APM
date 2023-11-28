#include "ExecutionBin.hpp"
#include "CLUtils.hpp"
#include <cstdio>

namespace APM {
	ExecutionBin::ExecutionBin(cl_device_id Device) {
		cl_int Err;
		this->Device = Device; clRetainDevice(Device);
		this->Context = clCreateContext(NULL, 1, &Device, NULL, NULL, &Err);
		CLUtils::PrintAndHaltIfError(Err);
		this->Queue = clCreateCommandQueue(this->Context, Device, CL_QUEUE_PROFILING_ENABLE, &Err);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	ExecutionBin::~ExecutionBin() {
		for (unsigned int DispatcherIndex = 0; DispatcherIndex < this->Dispatchers.size(); DispatcherIndex++) {
			delete this->Dispatchers[DispatcherIndex];
		}
		clReleaseCommandQueue(this->Queue);
		clReleaseContext(this->Context);
		clReleaseDevice(this->Device);
	};
}
