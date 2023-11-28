#include "Engine.hpp"
#include "CLUtils.hpp"
#include "Scene/RenderDispatcher/Base.hpp"
#include <cmath>

namespace APM {
	Engine::Engine(std::vector<ExecutionBin*> _ExecutionBins) {
		this->ExecutionBins = _ExecutionBins;
	}
	
	void Engine::LaunchJobsThread() {
		this->JobsThread = std::thread(&Engine::ProcessJobs, this);
	}
	
	void Engine::StopJobsThread() {
		this->ShutdownJobsThread = true;
		this->CheckForWork = true;
		this->CheckForWork.notify_all();
		this->JobsThread.join();
	}
	
	void Engine::EnqueueJob(Scene::Description Scene, float TimeDelta, size_t Iterations, std::vector<float*> Outputs, std::atomic_bool* CompletionSignal) {
		std::lock_guard<std::mutex> lock(this->QueueMutex);
		this->Jobs.push((Job){
			.Scene = Scene,
			.TimeDelta = TimeDelta,
			.Iterations = Iterations,
			.Outputs = Outputs,
			.CompletionSignal = CompletionSignal,
		});
		this->JobCount++;
		this->CheckForWork = true;
		this->CheckForWork.notify_one();
	}
	
	void Engine::ProcessJobs() {
		this->JobsThreadRunning = true;
		while (true) {
			this->CheckForWork.wait(false);
			this->CheckForWork = false;
			if (this->ShutdownJobsThread == true) {
				break;
			}
			while (this->JobCount > 0) {
				this->JobCount--;
				std::unique_lock<std::mutex> Lock(this->QueueMutex);
				Job CurrentJob = this->Jobs.front();
				this->Jobs.pop();
				Lock.release();
				
				size_t SmallestTaskSize = std::numeric_limits<size_t>::max();
				size_t LargestTaskSize = 0;
				
				for (auto ObjectIterator = CurrentJob.Scene.Objects.begin(); ObjectIterator != CurrentJob.Scene.Objects.end(); ObjectIterator++) {
					size_t TaskSize = (*ObjectIterator)->GetTaskSize();
					SmallestTaskSize = std::min(TaskSize, SmallestTaskSize);
					LargestTaskSize = std::max(TaskSize, LargestTaskSize);
				}
				
				std::vector<Scene::RenderDispatcher::Base::Task*> Tasks;
				size_t TaskSizeRange = LargestTaskSize - SmallestTaskSize;
				for (auto ObjectIterator = CurrentJob.Scene.Objects.begin(); ObjectIterator != CurrentJob.Scene.Objects.end(); ObjectIterator++) {
					size_t TaskSize = (*ObjectIterator)->GetTaskSize();
					size_t Distance = TaskSize - SmallestTaskSize;
					float Scale = (float)Distance/(float)TaskSizeRange;
					size_t BinIndex = ExecutionBins.size() - 1 - roundf(Scale*((float)ExecutionBins.size()-1.0f)); //TODO check this is right
					ExecutionBin* Bin = ExecutionBins[BinIndex];
					
					for (auto DispatcherIterator = Bin->Dispatchers.begin(); DispatcherIterator != Bin->Dispatchers.end(); DispatcherIterator++) {
						if ((*DispatcherIterator)->Handles(*ObjectIterator)) {
							Tasks.push_back((*DispatcherIterator)->CreateTask(*ObjectIterator));
						}
					}
				}
				
				cl_event* CompletionEvents = (cl_event*)calloc(Tasks.size(), sizeof(cl_event));
				for (size_t Iteration = 0; Iteration < CurrentJob.Iterations; Iteration++) {
					for (size_t TaskIndex = 0; TaskIndex < Tasks.size(); TaskIndex++) {
						Tasks[TaskIndex]->EnqueueExecution(CurrentJob.TimeDelta, (Iteration+1)%2, 0, NULL, CompletionEvents + TaskIndex);
					}
					clWaitForEvents(Tasks.size(), CompletionEvents);
					for (size_t EventIndex = 0; EventIndex < Tasks.size(); EventIndex++) {
						clReleaseEvent(CompletionEvents[EventIndex]);
					}
					
					for (size_t ConnectionIndex = 0; ConnectionIndex < CurrentJob.Scene.Connections.size(); ConnectionIndex++) {
						Scene::Description::Connection Connection = CurrentJob.Scene.Connections[ConnectionIndex];
						
						Tasks[Connection.SinkObjectID]->SetSinkValue(
							Connection.SinkPlugID, 
							Tasks[Connection.SourceObjectID]->GetSourceValue(Connection.SourcePlugID)
						);
					}
				}
				free(CompletionEvents);
				CurrentJob.CompletionSignal->store(true);
				CurrentJob.CompletionSignal->notify_all();
			}
		}
		this->JobsThreadRunning.store(false);
		this->JobsThreadRunning.notify_all();
	}
}
