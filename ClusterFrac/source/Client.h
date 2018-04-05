#pragma once
#include <string>
#include <atomic>
#include <list>
#include <thread>
#include <future>
#include <unordered_set>
#include <SFML\Network.hpp>
#include "DllExport.h"
#include "ConsoleMessager.hpp"
#include "Task.h"
#include "Result.h"
#include "ClientListener.h"
#include "ClientSender.h"

namespace cf
{
	/**
	* Client class. The ClusterFrac client connects to a chosen host and processes
	* work packets it is sent.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL Client
	{

		//These classes require full access to client.
		friend class ClientListener;
		friend class ClientSender;

	public:

		/**
		* Default constructor.
		*/
		Client();

		/**
		* Default destructor.
		*/
		~Client();

		inline void registerTaskType(std::string name, std::function<Task *()> f) 
		{ 
			CF_SAY("Registered task type " + name, Settings::LogLevels::Info); 
			taskConstructMap[name] = f; 
		};

		inline void registerResultType(std::string name, std::function<Result *()> f) 
		{ 
			CF_SAY("Registered result type " + name, Settings::LogLevels::Info); 
			resultConstructMap[name] = f; 
		};

		/**
		* Start the client.
		* @returns void.
		*/
		void start();

		/**
		* Set the port number of the host the client will connect to.
		* @param portNum The port number to use. Must be in the range [1 .. 65535].
		* @returns void.
		*/
		void setPort(int portNum);

		/**
		* Set the IP address of the host the client will connect to.
		* @param ip The IP address to connect to, in the IPv4 form like "192.168.0.1".
		* @returns void.
		*/
		inline void setIPAddress(std::string ip) { ipAddress = sf::IpAddress(ip); };

		/**
		* Connect to the host.
		* The client must be started via start() and a port number and IP address must be 
		* set via setPort() and setIPAddress() before calling this function, and the client
		* must not already be connected to a server or the new connection attempt will fail.
		* @returns True if connection attempt succeeded, false if not.
		*/
		bool connect();

		/**
		* Disconnect from the host.
		* If the client is not connected, this has no effect.
		* @returns void.
		*/
		void disconnect();

		/**
		* Add a task to the task queue for processing.
		* @param task The task to add.
		* @returns void.
		*/
		void addTaskToQueue(Task *task);

		/**
		* Check if the client is currently connected to a host.
		* @returns True if this client is connected to as host, false if not.
		*/
		inline bool isConnected() { return connected; };

	private:

		//Maximum threads for multi process forking.
		int MAX_THREADS;

		//Socket for the network connection.
		sf::TcpSocket socket;

		//Socket mutex for this client, for locking the socket during use.
		std::mutex socketMutex;

		//Has the host been started?
		std::atomic<bool> started;

		//Is the client connected to a host?
		std::atomic<bool> connected;

		//Port number to connect to.
		int port;

		//Listener object responsible for receiving incoming work packets.
		ClientListener listener{ this };

		//Sender object responsible for sending completed results to host.
		ClientSender sender{ this };

		//IP address to connect to.
		sf::IpAddress ipAddress;

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
		std::map<std::string, std::function<Task *()>> taskConstructMap;

		//Construction map for user defined Results.
		std::map<std::string, std::function<Result *()>> resultConstructMap;

		//Thread used to process task chunks locally on the host.
		std::thread TaskProcessingThread;

		//Should the client tasks processing thread continue to run?
		std::atomic<bool> ProcessTaskThreadRun;

		void ProcessTaskThread();

		/**
		* Check the incomplete results queue for complete result sets.
		* Completed results set that are found are moved to the complete results queue.
		* @returns void.
		*/
		void checkForCompleteResults();
	};
}