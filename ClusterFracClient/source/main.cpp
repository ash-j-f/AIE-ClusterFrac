#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <SFML/Network.hpp>
#include <ClusterFrac.h>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp"

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{

	///////////
	// REFERENCE CLIENT
	///////////

	std::cout << "Starting ClusterFrac CLIENT." << std::endl;

	const unsigned short PORT = 5000;

	//Set host address to connect to from command line.
	std::string ip;
	if (argc > 1)
	{
		ip = ip.assign(argv[1]);
	}
	else
	{
		//Use local address if none given.
		ip = sf::IpAddress::getLocalAddress().toString();
	}
	sf::IpAddress HOST_IPADDRESS(ip);
	
	sf::IpAddress MY_IPADDRESS(sf::IpAddress::getLocalAddress().toString());

	std::cout << "My IP address: " << MY_IPADDRESS.toString() << std::endl;
	
	sf::TcpSocket socket;

	cf::WorkPacket packet;

	bool connected = false;

	while (true)
	{
		if (!connected)
		{
			//Use socket blocking during connect attempt.
			socket.setBlocking(true);
			if (socket.connect(HOST_IPADDRESS, PORT) == sf::Socket::Done)
			{
				std::cout << "Connected to host at " << HOST_IPADDRESS.toString() << "." << std::endl;
				connected = true;
			}
			else
			{
				std::cout << "Failed to connect to host at " << HOST_IPADDRESS.toString() << ". Trying again." << std::endl;
				socket.disconnect();
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				connected = false;
			}
			socket.setBlocking(false);
		}

		while (connected)
		{

			std::cout << "Waiting for work packet." << std::endl;

			socket.setBlocking(false);
			sf::Socket::Status status;
			while (true)
			{
				status = socket.receive(packet);
				if (status == sf::Socket::Status::Done)
				{
					std::cout << "Received work packet." << std::endl;
					break;
				}
				else if (status == sf::Socket::Status::Disconnected)
				{
					std::cout << "Disconnected by host." << std::endl;
					socket.disconnect();
					connected = false;
					break;
				}
			}

			if (!connected) break;

			std::string type;
			packet >> type;

			std::string subType;
			packet >> subType;

			cf::Task *bmt1 = new BenchmarkTask();

			bmt1->deserialize(packet);

			packet.clear();

			std::cout << "Running task on multiple CPU threads." << std::endl;

			//Split the task among available threads and run.
			std::vector<cf::Task *> tasks = bmt1->split(std::thread::hardware_concurrency());

			delete bmt1;

			std::vector<std::future<cf::Result *>> threads = std::vector<std::future<cf::Result *>>();

			for (auto &task : tasks)
			{
				threads.push_back(std::async(std::launch::async, [&task]() { return task->run(); }));
			}

			std::vector<cf::Result *> results;

			for (auto &thread : threads)
			{
				std::cout << "Waiting for results and merging..." << std::endl;
				auto result = thread.get();
				results.push_back(result);
			}

			cf::Result *bmr1 = new BenchmarkResult();
			bmr1->merge(results);

			//Clean up temporary results objects.
			for (auto &r : results) delete r;

			std::cout << "Task complete." << std::endl;

			//Clean up temporary task objects.
			for (auto &task : tasks) delete task;

			//Remove old thread data.
			threads.clear();

			packet.clear();

			packet.setFlag(cf::WorkPacket::Flag::Result);

			bmr1->serialize(packet);

			delete bmr1;

			std::cout << "Sending results packet." << std::endl;
			
			socket.setBlocking(false);
			while (true)
			{
				status = socket.send(packet);
				if (status == sf::Socket::Status::Done)
				{
					break;
				}
				else if (status == sf::Socket::Status::Partial)
				{
					//Loop.
				}
				else if (status == sf::Socket::Status::Disconnected)
				{
					//Disconnected while sending.
					std::cout << "Disconnected by host during send." << std::endl;
					socket.disconnect();
					connected = false;
					break;
				}
				else
				{
					//Error while sending.
					std::cout << "Error during send. Forcing reconnect." << std::endl;
					socket.disconnect();
					connected = false;
					break;
				}
			}

			packet.clear();

			std::cout << "Done." << std::endl;
		}

	}
	system("pause");

}