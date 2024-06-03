#ifndef NX_COLLISION_NXPMAP
#define NX_COLLISION_NXPMAP
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

/**
\brief PMap data structure for mesh collision detection.

\warning Legacy class

Used by the functions NxCreatePMap and NxReleasePMap.

This structure can be assigned to NxTriangleMeshDesc::pmap or passed to NxTriangleMesh::loadPMap().

<h3>Creation</h3>

Example:

\include NxPMap_Create.cpp

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes (Software fallback)
\li PS3  : Yes
\li XB360: Yes

@see NxTriangleMesh.loadPMap() NxConvexShape.loadPMap()
*/
class NxPMap
	{
	public:
	NxU32		dataSize;	//!< size of data buffer in bytes
	void*		data;		//!< data buffer that stores the PMap information.
	};

//#ifdef NX_COOKING
	/**
	\brief Creates a PMap from a triangle mesh.
	
	\warning Legacy function

	A PMap is an optional data structure which makes mesh-mesh collision 
	detection more robust at the cost of higher	memory consumption.
	
	This structure can then be assigned to NxTriangleMeshDesc::pmap or passed to NxTriangleMesh::loadPMap().

	You may wish to store the PMap on disk (just write the above data block to a file of your choice) after
	computing it because the creation process can be quite expensive. Then you won't have to create it the next time
	you need it.

	\param[out] pmap Used to store details of the created PMap.
	\param[in] mesh Mesh to create PMap from.
	\param[in] density The density(resolution) of the PMap.
	\param[in] outputStream User supplied interface for reporting errors and displaying messages(see #NxUserOutputStream)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPMap NxTriangleMesh.loadPMap() NxConvexShape.loadPMap() NxReleasePMap
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxCreatePMap(NxPMap& pmap, const NxTriangleMesh& mesh, NxU32 density, NxUserOutputStream* outputStream = NULL);

	/**
	\brief Releases PMap previously created with NxCreatePMap.
	
	\warning Legacy function

	You should not call this on PMap data you have loaded from
	disc yourself. Don't release a PMap while it is being used by a NxTriangleMesh object.

	\param[in] pmap Pmap to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPMap NxTriangleMesh.loadPMap() NxConvexShape.loadPMap() NxCreatePMap
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxReleasePMap(NxPMap& pmap);
//#endif

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
