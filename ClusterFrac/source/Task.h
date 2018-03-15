#pragma once
#include "DllExport.h"
#include <vector>

namespace cf
{
	class DLL Task
	{
	public:
		Task();
		~Task();

		/**
		* Split this task up as equally as possible in to N chunks, and return
		* a std::vector of pointers to those split tasks.
		* @param count Split the task into this many subtasks.
		* @returns A std::vector of pointers to the new split tasks.
		*/
		virtual std::vector<Task *> Split(int count) const = 0;
	};
}
