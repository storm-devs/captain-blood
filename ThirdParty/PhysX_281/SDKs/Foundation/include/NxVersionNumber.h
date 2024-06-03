/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

/*
VersionNumbers:  The combination of these
numbers uniquely identifies the API, and should
be incremented when the SDK API changes.  This may
include changes to file formats.

This header is included in the main SDK header files
so that the entire SDK and everything that builds on it
is completely rebuilt when this file changes.  Thus,
this file is not to include a frequently changing
build number.  See BuildNumber.h for that.

Each of these three values should stay below 255 because
sometimes they are stored in a byte.
*/
/** \addtogroup foundation
  @{
*/
#ifndef NX_VERSION_NUMBER_H

#define NX_VERSION_NUMBER_H

#define NX_SDK_VERSION_MAJOR  2

#define NX_SDK_VERSION_MINOR  8

#define NX_SDK_VERSION_BUGFIX 1

#define NX_SDK_VERSION_NUMBER 281

#define NX_SDK_API_REV	1	// bump this number *any* time a public interface changes in any way!!!
#define NX_SDK_DESC_REV 0   // bump this number *any* time the serialization data ever changes in any way, this includes NxParameters!!
#define NX_SDK_BRANCH   0   // no longer used.

#define NXAPI_HAS_PHYSXLOADER 1
#define NX_CHANGELIST_NUMBER  30725
#endif

 /** @} */
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
