#include "ClientListener.h"
#include "Client.h"

namespace cf
{
	ClientListener::ClientListener(Client *newClient)
	{
		client = newClient;

		//Listening default status.
		listen = false;
		listening = false;

		started = false;
	}

	ClientListener::~ClientListener()
	{
		stop();
	}

	void ClientListener::start()
	{
		//If listener is already started, do nothing.
		if (started) return;

		started = true;

		//Launch listener thread.
		listen = true;
		listenerThread = std::thread([this] { listenThread(); });
	}

	void ClientListener::stop()
	{
		//If already stopped, do nothing.
		if (!started) return;

		//Set listening status to signal listener thread to shut down.
		listen = false;

		//Wait for the listening thread to shut down.
		if (listenerThread.joinable()) listenerThread.join();

		started = false;
	}

	void ClientListener::listenThread()
	{
		try
		{

			//If there is already a thread listening, abort.
			if (listening) return;

			//Register listening active.
			listening = true;

			sf::Socket::Status status;

			cf::WorkPacket packet;

			//Enable compression if requested.
			packet.setCompression(client->compression);

			CF_SAY("Listener thread started. Waiting for data from host.", Settings::LogLevels::Info);
			//Endless loop that waits for new connections.
			//Aborts if listening flag is set false.
			while (listen && !cf::ConsoleMessager::getInstance()->exceptionThrown)
			{
				//Skip checking socket if we're not connected to a host.
				if (client->connected)
				{

					//CF_SAY("Listening.");

					//Attempt to get socket lock. Try again later if another process already has a lock on this socket.
					//CF_SAY("listenThread incoming client data try lock.");
					std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
					if (lock.owns_lock())
					{

						//Get socket status
						status = (client->socket).receive(packet);
						lock.unlock();

						if (status == sf::Socket::Status::Done && packet.getDataSize() == 0)
						{
							//Empty packet, so do nothing.
							packet.clear();
						}
						else if (status == sf::Socket::Status::Done && packet.getDataSize() > 0)
						{
							//Incoming data from host.

							CF_SAY("Incoming data from host.", Settings::LogLevels::Debug);

							if (packet.getFlag() == cf::WorkPacket::Flag::None)
							{
								CF_SAY("Received unknown packet from host.", Settings::LogLevels::Error);
							}
							else if (packet.getFlag() == cf::WorkPacket::Flag::Task)
							{
								CF_SAY("Received task packet from host.", Settings::LogLevels::Info);

								std::string type;
								std::string subType;

								packet >> type;
								packet >> subType;

								if (type != "Task")
								{
									std::string s = "Received unknown packet from host.";
									CF_SAY(s, Settings::LogLevels::Error);
									CF_THROW(s);
								}

								//Check subtype exists in the constuction map.
								if (client->taskConstructMap.size() == 0 ||
									client->taskConstructMap.find(subType) == client->taskConstructMap.end()) {
									std::string s = "Unknown subtype in packet from host.";
									CF_SAY(s, Settings::LogLevels::Error);
									CF_THROW(s);
								}

								//Instantiate the resulting derived class.
								cf::Task *task = client->taskConstructMap[subType]();

								task->deserialize(packet);

								//Add task data to the client tasks queue.
								std::unique_lock<std::mutex> lock2(client->taskQueueMutex);
								client->taskQueue.push_back(task);
								CF_SAY("Added task " + std::to_string(task->getInitialTaskID()) + " to queue.", Settings::LogLevels::Info);
								lock2.unlock();

							}
							else
							{
								CF_THROW("Invalid flag data in packet from host. Are compression options set correctly on host and client?");
							}

							packet.clear();
						}
						else if (status == sf::Socket::Status::Disconnected)
						{
							CF_SAY("Disconnected by host.", Settings::LogLevels::Info);
							client->disconnect();
							packet.clear();
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
							CF_SAY("Invalid data from host. Ignoring.", Settings::LogLevels::Error);
							packet.clear();
						}
					}
				}

				//Sleep before looping again.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			listening = false;
			CF_SAY("Listener thread ended.", Settings::LogLevels::Info);
		}
		catch (...)
		{
			//Do nothing with exceptions in threads. Main thread will see the exception message via ConsoleMessager object.

			if (!cf::ConsoleMessager::getInstance()->exceptionThrown)
			{
				cf::ConsoleMessager::getInstance()->exceptionThrown = true;
				cf::ConsoleMessager::getInstance()->exceptionMessage = "Unknown exception in ClientListener listenThread.";
			}
		}
	}

}