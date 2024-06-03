#ifndef NX_PHYSICS_NXFORCEFIELDDESC
#define NX_PHYSICS_NXFORCEFIELDDESC
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
#include "NxArray.h"
#include "NxForceFieldShapeDesc.h"

class NxActor;
class NxForceFieldKernel;
class NxForceFieldShapeGroup;

/**
\brief Type of force field coordinate space
*/
enum NxForceFieldCoordinates { NX_FFC_CARTESIAN, NX_FFC_SPHERICAL, NX_FFC_CYLINDRICAL, NX_FFC_TOROIDAL };

enum NxForceFieldType
	{
	NX_FF_TYPE_DUMMY_0,				// deprecated enum placeholder
	NX_FF_TYPE_DUMMY_1,				// deprecated enum placeholder
	NX_FF_TYPE_GRAVITATIONAL,		//!< scales the force by the mass of the particle or body
	NX_FF_TYPE_OTHER,				//!< does not scale the value from the force field
	NX_FF_TYPE_NO_INTERACTION		//!< used to disable force field interaction with a specific feature
	};

enum NxForceFieldFlags 
	{ 
	NX_FFF_DUMMY_0						= (1<<0),	// deprecated flag placeholder
	NX_FFF_DUMMY_1						= (1<<1),	// deprecated flag placeholder
	NX_FFF_DUMMY_2						= (1<<2),	// deprecated flag placeholder
	NX_FFF_DUMMY_3						= (1<<3),	// deprecated flag placeholder
	NX_FFF_VOLUMETRIC_SCALING_FLUID		= (1<<5),	//!< indicates whether the force is scaled by the amount of volume represented by the feature.
	NX_FFF_VOLUMETRIC_SCALING_CLOTH		= (1<<6),	//!< indicates whether the force is scaled by the amount of volume represented by the feature.
	NX_FFF_VOLUMETRIC_SCALING_SOFTBODY	= (1<<7),	//!< indicates whether the force is scaled by the amount of volume represented by the feature.
	NX_FFF_VOLUMETRIC_SCALING_RIGIDBODY	= (1<<8),	//!< indicates whether the force is scaled by the amount of volume represented by the feature.
	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 \brief Descriptor class for NxForceField class.


<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes [SW fallback]
\li PS3  : Yes
\li XB360: Yes

 @see NxForceField
*/
class NxForceFieldDesc
	{
	public:

	/**
	\brief Global or (if actor is set) actor relative transformation of the force field. 
	Detaching from the actor will cause the force field's pose to be relative to the world frame.

	<b>Default:</b> Identity

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxMat34					pose;

	/**
	\brief The field's pose is relative to the actor's pose and relative to the world frame if field is null.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxActor*				actor;
	
	/**
	\brief Coordinate space of the field.

	<b>Default:</b> NX_FFC_CARTESIAN

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldCoordinates	coordinates;

	/**
	\brief Array of force field shapes descriptors which will be created inside the include group of this force field. This group moves with the force field and cannot be shared.

	<b>Default:</b> empty

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxArray<NxForceFieldShapeDesc*> 
							includeGroupShapes;	
	
	/**
	\brief a collection of NxForceFieldShapeGroup objects. @see NxForceFieldShapeGroup

	<b>Default:</b> empty

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxArray<NxForceFieldShapeGroup*> 
							shapeGroups;
	/**
	\brief Collision group used for collision filtering.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxCollisionGroup		group;
	
	/**
	\brief Groups mask used for collision filtering.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxGroupsMask			groupsMask;

	/**
	\brief kernel function of the force field.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldKernel*		kernel;	

	/**
	\brief Force Field Variety Index, index != 0 has to be created.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldVariety		forceFieldVariety;

	/**
	\brief Force field type for fluids

	<b>Default:</b> NX_FF_TYPE_OTHER

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldType		fluidType;

	/**
	\brief Force field type for cloth

	<b>Default:</b> NX_FF_TYPE_OTHER

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldType		clothType;
	
	/**
	\brief Force field type for soft bodies

	<b>Default:</b> NX_FF_TYPE_OTHER

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldType		softBodyType;

	/**
	\brief Force field type for rigid bodies

	<b>Default:</b> NX_FF_TYPE_OTHER

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxForceFieldType		rigidBodyType;

	/**
	\brief Force field flags; @see NxForceFieldFlags

	<b>Default:</b> NX_FFF_VOLUMETRIC_SCALING_FLUID | NX_FFF_VOLUMETRIC_SCALING_CLOTH | NX_FFF_VOLUMETRIC_SCALING_SOFTBODY | NX_FFF_VOLUMETRIC_SCALING_RIGIDBODY

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32					flags;					//!< 

	/**
	\brief Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	<b>Default</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	const char* name;

	/**
	\brief Will be copied to NxForceField::userData

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceField.userData
	*/
	void*					userData; 


	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxForceFieldDesc();

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

NX_INLINE NxForceFieldDesc::NxForceFieldDesc()
	{
	setToDefault();
	}

NX_INLINE void NxForceFieldDesc::setToDefault()
	{
	pose.id();
	actor = NULL;
	coordinates = NX_FFC_CARTESIAN;

	includeGroupShapes	.clear();

	group				= 0;
	groupsMask.bits0	= 0;
	groupsMask.bits1	= 0;
	groupsMask.bits2	= 0;
	groupsMask.bits3	= 0;

	kernel = NULL;

	flags = NX_FFF_VOLUMETRIC_SCALING_FLUID | 
			NX_FFF_VOLUMETRIC_SCALING_CLOTH |
			NX_FFF_VOLUMETRIC_SCALING_SOFTBODY |
			NX_FFF_VOLUMETRIC_SCALING_RIGIDBODY;

	forceFieldVariety	= 0;

	fluidType		= NX_FF_TYPE_OTHER;
	clothType		= NX_FF_TYPE_OTHER;
	softBodyType	= NX_FF_TYPE_OTHER;
	rigidBodyType	= NX_FF_TYPE_OTHER;

	name		= NULL;
	userData	= NULL;
	}

NX_INLINE bool NxForceFieldDesc::isValid() const
	{
	
	for(NxU32 i = 0; i < includeGroupShapes.size(); i++)
		{
		if(!includeGroupShapes[i]->isValid())
			return false;
		}

	if(group>=32)
		return false;	// We only support 32 different groups

	if(!kernel)
		return false; 

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
