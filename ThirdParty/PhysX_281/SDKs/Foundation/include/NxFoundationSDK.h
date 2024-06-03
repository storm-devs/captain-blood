#ifndef NX_FOUNDATION_NXFOUNDATIONSDK
#define NX_FOUNDATION_NXFOUNDATIONSDK
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

#include "Nxf.h"
#include "NxVersionNumber.h"

class NxUserOutputStream;
class NxUserAllocator;
class NxProfilingZone;
class NxDebugRenderable;
class NxUserDebugRenderer;
class NxDebugRenderable;
class NxRemoteDebugger;

/**
\brief Foundation SDK singleton class.

You need to have an instance of this class to instance the higher level SDKs.
*/
class NxFoundationSDK
	{
	public:
	/**
	Destroys the instance it is called on.

	Use this release method to destroy an instance of this class. Be sure
	to not keep a reference to this object after calling release.

	Note: the foundation SDK instance used by the SDK should not be manually released, 
	please use the NxReleasePhysicsSDK() function to release the SDK and foundation SDK.
	*/
	virtual	void release() = 0;

	/**
	Sets an error stream provided by the user.

	After an error stream has been set, human readable error messages 
	will be inserted into it.  

	\param stream Stream to report error on.
	*/
	virtual void setErrorStream(NxUserOutputStream* stream) = 0;

	/**
	retrieves error stream
	*/
	virtual NxUserOutputStream* getErrorStream() = 0;

	/**
	retrieves information about the last (most recent) error that has occurred, and then
	resets both the last error code to NXE_NO_ERROR.
	*/
	virtual NxErrorCode getLastError() = 0;

	/**
	retrieves information about the first error that has occurred since the last call to
	getLastError() or getFirstError(), and then	this error code to NXE_NO_ERROR.
	*/
	virtual NxErrorCode getFirstError() = 0;

	/**
	retrieves the current allocator.
	*/
	virtual NxUserAllocator & getAllocator() = 0;

	/**
	retrieves the current remote debugger.
	*/
	virtual NxRemoteDebugger* getRemoteDebugger() = 0;

	/*
	creates a profiling zone.  At the moment this is not needed by the user.
	*/
	//virtual NxProfilingZone* createProfilingZone(const char * x) = 0;
	
	/**
	Sets the threshold for internal stack allocation. By default, stack allocations
	are limited to half the default stack size for the platform or half the smallest
	stack size of any thread created by the SDK. If called with threshold 0, this automatic
	scheme is re-activated.
	*/
	virtual void setAllocaThreshold(NxU32 threshold) = 0;

	protected:
	virtual ~NxFoundationSDK(){};
	};

/**
The constant NX_FOUNDATION_SDK_VERSION is used when creating the NxFoundationSDK object, 
which is an internally created object. This is to ensure that the application is using 
the same header version as the library was built with.
*/
#define NX_FOUNDATION_SDK_VERSION ((   NX_SDK_VERSION_MAJOR   <<24)+(NX_SDK_VERSION_MINOR    <<16)+(NX_SDK_VERSION_BUGFIX    <<8) + 0)

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
