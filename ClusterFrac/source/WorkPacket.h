#pragma once
#include "DllExport.h"
#include <SFML\Network.hpp>
#include <zlib.h>
#include "ConsoleMessager.hpp"

namespace cf
{
	/**
	* Work packet class based on SFML Packet class.
	* Work packets are serialised data designed to be sent over a network.
	* Extends the SFML Packet class by adding packet flag type and data compression.
	* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
	*/
	class WorkPacket : public sf::Packet
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
		DLL WorkPacket();

		/**
		* Constructor with packet flag specified.
		* @param newFlag Set the packet flag.
		*/
		DLL WorkPacket(Flag newFlag);

		/**
		* Default constructor.
		*/
		DLL ~WorkPacket();

		/**
		* Initialise the work packet.
		* @returns void.
		*/
		DLL void init();

		/*
		* Set the work packet flag.
		* Flags are used to identify the packet type when decoding.
		* @returns void.
		*/
		DLL void setFlag(Flag newFlag);

		/**
		* Get the work packet flag.
		* Flags are used to identify the packet type when decoding.
		* @returns The work packet's identifying flag.
		*/
		DLL inline Flag getFlag() const { return static_cast<Flag>(flag); }

		/**
		* Clear the packet, and set the packet flag to None.
		* Hides the base class clear() function.
		* @returns void.
		*/
		DLL inline virtual void clear() { static_cast<sf::Packet*>(this)->clear(); flag = None; };

		/**
		* Turn compression during send/receive on or off.
		* NOTE: Send/receive will FAIL if the compression status does not match for the work packets
		* used by the host and client when sneding/receiving.
		* @param state True to enable compression, false to disable.
		*/
		DLL inline void setCompression(bool state) { compression = state; };

	private:

		//Is compression during network sending turned on or off?
		bool compression;

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