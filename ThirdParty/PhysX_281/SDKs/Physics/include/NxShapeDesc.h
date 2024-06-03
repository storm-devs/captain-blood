#ifndef NX_COLLISION_NXSHAPEDESC
#define NX_COLLISION_NXSHAPEDESC
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

#include "NxShape.h"


/**
\brief Describes the compartment types a rigid body shape might interact with
*/
enum NxShapeCompartmentType
	{
	NX_COMPARTMENT_SW_RIGIDBODY		= (1<<0),	//!< Software rigid body compartment
	NX_COMPARTMENT_HW_RIGIDBODY		= (1<<1),	//!< Hardware rigid body compartment
	NX_COMPARTMENT_SW_FLUID			= (1<<2),	//!< Software fluid compartment
	NX_COMPARTMENT_HW_FLUID			= (1<<3),	//!< Hardware fluid compartment
	NX_COMPARTMENT_SW_CLOTH			= (1<<4),	//!< Software cloth compartment
	NX_COMPARTMENT_HW_CLOTH			= (1<<5),	//!< Hardware cloth compartment
	NX_COMPARTMENT_SW_SOFTBODY		= (1<<6),	//!< Software softbody compartment
	NX_COMPARTMENT_HW_SOFTBODY		= (1<<7),	//!< Hardware softbody compartment
	};


/**
\brief Descriptor for #NxShape class. 

Used for saving and loading the shape state.

See the derived classes for the different shape types.

@see NxActor.createShape() NxSphereShapeDesc NxPlaneShapeDesc NxConvexShapeDesc NxTriangleMeshShapeDesc
NxCapsuleShapeDesc NxBoxShapeDesc
*/
class NxShapeDesc
	{
	protected:
	
	/**
	\brief The type of the shape (see NxShape.h).
	
	This gets set by the derived class' ctor, the user should not have to change it.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	const NxShapeType		type;
	public:

	/**
	\brief The pose of the shape in the coordinate frame of the owning actor.

	<b>Range:</b> rigid body transform<br>
	<b>Default:</b> Identity

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape.setLocalPose
	*/
	NxMat34					localPose;

	/**
	\brief A combination of ::NxShapeFlag values.

	<b>Default:</b> NX_SF_VISUALIZATION

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape.setFlag() NxShapeFlag
	*/
	NxU32					shapeFlags;

	/**
	\brief See the documentation for NxShape::setGroup().

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape.setGroup()
	*/
	NxCollisionGroup		group;

	/**
	\brief The material index of the shape.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene.createMaterial() NxShape.setMaterial()
	*/
	NxMaterialIndex			materialIndex;
	
	/**
	\brief CCD Skeleton

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape.setCCDSkeleton() NxPhysicsSDK.createCCDSkeleton()
	*/	
	NxCCDSkeleton*			ccdSkeleton;
	
	/**
	\brief density of this individual shape when computing mass inertial properties for a rigidbody (unless a valid mass >0.0 is provided).
	Note that this will only be used if the body has a zero inertia tensor, or if you call #NxActor::updateMassFromShapes explicitly.
	
	See #NxActorDesc for a description of how the density is applied.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxReal					density;

	/**
	\brief mass of this individual shape when computing mass inertial properties for a rigidbody.  When mass<=0.0 then density and volume determine the mass.
	Note that this will only be used if the body has a zero inertia tensor, or if you call #NxActor::updateMassFromShapes explicitly.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> -1.0 (I.e. calculate mass from density)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxReal					mass;
	
	/**
	\brief Specifies by how much shapes can interpenetrate

	Two shapes will interpenetrate by the sum of their skin widths. This means that their graphical representations should be adjusted
	so that they just touch when the shapes are interpenetrating.

	The default skin width is the NX_SKIN_WIDTH SDK parameter.
	This is used if the skinWidth member is set to -1(which is the default).
	
	A skin width sum of	zero for two bodies is not permitted because it will lead to an unstable simulation. 
	
	If your simulation jitters because resting bodies occasionally lose contact, increasing the size of your collision volumes 
	and the skin width may improve things.

	Units: distance. 
	
	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> -1.0 (use the default specified with NX_SKIN_WIDTH)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxParameter
	*/
	NxReal					skinWidth;


	/**
	\brief Will be copied to NxShape::userData.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	void*					userData;

	/**
	\brief Possible debug name. The string is not copied by the SDK, only the pointer is stored.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	const char*				name;

	/**
	\brief Groups bit mask for collision filtering

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxGroupsMask			groupsMask;

	/**
	\brief A combination of ::NxShapeCompartmentType values.

	Defines which compartment types the shape should not interact with.

	\note This member is ignored in the following cases:

	\li Explicitly adding the shape, i.e., its actor, to a rigid body compartment.
	\li Attaching the shape to a fluid emitter.
	\li Attaching the shape to a cloth.
	\li Attaching the shape to a soft body.

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxShape.setNonInteractingCompartmentTypes() NxShapeCompartmentType
	*/
	NxU32					nonInteractingCompartmentTypes;

	NX_INLINE virtual		~NxShapeDesc();
	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void setToDefault();
	
	/**
	\brief Returns true if the descriptor is valid.
	
	\return true if the current settings are valid
	*/
	NX_INLINE virtual	bool isValid() const;

	/**
	\brief Retrieves the shape type.

	\return The type of the shape. See #NxShapeType.
	*/
	NX_INLINE			NxShapeType	getType()	const	{ return type; }

	protected:
	/**
	constructor sets to default.

	\param[in] shapeType The type of shape this desc is created for.
	*/
	NX_INLINE				NxShapeDesc(NxShapeType shapeType);

	NxShapeDesc& operator=( const NxShapeDesc& ) {};
	};

NX_INLINE NxShapeDesc::NxShapeDesc(NxShapeType t) : type(t)
	{
	setToDefault();
	}

NX_INLINE NxShapeDesc::~NxShapeDesc()
	{
	}

NX_INLINE void NxShapeDesc::setToDefault()
	{
	localPose.id();
	shapeFlags			= NX_SF_VISUALIZATION | NX_SF_CLOTH_TWOWAY | NX_SF_SOFTBODY_TWOWAY;
	group				= 0;
	materialIndex		= 0;
	ccdSkeleton		= NULL;
	skinWidth			= -1.0f;
	density				=  1.0f;
	mass				= -1.0f;  // by default we let the mass be determined by its density.  
	userData			= NULL;
	name				= NULL;
	groupsMask.bits0	= 0;
	groupsMask.bits1	= 0;
	groupsMask.bits2	= 0;
	groupsMask.bits3	= 0;
	nonInteractingCompartmentTypes = 0;
	}

NX_INLINE bool NxShapeDesc::isValid() const
	{
	if(!localPose.isFinite())
		return false;
	if(group>=32)
		return false;	// We only support 32 different groups
	// dsullins: I removed this bogus shapeFlags check because it was preventing core dumps from loading
	//if(shapeFlags&0xffff0000)
	//	return false;	// Only 16-bit flags supported here
	if(type >= NX_SHAPE_COUNT)
		return false;
	if(materialIndex==0xffff)
		return false;	// 0xffff is reserved for internal usage
	if (skinWidth != -1 && skinWidth < 0)
		return false;
	return true;
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
