#pragma once
#include "DllExport.h"
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

		~Task();

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

		std::vector<Task *> split(int count);

		inline void serialize(WorkPacket &p) { p << getType(); p << getSubtype(); serializeLocal(p); };

		inline void deserialize(WorkPacket &p) { deserializeLocal(p); };

		virtual Result *run() = 0;

	private:

		/**
		* Split this task up as equally as possible in to N chunks, and return
		* a std::vector of pointers to those split tasks.
		* @param count Split the task into this many subtasks.
		* @returns A std::vector of pointers to the new split tasks.
		*/
		virtual std::vector<Task *> splitLocal(int count) const = 0;

		virtual void serializeLocal(WorkPacket &p) = 0;

		virtual void deserializeLocal(WorkPacket &p) = 0;

		//The ID of this task.
		int taskID;
		
		//The ID of the initial task before it was split.
		int initialTaskID;

		//Part number of this task relative to initial task since last split.
		int taskPartNumber;

		//Total parts relative to initial task since last split.
		int taskPartsTotal;

	};
}
