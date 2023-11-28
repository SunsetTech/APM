#pragma once

#include "Base.hpp"

namespace APM::Scene::RenderDispatcher {
	class WaveStructure: public Base {
		void Prepare(cl_context Context, cl_device_id Device, cl_command_queue Queue) override;
		bool Handles(Object::Base* Object) override;
		Task* CreateTask(cl_context Context, cl_device_id Device, cl_command_queue Queue, Object::Base *Object) override;
		void Cleanup(cl_context Context, cl_device_id Device, cl_command_queue Queue) override;
	};
}
