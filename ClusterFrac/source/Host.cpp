#include "Host.h"

namespace cf
{
	Host::Host()
	{

		//Use default max thread concurrency setting determined by the OS.
		MAX_THREADS = std::thread::hardware_concurrency();

		started = false;

		//Default port number.
		port = 5000;

		//Should the host process tasks like a client, by default?
		hostAsClient = true;

		//Default busy status.
		busy = false;

		//Max benchmark elapsed times to store.
		maxBenchmarkTimes = 100;

	}
	
	Host::~Host()
	{
		stop();
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

		//Reset time since startup.
		clock.restart();

		CF_SAY("Starting ClusterFrac HOST at " + sf::IpAddress::getLocalAddress().toString() + " on port " + std::to_string(port) + ".", Settings::LogLevels::Info);
		listener.start();

		watcher.start();

		//Launch host as client task processing thread.
		if (hostAsClient)
		{
			hostAsClientTaskProcessThreadRun = true;
			hostAsClientTaskProcessingThread = std::thread([this] { hostAsClientProcessTaskThread(); });
		}
	}

	void Host::stop()
	{
		if (hostAsClientTaskProcessThreadRun)
		{
			//Shut down host as client processing.
			hostAsClientTaskProcessThreadRun = false;

			//Wait for host as client task processing thread to finish.
			if (hostAsClientTaskProcessingThread.joinable()) hostAsClientTaskProcessingThread.join();
		}

		//Stop the listener.
		listener.stop();

		//Stop the task status watcher.
		watcher.stop();

		//Clean up any remaining registered clients.
		std::unique_lock<std::mutex> clientsLock(clientsMutex);
		for (auto &c : clients)
		{
			if (!c->remove)
			{
				//Send disconnect message to client.
				std::unique_lock<std::mutex> socketLock(c->socketMutex);
				c->socket->disconnect();
			}

			delete c;
			c = nullptr;
		}
		clientsLock.unlock();

		//Clean up any tasks and results in queues.
		//Build an unordered set of task/result pointers in case the same
		//pointer is recorded in two places due to an unusual shutdown.
		//Unordered set will ignore duplicate entries.

		std::unordered_set<Task *> removeTasks;
		for (auto &t : localHostAsClientTaskQueue) removeTasks.insert(t);
		for (auto &t : taskQueue) removeTasks.insert(t);
		for (auto &t : removeTasks) delete t;

		std::unordered_set<Result *> removeResults;
		for (auto &r : resultQueueComplete) removeResults.insert(r);
		for (auto &r : resultQueueIncomplete) removeResults.insert(r);
		for (auto &r : removeResults) delete r;
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
			CF_THROW(s);
		};
	}

	void Host::addTaskToQueue(Task *task)
	{
		//Ensure this task has an ID assigned.
		task->assignID();
		std::unique_lock<std::mutex> lock(taskQueueMutex);
		taskQueue.push_back(task);
		CF_SAY("Added task " + std::to_string(task->getInitialTaskID()) + " to queue.", Settings::LogLevels::Info);
	}

	bool Host::divideTasksIntoSubTaskQueue()
	{

		int clientCount = getClientsCount();

		std::unique_lock<std::mutex> taskQueueLock(taskQueueMutex);

		if (clientCount < 1)
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
		std::vector<Task *> tmpSubTaskQueue;
		std::vector<Task *> dividedTasks;

		CF_SAY("Dividing tasks among clients.", Settings::LogLevels::Info);
		for (auto &task : taskQueue)
		{
			//Only divide task if there's more than one client, and the task allows itself to be split,
			//and allows itself to be run on remote clients. Otherwise just use pointer to the original task.
			if (task->allowNodeTaskSplit && task->getNodeTargetType() != cf::Task::NodeTargetTypes::Local && clientCount > 1)
			{
				dividedTasks = task->split(clientCount);
				//Remove original task from memory.
				delete task;
				task = nullptr;
			}
			else
			{
				//Just use the task pointer and don't split the task.
				//We DON'T remove the original task queue pointer object from memory here as we'll keep using it.
				dividedTasks = std::vector<Task *>{task};
			}
			tmpSubTaskQueue.insert(tmpSubTaskQueue.end(), dividedTasks.begin(), dividedTasks.end());
		}

		//Empty the main task queue.
		taskQueue.clear();

		taskQueueLock.unlock();

		//Add sub tasks to sub task queue. 
		std::unique_lock<std::mutex> subTaskQueueLock(subTaskQueueMutex);
		subTaskQueue.insert(subTaskQueue.end(), tmpSubTaskQueue.begin(), tmpSubTaskQueue.end());
		subTaskQueueLock.unlock();

		return true;
	}

	void Host::addResultToQueue(Result *result)
	{
		std::unique_lock<std::mutex> lock(resultsQueueIncompleteMutex);
		resultQueueIncomplete.push_back(result);
		CF_SAY("Added result to queue.", Settings::LogLevels::Info);
	}

	void Host::removeResultFromQueue(Result *result)
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lock(resultsQueueCompleteMutex);
		if (std::find(resultQueueComplete.begin(), resultQueueComplete.end(), result) == resultQueueComplete.end())
		{
			CF_THROW("Remove failed. Cannot find that result in the completed results queue.");
		}
		resultQueueComplete.erase(std::remove(resultQueueComplete.begin(), resultQueueComplete.end(), result), 
			resultQueueComplete.end());
		delete result;
	}

	Result *Host::getAvailableResult(unsigned __int64 taskID)
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lock(resultsQueueCompleteMutex);

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

		if (hostAsClient)
		{
			CF_SAY("Host as client is set ON", Settings::LogLevels::Info);
		}
		else
		{
			CF_SAY("Host as client is set OFF", Settings::LogLevels::Info);
		}
		
	}

	bool Host::markTaskFinished(Result *result)
	{
		bool resultValid = false;

		//Check the HOST as a pseudo-client in case it processed the task directly rather than sending to a network client.
		std::unique_lock<std::mutex> lock(tasksAssignedAsClientMutex);
		for (auto &t : tasksAssignedAsClient)
		{
			if (t->getInitialTaskID() == result->getInitialTaskID() &&
				t->getTaskPartNumber() == result->getTaskPartNumber())
			{
				//Record the time this task was started.
				result->setHostTimeSent(t->getHostTimeSent());

				//Remove this task from the host-as-client.
				delete t;
				tasksAssignedAsClient.erase(std::remove(tasksAssignedAsClient.begin(),
					tasksAssignedAsClient.end(), t), tasksAssignedAsClient.end());
				resultValid = true;
				break;
			}
		}
		lock.unlock();

		//Check CLIENTS to see if one of them processed this task.
		if (!resultValid)
		{
			std::unique_lock<std::mutex> clientsLock(clientsMutex);
			for (auto &c : clients)
			{
				std::unique_lock<std::mutex> lock(c->taskMutex);
				for (auto &t : c->tasks)
				{
					if (t->getInitialTaskID() == result->getInitialTaskID() &&
						t->getTaskPartNumber() == result->getTaskPartNumber())
					{
						//Record the time this task was started.
						result->setHostTimeSent(t->getHostTimeSent());

						//Remove this task from the client.
						delete t;
						c->tasks.erase(std::remove(c->tasks.begin(), c->tasks.end(), t), c->tasks.end());
						resultValid = true;
						break;
					}
				}
				lock.unlock();
				if (resultValid) break;
			}
			clientsLock.unlock();
		}

		return resultValid;
	}

	inline int Host::getClientsCount()
	{
		std::unique_lock<std::mutex> lock(clientsMutex);

		int count = 0;

		//Count connected clients.
		for (auto &c : clients)
		{
			if (!c->remove) count++;
		}

		if (hostAsClient) count++;

		return count;
	}

	bool Host::checkAvailableResult(unsigned __int64 taskID)
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lock(resultsQueueCompleteMutex);

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
		try
		{

			while (hostAsClientTaskProcessThreadRun && !cf::ConsoleMessager::getInstance()->exceptionThrown)
			{

				if (localHostAsClientTaskQueue.size() > 0)
				{
					//Copy the local task queue.
					std::unique_lock<std::mutex> lock(localHostAsClientTaskQueueMutex);
					std::list<Task *> localHostAsClientTaskQueueCOPY = localHostAsClientTaskQueue;
					lock.unlock();

					for (auto &t : localHostAsClientTaskQueueCOPY)
					{
						CF_SAY("Processing task " + std::to_string(t->getInitialTaskID()) + " locally.", Settings::LogLevels::Info);

						//Split the task among available threads and run.
						//If there is only one thread, don't split the task and just use the original
						//task object pointer.
						std::vector<cf::Task *> tasks;
						Task *removeTask = t;
						if (MAX_THREADS > 1)
						{
							tasks = t->split(MAX_THREADS);
						}
						else
						{
							tasks = std::vector<cf::Task *>{ t };
						}

						//Remove the task from the local task parts queue.
						std::unique_lock<std::mutex> lock3(localHostAsClientTaskQueueMutex);
						localHostAsClientTaskQueue.erase(std::remove(localHostAsClientTaskQueue.begin(),
							localHostAsClientTaskQueue.end(), removeTask), localHostAsClientTaskQueue.end());
						lock3.unlock();

						std::vector<std::future<cf::Result *>> threads = std::vector<std::future<cf::Result *>>();

						//Start benchmark timer.
						auto start = std::chrono::steady_clock::now();

						for (auto &task : tasks)
						{
							threads.push_back(std::async(std::launch::async, [&task]() { return task->run(); }));
						}

						std::vector<cf::Result *> results;

						unsigned int threadID = 0;
						for (auto &thread : threads)
						{
							CF_SAY("Processing task locally on thread " + std::to_string(threadID++) + ".", Settings::LogLevels::Info);
							auto result = thread.get();
							results.push_back(result);
						}

						//Remove split subtasks from memory. 
						//If there was just one, then that means we used the original task object so don't
						//remove it from memory here.
						if (tasks.size() > 1)
						{
							for (auto &task : tasks)
							{
								delete task;
								task = nullptr;
							}
						}

						//Stop benchmark test clock.
						auto end = std::chrono::steady_clock::now();
						auto diff = end - start;

						CF_SAY("Local computation time: " + std::to_string(std::chrono::duration <double, std::milli>(diff).count()) + " ms.", Settings::LogLevels::Info);

						cf::Result *result;

						//Merge result objects if there was more than one in the resulting set.
						if (results.size() > 1)
						{
							if (resultConstructMap.size() == 0 || resultConstructMap.find(results.front()->getSubtype()) == resultConstructMap.end()) CF_THROW("Invalid results type.");
							result = resultConstructMap[results.front()->getSubtype()]();

							result->merge(results);
						}
						else
						{
							result = results.front();
						}

						//Clean up temporary results objects, but only if they were a set of more than one.
						//If there was just one result than we need to keep the single result object in memory
						//and just pass it to the completed results list.
						if (results.size() > 1)
						{
							for (auto &r : results)
							{
								delete r;
								r = nullptr;
							}
						}

						CF_SAY("Processing task locally - completed.", Settings::LogLevels::Info);

						//Work out which client, if any, owns the task this result came from.
						//If a client is found to own the task, remove the task from the client and delete it from memory.
						//If no clients own this task, ignore it.
						//The host is also checked in case it was running as a pseudo-client for this task.
						if (!markTaskFinished(result))
						{
							CF_THROW("Results processed locally are invalid. No owner found.");
						}

						//Place the result in the host result parts queue.
						std::unique_lock<std::mutex> lock2(resultsQueueIncompleteMutex);
						resultQueueIncomplete.push_back(result);
						lock2.unlock();

						//Scan the incomplete results queue for complete results sets and move them to the complete results queue.
						checkForCompleteResults();

					}

					busy = false;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		catch (...)
		{
			//Do nothing with exceptions in threads. Main thread will see the exception message via ConsoleMessager object.
		}
	}

	void Host::checkForCompleteResults()
	{
		//Aquire lock on result queues.
		std::unique_lock<std::mutex> lockResCopy(resultsQueueIncompleteMutex);
		std::list<Result *> resultQueueIncompleteCOPY = resultQueueIncomplete;
		lockResCopy.unlock();

		std::vector<Result *> set;
		std::vector<Result *> remove;
		for (auto &r1 : resultQueueIncompleteCOPY)
		{
			//If this result part is already in the removal list, then skip it.
			if (remove.size() > 0 && std::find(remove.begin(), remove.end(), r1) != remove.end()) continue;

			set.push_back(r1);

			for (auto &r2 : resultQueueIncompleteCOPY)
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
				cf::Result *rNew;
			
				//Merge if the results set is in multiple parts. 
				//Otherwise just copy the results set pointer.
				if (set.size() > 1)
				{
					if (resultConstructMap.size() == 0 || resultConstructMap.find(r1->getSubtype()) == resultConstructMap.end()) CF_THROW("Invalid results type.");
					rNew = resultConstructMap[r1->getSubtype()]();
					rNew->merge(set);

					//Transfer the task start time from the set to the new merged result.
					rNew->setHostTimeSent(set[0]->getHostTimeSent());
				}
				else
				{
					rNew = set.front();
				}

				//Record the finish time for this result.
				rNew->setHostTimeFinished(getTime());

				//Add the elapsed time to the benchmark tracker.
				addBenchmarkTime(rNew->getHostTimeFinished() - rNew->getHostTimeSent());

				//Store the completed result.
				std::unique_lock<std::mutex> lockComp(resultsQueueCompleteMutex);
				resultQueueComplete.push_back(rNew);
				lockComp.unlock();

				//Record these results for removal from the incomplete results set.
				remove.insert(remove.end(), set.begin(), set.end());

			}
			set.clear();
		}

		std::unique_lock<std::mutex> lockRemove(resultsQueueIncompleteMutex);
		for (auto &r : remove)
		{

			//Remove completed results from incomplete results set.
			resultQueueIncomplete.erase(std::remove(resultQueueIncomplete.begin(), resultQueueIncomplete.end(), r), resultQueueIncomplete.end());

			//If the result is part of a set, delete the result set part from memory 
			//as we will have merged it into a new results set previously.
			if (r->getCurrentTaskPartsTotal() > 1)
			{
				delete r;
				r = nullptr;
			}

		}
		lockRemove.unlock();

	}

	void Host::sendSubTasks()
	{

		//Do nothing if subtask queue is empty. 
		if (subTaskQueue.size() == 0) return;

		//Copy current subtask queue entries for processing.
		std::unique_lock<std::mutex> copyLock(subTaskQueueMutex);
		std::list<Task *> subTaskQueueCOPY = subTaskQueue;
		copyLock.unlock();

		std::list<Task *>::iterator it = subTaskQueueCOPY.begin();

		while (it != subTaskQueueCOPY.end())
		{

			Task *task = *it;

			//Can this task ONLY be sent to local node, and host-as-client is enabled?
			//Then send this task to the local host-as-client regardless of its busy status.
			//OR
			//If this task can be sent to any node type AND host-as-client is enabled AND host isn't busy.
			//OR
			//Task can only be sent to remote target node but none are connected AND host-as-client is enabled, 
			//so process it locally anyway.
			if ((hostAsClient && task->getNodeTargetType() == Task::NodeTargetTypes::Local)
				||
				(hostAsClient && !busy && task->getNodeTargetType() == Task::NodeTargetTypes::Any)
				||
				(hostAsClient && !busy && task->getNodeTargetType() == Task::NodeTargetTypes::Remote && getClientsCount() == 1)
				)
			{
				std::unique_lock<std::mutex> hostAsClientLock(localHostAsClientTaskQueueMutex);
				CF_SAY("Sending task to local client.", Settings::LogLevels::Info);

				busy = true;

				//Set a host-relative timestamp on the task so we can track how long it is taking.
				task->setHostTimeSent(getTime());
				//Assign the task to the host-as-client so we can track its progress.
				trackTask(task);
				localHostAsClientTaskQueue.push_back(task);
				hostAsClientLock.unlock();

				it++;
			}
			else
			{
				//Search for the next available client.
				ClientDetails *freeClient = nullptr;
				std::unique_lock<std::mutex> clientsLock(clientsMutex);
				for (auto &client : clients)
				{
					//Skip busy or removed clients.
					if (client->busy || client->remove) continue;
					freeClient = client;
				}
				clientsLock.unlock();

				if (freeClient != nullptr)
				{
					CF_SAY("Sending task to remote client.", Settings::LogLevels::Info);

					freeClient->busy = true;

					//Set a host-relative timestamp on the task so we can track how long it is taking.
					task->setHostTimeSent(getTime());

					//Assign the task to the client so we can track its progress.
					freeClient->trackTask(task);

					sender.sendTask(freeClient, task);

					it++;
				}

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}

		//Empty the subtask queue.
		subTaskQueue.clear();

		//Wait for sender threads to finish.
		sender.waitForComplete();

		std::unique_lock<std::mutex> lockRemove(subTaskQueueMutex);
		for (auto &r : subTaskQueueCOPY)
		{
			//Remove subtasks from queue that have been sent.
			subTaskQueue.erase(std::remove(subTaskQueue.begin(), subTaskQueue.end(), r), subTaskQueue.end());
		}
		lockRemove.unlock();

	}

	void Host::addBenchmarkTime(const sf::Time elapsed)
	{
		benchmarkTimes.push_back(elapsed);

		//Remove excess times stored in list.
		while ((int)benchmarkTimes.size() > (int)maxBenchmarkTimes)
		{
			benchmarkTimes.pop_front();
		}
	}

	sf::Time Host::getAverageBenchmarkTime()
	{
		sf::Time sum;

		for (auto &t : benchmarkTimes)
		{
			sum += t;
		}

		return sum / (float)benchmarkTimes.size();
	}

}