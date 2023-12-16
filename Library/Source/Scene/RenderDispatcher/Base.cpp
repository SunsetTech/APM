#include "Base.hpp"
#include <cstdio>
namespace APM::Scene::RenderDispatcher {
	Base::Task::Task(cl_context Context, cl_command_queue Queue, size_t MaxBlockSize) {
		this->MaxBlockSize = MaxBlockSize;
		this->Context = Context; clRetainContext(Context);
		this->Queue = Queue; clRetainCommandQueue(Queue);
	}
	
	Base::Task::~Task() {
		clReleaseCommandQueue(this->Queue);
		clReleaseContext(this->Context);
	}

	Base::Base(cl_context Context, cl_device_id Device, cl_command_queue Queue) {
		this->Context = Context; clRetainContext(Context);
		this->Device = Device; clRetainDevice(Device);
		this->Queue = Queue; clRetainCommandQueue(Queue);
	}
	
	Base::~Base() {
		clReleaseCommandQueue(this->Queue);
		clReleaseDevice(this->Device);
		clReleaseContext(this->Context);
	}
}
