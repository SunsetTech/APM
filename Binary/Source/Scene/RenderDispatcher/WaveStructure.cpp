#include "WaveStructure.hpp"

#include <cstring>
#include <typeinfo>

#include "../../CLUtils.hpp"
#include "../../Math.hpp"

namespace APM::Scene::RenderDispatcher {
	void WaveStructure::Task::SetupSpacetimeBuffer() {
		this->SpacetimeBuffer = new float[this->Structure->BufferLength*2];
		std::memcpy(
			this->SpacetimeBuffer,
			this->Structure->SpaceBuffer,
			this->Structure->BufferLength * sizeof(float)
		);
		cl_int Err;
		this->SpacetimeBufferCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * this->Structure->BufferLength * 2, this->SpacetimeBuffer, &Err);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	WaveStructure::Task::Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::WaveStructure* Structure) {
		this->Context = Context; clRetainContext(Context);
		this->Queue = Queue; clRetainCommandQueue(Queue);
		this->Kernel = Kernel; clRetainKernel(Kernel);
		this->Structure = Structure;
		this->SpacetimeBounds = new cl_uint[Structure->Dimensions+1];
		this->SpacetimeBounds[0] = 2;
		for (cl_uint Dimension = 0; Dimension < Structure->Dimensions; Dimension++) {
			this->SpacetimeBounds[Dimension+1] = Structure->SpatialBounds[Dimension];
		}
		cl_int Err;
		this->ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_CellParameters) * Structure->BufferLength, Structure->CellParameterBuffer, &Err);
		CLUtils::PrintAndHaltIfError(Err);
		this->SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * (Structure->Dimensions+1), this->SpacetimeBounds, &Err);
		CLUtils::PrintAndHaltIfError(Err);
		this->SetupSpacetimeBuffer();
	}
	
	void WaveStructure::Task::EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		const CLUtils::ArgumentDefintion Arguments[] = {
			{sizeof(cl_mem), &this->ParameterBufferCL},
			{sizeof(cl_mem), &this->SpacetimeBufferCL},
			{sizeof(cl_uint), &Timestep},
			{sizeof(cl_uint), &this->Structure->Dimensions},
			{sizeof(cl_mem), &this->SpacetimeBoundsCL},
		};
		
		cl_int Err = CLUtils::SetKernelArguments(this->Kernel, std::size(Arguments), Arguments);
		CLUtils::PrintAndHaltIfError(Err);
		
		size_t GlobalSize[] = {0,0,0};
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			GlobalSize[Dimension] = this->Structure->SpatialBounds[Dimension];
		}
		cl_event ExecutionEvent;
		Err = clEnqueueNDRangeKernel(this->Queue, this->Kernel, this->Structure->Dimensions, NULL, GlobalSize, NULL, WaitEventCount, WaitEvents, &ExecutionEvent);
		CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(this->Queue, this->SpacetimeBufferCL, false, 0, sizeof(float) * 2 * this->Structure->BufferLength, this->SpacetimeBuffer, 1, &ExecutionEvent, CompletionEvent);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	void WaveStructure::Task::EnqueueReadMemory(cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_int Err = clEnqueueReadBuffer(
			this->Queue,
			this->SpacetimeBufferCL,
			false,
			0, sizeof(float) * 2 * this->Structure->BufferLength,
			this->SpacetimeBuffer,
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	void WaveStructure::Task::EnqueueWriteMemory(cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_int Err = clEnqueueWriteBuffer(
			this->Queue,
			this->SpacetimeBufferCL,
			false,
			0, sizeof(float) * 2 * this->Structure->BufferLength,
			this->SpacetimeBuffer,
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError(Err);
	}
	
	float WaveStructure::Task::GetSourceValue(size_t ID, size_t Timestep) { //TODO
		Object::WaveStructure::Plug CurrentPlug = this->Structure->Inputs[ID];
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			Cursor[Dimension+1] = CurrentPlug.Position[Dimension];
		}
		
		float Value = this->SpacetimeBuffer[Math::MapIndex<3>(Cursor, this->SpacetimeBounds)];
		delete[] Cursor;
		return Value;
	}
	
	void WaveStructure::Task::SetSinkValue(size_t ID, size_t Timestep, float Value) { //TODO
		Object::WaveStructure::Plug CurrentPlug = this->Structure->Inputs[ID];
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			Cursor[Dimension+1] = CurrentPlug.Position[Dimension];
		}
		this->SpacetimeBuffer[Math::MapIndex<3>(Cursor, this->SpacetimeBounds)] = Value;
		delete[] Cursor;
	}
	
	WaveStructure::Task::~Task() {
		delete[] this->SpacetimeBuffer;
		clReleaseMemObject(this->SpacetimeBufferCL);
		delete[] this->SpacetimeBounds;
		clReleaseMemObject(this->SpacetimeBoundsCL);
		clReleaseMemObject(this->ParameterBufferCL);
	}
	
	WaveStructure::WaveStructure(cl_context Context, cl_device_id Device, cl_command_queue Queue): Base(Context, Device, Queue) {
		const char* SourcePaths[3] = {
			"OpenCL/Kernels/Wave/1Dto3D/Large.cl.c",
			"OpenCL/Common/Wave.cl.c",
			"OpenCL/Common/Math.cl.c"
		};
		
		this->Program = CLUtils::CompileProgramFromFiles( //TODO make this a provided var?
			Context, Device, 
			3, SourcePaths,
			"-I OpenCL/Common/ -cl-std=CL2.0"
		);
		
		cl_int Err;
		this->Kernel = clCreateKernel(Program, "Wave_1Dto3D_Large", &Err);
		CLUtils::PrintAndHaltIfError(Err);
	}

	bool WaveStructure::Handles(Object::Base* Object) {
		return typeid(*Object) == typeid(Object::WaveStructure);
	}
	
	Base::Task* WaveStructure::CreateTask(Object::Base* TaskObject) {
		return new WaveStructure::Task(this->Context, this->Queue, this->Kernel, dynamic_cast<Object::WaveStructure*>(TaskObject));
	}

	WaveStructure::~WaveStructure() {
		clReleaseKernel(this->Kernel);
		clReleaseProgram(this->Program);
	}

}
