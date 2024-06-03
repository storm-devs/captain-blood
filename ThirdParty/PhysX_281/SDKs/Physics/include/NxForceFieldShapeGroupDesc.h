#ifndef NX_PHYSICS_NXFORCEFIELDSHAPEGROUPDESC
#define NX_PHYSICS_NXFORCEFIELDSHAPEGROUPDESC
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
#include "NxForceFieldShapeDesc.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum NxForceFieldShapeGroupFlags
	{
	NX_FFSG_EXCLUDE_GROUP = (1 << 0),	//!< Defines whether the shapes in this group will be include or exclude volumes
	};

/**
 \brief Descriptor class for NxForceFieldShapeGroup class.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes [SW fallback]
\li PS3  : Yes
\li XB360: Yes

 @see NxForceFieldShapeGroup
*/
class NxForceFieldShapeGroupDesc
	{
	public:

	/**
	\brief Flags of the force field shape group.

	<b>Default</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxForceFieldShapeGroupFlags
	*/
	NxU32							flags;

	/**
	\brief A list of force field shape descriptors which will be added to the group.

	<b>Default</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxArray<NxForceFieldShapeDesc*>	shapes;

	/**
	\brief Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	<b>Default</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	const char* name;

	/**
	\brief Will be copied to NxForceFieldShapeGroupDesc::userData

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceFieldShapeGroupDesc.userData
	*/
	void*					userData;

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxForceFieldShapeGroupDesc();

	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();

	/**
	\brief Returns true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxForceFieldShapeGroupDesc::NxForceFieldShapeGroupDesc()
	{
	setToDefault();
	}

NX_INLINE void NxForceFieldShapeGroupDesc::setToDefault()
	{
	flags		= 0;
	name		= NULL;
	userData	= NULL;
	shapes		.clear();
	}

NX_INLINE bool NxForceFieldShapeGroupDesc::isValid() const
	{
	for(NxU32 i = 0; i < shapes.size(); i++)
		{
		if(!shapes[i]->isValid())
			return false;
		}
	return true;
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
