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
		CF_SAY("Starting ClusterFrac HOST on port " + std::to_string(port) + ".");

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
			if (selector.wait(sf::Time(sf::seconds(1.0f))))
			{
				//Test the listener.
				if (selector.isReady(listener))
				{
					//The listener is ready: there is a pending connection.
					ClientDetails *newClient = new ClientDetails(getNextClientID());
					std::unique_lock<std::mutex> lock(newClient->socketMutex);
					if (listener.accept(*newClient->socket) == sf::Socket::Done)
					{
						//Add the new client to the clients list.
						clients.push_back(newClient);
						//Add the new client to the selector so that we will
						//be notified when it sends something.
						selector.add(*newClient->socket);
						
						CF_SAY("Client ID " + std::to_string(newClient->getClientID()) + " from IP " 
							+ (*newClient->socket).getRemoteAddress().toString() + " connected.");
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
						//Attempt to get socket lock. Try again later if another process already has a lock on this socket.
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
									//{
										//If packet was a finished work task, mark the client not busy.
										//TODO
										//client->busy = false;
									//}

								}
								else if ((*client->socket).receive(*packet) == sf::Socket::Disconnected)
								{
									CF_SAY("Client ID " + std::to_string(client->getClientID()) + " from IP "
										+ (*client->socket).getRemoteAddress().toString() + " disconnected.");
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
		CF_SAY("Added task to queue.");
	}

	bool Host::sendTasks()
	{
		if (getClientsCount() < 1)
		{
			CF_SAY("Cannot send tasks. No clients connected.");
			return false;
		}

		//Divide tasks among clients.
		std::vector<Task *> subTaskQueue;
		std::vector<Task *> dividedTasks;
		CF_SAY("Diving tasks among clients.");
		for (auto &task : taskQueue)
		{
			dividedTasks = task->split(getClientsCount());
			subTaskQueue.insert(subTaskQueue.end(), dividedTasks.begin(), dividedTasks.end());
		}

		CF_SAY("Sending tasks to clients.");
		//Spawn a sender thread for each task.
		std::vector<std::thread> taskSendThreads;
		//Distribute tasks among available clients.
		std::vector<Task *>::iterator it = subTaskQueue.begin();
		while (it != subTaskQueue.end())
		{
			//Search for the next available client.
			ClientDetails *freeClient = nullptr;
			for (auto &client : clients)
			{
				//Skip busy clients.
				if (client->busy) continue;
				freeClient = client;
			}

			if (freeClient != nullptr)
			{
				freeClient->busy = true;
				Task *task = *it;
				it++;
				taskSendThreads.push_back(std::thread([this, freeClient, task]() { sendTaskThread(freeClient, task); }));
				CF_SAY("Task send thread started for client " + std::to_string(freeClient->getClientID()) + ".");
			}
			
		}

		//Empty the task queue.
		taskQueue.clear();

		//Wait for threads to finish.
		for (auto &thread : taskSendThreads)
		{
			thread.join();
		}

		//Destroy the subtask copies and empty the subtask list.
		for (auto &task : subTaskQueue) delete task;
		subTaskQueue.clear();

		CF_SAY("Task sending finished.");

		return true;
	}

	void Host::sendTaskThread(ClientDetails *client, Task *task)
	{
		bool done = false;
		while (!done)
		{
			std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
			if (lock.owns_lock())
			{
				CF_SAY("Sending task to client " + std::to_string(client->getClientID()) + ".");

				//Send task to client.
				cf::WorkPacket packet;
				task->serialize(packet);

				client->socket->send(packet);

				packet.clear();

				CF_SAY("Sending task finished for client " + std::to_string(client->getClientID()) + ".");

				done = true;
			}

			//Abort after a timeout.
			//TODO
			//done = true;
		}
	}

}