#pragma once
#include "DllExport.h"
#include <SFML\Network.hpp>
#include <zlib.h>

namespace cf
{
	class DLL WorkPacket : public sf::Packet
	{
	public:
		WorkPacket();
		~WorkPacket();

		//Is compression during network send on or off?
		bool compression;

	private:
		std::vector<Bytef> oCompressionBuffer;
		//virtual const void* onSend(std::size_t& size);
		//virtual void onReceive(const void* data, std::size_t size);
	};
}