#pragma once
#include <atomic>
#include <thread>
#include <mutex>
#include "DllExport.h"
#include <SFML\Network.hpp>
#include "ClientDetails.hpp"
#include "Task.h"
#include "ConsoleMessager.hpp"

namespace cf
{
	//Forward declatarions.
	class Host;

	class DLL Sender
	{

	public:
		Sender(Host *newHost);
		~Sender();

		void sendTask(ClientDetails *client, Task *task);

		void waitForComplete();

	private:

		Host *host;

		//Mutex to lock the sender object.
		std::mutex senderMutex;

		//Spawn a sender thread for each task.
		std::vector<std::thread> taskSendThreads;

		/**
		* Send task to a client.
		* To be used by a dedicated thread.
		* @param client The client to send the task to.
		* @param Task the task to send.
		* @returns void.
		*/
		void sendTaskThread(ClientDetails *client, Task *task);
	};
}