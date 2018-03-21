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

		CF_SAY("Listener thread started. Waiting for clients to connect.");
		//Endless loop that waits for new connections.
		//Aborts if listening flag is set false.
		while (listen)
		{
			//Make the selector wait for data on any socket.
			//A timeout of N seconds is set to avoid locking the thread indefinitely.
			if (selector.wait(sf::Time(sf::seconds(5))))
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
						
						CF_SAY("Client ID " << std::to_string(newClient->getClientID()) << " from IP " 
							<< (*newClient->socket).getRemoteAddress().toString() << " connected.");
					}
					else
					{
						// Error, we won't get a new connection, delete the socket.
						delete newClient;
					}
				}
				else
				{
					//The listener socket is not ready, test all other sockets (the clients)
					std::vector<ClientDetails *>::iterator it;
					//Using erase-or-increment method here, so increment iterator at END of loop.
					for (it = clients.begin(); it != clients.end();)
					{
						bool erasedOne = false;
						
						ClientDetails *client = *it;
						std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
						if (lock.owns_lock())
						{
							if (selector.isReady(*client->socket))
							{
								// The client has sent some data, we can receive it
								sf::Packet *packet = new sf::Packet();
								if ((*client->socket).receive(*packet) == sf::Socket::Done)
								{
									//Check incoming packet type.
									//TODO

									//Perform action based on packet type.
									//TODO
								}
								else if ((*client->socket).receive(*packet) == sf::Socket::Disconnected)
								{
									CF_SAY("Client ID " << std::to_string(client->getClientID()) << " from IP "
										<< (*client->socket).getRemoteAddress().toString() << " disconnected.");
									selector.remove(*client->socket);
									(*client->socket).disconnect();
									delete client;
									//Erase the client, and increment the client iterator to the next client.
									it = clients.erase(it);
									erasedOne = true;
								}
								delete packet;
							}
						}
						//Increment client iterator only if an erase wasn't called.
						if (!erasedOne) it++;
					}
				}
			}
		}

		listening = false;
		CF_SAY("Listener thread ended.");
	}

	void Host::addTaskToQueue(Task *task)
	{
		taskQueue.push_back(task);
	}

	void Host::sendTasks()
	{
		//Spawn a sender thread for each client.
		//TODO

		//Empty the task queue.
		taskQueue.clear();
	}

	void Host::sendTaskThread(ClientDetails *client, Task *task)
	{
		std::unique_lock<std::mutex> lock(client->socketMutex);
		if (lock.owns_lock())
		{
			//Send task to client.
			//TODO 
		}
	}

}