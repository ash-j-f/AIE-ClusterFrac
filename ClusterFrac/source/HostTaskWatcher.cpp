#include "HostTaskWatcher.h"
#include "Host.h"

namespace cf
{
	HostTaskWatcher::HostTaskWatcher(Host *newHost)
	{
		host = newHost;

		//Listening default status.
		watch = false;
		watching = false;

		started = false;
	}

	HostTaskWatcher::~HostTaskWatcher()
	{
		stop();
	}

	void HostTaskWatcher::start()
	{
		//If watcher is already started, do nothing.
		if (started) return;

		started = true;

		//Launch watcher thread.
		watch = true;
		watcherThread = std::thread([this] { watchThread(); });
	}

	void HostTaskWatcher::stop()
	{
		//If already stopped, do nothing.
		if (!started) return;

		//Set watch status to signal watcher thread to shut down.
		watch = false;

		//Wait for the watcher thread to shut down.
		if (watcherThread.joinable()) watcherThread.join();

		started = false;
	}

	void HostTaskWatcher::watchThread()
	{
		//If there is already a thread watching, abort.
		if (watching) return;

		//Register watcher active.
		watching = true;

		CF_SAY("Watcher thread started. Watching task status.", Settings::LogLevels::Info);
		//Endless loop that watches task status.
		//Aborts if watch flag is set false.
		while (watch)
		{
			//CF_SAY("Watching.");


			//TODO
			//xxxx
			//DO TASK STATUS WATCHING STUFF HERE
		}

		watching = false;
		CF_SAY("Watcher thread ended.", Settings::LogLevels::Info);
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
						std::unique_lock<std::mutex> lock(host->resultsQueueMutex);
						host->resultQueueIncomplete.push_back(result);
						lock.unlock();
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
				(*client->socket).disconnect();
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