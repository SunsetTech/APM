
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

typedef struct {
	cl_float WaveVelocity;
	cl_float TransferEfficiency;
} CellParameters;

void VisualizeGrid(cl_float* WorkBuffer, cl_uint GroupX, cl_uint GroupY, cl_uint Iteration, const cl_uint Dimensions[5]) {
	for (cl_uint Y = 0; Y < Dimensions[4]; Y++) { //TODO this is upside down
		for (cl_uint X = 0; X < Dimensions[3]; X++) {
			cl_uint Index = Math::MapIndex::From5DTo1D(GroupX, GroupY, Iteration, X, Y, Dimensions);
			cl_float Cell = WorkBuffer[Index];
			cl_uint Brightness = Math::Clamp<cl_int>(0,floor((Cell+1.0f)/2.0f*255.0f),255);
			printf("\x1b[38;2;%i;%i;%im█", Brightness, Brightness, Brightness);
		}
		printf("\x1b[0m\n");
	}
}

void VisualizeSampleBuffer(cl_float* Buffer, cl_uint GroupX, cl_uint GroupY, const cl_uint Dimensions[3]) {
	for (cl_uint SampleIndex = 0; SampleIndex < Dimensions[2]; SampleIndex++) {
		cl_float Cell = Buffer[Math::MapIndex::From3DTo1D(GroupX, GroupY, SampleIndex, Dimensions)];
		cl_uint Brightness = Math::Clamp<cl_int>(0,floor((Cell+1.0f)/2.0f*255.0f),255);
		printf("\x1b[38;2;%i;%i;%im█", Brightness, Brightness, Brightness);
	}
	printf("\x1b[0m\n");
}

int main() {
	srand((unsigned int)time(NULL));
	
	
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
	
	const cl_uint MegaIterations = 1024;
	const cl_uint IterationsBufferSize = 2;
	const cl_uint IterationsOnDevice = 128;
	
	const cl_uint ParameterBufferSize = GroupWidth * GroupHeight * GridWidth * GridHeight;
	const cl_uint ParameterDimensions[4] = {GroupWidth, GroupHeight, GridWidth, GridHeight};
	CellParameters* ParameterBuffer = (CellParameters*) calloc(ParameterBufferSize, sizeof(CellParameters));
	printf("ParameterBufferSize %lu kilobytes\n", ParameterBufferSize * sizeof(CellParameters) / 1024);
	
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
			
			for (cl_uint ImpulseCounter = 0; ImpulseCounter < 1; ImpulseCounter++) {
				WorkBuffer[Math::MapIndex::From5DTo1D<cl_uint>(GroupX, GroupY, -1, rand(), rand(), WorkDimensions)] = 1;
			}
		}
	}

	printf("Setting up\n");
	cl_context Context;
	cl_command_queue Queue;
	cl_program Program;
	cl_kernel Kernel;
	cl_int Err;
	
	// Get platform and device
	cl_platform_id Platforms[10];
	cl_uint NumPlatformsAvailable;
	Err = clGetPlatformIDs(10, Platforms, &NumPlatformsAvailable);CLUtils::PrintAndHaltIfError(Err);
	for (cl_uint PlatformIndex = 0; PlatformIndex < NumPlatformsAvailable; PlatformIndex++) {
		size_t NameLength;
		clGetPlatformInfo(Platforms[PlatformIndex], CL_PLATFORM_NAME, 0, NULL, &NameLength);
		char* Name = (char*)calloc(NameLength, sizeof(char));
		clGetPlatformInfo(Platforms[PlatformIndex], CL_PLATFORM_NAME, NameLength * sizeof(char), Name, NULL);
		printf("%u) %s\n", PlatformIndex+1, Name);
	}
	cl_uint PlatformIndexChoice;
	scanf("%u", &PlatformIndexChoice);
	
	cl_platform_id Platform = Platforms[PlatformIndexChoice-1];
	
	cl_device_id Devices[10];
	cl_uint NumDevicesAvailable;
	Err = clGetDeviceIDs(Platform, CL_DEVICE_TYPE_ALL, 10, Devices, &NumDevicesAvailable);CLUtils::PrintAndHaltIfError(Err);
	cl_device_id Device;
	if (NumDevicesAvailable > 1) {
		for (cl_uint DeviceIndex = 0; DeviceIndex < NumDevicesAvailable; DeviceIndex++) {
			size_t NameLength;
			clGetDeviceInfo(Devices[DeviceIndex], CL_DEVICE_NAME, 0, NULL, &NameLength);
			char* Name = (char*)calloc(NameLength, sizeof(char));
			clGetDeviceInfo(Devices[DeviceIndex], CL_DEVICE_NAME, NameLength * sizeof(char), Name, NULL);
			printf("%u) %s\n", DeviceIndex+1, Name);
		}
		cl_uint DeviceIndexChoice;
		scanf("%u", &DeviceIndexChoice);
		
		Device = Devices[DeviceIndexChoice-1];
	} else {
		Device = Devices[0];
	}
	
	Context = clCreateContext(NULL, 1, &Device, NULL, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	Queue = clCreateCommandQueue(Context, Device, CL_QUEUE_PROFILING_ENABLE, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	const char* SourcePaths[3] = {
		"CLC/Source/Kernels/FDM/2D/Buffered/SOG.cl.c",
		"CLC/Source/Common/Math.cl.c",
		"CLC/Source/Common/FDM.cl.c"
	};
	size_t Kernel_Lengths[3];
	//char* Kernel_Source = Utils::ReadFile("CLC/Source/Kernels/FDM/2D/Buffered/SOG.cl.c", false, &Kernel_Length);
	char* Kernel_Sources[3];
	
	/*Kernel_Sources[0] = Utils::ReadFile("CLC/Source/Kernels/FDM/2D/Buffered/SOG.cl.c", false, &Kernel_Lengths[0]);
	Kernel_Sources[1] = Utils::ReadFile("CLC/Source/Common/Math.cl.c", false, &Kernel_Lengths[1]);
	Kernel_Sources[2] = Utils::ReadFile("CLC/Source/Common/FDM.cl.c", false, &Kernel_Lengths[2]);*/
	for (unsigned int SourceIndex = 0; SourceIndex < 3; SourceIndex++) {
		Kernel_Sources[SourceIndex] = Utils::ReadFile(SourcePaths[SourceIndex], false, &Kernel_Lengths[SourceIndex]);
	}
	
	Program = clCreateProgramWithSource(Context, 3, (const char**)Kernel_Sources, Kernel_Lengths, &Err);
	//Program = clCreateProgramWithIL(Context, Kernel_SPIRV, Kernel_SPIRVLength, &Err);CLUtils::PrintAndHaltIfError(Err);
	Err = clBuildProgram(Program, 1, &Device, "-I CLC/Source/Common/", NULL, NULL);//CLUtils::PrintAndHaltIfError(Err);
	if (Err == CL_BUILD_PROGRAM_FAILURE) {
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char *log = (char *) malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);
		exit(-1);
	}
	Kernel = clCreateKernel(Program, "FDM_2D_Buffered_SOG", &Err);CLUtils::PrintAndHaltIfError(Err);
	
	cl_mem WorkBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * WorkBufferSize, WorkBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(CellParameters) * ParameterBufferSize, ParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SampleBufferLeftCL = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * SampleBufferSize, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SampleBufferRightCL = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * SampleBufferSize, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	printf("Setting Parameters\n");
	
	const CLUtils::ArgumentDefintion Arguments[] = {
		{sizeof(cl_mem ), &ParameterBufferCL },
		{sizeof(cl_mem ), &WorkBufferCL       },
		{sizeof(cl_uint), &GroupWidth         },
		{sizeof(cl_uint), &GroupHeight        },
		{sizeof(cl_uint), &WorkPerGroupX      },
		{sizeof(cl_uint), &WorkPerGroupY      },
		{sizeof(cl_uint), &CellsPerWorkX      },
		{sizeof(cl_uint), &CellsPerWorkY      },
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
	for (cl_uint MegaIteration = 0; MegaIteration < MegaIterations; MegaIteration++) {
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
	clFinish(Queue);
	
	clReleaseMemObject(WorkBufferCL);
	clReleaseMemObject(ParameterBufferCL);
	clReleaseKernel(Kernel);
	clReleaseProgram(Program);
	clReleaseCommandQueue(Queue);
	clReleaseContext(Context);

	free(Kernel_Sources[0]);
	free(Kernel_Sources[1]);
	free(Kernel_Sources[2]);

	return 0;
}
