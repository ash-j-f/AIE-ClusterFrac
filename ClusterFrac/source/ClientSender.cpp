#include "ClientSender.h"
#include "Client.h"

namespace cf
{
	ClientSender::ClientSender(Client *newClient)
	{
		client = newClient;

		//Sender default status.
		send = false;
		sending = false;

		started = false;
	}

	ClientSender::~ClientSender()
	{
		if (send)
		{
			//Set send status to signal sender thread to shut down.
			send = false;

			//Wait for the sender thread to shut down.
			if (senderThread.joinable()) senderThread.join();
		}
	}

	void ClientSender::start()
	{
		//If sender is already started, do nothing.
		if (started) return;

		started = true;

		//Launch sender thread.
		send = true;
		senderThread = std::thread([this] { sendThread(); });
	}

	void ClientSender::stop()
	{
		//If already stopped, do nothing.
		if (!started) return;

		//Set send status to signal sender thread to shut down.
		send = false;

		//Wait for the sender thread to shut down.
		if (senderThread.joinable()) senderThread.join();

		started = false;
	}

	void ClientSender::sendThread()
	{
		//If there is already a thread listening, abort.
		if (sending) return;

		//Register listening active.
		sending = true;

		sf::Socket::Status status;

		cf::WorkPacket packet;

		CF_SAY("Sender thread started. Automatically sending completed results to host.", Settings::LogLevels::Info);
		//Endless loop that waits for completed results and sends them to the host.
		//Aborts if send flag is set false.
		while (send)
		{
			//Try to send one completed result object at a time.
			//Only proceed if there are results to send, and we are connected to the host.
			std::unique_lock<std::mutex> lock(client->resultsQueueMutex);
			cf::Result *result = client->resultQueueComplete.size() > 0 ? client->resultQueueComplete.front() : nullptr;
			lock.unlock();
			if (client->connected && result != nullptr)
			{

				packet.setFlag(cf::WorkPacket::Flag::Result);

				result->serialize(packet);

				CF_SAY("Sending results packet.", Settings::LogLevels::Info);
				
				//Get socket lock.
				std::unique_lock<std::mutex> lock2(client->socketMutex);
				while (true)
				{
					status = client->socket.send(packet);
					if (status == sf::Socket::Status::Done)
					{
						//Send was successful. Delete result object from memory and from the completed results list.
						std::unique_lock<std::mutex> lock3(client->resultsQueueMutex);
						delete result;
						client->resultQueueComplete.erase(std::remove(client->resultQueueComplete.begin(),
							client->resultQueueComplete.end(), result), client->resultQueueComplete.end());
						lock3.unlock();
						break;
					}
					else if (status == sf::Socket::Status::Partial)
					{
						//Sending only partially complete, so continue to loop.
					}
					else if (status == sf::Socket::Status::Disconnected)
					{
						//Disconnected while sending.
						CF_SAY("Disconnected by host during send.", Settings::LogLevels::Error);
						client->disconnect();
						break;
					}
					else
					{
						//Error while sending.
						CF_SAY("Error during send. Forcing reconnect.", Settings::LogLevels::Error);
						client->disconnect();
						break;
					}
				}
				lock2.unlock();

			}
			













			//Skip checking socket if we're not connected to a host.
			if (client->connected)
			{

				//CF_SAY("Sending.");

				//Attempt to get socket lock. Try again later if another process already has a lock on this socket.
				//CF_SAY("sendThread send client data try lock.");
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
								throw s;
							}

							//Check subtype exists in the constuction map.
							if (client->taskConstructMap.size() == 0 ||
								client->taskConstructMap.find(subType) == client->taskConstructMap.end()) {
								std::string s = "Unknown subtype in packet from host.";
								CF_SAY(s, Settings::LogLevels::Error);
								throw s;
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
							throw "Invalid flag data in packet from host.";
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

		sending = false;
		CF_SAY("Sender thread ended.", Settings::LogLevels::Info);
	}

}