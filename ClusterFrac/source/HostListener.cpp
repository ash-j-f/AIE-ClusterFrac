#include "HostListener.h"
#include "Host.h"

namespace cf
{
	HostListener::HostListener(Host *newHost)
	{
		host = newHost;

		//Listening default status.
		listen = false;
		listening = false;

		started = false;
	}

	HostListener::~HostListener()
	{
		stop();
	}

	void HostListener::start()
	{
		//If listener is already started, do nothing.
		if (started) return;

		started = true;

		//Initialise incoming connection listener.
		tcpListener.listen(host->port);
		tcpListener.setBlocking(false);
		//Add the listener to the selector.
		selector.add(tcpListener);

		//Launch listener thread.
		listen = true;
		listenerThread = std::thread([this] { listenThread(); });
	}

	void HostListener::stop()
	{
		//If already stopped, do nothing.
		if (!started) return;

		//Set listening status to signal listener thread to shut down.
		listen = false;

		//Wait for the listening thread to shut down.
		if (listenerThread.joinable()) listenerThread.join();

		started = false;
	}

	void HostListener::listenThread()
	{
		//If there is already a thread listening, abort.
		if (listening) return;

		//Register listening active.
		listening = true;

		CF_SAY("Listener thread started. Waiting for clients to connect.", Settings::LogLevels::Info);
		//Endless loop that waits for new connections.
		//Aborts if listening flag is set false.
		while (listen)
		{
			//CF_SAY("Listening.");

			//Make the selector wait for data on any socket.
			//A timeout is set to avoid locking the thread indefinitely.
			if (selector.wait(sf::Time(sf::milliseconds(1))))
			{
				//Test the listener.
				if (selector.isReady(tcpListener))
				{
					CF_SAY("New incoming connection started.", Settings::LogLevels::Debug);
					//The listener is ready: there is a pending connection.
					ClientDetails *newClient = new ClientDetails(CF_ID->getNextClientID());
					//CF_SAY("listenThread incoming connection lock.");
					std::unique_lock<std::mutex> lock(newClient->socketMutex);
					if (tcpListener.accept(*newClient->socket) == sf::Socket::Done)
					{
						//Add the new client to the clients list.
						std::unique_lock<std::mutex> clientsLock(host->clientsMutex);
						host->clients.push_back(newClient);
						clientsLock.unlock();

						//Add the new client to the selector so that we will
						//be notified when it sends something.
						selector.add(*newClient->socket);

						CF_SAY("Client ID " + std::to_string(newClient->getClientID()) + " from IP "
							+ (*newClient->socket).getRemoteAddress().toString() + " connected.", Settings::LogLevels::Info);
					}
					else
					{
						CF_SAY("Incoming connection failed.", Settings::LogLevels::Debug);
						// Error, we won't get a new connection, delete the socket.
						delete newClient;
						newClient = nullptr;
					}
				}
				else
				{
					//The listener socket is not ready, test all other sockets (the clients)
					std::unique_lock<std::mutex> clientsLock(host->clientsMutex);
					for (auto &client : host->clients)
					{

						//Skip deleted clients.
						if (client->remove) continue;

						//Attempt to get socket lock. Try again later if another process already has a lock on this socket.
						//CF_SAY("listenThread incoming client data try lock.");
						std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
						if (lock.owns_lock())
						{
							if (selector.isReady(*client->socket))
							{
								CF_SAY("Incoming client data. Launching receiver thread.", Settings::LogLevels::Debug);
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
					clientsLock.unlock();
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
					clientReceiveThreadsFinishedFlags[i] = nullptr;
					clientReceiveThreadsFinishedFlags.erase(clientReceiveThreadsFinishedFlags.begin() + i);
				}
			}

			//Erase dead clients from client list.
			std::vector<ClientDetails *>::iterator deadIt;
			std::unique_lock<std::mutex> clientsLock(host->clientsMutex);
			for (deadIt = host->clients.begin(); deadIt != host->clients.end();)
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
						client = nullptr;
						deadIt = host->clients.erase(deadIt);
						removedOne = true;
					}
				}

				if (!removedOne) deadIt++;

			}
			clientsLock.unlock();
		}

		listening = false;
		CF_SAY("Listener thread ended.", Settings::LogLevels::Info);
	}

	void HostListener::clientReceiveThread(ClientDetails *client, std::atomic<bool> *cFlag)
	{
		//CF_SAY("clientReceiveThread lock.");

		//Obtain lock on the client socket.
		std::unique_lock<std::mutex> lock(client->socketMutex);

		// The client has sent some data, we can receive it
		cf::WorkPacket *packet = new cf::WorkPacket();

		sf::Socket::Status status;

		//Loop continuously until a useful status is returned from the socket.
		//We have to do this as the socket is in non blocking mode, and may require more than one 
		//status check before all data is received.
		while (true)
		{
			//Get socket status
			status = (*client->socket).receive(*packet);

			if (status == sf::Socket::Status::Done)
			{
				if (packet->getFlag() == cf::WorkPacket::Flag::None)
				{
					CF_SAY("Received unknown packet from client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Error);
				}
				else if (packet->getFlag() == cf::WorkPacket::Flag::Result)
				{

					CF_SAY("Received result packet from client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Info);

					std::string type;
					std::string subType;

					*packet >> type;
					*packet >> subType;

					if (type != "Result")
					{
						std::string s = "Received unknown packet from client " + std::to_string(client->getClientID()) + ".";
						CF_SAY(s, Settings::LogLevels::Error);
						CF_THROW(s);
					}

					//Check subtype exists in the constuction map.
					if (host->resultConstructMap.size() == 0 || host->resultConstructMap.find(subType) == host->resultConstructMap.end()) {
						std::string s = "Unknown subtype in packet from client " + std::to_string(client->getClientID()) + ".";
						CF_SAY(s, Settings::LogLevels::Error);
						CF_THROW(s);
					}

					//Instantiate the resulting derived class.
					cf::Result *result = host->resultConstructMap[subType]();

					result->deserialize(*packet);

					//Work out which client, if any, owns the task this result came from.
					//If a client is found to own the task, remove the task from the client and delete it from memory.
					//If no clients own this task, ignore it.
					//The host is also checked in case it was running as a pseudo-client for this task.
					if (host->markTaskFinished(result))
					{
						CF_SAY("Result packet from client " + std::to_string(client->getClientID()) + " is valid.", Settings::LogLevels::Info);

						//Add result data to the host incomplete results queue.
						std::unique_lock<std::mutex> lock3(host->resultsQueueMutex);
						host->resultQueueIncomplete.push_back(result);
						lock3.unlock();
					}
					else
					{
						CF_SAY("Result packet from client " + std::to_string(client->getClientID()) + " is INVALID. Rejecting.", Settings::LogLevels::Info);
						delete result;
						result = nullptr;
					}

					//Scan the incomplete results queue for complete results sets and move them to the complete results queue.
					host->checkForCompleteResults();

					client->busy = false;

				}
				else
				{
					CF_THROW("Invalid flag data in packet from client " + std::to_string(client->getClientID()) + ".");
				}

				break;

			}
			else if (status == sf::Socket::Status::Disconnected)
			{
				CF_SAY("Client ID " + std::to_string(client->getClientID()) + " from IP "
					+ (*client->socket).getRemoteAddress().toString() + " disconnected.", Settings::LogLevels::Info);
				selector.remove(*client->socket);
				
				//Disconnect the client.
				client->socket->disconnect();

				//Distribute this client's tasks to other available clients.
				std::unique_lock<std::mutex> lock2(client->taskMutex);
				if (client->tasks.size() > 0)
				{
					CF_SAY("Client ID " + std::to_string(client->getClientID()) + " disconnected with unfinished tasks. Redistributing.", Settings::LogLevels::Info);
					host->distributeSubTasks(client->tasks);
				}
				client->tasks.clear();
				lock2.unlock();

				//Mark client data for erasure.
				client->remove = true;

				break;
			}
			else if (status == sf::Socket::Status::Partial)
			{
				//Partial packet, so continue looping.
			}
			else if (status == sf::Socket::Status::NotReady)
			{
				//Socket not ready, so continue looping.
			}
			else
			{
				//Invalid data from client.
				CF_SAY("Invalid data from client. Ignoring.", Settings::LogLevels::Error);
				break;
			}
		}

		delete packet;
		packet = nullptr;

		//Signal to our parent thread that this thread has finished.
		*cFlag = true;
		lock.unlock();
	}
}