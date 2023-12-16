#include "WaveStructure.hpp"

#include <cstring>
#include <cstdio>
#include <typeinfo>

#include "../../CLUtils.hpp"
#include "../../Math.hpp"
#include <cassert>

namespace APM::Scene::RenderDispatcher {
	void WaveStructure::Task::SetupBoundsBuffer() {
		this->SpacetimeBounds = new cl_uint[Structure->Dimensions+1];
		this->SpacetimeBounds[0] = 2;
		for (cl_uint Dimension = 0; Dimension < Structure->Dimensions; Dimension++) {
			this->SpacetimeBounds[Dimension+1] = Structure->SpatialBounds[Dimension];
		}
		cl_int Err;
		this->SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * (Structure->Dimensions+1), this->SpacetimeBounds, &Err);
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBoundsCL", Err);
	}
	
	void WaveStructure::Task::SetupParameterBuffers() {
		cl_int Err;
		this->WaveVelocityCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * Structure->BufferLength, Structure->WaveVelocity, &Err);
		CLUtils::PrintAndHaltIfError("Creating WaveVelocityCL", Err);
		this->TransferEfficiencyCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * Structure->BufferLength, Structure->TransferEfficiency, &Err);
		CLUtils::PrintAndHaltIfError("Creating TransferEfficiencyCL", Err);
	}
	
	void WaveStructure::Task::SetupSpacetimeBuffer() {
		this->Spacetime = new Wave_PrecisionType[this->Structure->BufferLength*2];
		std::memset(
			this->Spacetime,
			0,
			this->Structure->BufferLength * 2 * sizeof(Wave_PrecisionType)
		);
		std::memcpy(
			this->Spacetime,
			this->Structure->SpaceBuffer,
			this->Structure->BufferLength * sizeof(Wave_PrecisionType)
		);
		cl_int Err;
		this->SpacetimeCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * this->Structure->BufferLength * 2, this->Spacetime, &Err);
		delete[] this->Spacetime;
		CLUtils::PrintAndHaltIfError("Creating SpacetimeBufferCL in WaveStructure task", Err);
	}
	
	void WaveStructure::Task::SetupInputBuffers() {
		cl_uint InputCount = this->Structure->Inputs.size();
		if (InputCount == 0) return;
		cl_uint InputPositionsLength = InputCount * this->Structure->Dimensions;
		this->InputPositions = new cl_uint[InputPositionsLength];
		for (cl_uint InputIndex = 0; InputIndex < InputCount; InputIndex++) {
			for (cl_uint Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
				this->InputPositions[(InputIndex*this->Structure->Dimensions)+Dimension] = this->Structure->Inputs[InputIndex].Position[Dimension];
			}
		}
		cl_int Err;
		printf("InputPositionsLength=%u\n",InputPositionsLength);
		this->InputPositionsCL = clCreateBuffer(this->Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * InputPositionsLength, this->InputPositions, &Err);
		CLUtils::PrintAndHaltIfError("Creating InputPositionsCL", Err);
		
		cl_uint InputBuffersLength = InputCount * this->MaxBlockSize;
		this->InputBuffers = new Wave_PrecisionType[InputBuffersLength];
		std::memset(this->InputBuffers, 0, sizeof(Wave_PrecisionType) * InputBuffersLength);
		this->InputBuffersCL = clCreateBuffer(this->Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * InputBuffersLength, this->InputBuffers, &Err);
		CLUtils::PrintAndHaltIfError("Creating InputBuffersCL", Err);
	}
	
	void WaveStructure::Task::SetupOutputBuffers() {
		cl_uint OutputCount = this->Structure->Outputs.size();
		if (OutputCount == 0) return;
		cl_uint OutputPositionsLength = OutputCount * this->Structure->Dimensions;
		this->OutputPositions = new cl_uint[OutputPositionsLength];
		for (cl_uint OutputIndex = 0; OutputIndex < OutputCount; OutputIndex++) {
			for (cl_uint Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
				this->OutputPositions[(OutputIndex*this->Structure->Dimensions)+Dimension] = this->Structure->Outputs[OutputIndex].Position[Dimension];
			}
		}
		
		cl_int Err;
		this->OutputPositionsCL = clCreateBuffer(this->Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * OutputPositionsLength, this->OutputPositions, &Err);
		CLUtils::PrintAndHaltIfError("Creating OutputPositionsCL", Err);
		
		cl_uint OutputBuffersLength = OutputCount * this->MaxBlockSize;
		this->OutputBuffers = new Wave_PrecisionType[OutputBuffersLength];
		printf("Allocated %lu bytes for OutputBuffers at %p\n", sizeof(Wave_PrecisionType)*OutputBuffersLength, this->OutputBuffers);
		std::memset(this->OutputBuffers, 0, sizeof(Wave_PrecisionType) * OutputBuffersLength);
		this->OutputBuffersCL = clCreateBuffer(this->Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Wave_PrecisionType) * OutputBuffersLength, this->OutputBuffers, &Err);
		CLUtils::PrintAndHaltIfError("Creating OutputBuffersCL", Err);
	}
	
	WaveStructure::Task::Task(cl_context Context, cl_command_queue Queue, cl_kernel ScatterKernel, cl_kernel ComputationKernel, cl_kernel GatherKernel, size_t MaxBlockSize, Object::WaveStructure* Structure): Base::Task(Context, Queue, MaxBlockSize) {
		this->ScatterKernel = clCloneKernel(ScatterKernel, nullptr);
		this->ComputationKernel = clCloneKernel(ComputationKernel, nullptr);
		this->GatherKernel = clCloneKernel(GatherKernel, nullptr);
		
		this->Structure = Structure;
		
		this->SetupBoundsBuffer();
		this->SetupParameterBuffers();
		this->SetupSpacetimeBuffer();
		this->SetupInputBuffers();
		this->SetupOutputBuffers();
		
		cl_int Err;
		
		cl_uint InputCount = this->Structure->Inputs.size();
		if (InputCount > 0) {
			const CLUtils::ArgumentDefintion ScatterArguments[] = {
				{sizeof(cl_uint), &this->Structure->Dimensions},
				{sizeof(cl_uint), &this->MaxBlockSize         },
				{0              ,  nullptr                    },
				{0              ,  nullptr                    },
				{sizeof(cl_uint), &InputCount                 },
				{sizeof(cl_mem) , &this->InputPositionsCL     },
				{sizeof(cl_mem) , &this->InputBuffersCL       },
				{sizeof(cl_mem) , &this->SpacetimeBoundsCL    },
				{sizeof(cl_mem) , &this->SpacetimeCL    },
			};
			Err = CLUtils::SetKernelArguments(this->ScatterKernel, std::size(ScatterArguments), ScatterArguments);
			CLUtils::PrintAndHaltIfError("Setting initial Scatter arguments", Err);
		}
		
		const Wave_PrecisionType SpaceDelta = 1.0f/10000.0f;
		const CLUtils::ArgumentDefintion ComputationArguments[] = {
			{sizeof(cl_uint), &this->Structure->Dimensions},
			{0, nullptr}, 
			{0, nullptr},
			{sizeof(cl_float), &SpaceDelta},
			{sizeof(cl_mem), &this->SpacetimeBoundsCL},
			{sizeof(cl_mem), &this->WaveVelocityCL},
			{sizeof(cl_mem), &this->TransferEfficiencyCL},
			{sizeof(cl_mem), &this->SpacetimeCL},
		};
		Err = CLUtils::SetKernelArguments(this->ComputationKernel, std::size(ComputationArguments), ComputationArguments);
		CLUtils::PrintAndHaltIfError("Setting initial Computation arguments", Err);
		
		cl_uint OutputCount = this->Structure->Outputs.size();
		if (OutputCount > 0) {
			const CLUtils::ArgumentDefintion GatherArguments[] = {
				{sizeof(cl_uint), &this->Structure->Dimensions}, //0
				{sizeof(cl_uint), &this->MaxBlockSize         }, //1
				{0              ,  nullptr                    }, //2
				{0              ,  nullptr                    }, //3
				{sizeof(cl_uint), &OutputCount                }, //4
				{sizeof(cl_mem) , &this->OutputPositionsCL    }, //5
				{sizeof(cl_mem) , &this->OutputBuffersCL      }, //6
				{sizeof(cl_mem) , &this->SpacetimeBoundsCL    }, //7
				{sizeof(cl_mem) , &this->SpacetimeCL          }, //8
			};
			Err = CLUtils::SetKernelArguments(this->GatherKernel, std::size(GatherArguments), GatherArguments);	
			CLUtils::PrintAndHaltIfError("Setting initial Gather arguments", Err);
		}
		printf("Task setup complete\n");
	}
	
	void WaveStructure::Task::EnqueueFlushMemory(cl_uint BlockSize, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		if (this->Structure->Inputs.size() == 0) {
			if (WaitEventCount == 0) {
				cl_int Err;
				*CompletionEvent = clCreateUserEvent(this->Context, &Err);
				CLUtils::PrintAndHaltIfError("Faking CompletionEvent", Err);
				Err = clSetUserEventStatus(*CompletionEvent, CL_COMPLETE);
				CLUtils::PrintAndHaltIfError("Marking CompletionEvent CL_COMPLETE", Err);
			} else {
				clEnqueueMarkerWithWaitList(this->Queue, WaitEventCount, WaitEvents, CompletionEvent);
			}
			return;
		};
		
		cl_int Err = clEnqueueWriteBuffer(
			this->Queue,
			this->InputBuffersCL,
			CL_FALSE,
			0, sizeof(Wave_PrecisionType) * this->MaxBlockSize * this->Structure->Inputs.size(),
			this->InputBuffers,
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError("Writing to InputBuffersCL", Err);
	}
	
	void WaveStructure::Task::EnqueueExecution(cl_uint StartIteration, cl_uint BlockSize, float TimeDelta, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		cl_event StartEvent;
		cl_int Err = clEnqueueMarkerWithWaitList(this->Queue, WaitEventCount, WaitEvents, &StartEvent);
		CLUtils::PrintAndHaltIfError("Coalescing WaitEvents", Err);
		for (cl_uint SubIteration = 0; SubIteration < BlockSize; SubIteration++) {
			cl_event ScatterEvent, ComputationEvent, GatherEvent;
			
			cl_uint Iteration = StartIteration + SubIteration;
			cl_uint PreviousTimestep = (Iteration) % 2;
			cl_uint CurrentTimestep = (Iteration+1) % 2;
			
			if (this->Structure->Inputs.size() > 0) {
				Err = clSetKernelArg(this->ScatterKernel, 2, sizeof(cl_uint), &SubIteration);
				Err = clSetKernelArg(this->ScatterKernel, 3, sizeof(cl_uint), &PreviousTimestep);
				const size_t ScatterWorkSize[] = {this->Structure->Inputs.size()};
				Err = clEnqueueNDRangeKernel(
					this->Queue,
					this->ScatterKernel,
					1,
					nullptr, ScatterWorkSize, nullptr,
					1, &StartEvent, &ScatterEvent
				); 
				Err = clReleaseEvent(StartEvent); 
				StartEvent = ScatterEvent;
			}
			
			Err = clSetKernelArg(this->ComputationKernel, 1, sizeof(cl_uint), &CurrentTimestep);
			CLUtils::PrintAndHaltIfError("Setting Computation argument 1",Err);
			Err = clSetKernelArg(this->ComputationKernel, 2, sizeof(Wave_PrecisionType), &TimeDelta);
			CLUtils::PrintAndHaltIfError("Setting Computation argument 2", Err);
			size_t ComputationWorkSize[] = {0,0,0};
			for (cl_uint Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
				ComputationWorkSize[Dimension] = this->Structure->SpatialBounds[Dimension];
			}
			Err = clEnqueueNDRangeKernel(
				this->Queue,
				this->ComputationKernel,
				this->Structure->Dimensions,
				nullptr, ComputationWorkSize, nullptr,
				1, &StartEvent, &ComputationEvent
			); 
			CLUtils::PrintAndHaltIfError("Enqueuing Computation kernel", Err);
			Err = clReleaseEvent(StartEvent);
			CLUtils::PrintAndHaltIfError("Releasing StartEvent after Computation", Err);
			StartEvent = ComputationEvent;
			
			if (this->Structure->Outputs.size() > 0) {
				Err = clSetKernelArg(this->GatherKernel, 2, sizeof(cl_uint), &SubIteration);
				CLUtils::PrintAndHaltIfError("Setting Gather argument 2", Err);
				Err = clSetKernelArg(this->GatherKernel, 3, sizeof(cl_uint), &CurrentTimestep);
				CLUtils::PrintAndHaltIfError("Setting Gather argument 3", Err);
				const size_t GatherWorkSize[] = {this->Structure->Outputs.size()};
				Err = clEnqueueNDRangeKernel(
					this->Queue,
					this->GatherKernel,
					1, nullptr, GatherWorkSize, nullptr,
					1, &StartEvent, &GatherEvent
				); 
				CLUtils::PrintAndHaltIfError("Enqueuing Gather kernel", Err);
				Err = clReleaseEvent(StartEvent);
				CLUtils::PrintAndHaltIfError("Releasing StartEvent after Gather", Err);
				StartEvent = GatherEvent;
			}
		}
		*CompletionEvent = StartEvent;
	}
	
	void WaveStructure::Task::EnqueueReadyMemory(cl_uint Timestep, cl_uint WaitEventCount, const cl_event *WaitEvents, cl_event *CompletionEvent) {
		if (this->Structure->Outputs.size() == 0) {
			if (WaitEventCount == 0) {
				*CompletionEvent = clCreateUserEvent(this->Context, nullptr);
				clSetUserEventStatus(*CompletionEvent, CL_COMPLETE);
			} else {
				clEnqueueMarkerWithWaitList(this->Queue, WaitEventCount, WaitEvents, CompletionEvent);
			}
			return;
		}
		cl_int Err = clEnqueueReadBuffer(
			this->Queue,
			this->OutputBuffersCL,
			CL_FALSE,
			0, sizeof(Wave_PrecisionType) * this->MaxBlockSize * this->Structure->Outputs.size(),
			this->OutputBuffers,
			WaitEventCount, WaitEvents, CompletionEvent
		);
		CLUtils::PrintAndHaltIfError("Reading from OutputBuffersCL", Err);
	}
	
	float WaveStructure::Task::GetSourceValue(size_t ID, size_t Timestep) { //TODO
		/*Object::WaveStructure::Plug CurrentPlug = this->Structure->Outputs[ID];
		
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			Cursor[Dimension+1] = CurrentPlug.Position[Dimension];
		}
		
		float Value = this->SpacetimeBuffer[Math::MapIndex(this->Structure->Dimensions+1, Cursor, this->SpacetimeBounds)];
		
		delete[] Cursor;
		
		return Value;*/
		return 0.0;
	}
	
	void WaveStructure::Task::SetSinkValue(size_t ID, size_t Timestep, float Value) { //TODO
		/*Object::WaveStructure::Plug CurrentPlug = this->Structure->Inputs[ID];
		cl_uint* Cursor = new cl_uint[this->Structure->Dimensions+1];
		Cursor[0] = Timestep;
		for (unsigned int Dimension = 0; Dimension < this->Structure->Dimensions; Dimension++) {
			Cursor[Dimension+1] = CurrentPlug.Position[Dimension];
		}
		this->SpacetimeBuffer[Math::MapIndex<3>(Cursor, this->SpacetimeBounds)] = Value;
		delete[] Cursor;*/
	}
	
	WaveStructure::Task::~Task() {
		delete[] this->Spacetime;
		delete[] this->InputBuffers;
		delete[] this->OutputBuffers;
		delete[] this->InputPositions;
		delete[] this->OutputPositions;
		delete[] this->SpacetimeBounds;
		clReleaseMemObject(this->InputPositionsCL);
		clReleaseMemObject(this->OutputPositionsCL);
		clReleaseMemObject(this->OutputBuffersCL);
		clReleaseMemObject(this->InputBuffersCL);
		clReleaseMemObject(this->SpacetimeCL);
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
		this->ScatterKernel = clCreateKernel(this->Program, "Wave_1Dto3D_Scatter", &Err);
		CLUtils::PrintAndHaltIfError("Creating kernel for Wave_1Dto3D_Scatter", Err);
		this->ComputationKernel = clCreateKernel(this->Program, "Wave_1Dto3D_Large", &Err);
		CLUtils::PrintAndHaltIfError("Creating kernel for Wave_1Dto3D_Large", Err);
		this->GatherKernel = clCreateKernel(this->Program, "Wave_1Dto3D_Gather", &Err);
		CLUtils::PrintAndHaltIfError("Creating kernel for Wave_1Dto3D_Gather", Err);
	}

	bool WaveStructure::Handles(Object::Base* Object) {
		return typeid(*Object) == typeid(Object::WaveStructure);
	}
	
	Base::Task* WaveStructure::CreateTask(Object::Base* TaskObject, cl_uint MaxBlockSize) {
		return new WaveStructure::Task(this->Context, this->Queue, this->ScatterKernel, this->ComputationKernel, this->GatherKernel, MaxBlockSize, dynamic_cast<Object::WaveStructure*>(TaskObject));
	}

	WaveStructure::~WaveStructure() {
		clReleaseKernel(this->ScatterKernel);
		clReleaseKernel(this->ComputationKernel);
		clReleaseKernel(this->GatherKernel);
		clReleaseProgram(this->Program);
	}

}
