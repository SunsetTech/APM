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
			Engine(std::vector<ExecutionBin*> ExecutionBins);
			void LaunchJobsThread();
			void StopJobsThread();
			void EnqueueJob(Scene::Description Scene, float TimeDelta, size_t Iterations, std::vector<float*> Outputs, std::atomic_bool* CompletionEvent);
		private:
			struct Job {
				Scene::Description Scene;
				float TimeDelta;
				size_t Iterations;
				std::vector<float*> Outputs;
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
