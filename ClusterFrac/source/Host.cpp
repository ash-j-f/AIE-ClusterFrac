#include "Host.h"

namespace cf
{
	Host::Host()
	{
		started = false;

		//Default port number.
		port = 5000;

		//Should the host process tasks like a client, by default?
		hostAsClient = true;
	}
	
	Host::~Host()
	{

		if (hostAsClientTaskProcessThreadRun)
		{
			//Shut down host as client processing.
			hostAsClientTaskProcessThreadRun = false;

			//Wait forhost as client task processing thread to finish.
			if (hostAsClientTaskProcessingThread.joinable()) hostAsClientTaskProcessingThread.join();
		}

		//Stop the listener.
		listener.stop();

		//Clean up any remaining registered clients.
		for (auto &c : clients) delete c;
	}

	void Host::start()
	{
		//If host already started, do nothing.
		if (started)
		{
			CF_SAY("Host already started.", Settings::LogLevels::Error);
			return;
		}

		started = true;

		CF_SAY("Starting ClusterFrac HOST at " + sf::IpAddress::getLocalAddress().toString() + " on port " + std::to_string(port) + ".", Settings::LogLevels::Info);

		listener.start();

		//Launch host as client task processing thread.
		if (hostAsClient)
		{
			hostAsClientTaskProcessThreadRun = true;
			hostAsClientTaskProcessingThread = std::thread([this] { hostAsClientProcessTaskThread(); });
		}
	}

	void Host::setPort(int portNum)
	{
		if (portNum > 0 && portNum <= 65535) 
		{ 
			port = portNum; 
		}
		else 
		{ 
			std::string s = "Invalid port number."; 
			CF_SAY(s, Settings::LogLevels::Error); 
			throw s;
		};
	}

	void Host::addTaskToQueue(Task *task)
	{
		std::unique_lock<std::mutex> lock(taskQueueMutex);
		taskQueue.push_back(task);
		CF_SAY("Added task " + std::to_string(task->getInitialTaskID()) + " to queue.", Settings::LogLevels::Info);
	}

	bool Host::sendTasks()
	{

		std::unique_lock<std::mutex> lock(taskQueueMutex);

		if (getClientsCount() < 1)
		{
			CF_SAY("Cannot send tasks. No clients connected.", Settings::LogLevels::Error);
			return false;
		}

		if (taskQueue.size() == 0)
		{
			CF_SAY("Cannot send tasks. No tasks in queue.", Settings::LogLevels::Error);
			return false;
		}

		//Divide tasks among clients.
		std::vector<Task *> subTaskQueue;
		std::vector<Task *> dividedTasks;
		CF_SAY("Dividing tasks among clients.", Settings::LogLevels::Info);
		for (auto &task : taskQueue)
		{
			dividedTasks = task->split(getClientsCount());
			subTaskQueue.insert(subTaskQueue.end(), dividedTasks.begin(), dividedTasks.end());
		}

		//Distribute tasks among available clients.

		//Send one task chunk to the host itself if hostAsClient is enabled.
		if (hostAsClient)
		{
			std::unique_lock<std::mutex> lock2(localHostAsClientTaskQueueMutex);
			CF_SAY("Sending task to local client.", Settings::LogLevels::Info);
			localHostAsClientTaskQueue.push_back(subTaskQueue.back());
			subTaskQueue.pop_back();
			lock2.unlock();
		}

		CF_SAY("Sending tasks to remote clients.", Settings::LogLevels::Info);
		std::vector<Task *>::iterator it = subTaskQueue.begin();
		while (it != subTaskQueue.end())
		{
			//Search for the next available client.
			ClientDetails *freeClient = nullptr;
			for (auto &client : clients)
			{
				//Skip busy clients.
				if (client->busy) continue;
				freeClient = client;
			}

			if (freeClient != nullptr)
			{
				freeClient->busy = true;
				Task *task = *it;
				it++;
				sender.sendTask(freeClient, task);				
			}
			
		}

		//Empty the task queue.
		taskQueue.clear();

		//Wait for threads to finish.
		sender.waitForComplete();

		//Destroy the subtask copies and empty the subtask list.
		for (auto &task : subTaskQueue) delete task;
		subTaskQueue.clear();

		CF_SAY("Task sending finished.", Settings::LogLevels::Info);

		return true;
	}

	void Host::addResultToQueue(Result *result)
	{
		std::unique_lock<std::mutex> lock(resultsQueueMutex);
		resultQueueIncomplete.push_back(result);
		CF_SAY("Added result to queue.", Settings::LogLevels::Info);
	}

	Result *Host::getAvailableResult(int taskID)
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lock(resultsQueueMutex);

		if (resultQueueComplete.size() > 0)
		{
			for (auto &r : resultQueueComplete)
			{
				if (r->getInitialTaskID() == taskID) return r;
			}
		}
		
		//No such task ID found.
		return nullptr;
	}

	void Host::setHostAsClient(bool state)
	{

		//Do nothing if the state isn't changing.
		if (hostAsClient == state) return;

		//Check if the host has already been started, then start or stop host as client processing thread as required.
		if (started)
		{
			//Start processing thread if it has not already been started, and the host is being set to process tasks locally.
			if (hostAsClientTaskProcessThreadRun == false && state)
			{
				//Shut down host as client processing.
				hostAsClientTaskProcessThreadRun = true;

				//Launch host as client task processing thread.
				if (hostAsClient) hostAsClientTaskProcessingThread = std::thread([this] { hostAsClientProcessTaskThread(); });
			}

			//Shut down processing thread if it has been started, and the host is being set to no longer process tasks locally.
			if (hostAsClientTaskProcessThreadRun && !state)
			{
				//Shut down host as client processing.
				hostAsClientTaskProcessThreadRun = false;

				//Wait forhost as client task processing thread to finish.
				if (hostAsClientTaskProcessingThread.joinable()) hostAsClientTaskProcessingThread.join();
			}
		}

		hostAsClient = state;
	}

	inline int Host::getClientsCount() const
	{
		int count = 0;

		//Count connected clients.
		for (auto &c : clients)
		{
			if (!c->remove) count++;
		}

		if (hostAsClient) count++;

		return count;
	}

	bool Host::checkAvailableResult(int taskID)
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lock(resultsQueueMutex);

		if (resultQueueComplete.size() > 0)
		{
			for (auto &r : resultQueueComplete)
			{
				if (r->getInitialTaskID() == taskID) return true;
			}
		}

		//No such taks ID or no such results set available yet.
		return false;
	}

	void Host::hostAsClientProcessTaskThread()
	{
		while (hostAsClientTaskProcessThreadRun)
		{
			//Copy the local task queue.
			std::unique_lock<std::mutex> lock(localHostAsClientTaskQueueMutex);
			std::list<Task *> localHostAsClientTaskQueueCOPY = localHostAsClientTaskQueue;
			lock.unlock();

			for (auto &t : localHostAsClientTaskQueueCOPY)
			{
				CF_SAY("Processing task locally - started.", Settings::LogLevels::Info);
				
				//Split the task among available threads and run.
				std::vector<cf::Task *> tasks = t->split(std::thread::hardware_concurrency());

				std::vector<std::future<cf::Result *>> threads = std::vector<std::future<cf::Result *>>();

				//Start benchmark timer.
				auto start = std::chrono::steady_clock::now();

				for (auto &task : tasks)
				{
					threads.push_back(std::async(std::launch::async, [&task]() { return task->run(); }));
				}

				std::vector<cf::Result *> results;

				for (auto &thread : threads)
				{
					CF_SAY("Processing task locally - waiting for results and merging.", Settings::LogLevels::Info);
					auto result = thread.get();
					results.push_back(result);
				}

				//Stop benchmark test clock.
				auto end = std::chrono::steady_clock::now();
				auto diff = end - start;

				CF_SAY("Local computation time: " + std::to_string(std::chrono::duration <double, std::milli>(diff).count()) + " ms.", Settings::LogLevels::Info);

				cf::Result *result = resultConstructMap[results.front()->getSubtype()]();
				result->merge(results);

				//Clean up temporary results objects.
				for (auto &r : results) delete r;

				CF_SAY("Processing task locally - completed.", Settings::LogLevels::Info);

				//Place the result in the host result parts queue.
				std::unique_lock<std::mutex> lock2(resultsQueueMutex);
				resultQueueIncomplete.push_back(result);
				lock2.unlock();

				//Scan the incomplete results queue for complete results sets and move them to the complete results queue.
				checkForCompleteResults();

				std::unique_lock<std::mutex> lock3(localHostAsClientTaskQueueMutex);
				//Remove the task from memory.
				delete t;
				//Remove the task from the local task parts queue.
				localHostAsClientTaskQueue.erase(std::remove(localHostAsClientTaskQueue.begin(), 
					localHostAsClientTaskQueue.end(), t), localHostAsClientTaskQueue.end());
				lock3.unlock();
			}
		}
	}

	void Host::checkForCompleteResults()
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lock(resultsQueueMutex);

		std::vector<Result *> set;
		std::vector<Result *> remove;
		for (auto &r1 : resultQueueIncomplete)
		{
			//If this result part is already in the removal list, then skip it.
			if (remove.size() > 0 && std::find(remove.begin(), remove.end(), r1) != remove.end()) continue;

			set.push_back(r1);

			for (auto &r2 : resultQueueIncomplete)
			{
				//Skip checking against ourself.
				if (r1 == r2) continue;

				//If this result part is already in the removal list, then skip it.
				if (remove.size() > 0 && std::find(remove.begin(), remove.end(), r2) != remove.end()) continue;

				//Compare initial task ID of the two result parts.
				if (r2->getInitialTaskID() == r1->getInitialTaskID()) set.push_back(r2);
			}

			//If all parts of the result set are present, merge them and place the 
			//combined result on the completed results queue.
			if (set.size() == r1->getCurrentTaskPartsTotal())
			{
				cf::Result *rNew = resultConstructMap[r1->getSubtype()]();
				
				rNew->merge(set);

				resultQueueComplete.push_back(rNew);

				//Record these results for removal from the incomplete results set.
				remove.insert(remove.end(), set.begin(), set.end());

			}
			set.clear();
		}

		for (auto &r : remove)
		{
			//Delete the result part from memory.
			delete r;

			//Remove completed results from incomplete results set.
			resultQueueIncomplete.erase(std::remove(resultQueueIncomplete.begin(), resultQueueIncomplete.end(), r), resultQueueIncomplete.end());
		}

	}

}