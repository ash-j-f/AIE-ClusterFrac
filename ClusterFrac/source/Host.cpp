#include "Host.h"

namespace cf
{
	Host::Host()
	{
		//Default port number.
		port = 5000;

		//Listening default status.
		listening = false;
	}
	
	Host::~Host()
	{
		//Clean up any remaining registered clients.
		for (auto &c : clients) delete c;
	}

	void Host::start()
	{
		//Initialise incoming connection listener.
		listening = true;
		listener.listen(port);
		//Add the listener to the selector.
		selector.add(listener);
	}

	void Host::listenThread()
	{
		//Endless loop that waits for new connections.
		while (listening)
		{
			//Make the selector wait for data on any socket.
			if (selector.wait())
			{
				//Test the listener.
				if (selector.isReady(listener))
				{
					//The listener is ready: there is a pending connection.
					ClientDetails *newClient = new ClientDetails();
					if (listener.accept(*newClient->socket) == sf::Socket::Done)
					{
						//Add the new client to the clients list.
						clients.push_back(newClient);
						//Add the new client to the selector so that we will
						//be notified when it sends something.
						selector.add(*newClient->socket);
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
	}

}