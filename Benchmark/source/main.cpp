#include <iostream>
#include <vector>
#include <chrono>
#include <ClusterFrac.h>
#include <SFML\Window\Keyboard.hpp>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp" 

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{

	//Set log level for console messages.
	CF_SETTINGS->setLogLevel(cf::Settings::LogLevels::Info);

	//Create new host object.
	cf::Host *host = new cf::Host();

	//Set user defined Task and Result types.
	host->registerTaskType("BenchmarkTask", []() { BenchmarkTask *b = new BenchmarkTask(); return static_cast<cf::Task *>(b); });
	host->registerResultType("BenchmarkResult", []() { BenchmarkResult *b = new BenchmarkResult(); return static_cast<cf::Result *>(b); });
	
	//Allow this host to process tasks as a client.
	host->setHostAsClient(true);
	//Start the host.
	host->start();
	CF_SAY("Generating test data - started.", cf::Settings::LogLevels::Info);

	BenchmarkTask *testTask = new BenchmarkTask();
	
	//Generate test data.
	{
		//Split the task among available threads and run.
		int maxThreads = std::thread::hardware_concurrency();
		int dataSize = 2073600;
		std::vector<std::future<std::vector<float>>> threads = std::vector<std::future<std::vector<float>>>();

		for (int i = 0; i < maxThreads; i++)
		{
			threads.push_back(std::async(std::launch::async, [maxThreads, dataSize]() {
				std::vector<float> f;
				for (int i = 0; i < (dataSize / maxThreads); i++)
				{
					f.push_back((float)rand() / 100.0f);
				}
				return f; 
			}));
		}

		for (auto &thread : threads)
		{
			auto result = thread.get();
			testTask->numbers.insert(testTask->numbers.end(), result.begin(), result.end());
		}
	}

	int taskID = testTask->getInitialTaskID();

	CF_SAY("Generating test data - complete.", cf::Settings::LogLevels::Info);

	host->addTaskToQueue(testTask);

	//Wait for at least one client.
	CF_SAY("Waiting for clients.", cf::Settings::LogLevels::Info);
	while (host->getClientsCount() < 1)
	{
		//WAIT.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	//Wait for user input to continue.
	CF_SAY("Waiting for user to press B to start test.", cf::Settings::LogLevels::Info);
	while (!sf::Keyboard::isKeyPressed(sf::Keyboard::B))
	{
		//WAIT.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	//Start benchmark timer.
	auto start = std::chrono::steady_clock::now();

	if (host->sendTasks())
	{
		//Wait for results to be complete.
		CF_SAY("Waiting for completed results.", cf::Settings::LogLevels::Info);
		while (!host->checkAvailableResult(taskID))
		{
			//WAIT
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		cf::Result *finished = host->getAvailableResult(taskID);
		BenchmarkResult *output = static_cast<BenchmarkResult *>(finished);

		//Stop benchmark test clock.
		auto end = std::chrono::steady_clock::now();
		auto diff = end - start;

		//List results.
		CF_SAY("Results received (" + std::to_string(output->numbers.size()) + "):", cf::Settings::LogLevels::Info);
		for (int i = 0; i < 5; i++)
		{
			CF_SAY(std::to_string(output->numbers[i]), cf::Settings::LogLevels::Info);
		}
		CF_SAY("...", cf::Settings::LogLevels::Info);

		delete finished;

		CF_SAY("Computation and network time: " + std::to_string(std::chrono::duration <double, std::milli>(diff).count()) + " ms.", cf::Settings::LogLevels::Info);

		//Wait for user input to continue.
		CF_SAY("Waiting for user to press E to end test.", cf::Settings::LogLevels::Info);
		while (!sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			//WAIT
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	else
	{
		CF_SAY("Unable to send task. Aborting.", cf::Settings::LogLevels::Error);
	}

	delete testTask;

	delete host;

	///////////
	// REFERENCE HOST
	///////////

	//std::cout << "Starting ClusterFrac HOST." << std::endl;

	//const unsigned short PORT = 5000;
	//const sf::IpAddress IPADDRESS = sf::IpAddress::getLocalAddress();

	//std::cout << "My IP address: " << IPADDRESS.toString() << std::endl;

	//std::vector<ClientDetails *> clients;

	//clients.push_back(new ClientDetails());
	//
	//std::cout << "Waiting for client to connect." << std::endl;

	//sf::TcpListener listener;
	//listener.listen(PORT);
	//listener.accept(*clients[0]->socket);
	//std::cout << "New client connected: " << (*clients[0]->socket).getRemoteAddress() << std::endl;

	//std::cout << "Generating test data." << std::endl;

	//BenchmarkTask *bmt1 = new BenchmarkTask();
	//for (int i = 0; i < 1000000000; i++) bmt1->numbers.push_back((float)rand() / 10.0f);

	//cf::WorkPacket packet;
	////globalMutex.lock();
	//
	//
	//std::cout << "Sending data." << std::endl;

	////TIME TEST START
	//auto start = std::chrono::steady_clock::now();

	//bmt1->serialize(packet);

	//delete bmt1;

	////globalMutex.unlock();

	//clients[0]->socket->send(packet);

	//std::cout << "Data sent." << std::endl;

	//packet.clear();

	//std::cout << "Waiting for results." << std::endl;

	//while (packet.getDataSize() == 0)
	//{
	//	clients[0]->socket->receive(packet);
	//}

	//std::cout << "Results received." << std::endl;

	////TIME TEST END
	//auto end = std::chrono::steady_clock::now();
	//auto diff = end - start;
	//
	//std::cout << "Computation and network time: " << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;

	//std::cout << "Deserializing results packet." << std::endl;

	//std::string type;
	//packet >> type;

	//std::string subType;
	//packet >> subType;

	//if (type != "Result") throw "Not a result type.";

	//cf::Result *bmr1;
	//
	//if (subType == "BenchmarkResult")
	//{
	//	bmr1 = new BenchmarkResult();
	//}
	//else
	//{
	//	throw "Not a recognised result type.";
	//}
	//	
	//bmr1->deserialize(packet);

	//packet.clear();

	//std::cout << "Square Roots (" << std::to_string(((BenchmarkResult *)bmr1)->numbers.size()) << " total):" << std::endl;

	//for (int i = 0; i < 5 /*((BenchmarkResult *)bmr1)->numbers.size()*/; i++)
	//{
	//	std::cout << std::to_string(((BenchmarkResult *)bmr1)->numbers[i]) << std::endl;
	//}

	//std::cout << "..." << std::endl;

	//delete bmr1;

	////Clean up client detail objects.
	//for (auto &c : clients) delete c;

	//std::cout << "Done" << std::endl;

	system("pause");

}