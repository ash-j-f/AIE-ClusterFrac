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
	sf::IpAddress IPADDRESS(ip);
	
	std::cout << "My IP address: " << IPADDRESS.toString() << std::endl;
	
	sf::TcpSocket socket;

	if (socket.connect(IPADDRESS, PORT) == sf::Socket::Done)
	{
		std::cout << "Connected to host at " << IPADDRESS.toString() << "." << std::endl;
	}
	else
	{
		std::cout << "Failed to connect to host." << std::endl;
	}

	cf::WorkPacket packet;

	std::cout << "Waiting for work packet." << std::endl;

	while (packet.getDataSize() == 0)
	{
		socket.receive(packet);
	}

	std::cout << "Received work packet." << std::endl;

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
		auto result = thread.get();
		std::cout << "Waiting for results and merging..." << std::endl;
		results.push_back(result);
	}
	
	cf::Result *bmr1 = new BenchmarkResult();
	bmr1->merge(results);

	//Clean up temporary results objects.
	for (auto &r : results ) delete r;

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

	socket.send(packet);

	packet.clear();
	
	std::cout << "Done." << std::endl;

	system("pause");

}