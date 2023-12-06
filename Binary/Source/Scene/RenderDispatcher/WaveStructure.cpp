#include "WaveStructure.hpp"

#include <cstring>
#include <cstdio>
#include <typeinfo>

#include "../../CLUtils.hpp"
#include "../../Math.hpp"
#include <cassert>

namespace APM::Scene::RenderDispatcher {
	void WaveStructure::Task::SetupSpacetimeBuffer() {
		this->SpacetimeBuffer = new Wave_ValueType[this->Structure->BufferLength*2];
		std::memcpy(
			this->SpacetimeBuffer,
			this->Structure->SpaceBuffer,
			this->Structure->BufferLength * sizeof(Wave_ValueType)
		);
		cl_int Err;
		this->SpacetimeBufferCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Wave_ValueType) * this->Structure->BufferLength * 2, this->SpacetimeBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBufferCL in WaveStructure task", Err);
	}
	
	WaveStructure::Task::Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, Object::WaveStructure* Structure): Base::Task(Context, Queue, Kernel) {
		this->Structure = Structure;
		this->SpacetimeBounds = new cl_uint[Structure->Dimensions+1];
		this->SpacetimeBounds[0] = 2;
		for (cl_uint Dimension = 0; Dimension < Structure->Dimensions; Dimension++) {
			this->SpacetimeBounds[Dimension+1] = Structure->SpatialBounds[Dimension];
		}
		cl_int Err;
		this->ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_CellParameters) * Structure->BufferLength, Structure->CellParameterBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating ParameterBufferCL in WaveStructure task", Err);
		this->SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * (Structure->Dimensions+1), this->SpacetimeBounds, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBoundsCL in WaveStructure task", Err);
		this->SetupSpacetimeBuffer();
	}
	
	void WaveStructure::Task::EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		const Wave_ValueType SpaceDelta = 1.0f/10000.0f;
		const CLUtils::ArgumentDefintion Arguments[] = {
			{sizeof(cl_uint), &this->Structure->Dimensions},
			{sizeof(cl_mem), &this->SpacetimeBoundsCL},
			{sizeof(cl_mem), &this->ParameterBufferCL},
			{sizeof(cl_float), &SpaceDelta},
			{sizeof(cl_float), &TimeDelta},
			{sizeof(cl_uint), &Timestep},
			{sizeof(cl_mem), &this->SpacetimeBufferCL},
		};
		
		cl_int Err = CLUtils::SetKernelArguments(this->Kernel, std::size(Arguments), Arguments);
		CLUtils::PrintAndHaltIfError("Setting kernel arguments in WaveStructure task", Err);
		
		size_t GlobalSize[] = {0,0,0};
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			GlobalSize[Dimension] = this->Structure->SpatialBounds[Dimension];
		}
		//cl_event ExecutionEvent;
		Err = clEnqueueNDRangeKernel(this->Queue, this->Kernel, this->Structure->Dimensions, NULL, GlobalSize, NULL, WaitEventCount, WaitEvents, CompletionEvent);
		CLUtils::PrintAndHaltIfError("Enqueuing execution in WaveStructure task", Err);
	}
	
	void WaveStructure::Task::EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		const size_t BufferOffset = Timestep * this->Structure->BufferLength;
		const size_t BufferSizeBytes = this->Structure->BufferLength * sizeof(Wave_ValueType);
		const size_t BufferOffsetBytes = BufferOffset * sizeof(Wave_ValueType);
		const cl_int Err = clEnqueueReadBuffer(
			this->Queue,
			this->SpacetimeBufferCL,
			CL_TRUE,
			BufferOffsetBytes,
			BufferSizeBytes,
			&this->SpacetimeBuffer[BufferOffset],
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError("Enqueuing read buffer in WaveStructure task", Err);
	}
	
	void WaveStructure::Task::EnqueueFlushMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		const size_t BufferOffset = Timestep * this->Structure->BufferLength;
		const size_t BufferSizeBytes = this->Structure->BufferLength * sizeof(Wave_ValueType);
		const size_t BufferOffsetBytes = BufferOffset * sizeof(Wave_ValueType);
		const cl_int Err = clEnqueueWriteBuffer(
			this->Queue,
			this->SpacetimeBufferCL,
			false,
			BufferOffsetBytes, 
			BufferSizeBytes,
			&this->SpacetimeBuffer[BufferOffset],
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError("Enqueuing write buffer in WaveStructure task", Err);
	}
	
	float WaveStructure::Task::GetSourceValue(size_t ID, size_t Timestep) { //TODO
		Object::WaveStructure::Plug CurrentPlug = this->Structure->Outputs[ID];
		
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			Cursor[Dimension+1] = CurrentPlug.Position[Dimension];
		}
		
		float Value = this->SpacetimeBuffer[Math::MapIndex(this->Structure->Dimensions+1, Cursor, this->SpacetimeBounds)];
		
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
		clReleaseMemObject(this->ParameterBufferCL); //We don't delete[] the corresponding buffer because we don't own it
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
		CLUtils::PrintAndHaltIfError("Creating kernel for Wave_1Dto3D_Large",Err);
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
