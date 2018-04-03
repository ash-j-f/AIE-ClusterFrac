#pragma once
#include <atomic>
#include "DllExport.h"
#include <SFML\Network.hpp>
#include "ConsoleMessager.hpp"

namespace cf
{
	//Forward declarations.
	class Client;

	class DLL ClientListener
	{

	public:
		ClientListener(Client *newClient);
		~ClientListener();

		void start();

		void stop();

	private:

		bool started;

		Client *client;

		//Should the listener thread be listening for connections?
		std::atomic<bool> listen;

		//Is the server listening for connections?
		std::atomic<bool> listening;

		//Connection listening thread.
		std::thread listenerThread;

		/**
		* Listen for incoming connections.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void listenThread();

	};
}