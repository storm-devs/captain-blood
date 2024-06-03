#ifndef NX_INTERSECTION_SWEPT_SPHERES
#define NX_INTERSECTION_SWEPT_SPHERES
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
#include "NxSphere.h"
#include "PhysXLoader.h"

	/**
	\brief Sphere-sphere sweep test.
	
	Returns true if spheres intersect during their linear motion along provided velocity vectors.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] sphere0 First sphere to test. <b>Range:</b> See #NxSphere
	\param[in] velocity0 Velocity of the first sphere(ie the vector to sweep the sphere along). <b>Range:</b> velocity/direction vector
	\param[in] sphere1 Second sphere to test <b>Range:</b> See #NxSphere
	\param[in] velocity1 Velocity of the second sphere(ie the vector to sweep the sphere along). <b>Range:</b> velocity/direction vector

	\return True if spheres intersect.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphere
	*/
	NX_INLINE bool NxSweptSpheresIntersect(	const NxSphere& sphere0, const NxVec3& velocity0,
																const NxSphere& sphere1, const NxVec3& velocity1)
		{
		return NxGetUtilLib()->NxSweptSpheresIntersect(sphere0, velocity0,sphere1,velocity1);
		}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND


