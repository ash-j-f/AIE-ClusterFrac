#include "WorkPacket.h"

namespace cf
{
	WorkPacket::WorkPacket()
	{

		//Packets have no flag by default.
		flag = None;

		init();
	}

	WorkPacket::WorkPacket(Flag newFlag)
	{
		//Set flag.
		setFlag(newFlag);

		init();
	}

	WorkPacket::~WorkPacket()
	{
	}

	void WorkPacket::init()
	{
		//Compression default status.
		compression = false;
	}

	DLL void WorkPacket::setFlag(Flag newFlag)
	{
		flag = newFlag;
	}

	const void * WorkPacket::onSend(std::size_t & size)
	{
		//Append flag to data stream.
		*this << flag;

		const void *tmpData;

		if (compression)
		{

			// Cast the data to a bytef pointer
			const Bytef* srcData = static_cast<const Bytef*>(getData());

			// Get the size of the packet to send
			size_t tmpSize = getDataSize();

			//Zlib by default only supports 32 bits max for the size integer.
			if (tmpSize > ULONG_MAX) CF_THROW("Data too large to compress using Zlib.");

			uLong srcSize = (uLong)tmpSize;

			//Compute the size of the compressed data
			uLong dstSize = compressBound(srcSize);

			//Resize the vector to accomodate the compressed data,
			//plus four bytes for our uncompressed size
			oCompressionBuffer.resize(dstSize + 4);

			//Store srcSize as the first four bytes of the buffer
			oCompressionBuffer[0] = srcSize & 0xFF;
			oCompressionBuffer[1] = (srcSize >> 8) & 0xFF;
			oCompressionBuffer[2] = (srcSize >> 16) & 0xFF;
			oCompressionBuffer[3] = (srcSize >> 24) & 0xFF;

			//Compress the data into the rest of the buffer
			compress(oCompressionBuffer.data() + 4, &dstSize, srcData, srcSize);

			//Set the size to the compressed size plus
			//the four bytes for the size marker
			size = (dstSize + 4);

			tmpData = oCompressionBuffer.data();

		}
		else
		{
			//Skip compression.
			size = getDataSize();
			tmpData = getData();
		}

		//Return data to send
		return tmpData;
	}

	void WorkPacket::onReceive(const void *data, std::size_t size)
	{
		if (compression)
		{
			//Cast the data to Bytef*, the format zlib deals with
			const Bytef* srcData = static_cast<const Bytef*>(data);

			//Extract the uncompressed data size from the first four
			//bytes in the packet so we can use it for the buffer
			//Use SFML sf::Uint32 to ensure byte order and size is sane across different platforms.
			sf::Uint32 uncompressedSize = srcData[3] << 24 | srcData[2] << 16 | srcData[1] << 8 | srcData[0];

			//Resize the vector to accomodate the uncompressed data
			oCompressionBuffer.resize(uncompressedSize);

			//Declare a variable for the destination size
			uLong dstSize;

			//Uncompress the data (skip the first four bytes which were the size data)
			uncompress(oCompressionBuffer.data(), &dstSize, (srcData + 4), (uLong)size - 4);

			//Assert that the uncompressed size is the same as the
			//size we were sent for the buffer
			if (dstSize != uncompressedSize) CF_THROW("Size mismatch during data decompression. Are compression options set correctly on host and client?");

			//Retrieve the packet flag data.
			std::size_t soFlag = sizeof flag;
			std::memcpy(&flag, oCompressionBuffer.data() + dstSize - soFlag, soFlag);

			//Append data to the packet.
			append(oCompressionBuffer.data(), dstSize - soFlag);

			oCompressionBuffer.clear();
			oCompressionBuffer.resize(0);
		}
		else
		{
			//Skip decompression.

			//Retrieve the packet flag data.
			std::size_t soFlag = sizeof flag;
			std::memcpy(&flag, (Bytef*)data + size - soFlag, soFlag);

			//Append data to the packet.
			append(data, size - soFlag);
		}
	}
}