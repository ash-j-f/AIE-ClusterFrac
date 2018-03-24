#include "Sender.h"
#include "Host.h"

namespace cf
{
	Sender::Sender(Host *newHost)
	{
		host = newHost;

		
	}

	Sender::~Sender()
	{
		
	}

	void Sender::sendTask(ClientDetails *client, Task *task)
	{
		std::unique_lock<std::mutex> lock(senderMutex);
		taskSendThreads.push_back(std::thread([this, client, task]() { sendTaskThread(client, task); }));
		CF_SAY("Task send thread started for client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Debug);
	}

	void Sender::waitForComplete()
	{
		std::unique_lock<std::mutex> lock(senderMutex);
		//Wait for threads to finish.
		for (auto &thread : taskSendThreads)
		{
			thread.join();
		}
		taskSendThreads.clear();
	}

	void Sender::sendTaskThread(ClientDetails *client, Task *task)
	{
		bool done = false;
		while (!done)
		{
			//CF_SAY("sendTaskThread trying lock.");
			std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
			if (lock.owns_lock())
			{
				CF_SAY("Sending task to client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Info);

				//Send task to client.
				cf::WorkPacket packet(cf::WorkPacket::Flag::Task);
				task->serialize(packet);

				//Socket is in non blocking mode, so more than one call to send may be needed to send all the data.
				sf::Socket::Status status;
				while (true)
				{
					status = client->socket->send(packet);
					if (status == sf::Socket::Status::Done)
					{
						CF_SAY("Sending task finished for client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Info);
						break;
					}
					else if (status == sf::Socket::Status::Partial)
					{
						//Partial send, so keep looping to continue sending.
						CF_SAY("Partial send to client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Debug);
					}
					else
					{
						std::string s = "Error while sending to client " + std::to_string(client->getClientID()) + ".";
						CF_SAY(s, Settings::LogLevels::Error);
						throw s;
						break;
					}
				};

				packet.clear();

				done = true;
				lock.unlock();
			}
			//Abort after a timeout.
			//TODO
			//done = true;
		}
	}
}