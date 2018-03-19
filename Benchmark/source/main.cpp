#include <iostream>
#include <vector>
#include <ClusterFrac.h>
#include "BenchmarkTask.h" 
#include "BenchmarkResult.h" 

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
	bmt1->serialize(packet);
	//globalMutex.unlock();

	clients[0]->socket->send(packet);

	//Clean up client detail objects.
	for (auto &c : clients) delete c;

	system("pause");

		//BenchmarkTask *bmt1 = new BenchmarkTask();

		//for (int i = 0; i < 1000000; i++) bmt1->numbers.push_back((float)rand() / 10.0f);

		//std::vector<cf::Task *> splitTasks = bmt1->split(8);

		//std::vector<cf::Result *> resultList;
		//for (auto task : splitTasks) resultList.push_back(task->run());

		//cf::Result *mergedResults = resultList[0]->merge(resultList);

		////cf::Result *mergedResults = bmr1->merge(std::vector<cf::Result *> { bmr1, bmr2 });

		//cf::WorkPacket p;
		////splitTasks[0]->serialize(p);
		////
		//////Determine type here.
		////std::string type;
		////p >> type;
		////
		////bmt2->deserialize(p);

		//bmt1->serialize(p);

	
}