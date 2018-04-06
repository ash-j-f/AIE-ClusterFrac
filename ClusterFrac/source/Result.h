#pragma once
#include "DllExport.h"
#include <vector>
#include <string>
#include <algorithm>
#include <SFML\Network.hpp>
#include "WorkPacket.h"

namespace cf
{
	class DLL Result
	{

		//Task class needs access to Result private members.
		friend class Task;

	public:

		Result();
		
		virtual ~Result();

		/**
		* Get the type ID of this class.
		* This identifies the base class type during serialisation and deserialisation.
		* This value must be unique among all classes.
		* @returns The type ID of this class as a string.
		*/
		inline virtual std::string getType() const { return "Result"; }

		/**
		* Get the subtype ID of this class.
		* Pure virtual function, which identifies the polymorphic derived class type
		* during serialisation and deserialisation.
		* This value must be unique among all classes.
		* @returns The subtype ID of this class as a string.
		*/
		virtual std::string getSubtype() const = 0;

		inline int getTaskPartNumber() const { if (taskPartNumberStack.size() == 0) { CF_THROW("Task part number list is empty."); } return taskPartNumberStack.back(); };

		inline int getCurrentTaskPartsTotal() const { if (taskPartsTotalStack.size() == 0) { CF_THROW("Task part total list is empty."); } return taskPartsTotalStack.back(); }

		inline unsigned __int64 getInitialTaskID() const { return initialTaskID; };

		/**
		* Merge other results in a std::vector into this result.
		* Merge must include all results in the current set or merge will fail with an error.
		* @param others A std::vector of pointers to the all results in a set to merge with this one.
		* @returns A pointer to a single new merged result.
		*/
		virtual void merge(const std::vector<Result *> others);
	
		void serialize(cf::WorkPacket &p) const;

		void deserialize(cf::WorkPacket &p);

	private:

		//The ID of the initial task before it was split.
		sf::Uint64 initialTaskID;

		//Part number of this task after last split.
		//Stored as a stack of all values up the task tree from this point
		//to allow growing and unrolling of the stack as tasks are split and 
		//results are merged.
		std::vector<sf::Uint32> taskPartNumberStack;

		//Total parts since last split.
		//Stored as a stack of all values up the task tree from this point
		//to allow growing and unrolling of the stack as tasks are split and 
		//results are merged.
		std::vector<sf::Uint32> taskPartsTotalStack;

		virtual void mergeLocal(const std::vector<Result *> others) = 0;

		virtual void serializeLocal(cf::WorkPacket &p) const = 0;

		virtual void deserializeLocal(cf::WorkPacket &p) = 0;

	};
}
