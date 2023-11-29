#include <ctime>
#include <cmath>

#include <CL/cl.h>

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
			if (Answer == 'n' || Answer == 'N') continue;
			free(DeviceName);
			
			APM::ExecutionBin* DeviceBin = new APM::ExecutionBin(Devices[DeviceIndex]);
			DeviceBin->AttachDispatcher<APM::Scene::RenderDispatcher::SpringBundle>();
			DeviceBin->AttachDispatcher<APM::Scene::RenderDispatcher::WaveStructure>();
			ExecutionBins.push_back(DeviceBin);
		}
	}
	//TODO benchmark selected devices for initial performance ordering
	printf("Setting up scene...\n");
	APM::Scene::Description TestScene;
	APM::Scene::Object::SpringBundle TestBundle(2,16);
	for (unsigned int FiberIndex = 0; FiberIndex < 64; FiberIndex++) {
		TestBundle.NodeParameterBuffer[TestBundle.MapIndex(FiberIndex, 0 )].Fixed = true;
		for (unsigned int NodeIndex = 1; NodeIndex < 16; NodeIndex++) {
			size_t BufferIndex = TestBundle.MapIndex(FiberIndex, NodeIndex);
			TestBundle.NodeParameterBuffer[BufferIndex].Mass = 1.0f;
			TestBundle.NodeParameterBuffer[BufferIndex].Damping = 1.0f;
			TestBundle.SpringParameterBuffer[BufferIndex] = {
				.RestLength = 1.0f,
				.Stiffness = 1.0f,
			};
			TestBundle.SpaceBuffer[BufferIndex].Position = (float)NodeIndex;
		}
		TestBundle.NodeParameterBuffer[TestBundle.MapIndex(FiberIndex, 15)].Fixed = true;
	}
	TestBundle.SpaceBuffer[TestBundle.MapIndex(0,1)].Position -= 0.5f;
	TestBundle.Outputs.push_back({0,14,14.0f});
	TestBundle.Inputs.push_back({1,1,1.0f});
	TestBundle.Outputs.push_back({1,2,2.0f});
	TestScene.Objects.push_back(&TestBundle);
	
	cl_uint TestStructureBounds[] = {64, 64, 64};
	APM::Scene::Object::WaveStructure TestStructure(3, TestStructureBounds);
	cl_uint Cursor[3];
	for (cl_uint X = 0; X < TestStructureBounds[0]; X++) {
		for (cl_uint Y = 0; Y < TestStructureBounds[1]; Y++) {
			for (cl_uint Z = 0; Z < TestStructureBounds[2]; Z++) {
				Cursor[0] = X;
				Cursor[1] = Y;
				Cursor[2] = Z;
				TestStructure.SpaceBuffer[TestStructure.MapIndex(Cursor)] = 0.0f;
				if (
					   X == 0 || X == TestStructureBounds[0]-1
					|| Y == 0 || Y == TestStructureBounds[1]-1
					|| Z == 0 || Z == TestStructureBounds[2]-1
				) {
					TestStructure.CellParameterBuffer[TestStructure.MapIndex(Cursor)].TransferEfficiency = 0;
				} else {
					TestStructure.CellParameterBuffer[TestStructure.MapIndex(Cursor)].TransferEfficiency = 1.0f-(1.0f/4000.0f);
					TestStructure.CellParameterBuffer[TestStructure.MapIndex(Cursor)].WaveVelocity = powf(1.0f, 2.0f);
				}
			}
		}
	}
	
	cl_uint SpringAttachPos[3] = {1,1,1};
	TestStructure.Inputs.push_back(
		(APM::Scene::Object::WaveStructure::Plug) {
			.Position = SpringAttachPos,
		}
	);
	
	cl_uint WaveOutputPos[3] = {62, 62, 62};
	TestStructure.Outputs.push_back(
		(APM::Scene::Object::WaveStructure::Plug) {
			.Position = WaveOutputPos,
		}
	);
	
	TestScene.Objects.push_back(&TestStructure);
	
	/*TestScene.Connections.push_back((APM::Scene::Description::Connection){
		.SinkObjectID = 0, .SinkPlugID = 0,
		.SourceObjectID = 1, .SourcePlugID = 0,
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
	float* OutputBuffer = (float*)calloc(44100,sizeof(float));
	TestEngine.EnqueueJob(
		TestScene, 1.0f/44100.0f, 44100, 
		{
			(APM::Engine::Output){
				.Object = 1, .Plug = 0,
				.Buffer = OutputBuffer
			}
		}, 
		&ProgressTracker, &JobComplete
	);
	printf("Waiting for completion...\n");
	while (!JobComplete) {
		ProgressTracker.wait(LastProgress);
		printf("\033[2J"); //Clear screen
		LastProgress = ProgressTracker;
		float Value = OutputBuffer[LastProgress];
		printf("Node displacement %f\n", Value);
		unsigned int Percent = floorf(((float)LastProgress/44099.0f)*100.0f);
		if (Percent > LastPercent) {
			LastPercent = Percent;
		}
		printf("%i%% complete..\n", LastPercent);
	}
	printf("Shutting down job processing thread...\n");
	TestEngine.StopJobsThread();
	printf("Deleting execution bins...\n");
	for (unsigned int BinIndex = 0; BinIndex < ExecutionBins.size(); BinIndex++) {
		delete ExecutionBins[BinIndex];
	}
	printf("Bye.\n");
	return 0;
}
