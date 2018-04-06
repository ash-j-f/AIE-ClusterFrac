#pragma once
#include <atomic>
#include "DllExport.h"
#include <SFML\Network.hpp>
#include "ClientDetails.hpp"
#include "ConsoleMessager.hpp"

namespace cf
{
	//Forward declarations.
	class Host;

	class DLL HostTaskWatcher
	{

	public:
		HostTaskWatcher(Host *newHost);
		~HostTaskWatcher();

		void start();

		void stop();

	private:

		bool started;

		Host *host;

		//Should the watcher thread be watching tasks?
		std::atomic<bool> watch;

		//Is the host watching task status?
		std::atomic<bool> watching;

		//Tash watcher thread.
		std::thread watcherThread;

		/**
		* Watch task status.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void watchThread();

	};
}