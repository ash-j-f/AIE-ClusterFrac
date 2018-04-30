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
#include "HostTaskWatcher.h"
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
		friend class HostTaskWatcher;

	public:

		/**
		* Default constructor.
		*/
		Host();

		/**
		* Default destructor.
		*/
		~Host();

		/**
		* Register new task construction callback.
		* @param name The name of the new task.
		* @param f The callback function to use to construct a new task of this type.
		* @returns void.
		*/
		inline void registerTaskType(std::string name, std::function<Task *()> f) 
		{
			CF_SAY("Registered task type " + name, Settings::LogLevels::Info); 
			taskConstuctMap[name] = f; 
		};

		/**
		* Register new result construction callback.
		* @param name The name of the new result.
		* @param f The callback function to use to construct a new result of this type.
		* @returns void.
		*/
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
		* Stop a host server.
		* Disconnects all connected clients and deletes all task and result data.
		* @returns void.
		*/
		void stop();

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
		inline int getClientsCount();

		/**
		* Get a count of the tasks in the task queue.
		* @returns The number of tasks in the task queue.
		*/
		inline int getTasksCount() { std::unique_lock<std::mutex> lock(taskQueueMutex); return (int)taskQueue.size(); };

		/**
		* Check if a result with a specified task ID is available in the results queue.
		* @param taskID The ID of the task that created the result.
		* @returns True if the result is in the queue, false if not.
		*/
		bool checkAvailableResult(unsigned __int64 taskID);

		/**
		* Get a pointer to a result in results queue based on its task ID.
		* @param taskID The ID of the task that created the result.
		* @returns A pointer to the result if it is available, or nullptr if it is not available.
		*/
		Result* getAvailableResult(unsigned __int64 taskID);

		/**
		* Is host-as-client enabled on this host?
		* @returns True if host-as-client is enabled, false if not.
		*/
		inline bool getHostAsClient() const { return hostAsClient; };

		/**
		* Enable or disable the host-as-client feature for this host.
		* Host-as-client means the host will process tasks as if it were one of the connected clients.
		* @param state Set true to enable host-as-client, false to disable.
		* @returns void.
		*/
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

		/**
		* Assign a task to this host as a client so that its progress can be tracked.
		* @returns void.
		*/
		inline void trackTask(Task* t) { std::unique_lock<std::mutex> lock(tasksAssignedAsClientMutex); tasksAssignedAsClient.push_back(t); };

		/**
		* Check which client (or host-as-client) was processing the task associated with a final result object.
		* Removes the task from that client and deletes the original task object from memory.
		* @param result A pointer to the result to check.
		* @returns True if a task matching the given result was found, and the task removal was successful, false if not.
		*/
		bool markTaskFinished(Result *result);

		/**
		* Get the average elapsed time for task processing.
		* @returns The average elapsed time for task processing, in sf::Time format.
		*/
		sf::Time getAverageBenchmarkTime();

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

		//Watcher object responsible for managing the watcher thread.
		HostTaskWatcher watcher{ this };

		//Connected client details.
		std::vector<ClientDetails *> clients;

		//Mutex for clients list.
		std::mutex clientsMutex;

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

		//Tasks assigned to this client.
		std::vector<Task *> tasksAssignedAsClient;

		//Task tracking list mutex.
		std::mutex tasksAssignedAsClientMutex;

		//Construction map for user defined Tasks.
		std::map<std::string, std::function<Task *()>> taskConstuctMap;

		//Construction map for user defined Results.
		std::map<std::string, std::function<Result *()>> resultConstructMap;

		//Is this host acting as a client for task processing?
		bool hostAsClient;

		//Is the host busy with a task?
		std::atomic<bool> busy;

		//Get the time since host startup.
		sf::Time getTime() { return clock.getElapsedTime(); };

		//Max number of benchmark times to store.
		unsigned int maxBenchmarkTimes;

		//Benchmark elapsed times for results processing.
		std::list<sf::Time> benchmarkTimes;

		void hostAsClientProcessTaskThread();

		/**
		* Check the incomplete results queue for complete result sets.
		* Completed results set that are found are moved to the complete results queue.
		* @returns void.
		*/
		void checkForCompleteResults();

		/**
		* Send sub tasks to connecte clients, and/or to the host as if it were a client if host-as-client is enabled.
		* @param subTaskQueue The queue of divided tasks to send to clients.
		* @returns void.
		*/
		void distributeSubTasks(std::vector<Task *> subTaskQueue);

		/**
		* Add an elapsed task time to the benchmark list.
		* @param elapsed The elapsed time in which a task was completed, in sf::Time format.
		* @returns void.
		*/
		void addBenchmarkTime(const sf::Time elapsed);

	};
}