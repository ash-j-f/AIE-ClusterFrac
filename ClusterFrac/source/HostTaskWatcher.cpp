#include "HostTaskWatcher.h"
#include "Host.h"

namespace cf
{
	HostTaskWatcher::HostTaskWatcher(Host *newHost)
	{
		host = newHost;

		//Listening default status.
		watch = false;
		watching = false;

		started = false;
	}

	HostTaskWatcher::~HostTaskWatcher()
	{
		stop();
	}

	void HostTaskWatcher::start()
	{
		//If watcher is already started, do nothing.
		if (started) return;

		started = true;

		//Launch watcher thread.
		watch = true;
		watcherThread = std::thread([this] { watchThread(); });
	}

	void HostTaskWatcher::stop()
	{
		//If already stopped, do nothing.
		if (!started) return;

		//Set watch status to signal watcher thread to shut down.
		watch = false;

		//Wait for the watcher thread to shut down.
		if (watcherThread.joinable()) watcherThread.join();

		started = false;
	}

	void HostTaskWatcher::watchThread()
	{
		//If there is already a thread watching, abort.
		if (watching) return;

		//Register watcher active.
		watching = true;

		CF_SAY("Watcher thread started. Watching task status.", Settings::LogLevels::Info);
		//Endless loop that watches task status.
		//Aborts if watch flag is set false.
		while (watch)
		{
			//CF_SAY("Watching.");

			//Check client tasks for any that have taken too long.
			std::vector<Task *> redistributeTasks;
			std::unique_lock<std::mutex> clientsLock(host->clientsMutex);
			for (auto &c : host->clients)
			{
				//Skip removed clients.
				if (c->remove) continue;

				std::unique_lock<std::mutex> taskLock(c->taskMutex);
				std::vector<Task *>::iterator it;
				for (it = c->tasks.begin(); it != c->tasks.end();)
				{
					Task *t = *it;
					if ((host->getTime() - t->getHostTimeSent()).asMilliseconds() > (sf::Int32)t->getMaxTaskTimeMilliseconds())
					{
						//Task has taken too long, add it to the list of tasks to distribute to other clients.
						CF_SAY("Client " + std::to_string(c->getClientID()) + " task " + std::to_string(t->getInitialTaskID()) + " timed out. Redistributing.", Settings::LogLevels::Info);
						redistributeTasks.push_back(t);
						//Remove this task from the client's own assigned task list.
						it = c->tasks.erase(it);
					}
					else
					{
						it++;
					}
				}
				taskLock.unlock();
			}
			clientsLock.unlock();

			//Distribute any now unowned tasks to other clients.
			if (redistributeTasks.size() > 0)
			{
				std::unique_lock<std::mutex> lock3(host->subTaskQueueMutex);
				host->subTaskQueue.insert(host->subTaskQueue.end(), redistributeTasks.begin(), redistributeTasks.end());
				lock3.unlock();
			}

			//Send any pending tasks waiting on the host. 
			if (host->getTasksCount() > 0 && host->getClientsCount() > 0) host->sendTasks();

			//Send any pending subtasks waiting on the host.
			host->distributeSubTasks();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		watching = false;
		CF_SAY("Watcher thread ended.", Settings::LogLevels::Info);
	}

}