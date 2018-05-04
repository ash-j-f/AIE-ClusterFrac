#pragma once
#include <atomic>
#include <mutex>
#include <SFML\Network.hpp>
#include "Task.h"
#include "DllExport.h"

namespace cf
{

	/**
	* Client details class. Tracks client ID, socket pointer and other essential 
	* information about the connected remote client.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class ClientDetails
	{

	public:

		/**
		* Constructor with client ID.
		* @param newID The client ID to use for this new client.
		*/
		DLL ClientDetails(unsigned __int64 newID)
		{
			ID = newID;
			init();
		};

		/**
		* Default destructor.
		*/
		DLL ~ClientDetails()
		{
			//Delete from memory any tasks still assigned to this client.
			std::unique_lock<std::mutex> lock(taskMutex);
			for (auto &t : tasks)
			{
				delete t;
				t = nullptr;
			}
			tasks.clear();
			lock.unlock();

			delete socket;
			socket = nullptr;
		};

		/**
		* Standard class initialisation.
		* @returns void.
		*/
		DLL void init()
		{
			socket = new sf::TcpSocket();
			socket->setBlocking(false);
			busy = false;
			remove = false;
		};

		//Socket used to communicate with this client.
		sf::TcpSocket *socket;

		//Socket mutex for this client, for locking the socket during use.
		std::mutex socketMutex;

		//Is this client busy with a task?
		std::atomic<bool> busy;

		//Should this client be removed?
		std::atomic<bool> remove;

		//Tasks assigned to this client.
		std::vector<Task *> tasks;

		//Task tracking list mutex.
		std::mutex taskMutex;

		/**
		* Assign a task to this client so that its progress can be tracked.
		* @returns void.
		*/
		DLL inline void trackTask(Task* t) { std::unique_lock<std::mutex> lock(taskMutex); tasks.push_back(t); };

		/**
		* Get client ID.
		* @returns The client's ID.
		*/
		DLL inline unsigned __int64 getClientID() const { return ID; }

	private:

		//Unique client ID on this host.
		unsigned __int64 ID;
	};
}