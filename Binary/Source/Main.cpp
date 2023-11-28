#include <ctime>
#include <cmath>

#include <CL/cl.h>

#include "Engine.hpp"
#include "ExecutionBin.hpp"
#include "Scene/Description.hpp"
#include "Scene/Object/SpringBundle.hpp"
#include "Scene/RenderDispatcher/SpringBundle.hpp"

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
			
			printf("Use %s? (y/n): \n", DeviceName);
			char Answer = getchar(); while(getchar() != '\n'){};
			if (Answer == 'n' || Answer == 'N') continue;
			printf("Setting up Execution Bin for device %i '%s'\n", DeviceIndex, DeviceName);
			free(DeviceName);
			
			APM::ExecutionBin* DeviceBin = new APM::ExecutionBin(Devices[DeviceIndex]);
			DeviceBin->AttachDispatcher<APM::Scene::RenderDispatcher::SpringBundle>();
			ExecutionBins.push_back(DeviceBin);
		}
	}
	//TODO benchmark selected devices for initial performance ordering
	printf("Setting up scene...\n");
	APM::Scene::Description TestScene;
	APM::Scene::Object::SpringBundle TestBundle(64,16);
	TestScene.Objects.push_back(&TestBundle);
	for (unsigned int FiberIndex = 0; FiberIndex < 64; FiberIndex++) {
		TestBundle.NodeParameterBuffer[TestBundle.MapIndex(FiberIndex, 0 )].Fixed = true;
		for (unsigned int NodeIndex = 1; NodeIndex < 15; NodeIndex++) {
			size_t BufferIndex = TestBundle.MapIndex(FiberIndex, NodeIndex);
			TestBundle.NodeParameterBuffer[BufferIndex].Mass = 1.0f;
			TestBundle.NodeParameterBuffer[BufferIndex].Damping = 0.9999f;
			TestBundle.SpringParameterBuffer[BufferIndex] = {
				.RestLength = 0.5f,
				.Stiffness = 1.0f,
			};
			TestBundle.SpaceBuffer[BufferIndex].Position = (float)NodeIndex;
		}
		TestBundle.NodeParameterBuffer[TestBundle.MapIndex(FiberIndex, 15)].Fixed = true;
	}
	printf("Creating engine...\n");
	APM::Engine TestEngine = APM::Engine(ExecutionBins);
	printf("Launching job processing thread...\n");
	TestEngine.LaunchJobsThread();
	std::atomic_bool JobComplete{false};
	printf("Enqueuing job...\n");
	TestEngine.EnqueueJob(TestScene, 1.0f/44100.0f, 100, {}, &JobComplete);
	printf("Waiting for completion...\n");
	JobComplete.wait(false);
	printf("It's done.\n");
	printf("Shutting down job processing thread...\n");
	TestEngine.StopJobsThread();
	printf("Deleting execution bins...\n");
	for (unsigned int BinIndex = 0; BinIndex < ExecutionBins.size(); BinIndex++) {
		delete ExecutionBins[BinIndex];
	}
	printf("Bye.\n");
	return 0;
}
