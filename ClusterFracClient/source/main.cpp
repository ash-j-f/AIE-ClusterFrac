#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <ClusterFrac.h>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp"

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{
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
		ip = "10.10.1.89";
	}
	const sf::IpAddress IPADDRESS(ip);
	
	std::cout << "My IP address: " << IPADDRESS.toString() << std::endl;
	
	sf::TcpSocket socket;

	if (socket.connect(IPADDRESS, PORT) == sf::Socket::Done)
	{
		std::cout << "Connected." << std::endl;
	}
	else
	{
		std::cout << "Failed." << std::endl;
	}

	cf::WorkPacket packet;

	while (packet.getDataSize() == 0)
	{
		socket.receive(packet);
	}

	std::string type;
	packet >> type;

	std::string subType;
	packet >> subType;
	
	cf::Task *bmt1 = new BenchmarkTask();

	bmt1->deserialize(packet);

	cf::Result *bmr1 = bmt1->run();

	packet.clear();

	bmr1->serialize(packet);

	socket.send(packet);

	std::cout << "Done." << std::endl;

	system("pause");

}