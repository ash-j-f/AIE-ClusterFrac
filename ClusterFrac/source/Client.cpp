#include "Client.h"

namespace cf
{

	Client::Client()
	{
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

		//Launch internal loop thread.
		loopThreadRun = true;
		loopingThread = std::thread([this] { loopThread(); });

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
			throw s;
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

	void Client::addTaskToQueue(Task *task)
	{
		std::unique_lock<std::mutex> lock(taskQueueMutex);
		taskQueue.push_back(task);
		CF_SAY("Added task " + std::to_string(task->getInitialTaskID()) + " to queue.", Settings::LogLevels::Info);
	}

	void Client::loopThread()
	{
		while (loopThreadRun)
		{
			//Perform repeated tasks here...

			//Sleep before running loop again.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
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
				CF_SAY("Task " + std::to_string(t->getInitialTaskID()) + " - started.", Settings::LogLevels::Info);

				//Split the task among available threads and run.
				std::vector<Task *> tasks = t->split(std::thread::hardware_concurrency());

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
					CF_SAY("Task " + std::to_string(t->getInitialTaskID()) + " - waiting for results and merging.", Settings::LogLevels::Info);
					auto result = thread.get();
					results.push_back(result);
				}

				//Stop benchmark test clock.
				auto end = std::chrono::steady_clock::now();
				auto diff = end - start;

				CF_SAY("Task " + std::to_string(t->getInitialTaskID()) + " time: " + std::to_string(std::chrono::duration <double, std::milli>(diff).count()) + " ms.", Settings::LogLevels::Info);

				Result *result = resultConstructMap[results.front()->getSubtype()]();
				result->merge(results);

				//Clean up temporary results objects.
				for (auto &r : results) delete r;

				CF_SAY("Task " + std::to_string(t->getInitialTaskID()) + " - completed.", Settings::LogLevels::Info);

				//Place the result in the host result parts queue.
				std::unique_lock<std::mutex> lock2(resultsQueueMutex);
				resultQueueIncomplete.push_back(result);
				lock2.unlock();

				//Scan the incomplete results queue for complete results sets and move them to the complete results queue.
				checkForCompleteResults();

				std::unique_lock<std::mutex> lock3(taskQueueMutex);
				//Remove the task from memory.
				delete t;
				//Remove the task from the local task parts queue.
				taskQueue.erase(std::remove(taskQueue.begin(), taskQueue.end(), t), taskQueue.end());
				lock3.unlock();
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
				Result *rNew = resultConstructMap[r1->getSubtype()]();

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