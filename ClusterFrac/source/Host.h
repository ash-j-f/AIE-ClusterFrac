#pragma once
#include "DllExport.h"
#include <vector>
#include <SFML\Network.hpp>

namespace cf
{
	class DLL Host
	{
	public:
		Host();
		~Host();

		void start();

	private:
		
		class ClientDetails
		{
		public:
			ClientDetails()
			{
				socket = new sf::TcpSocket();
			};
			~ClientDetails()
			{
				delete socket;
			};

			sf::TcpSocket *socket;
		};

		int port;

		//Connected client details.
		std::vector<ClientDetails> clients;

		//Socket selector for managing multiple connections.
		sf::SocketSelector selector;

		//Listener for incoming connections
		sf::TcpListener listener;
	};
}