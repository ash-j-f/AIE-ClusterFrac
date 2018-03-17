#pragma once
#include "DllExport.h"
#include <SFML\Network.hpp>
#include <zlib.h>

namespace cf
{
	class DLL WorkPacket : public sf::Packet
	{
	private:
		virtual const void* onSend(std::size_t& size);
		virtual void onReceive(const void* data, std::size_t size);
		std::vector<Bytef> oCompressionBuffer;
	};
}