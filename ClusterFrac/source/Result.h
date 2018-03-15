#pragma once
#include "DllExport.h"
#include <vector>

namespace cf
{
	class DLL Result
	{
	public:
		Result();
		~Result();

		/**
		* Return a pointer to a new result object that is a merged version
		* of this result combined with all others in a std::vector.
		* @param others A std::vector of pointers to the other results to merge with this one.
		* @returns A pointer to a single new merged result.
		*/
		virtual Result *Merge(std::vector<Result *> others) const = 0;
	};
}
