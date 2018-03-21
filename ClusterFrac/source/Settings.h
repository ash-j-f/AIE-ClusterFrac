#pragma once
#include "DllExport.h"

#define CF_SETTINGS cf::Settings::getInstance()

namespace cf
{

	class DLL Settings
	{

	public:

		Settings();

		~Settings();

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

		//The current debug mode status.
		bool debug;

	};
}