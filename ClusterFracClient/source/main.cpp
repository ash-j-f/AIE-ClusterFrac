#include <iostream>
#include <SFML/Network.hpp>

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{

	std::cout << "Starting ClusterFrac client." << std::endl;

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

	system("pause");

}