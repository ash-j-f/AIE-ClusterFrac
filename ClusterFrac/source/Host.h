#pragma once
#include <vector>
#include <list>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>
#include <string>
#include <map>
#include <unordered_set>
#include <SFML\Network.hpp>
#include "DllExport.h"
#include "ConsoleMessager.hpp"
#include "Task.h"
#include "Result.h"
#include "IDManager.h"
#include "HostListener.h"
#include "HostSender.h"
#include "ClientDetails.hpp"

namespace cf
{
	/**
	* Host class. The host ClusterFrac server which manages connections to clients,
	* and sends/receives work packets.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL Host
	{

		//These classes require full access to host.
		friend class HostListener;
		friend class HostSender;

	public:

		/**
		* Default constructor.
		*/
		Host();

		/**
		* Default destructor.
		*/
		~Host();

		inline void registerTaskType(std::string name, std::function<Task *()> f) 
		{
			CF_SAY("Registered task type " + name, Settings::LogLevels::Info); 
			taskConstuctMap[name] = f; 
		};

		inline void registerResultType(std::string name, std::function<Result *()> f) 
		{
			CF_SAY("Registered result type " + name, Settings::LogLevels::Info);
			resultConstructMap[name] = f; 
		};

		/**
		* Start a host server.
		* @returns void.
		*/
		void start();

		/**
		* Set the port number for the host server.
		* @param portNum The port number to use. Must be in the range [1 .. 65535].
		* @returns void.
		*/
		void setPort(int portNum);

		/**
		* Add a task to the task queue for sending to clients.
		* @param task The task to add.
		* @returns void.
		*/
		void addTaskToQueue(Task *task);

		/**
		* Send tasks to clients for processing.
		* @returns True if sending succeeded, false if not.
		*/
		bool sendTasks();

		/**
		* Add a result to the result queue.
		* @param result The result to add.
		* @returns void.
		*/
		void addResultToQueue(Result *result);

		/**
		* Remove a result from the result queue.
		* @param result The result to remove.
		* @returns void.
		*/
		void removeResultFromQueue(Result *result);

		/**
		* Get a count of all connected clients.
		* Includes the host itself if hostAsClient is set true.
		* @returns The number of connected clients.
		*/
		inline int getClientsCount() const;

		bool checkAvailableResult(int taskID);

		Result* getAvailableResult(int taskID);

		inline bool getHostAsClient() const { return hostAsClient; };

		void setHostAsClient(bool state);

		/**
		* Set thread concurrency to use while procesing tasks.
		* Setting value to 0 means use hardware defined max concurrency.
		* Will produce a warning if threads is set lower or higher than the hardware
		* defined maximum.
		* @param n The number of concurrent threads to run while processing tasks.
		* @returns void.
		*/
		inline void setConcurrency(unsigned int n)
		{
			if (n > 65535) CF_THROW("Invalid concurrency value.");
			if (n == 0) n = std::thread::hardware_concurrency();
			MAX_THREADS = n;
			CF_SAY("Concurrency is set to " + std::to_string(MAX_THREADS) + " thread(s).", Settings::LogLevels::Info);
			if (MAX_THREADS < std::thread::hardware_concurrency()) CF_SAY("WARNING: Concurrency is LOWER than CPU max concurrent threads.", Settings::LogLevels::Info);
			if (MAX_THREADS > std::thread::hardware_concurrency()) CF_SAY("WARNING: Concurrency is HIGHER than CPU max concurrent threads.", Settings::LogLevels::Info);
		};

	private:

		//clock used to track time since startup.
		sf::Clock clock;

		//Maximum threads for multithread process forking.
		unsigned int MAX_THREADS;

		//Has the host been started?
		std::atomic<bool> started;

		//Port number this server is using.
		int port;

		//Listener object responsible for managing the TCP listener thread.
		HostListener listener{ this };

		//Sender object responsible for managing sender threads.
		HostSender sender{ this };

		//Connected client details.
		std::vector<ClientDetails *> clients;

		//Task queue.
		std::list<cf::Task *> taskQueue;

		//Mutex for task queue
		std::mutex taskQueueMutex;

		//Incomplete results queue.
		std::list<cf::Result *> resultQueueIncomplete;

		//Complete results queue.
		std::list<cf::Result *> resultQueueComplete;

		//Local task queue for host, that it should process as a client if hostAsClient is enabled.
		std::list<cf::Task *> localHostAsClientTaskQueue;

		//Mutex for local task queue.
		std::mutex localHostAsClientTaskQueueMutex;

		//Thread used to process task chunks locally on the host.
		std::thread hostAsClientTaskProcessingThread;

		//Should the host processing tasks as a client thread continue to run?
		std::atomic<bool> hostAsClientTaskProcessThreadRun;

		//Mutex for results queues
		std::mutex resultsQueueMutex;

		//Construction map for user defined Tasks.
		std::map<std::string, std::function<Task *()>> taskConstuctMap;

		//Construction map for user defined Results.
		std::map<std::string, std::function<Result *()>> resultConstructMap;

		//Is this host acting as a client for task processing?
		bool hostAsClient;

		//Get the time since host startup.
		sf::Time getTime() { return clock.getElapsedTime(); };

		void hostAsClientProcessTaskThread();

		/**
		* Check the incomplete results queue for complete result sets.
		* Completed results set that are found are moved to the complete results queue.
		* @returns void.
		*/
		void checkForCompleteResults();
	};
}