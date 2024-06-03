#ifndef NX_COLLISION_NXHEIGHTFIELDSHAPEDESC
#define NX_COLLISION_NXHEIGHTFIELDSHAPEDESC
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
#include "NxHeightField.h"
#include "NxHeightFieldShape.h"

/**
\brief Descriptor class for #NxHeightFieldShape.

@see NxHeightFieldShape
*/
class NxHeightFieldShapeDesc : public NxShapeDesc
	{
	public:
	
	/**
	\brief References the height field that we want to instance.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField
	*/
	NxHeightField *					heightField;

	/**
	\brief Multiplier to transform sample height values to shape space y coordinates.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0f

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField NxHeightFieldDesc NxHeightFieldFormat
	*/
	NxReal							heightScale;

	/**
	\brief Multiplier to transform height field rows to shape space x coordinates.

	<b>Range:</b> (-inf,0), (0,inf)<br>
	<b>Default:</b> 1.0f

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField NxHeightFieldDesc
	*/
	NxReal							rowScale;

	/**
	\brief Multiplier to transform height field columns to shape space z coordinates.

	<b>Range:</b> (-inf,0), (0,inf)<br>
	<b>Default:</b> 1.0f

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField NxHeightFieldDesc
	*/
	NxReal							columnScale;

	/**
	\brief The high 9 bits of this number are used to complete the material indices in the samples. 

	The remaining low 7 bits must be zero.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField NxHeightFieldDesc
	*/
	NxMaterialIndex					materialIndexHighBits;

	/**
	\brief The the material index that designates holes in the height field.

	This number is compared directly to sample materials.
	Consequently the high 9 bits must be zero.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightField NxHeightFieldDesc
	*/
	NxMaterialIndex					holeMaterial;

	/**
	\brief Combination of ::NxMeshShapeFlag.
	So far the only value permitted here is 0 or NX_MESH_SMOOTH_SPHERE_COLLISIONS.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMeshShapeFlag
	*/
	NxU32							meshFlags;

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE						NxHeightFieldShapeDesc();

	/**
	\brief (re)sets the structure to the default.	
	*/
	
	NX_INLINE virtual	void		setToDefault();
	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid
	*/

	NX_INLINE virtual	bool		isValid() const;
	};

NX_INLINE NxHeightFieldShapeDesc::NxHeightFieldShapeDesc() : NxShapeDesc(NX_SHAPE_HEIGHTFIELD)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxHeightFieldShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	heightField = 0;
	heightScale = 1.0f;
	rowScale = 1.0f;
	columnScale = 1.0f;
	materialIndexHighBits = 0;
	holeMaterial = 0;
	meshFlags = 0;
	}

NX_INLINE bool NxHeightFieldShapeDesc::isValid() const
	{
	if (!heightField) return false;

	if (heightScale				 <= NX_EPS_REAL) return false;
	if (NxMath::abs(rowScale)	 <= NX_EPS_REAL) return false;
	if (NxMath::abs(columnScale) <= NX_EPS_REAL) return false;
	
	switch (heightField->getFormat())
		{
		case NX_HF_S16_TM:
			if (0x7f & materialIndexHighBits) return false;
			if ((0x7f & holeMaterial) != holeMaterial) return false;
			break;
		default:
			return false;
		}	


	if (meshFlags & (~NX_MESH_SMOOTH_SPHERE_COLLISIONS)) return false;

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
