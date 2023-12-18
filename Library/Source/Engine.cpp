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
	
	void Engine::EnqueueJob(Scene::Description Scene, float TimeDelta, size_t Iterations, size_t BlockSize, std::vector<Output> Outputs, std::atomic_size_t* ProgressTracker, std::atomic_bool* CompletionSignal) {
		std::lock_guard<std::mutex> lock(this->QueueMutex);
		this->Jobs.push((Job){
			.Scene = Scene,
			.TimeDelta = TimeDelta,
			.Iterations = Iterations,
			.BlockSize = BlockSize,
			.Outputs = Outputs,
			.ProgressTracker = ProgressTracker,
			.CompletionSignal = CompletionSignal,
		});
		this->JobCount++;
		this->CheckForWork = true;
		this->CheckForWork.notify_one();
	}
	
	void Engine::ProcessBlock(std::vector<Scene::RenderDispatcher::Base::Task *> Tasks, std::vector<Scene::Description::Connection> Connections, std::vector<Output> Outputs, cl_uint StartIteration, size_t BlockSize, float TimeDelta) {
		cl_event* ReadyEvents = new cl_event[Tasks.size()];
		for (size_t TaskIndex = 0; TaskIndex < Tasks.size(); TaskIndex++) {
			Scene::RenderDispatcher::Base::Task* CurrentTask = Tasks[TaskIndex];
			cl_event ExecutionEvent, FlushEvent;
			CurrentTask->EnqueueFlushMemory(BlockSize, 0, nullptr, &FlushEvent);
			CurrentTask->EnqueueExecution(StartIteration, BlockSize, TimeDelta, 1, &FlushEvent, &ExecutionEvent);
			cl_int Err = clReleaseEvent(FlushEvent);
			CLUtils::PrintAndHaltIfError("Releasing FlushEvent", Err);
			CurrentTask->EnqueueReadyMemory(BlockSize, 1, &ExecutionEvent, ReadyEvents+TaskIndex);
			Err = clReleaseEvent(ExecutionEvent);
			CLUtils::PrintAndHaltIfError("Releasing ExecutionEvent", Err);
		}
		
		for (size_t EventIndex = 0; EventIndex < Tasks.size(); EventIndex++) {
			clWaitForEvents(1, ReadyEvents + EventIndex);
			clReleaseEvent(ReadyEvents[EventIndex]);
		}
		
		//Process connections
		/*for (size_t ConnectionIndex = 0; ConnectionIndex < Connections.size(); ConnectionIndex++) {
			Scene::Description::Connection Connection = Connections[ConnectionIndex];
			Scene::RenderDispatcher::Base::Task* Dest = Tasks[Connection.SinkObjectID];
			Scene::RenderDispatcher::Base::Task* Src = Tasks[Connection.SourceObjectID];
			float Value = Src->GetSourceValue(Connection.SourcePlugID, Timestep);
			Dest->SetSinkValue(Connection.SinkPlugID, Timestep, Value);
		}*/
		
		//Process outputs
		for (size_t OutputIndex = 0; OutputIndex < Outputs.size(); OutputIndex++) {
			for (cl_uint SubIteration = 0; SubIteration < BlockSize; SubIteration++) {
				Output CurrentOutput = Outputs[OutputIndex];
				float Value = Tasks[CurrentOutput.Object]->GetSourceValue(CurrentOutput.Plug, SubIteration);
				CurrentOutput.Buffer[StartIteration+SubIteration] = Value;
			}
		}
		
		delete[] ReadyEvents;
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
							Tasks.push_back((*DispatcherIterator)->CreateTask(*ObjectIterator, CurrentJob.BlockSize));
							continue;
						}
					}
				}
				
				size_t BlockIterations = CurrentJob.Iterations / CurrentJob.BlockSize;
				size_t Remainder = CurrentJob.Iterations % CurrentJob.BlockSize;
				for (size_t BlockIteration = 0; BlockIteration < BlockIterations; BlockIteration++) {
					Engine::ProcessBlock(Tasks, CurrentJob.Scene.Connections, CurrentJob.Outputs, BlockIteration*CurrentJob.BlockSize, CurrentJob.BlockSize, CurrentJob.TimeDelta);
					CurrentJob.ProgressTracker->store(BlockIteration*CurrentJob.BlockSize);
					CurrentJob.ProgressTracker->notify_all();
				}
				if (Remainder > 0) {
					Engine::ProcessBlock(Tasks, CurrentJob.Scene.Connections, CurrentJob.Outputs, BlockIterations*CurrentJob.BlockSize, Remainder, CurrentJob.TimeDelta);
					CurrentJob.ProgressTracker->store(CurrentJob.Iterations-1);
					CurrentJob.ProgressTracker->notify_all();
				};
				CurrentJob.CompletionSignal->store(true);
				CurrentJob.CompletionSignal->notify_all();
			}
		}
		this->JobsThreadRunning.store(false);
		this->JobsThreadRunning.notify_all();
	}
}
