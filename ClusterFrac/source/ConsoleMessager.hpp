#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include "Settings.h"
#include "DllExport.h"

#define CF_CONSOLE cf::ConsoleMessager::getInstance()

#define CF_SAY(x) CF_CONSOLE->say(x)

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
		* @returns void.
		*/
		inline void say(std::string s) { console.lock(); if (CF_SETTINGS->getDebug() == true) std::cout << s << std::endl; console.unlock(); };

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