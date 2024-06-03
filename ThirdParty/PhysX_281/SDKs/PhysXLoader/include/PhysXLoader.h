#ifndef PHYSX_LOADER_H
#define PHYSX_LOADER_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to AGEIA PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxPhysicsSDK.h"
#include "NxUtilLib.h"

#ifdef _USRDLL
    #if defined(WIN32)
	#define NXPHYSXLOADERDLL_API extern "C" __declspec(dllexport)
    #elif defined(LINUX) 
      #if defined(NX_LINUX_USE_VISIBILITY)
        #define NXPHYSXLOADERDLL_API extern "C" __attribute__ ((visibility ("default")))
      #else
        #define NXPHYSXLOADERDLL_API extern "C"
      #endif
    #else
        #define NXPHYSXLOADERDLL_API
    #endif
#elif defined NX_USE_SDK_STATICLIBS
	#define NXPHYSXLOADERDLL_API
#else
    #if defined(WIN32)
	#define NXPHYSXLOADERDLL_API extern "C" __declspec(dllimport)
    #elif defined(LINUX)
	#define NXPHYSXLOADERDLL_API extern "C" 
    #else
	#define NXPHYSXLOADERDLL_API 
    #endif
#endif


/** \addtogroup PhysXLoader
  @{
*/

/**
\brief Creates an instance of the physics SDK.

Creates an instance of this class. May not be a class member to avoid name mangling.
Pass the constant NX_PHYSICS_SDK_VERSION as the argument.
Because the class is a singleton class, multiple calls return the same object. However, each call must be
matched by a corresponding call to NxReleasePhysicsSDK, as the SDK uses a reference counter to the singleton.

NOTE: Calls after the first will not change the allocator used in the SDK, but they will affect the output stream.

\param sdkVersion Version number we are expecting(should be NX_PHYSICS_SDK_VERSION)
\param allocator User supplied interface for allocating memory(see #NxUserAllocator)
\param outputStream User supplied interface for reporting errors and displaying messages(see #NxUserOutputStream)
\param desc Optional descriptor used to define hardware allocation parameters
\param errorCode Optional error code output parameter
*/
NXPHYSXLOADERDLL_API NxPhysicsSDK*	NX_CALL_CONV NxCreatePhysicsSDK(NxU32 sdkVersion, NxUserAllocator* allocator = NULL, NxUserOutputStream* outputStream = NULL, const NxPhysicsSDKDesc& desc = NxPhysicsSDKDesc(), NxSDKCreateError* errorCode=NULL);

/**
\brief Creates an instance of the physics SDK with an ID string for application identification.

Creates an instance of this class. May not be a class member to avoid name mangling.
Pass the constant NX_PHYSICS_SDK_VERSION as the argument.
Because the class is a singleton class, multiple calls return the same object. However, each call must be
matched by a corresponding call to NxReleasePhysicsSDK, as the SDK uses a reference counter to the singleton.

NOTE: Calls after the first will not change the allocator used in the SDK, but they will affect the output stream.

\param sdkVersion Version number we are expecting(should be NX_PHYSICS_SDK_VERSION)
\param companyNameStr Character string for the game or application developer company name
\param appNameStr Character string for the game or application name
\param appVersionStr Character string for the game or application version
\param appUserDefinedStr Character string for additional, user defined data
\param allocator User supplied interface for allocating memory(see #NxUserAllocator)
\param outputStream User supplied interface for reporting errors and displaying messages(see #NxUserOutputStream)
\param desc Optional descriptor used to define hardware allocation parameters
\param errorCode Optional error code output parameter
*/
NXPHYSXLOADERDLL_API NxPhysicsSDK* NX_CALL_CONV NxCreatePhysicsSDKWithID(NxU32 sdkVersion, 
                                                             char *companyNameStr, 
                                                             char *appNameStr,
                                                             char *appVersionStr,
                                                             char *appUserDefinedStr,
                                                             NxUserAllocator* allocator = NULL, 
                                                             NxUserOutputStream* outputStream = NULL, 
                                                             const NxPhysicsSDKDesc &desc = NxPhysicsSDKDesc(),
															 NxSDKCreateError* errorCode=NULL);


/**
\brief Release an instance of the PhysX SDK.

Note that this must be called once for each prior call to NxCreatePhysicsSDK or NxCreatePhysicsSDKWithID, as
there is a reference counter. Also note that you mustn't destroy the allocator or the outputStream (if available) until after the
reference count reaches 0 and the SDK is actually removed.

Releasing an SDK will also release any scenes, triangle meshes, convex meshes, heightfields, CCD skeletons, and cloth
meshes created through it, provided the user hasn't already done so.

You should release all scenes created with the SDK prior to calling this method, or resources may not be released.
*/
NXPHYSXLOADERDLL_API void			NX_CALL_CONV NxReleasePhysicsSDK(NxPhysicsSDK* sdk);

/**
\brief Retrieves the Physics SDK allocator.

Used by NxAllocateable's inlines and other macros below.

Before using this function the user must call #NxCreatePhysicsSDK(). If #NxCreatePhysicsSDK()
has not been called then NULL will be returned.
*/
NXPHYSXLOADERDLL_API NxUserAllocator* NX_CALL_CONV NxGetPhysicsSDKAllocator();

/**
\brief Retrieves the Foundation SDK after it has been created.

Before using this function the user must call #NxCreatePhysicsSDK(). If #NxCreatePhysicsSDK()
has not been called then NULL will be returned..
*/
NXPHYSXLOADERDLL_API NxFoundationSDK* NX_CALL_CONV NxGetFoundationSDK();

/**
\brief Retrieves the Physics SDK after it has been created.

Before using this function the user must call #NxCreatePhysicsSDK(). If #NxCreatePhysicsSDK()
has not been called then NULL will be returned.
*/
NXPHYSXLOADERDLL_API NxPhysicsSDK*    NX_CALL_CONV NxGetPhysicsSDK();

/**
\brief Retrieves the Physics SDK Utility Library

Before using this function the user must call #NxCreatePhysicsSDK(). If #NxCreatePhysicsSDK()
has not been called then NULL will be returned.
*/

NXPHYSXLOADERDLL_API NxUtilLib* NX_CALL_CONV NxGetUtilLib();

/**
\brief Retrieves the Physics SDK Cooking Library for a specific version of the SDK.

\note Not supported on platforms where dynamic linking is not used (Xbox 360 and PS3).

\param sdk_version_number the version of the PhysX SDK, in hexadecimal format (e.g. 0x02060000 for
2.6.0). You may also use the NX_PHYSICS_SDK_VERSION constant.
*/
class NxCookingInterface;
NXPHYSXLOADERDLL_API NxCookingInterface * NX_CALL_CONV NxGetCookingLib(NxU32 sdk_version_number);
NXPHYSXLOADERDLL_API NxCookingInterface * NX_CALL_CONV NxGetCookingLibWithID(NxU32 sdk_version_number, 
																	        char *companyNameStr, 
																			char *appNameStr,
																			char *appVersionStr,
																			char *appUserDefinedStr);

/** @} */

#endif


//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AGEIA Technologies, Inc.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND

