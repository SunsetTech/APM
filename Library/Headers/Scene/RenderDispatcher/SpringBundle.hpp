#pragma once
#include "Base.hpp"
#include "../Object/SpringBundle.hpp"

namespace APM::Scene::RenderDispatcher {
	class SpringBundle: public Base {
		private:
			class SBTask: public Base::Task {
				private:
					Object::SpringBundle* Bundle;
					cl_uint SpacetimeBounds[2];
					cl_mem SpringParameterBufferCL, NodeParameterBufferCL, SpacetimeBufferCL, SpacetimeBoundsCL;
					Spring_NodeState* SpacetimeBuffer;
					void SetupSpacetimeBuffer();
				
				public:
					SBTask(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::SpringBundle* Bundle);
					void EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					void EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					void EnqueueFlushMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					float GetSourceValue(size_t ID, size_t Timestep) override;
					void SetSinkValue(size_t ID, size_t Timestep, float Value) override;
					~SBTask();
			};
			
			cl_program Program;
			cl_kernel Kernel;
		
		public:
			SpringBundle(cl_context Context, cl_device_id Device, cl_command_queue Queue);
			bool Handles(Object::Base* Object) override;
			Base::Task* CreateTask(Object::Base* Object) override;
			~SpringBundle();
	};
}
