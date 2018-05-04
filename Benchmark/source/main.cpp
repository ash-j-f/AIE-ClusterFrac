#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <Host.h>
#include <SFML\Window\Keyboard.hpp>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp" 
#include "BenchmarkStatManager.h"

/**
* Benchmark reference application that provides an example of using the ClusterFrac library host class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/

/**
* Main function, performing the basic benchmark task.
* @param argc The number of strings in array argv.
* @param argv The array of command-line argument strings.
* @param envp The array of environment variable strings.
*/
int main(int argc, char *argv[], char *envp[])
{
	try
	{

		//Set log level for console messages.
		//CF_SETTINGS->setLogLevel(cf::Settings::LogLevels::Debug);

		//Statistics record manager. Saves and loads statistics.
		BenchmarkStatManager bsm;

		CF_SAY("Hold Ctrl-Q to quit.", cf::Settings::LogLevels::Info);

		//Create new host object.
		cf::Host *host = new cf::Host();

		//Check if a non default port was specified.
		if (argc > 1)
		{
			host->setPort(atoi(argv[1]));
		}

		//Check if a non default concurrency was specified.
		if (argc > 2)
		{
			host->setConcurrency(atoi(argv[2]));
		}

		bool compression;
		//Check if a non default compression status was specified.
		if (argc > 3)
		{
			if (std::string(argv[3]) != "compression_on" && std::string(argv[3]) != "compression_off") CF_THROW("Unrecognised compression option on command line.");
			compression = std::string(argv[3]) == "compression_on";
		}
		else
		{
			compression = false;
		}

		bool autoRun = false;

		bool quit = false;

		//Set user defined Task and Result types.
		host->registerTaskType("BenchmarkTask", []{ BenchmarkTask *b = new BenchmarkTask(); return static_cast<cf::Task *>(b); });
		host->registerResultType("BenchmarkResult", []{ BenchmarkResult *b = new BenchmarkResult(); return static_cast<cf::Result *>(b); });

		host->setCompression(compression);

		//Allow this host to process tasks as a client.
		host->setHostAsClient(true);
		//Start the host.
		host->start();
		CF_SAY("Generating test data - started.", cf::Settings::LogLevels::Info);

		//Generate test data. Data range is INCLUSIVE.
		int dataRangeStart = 1;
		int dataRangeEnd = 100;
		unsigned int cycles = 10000000; //100000000;
		int timeout = 30000; //Timeout per task chunk in ms.
		int valueCount = (dataRangeEnd - dataRangeStart) + 1;
		std::vector<double> expectedResults(valueCount);

		{
			//Split the task among available threads and run.
			unsigned int maxThreads = std::thread::hardware_concurrency();
			std::vector<std::future<std::vector<double>>> threads = std::vector<std::future<std::vector<double>>>();

			const unsigned int step = (int)floor(valueCount / (float)maxThreads);
			int start = dataRangeStart;
			int end = dataRangeStart + (step - 1);
			for (unsigned int i = 0; i < maxThreads; i++)
			{
				//If this is the final split, then get the remainder of items.
				if (i == maxThreads - 1) end = dataRangeStart + valueCount;

				int tstart = start;
				int tend = std::min(end, dataRangeStart + (valueCount - 1));

				threads.push_back(std::async(std::launch::async, [maxThreads, valueCount, tstart, tend, cycles]() {
					std::vector<double> f;
					for (int j = tstart; j <= tend; j++)
					{

						double r = 0;
						for (unsigned int c = 0; c < cycles; c++)
						{
							r += ((double)sqrtl(j + c)) / (double)cycles;
						}

						f.push_back(r);
					}
					return f;
				}));

				start = start + step;
				end = end + step;
			}

			unsigned int j = 0;
			for (auto &thread : threads)
			{
				auto result = thread.get();
				size_t resultCount = result.size();
				for (size_t i = 0; i < resultCount; i++)
				{
					expectedResults[j] = result[i];
					j++;
				}
			}
		}

		CF_SAY("Generating test data - complete.", cf::Settings::LogLevels::Info);

		while (!quit && !cf::ConsoleMessager::getInstance()->exceptionThrown)
		{
			BenchmarkTask *testTask = new BenchmarkTask();
			
			//Set a high timeout value. This task could take a long time per client node.
			testTask->setMaxTaskTimeMilliseconds(timeout);

			//Assign an ID to this task.
			testTask->assignID();

			//Insert test data into test task.
			testTask->dataRangeStart = dataRangeStart;
			testTask->dataRangeEnd = dataRangeEnd;
			testTask->cycles = cycles;

			unsigned __int64 taskID = testTask->getInitialTaskID();

			host->addTaskToQueue(testTask);

			//Wait for at least one client.
			CF_SAY("Waiting for clients. Hold Ctrl-Q to quit.", cf::Settings::LogLevels::Info);
			while (host->getClientsCount() < 1 && !(sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))))
			{
				//WAIT.
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))) break;

			//Start benchmark timer.
			auto start = std::chrono::steady_clock::now();

			//Wait for results to be complete.
			CF_SAY("Waiting for completed results.", cf::Settings::LogLevels::Info);
			while (!host->checkAvailableResult(taskID) && !cf::ConsoleMessager::getInstance()->exceptionThrown)
			{
				//WAIT
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			//Abort main loop if exception was thrown by a thread.
			if (cf::ConsoleMessager::getInstance()->exceptionThrown) break;

			cf::Result *finished = host->getAvailableResult(taskID);
			BenchmarkResult *output = static_cast<BenchmarkResult *>(finished);

			//Stop benchmark test clock.
			auto end = std::chrono::steady_clock::now();
			auto diff = end - start;

			//List results.
			CF_SAY("Results received (" + std::to_string(output->numbers.size()) + "):", cf::Settings::LogLevels::Info);
			for (int i = 0; i < (output->numbers.size() >= 10 ? 10 : (int)output->numbers.size()); i++)
			{

				char buffer[1024] = { '\0' };
				sprintf_s(buffer, "%.15g", output->numbers[i]);

				CF_SAY(std::string(buffer), cf::Settings::LogLevels::Info);
			}
			CF_SAY("...", cf::Settings::LogLevels::Info);

			CF_SAY("Verifying results.", cf::Settings::LogLevels::Info);
			for (int i = 0; i < (int)expectedResults.size(); i++)
			{
				if (expectedResults[i] != output->numbers[i]) CF_THROW("Results verification failed. Results did not match!");
			}
			CF_SAY("Results verified OK from " + std::to_string(host->getClientsCount()) + " clients.", cf::Settings::LogLevels::Info);

			//Remove the result from the completed results queue.
			host->removeResultFromQueue(finished);
			finished = nullptr;
			output = nullptr;

			//Collect test statistics.
			double timeMilliseonds = std::chrono::duration <double, std::milli>(diff).count();
			bsm.addStat(timeMilliseonds);
			double average = bsm.getAverage();
			unsigned int testCount = bsm.getCount();

			CF_SAY("Computation and network time: " + std::to_string(timeMilliseonds) + " ms.", cf::Settings::LogLevels::Info);
			if (testCount > 0) CF_SAY("Previous " + std::to_string(testCount) + " tests average time: " + std::to_string(average) + " ms.", cf::Settings::LogLevels::Info);
			CF_SAY("Test complete.\n", cf::Settings::LogLevels::Info);

			//Abort main loop if exception was thrown by a thread.
			if (cf::ConsoleMessager::getInstance()->exceptionThrown) break;

			if (autoRun)
			{
				CF_SAY("Automatically running test again in 3 seconds. Q to cancel.", cf::Settings::LogLevels::Info);
				sf::Clock elapsed;
				while (elapsed.getElapsedTime().asSeconds() < 3.0f)
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
					{
						CF_SAY("Auto-run cancelled.\n", cf::Settings::LogLevels::Info);
						autoRun = false;
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				if (autoRun) CF_SAY("Running test.\n", cf::Settings::LogLevels::Info);
			}

			if (!autoRun)
			{
				CF_SAY("R to run test again. A to enable auto-run. Ctrl-Q to quit.", cf::Settings::LogLevels::Info);

				while (true)
				{

					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))
					{
						CF_SAY("User requested program shutdown.\n", cf::Settings::LogLevels::Info);
						quit = true;
						break;
					}

					if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
					{
						CF_SAY("Auto-run enabled.", cf::Settings::LogLevels::Info);
						CF_SAY("Running test.\n", cf::Settings::LogLevels::Info);
						autoRun = true;
						break;
					}

					if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
					{
						CF_SAY("Running test.\n", cf::Settings::LogLevels::Info);
						break;
					}
				}
			}

		}

		if (cf::ConsoleMessager::getInstance()->exceptionThrown)
		{
			CF_SAY("\nExeception thrown. Aborting.", cf::Settings::LogLevels::Error);
			CF_SAY("Exeception was: " + cf::ConsoleMessager::getInstance()->exceptionMessage + "\n", cf::Settings::LogLevels::Error);
		}

		delete host;
		host = nullptr;
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