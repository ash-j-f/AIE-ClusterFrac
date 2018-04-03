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

	class DLL HostListener
	{

	public:
		HostListener(Host *newHost);
		~HostListener();

		void start();

		void stop();

	private:

		bool started;

		Host *host;

		//Socket selector for managing multiple connections.
		sf::SocketSelector selector;

		//Listener for incoming connections
		sf::TcpListener tcpListener;

		//Should the listener thread be listening for connections?
		std::atomic<bool> listen;

		//Is the server listening for connections?
		std::atomic<bool> listening;

		//Connection listening thread.
		std::thread listenerThread;

		//Client data receive threads.
		std::vector<std::thread> clientReceiveThreads;

		//Indicators for when client data recieve threads have finished.
		std::vector<std::atomic<bool> *> clientReceiveThreadsFinishedFlags;

		/**
		* Listen for incoming connections.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void listenThread();

		void clientReceiveThread(ClientDetails *client, std::atomic<bool> *cFlag);
	};
}