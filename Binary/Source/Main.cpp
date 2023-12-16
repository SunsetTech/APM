#include <ctime>
#include <cmath>

#include <CL/cl.h>
#include "tinywav.h"
#include "Utils.hpp"
#include "Engine.hpp"
#include "ExecutionBin.hpp"
#include "Scene/Description.hpp"
#include "Scene/Object/SpringBundle.hpp"
#include "Scene/Object/WaveStructure.hpp"
#include "Scene/RenderDispatcher/SpringBundle.hpp"
#include "Scene/RenderDispatcher/WaveStructure.hpp"

int main() {
	srand((unsigned int)time(NULL));
	
	cl_uint NumPlatforms;
	clGetPlatformIDs(0, NULL, &NumPlatforms);
	cl_platform_id* Platforms = (cl_platform_id*)calloc(NumPlatforms, sizeof(cl_platform_id));
	clGetPlatformIDs(NumPlatforms, Platforms, NULL);
	std::vector<APM::ExecutionBin*> ExecutionBins;
	for (cl_uint PlatformIndex = 0; PlatformIndex < NumPlatforms; PlatformIndex++) {
		size_t PlatformNameLength;
		clGetPlatformInfo(Platforms[PlatformIndex], CL_PLATFORM_NAME, 0, NULL, &PlatformNameLength);
		char* PlatformName = (char*)calloc(PlatformNameLength, sizeof(char));
		clGetPlatformInfo(Platforms[PlatformIndex], CL_PLATFORM_NAME, PlatformNameLength * sizeof(char), PlatformName, NULL);
		printf("Setting up Execution Bins for platform %i '%s'\n", PlatformIndex, PlatformName);
		free(PlatformName);
		
		cl_uint NumDevices;
		clGetDeviceIDs(Platforms[PlatformIndex], CL_DEVICE_TYPE_ALL, 0, NULL, &NumDevices);
		cl_device_id* Devices = (cl_device_id*)calloc(NumDevices, sizeof(cl_device_id));
		clGetDeviceIDs(Platforms[PlatformIndex], CL_DEVICE_TYPE_ALL, NumDevices, Devices, NULL);
		for (cl_uint DeviceIndex = 0; DeviceIndex < NumDevices; DeviceIndex++) {
			size_t NameLength;
			clGetDeviceInfo(Devices[DeviceIndex], CL_DEVICE_NAME, 0, NULL, &NameLength);
			char* DeviceName = (char*)calloc(NameLength, sizeof(char));
			clGetDeviceInfo(Devices[DeviceIndex], CL_DEVICE_NAME, NameLength * sizeof(char), DeviceName, NULL);
			
			char Device_CL_Version[100];
			clGetDeviceInfo(Devices[DeviceIndex], CL_DEVICE_VERSION, sizeof(Device_CL_Version), Device_CL_Version, NULL);
			int MajorVersion, MinorVersion;
			sscanf(Device_CL_Version, "OpenCL %d.%d", &MajorVersion, &MinorVersion);
			if (MajorVersion < 2) {
				printf("Skipping '%s' (OpenCL < 2.0)\n", DeviceName);
				continue;
			}
			printf("Use '%s'? (y/n): \n", DeviceName);
			char Answer = getchar(); while(getchar() != '\n'){};
			if (Answer != 'y' && Answer != 'Y') continue;
			free(DeviceName);
			
			APM::ExecutionBin* DeviceBin = new APM::ExecutionBin(Devices[DeviceIndex]);
			//DeviceBin->AttachDispatcher<APM::Scene::RenderDispatcher::SpringBundle>();
			DeviceBin->AttachDispatcher<APM::Scene::RenderDispatcher::WaveStructure>();
			ExecutionBins.push_back(DeviceBin);
		}
	}
	//TODO benchmark selected devices for initial performance ordering
	printf("Setting up scene...\n");
	APM::Scene::Description TestScene;
	/*APM::Scene::Object::SpringBundle TestBundle(1,5);
	for (unsigned int FiberIndex = 0; FiberIndex < TestBundle.FiberCount; FiberIndex++) {
		for (unsigned int NodeIndex = 0; NodeIndex < TestBundle.FiberLength; NodeIndex++) {
			size_t BufferIndex = TestBundle.MapIndex(FiberIndex, NodeIndex);
			TestBundle.NodeParameterBuffer[BufferIndex].Mass = 1.0f;
			TestBundle.NodeParameterBuffer[BufferIndex].Damping = 0.9999f;
			TestBundle.NodeParameterBuffer[BufferIndex].Fixed = false;
			TestBundle.SpringParameterBuffer[BufferIndex] = {
				.RestLength = 1.0f,
				.Stiffness = 8.0f,
			};
			TestBundle.SpaceBuffer[BufferIndex].Position = (float)NodeIndex;
			TestBundle.SpaceBuffer[BufferIndex].Velocity = 0.0f;
		}
		TestBundle.NodeParameterBuffer[TestBundle.MapIndex(FiberIndex, 0 )].Fixed = true;
		TestBundle.NodeParameterBuffer[TestBundle.MapIndex(FiberIndex, TestBundle.FiberLength-1)].Fixed = true;
		TestBundle.SpringParameterBuffer[TestBundle.MapIndex(FiberIndex, TestBundle.FiberLength-1)].Stiffness = 0.0f;
	}
	TestBundle.SpaceBuffer[TestBundle.MapIndex(0,1)].Position -= 0.99f;
	TestBundle.Outputs.push_back({0,TestBundle.FiberLength-2,(float)TestBundle.FiberLength-2.0f});
	TestScene.Objects.push_back(&TestBundle);*/
	
	cl_uint TestStructureBounds[] = {64, 64};
	APM::Scene::Object::WaveStructure TestStructure(2, TestStructureBounds);
	cl_uint Cursor[2];
	for (cl_uint X = 0; X < TestStructureBounds[0]; X++) {
		for (cl_uint Y = 0; Y < TestStructureBounds[1]; Y++) {
			//for (cl_uint Z = 0; Z < TestStructureBounds[2]; Z++) {
				Cursor[0] = X;
				Cursor[1] = Y;
				//Cursor[2] = Z;
				size_t Index = TestStructure.MapIndex(Cursor);
				TestStructure.SpaceBuffer[Index] = 0.0f;
				TestStructure.WaveVelocity[Index] = powf(1.0f, 2.0f);
				if (
					   X == 0 || X == TestStructureBounds[0]-1
					|| Y == 0 || Y == TestStructureBounds[1]-1
					//|| Z == 0 || Z == TestStructureBounds[2]-1
				) {
					TestStructure.TransferEfficiency[Index] = 0.0;
				} else {
					TestStructure.TransferEfficiency[Index] = 0.9999f;
				}
			//}
		}
	}
	
	/*cl_uint SpringAttachPos[] = {1,1};
	TestStructure.Inputs.push_back(
		(APM::Scene::Object::WaveStructure::Plug) {
			.Position = SpringAttachPos,
		}
	);*/
	
	cl_uint WaveOutputPosA[] = {7,8};
	TestStructure.Outputs.push_back(
		(APM::Scene::Object::WaveStructure::Plug) {
			.Position = WaveOutputPosA,
		}
	);
	
	cl_uint WaveOutputPosB[] = {8,7};
	TestStructure.Outputs.push_back(
		(APM::Scene::Object::WaveStructure::Plug) {
			.Position = WaveOutputPosB,
		}
	);
	
	TestScene.Objects.push_back(&TestStructure);
	
	/*TestScene.Connections.push_back((APM::Scene::Description::Connection){
		.SinkObjectID = 1, .SinkPlugID = 0,
		.SourceObjectID = 0, .SourcePlugID = 0,
	});*/
	
	printf("Creating engine...\n");
	APM::Engine TestEngine = APM::Engine(ExecutionBins);
	printf("Launching job processing thread...\n");
	TestEngine.LaunchJobsThread();
	printf("Enqueuing job...\n");
	std::atomic_bool JobComplete{false};
	size_t LastProgress = 1;
	unsigned int LastPercent = 0;
	std::atomic_size_t ProgressTracker = LastProgress;
	cl_uint SampleRate = 44100;
	float LengthInSecs = 3;
	cl_uint LengthInSamples = (float)SampleRate*LengthInSecs;
	float* OutputBufferL = new float[LengthInSamples];
	float* OutputBufferR = new float[LengthInSamples];
	//float* DebugBuffer = new float[LengthInSamples];
	long long StartTime = Utils::Time::Milliseconds();
	TestEngine.EnqueueJob(
		TestScene, 1.0f/(float)SampleRate, LengthInSamples, 64, 
		{
			/*(APM::Engine::Output){
				.Object = 0, .Plug = 0,
				.Buffer = DebugBuffer
			},*/
			(APM::Engine::Output){
				.Object = 0, .Plug = 0,
				.Buffer = OutputBufferL
			},
			(APM::Engine::Output){
				.Object = 0, .Plug = 1,
				.Buffer = OutputBufferR
			}
		}, 
		&ProgressTracker, &JobComplete
	);
	printf("Waiting for completion...\n");
	while (!JobComplete) {
		ProgressTracker.wait(LastProgress);
		printf("\033[2J"); //Clear screen
		LastProgress = ProgressTracker;
		float ValueL = OutputBufferL[LastProgress];
		float ValueR = OutputBufferR[LastProgress];
		//printf("Debug: %f\n", DebugBuffer[LastProgress]);
		printf("L: %f, R: %f\n", ValueL, ValueR);
		unsigned int Percent = floorf(((float)LastProgress/(LengthInSamples-1.0f))*100.0f);
		LastPercent = Percent;
		printf("%i%% complete..\n", LastPercent);
	}
	printf("Took %.2fs\n", (double)(Utils::Time::Milliseconds() - StartTime)/1000.0);
	printf("Writing output .wav files\n");
	float* StereoBuffer[] = {OutputBufferL, OutputBufferR};
	Utils::WriteWAV_File("Output.wav", SampleRate, 2, LengthInSamples, StereoBuffer);
	//float* MonoBuffer[] = {DebugBuffer};
	//Utils::WriteWAV_File("Output_Debug.wav", SampleRate, 1, LengthInSamples, MonoBuffer);
	delete[] OutputBufferL;
	delete[] OutputBufferR;
	//delete[] DebugBuffer;
	printf("Shutting down job processing thread...\n");
	TestEngine.StopJobsThread();
	printf("Deleting execution bins...\n");
	ExecutionBins.clear();
	printf("Bye.\n");
	return 0;
}
