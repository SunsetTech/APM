#pragma once
#include "Base.hpp"
#include "../Object/SpringBundle.hpp"

namespace APM::Scene::RenderDispatcher {
	class SpringBundle: public Base {
		private:
			cl_program Program;
			cl_kernel Kernel;
			class Task: public Base::Task {
				private:
					cl_context Context;
					cl_command_queue Queue;
					cl_kernel Kernel;
					Object::SpringBundle* Bundle;
					cl_uint SpacetimeBounds[2];
					cl_mem SpringParameterBufferCL, NodeParameterBufferCL, SpacetimeBufferCL, SpacetimeBoundsCL;
					Spring_NodeState* SpacetimeBuffer;
					void SetupSpacetimeBuffer();
				public:
					Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::SpringBundle* Bundle);
					void EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					//void EnqueueMapMemory(cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					//void EnqueueUnmapMemory(cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					float GetSourceValue(size_t ID) override;
					void SetSinkValue(size_t ID, float Value) override;
					~Task();
			};
		public:
			SpringBundle(cl_context Context, cl_device_id Device, cl_command_queue Queue);
			bool Handles(Object::Base* Object) override;
			Base::Task* CreateTask(Object::Base* Object) override;
			~SpringBundle();
	};
}
