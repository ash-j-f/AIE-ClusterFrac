#pragma once
#include <vector>
#include <list>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>
#include <SFML\Network.hpp>
#include "DllExport.h"
#include "ConsoleMessage.h"
#include "Task.h"

namespace cf
{
	/**
	* Host class. The host ClusterFrac server which manages connections to clients,
	* and sends/receives work packets.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL Host
	{
	public:

		/**
		* Default constructor.
		*/
		Host();

		/**
		* Default destructor.
		*/
		~Host();

		/**
		* Start a host server.
		* @returns void.
		*/
		void start();

		/**
		* Set the port number for the host server.
		* @param portNum The port number to use. Must be in the range [1 .. 65535].
		* @returns void.
		*/
		inline void setPort(int portNum) { if (portNum > 0 && portNum <= 65535) { port = portNum; } else { throw "Invalid port number.";  }; };

		/**
		* Get and reserve the next available client ID, and then increment the internal next client ID counter.
		* @returns The next available client ID.
		*/
		inline int getNextClientID() { return nextClientID++; }

		/**
		* Add a task to the task queue for sending to clients.
		* @param task The task to add.
		* @returns void.
		*/
		void addTaskToQueue(Task *task);

		/**
		* Send tasks to clients for processing.
		* @returns void.
		*/
		void sendTasks();

		/**
		* Get a count of all connected clients.
		* @returns The number of connected clients.
		*/
		inline int getClientsCount() const { return (int)clients.size(); };

	private:
		
		////////////////////////////////////////////////////
		// ClientDetails class.
		////////////////////////////////////////////////////
		/**
		* Client details class.
		* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
		*/
		class ClientDetails
		{
		public:
			
			/**
			* Default constructor.
			*/
			ClientDetails()
			{
				ID = 0;
				init();
			};

			/**
			* Constructor with client ID.
			* @param newID The client ID to use for this new client.
			*/
			ClientDetails(int newID)
			{
				ID = newID;
				init();
			};

			/**
			* Default destructor.
			*/
			~ClientDetails()
			{
				delete socket;
			};

			/**
			* Standard class initialisation.
			* @returns void.
			*/
			void init()
			{
				socket = new sf::TcpSocket();
				busy = false;
			};

			//Socket used to communicate with this client.
			sf::TcpSocket *socket;
		
			//Socket mutex for this client, for locking the socket during use.
			std::mutex socketMutex;

			//Is this client busy with a task?
			std::atomic<bool> busy;

			/**
			* Get client ID.
			* @returns The client's ID.
			*/
			int getClientID() { return ID; }

		private:

			//Unique client ID on this host.
			int ID;
		};
		////////////////////////////////////////////////////
		// End ClientDetails class.
		////////////////////////////////////////////////////

		//Port number this server is using.
		int port;

		//Connected client details.
		std::vector<ClientDetails *> clients;

		//Socket selector for managing multiple connections.
		sf::SocketSelector selector;

		//Listener for incoming connections
		sf::TcpListener listener;

		//Should the listener thread be listening for connections?
		std::atomic<bool> listen;

		//Is the server listening for connections?
		std::atomic<bool> listening;

		//Connection listening thread.
		std::thread listenerThread;

		//Next client ID to use.
		int nextClientID;

		//Task queue.
		std::list<cf::Task *> taskQueue;

		/**
		* Listen for incoming connections.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void listenThread();

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