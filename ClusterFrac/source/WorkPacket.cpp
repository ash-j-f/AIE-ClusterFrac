#include "WorkPacket.h"

cf::WorkPacket::WorkPacket()
{
	//Compression default status.
	compression = false;
}

cf::WorkPacket::~WorkPacket()
{
}

const void * cf::WorkPacket::onSend(std::size_t & size)
{
	const void *tmpData;

	if (compression)
	{

		// Cast the data to a bytef pointer
		const Bytef* srcData = static_cast<const Bytef*>(getData());

		// Get the size of the packet to send
		uLong srcSize = (uLong)getDataSize();

		// Compute the size of the compressed data
		uLong dstSize = compressBound(srcSize);

		// Resize the vector to accomodate the compressed data,
		// plus four bytes for our uncompressed size
		oCompressionBuffer.resize(dstSize + 4);

		// Store srcSize as the first four bytes of the buffer
		oCompressionBuffer[0] = srcSize & 0xFF;
		oCompressionBuffer[1] = (srcSize >> 8) & 0xFF;
		oCompressionBuffer[2] = (srcSize >> 16) & 0xFF;
		oCompressionBuffer[3] = (srcSize >> 24) & 0xFF;

		// Compress the data into the rest of the buffer
		compress(oCompressionBuffer.data() + 4, &dstSize, srcData, srcSize);

		// Set the size to the compressed size plus
		// the four bytes for the size marker
		size = (dstSize + 4);

		tmpData = oCompressionBuffer.data();

	}
	else
	{
		size = getDataSize();
		tmpData = getData();
	}

	// Return data to send
	return tmpData;
}

void cf::WorkPacket::onReceive(const void * data, std::size_t size)
{
	if (compression)
	{
		// Cast the data to Bytef*, the format zlib deals with
		const Bytef* srcData = static_cast<const Bytef*>(data);

		// Extract the uncompressed data size from the first four
		// bytes in the packet so we can use it for the buffer
		sf::Uint32 uncompressedSize = srcData[3] << 24 | srcData[2] << 16 | srcData[1] << 8 | srcData[0];

		// Resize the vector to accomodate the uncompressed data
		oCompressionBuffer.resize(uncompressedSize);

		// Declare a variable for the destination size
		uLong dstSize;

		// Uncompress the data (skip the first four bytes which were the size data)
		uncompress(oCompressionBuffer.data(), &dstSize, (srcData + 4), (uLong)size - 4);

		// Assert that the uncompressed size is the same as the
		// size we were sent for the buffer
		//assert(dstSize == uncompressedSize);

		// Append data to the packet
		append(oCompressionBuffer.data(), dstSize);
	}
	else
	{
		append(data, size);
	}
}
