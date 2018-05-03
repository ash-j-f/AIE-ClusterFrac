#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <SFML\Window\Keyboard.hpp>
#include <SFML\Network.hpp>
#include <Client.h>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp"
#include "MandelbrotTask.hpp"
#include "MandelbrotResult.hpp"

/**
* Client reference application that provides an example of using the ClusterFrac library client class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/

/**
* Main function, performing the core client task.
* @param argc The number of strings in array argv.
* @param argv The array of command-line argument strings.
* @param envp The array of environment variable strings.
*/
int main(int argc, char *argv[], char *envp[])
{

	//CF_SETTINGS->setLogLevel(cf::Settings::LogLevels::Error);

	try
	{
		bool quit = false;

		CF_SAY("Hold Ctrl-Q to quit.", cf::Settings::LogLevels::Info);

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

		int concurrency;
		//Check if a non default concurrency was specified.
		if (argc > 3)
		{
			concurrency = atoi(argv[3]);
		}
		else
		{
			concurrency = 0;
		}

		cf::Client *c = new cf::Client();

		//Set user defined Task and Result types.
		c->registerTaskType("BenchmarkTask", []{ BenchmarkTask *b = new BenchmarkTask(); return static_cast<cf::Task *>(b); });
		c->registerResultType("BenchmarkResult", []{ BenchmarkResult *b = new BenchmarkResult(); return static_cast<cf::Result *>(b); });
		c->registerTaskType("MandelbrotTask", []{ MandelbrotTask *m = new MandelbrotTask(); return static_cast<cf::Task *>(m); });
		c->registerResultType("MandelbrotResult", []{ MandelbrotResult *m = new MandelbrotResult(); return static_cast<cf::Result *>(m); });

		c->setPort(port);

		c->setIPAddress(ip);

		c->setConcurrency(concurrency);

		c->start();

		//Loop infinitely, trying to connect to host, and processing tasks once connected.
		while (!quit)
		{
			//Try to connect to host.
			while (true)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))
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
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))
				{
					quit = true;
					break;
				}
				//Sleep while threads wait for tasks and process them.
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		delete c;
		c = nullptr;
	}
	catch (std::string e)
	{
		std::cerr << e << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception." << std::endl;
	}

}