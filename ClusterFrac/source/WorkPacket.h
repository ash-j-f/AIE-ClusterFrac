#pragma once
#include "DllExport.h"
#include <SFML\Network.hpp>
#include <zlib.h>

namespace cf
{
	/**
	* Work packet class based on SFML Packet class.
	* Work packets are serialised data designed to be sent over a network.
	* Extends the SFML Packet class by adding packet flag type and data compression.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class DLL WorkPacket : public sf::Packet
	{
	public:
		
		//The packet type.
		enum Flag
		{
			None,
			Task,
			Result
		};

		/**
		* Default constructor.
		*/
		WorkPacket();

		/**
		* Constructor with packet flag specified.
		* @param newFlag Set the packet flag.
		*/
		WorkPacket(Flag newFlag);

		/**
		* Default constructor.
		*/
		~WorkPacket();

		//Is compression during network send on or off?
		bool compression;

		/**
		* Initialise the work packet.
		* @returns void.
		*/
		void init();

		/*
		* Set the work packet flag.
		* Flags are used to identify the packet type when decoding.
		* @returns void.
		*/
		inline void setFlag(Flag newFlag) { flag = newFlag; };

		/**
		* Get the work packet flag.
		* Flags are used to identify the packet type when decoding.
		* @returns The work packet's identifying flag.
		*/
		inline Flag getFlag() const { return static_cast<Flag>(flag); }

		/**
		* Clear the packet, and set the packet flag to None.
		* Hides the base class clear() function.
		* @returns void.
		*/
		inline virtual void clear() { static_cast<sf::Packet*>(this)->clear(); flag = None; };

	private:

		//The work packet's identifying type flag. Stored as a Uint8 for 
		//maximum cross platform and network compatibility.
		sf::Uint8 flag;

		//Data buffer to use during compression.
		std::vector<Bytef> oCompressionBuffer;

		/**
		* Actions to perform before the work packet is sent across the network.
		* @returns void.
		*/
		virtual const void* onSend(std::size_t& size);

		/**
		* Actions to perform after the work packet is received from the network.
		* @returns void.
		*/
		virtual void onReceive(const void* data, std::size_t size);
	};
}