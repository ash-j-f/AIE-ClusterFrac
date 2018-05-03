#pragma once
#include "DllExport.h"
#include <vector>
#include <string>
#include <algorithm>
#include <SFML\Network.hpp>
#include "WorkPacket.h"
#include "ConsoleMessager.hpp"

namespace cf
{
	
	/**
	* Result object that stores the results of processing a Task.
	* Pure virtual class, intended to be used as a base class for derived
	* custom Result objects.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class Result
	{

		//Task class needs access to Result private members.
		friend class Task;

	public:

		/**
		* Default constructor.
		*/
		DLL Result();
		
		/**
		* Default destructor.
		* Virtual, as this is used as a base class.
		*/
		DLL virtual ~Result();

		/**
		* Get the type ID of this class.
		* This identifies the base class type during serialisation and deserialisation.
		* This value must be unique among all classes.
		* @returns The type ID of this class as a string.
		*/
		DLL inline virtual std::string getType() const { return "Result"; }

		/**
		* Get the subtype ID of this class.
		* Pure virtual function, which identifies the polymorphic derived class type
		* during serialisation and deserialisation.
		* This value must be unique among all classes.
		* @returns The subtype ID of this class as a string.
		*/
		DLL virtual std::string getSubtype() const = 0;

		/**
		* Get the task part number of this result, realtive to the result set this result
		* belongs to. Task part numbering starts at 0.
		* @returns The task part number of this result.
		*/
		DLL inline int getTaskPartNumber() const { if (taskPartNumberStack.size() == 0) { CF_THROW("Task part number list is empty."); } return taskPartNumberStack.back(); };

		/**
		* Get the total number of task parts for the result set this result belongs to.
		* @returns The total number of task parts in this result set.
		*/
		DLL inline int getCurrentTaskPartsTotal() const { if (taskPartsTotalStack.size() == 0) { CF_THROW("Task part total list is empty."); } return taskPartsTotalStack.back(); }

		/**
		* Get the task ID for this task. The task ID is set when a task is first created.
		* When a task is split, all sub tasks that for a set for one task share a task ID.
		* @returns The task ID for this result.
		*/
		DLL inline unsigned __int64 getInitialTaskID() const { return initialTaskID; };

		/**
		* Merge other results in a std::vector into this result.
		* Merge must include all results in the current set or merge will fail with an error.
		* @param others A std::vector of pointers to the all results in a set to merge with this one.
		* @returns A pointer to a single new merged result.
		*/
		DLL virtual void merge(const std::vector<Result *> others);
	
		/**
		* Serialize this result and store the data in a given packet.
		* @param p The packet to store the data in.
		* @returns void.
		*/
		DLL void serialize(cf::WorkPacket &p) const;

		/**
		* Deserialize this result from data provided by a packet.
		* @param p The packet to retrieve the task data from.
		* @returns void.
		*/
		DLL void deserialize(cf::WorkPacket &p);

		/**
		* Record the time the task this result is associated with was started.
		* @param t The time the task was started, in sf::Time format.
		* @returns void.
		*/
		DLL inline void setHostTimeSent(sf::Time t) { hostTimeSent = t; };

		/**
		* Get the time the task this result is associated with was started.
		* @returns The time the task this result is associated with was started, 
		* in sf::Time format.
		*/
		DLL inline sf::Time getHostTimeSent() const { return hostTimeSent; };

		/**
		* Set the time the task this result was associated with was completed.
		* @param t The time the task this result was associated with was completed,
		* in sf::Time format.
		*/
		DLL inline void setHostTimeFinished(sf::Time t) { hostTimeFinished = t; };

		/**
		* Get the time the task this result was associated with was completed.
		* @returns The time the task this result was associated with was completed, 
		* in sf::Time format.
		*/
		DLL inline sf::Time getHostTimeFinished() const { return hostTimeFinished; };

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

		//Host time this task was sent to the client.
		//Only used by the host.
		sf::Time hostTimeSent;

		//Time this result was accepted as finished by the host.
		//Only used by the host.
		sf::Time hostTimeFinished;

		/**
		* Merge other results in a std::vector into this result.
		* Merge must include all results in the current set or merge will fail with an error.
		* @param others A std::vector of pointers to the all results in a set to merge with this one.
		* @returns void.
		*/
		virtual void mergeLocal(const std::vector<Result *> others) = 0;

		/**
		* Serialize this result and store the data in a given packet.
		* @param p The packet to store the data in.
		* @returns void.
		*/
		virtual void serializeLocal(cf::WorkPacket &p) const = 0;

		/**
		* Deserialize this result from data provided by a packet.
		* @param p The packet to retrieve the task data from.
		* @returns void.
		*/
		virtual void deserializeLocal(cf::WorkPacket &p) = 0;

	};
}
