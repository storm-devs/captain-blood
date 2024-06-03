#ifndef NX_CHARACTER_H
#define NX_CHARACTER_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#if defined NXCHARACTER_STATIC

	#define NXCHARACTER_API

#elif defined NX_USE_SDK_STATICLIBS

	#define NXCHARACTER_API

#elif defined NXCHARACTER_EXPORTS
    #if defined(WIN32)
	#define NXCHARACTER_API __declspec(dllexport)
    #elif defined(LINUX) && defined(NX_LINUX_USE_VISIBILITY)
	#define NXCHARACTER_API __attribute__ ((visibility ("default")))
    #else
	#define NXCHARACTER_API
    #endif

#else
    #if defined(WIN32)
	#define NXCHARACTER_API __declspec(dllimport)
    #elif defined(LINUX)
	#define NXCHARACTER_API
    #else
	#define NXCHARACTER_API 
    #endif

#endif

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
