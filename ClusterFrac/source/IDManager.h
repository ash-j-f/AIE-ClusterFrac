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
		inline unsigned __int64 getNextClientID() { return nextClientID++; }

		/**
		* Get and reserve the next available task ID, and then increment the internal next task ID counter.
		* @returns The next available task ID.
		*/
		inline unsigned __int64 getNextTaskID() { return nextTaskID++; }

		/**
		* Get and reserve the next available result ID, and then increment the internal next result ID counter.
		* @returns The next available result ID.
		*/
		inline unsigned __int64 getNextResultID() { return nextResultID++; }

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
		unsigned __int64 nextClientID;

		//Next task ID to use.
		unsigned __int64 nextTaskID;

		//Next result ID to use.
		unsigned __int64 nextResultID;

	};
}