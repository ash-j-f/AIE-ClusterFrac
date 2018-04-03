#pragma once
#include <atomic>
#include "DllExport.h"
#include <SFML\Network.hpp>
#include "ConsoleMessager.hpp"

namespace cf
{
	//Forward declarations.
	class Client;

	class DLL ClientSender
	{

	public:
		ClientSender(Client *newClient);
		~ClientSender();

		void start();

		void stop();

	private:

		bool started;

		Client *client;

		//Should the sender thread be sending completed results?
		std::atomic<bool> send;

		//Is the client sending of completed results enabled?
		std::atomic<bool> sending;

		//Sending thread.
		std::thread senderThread;

		/**
		* Send completed results to the host.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void sendThread();

	};
}