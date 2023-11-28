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
		this->SpacetimeBuffer = (Spring_NodeState*)calloc(this->Bundle->BufferLength*2, sizeof(Spring_NodeState));
		for (unsigned int FiberIndex = 0; FiberIndex < this->Bundle->FiberCount; FiberIndex++) {
			SpaceCursor[0] = FiberIndex;
			SpacetimeCursor[0] = FiberIndex;
			
			std::memcpy(
				this->SpacetimeBuffer + Math::MapIndex<3>(SpacetimeCursor, SpacetimeBounds), 
				this->Bundle->SpaceBuffer + Math::MapIndex<2>(SpaceCursor, SpaceBounds),
				this->Bundle->FiberLength
			);
		}
		
		cl_int Err;
		this->SpacetimeBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(Spring_NodeState) * Bundle->BufferLength * 2, this->SpacetimeBuffer, &Err);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	SpringBundle::Task::Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::SpringBundle* Bundle) {
		this->Context = Context; clRetainContext(Context);
		this->Queue = Queue; clRetainCommandQueue(Queue);
		this->Kernel = Kernel; clRetainKernel(Kernel);
		this->Bundle = Bundle;
		this->SpacetimeBounds[1] = Bundle->FiberLength;
		cl_int Err;
		this->NodeParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Spring_NodeParameters) * Bundle->BufferLength, Bundle->NodeParameterBuffer, &Err);
		this->SpringParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Spring_SpringParameters) * Bundle->BufferLength, Bundle->SpringParameterBuffer, &Err);
		this->SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 2, this->SpacetimeBounds, &Err);
		this->SetupSpacetimeBuffer();
	}
	
	void SpringBundle::Task::EnqueueExecution(cl_float TimeDelta, cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
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
		CLUtils::PrintAndHaltIfError(Err);
		
		size_t GlobalSize[] = {this->Bundle->FiberCount, this->Bundle->FiberLength};
		Err = clEnqueueNDRangeKernel(this->Queue, this->Kernel, 2, NULL, GlobalSize, NULL, WaitEventCount, WaitEvents, CompletionEvent);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	float SpringBundle::Task::GetSourceValue(size_t ID) { //TODO
		return 0;
	}
	
	void SpringBundle::Task::SetSinkValue(size_t ID, float Value) { //TODO
	}
	
	SpringBundle::Task::~Task() {
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
		CLUtils::PrintAndHaltIfError(Err);
	}

	bool SpringBundle::Handles(Object::Base* Object) {
		return typeid(*Object) == typeid(Object::SpringBundle);
	}
	
	Base::Task* SpringBundle::CreateTask(Object::Base* TaskObject) {
		return new SpringBundle::Task(this->Context, this->Queue, this->Kernel, dynamic_cast<Object::SpringBundle*>(TaskObject));
	}

	SpringBundle::~SpringBundle() {
		clReleaseKernel(this->Kernel);
		clReleaseProgram(this->Program);
	}
}
