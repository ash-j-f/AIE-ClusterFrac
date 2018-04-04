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

		inline void registerTaskType(std::string name, std::function<Task *()> f) { taskConstuctMap[name] = f; };

		inline void registerResultType(std::string name, std::function<Result *()> f) { resultConstructMap[name] = f; };

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
		* Get a count of all connected clients.
		* Includes the host itself if hostAsClient is set true.
		* @returns The number of connected clients.
		*/
		inline int getClientsCount() const;

		bool checkAvailableResult(int taskID);

		Result* getAvailableResult(int taskID);

		inline bool getHostAsClient() const { return hostAsClient; };

		void setHostAsClient(bool state);

	private:

		//Maximum threads for multithread process forking.
		int MAX_THREADS;

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

		void hostAsClientProcessTaskThread();

		/**
		* Check the incomplete results queue for complete result sets.
		* Completed results set that are found are moved to the complete results queue.
		* @returns void.
		*/
		void checkForCompleteResults();
	};
}