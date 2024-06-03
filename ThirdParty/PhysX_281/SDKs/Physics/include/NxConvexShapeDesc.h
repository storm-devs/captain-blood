#ifndef NX_COLLISION_NXCONVEXSHAPEDESC
#define NX_COLLISION_NXCONVEXSHAPEDESC
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

#include "NxShapeDesc.h"
#include "NxConvexShape.h"

class NxConvexMesh;

/**
\brief Descriptor class for #NxConvexShape.

@see NxConvexShape NxShapeDesc NxActor.createShape() NxConvexMesh NxPhysicsSDK.createConvexMesh()
*/
class NxConvexShapeDesc : public NxShapeDesc
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
	\brief Combination of ::NxMeshShapeFlag

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMeshShapeFlag
	*/
	NxU32			meshFlags;

#ifdef NX_SUPPORT_CONVEX_SCALE
	NxReal			scale;
#endif
	
	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE					NxConvexShapeDesc();	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	\brief Returns true if the descriptor is valid.

	\return returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxConvexShapeDesc::NxConvexShapeDesc() : NxShapeDesc(NX_SHAPE_CONVEX)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxConvexShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	meshData	= NULL;
	meshFlags	= 0;
#ifdef NX_SUPPORT_CONVEX_SCALE
	scale		= 1.0f;
#endif
	}

NX_INLINE bool NxConvexShapeDesc::isValid() const
	{
	if(!meshData)	return false;
#ifdef NX_SUPPORT_CONVEX_SCALE
	if(scale<=0.0f)	return false;
#endif
	return NxShapeDesc::isValid();
	}
/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
