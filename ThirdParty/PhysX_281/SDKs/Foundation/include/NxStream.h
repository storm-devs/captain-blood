#ifndef NX_FOUNDATION_NXSTREAM
#define NX_FOUNDATION_NXSTREAM
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup foundation
  @{
*/

/**
\brief Callback class for data serialization.

The user needs to supply an NxStream implementation to a number of methods to allow the SDK to read or write
chunks of binary data. This allows flexibility for the source/destination of the data. For example the NxStream
could store data in a file, memory buffer or custom file format.

\note It is the users resposibility to ensure that the data is written to the appropriate offset. NxStream does not 
expose any seeking functionality.

<h3>Example</h3>

\include NxStream_Example.cpp

@see NxPhysicsSDK.createTriangleMesh() NxPhysicsSDK.createConvexMesh() NxTriangleMesh.load()

*/
class NxStream
	{
	public:
	/**
	\brief Empty constructor.
	*/

								NxStream()				{}
	/**
	\brief Virtual destructor.
	*/

	virtual						~NxStream()				{}

	// Loading API
	
	/**
	\brief Called to read a single unsigned byte(8 bits)

	\return Byte read.
	*/
	virtual		NxU8			readByte()								const	= 0;
	
	/**
	\brief Called to read a single unsigned word(16 bits)

	\return Word read.
	*/
	virtual		NxU16			readWord()								const	= 0;
	
	/**
	\brief Called to read a single unsigned dword(32 bits)

	\return DWord read.
	*/
	virtual		NxU32			readDword()								const	= 0;
	
	/**
	\brief Called to read a single precision floating point value(32 bits)

	\return Floating point value read.
	*/
	virtual		NxF32			readFloat()								const	= 0;
	
	/**
	\brief Called to read a double precision floating point value(64 bits)

	\return Floating point value read.
	*/
	virtual		NxF64			readDouble()							const	= 0;
	
	/**
	\brief Called to read a number of bytes.

	\param[out] buffer Buffer to read bytes into, must be at least size bytes in size.
	\param[in] size The size of the buffer in bytes.
	*/
	virtual		void			readBuffer(void* buffer, NxU32 size)	const	= 0;

	// Saving API
	
	/**
	\brief Called to write a single unsigned byte to the stream(8 bits).

	\param b Byte to store.
	\return Reference to the current NxStream object.
	*/
	virtual		NxStream&		storeByte(NxU8 b)								= 0;
	
	/**
	\brief Called to write a single unsigned word to the stream(16 bits).
	
	\param w World to store.
	\return Reference to the current NxStream object.
	*/
	virtual		NxStream&		storeWord(NxU16 w)								= 0;
	
	/**
	\brief Called to write a single unsigned dword to the stream(32 bits).

	\param d DWord to store.
	\return Reference to the current NxStream object.
	*/
	virtual		NxStream&		storeDword(NxU32 d)								= 0;
	
	/**
	\brief Called to write a single precision floating point value to the stream(32 bits).

	\param f floating point value to store.
	\return Reference to the current NxStream object.
	*/
	virtual		NxStream&		storeFloat(NxF32 f)								= 0;
	
	/**
	\brief Called to write a double precision floating point value to the stream(64 bits).

	\param f floating point value to store.
	\return Reference to the current NxStream object.
	*/
	virtual		NxStream&		storeDouble(NxF64 f)							= 0;
	
	/**
	\brief Called to write an array of bytes to the stream.

	\param[in] buffer Array of bytes, size bytes in size.
	\param[in] size Size, in bytes of buffer.
	\return Reference to the current NxStream object.
	*/
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size)		= 0;

	
	/**
	\brief Store a signed byte(wrapper for the unsigned version).

	\param b Byte to store.
	\return Reference to the current NxStream object.
	*/
	NX_INLINE	NxStream&		storeByte(NxI8 b)		{ return storeByte(NxU8(b));	}
	
	/**
	\brief Store a signed word(wrapper for the unsigned version).

	\param w Word to store.
	\return Reference to the current NxStream object.
	*/
	NX_INLINE	NxStream&		storeWord(NxI16 w)		{ return storeWord(NxU16(w));	}
	
	/**
	\brief Store a signed dword(wrapper for the unsigned version).

	\param d DWord to store.
	\return Reference to the current NxStream object.
	*/
	NX_INLINE	NxStream&		storeDword(NxI32 d)		{ return storeDword(NxU32(d));	}
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
