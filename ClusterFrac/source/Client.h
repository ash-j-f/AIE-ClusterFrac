#pragma once
#include <string>
#include <atomic>
#include <list>
#include <thread>
#include <future>
#include <SFML\Network.hpp>
#include "DllExport.h"
#include "ConsoleMessager.hpp"
#include "Task.h"
#include "Result.h"

namespace cf
{
	/**
	* Client class. The ClusterFrac client connects to a chosen host and processes
	* work packets it is sent.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL Client
	{
	public:

		/**
		* Default constructor.
		*/
		Client();

		/**
		* Default destructor.
		*/
		~Client();

		inline void registerTaskType(std::string name, std::function<Task *()> f) { taskConstuctMap[name] = f; };

		inline void registerResultType(std::string name, std::function<Result *()> f) { resultConstructMap[name] = f; };

		/**
		* Start the client.
		* @returns void.
		*/
		void start();

		/**
		* Set the port number for the client to use for outgoing connections.
		* @param portNum The port number to use. Must be in the range [1 .. 65535].
		* @returns void.
		*/
		void setPort(int portNum);

		/**
		* Add a task to the task queue for processing.
		* @param task The task to add.
		* @returns void.
		*/
		void addTaskToQueue(Task *task);

	private:

		//Has the host been started?
		std::atomic<bool> started;

		//Port number this server is using.
		int port;

		//Task queue.
		std::list<cf::Task *> taskQueue;

		//Mutex for task queue
		std::mutex taskQueueMutex;

		//Incomplete results queue.
		std::list<cf::Result *> resultQueueIncomplete;

		//Complete results queue.
		std::list<cf::Result *> resultQueueComplete;

		//Mutex for results queues
		std::mutex resultsQueueMutex;

		//Construction map for user defined Tasks.
		std::map<std::string, std::function<Task *()>> taskConstuctMap;

		//Construction map for user defined Results.
		std::map<std::string, std::function<Result *()>> resultConstructMap;

		//Thread that loops continuously while client is running.
		std::thread loopingThread;

		//Thread used to process task chunks locally on the host.
		std::thread TaskProcessingThread;

		//Should the continous loop thread keep running?
		std::atomic<bool> loopThreadRun;

		//Should the client tasks processing thread continue to run?
		std::atomic<bool> ProcessTaskThreadRun;

		void loopThread();

		void ProcessTaskThread();

		/**
		* Check the incomplete results queue for complete result sets.
		* Completed results set that are found are moved to the complete results queue.
		* @returns void.
		*/
		void checkForCompleteResults();
	};
}