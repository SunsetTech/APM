#include <typeinfo>

#include <array>
#include <cstring>
#include <cstdio>
#include "../../CLUtils.hpp"
#include "Base.hpp"
#include "SpringBundle.hpp"
#include "../Object/SpringBundle.hpp"
#include "../../Math.hpp"

/*namespace APM::Scene::RenderDispatcher {
	void SpringBundle::SBTask::SetupSpacetimeBuffer() {
		int SpaceCursor[] = {0, 0};
		int SpacetimeCursor[] = {0, 0, 0};
		unsigned int SpaceBounds[] = {this->Bundle->FiberCount, this->Bundle->FiberLength};
		unsigned int SpacetimeBounds[] = {this->Bundle->FiberCount, 2, this->Bundle->FiberLength};
		this->SpacetimeBuffer = new Spring_NodeState[this->Bundle->BufferLength * 2];
		for (unsigned int FiberIndex = 0; FiberIndex < this->Bundle->FiberCount; FiberIndex++) {
			SpaceCursor[0] = FiberIndex;
			SpacetimeCursor[0] = FiberIndex;
			
			std::memcpy(
				this->SpacetimeBuffer + Math::MapIndex<3>(SpacetimeCursor, SpacetimeBounds), 
				this->Bundle->SpaceBuffer + Math::MapIndex<2>(SpaceCursor, SpaceBounds),
				this->Bundle->FiberLength*sizeof(Spring_NodeState)
			);
		}
		
		cl_int Err;
		this->SpacetimeBufferCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Spring_NodeState) * Bundle->BufferLength * 2, this->SpacetimeBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBufferCL in SpringBundle task", Err);
	}
	
	SpringBundle::SBTask::SBTask(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::SpringBundle* Bundle): Task(Context, Queue, Kernel) {
		this->Bundle = Bundle;
		this->SpacetimeBounds[0] = 2;
		this->SpacetimeBounds[1] = Bundle->FiberLength;
		cl_int Err;
		this->NodeParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Spring_NodeParameters) * Bundle->BufferLength, Bundle->NodeParameterBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating NodeParameterBufferCL in SpringBundle task", Err);
		this->SpringParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Spring_SpringParameters) * Bundle->BufferLength, Bundle->SpringParameterBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpringParameterBufferCL in SpringBundle task", Err);
		this->SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 2, this->SpacetimeBounds, &Err);
		CLUtils::PrintAndHaltIfError("fuck off", Err);
		printf("Waaaa %p %p %p\n", this, Context, this->Context);
		this->SetupSpacetimeBuffer();
	}
	
	void SpringBundle::SBTask::EnqueueExecution(cl_float TimeDelta, cl_uint Timestep, cl_uint Iterations, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		const CLUtils::ArgumentDefintion Arguments[] = {
			{sizeof(cl_mem), &SpringParameterBufferCL},
			{sizeof(cl_mem), &NodeParameterBufferCL},
			{sizeof(cl_uint), &this->Bundle->FiberCount},
			{sizeof(cl_mem), &SpacetimeBufferCL},
			{sizeof(cl_mem), &SpacetimeBoundsCL},
			{sizeof(cl_uint), &Timestep},
			{sizeof(cl_float), &TimeDelta},
		};
		
		cl_int Err = CLUtils::SetKernelArguments(this->Kernel, std::size(Arguments), Arguments);
		CLUtils::PrintAndHaltIfError("Setting kernel arguments in SpringBundle task", Err);
		
		size_t GlobalSize[] = {this->Bundle->FiberCount, this->Bundle->FiberLength};
		cl_event* ExecutionEvents = new cl_event[Iterations+1];
		clEnqueueMarkerWithWaitList(this->Queue, WaitEventCount, WaitEvents, ExecutionEvents+0);
		for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
			cl_uint EventIndex = Iteration+1;
			cl_uint LocalTimestep = (Timestep+Iteration)%2;
			clSetKernelArg(this->Kernel, 5, sizeof(cl_uint), &LocalTimestep);
			Err = clEnqueueNDRangeKernel(this->Queue, this->Kernel, 2, NULL, GlobalSize, NULL, 1, ExecutionEvents+EventIndex-1, ExecutionEvents+EventIndex);
			CLUtils::PrintAndHaltIfError("Enqueuing execution in SpringBundle task", Err);
		}
		CLUtils::ReleaseEvents(Iterations, ExecutionEvents);
		*CompletionEvent = ExecutionEvents[Iterations];
		delete[] ExecutionEvents;
	}
	
	void SpringBundle::SBTask::EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_int Err = clEnqueueReadBuffer(
			this->Queue,
			this->SpacetimeBufferCL,
			false,
			0, sizeof(Spring_NodeState) * 2 * this->Bundle->BufferLength,
			this->SpacetimeBuffer,
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError("awoo",Err);
	}
	
	void SpringBundle::SBTask::EnqueueFlushMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_int Err = clEnqueueWriteBuffer(
			this->Queue,
			this->SpacetimeBufferCL,
			false,
			0, sizeof(Spring_NodeState) * 2 * this->Bundle->BufferLength,
			this->SpacetimeBuffer,
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError("bark", Err);
	}
	
	float SpringBundle::SBTask::GetSourceValue(size_t ID, size_t Timestep) { //TODO
		Object::SpringBundle::Plug CurrentPlug = this->Bundle->Outputs[ID];
		cl_uint Cursor[] = {CurrentPlug.FiberID, (cl_uint)Timestep, CurrentPlug.NodeID};
		cl_uint Bounds[] = {this->Bundle->FiberCount, 2, this->Bundle->FiberLength};
		cl_uint BufferIndex = Math::MapIndex<3>(Cursor, Bounds);
		float Position = this->SpacetimeBuffer[BufferIndex].Position;
		return Position - CurrentPlug.Center;
	}
	
	void SpringBundle::SBTask::SetSinkValue(size_t ID, size_t Timestep, float Value) { //TODO
		Object::SpringBundle::Plug CurrentPlug = this->Bundle->Inputs[ID];
		cl_uint Cursor[] = {CurrentPlug.FiberID, (cl_uint)Timestep, CurrentPlug.NodeID};
		cl_uint Bounds[] = {this->Bundle->FiberCount, 2, this->Bundle->FiberLength};
		this->SpacetimeBuffer[Math::MapIndex<3>(Cursor, Bounds)].Position = CurrentPlug.Center + Value;
	}
	
	SpringBundle::SBTask::~SBTask() {
		clReleaseMemObject(this->SpacetimeBufferCL);
		clReleaseMemObject(this->SpacetimeBoundsCL);
		clReleaseMemObject(this->SpringParameterBufferCL);
		clReleaseMemObject(this->NodeParameterBufferCL);
		clReleaseCommandQueue(Queue);
		clReleaseContext(Context);
	}
	
	SpringBundle::SpringBundle(cl_context Context, cl_device_id Device, cl_command_queue Queue): Base(Context, Device, Queue) {
		const char* SourcePaths[3] = {
			"OpenCL/Kernels/Spring/Bundle/Large.cl.c",
			"OpenCL/Common/Spring.cl.c",
			"OpenCL/Common/Math.cl.c"
		};
		
		this->Program = CLUtils::CompileProgramFromFiles( //TODO make this a provided var?
			Context, Device, 
			3, SourcePaths,
			"-I OpenCL/Common/ -cl-std=CL2.0"
		);
		
		cl_int Err;
		this->Kernel = clCreateKernel(Program, "Spring_Bundle_Large", &Err);
		CLUtils::PrintAndHaltIfError("arf", Err);
	}

	bool SpringBundle::Handles(Object::Base* Object) {
		return typeid(*Object) == typeid(Object::SpringBundle);
	}
	
	Base::Task* SpringBundle::CreateTask(Object::Base* TaskObject) {
		return new SpringBundle::SBTask(this->Context, this->Queue, this->Kernel, dynamic_cast<Object::SpringBundle*>(TaskObject));
	}

	SpringBundle::~SpringBundle() {
		clReleaseKernel(this->Kernel);
		clReleaseProgram(this->Program);
	}
}*/
