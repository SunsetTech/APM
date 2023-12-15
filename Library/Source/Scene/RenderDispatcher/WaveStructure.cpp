#include "WaveStructure.hpp"

#include <cstring>
#include <cstdio>
#include <typeinfo>

#include "../../CLUtils.hpp"
#include "../../Math.hpp"
#include <cassert>

namespace APM::Scene::RenderDispatcher {
	void WaveStructure::Task::SetupSpacetimeBuffer() {
		this->SpacetimeBuffer = new Wave_PrecisionType[this->Structure->BufferLength*2];
		std::memset(
			this->SpacetimeBuffer,
			0,
			this->Structure->BufferLength * 2 * sizeof(Wave_PrecisionType)
		);
		std::memcpy(
			this->SpacetimeBuffer,
			this->Structure->SpaceBuffer,
			this->Structure->BufferLength * sizeof(Wave_PrecisionType)
		);
		cl_int Err;
		this->SpacetimeBufferCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * this->Structure->BufferLength * 2, this->SpacetimeBuffer, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBufferCL in WaveStructure task", Err);
	}
	
	WaveStructure::Task::Task(cl_context Context, cl_command_queue Queue, cl_kernel Kernel, size_t MaxBlockSize, Object::WaveStructure* Structure): Base::Task(Context, Queue, Kernel, MaxBlockSize) {
		this->Structure = Structure;
		this->SpacetimeBounds = new cl_uint[Structure->Dimensions+1];
		this->SpacetimeBounds[0] = 2;
		for (cl_uint Dimension = 0; Dimension < Structure->Dimensions; Dimension++) {
			this->SpacetimeBounds[Dimension+1] = Structure->SpatialBounds[Dimension];
		}
		this->WaveVelocityCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * Structure->BufferLength, Structure->WaveVelocity, nullptr);
		this->TransferEfficiencyCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * Structure->BufferLength, Structure->TransferEfficiency, nullptr);
		this->SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * (Structure->Dimensions+1), this->SpacetimeBounds, nullptr);
		this->SetupSpacetimeBuffer();
		
		const Wave_PrecisionType SpaceDelta = 1.0f/10000.0f;
		const CLUtils::ArgumentDefintion Arguments[] = {
			{sizeof(cl_uint), &this->Structure->Dimensions},
			{sizeof(cl_mem), &this->SpacetimeBoundsCL},
			{sizeof(cl_mem), &this->WaveVelocityCL},
			{sizeof(cl_mem), &this->TransferEfficiencyCL},
			{sizeof(cl_float), &SpaceDelta},
			{0, nullptr}, 
			{0, nullptr},
			{sizeof(cl_mem), &this->SpacetimeBufferCL},
		};
		cl_int Err = CLUtils::SetKernelArguments(this->Kernel, std::size(Arguments), Arguments);
		CLUtils::PrintAndHaltIfError("Setting initial kernel arguments in WaveStructure task", Err);
	}
	
	void WaveStructure::Task::EnqueueFlushMemory(cl_uint BlockSize, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		/*cl_event* WriteEvents = new cl_event[this->Structure->Inputs.size()];
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (size_t PlugIndex = 0; PlugIndex < this->Structure->Inputs.size(); PlugIndex++) {
			for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
				Cursor[Dimension+1] = this->Structure->Inputs[PlugIndex].Position[Dimension];
			}
			size_t BufferOffset = Math::MapIndex(this->Structure->Dimensions+1, Cursor, this->SpacetimeBounds);
			size_t BufferOffsetBytes = sizeof(Wave_PrecisionType) * BufferOffset;
			const cl_int Err = clEnqueueWriteBuffer(
				this->Queue,
				this->SpacetimeBufferCL,
				CL_FALSE,
				BufferOffsetBytes,
				sizeof(Wave_PrecisionType),
				this->SpacetimeBuffer + BufferOffset,
				WaitEventCount, WaitEvents, WriteEvents + PlugIndex
			);
			CLUtils::PrintAndHaltIfError("Enqueuing read buffer in WaveStructure task", Err);
		}
		clEnqueueMarkerWithWaitList(
			this->Queue,
			this->Structure->Inputs.size(), WriteEvents,
			CompletionEvent
		);
		CLUtils::ReleaseEvents(this->Structure->Inputs.size(), WriteEvents);*/
	}
	
	void WaveStructure::Task::EnqueueExecution(float TimeDelta, cl_uint Timestep, cl_uint Iterations, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_int Err = clSetKernelArg(this->Kernel, 5, sizeof(cl_float), &TimeDelta);
		Err = clSetKernelArg(this->Kernel, 6, sizeof(cl_uint), &Timestep);
		size_t GlobalSize[] = {0,0,0};
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			GlobalSize[Dimension] = this->Structure->SpatialBounds[Dimension];
		}
		cl_event* ExecutionEvents = new cl_event[Iterations+1];
		clEnqueueMarkerWithWaitList(this->Queue, WaitEventCount, WaitEvents, ExecutionEvents+0);
		for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
			cl_uint EventIndex = Iteration+1;
			cl_uint LocalTimestep = (Timestep+Iteration)%2;
			clSetKernelArg(this->Kernel, 6, sizeof(cl_uint), &LocalTimestep);
			Err = clEnqueueNDRangeKernel(this->Queue, this->Kernel, this->Structure->Dimensions, NULL, GlobalSize, NULL, 1, ExecutionEvents+EventIndex-1, ExecutionEvents+EventIndex);
			CLUtils::PrintAndHaltIfError("Enqueuing execution in WaveStructure task", Err);
		}
		CLUtils::ReleaseEvents(Iterations, ExecutionEvents);
		*CompletionEvent = ExecutionEvents[Iterations];
		delete[] ExecutionEvents;
	}
	
	void WaveStructure::Task::EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_event* ReadEvents = new cl_event[this->Structure->Outputs.size()];
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (size_t PlugIndex = 0; PlugIndex < this->Structure->Outputs.size(); PlugIndex++) {
			for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
				Cursor[Dimension+1] = this->Structure->Outputs[PlugIndex].Position[Dimension];
			}
			size_t BufferOffset = Math::MapIndex(this->Structure->Dimensions+1, Cursor, this->SpacetimeBounds);
			size_t BufferOffsetBytes = sizeof(Wave_PrecisionType) * BufferOffset;
			const cl_int Err = clEnqueueReadBuffer(
				this->Queue,
				this->SpacetimeBufferCL,
				CL_FALSE,
				BufferOffsetBytes,
				sizeof(Wave_PrecisionType),
				this->SpacetimeBuffer + BufferOffset,
				WaitEventCount, WaitEvents, ReadEvents + PlugIndex
			);
			CLUtils::PrintAndHaltIfError("Enqueuing read buffer in WaveStructure task", Err);
		}
		clEnqueueMarkerWithWaitList(
			this->Queue,
			this->Structure->Outputs.size(), ReadEvents,
			CompletionEvent
		);
		CLUtils::ReleaseEvents(this->Structure->Outputs.size(), ReadEvents);
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
		clReleaseMemObject(this->WaveVelocityCL); //We don't delete[] the corresponding buffers because we don't own them
		clReleaseMemObject(this->TransferEfficiencyCL);
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
