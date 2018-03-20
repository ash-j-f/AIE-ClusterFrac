#include <iostream>
#include <vector>
#include <chrono>
#include <ClusterFrac.h>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp" 

class ClientDetails
{
public: 
	ClientDetails() 
	{
		socket = new sf::TcpSocket();
	};
	~ClientDetails() 
	{
		delete socket;
	};

	sf::TcpSocket *socket;
};

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{

	std::cout << "Starting ClusterFrac HOST." << std::endl;

	const unsigned short PORT = 5000;
	const sf::IpAddress IPADDRESS = sf::IpAddress::getLocalAddress();

	std::cout << "My IP address: " << IPADDRESS.toString() << std::endl;

	std::vector<ClientDetails *> clients;

	clients.push_back(new ClientDetails());

	sf::TcpListener listener;
	listener.listen(PORT);
	listener.accept(*clients[0]->socket);
	std::cout << "New client connected: " << (*clients[0]->socket).getRemoteAddress() << std::endl;

	BenchmarkTask *bmt1 = new BenchmarkTask();
	for (int i = 0; i < 10; i++) bmt1->numbers.push_back((float)rand() / 10.0f);

	cf::WorkPacket packet;
	//globalMutex.lock();
	
	
	//TIME TEST START
	auto start = std::chrono::steady_clock::now();

	bmt1->serialize(packet);
	//globalMutex.unlock();

	clients[0]->socket->send(packet);

	packet.clear();

	while (packet.getDataSize() == 0)
	{
		clients[0]->socket->receive(packet);
	}

	//TIME TEST END
	auto end = std::chrono::steady_clock::now();
	auto diff = end - start;
	
	std::cout << "Computation and network time: " << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;

	std::string type;
	packet >> type;

	std::string subType;
	packet >> subType;

	BenchmarkResult *bmr1 = new BenchmarkResult();

	bmr1->deserialize(packet);

	std::cout << "Square Roots:" << std::endl;

	for (int i = 0; i < bmr1->numbers.size(); i++)
	{
		std::cout << std::to_string(bmr1->numbers[i]) << std::endl;
	}

	//Clean up client detail objects.
	for (auto &c : clients) delete c;

	system("pause");

}