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
	class DLL ConsoleMessager
	{

	public:
		ConsoleMessager() {};
		~ConsoleMessager() {};

		/**
		* Create or get static instance.
		* @returns A pointer to the single ConsoleMessager object.
		*/
		inline static class ConsoleMessager *getInstance() { static ConsoleMessager cm; return &cm; };

		inline void say(std::string s) { console.lock(); if (CF_SETTINGS->getDebug() == true) std::cout << s << std::endl; console.unlock(); };

	private:

		//Mutex to ensure console is only written to by one thread at a time.
		std::mutex console;
	};
}