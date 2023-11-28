#define CL_TARGET_OPENCL_VERSION 200
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>

#include <CL/cl.h>
extern "C" {
	#include <unistd.h>
}

#include "CLUtils.hpp"
#include "Utils.hpp"
#include "Math.hpp"

#include "tinywav.h"

#include "FDM.cl.h"
#include "Spring.cl.h"

void Visualize2DSlice(cl_float* Buffer, const cl_uint Dimensions[2]) {
	cl_uint Cursor[2];
	for (cl_uint Y = 0; Y < Dimensions[1]; Y++) {
		Cursor[1] = Y;
		for (cl_uint X = 0; X < Dimensions[0]; X++) {
			Cursor[0] = X;
			cl_uint Index = Math::MapIndex<2>(Cursor, Dimensions);
			cl_float Value = Buffer[Index];
			cl_uint Brightness = Math::Clamp<cl_int>(0, floor((Value+1.0f)/2.0f*255.0f),255);
			printf("\x1b[38;2;%i;%i;%im█", Brightness, Brightness, Brightness);
		}
		printf("\x1b[0m\n");
	}
}

/*void VisualizeSampleBuffer(cl_float* Buffer, cl_uint GroupX, cl_uint GroupY, const cl_uint Dimensions[3]) {
	for (cl_uint SampleIndex = 0; SampleIndex < Dimensions[2]; SampleIndex++) {
		cl_float Cell = Buffer[Math::MapIndex::From3DTo1D(GroupX, GroupY, SampleIndex, Dimensions)];
		cl_uint Brightness = Math::Clamp<cl_int>(0,floor((Cell+1.0f)/2.0f*255.0f),255);
		printf("\x1b[38;2;%i;%i;%im█", Brightness, Brightness, Brightness);
	}
	printf("\x1b[0m\n");
}*/

void Test_2D_Buffered_SOG(cl_context Context, cl_device_id Device, cl_command_queue Queue) {
	/*srand((unsigned int)time(NULL));
	
	const cl_uint WorkPerGroupX = 16;
	const cl_uint WorkPerGroupY = 16;
	const cl_uint GroupWidth = 64/WorkPerGroupX;
	const cl_uint GroupHeight = 64/WorkPerGroupY;
	const cl_uint CellsPerWorkX = 1;
	const cl_uint CellsPerWorkY = 1;
	const cl_uint GridWidth = WorkPerGroupX*CellsPerWorkX;
	const cl_uint GridHeight = WorkPerGroupY*CellsPerWorkY;
	const cl_uint TotalWorkX = GroupWidth * WorkPerGroupX;
	const cl_uint TotalWorkY = GroupHeight * WorkPerGroupY;
	
	const cl_uint Iterations = 1024;
	const cl_uint IterationsBufferSize = 2;
	const cl_uint IterationsOnDevice = 128;
	
	const cl_uint ParameterBufferSize = GroupWidth * GroupHeight * GridWidth * GridHeight;
	const cl_uint ParameterDimensions[4] = {GroupWidth, GroupHeight, GridWidth, GridHeight};
	FDM_CellParameters* ParameterBuffer = (FDM_CellParameters*) calloc(ParameterBufferSize, sizeof(FDM_CellParameters));
	printf("ParameterBufferSize %lu kilobytes\n", ParameterBufferSize * sizeof(FDM_CellParameters) / 1024);
	
	const cl_uint WorkDimensions[5] = {GroupWidth, GroupHeight, IterationsBufferSize, GridWidth, GridHeight};
	const cl_uint WorkBufferSize = GroupWidth * GroupHeight * IterationsBufferSize * GridWidth * GridHeight;
	cl_float* WorkBuffer = (cl_float*) calloc(WorkBufferSize, sizeof(cl_float));
	printf("WorkBufferSize %lu kilobytes\n",WorkBufferSize * sizeof(cl_float) / 1024);
	
	const cl_uint SampleBufferSize = GroupWidth * GroupHeight * IterationsOnDevice;
	const cl_uint SampleBufferDimensions[3] = {GroupWidth, GroupHeight, IterationsOnDevice};
	cl_float* SampleBufferLeft = (cl_float*) calloc(SampleBufferSize, sizeof(cl_float));
	cl_float* SampleBufferRight = (cl_float*) calloc(SampleBufferSize, sizeof(cl_float));
	printf("SampleBufferSize %lu kilobytes\n", SampleBufferSize * sizeof(cl_float) / 1024);
	
	printf("Initializing Parameters and Initial State\n");
	
	for (cl_uint GroupX = 0; GroupX < GroupWidth; GroupX++) {
		for (cl_uint GroupY = 0; GroupY < GroupHeight; GroupY++) {
			for (cl_uint CellX = 0; CellX < GridWidth; CellX++) {
				for (cl_uint CellY = 0; CellY < GridHeight; CellY++) {
					ParameterBuffer[Math::MapIndex::From4DTo1D(GroupX,GroupY,CellX,CellY,ParameterDimensions)].WaveVelocity = powf(1.0f,2.0f);
					if (CellX == 0 || CellX == GridWidth-1 || CellY == 0 || CellY == GridHeight-1) {
						ParameterBuffer[Math::MapIndex::From4DTo1D(GroupX,GroupY,CellX,CellY,ParameterDimensions)].TransferEfficiency = 0.0f;
					} else {
						ParameterBuffer[Math::MapIndex::From4DTo1D(GroupX,GroupY,CellX,CellY,ParameterDimensions)].TransferEfficiency = 1.0f-(1.0f/4000.0f);
					}
				}
			}
			
			for (cl_uint ImpulseCounter = 0; ImpulseCounter < 5; ImpulseCounter++) {
				WorkBuffer[Math::MapIndex::From5DTo1D<cl_uint>(GroupX, GroupY, -1, rand(), rand(), WorkDimensions)] = 1;
			}
		}
	}

	cl_mem WorkBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * WorkBufferSize, WorkBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(FDM_CellParameters) * ParameterBufferSize, ParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SampleBufferLeftCL = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * SampleBufferSize, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SampleBufferRightCL = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * SampleBufferSize, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	printf("Setting Parameters\n");
	
	const CLUtils::ArgumentDefintion Arguments[] = {
		{sizeof(cl_mem ), &ParameterBufferCL },
		{sizeof(cl_mem ), &WorkBufferCL		  },
		{sizeof(cl_uint), &GroupWidth		  },
		{sizeof(cl_uint), &GroupHeight		  },
		{sizeof(cl_uint), &WorkPerGroupX	  },
		{sizeof(cl_uint), &WorkPerGroupY	  },
		{sizeof(cl_uint), &CellsPerWorkX	  },
		{sizeof(cl_uint), &CellsPerWorkY	  },
		{sizeof(cl_uint), &IterationsOnDevice },
		{sizeof(cl_mem ), &SampleBufferLeftCL },
		{sizeof(cl_mem ), &SampleBufferRightCL},
	};
	
	Err = CLUtils::SetKernelArguments(Kernel, std::size(Arguments), Arguments); CLUtils::PrintAndHaltIfError(Err);
	
	size_t globalSize[] = {TotalWorkX, TotalWorkY, 1};
	size_t localSize[] = {WorkPerGroupX, WorkPerGroupY, 1};
	
	cl_event ReadEvents[3];
	TinyWav WavOutputHandle;
	tinywav_open_write(
		&WavOutputHandle,
		2,
		44100,
		TW_FLOAT32,
		TW_SPLIT,
		"Output.wav"
	);
	
	printf("Executing\n");
	
	long long GlobalStartTime = Utils::Time::Microseconds();
	for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
		long long StartTime = Utils::Time::Microseconds();
		
		Err = clEnqueueNDRangeKernel(Queue, Kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);CLUtils::PrintAndHaltIfError(Err);
		
		Err = clEnqueueReadBuffer(Queue, SampleBufferLeftCL, CL_FALSE, 0, SampleBufferSize * sizeof(cl_float), SampleBufferLeft, 0, NULL, &ReadEvents[0]); CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, SampleBufferRightCL, CL_FALSE, 0, SampleBufferSize * sizeof(cl_float), SampleBufferRight, 0, NULL, &ReadEvents[1]); CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, WorkBufferCL, CL_FALSE, 0, WorkBufferSize * sizeof(cl_float), WorkBuffer, 0, NULL, &ReadEvents[2]); CLUtils::PrintAndHaltIfError(Err);
		
		clWaitForEvents(3, ReadEvents);
		
		clReleaseEvent(ReadEvents[0]);
		clReleaseEvent(ReadEvents[1]);
		clReleaseEvent(ReadEvents[2]);

		long long TotalTime = Utils::Time::Microseconds() - StartTime;
		cl_float* StereoBuffer[2] = {SampleBufferLeft, SampleBufferRight};

		tinywav_write_f(&WavOutputHandle, StereoBuffer, IterationsOnDevice);
		
		for (int Iteration = 0; Iteration < 2; Iteration++) {
			printf("\033[2J"); //Clear screen
			
			printf("Time taken: %lld us\n", TotalTime);
			printf("Per Sample: %lld us\n", TotalTime / IterationsOnDevice);
			printf("Time Spare: %lld us\n", IterationsOnDevice*22 - TotalTime);
			
			VisualizeSampleBuffer(SampleBufferLeft, 0, 0, SampleBufferDimensions);
			VisualizeSampleBuffer(SampleBufferRight, 0, 0, SampleBufferDimensions);
			VisualizeGrid(WorkBuffer, 0, 0, Iteration, WorkDimensions);
			
			usleep(33333);
		}
	}
	printf("Total time taken %lld\n", Utils::Time::Microseconds() - GlobalStartTime);
	printf("Cleaning Up\n");
	tinywav_close_write(&WavOutputHandle);
	*/
}

void Test_3D_Large(cl_context Context, cl_device_id Device, cl_command_queue Queue, unsigned int Iterations) {
	const cl_uint GroupSize[] = {32, 32, 32};
	const cl_uint SpatialBounds[] = {32, 32, 32};
	cl_uint SpacetimeBounds[4] = {2};
	for (unsigned int Dimension = 0; Dimension < 3; Dimension++) {
		SpacetimeBounds[Dimension+1] = SpatialBounds[Dimension];
	}
	
	const cl_uint ParameterBufferSize = Math::GridBufferSize<3>(SpatialBounds);
	FDM_CellParameters* ParameterBuffer = (FDM_CellParameters*) calloc(ParameterBufferSize, sizeof(FDM_CellParameters));
	
	const cl_uint SpacetimeBufferSize = Math::GridBufferSize<4>(SpacetimeBounds);
	cl_float* SpacetimeBuffer = (cl_float*) calloc(SpacetimeBufferSize, sizeof(cl_float));
	
	cl_uint ParameterCursor[3];
	for (cl_uint X = 0; X < SpatialBounds[0]; X++) {
		ParameterCursor[0] = X;
		for (cl_uint Y = 0; Y < SpatialBounds[1]; Y++) {
			ParameterCursor[1] = Y;
			for (cl_uint Z = 0; Z < SpatialBounds[2]; Z++) {
				ParameterCursor[2] = Z;
				const cl_uint CellIndex = Math::MapIndex<3>(ParameterCursor, SpatialBounds);
				ParameterBuffer[CellIndex].WaveVelocity = powf(1.0f,2.0f);
				if (X == 0 || X == SpatialBounds[0]-1 || Y == 0 || Y == SpatialBounds[1]-1 || Z == 0 || Z == SpatialBounds[2]-1) {
					ParameterBuffer[CellIndex].TransferEfficiency = 0.0f;
				} else {
					ParameterBuffer[CellIndex].TransferEfficiency = 1.0f-(1.0f/4000.0f);
				}
			}
		}
	}
	
	cl_uint CellCursor[4] = {0};
	for (unsigned int ImpulseCounter = 0; ImpulseCounter < 1; ImpulseCounter++) {
		CellCursor[1] = rand();
		CellCursor[2] = rand();
		CellCursor[3] = rand();
		SpacetimeBuffer[Math::MapIndex<4>(CellCursor, SpacetimeBounds)] = 1.0f;
	}
	
	const char* SourcePaths[3] = {
		"OpenCL/Kernels/FDM/3D/Large.cl.c",
		"OpenCL/Common/FDM.cl.c",
		"OpenCL/Common/Math.cl.c"
	};
	
	cl_program Program = CLUtils::CompileProgramFromFiles(
		Context, Device, 
		3, SourcePaths,
		"-I OpenCL/Common/ -cl-std=CL2.0"
	);
	
	cl_int Err;
	cl_kernel Kernel = clCreateKernel(Program, "FDM_3D_Large", &Err);
	
	cl_mem SpacetimeBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * SpacetimeBufferSize, SpacetimeBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(FDM_CellParameters) * ParameterBufferSize, ParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 4, SpacetimeBounds, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	cl_uint Timestep = 1;
	const CLUtils::ArgumentDefintion Arguments[] = {
		{sizeof(cl_mem), &ParameterBufferCL},
		{sizeof(cl_mem), &SpacetimeBufferCL},
		{sizeof(cl_uint), &Timestep},
		{sizeof(cl_mem), &SpacetimeBoundsCL},
	};

	Err = CLUtils::SetKernelArguments(Kernel, std::size(Arguments), Arguments); CLUtils::PrintAndHaltIfError(Err);
	size_t GlobalSize[] = {SpatialBounds[0], SpatialBounds[1], SpatialBounds[2]};
	size_t LocalSize[] = {SpatialBounds[0]/GroupSize[0], SpatialBounds[1]/GroupSize[1], SpatialBounds[2]/GroupSize[2]};
	
	cl_event KernelCompleteEvent;
	cl_event ReadEvent;
	TinyWav WavOutputHandle;
	tinywav_open_write(
		&WavOutputHandle,
		2,
		44100,
		TW_FLOAT32,
		TW_SPLIT,
		"Output.wav"
	);
	
	printf("Executing\n");
	
	cl_uint LeftChannelSampleCursor[] = {0,1,1,1};
	cl_uint RightChannelSampleCursor[] = {0,SpatialBounds[0]-2,SpatialBounds[1]-2,SpatialBounds[2]-2};
	long long GlobalStartTime = Utils::Time::Microseconds();
	for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
		printf("\033[2J"); //Clear screen
		Timestep = (Iteration+1)%2;
		long long StartTime = Utils::Time::Microseconds();
		clSetKernelArg(Kernel, 2, sizeof(cl_uint), &Timestep);
		Err = clEnqueueNDRangeKernel(Queue, Kernel, 3, NULL, GlobalSize, LocalSize, 0, NULL, &KernelCompleteEvent);CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, SpacetimeBufferCL, CL_FALSE, 0, SpacetimeBufferSize * sizeof(cl_float), SpacetimeBuffer, 1, &KernelCompleteEvent, &ReadEvent); CLUtils::PrintAndHaltIfError(Err);
		clWaitForEvents(1, &ReadEvent);
		
		long long TotalTime = Utils::Time::Microseconds() - StartTime;
			
		printf("Time for sample: %lld us\n", TotalTime);
		printf("Progress: %f%%\n", (float)Iteration/Iterations * 100.0f);
		LeftChannelSampleCursor[0] = Timestep;
		RightChannelSampleCursor[0] = Timestep;
		float LeftSample = SpacetimeBuffer[Math::MapIndex<4>(LeftChannelSampleCursor,SpacetimeBounds)];
		float RightSample = SpacetimeBuffer[Math::MapIndex<4>(RightChannelSampleCursor,SpacetimeBounds)];
		float* StereoBuffer[2] = {&LeftSample, &RightSample};
		tinywav_write_f(&WavOutputHandle, StereoBuffer, 1);
	}
	
	printf("Total time taken %lld us\n", Utils::Time::Microseconds() - GlobalStartTime);
	printf("Cleaning Up\n");
	tinywav_close_write(&WavOutputHandle);
}

void Test_4D_Large(cl_context Context, cl_device_id Device, cl_command_queue Queue, unsigned int Iterations) {
	const cl_uint GroupSize[] = {32, 32, 32};
	const cl_uint SpatialBounds[] = {32, 32, 32, 32};
	cl_uint SpacetimeBounds[5] = {2};
	for (unsigned int Dimension = 0; Dimension < 4; Dimension++) {
		SpacetimeBounds[Dimension+1] = SpatialBounds[Dimension];
	}
	
	const cl_uint ParameterBufferSize = Math::GridBufferSize<4>(SpatialBounds);
	FDM_CellParameters* ParameterBuffer = (FDM_CellParameters*) calloc(ParameterBufferSize, sizeof(FDM_CellParameters));
	
	const cl_uint SpacetimeBufferSize = Math::GridBufferSize<5>(SpacetimeBounds);
	cl_float* SpacetimeBuffer = (cl_float*) calloc(SpacetimeBufferSize, sizeof(cl_float));
	
	cl_uint ParameterCursor[4];
	for (cl_uint W = 0; W < SpatialBounds[0]; W++) {
		ParameterCursor[0] = W;
		for (cl_uint X = 0; X < SpatialBounds[1]; X++) {
			ParameterCursor[1] = X;
			for (cl_uint Y = 0; Y < SpatialBounds[2]; Y++) {
				ParameterCursor[2] = Y;
				for (cl_uint Z = 0; Z < SpatialBounds[3]; Z++) {
					ParameterCursor[3] = Z;
					const cl_uint CellIndex = Math::MapIndex<4>(ParameterCursor, SpatialBounds);
					
					ParameterBuffer[CellIndex].WaveVelocity = powf(1.0f,2.0f);
					
					if (
						   W == 0 || W == SpatialBounds[0]-1
						|| X == 0 || X == SpatialBounds[1]-1 
						|| Y == 0 || Y == SpatialBounds[2]-1 
						|| Z == 0 || Z == SpatialBounds[3]-1
					) {
						ParameterBuffer[CellIndex].TransferEfficiency = 0.0f;
					} else {
						ParameterBuffer[CellIndex].TransferEfficiency = 1.0f-(1.0f/4000.0f);
					}
				}
			}
		}
	}
	
	cl_uint CellCursor[5] = {0};
	for (unsigned int ImpulseCounter = 0; ImpulseCounter < 1; ImpulseCounter++) {
		CellCursor[1] = rand();
		CellCursor[2] = rand();
		CellCursor[3] = rand();
		CellCursor[4] = rand();
		SpacetimeBuffer[Math::MapIndex<5>(CellCursor, SpacetimeBounds)] = 1.0f;
	}
	
	const char* SourcePaths[3] = {
		"OpenCL/Kernels/FDM/4D/Large.cl.c",
		"OpenCL/Common/FDM.cl.c",
		"OpenCL/Common/Math.cl.c"
	};
	
	cl_program Program = CLUtils::CompileProgramFromFiles(
		Context, Device, 
		3, SourcePaths,
		"-I OpenCL/Common/ -cl-std=CL2.0"
	);
	
	cl_int Err;
	cl_kernel Kernel = clCreateKernel(Program, "FDM_4D_Large", &Err);
	
	cl_mem SpacetimeBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * SpacetimeBufferSize, SpacetimeBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(FDM_CellParameters) * ParameterBufferSize, ParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 5, SpacetimeBounds, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	cl_uint Timestep = 1;
	const CLUtils::ArgumentDefintion Arguments[] = {
		{sizeof(cl_mem), &ParameterBufferCL},
		{sizeof(cl_mem), &SpacetimeBufferCL},
		{sizeof(cl_uint), &Timestep},
		{sizeof(cl_mem), &SpacetimeBoundsCL},
	};

	Err = CLUtils::SetKernelArguments(Kernel, std::size(Arguments), Arguments); CLUtils::PrintAndHaltIfError(Err);
	size_t GlobalSize[] = {SpatialBounds[0], SpatialBounds[1], SpatialBounds[2]};
	size_t LocalSize[] = {SpatialBounds[0]/GroupSize[0], SpatialBounds[1]/GroupSize[1], SpatialBounds[2]/GroupSize[2]};
	
	cl_event KernelCompleteEvent;
	cl_event ReadEvent;
	TinyWav WavOutputHandle;
	tinywav_open_write(
		&WavOutputHandle,
		2,
		44100,
		TW_FLOAT32,
		TW_SPLIT,
		"Output.wav"
	);
	
	printf("Executing\n");
	
	cl_uint LeftChannelSampleCursor[] = {0,1,1,1,1};
	cl_uint RightChannelSampleCursor[] = {0,SpatialBounds[0]-2,SpatialBounds[1]-2,SpatialBounds[2]-2,SpatialBounds[3]-2};
	long long GlobalStartTime = Utils::Time::Microseconds();
	for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
		printf("\033[2J"); //Clear screen
		Timestep = (Iteration+1)%2;
		long long StartTime = Utils::Time::Microseconds();
		clSetKernelArg(Kernel, 2, sizeof(cl_uint), &Timestep);
		Err = clEnqueueNDRangeKernel(Queue, Kernel, 3, NULL, GlobalSize, LocalSize, 0, NULL, &KernelCompleteEvent);CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, SpacetimeBufferCL, CL_FALSE, 0, SpacetimeBufferSize * sizeof(cl_float), SpacetimeBuffer, 1, &KernelCompleteEvent, &ReadEvent); CLUtils::PrintAndHaltIfError(Err);
		clWaitForEvents(1, &ReadEvent);
		
		long long TotalTime = Utils::Time::Microseconds() - StartTime;
			
		printf("Time for sample: %lld us\n", TotalTime);
		printf("Progress: %f%%\n", (float)Iteration/Iterations * 100.0f);
		LeftChannelSampleCursor[0] = Timestep;
		RightChannelSampleCursor[0] = Timestep;
		float LeftSample = SpacetimeBuffer[Math::MapIndex<5>(LeftChannelSampleCursor,SpacetimeBounds)];
		float RightSample = SpacetimeBuffer[Math::MapIndex<5>(RightChannelSampleCursor,SpacetimeBounds)];
		float* StereoBuffer[2] = {&LeftSample, &RightSample};
		tinywav_write_f(&WavOutputHandle, StereoBuffer, 1);
	}
	
	printf("Total time taken %lld us\n", Utils::Time::Microseconds() - GlobalStartTime);
	printf("Cleaning Up\n");
	tinywav_close_write(&WavOutputHandle);
}

void Test_Spring(cl_context Context, cl_device_id Device, cl_command_queue Queue, unsigned int Iterations) {
	const cl_uint GroupSize = 3;
	const cl_uint SpatialBounds = 3;
	cl_uint SpacetimeBounds[2] = {2, SpatialBounds};
	
	const cl_uint ParameterBufferSize = SpatialBounds;
	Spring_SpringParameters* SpringParameterBuffer = (Spring_SpringParameters*) calloc(ParameterBufferSize, sizeof(Spring_SpringParameters));
	Spring_NodeParameters* NodeParameterBuffer = (Spring_NodeParameters*) calloc(ParameterBufferSize,  sizeof(Spring_NodeParameters));
	
	const cl_uint SpacetimeBufferSize = Math::GridBufferSize<2>(SpacetimeBounds);
	Spring_NodeState* SpacetimeBuffer = (Spring_NodeState*) calloc(SpacetimeBufferSize, sizeof(Spring_NodeState));
	
	for (unsigned int Index = 0; Index < SpatialBounds; Index++) {
		SpringParameterBuffer[Index] = {
			.RestLength=2.0f,
			.Stiffness=2.0f 
		};
		NodeParameterBuffer[Index] = {
			.Mass = 1.0f, 
			.Damping = 0.99999f,
			.Fixed = Index==0 || Index==SpatialBounds-1
		};
		unsigned int SpacetimeCursor[] = {0, Index};
		SpacetimeBuffer[Math::MapIndex<2>(SpacetimeCursor, SpacetimeBounds)] = {
			.Position = (float)Index*2.0f,
			.Velocity = 0.0f
		};
	}
	SpacetimeBuffer[1].Position = 3.0f;
	const char* SourcePaths[3] = {
		"OpenCL/Kernels/Spring/1D/Large.cl.c",
		"OpenCL/Common/Spring.cl.c",
		"OpenCL/Common/Math.cl.c"
	};
	
	cl_program Program = CLUtils::CompileProgramFromFiles(
		Context, Device, 
		3, SourcePaths,
		"-I OpenCL/Common/"
	);
	
	cl_int Err;
	cl_kernel Kernel = clCreateKernel(Program, "Spring_1D", &Err);
	
	cl_mem SpacetimeBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Spring_NodeState) * SpacetimeBufferSize, SpacetimeBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SpringParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Spring_SpringParameters) * ParameterBufferSize, SpringParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem NodeParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Spring_NodeParameters) * ParameterBufferSize, NodeParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SpacetimeBoundsCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * 2, SpacetimeBounds, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	cl_uint Timestep = 1;
	cl_float Delta = 1.0f/44100.0f;
	const CLUtils::ArgumentDefintion Arguments[] = {
		{sizeof(cl_mem), &SpringParameterBufferCL},
		{sizeof(cl_mem), &NodeParameterBufferCL},
		{sizeof(cl_mem), &SpacetimeBufferCL},
		{sizeof(cl_mem), &SpacetimeBoundsCL},
		{sizeof(cl_uint), &Timestep},
		{sizeof(cl_float), &Delta},
	};

	Err = CLUtils::SetKernelArguments(Kernel, std::size(Arguments), Arguments); CLUtils::PrintAndHaltIfError(Err);
	size_t GlobalSize[] = {SpatialBounds};
	size_t LocalSize[] = {SpatialBounds/GroupSize};
	
	cl_event KernelCompleteEvent;
	cl_event ReadEvent;
	TinyWav WavOutputHandle;
	tinywav_open_write(
		&WavOutputHandle,
		2,
		44100,
		TW_FLOAT32,
		TW_SPLIT,
		"Output.wav"
	);
	
	cl_uint SampleCursor[] = {0, SpatialBounds-2};
	long long GlobalStartTime = Utils::Time::Microseconds();
	unsigned int LastProgress = 0;
	for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
		Timestep = (Iteration+1)%2;
		long long StartTime = Utils::Time::Microseconds();
		clSetKernelArg(Kernel, 4, sizeof(cl_uint), &Timestep);
		Err = clEnqueueNDRangeKernel(Queue, Kernel, 1, NULL, GlobalSize, LocalSize, 0, NULL, &KernelCompleteEvent);CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, SpacetimeBufferCL, CL_FALSE, 0, SpacetimeBufferSize * sizeof(Spring_NodeState), SpacetimeBuffer, 1, &KernelCompleteEvent, &ReadEvent); CLUtils::PrintAndHaltIfError(Err);
		clWaitForEvents(1, &ReadEvent);
		
		long long TotalTime = Utils::Time::Microseconds() - StartTime;
			
		float Progress = (float)Iteration/(Iterations-1) * 100.0f;

		if (floor(Progress) > LastProgress) {
			//printf("\033[2J"); //Clear screen
			LastProgress = floor(Progress);
			printf("Progress: %i\n", LastProgress);
		}
		
		SampleCursor[0] = Timestep;
		float Sample = SpacetimeBuffer[Math::MapIndex<2>(SampleCursor,SpacetimeBounds)].Position;
		float Deflection = Sample - (SpatialBounds-2)*2;
		float* StereoBuffer[2] = {&Deflection, &Deflection};
		tinywav_write_f(&WavOutputHandle, StereoBuffer, 1);
	}
	
	printf("Total time taken %f s\n", (Utils::Time::Microseconds() - GlobalStartTime) / 1e+6);
	printf("Cleaning Up\n");
	tinywav_close_write(&WavOutputHandle);

}

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void Test_GUI() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) return;
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (window == nullptr) return;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	  // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");							// Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");				// Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);		// Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);			// Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))							// Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);	// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
