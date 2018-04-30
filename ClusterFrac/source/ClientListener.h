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
	* ClientListener class. Manages the thread that listens for incoming messages from the host.
	* This is an essential component of the Client class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL ClientListener
	{

	public:

		/**
		* Constructor that takes a client object as a parameter.
		* @param newClient Pointer to the client object.
		*/
		ClientListener(Client *newClient);

		/**
		* Default destructor.
		*/
		~ClientListener();

		/**
		* Start the client listener thread.
		* @returns void.
		*/
		void start();

		/**
		* Stop the client listener thread.
		* @returns void.
		*/
		void stop();

	private:

		//Has the client listener thread been started?
		bool started;

		//The client object the client listener thread belongs to.
		Client *client;

		//Should the listener thread be listening for connections?
		std::atomic<bool> listen;

		//Is the server listening for connections?
		std::atomic<bool> listening;

		//Connection listening thread.
		std::thread listenerThread;

		/**
		* Listen for incoming connections and messages.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void listenThread();

	};
}