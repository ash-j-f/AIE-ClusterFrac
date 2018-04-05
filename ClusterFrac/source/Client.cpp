#include "Client.h"

namespace cf
{

	Client::Client()
	{
		//Use default max thread concurrency setting determined by the OS.
		setConcurrency(0);

		//Default port number
		port = 5000;

		//Default start state.
		started = false;

		//Default connection state.
		connected = false;

		//Set network socket to non blocking mode by default.
		socket.setBlocking(false);
	}

	Client::~Client()
	{

		if (ProcessTaskThreadRun)
		{
			//Shut down task processing.
			ProcessTaskThreadRun = false;

			//Wait for task processing thread to finish.
			if (TaskProcessingThread.joinable()) TaskProcessingThread.join();
		}

		//Stop the listener.
		listener.stop();

		//Stop the sender.
		sender.stop();

		//Shut down network connection if it is still connected.
		if (connected) disconnect();

		//Clean up any tasks and results in queues.
		//Build an unordered set of task/result pointers in case the same
		//pointer is recorded in two places due to an unusual shutdown.
		//Unordered set will ignore duplicate entries.

		std::unordered_set<Task *> removeTasks;
		for (auto &t : taskQueue) removeTasks.insert(t);
		for (auto &t : removeTasks) delete t;

		std::unordered_set<Result *> removeResults;
		for (auto &r : resultQueueComplete) removeResults.insert(r);
		for (auto &r : resultQueueIncomplete) removeResults.insert(r);
		for (auto &r : removeResults) delete r;

	}

	void Client::start()
	{
		//If host already started, do nothing.
		if (started)
		{
			CF_SAY("Client already started.", Settings::LogLevels::Error);
			return;
		}

		started = true;

		CF_SAY("Starting ClusterFrac CLIENT.", Settings::LogLevels::Info);

		listener.start();

		sender.start();

		ProcessTaskThreadRun = true;
		TaskProcessingThread = std::thread([this] { ProcessTaskThread(); });
		
	}

	void Client::setPort(int portNum)
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

	bool Client::connect()
	{
		if (connected)
		{
			CF_SAY("Cannot connect. Already connected to a host.", Settings::LogLevels::Error);
			return false;
		}

		if (!started)
		{
			CF_SAY("Cannot connect. Client not started.", Settings::LogLevels::Error);
			return false;
		}

		if (port <= 0 || port > 65535)
		{
			CF_SAY("Cannot connect. Invalid port number.", Settings::LogLevels::Error);
			return false;
		}

		if (ipAddress.toString() == "")
		{
			CF_SAY("Cannot connect. Invalid IP address.", Settings::LogLevels::Error);
			return false;
		}

		//Use socket blocking during a connection attempt.
		socket.setBlocking(true);
		CF_SAY("Trying to connect to host at " + ipAddress.toString() + " on port " + std::to_string(port) + ".", Settings::LogLevels::Info);
		if (socket.connect(ipAddress, port) == sf::Socket::Done)
		{
			CF_SAY("Connected to host.", Settings::LogLevels::Info);
			connected = true;
		}
		else
		{
			CF_SAY("Failed to connect to host.", Settings::LogLevels::Error);
			socket.disconnect();
			connected = false;
		}
		socket.setBlocking(false);

		return connected;
	}

	void Client::disconnect()
	{
		socket.disconnect();
		connected = false;
	}

	void Client::addTaskToQueue(Task *task)
	{
		std::unique_lock<std::mutex> lock(taskQueueMutex);
		taskQueue.push_back(task);
		CF_SAY("Added task " + std::to_string(task->getInitialTaskID()) + " to queue.", Settings::LogLevels::Info);
	}

	void Client::ProcessTaskThread()
	{
		while (ProcessTaskThreadRun)
		{
			//Copy the local task queue.
			std::unique_lock<std::mutex> lock(taskQueueMutex);
			std::list<Task *> taskQueueCOPY = taskQueue;
			lock.unlock();

			for (auto &t : taskQueueCOPY)
			{
				int taskID = t->getInitialTaskID();

				CF_SAY("Task " + std::to_string(taskID) + " - started.", Settings::LogLevels::Info);

				//Split the task among available threads and run.
				std::vector<Task *> tasks;
				Task *removeTask = t;
				if (MAX_THREADS > 1)
				{
					tasks = t->split(MAX_THREADS);
					
					//Remove the original task from memory.
					delete t;
					t = nullptr;
				}
				else
				{
					tasks = std::vector<Task *>{t};
					//Don't remove original task from memory here. We'll be using it passed on as a subtask.
					//IT will get cleaned up later as a subtask.
				}

				//Remove the task from the local task parts queue.
				std::unique_lock<std::mutex> lock3(taskQueueMutex);
				taskQueue.erase(std::remove(taskQueue.begin(), taskQueue.end(), removeTask), taskQueue.end());
				lock3.unlock();

				std::vector<std::future<Result *>> threads = std::vector<std::future<Result *>>();

				//Start benchmark timer.
				auto start = std::chrono::steady_clock::now();

				for (auto &task : tasks)
				{
					threads.push_back(std::async(std::launch::async, [&task]() { return task->run(); }));
				}

				std::vector<Result *> results;

				for (auto &thread : threads)
				{
					CF_SAY("Task " + std::to_string(taskID) + " - waiting for results and merging.", Settings::LogLevels::Info);
					auto result = thread.get();
					results.push_back(result);
				}

				for (auto &task : tasks)
				{
					delete task;
					task = nullptr;
				}

				//Stop benchmark test clock.
				auto end = std::chrono::steady_clock::now();
				auto diff = end - start;

				CF_SAY("Task " + std::to_string(taskID) + " time: " + std::to_string(std::chrono::duration <double, std::milli>(diff).count()) + " ms.", Settings::LogLevels::Info);

				Result *result; 
				
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

				CF_SAY("Task " + std::to_string(taskID) + " - completed.", Settings::LogLevels::Info);

				//Place the result in the host result parts queue.
				std::unique_lock<std::mutex> lock2(resultsQueueMutex);
				resultQueueIncomplete.push_back(result);
				lock2.unlock();

				//Scan the incomplete results queue for complete results sets and move them to the complete results queue.
				checkForCompleteResults();
			}
		}
	}

	void Client::checkForCompleteResults()
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
				Result *rNew;
				
				//Merge if the results set is in multiple parts. 
				//Otherwise just copy the results set pointer.
				if (set.size() > 1)
				{
					if (resultConstructMap.size() == 0 || resultConstructMap.find(r1->getSubtype()) == resultConstructMap.end()) CF_THROW("Invalid results type.");

					rNew = resultConstructMap[r1->getSubtype()]();

					rNew->merge(set);
				}
				else
				{
					rNew = set.front();
				}

				resultQueueComplete.push_back(rNew);

				//Record these results for removal from the incomplete results set.
				remove.insert(remove.end(), set.begin(), set.end());

			}
			set.clear();
		}

		for (auto &r : remove)
		{
			//If the result is part of a set, delete the result set part from memory 
			//as we will have merged it into a new results set previously.
			if (r->getCurrentTaskPartsTotal() > 1)
			{
				delete r;
				r = nullptr;
			}

			//Remove completed results from incomplete results set.
			resultQueueIncomplete.erase(std::remove(resultQueueIncomplete.begin(), resultQueueIncomplete.end(), r), resultQueueIncomplete.end());
		}
	}
}