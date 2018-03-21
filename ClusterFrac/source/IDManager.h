#pragma once
#pragma once
#include "DllExport.h"

#define CF_ID cf::IDManager::getInstance()

namespace cf
{

	/**
	* ID management class.
	* Singleton class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL IDManager
	{

	public:

		/**
		* Create or get static instance.
		* @returns A pointer to the single Settings object.
		*/
		static class IDManager *getInstance();

		/**
		* Get and reserve the next available client ID, and then increment the internal next client ID counter.
		* @returns The next available client ID.
		*/
		inline unsigned int getNextClientID() { return nextClientID++; }

		/**
		* Get and reserve the next available task ID, and then increment the internal next task ID counter.
		* @returns The next available task ID.
		*/
		inline unsigned int getNextTaskID() { return nextTaskID++; }

	private:

		/**
		* Default constructor.
		*/
		IDManager();

		/**
		* Default destructor.
		*/
		~IDManager();

		//Next client ID to use.
		int nextClientID;

		//Next task ID to use.
		int nextTaskID;
	};
}