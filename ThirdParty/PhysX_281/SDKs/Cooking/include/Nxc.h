#ifndef NX_COOKING_NX
#define NX_COOKING_NX
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/**
DLL export macros
*/
#ifndef NXC_DLL_EXPORT
	#ifdef NX_COOKING_DLL
           #if defined(WIN32)
		#define NXC_DLL_EXPORT __declspec(dllexport)
           #elif defined(LINUX) && defined(NX_LINUX_USE_VISIBILITY)
		#define NXC_DLL_EXPORT __attribute__ ((visibility ("default")))
           #else
                #define NXC_DLL_EXPORT
           #endif
	#elif defined NX_COOKING_STATICLIB

		#define NXC_DLL_EXPORT

	#elif defined NX_USE_SDK_DLLS
            #if defined(WIN32)
		#define NXC_DLL_EXPORT __declspec(dllimport)
            #elif defined(LINUX)
		#define NXC_DLL_EXPORT
            #else
                #define NXC_DLL_EXPORT
            #endif

	#elif defined NX_USE_SDK_STATICLIBS

		#define NXC_DLL_EXPORT

	#else
            #if defined(WIN32)
		#define NXC_DLL_EXPORT __declspec(dllimport)
            #elif defined(LINUX)
		#define NXC_DLL_EXPORT
            #else
                #define NXC_DLL_EXPORT 
	    #endif
	#endif
#endif

#ifndef NX_C_EXPORT
	#define NX_C_EXPORT extern "C"
#endif

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
