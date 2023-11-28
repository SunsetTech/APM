#include "Base.hpp"

namespace APM::Scene::RenderDispatcher {
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
