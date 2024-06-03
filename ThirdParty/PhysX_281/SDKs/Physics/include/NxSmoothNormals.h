#ifndef NX_COLLISION_NXSMOOTHNORMALS
#define NX_COLLISION_NXSMOOTHNORMALS
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
#include "PhysXLoader.h"

	/**
	\brief Builds smooth vertex normals over a mesh.

	- "smooth" because smoothing groups are not supported here
	- takes angles into account for correct cube normals computation

	To use 32bit indices pass a pointer in dFaces and set wFaces to zero. Alternatively pass a pointer to 
	wFaces and set dFaces to zero.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] nbTris Number of triangles
	\param[in] nbVerts Number of vertices
	\param[in] verts Array of vertices
	\param[in] dFaces Array of dword triangle indices, or null
	\param[in] wFaces Array of word triangle indices, or null
	\param[out] normals Array of computed normals (assumes nbVerts vectors)
	\param[in] flip Flips the normals or not

	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE bool NX_CALL_CONV NxBuildSmoothNormals(
		NxU32 nbTris,
		NxU32 nbVerts,
		const NxVec3* verts,
		const NxU32* dFaces,
		const NxU16* wFaces,
		NxVec3* normals,
		bool flip=false
		)
		{
		return NxGetUtilLib()->NxBuildSmoothNormals(nbTris,nbVerts,verts,dFaces,wFaces,normals,flip);
		}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
