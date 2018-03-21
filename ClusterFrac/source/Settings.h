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

		/**
		* Create or get static instance.
		* @returns A pointer to the single Settings object.
		*/
		static class Settings *getInstance();

		/**
		* Get the debug mode status.
		* @returns True if debug mode is on, false if not.
		*/
		bool getDebug() { return debug; }

		/**
		* Set the debug mode status.
		* @params state The debug mode state to set. True is on, false is off.
		* @returns void.
		*/
		void setDebug(bool state) { debug = state; };

	private:

		/**
		* Default constructor.
		*/
		Settings();

		/**
		* Default destructor.
		*/
		~Settings();

		//The current debug mode status.
		bool debug;

	};
}