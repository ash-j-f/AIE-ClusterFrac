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
			//CF_SAY("Listening.");

			//Make the selector wait for data on any socket.
			//A timeout of N seconds is set to avoid locking the thread indefinitely.
			if (selector.wait(sf::Time(sf::seconds(1.0f))))
			{
				//Test the listener.
				if (selector.isReady(listener))
				{
					//The listener is ready: there is a pending connection.
					ClientDetails *newClient = new ClientDetails(CF_ID->getNextClientID());
					//CF_SAY("listenThread incoming connection lock.");
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
					for (it = clients.begin(); it != clients.end(); it++)
					{
						ClientDetails *client = *it;

						//Skip deleted clients.
						if (client->remove) continue;

						//Attempt to get socket lock. Try again later if another process already has a lock on this socket.
						//CF_SAY("listenThread incoming client data try lock.");
						std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
						if (lock.owns_lock())
						{
							if (selector.isReady(*client->socket))
							{
								CF_SAY("Incoming client data. Launching receiver thread.");
								//Launch a thread to deal with this client data.
								lock.unlock();
								std::atomic<bool> *cFlag = new std::atomic<bool>();
								//Set completion flag to false by default.
								*cFlag = false;
								clientReceiveThreads.push_back(std::thread([this, client, cFlag] { clientReceiveThread(client, cFlag); }));
								clientReceiveThreadsFinishedFlags.push_back(cFlag);
							}
						}
					}
				}
			}

			//Remove completed client receive threads.
			//Loop backwards as we are removing elements.
			for (int i = (int)clientReceiveThreads.size() - 1; i >= 0; i--)
			{
				if (clientReceiveThreadsFinishedFlags[i])
				{
					clientReceiveThreads[i].join();
					clientReceiveThreads.erase(clientReceiveThreads.begin() + i);
					delete clientReceiveThreadsFinishedFlags[i];
					clientReceiveThreadsFinishedFlags.erase(clientReceiveThreadsFinishedFlags.begin() + i);
				}
			}

			//Erase dead clients from client list.
			std::vector<ClientDetails *>::iterator deadIt;
			for (deadIt = clients.begin(); deadIt != clients.end();)
			{
				ClientDetails *client = *deadIt;

				bool removedOne = false;
				if (client->remove)
				{
					//Check if any other process still using the client socket.
					//If so, then try again later.
					//CF_SAY("listenThread remove dead clients try lock.");
					std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
					if (lock.owns_lock())
					{
						//Remove the socket from the selector.
						selector.remove(*client->socket);

						//Immediately unlock as we are about to delete the object that contains the mutex.
						lock.unlock();

						delete client;
						deadIt = clients.erase(deadIt);
						removedOne = true;
					}
				}

				if (!removedOne) deadIt++;

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

	void Host::addResultToQueue(Result *result)
	{
		resultQueue.push_back(result);
		CF_SAY("Added result to queue.");
	}

	void Host::sendTaskThread(ClientDetails *client, Task *task)
	{
		bool done = false;
		while (!done)
		{
			//CF_SAY("sendTaskThread trying lock.");
			std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
			if (lock.owns_lock())
			{
				CF_SAY("Sending task to client " + std::to_string(client->getClientID()) + ".");

				//Send task to client.
				cf::WorkPacket packet(cf::WorkPacket::Flag::Task);
				task->serialize(packet);

				client->socket->send(packet);

				packet.clear();

				CF_SAY("Sending task finished for client " + std::to_string(client->getClientID()) + ".");

				done = true;
				lock.unlock();
			}
			//Abort after a timeout.
			//TODO
			//done = true;
		}
	}

	void Host::clientReceiveThread(ClientDetails *client, std::atomic<bool> *cFlag)
	{
		//CF_SAY("clientReceiveThread lock.");
		//Obtain lock on the client socket.
		std::unique_lock<std::mutex> lock(client->socketMutex);

		// The client has sent some data, we can receive it
		cf::WorkPacket *packet = new cf::WorkPacket();
		if ((*client->socket).receive(*packet) == sf::Socket::Done)
		{
			switch (packet->getFlag())
			{
			case cf::WorkPacket::Flag::None:
				CF_SAY("Received unknown packet from client.");
				break;
			case cf::WorkPacket::Flag::Result:
				CF_SAY("Received result packet from client.");
				break;
			default:
				throw "Invalid flag data in packet.";
			}

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
			//Mark client data for erasure.
			client->remove = true;
		}
		delete packet;

		//Signal to our parent thread that this thread has finished.
		*cFlag = true;
		lock.unlock();
	}

}