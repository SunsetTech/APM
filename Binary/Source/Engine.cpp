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
	
	void Engine::EnqueueJob(Scene::Description Scene, float TimeDelta, size_t Iterations, std::vector<Output> Outputs, std::atomic_size_t* ProgressTracker, std::atomic_bool* CompletionSignal) {
		std::lock_guard<std::mutex> lock(this->QueueMutex);
		this->Jobs.push((Job){
			.Scene = Scene,
			.TimeDelta = TimeDelta,
			.Iterations = Iterations,
			.Outputs = Outputs,
			.ProgressTracker = ProgressTracker,
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
				
				cl_event* CompletionEvents = new cl_event[Tasks.size()];
				cl_event* MapEvents = new cl_event[Tasks.size()];
				cl_event* UnmapEvents = new cl_event[Tasks.size()];
				for (size_t Iteration = 0; Iteration < CurrentJob.Iterations; Iteration++) {
					cl_uint Timestep = (Iteration+1)%2;
					//Process simulations
					for (size_t TaskIndex = 0; TaskIndex < Tasks.size(); TaskIndex++) {
						Tasks[TaskIndex]->EnqueueExecution(CurrentJob.TimeDelta, Timestep, 0, NULL, CompletionEvents + TaskIndex);
						Tasks[TaskIndex]->EnqueueReadMemory(1, CompletionEvents + TaskIndex, MapEvents + TaskIndex);
					}
					for (size_t EventIndex = 0; EventIndex < Tasks.size(); EventIndex++) {
						clWaitForEvents(1, MapEvents + EventIndex);
						clReleaseEvent(MapEvents[EventIndex]);
						clReleaseEvent(CompletionEvents[EventIndex]);
					}
					
					//Process connections
					for (size_t ConnectionIndex = 0; ConnectionIndex < CurrentJob.Scene.Connections.size(); ConnectionIndex++) {
						Scene::Description::Connection Connection = CurrentJob.Scene.Connections[ConnectionIndex];
						
						Tasks[Connection.SinkObjectID]->SetSinkValue(
							Connection.SinkPlugID, Timestep,
							Tasks[Connection.SourceObjectID]->GetSourceValue(Connection.SourcePlugID, Timestep)
						);
					}
					
					//Process outputs
					for (size_t OutputIndex = 0; OutputIndex < CurrentJob.Outputs.size(); OutputIndex++) {
						Output CurrentOutput = CurrentJob.Outputs[OutputIndex];
						float Value = Tasks[CurrentOutput.Object]->GetSourceValue(CurrentOutput.Plug, Timestep);
						CurrentOutput.Buffer[Iteration] = Value;
					}
					
					//Commit changes
					for (size_t TaskIndex = 0; TaskIndex < Tasks.size(); TaskIndex++) {
						Tasks[TaskIndex]->EnqueueWriteMemory(0, NULL, UnmapEvents + TaskIndex);
					}
					for (size_t EventIndex = 0; EventIndex < Tasks.size(); EventIndex++) {
						clWaitForEvents(1, UnmapEvents + EventIndex);
						clReleaseEvent(UnmapEvents[EventIndex]);
					}
					
					CurrentJob.ProgressTracker->store(Iteration); //TODO store -1 as first value that way we get proper notification when sample 0 is ready
					CurrentJob.ProgressTracker->notify_all();
				}
				delete[] CompletionEvents;
				delete[] MapEvents;
				delete[] UnmapEvents;
				CurrentJob.CompletionSignal->store(true);
				CurrentJob.CompletionSignal->notify_all();
			}
		}
		this->JobsThreadRunning.store(false);
		this->JobsThreadRunning.notify_all();
	}
}
