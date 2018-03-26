#pragma once
#include <atomic>
#include <mutex>
#include <SFML\Network.hpp>

namespace cf
{

	/**
	* Client details class. Tracks client ID, socket pointer and other essential 
	* information about the connected remote client.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class ClientDetails
	{

	public:

		/**
		* Constructor with client ID.
		* @param newID The client ID to use for this new client.
		*/
		ClientDetails(unsigned int newID)
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
			socket->setBlocking(false);
			busy = false;
			remove = false;
		};

		//Socket used to communicate with this client.
		sf::TcpSocket *socket;

		//Socket mutex for this client, for locking the socket during use.
		std::mutex socketMutex;

		//Is this client busy with a task?
		std::atomic<bool> busy;

		//Should this client be removed?
		std::atomic<bool> remove;

		/**
		* Get client ID.
		* @returns The client's ID.
		*/
		unsigned int getClientID() { return ID; }

	private:

		//Unique client ID on this host.
		unsigned int ID;
	};
}