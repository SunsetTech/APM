#include <typeinfo>

#include <array>
#include <cstring>
#include <cstdio>
#include "../../CLUtils.hpp"
#include "Base.hpp"
#include "SpringBundle.hpp"
#include "../Object/SpringBundle.hpp"
#include "../../Math.hpp"

namespace APM::Scene::RenderDispatcher {
	void SpringBundle::Task::SetupSpacetimeBuffer() {
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
		this->SpacetimeCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Spring_NodeState) * Bundle->BufferLength * 2, this->SpacetimeBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBufferCL in SpringBundle task", Err);
	}
	
	SpringBundle::Task::Task(cl_context Context, cl_command_queue Queue, cl_kernel ScatterKernel, cl_kernel ComputationKernel, cl_kernel GatherKernel, size_t MaxBlockSize, Object::SpringBundle* Bundle): Base::Task(Context, Queue, MaxBlockSize) {
	}
	
	void SpringBundle::Task::EnqueueExecution(cl_uint StartIteration, cl_uint BlockSize, cl_float TimeDelta, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {

	}
	
	void SpringBundle::Task::EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
	}
	
	void SpringBundle::Task::EnqueueFlushMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
	}
	
	float SpringBundle::Task::GetSourceValue(size_t ID, size_t Timestep) { //TODO
		/*Object::SpringBundle::Plug CurrentPlug = this->Bundle->Outputs[ID];
		cl_uint Cursor[] = {CurrentPlug.FiberID, (cl_uint)Timestep, CurrentPlug.NodeID};
		cl_uint Bounds[] = {this->Bundle->FiberCount, 2, this->Bundle->FiberLength};
		cl_uint BufferIndex = Math::MapIndex<3>(Cursor, Bounds);
		float Position = this->SpacetimeBuffer[BufferIndex].Position;
		return Position - CurrentPlug.Center;*/
	}
	
	void SpringBundle::Task::SetSinkValue(size_t ID, size_t Timestep, float Value) { //TODO
		/*Object::SpringBundle::Plug CurrentPlug = this->Bundle->Inputs[ID];
		cl_uint Cursor[] = {CurrentPlug.FiberID, (cl_uint)Timestep, CurrentPlug.NodeID};
		cl_uint Bounds[] = {this->Bundle->FiberCount, 2, this->Bundle->FiberLength};
		this->SpacetimeBuffer[Math::MapIndex<3>(Cursor, Bounds)].Position = CurrentPlug.Center + Value;*/
	}
	
	SpringBundle::Task::~Task() {
		clReleaseKernel(this->ScatterKernel);
		clReleaseKernel(this->ComputationKernel);
		clReleaseKernel(this->GatherKernel);
		clReleaseMemObject(this->SpacetimeCL);
		clReleaseMemObject(this->SpacetimeBoundsCL);
		clReleaseMemObject(this->SpringParametersCL);
		clReleaseMemObject(this->NodeParametersCL);
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
		this->ScatterKernel = clCreateKernel(Program, "Spring_Bundle_Scatter", &Err);
		this->ComputationKernel = clCreateKernel(Program, "Spring_Bundle_Large", &Err);
		this->GatherKernel = clCreateKernel(Program, "Spring_Bundle_Gather", &Err);
	}

	bool SpringBundle::Handles(Object::Base* Object) {
		return typeid(*Object) == typeid(Object::SpringBundle);
	}
	
	Base::Task* SpringBundle::CreateTask(Object::Base* TaskObject, cl_uint MaxBlockSize) {
		return new SpringBundle::Task(this->Context, this->Queue, this->ScatterKernel, this->ComputationKernel, this->GatherKernel, MaxBlockSize, dynamic_cast<Object::SpringBundle*>(TaskObject));
	}

	SpringBundle::~SpringBundle() {
		clReleaseKernel(this->ScatterKernel);
		clReleaseKernel(this->ComputationKernel);
		clReleaseKernel(this->GatherKernel);
		clReleaseProgram(this->Program);
	}
}
