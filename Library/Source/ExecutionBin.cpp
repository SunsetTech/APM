#include "ExecutionBin.hpp"
#include "CLUtils.hpp"
#include <cstdio>

namespace APM {
	ExecutionBin::ExecutionBin(cl_device_id Device) {
		cl_int Err;
		this->Device = Device; clRetainDevice(Device);
		this->Context = clCreateContext(NULL, 1, &Device, NULL, NULL, &Err);
		CLUtils::PrintAndHaltIfError("Creating context", Err);
		cl_queue_properties QueueProperties[] = {
			CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
			0
		};
		this->Queue = clCreateCommandQueueWithProperties(this->Context, Device, QueueProperties, &Err);
		CLUtils::PrintAndHaltIfError("Creating command queue", Err);
		/*cl_queue_properties DefaultDeviceQueueProperties[] = {
			CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT,
			0
		};
		clCreateCommandQueueWithProperties(this->Context, Device, DefaultDeviceQueueProperties, &Err);
		CLUtils::PrintAndHaltIfError("Creating default device queue", Err);*/
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
