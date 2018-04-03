#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include "Settings.h"
#include "DllExport.h"

#define CF_CONSOLE cf::ConsoleMessager::getInstance()

#define CF_SAY(s, l) CF_CONSOLE->say(s, l)

#define CF_THROW(s) throw static_cast<std::string>(s);

namespace cf
{
	/**
	* Console message management class. Prints messages to the console and ensures thread safety 
	* by using a dedicated console mutex while doing so.
	* Singleton class.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL ConsoleMessager
	{

	public:

		/**
		* Create or get static instance.
		* @returns A pointer to the single ConsoleMessager object.
		*/
		inline static class ConsoleMessager *getInstance() { static ConsoleMessager cm; return &cm; };

		/**
		* Print a message to the console.
		* @param s The string to print to the console.
		* @param level The log level of the message.
		* @returns void.
		*/
		inline void say(std::string s, int level)
		{ 
			std::unique_lock<std::mutex> lock(console); 
			//Ignore the message if it is not covered by the current log level setting.
			if (level <= CF_SETTINGS->getLogLevel())
			{
				//Send errors to error output, and all other message types to standard output.
				if (level == Settings::LogLevels::Error)
				{
					std::cerr << s << std::endl;
				}
				else
				{
					std::cout << s << std::endl;
				}
			}
		};

	private:

		/**
		* Default constructor.
		*/
		ConsoleMessager() {};

		/**
		* Default destructor.
		*/
		~ConsoleMessager() {};

		//Mutex to ensure console is only written to by one thread at a time.
		std::mutex console;
	};
}