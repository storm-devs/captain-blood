#ifndef NX_PHYSICS_NX_SOFTBODYMESH
#define NX_PHYSICS_NX_SOFTBODYMESH
/** \addtogroup softbody
  @{
*/
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxSoftBodyMeshDesc.h"

class NxStream;

class NxSoftBodyMesh
{
protected:
	NX_INLINE NxSoftBodyMesh() {}
	virtual ~NxSoftBodyMesh() {}

public:
	/**
	\brief Saves the soft body mesh descriptor. 
	A soft body mesh is created via the cooker. The cooker potentially changes the
	order of the arrays references by the pointers vertices and triangles.
	Since saveToDesc returns the data of the cooked mesh, this data might
	differ from the originally provided data. Note that this is in contrast to the meshData
	member of NxSoftBodyDesc, which is guaranteed to provide data in the same order as
	that used to create the mesh.
	
	\param desc The descriptor used to retrieve the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyMeshDesc
	*/
	virtual	bool saveToDesc(NxSoftBodyMeshDesc& desc) const = 0;

	/**
	\brief Gets the number of soft body instances referencing this soft body mesh.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBody
	*/
	virtual	NxU32 getReferenceCount() const  = 0;
};
/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
