#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <Host.h>
#include <SFML\Window\Keyboard.hpp>
#include "BenchmarkTask.hpp" 
#include "BenchmarkResult.hpp" 

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

		CF_SAY("Hold Ctrl-Q to quit.", cf::Settings::LogLevels::Info);

		//Create new host object.
		cf::Host *host = new cf::Host();

		//Check if a non default port was specified.
		if (argc > 1)
		{
			host->setPort(atoi(argv[1]));
		}

		//Set user defined Task and Result types.
		host->registerTaskType("BenchmarkTask", []{ BenchmarkTask *b = new BenchmarkTask(); return static_cast<cf::Task *>(b); });
		host->registerResultType("BenchmarkResult", []{ BenchmarkResult *b = new BenchmarkResult(); return static_cast<cf::Result *>(b); });

		//Allow this host to process tasks as a client.
		host->setHostAsClient(true);
		//Start the host.
		host->start();
		CF_SAY("Generating test data - started.", cf::Settings::LogLevels::Info);

		//Generate test data. Data range is INCLUSIVE.
		int dataRangeStart = 1;
		int dataRangeEnd = 100000000;
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

				threads.push_back(std::async(std::launch::async, [maxThreads, valueCount, tstart, tend]() {
					std::vector<double> f;
					for (int v = tstart; v <= tend; v++)
					{
						f.push_back((double)sqrtl((double)v));
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

		while (true)
		{
			BenchmarkTask *testTask = new BenchmarkTask();
			
			//Assign an ID to this task.
			testTask->assignID();

			//Insert test data into test task.
			testTask->dataRangeStart = dataRangeStart;
			testTask->dataRangeEnd = dataRangeEnd;

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
			for (int i = 0; i < (output->numbers.size() >= 10 ? 10 : (int)output->numbers.size()); i++)
			{
				CF_SAY(std::to_string(output->numbers[i]), cf::Settings::LogLevels::Info);
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

			CF_SAY("Computation and network time: " + std::to_string(std::chrono::duration <double, std::milli>(diff).count()) + " ms.", cf::Settings::LogLevels::Info);
			
			CF_SAY("Test complete. Running again in 3 secs. Hold Ctrl-Q to quit.", cf::Settings::LogLevels::Info);
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
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