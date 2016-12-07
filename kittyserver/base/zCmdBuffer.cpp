#include "zCmdBuffer.h"

bool zCmdBuffer::write(const void* data, const unsigned int size)
{
	if (!this->tailAvailable(size))
		return false;

	memcpy(&this->buffer[this->tailPtr], data, size);
	this->tailPtr += size;
	return true;
}

zCmdBuffer& zCmdBuffer::operator = (const zCmdBuffer& other)
{
	if (&other == this)
		return *this;

	this->headPtr = other.headPtr;
	this->tailPtr = other.tailPtr;
	memcpy(&this->buffer[headPtr], &other.buffer[other.headPtr], this->tailPtr - this->headPtr);

	return *this;
}
