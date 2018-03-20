#pragma once

#define SETTINGS cf::Settings::getInstance()

namespace cf
{

	class Settings
	{

	public:

		Settings();

		~Settings();

		/**
		* Create or get static instance.
		* @returns A pointer to the single Settings object.
		*/
		static class Settings *getInstance();

		bool debug;

	};
}