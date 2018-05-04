#include "HostSender.h"
#include "Host.h"

namespace cf
{
	HostSender::HostSender(Host *newHost)
	{
		host = newHost;
	}

	HostSender::~HostSender()
	{
		
	}

	void HostSender::sendTask(ClientDetails *client, Task *task)
	{
		std::unique_lock<std::mutex> lock(senderMutex);
		taskSendThreads.push_back(std::thread([this, client, task]() { sendTaskThread(client, task); }));
		CF_SAY("Task send thread started for client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Debug);
	}

	void HostSender::waitForComplete()
	{
		std::unique_lock<std::mutex> lock(senderMutex);
		//Wait for threads to finish.
		for (auto &thread : taskSendThreads)
		{
			thread.join();
		}
		taskSendThreads.clear();
	}

	void HostSender::sendTaskThread(ClientDetails *client, Task *task)
	{
		try
		{

			bool done = false;
			while (!done && !cf::ConsoleMessager::getInstance()->exceptionThrown)
			{

				std::unique_lock<std::mutex> lock(client->socketMutex, std::try_to_lock);
				if (lock.owns_lock())
				{
					CF_SAY("Sending task to client " + std::to_string(client->getClientID()) + ".", Settings::LogLevels::Info);

					cf::WorkPacket packet(cf::WorkPacket::Flag::Task);

					//Enable compression if requested.
					packet.setCompression(host->compression);

					task->serialize(packet);

					//Socket is in non blocking mode, so more than one call to send may be needed to send all the data.
					sf::Socket::Status status;
					while (!cf::ConsoleMessager::getInstance()->exceptionThrown)
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
							std::string s = "Error while sending to client " + std::to_string(client->getClientID()) + ". Aborting.";
							CF_SAY(s, Settings::LogLevels::Error);
							CF_THROW(s);
							break;
						}
					};

					packet.clear();
					lock.unlock();

					done = true;

				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		catch (...)
		{
			//Do nothing with exceptions in threads. Main thread will see the exception message via ConsoleMessager object.
		}
	}
}