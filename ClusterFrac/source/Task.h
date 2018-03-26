#pragma once
#include "DllExport.h"
#include <list>
#include <vector>
#include <string>
#include <SFML\Network.hpp>
#include "WorkPacket.h"
#include "Result.h"
#include "IDManager.h"

namespace cf
{
	class DLL Task
	{
	public:

		Task();

		virtual ~Task();

		/**
		* Get the type ID of this class.
		* This identifies the base class type during serialisation and deserialisation.
		* This value must be unique among all classes.
		* @returns The type ID of this class as a string.
		*/
		inline virtual std::string getType() const { return "Task"; }

		/**
		* Get the subtype ID of this class.
		* Pure virtual function, which identifies the polymorphic derived class type
		* during serialisation and deserialisation.
		* This value must be unique among all classes.
		* @returns The subtype ID of this class as a string.
		*/
		virtual std::string getSubtype() const = 0;

		inline int getTaskPartNumber() const { if (taskPartNumberStack.size() == 0) { throw "Task part number list is empty."; } return taskPartNumberStack.back(); };

		inline int getCurrentTaskPartsTotal() const { if (taskPartsTotalStack.size() == 0) { throw "Task part total list is empty."; } return taskPartsTotalStack.back(); }

		inline int getInitialTaskID() const { return initialTaskID; };

		std::vector<Task *> split(int count) const;

		void serialize(WorkPacket &p) const;

		void deserialize(WorkPacket &p);

		Result *run() const;

	private:

		/**
		* Split this task up as equally as possible in to N chunks, and return
		* a std::vector of pointers to those split tasks.
		* @param count Split the task into this many subtasks.
		* @returns A std::vector of pointers to the new split tasks.
		*/
		virtual std::vector<Task *> splitLocal(int count) const = 0;

		virtual void serializeLocal(WorkPacket &p) const = 0;

		virtual void deserializeLocal(WorkPacket &p) = 0;
		
		virtual Result *runLocal() const = 0;

		//The ID of the initial task before it was split.
		sf::Uint32 initialTaskID;

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

	};
}
