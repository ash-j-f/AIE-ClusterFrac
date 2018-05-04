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
		stop();
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
		try
		{

			//If there is already a thread listening, abort.
			if (sending) return;

			//Register listening active.
			sending = true;

			sf::Socket::Status status;

			CF_SAY("Sender thread started. Automatically sending completed results to host.", Settings::LogLevels::Info);
			//Endless loop that waits for completed results and sends them to the host.
			//Aborts if send flag is set false.
			while (send && !cf::ConsoleMessager::getInstance()->exceptionThrown)
			{
				//Try to send one completed result object at a time.
				//Only proceed if there are results to send, and we are connected to the host.
				std::unique_lock<std::mutex> lock(client->resultsQueueMutex);
				cf::Result *result = client->resultQueueComplete.size() > 0 ? client->resultQueueComplete.front() : nullptr;
				lock.unlock();
				if (client->connected && result != nullptr)
				{

					cf::WorkPacket packet;

					packet.setFlag(cf::WorkPacket::Flag::Result);

					result->serialize(packet);

					CF_SAY("Sending results packet.", Settings::LogLevels::Info);

					//Packet send loop.
					while (!cf::ConsoleMessager::getInstance()->exceptionThrown)
					{
						//Get socket lock and send packet.
						std::unique_lock<std::mutex> lock2(client->socketMutex);
						status = client->socket.send(packet);
						lock2.unlock();

						if (status == sf::Socket::Status::Done)
						{
							//Send was successful. delete result object from memory and from the completed results list.
							std::unique_lock<std::mutex> lock3(client->resultsQueueMutex);
							client->resultQueueComplete.erase(std::remove(client->resultQueueComplete.begin(),
								client->resultQueueComplete.end(), result), client->resultQueueComplete.end());
							lock3.unlock();
							delete result;
							result = nullptr;
							CF_SAY("Packet sent.", Settings::LogLevels::Info);
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
				}

				//Sleep before looping again.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			sending = false;
			CF_SAY("Sender thread ended.", Settings::LogLevels::Info);
		}
		catch (...)
		{
			//Do nothing with exceptions in threads. Main thread will see the exception message via ConsoleMessager object.
		}
	}

}