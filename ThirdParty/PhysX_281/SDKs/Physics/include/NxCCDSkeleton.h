#ifndef NX_PHYSICS_NXCCDSKELETON
#define NX_PHYSICS_NXCCDSKELETON
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup physics
  @{
*/

#include "Nxp.h"

/**
 \brief A mesh that is only used for continuous collision detection.
*/
class NxCCDSkeleton
	{
	public:
	/**
	\brief saves out CCDSkeleton data.

	can be loaded again with NxPhysicsSDK::createCCDSkeleton(const void * , NxU32);

	returns number of bytes written.
	*/
	virtual NxU32 save(void * destBuffer, NxU32 bufferSize) = 0;

	/**
	\brief returns number of bytes a call to ::save() will require.
	*/
	virtual NxU32 getDataSize() = 0;

	/**
	\brief Returns the reference count for shared meshes.

	\return the current reference count, not used in any shapes if the count is 0.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32				getReferenceCount()					= 0;

	/**
	\brief writes the CCD skeleton back out as an NxSimpleTriangleMesh.

  \return The number of triangles in the skeleton or zero if failed.
	*/
	virtual NxU32 saveToDesc(NxSimpleTriangleMesh &desc) = 0;

	protected:
	virtual ~NxCCDSkeleton(){};
	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
