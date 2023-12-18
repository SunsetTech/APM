#pragma once
#include "Base.hpp"
#include "../Object/SpringBundle.hpp"

namespace APM::Scene::RenderDispatcher {
	class SpringBundle: public Base {
		private:
			class Task: public Base::Task {
				private:
					Object::SpringBundle* Bundle;
					
					cl_kernel ScatterKernel, ComputationKernel, GatherKernel;
					
					cl_uint SpacetimeBounds[2];
					cl_mem SpringParametersCL, NodeParametersCL, SpacetimeCL, SpacetimeBoundsCL, InputPositionsCL, InputBuffersCL, OutputPositionsCL, OutputBuffersCL;
					
					Spring_NodeState* SpacetimeBuffer;
					Spring_PrecisionType *InputBuffers, *OutputBuffers;
					cl_uint *InputPositions, *OutputPositions;
				
					void SetupBoundsBuffer();
					void SetupParameterBuffers();
					void SetupSpacetimeBuffer();
					void SetupInputBuffers();
					void SetupOutputBuffers();
				
				public:
					Task(cl_context Context, cl_command_queue Queue, cl_kernel ScatterKernel, cl_kernel ComputationKernel, cl_kernel GatherKernel, size_t MaxBlockSize, Object::SpringBundle* Bundle);
					
					void EnqueueFlushMemory(cl_uint BlockSize, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					void EnqueueExecution(cl_uint StartIteration, cl_uint BlockSize, float TimeDelta, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					void EnqueueReadyMemory(cl_uint BlockSize, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) override;
					
					float GetSourceValue(size_t ID, size_t Timestep) override;
					void SetSinkValue(size_t ID, size_t Timestep, float Value) override;
					
					~Task();
			};
			
			cl_program Program;
			cl_kernel ScatterKernel, ComputationKernel, GatherKernel;
		
		public:
			SpringBundle(cl_context Context, cl_device_id Device, cl_command_queue Queue);
			bool Handles(Object::Base* Object) override;
			Base::Task* CreateTask(Object::Base* Object, cl_uint MaxBlockSize) override;
			~SpringBundle();
	};
}
