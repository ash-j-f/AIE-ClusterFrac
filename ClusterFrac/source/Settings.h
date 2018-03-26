#pragma once
#include "DllExport.h"

#define CF_SETTINGS cf::Settings::getInstance()

namespace cf
{

	/**
	* Settings management class.
	* Singleton class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL Settings
	{

	public:

		//Available debug levels.
		//3 - Debug
		//2 - Info
		//1 - Error
		enum LogLevels { Error = 1, Info = 2, Debug = 3 };

		/**
		* Create or get static instance.
		* @returns A pointer to the single Settings object.
		*/
		static class Settings *getInstance();

		/**
		* Get the log level.
		* @returns The current log level.
		*/
		int getLogLevel() { return logLevel; }

		/**
		* Set the log level chosen from the LogLevels enum.
		* Error = Show only errors. 
		* Info = show errors and info messages.
		* Debug = Show all messages including detailed debug messages.
		* @params level The chosen log level from the LogLevels enum.
		* @returns void.
		*/
		void setLogLevel(LogLevels level) { logLevel = level; };

	private:

		/**
		* Default constructor.
		*/
		Settings();

		/**
		* Default destructor.
		*/
		~Settings();
		
		//The current log level.
		LogLevels logLevel;

	};
}