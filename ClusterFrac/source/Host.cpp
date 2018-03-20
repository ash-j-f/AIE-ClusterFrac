#include "Host.h"

namespace cf
{
	Host::Host()
	{
		//Default port number.
		port = 5000;

		//Listening default status.
		listen = false;
		listening = false;

		//Default starting client ID.
		nextClientID = 0;
	}
	
	Host::~Host()
	{
		//Set listening status to signal listener thread to shut down.
		listen = false;

		//Wait for the listening thread to shut down.
		if (listenerThread.joinable()) listenerThread.join();

		//Clean up any remaining registered clients.
		for (auto &c : clients) delete c;
	}

	void Host::start()
	{
		CF_SAY("Starting ClusterFrac HOST on port " << std::to_string(port) << ".");

		//Initialise incoming connection listener.
		listener.listen(port);
		//Add the listener to the selector.
		selector.add(listener);

		//Launch listener thread.
		listen = true;
		listenerThread = std::thread([this] { listenThread(); });
	}

	void Host::listenThread()
	{
		//If there is already a thread listening, abort.
		if (listening) return;

		//Register listening active.
		listening = true;

		CF_SAY("Listener thread started.");
		//Endless loop that waits for new connections.
		//Aborts if listening flag is set false.
		while (listen)
		{
			//Make the selector wait for data on any socket.
			if (selector.wait())
			{
				//Test the listener.
				if (selector.isReady(listener))
				{
					//The listener is ready: there is a pending connection.
					ClientDetails *newClient = new ClientDetails(getNextClientID());
					if (listener.accept(*newClient->socket) == sf::Socket::Done)
					{
						//Add the new client to the clients list.
						clients.push_back(newClient);
						//Add the new client to the selector so that we will
						//be notified when it sends something.
						selector.add(*newClient->socket);
						
						CF_SAY("Client ID " << std::to_string(newClient->getClientID()) << " connected from IP " << (*newClient->socket).getRemoteAddress().toString() << ".");
					}
					else
					{
						// Error, we won't get a new connection, delete the socket.
						delete newClient;
					}
				}
				else
				{
					// The listener socket is not ready, test all other sockets (the clients)
					for (auto &client: clients)
					{
						if (selector.isReady(*client->socket))
						{
							// The client has sent some data, we can receive it
							sf::Packet *packet = new sf::Packet();
							if ((*client->socket).receive(*packet) == sf::Socket::Done)
							{
								//DO STUFF HERE
							}
							delete packet;
						}
					}
				}
			}
		}

		listening = false;
		CF_SAY("Listener thread ended.");
	}

}