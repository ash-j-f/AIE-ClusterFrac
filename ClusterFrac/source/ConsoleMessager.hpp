#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include "Settings.h"
#include "DllExport.h"

#ifdef _WIN32
	#include <Windows.h>
#endif

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
		ConsoleMessager() 
		{
#ifdef _WIN32
			
			//Disable edit mode in the console window. Edit mode causes the application to pause
			//and output to break if a user clicks in the console window.

			consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
			DWORD consoleMode;

			// get current console mode
			if (!GetConsoleMode(consoleHandle, &consoleMode))
			{
				// Error: Unable to get console mode.
				std::cerr << "Error getting console mode." << std::endl;
				return;
			}

			originalConsoleMode = consoleMode;

			// Clear the quick edit bit in the mode flags
	
			consoleMode &= ~ENABLE_QUICK_EDIT_MODE;
			consoleMode &= ~ENABLE_MOUSE_INPUT;

			// set the new mode
			if (!SetConsoleMode(consoleHandle, consoleMode))
			{
				// ERROR: Unable to set console mode
				std::cerr << "Error setting console mode." << std::endl;
			}
#endif
		};

		/**
		* Default destructor.
		*/
		~ConsoleMessager() 
		{
#ifdef _WIN32
			//Restore original console mode options.
			if (!SetConsoleMode(consoleHandle, originalConsoleMode))
			{
				// ERROR: Unable to set console mode
				std::cerr << "Error setting console mode." << std::endl;
			}
#endif
		};

		//Mutex to ensure console is only written to by one thread at a time.
		std::mutex console;

#ifdef _WIN32
		//Console input handle.
		HANDLE consoleHandle;

		//Console mode settings detected on startup.
		DWORD originalConsoleMode;
#endif

	};
}