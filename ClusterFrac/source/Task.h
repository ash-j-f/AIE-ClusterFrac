#pragma once
#include "DllExport.h"
#include <vector>
#include <string>
#include <SFML\Network.hpp>

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

		/**
		* Split this task up as equally as possible in to N chunks, and return
		* a std::vector of pointers to those split tasks.
		* @param count Split the task into this many subtasks.
		* @returns A std::vector of pointers to the new split tasks.
		*/
		virtual std::vector<Task *> split(int count) const = 0;

		inline void serialize(sf::Packet &p) { p << getType(); serializeLocal(p); };

		inline void deserialize(sf::Packet &p) { deserializeLocal(p); };

	private:

		virtual void serializeLocal(sf::Packet &p) = 0;

		virtual void deserializeLocal(sf::Packet &p) = 0;

	};
}
