#pragma once
#include <string>
#include <atomic>
#include <list>
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

	private:

		//Has the host been started?
		std::atomic<bool> started;

		//Port number this server is using.
		int port;

		//Task queue.
		std::list<cf::Task *> taskQueue;

		//Results queue.
		std::list<cf::Result *> resultQueue;

		//Mutex for results queues
		std::mutex resultsQueueMutex;

	};
}