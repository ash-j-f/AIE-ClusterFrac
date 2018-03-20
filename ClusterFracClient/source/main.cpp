#include <iostream>
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
	const sf::IpAddress IPADDRESS = sf::IpAddress::getLocalAddress();
	
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
	
	BenchmarkTask *bmt1 = new BenchmarkTask();

	bmt1->deserialize(packet);

	bmt1->run();

	packet.clear();

	bmt1->serialize(packet);

	socket.send(packet);

	std::cout << "Done." << std::endl;

	system("pause");

}