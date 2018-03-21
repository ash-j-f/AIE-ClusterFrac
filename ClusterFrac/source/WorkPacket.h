#pragma once
#include "DllExport.h"
#include <SFML\Network.hpp>
#include <zlib.h>

namespace cf
{
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

		WorkPacket();
		WorkPacket(Flag newFlag);
		~WorkPacket();

		void init();

		//Is compression during network send on or off?
		bool compression;

		inline void setFlag(Flag newFlag) { flag = newFlag; };

		/**
		* Clear the packet, and set the packet flag to None.
		* Hides the base class clear() function.
		* @returns void.
		*/
		inline virtual void clear() { static_cast<sf::Packet*>(this)->clear(); flag = None; };

	private:
		sf::Uint8 flag;
		std::vector<Bytef> oCompressionBuffer;
		virtual const void* onSend(std::size_t& size);
		virtual void onReceive(const void* data, std::size_t size);
	};
}