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
		try
		{

			//If there is already a thread watching, abort.
			if (watching) return;

			//Register watcher active.
			watching = true;

			CF_SAY("Watcher thread started. Watching task status.", Settings::LogLevels::Info);
			//Endless loop that watches task status.
			//Aborts if watch flag is set false.
			while (watch && !cf::ConsoleMessager::getInstance()->exceptionThrown)
			{

				//Check client tasks for any that have taken too long.
				std::unique_lock<std::mutex> clientsLock(host->clientsMutex);
				for (auto &c : host->clients)
				{
					//Skip removed clients.
					if (c->remove) continue;

					std::unique_lock<std::mutex> taskLock(c->taskMutex);
					std::vector<Task *>::iterator it;
					for (auto &t : c->tasks)
					{
						if ((host->getTime() - t->getHostTimeSent()).asMilliseconds() > (sf::Int32)t->getMaxTaskTimeMilliseconds())
						{
							//Task has taken too long, abort.
							std::string s = "Client " + std::to_string(c->getClientID()) + " task " + std::to_string(t->getInitialTaskID()) + " timed out. Aborting.";
							CF_SAY(s, Settings::LogLevels::Error);
							CF_THROW(s);
						}
					}
					taskLock.unlock();
				}
				clientsLock.unlock();

				//Divide any pending tasks into the sub task queue.
				if (host->getTasksCount() > 0 && host->getClientsCount() > 0) host->divideTasksIntoSubTaskQueue();

				//Send pending subtasks waiting on the host to clients.
				if (host->subTaskQueue.size() > 0) host->sendSubTasks();

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			watching = false;
			CF_SAY("Watcher thread ended.", Settings::LogLevels::Info);
		}
		catch (...)
		{
			//Do nothing with exceptions in threads. Main thread will see the exception message via ConsoleMessager object.
		}
	}

}