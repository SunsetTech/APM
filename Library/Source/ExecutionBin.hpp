#pragma once

#include <vector>
#include <CL/cl.h>

#include "Scene/RenderDispatcher/Base.hpp"

namespace APM {
	class ExecutionBin {
		public:
			cl_context Context;
			cl_device_id Device;
			cl_command_queue Queue;
			std::vector<Scene::RenderDispatcher::Base*> Dispatchers;
			
			ExecutionBin(cl_device_id Device);
			template<typename DispatcherType> void AttachDispatcher() {
				this->Dispatchers.push_back(new DispatcherType(this->Context, this->Device, this->Queue));
			}
			~ExecutionBin();
	};
}

