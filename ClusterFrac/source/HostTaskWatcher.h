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

	/**
	* HostTaskWatcher class. Watch the task list for new tasks to process,
	* and for tasks that have passed their expiry time and need to be sent 
	* to a new client.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL HostTaskWatcher
	{

	public:
		
		/**
		* Constructor that takes a host object as a prameter.
		* @param newHost The host object this host task watcher belongs to.
		*/
		HostTaskWatcher(Host *newHost);

		/**
		* Default destructor.
		*/
		~HostTaskWatcher();

		/**
		* Start the host task watcher.
		* @return void.
		*/
		void start();

		/**
		* Stop the host task watcher.
		* @return void.
		*/
		void stop();

	private:

		//Has the host task watcher been started?
		bool started;

		//The host object this host task watcher belongs to.
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