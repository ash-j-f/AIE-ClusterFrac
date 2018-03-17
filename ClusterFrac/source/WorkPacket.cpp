#include "WorkPacket.h"

const void * cf::WorkPacket::onSend(std::size_t & size)
{
	// We only support data with a maximum size of
	// an unsigned short (so the size can be sent
	// in the first two bytes of the packet)
	//assert(size <= 65535);

	// Cast the data to a bytef pointer
	const Bytef* srcData = static_cast<const Bytef*>(getData());

	// Get the size of the packet to send
	uLong srcSize = getDataSize();

	// Compute the size of the compressed data
	uLong dstSize = compressBound(srcSize);

	// Resize the vector to accomodate the compressed data,
	// plus two bytes for our uncompressed size
	oCompressionBuffer.resize(dstSize + 2);

	// Take the first 8 bytes of srcSize
	oCompressionBuffer[0] = srcSize & 0xFF;

	// And the second 8 bytes
	oCompressionBuffer[1] = (srcSize >> 8) & 0xFF;

	// Compress the data into the rest of the buffer
	compress(oCompressionBuffer.data() + 2, &dstSize, srcData, srcSize);

	// Set the size to the compressed size plus
	// two bytes for the size marker
	size = (dstSize + 2);

	// Return data to send
	return oCompressionBuffer.data();
}

void cf::WorkPacket::onReceive(const void * data, std::size_t size)
{
	// Cast the data to Bytef*, the format zlib deals with
	const Bytef* srcData = static_cast<const Bytef*>(data);

	// Extract the uncompressed data size from the first two
	// bytes in the packet so we can use it for the buffer
	sf::Uint16 uncompressedSize = srcData[1] << 8 | srcData[0];

	// Resize the vector to accomodate the uncompressed data
	oCompressionBuffer.resize(uncompressedSize);

	// Declare a variable for the destination size
	uLong dstSize;

	// Uncompress the data (remove the first two bytes)
	uncompress(oCompressionBuffer.data(), &dstSize, (srcData + 2), size - 2);

	// Assert that the uncompressed size is the same as the
	// size we were sent for the buffer
	//assert(dstSize == uncompressedSize);

	// Append it to the packet
	append(oCompressionBuffer.data(), dstSize);
}
