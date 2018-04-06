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


			//TODO
			//xxxx
			//DO TASK STATUS WATCHING STUFF HERE
		}

		watching = false;
		CF_SAY("Watcher thread ended.", Settings::LogLevels::Info);
	}

}