#pragma once
#include "DllExport.h"
#include <vector>
#include <SFML\Network.hpp>

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
		};
		////////////////////////////////////////////////////
		// End ClientDetails class.
		////////////////////////////////////////////////////

		//Port number this server is using.
		int port;

		//Connected client details.
		std::vector<ClientDetails> clients;

		//Socket selector for managing multiple connections.
		sf::SocketSelector selector;

		//Listener for incoming connections
		sf::TcpListener listener;
	};
}