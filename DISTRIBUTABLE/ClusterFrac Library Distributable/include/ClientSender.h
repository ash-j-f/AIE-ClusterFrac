#pragma once
#include <atomic>
#include "DllExport.h"
#include <SFML\Network.hpp>
#include "ConsoleMessager.hpp"

namespace cf
{
	//Forward declarations.
	class Client;

	/**
	* ClientSender class. Manages the thread that sends messages to the host.
	* This is an essential component of the Client class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class ClientSender
	{

	public:

		/**
		* Constructor that takes a client object as a parameter.
		* @param newClient Pointer to the client object.
		*/
		DLL ClientSender(Client *newClient);

		/**
		* Default destructor.
		*/
		DLL ~ClientSender();

		/**
		* Start the client sender thread.
		* @returns void.
		*/
		DLL void start();

		/**
		* Stop the client sender thread.
		* @returns void.
		*/
		DLL void stop();

	private:
		
		//Has the client sender thread been started?
		bool started;

		//The client object the client sender thread belongs to.
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