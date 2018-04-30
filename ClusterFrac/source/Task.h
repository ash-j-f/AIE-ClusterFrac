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
	/**
	* Task object that stores a task to be completed by a client.
	* Pure virtual class, intended to be used as a base class for derived
	* custom Task objects.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL Task
	{
	public:

		/**
		* Default constructor.
		*/
		Task();

		/**
		* Default destructor.
		* Virtual, as this is used as a base class.
		*/
		virtual ~Task();

		//Which node types can this task be run on?
		//The task distributor will only run the task on the chosen node type.
		//If "host as client" is not enabled, type "Local" will be treated as "Remote".
		//Local - Local CPU.
		//Remote - CPU on network.
		//Any - No preference.
		enum NodeTargetTypes { Local = 0, Remote = 1, Any = 2 };

		//Is this task allowed to be split between nodes?
		//Some tasks may perform better when sent to a single node.
		bool allowNodeTaskSplit;

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
		* Get the task part number of this task, realtive to the task set this task
		* belongs to. Task part numbering starts at 0.
		* @returns The task part number of this result.
		*/
		inline int getTaskPartNumber() const { if (taskPartNumberStack.size() == 0) { CF_THROW("Task part number list is empty."); } return taskPartNumberStack.back(); };

		/**
		* Get the total number of task parts for the task set this task belongs to.
		* @returns The total number of task parts in this result set.
		*/
		inline int getCurrentTaskPartsTotal() const { if (taskPartsTotalStack.size() == 0) { CF_THROW("Task part total list is empty."); } return taskPartsTotalStack.back(); }

		/**
		* Get the task ID for this task. The task ID is set when a task is first created.
		* When a task is split, all sub tasks that for a set for one task share a task ID.
		* @returns The task ID for this task.
		*/
		inline unsigned __int64 getInitialTaskID() const { if (initialTaskID == 0) { CF_THROW("This task has no ID."); } return initialTaskID; };

		/**
		* Split this task up as equally as possible in to N chunks, and return
		* a std::vector of pointers to those split tasks.
		* @param count Split the task into this many subtasks.
		* @returns A std::vector of pointers to the new split tasks.
		*/
		std::vector<Task *> split(int count) const;

		/**
		* Serialize this task and store the data in a given packet.
		* @param p The packet to store the data in.
		* @returns void.
		*/
		void serialize(WorkPacket &p) const;

		/**
		* Deserialize this task from data provided by a packet.
		* @param p The packet to retrieve the task data from.
		* @returns void.
		*/
		void deserialize(WorkPacket &p);

		/**
		* Run the task and produce a results object.
		* @returns A pointer to the new results object.
		*/
		Result *run() const;

		/**
		* Record the time the task was started.
		* @param t The time the task was started, in sf::Time format.
		* @returns void.
		*/
		inline void setHostTimeSent(sf::Time t) { hostTimeSent = t; };

		/**
		* Get the time the task was started.
		* @returns The time the task was started, in sf::Time format.
		*/
		inline sf::Time getHostTimeSent() const { return hostTimeSent; };

		/**
		* Get the maximum time this task may take to execute before being cancelled and restarted.
		* Time is in milliseconds.
		* @return The maximum time this task may take to execute before being cancelled and restarted, in milliseconds.
		*/
		inline float getMaxTaskTimeMilliseconds() const { return maxTaskTimeMilliseconds; };

		/**
		* Assign this task a unique ID, if it doesn't already have one.
		* This is not done automatically when a task is created.
		* It should only be called for tasks that are the initial top-level 
		* task and not on a task-part that will have its ID overwritten by
		* the parent anyway.
		* @returns void.
		*/
		void assignID() { if (initialTaskID == 0) initialTaskID = CF_ID->getNextTaskID();  };

		/**
		* Get the client node type this task may be executed on.
		* @returns The node type this task may be executed on.
		*/
		NodeTargetTypes getNodeTargetType() { return (NodeTargetTypes)nodeTargetType; };

		/**
		* Set the client node type this task may be executed on.
		* For mapping client node types to integer, see NodeTargetTypes enum.
		* @param newNodeTargetType The node target type as a sf::Uint8.
		* @returns void.
		*/
		void setNodeTargetType(sf::Uint8 newNodeTargetType) { nodeTargetType = newNodeTargetType; };
		
		/**
		* Set the client node type this task may be executed on.
		* @param newNodeTargetType The node target type as an entry from NodeTargetTypes enum.
		* @returns void.
		*/
		void setNodeTargetType(NodeTargetTypes newNodeTargetType) { nodeTargetType = (sf::Uint8)newNodeTargetType; };

	private:

		//Which node type does this task prefer to be run on?
		//See NodeTargetTypes enum.
		sf::Uint8 nodeTargetType;

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

		//Maximum time in milliseconds that a client is allowed to spend on this task
		//(or task part) before the host will cancel the request and try again.
		//This time is from the host point of view, elapsed since task send started
		//so it includes all overheads like network and CPU time.
		//This value is only used for tasks or task parts sent to the client.
		float maxTaskTimeMilliseconds;

		//Host time this task was sent to the client. Used to calculate elapsed time
		//since the task was sent when checking expiry relative to maxTaskTimeMilliseconds.
		//This value is only used for tasks or task parts sent to the client.
		sf::Time hostTimeSent;

		/**
		* Split this task up as equally as possible in to N chunks, and return
		* a std::vector of pointers to those split tasks.
		* @param count Split the task into this many subtasks.
		* @returns A std::vector of pointers to the new split tasks.
		*/
		virtual std::vector<Task *> splitLocal(unsigned int count) const = 0;

		/**
		* Serialize this task and store the data in a given packet.
		* @param p The packet to store the data in.
		* @returns void.
		*/
		virtual void serializeLocal(WorkPacket &p) const = 0;

		/**
		* Deserialize this task from data provided by a packet.
		* @param p The packet to retrieve the task data from.
		* @returns void.
		*/
		virtual void deserializeLocal(WorkPacket &p) = 0;

		/**
		* Run the task and produce a results object.
		* @returns A pointer to the new results object.
		*/
		virtual Result *runLocal() const = 0;

	};
}
