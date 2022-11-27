

#include "PhysicsService.h"
#include "..\common_h\data_swizzle.h"


//============================================================================================
//PhysicsService::ErrorStream
//============================================================================================

PhysicsService::ErrorStream::ErrorStream()
{
	enabledWarnings = true;
}

void PhysicsService::ErrorStream::EnableWarnings(bool enable)
{
	enabledWarnings = enable;
}

void PhysicsService::ErrorStream::reportError(NxErrorCode e, const char* message, const char* file, int line)
{
	bool isTraceAsError = false;
	const char * error = "unknown error";	
	switch(e)
	{
	case NXE_INVALID_PARAMETER:
		error = "invalid parameter";
		isTraceAsError = true;
		break;
	case NXE_INVALID_OPERATION:
		error = "invalid operation";
		isTraceAsError = true;
		break;
	case NXE_OUT_OF_MEMORY:
		error = "out of memory";
		isTraceAsError = true;
		break;
	case NXE_DB_INFO:
		if (!enabledWarnings)
			return;
		error = "info";
		break;
	case NXE_DB_WARNING:
		if (!enabledWarnings)
			return;
		error = "warning";
		break;
	}

	if (isTraceAsError)
		api->Error("PhysX error: %s, (\"%s\", %s, %d)", error, message, file, line);
	else
		api->Trace("PhysX error: %s, (\"%s\", %s, %d)", error, message, file, line);
}

NxAssertResponse PhysicsService::ErrorStream::reportAssertViolation(const char * message, const char * file, int line)
{
	__Storm_Assert__(false, file, line, message);
	return NX_AR_BREAKPOINT;
}

void PhysicsService::ErrorStream::print(const char * message)
{
	api->Trace(message);
}


//============================================================================================
//PhysicsService::MemoryReadStream
//============================================================================================

PhysicsService::MemoryReadStream::MemoryReadStream(const void * _source, dword _size)
{
	current = 0;
	sourceSize = _size;
	source = (byte *)_source;
	if(!_source) sourceSize = 0;
}

PhysicsService::MemoryReadStream::~MemoryReadStream()
{
}

NxU8 PhysicsService::MemoryReadStream::readByte() const
{
	NxU8 v;
	return readData(v);
}

NxU16 PhysicsService::MemoryReadStream::readWord() const
{
	NxU16 v;
	readData(v);
	return v;
}

NxU32 PhysicsService::MemoryReadStream::readDword() const
{
	NxU32 v;
	readData(v);
	return v;
}

float PhysicsService::MemoryReadStream::readFloat() const
{
	float v;
	readData(v);
	return v;
}

double PhysicsService::MemoryReadStream::readDouble() const
{
	double v;
	readData(v);
	return v;
}

void PhysicsService::MemoryReadStream::readBuffer(void * buffer, NxU32 size) const
{
	Assert(current + size <= sourceSize);
	memcpy(buffer, source + current, size);
	current += size;
}

template<class T> __forceinline T & PhysicsService::MemoryReadStream::readData(T & v) const
{
	Assert(current + sizeof(T) <= sourceSize);
	//v = *(T *)(source + current);
	//current += sizeof(T);
	byte * ptr = (byte *)&v;
	for(long i = 0; i < sizeof(T); i++)
	{
		*ptr++ = source[current++];
	}
	return v;
}

NxStream & PhysicsService::MemoryReadStream::storeByte(NxU8 b)
{
	Assert(false);
	return *this;
}

NxStream & PhysicsService::MemoryReadStream::storeWord(NxU16 w)
{
	Assert(false);
	return *this;
}

NxStream & PhysicsService::MemoryReadStream::storeDword(NxU32 d)
{
	Assert(false);
	return *this;
}

NxStream & PhysicsService::MemoryReadStream::storeFloat(NxReal f)
{
	Assert(false);
	return *this;
}

NxStream & PhysicsService::MemoryReadStream::storeDouble(NxF64 f)
{
	Assert(false);
	return *this;
}

NxStream & PhysicsService::MemoryReadStream::storeBuffer(const void * buffer, NxU32 size)
{
	Assert(false);
	return *this;
}


//============================================================================================
//PhysicsService::MemoryWriteStream
//============================================================================================

PhysicsService::MemoryWriteStream::MemoryWriteStream() : data(_FL_)
{
}

PhysicsService::MemoryWriteStream::~MemoryWriteStream()
{
}

NxU8 PhysicsService::MemoryWriteStream::readByte() const
{
	Assert(false);
	return 0;
}

NxU16 PhysicsService::MemoryWriteStream::readWord() const
{
	Assert(false);
	return 0;
}

NxU32 PhysicsService::MemoryWriteStream::readDword() const
{
	Assert(false);
	return 0;
}

float PhysicsService::MemoryWriteStream::readFloat() const
{
	Assert(false);
	return 0;
}

double PhysicsService::MemoryWriteStream::readDouble() const
{
	Assert(false);
	return 0;
}

void PhysicsService::MemoryWriteStream::readBuffer(void * buffer, NxU32 size) const
{
	Assert(false);
}


NxStream & PhysicsService::MemoryWriteStream::storeByte(NxU8 b)
{
	return writeData(b);
}

NxStream & PhysicsService::MemoryWriteStream::storeWord(NxU16 w)
{
	return writeData(w);
}

NxStream & PhysicsService::MemoryWriteStream::storeDword(NxU32 d)
{
	return writeData(d);
}

NxStream & PhysicsService::MemoryWriteStream::storeFloat(NxReal f)
{
	return writeData(f);
}

NxStream & PhysicsService::MemoryWriteStream::storeDouble(NxF64 f)
{
	return writeData(f);
}

NxStream & PhysicsService::MemoryWriteStream::storeBuffer(const void * buffer, NxU32 size)
{
	const byte * ptr = (const byte *)buffer;
	for(NxU32 i = 0; i < size; i++)
	{
		data.Add(*ptr++);
	}
	return *this;
}

template<class T> NxStream & PhysicsService::MemoryWriteStream::writeData(const T & v)
{
	const byte * ptr = (const byte *)&v;
	for(long i = 0; i < sizeof(T); i++)
	{
		data.Add(*ptr++);
	}
	return *this;
}

const void * PhysicsService::MemoryWriteStream::Pointer()
{
	if(data > 0) return &data[0];
	return null;
}

dword PhysicsService::MemoryWriteStream::Size()
{
	return data;
}

