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

	/**
	* HostListener class. Manages the thread that listens for incoming connections and messages from clients.
	* This is an essential component of the Host class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL HostListener
	{

	public:

		/**
		* Constructor that takes a host object as a parameter.
		* @param newClient Pointer to the host object.
		*/
		HostListener(Host *newHost);

		/**
		* Default destructor.
		*/
		~HostListener();

		/**
		* Start the host listener thread.
		* @returns void.
		*/
		void start();

		/**
		* Stop the host listener thread.
		* @returns void.
		*/
		void stop();

	private:

		//Has the host listener thread been started?
		bool started;

		//The host object the host listener thread belongs to.
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

		/**
		* Listen for incoming connections and messages from clients.
		* To be used by a dedicated thread.
		* @returns void.
		*/
		void clientReceiveThread(ClientDetails *client, std::atomic<bool> *cFlag);
	};
}