#pragma once

#include <CL/cl.h>

#include "../Object/Base.hpp"

namespace APM::Scene::RenderDispatcher {
	class Base {
		public:
			class Task {
				protected:
					cl_context Context;
					cl_command_queue Queue;
					cl_kernel Kernel;
					
				public:
					Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel);
					virtual void EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint Iterations, cl_uint WaitEventCount, const cl_event* WaitEvents, cl_event* CompletionEvent) =0;
					virtual void EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event* WaitEvents, cl_event* CompletionEvent) =0;
					virtual void EnqueueFlushMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event* WaitEvents, cl_event* CompletionEvent) =0;
					virtual void SetSinkValue(size_t ID, size_t Timestep, float Value) =0;
					virtual float GetSourceValue(size_t ID, size_t Timestep) =0;
					virtual ~Task();
			};
			
			cl_context Context;
			cl_device_id Device;
			cl_command_queue Queue;
			
			Base(cl_context Context, cl_device_id Device, cl_command_queue Queue);
			virtual bool Handles(Object::Base* Object) =0;
			virtual Task* CreateTask(Object::Base* Object) =0;
			virtual ~Base();
	};
}
