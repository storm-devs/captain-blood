#ifndef NX_PHYSICS_NXCONVEXFORCEFIELDSHAPEDESC
#define NX_PHYSICS_NXCONVEXFORCEFIELDSHAPEDESC
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
 \brief A descriptor for NxConvexForceFieldShape
 

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

 @see NxConvexForceFieldShape NxForceFieldShapeDesc
*/
class NxConvexForceFieldShapeDesc : public NxForceFieldShapeDesc
	{
	public:

	/**
	\brief References the triangle mesh that we want to instance.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxConvexMesh NxConvexMeshDesc NxPhysicsSDK.createConvexMesh()
	*/
	NxConvexMesh*	meshData;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxConvexForceFieldShapeDesc();

	/**
	\brief (re)sets the structure to the default.	
	*/
	virtual NX_INLINE void setToDefault();

	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	virtual NX_INLINE bool isValid() const;


	};

NX_INLINE NxConvexForceFieldShapeDesc::NxConvexForceFieldShapeDesc() : NxForceFieldShapeDesc(NX_SHAPE_CONVEX)
	{
	setToDefault();
	}

NX_INLINE void NxConvexForceFieldShapeDesc::setToDefault()
	{
	NxForceFieldShapeDesc::setToDefault();
	meshData = NULL;
	}

NX_INLINE bool NxConvexForceFieldShapeDesc::isValid() const
	{
	if(!meshData)	return false;
	return NxForceFieldShapeDesc::isValid();
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
