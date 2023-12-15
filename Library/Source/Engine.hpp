#include <CL/cl.h>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <queue>

#include "ExecutionBin.hpp"
#include "Scene/Description.hpp"

namespace APM {
	class Engine {
		public:
			struct Output {
				size_t Object, Plug;
				float* Buffer;
			};
			
			Engine(std::vector<ExecutionBin*> ExecutionBins);
			void LaunchJobsThread();
			void StopJobsThread();
			void EnqueueJob(Scene::Description Scene, float TimeDelta, size_t Iterations, size_t BlockSize, std::vector<Output> Outputs, std::atomic_size_t* ProgressTracker, std::atomic_bool* CompletionEvent);
			static void ProcessBlock(std::vector<Scene::RenderDispatcher::Base::Task*> Tasks, std::vector<Scene::Description::Connection> Connections, std::vector<Output> Outputs, cl_uint Iteration, size_t BlockSize, float TimeDelta);
		private:
			struct Job {
				Scene::Description Scene;
				float TimeDelta;
				size_t Iterations;
				size_t BlockSize;
				std::vector<Output> Outputs;
				std::atomic_size_t* ProgressTracker;
				std::atomic_bool* CompletionSignal;
			};
			
			std::thread JobsThread;
			std::mutex QueueMutex;
			std::atomic_bool JobsThreadRunning, CheckForWork, ShutdownJobsThread;
			std::atomic_size_t JobCount;
			std::queue<Job> Jobs;
			std::vector<ExecutionBin*> ExecutionBins; //TODO
			
			void ProcessJobs();
	};
}
