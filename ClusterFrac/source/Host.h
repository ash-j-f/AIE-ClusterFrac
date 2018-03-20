#pragma once
#include <vector>
#include <atomic>
#include <thread>
#include <future>
#include <SFML\Network.hpp>
#include "DllExport.h"
#include "ConsoleMessage.h"

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
		* Listen for incoming connections.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void listenThread();

		/**
		* Get and reserve the next available client ID, and then increment the internal next client ID counter.
		* @returns The next available client ID.
		*/
		int getNextClientID() { return nextClientID++; }

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
				socket = new sf::TcpSocket();
				ID = 0;
			};

			/**
			* Constructor with client ID.
			* @param newID The client ID to use for this new client.
			*/
			ClientDetails(int newID)
			{
				socket = new sf::TcpSocket();
				ID = newID;
			};

			/**
			* Default destructor.
			*/
			~ClientDetails()
			{
				delete socket;
			};

			//Socket used to communicate with this client.
			sf::TcpSocket *socket;
		
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
	};
}