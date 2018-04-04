#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <SFML\Window\Keyboard.hpp>
#include <SFML\Network.hpp>
#include <Client.h>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp"

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{
	try
	{
		bool quit = false;

		CF_SAY("Hold Q to quit.", cf::Settings::LogLevels::Info);

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

		int port;
		//Check if a non default port was specified.
		if (argc > 2)
		{
			port = atoi(argv[2]);
		}
		else
		{
			//Use default port if none given.
			port = 5000;
		}

		cf::Client *c = new cf::Client();

		//Set user defined Task and Result types.
		c->registerTaskType("BenchmarkTask", []() { BenchmarkTask *b = new BenchmarkTask(); return static_cast<cf::Task *>(b); });
		c->registerResultType("BenchmarkResult", []() { BenchmarkResult *b = new BenchmarkResult(); return static_cast<cf::Result *>(b); });

		c->setPort(port);

		c->setIPAddress(ip);

		c->start();

		//Loop infinitely, trying to connect to host, and processing tasks once connected.
		while (!quit)
		{
			//Try to connect to host.
			while (true)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
				{
					quit = true;
					break;
				}
				if (c->connect()) break;
				CF_SAY("Unable to connect. Retrying.", cf::Settings::LogLevels::Error);
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			//Process tasks while connected.
			while (c->isConnected())
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
				{
					quit = true;
					break;
				}
				//Sleep while threads wait for tasks and process them.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		delete c;
	}
	catch (std::string e)
	{
		std::cerr << e << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception." << std::endl;
	}

	system("pause");

}