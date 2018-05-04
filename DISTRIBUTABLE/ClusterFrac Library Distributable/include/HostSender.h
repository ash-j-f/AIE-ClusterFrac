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

	/**
	* HostSender class. Manages the thread that sends messages to clients.
	* This is an essential component of the Host class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class HostSender
	{

	public:

		/**
		* Constructor that takes a host object as a parameter.
		* @param newClient Pointer to the host object.
		*/
		DLL HostSender(Host *newHost);

		/**
		* Default destructor.
		*/
		DLL ~HostSender();

		/**
		* Send a task to the selected client.
		* @param client The client to send the task to.
		* @param task The task to send to the client.
		* @returns void.
		*/
		DLL void sendTask(ClientDetails *client, Task *task);

		/**
		* Wait for all tasks being sent to clients to complete sending.
		* @returns void.
		*/
		DLL void waitForComplete();

	private:

		//The host object this host sender belongs to.
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
		void sendTaskThread(ClientDetails *client, Task *task) const;
	};
}