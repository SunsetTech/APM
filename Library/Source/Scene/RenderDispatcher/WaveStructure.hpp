#pragma once

#include "Base.hpp"
#include "../Object/WaveStructure.hpp"

namespace APM::Scene::RenderDispatcher {
	class WaveStructure: public Base {
		private:
			class Task: public Base::Task {
				private:
					Object::WaveStructure* Structure;
					Wave_PrecisionType* SpacetimeBuffer;
					cl_uint* SpacetimeBounds;
					cl_mem WaveVelocityCL, TransferEfficiencyCL, SpacetimeBufferCL, SpacetimeBoundsCL;
					void SetupSpacetimeBuffer();
				public:
					Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::WaveStructure* Bundle);
					void EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					void EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					void EnqueueFlushMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					float GetSourceValue(size_t ID, size_t Timestep) override;
					void SetSinkValue(size_t ID, size_t Timestep, float Value) override;
					~Task();
			};
			
			cl_program Program;
			cl_kernel Kernel;
		
		public:
			WaveStructure(cl_context Context, cl_device_id Device, cl_command_queue Queue);
			bool Handles(Object::Base* Object) override;
			Base::Task* CreateTask(Object::Base *Object) override;
			~WaveStructure();
	};
}
