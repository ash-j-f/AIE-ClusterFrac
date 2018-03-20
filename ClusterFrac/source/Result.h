#pragma once
#include "DllExport.h"
#include <vector>
#include <string>
#include <SFML\Network.hpp>
#include "WorkPacket.h"

namespace cf
{
	class DLL Result
	{
	public:
		Result();
		~Result();

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

		/**
		* Return a pointer to a new result object that is a merged version
		* of this result combined with all others in a std::vector.
		* @param others A std::vector of pointers to the other results to merge with this one.
		* @returns A pointer to a single new merged result.
		*/
		virtual Result *merge(const std::vector<Result *> others) const = 0;
	
		inline void serialize(cf::WorkPacket &p) { p << getType(); p << getSubtype(); serializeLocal(p); };

		inline void deserialize(cf::WorkPacket &p) { deserializeLocal(p); };

	private:

		virtual void serializeLocal(cf::WorkPacket &p) = 0;

		virtual void deserializeLocal(cf::WorkPacket &p) = 0;

	};
}
