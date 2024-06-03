#ifndef NX_PHYSICS_NXMATERIALDESC
#define NX_PHYSICS_NXMATERIALDESC
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
#include "NxSpringDesc.h"

/**
\brief Flags which control the behavior of a material.

@see NxMaterial NxMaterialDesc
*/
enum NxMaterialFlag
	{
	/**
	\brief Flag to enable anisotropic friction computation. 

	For a pair of actors, anisotropic friction is used only if at least one of the two actors' materials are anisotropic.
	The anisotropic friction parameters for the pair are taken from the material which is more anisotropic (i.e. the difference
	between its two dynamic friction coefficients is greater).

	The anisotropy direction of the chosen material is transformed to world space:

	dirOfAnisotropyWS = shape2world * dirOfAnisotropy

	Next, the directions of anisotropy in one or more contact planes (i.e. orthogonal to the contact normal) have to be determined. 
	The two directions are:

	uAxis = (dirOfAnisotropyWS ^ contactNormal).normalize()
	vAxis = contactNormal ^ uAxis

	This way [uAxis, contactNormal, vAxis] forms a basis.

	It may happen, however, that (dirOfAnisotropyWS | contactNormal).magnitude() == 1 
	and then (dirOfAnisotropyWS ^ contactNormal) has zero length. This happens when 
	the contactNormal is coincident to the direction of anisotropy. In this case we perform isotropic friction. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (SW fall-back)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterialDesc.dirOfAnisotropy
	*/
	NX_MF_ANISOTROPIC = 1 << 0,

	/**
	If this flag is set, friction computations are always skipped between shapes with this material and any other shape.
	It may be a good idea to use this when all friction is to be performed using the tire friction model (see ::NxWheelShape).

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxWheelShape
	*/
	NX_MF_DISABLE_FRICTION = 1 << 4,

	/**
	The difference between "normal" and "strong" friction is that the strong friction feature
	remembers the "friction error" between simulation steps. The friction is a force trying to
	hold objects in place (or slow them down) and this is handled in the solver. But since the
	solver is only an approximation, the result of the friction calculation can include a small
	"error" - e.g. a box resting on a slope should not move at all if the static friction is in
	action, but could slowly glide down the slope because of a small friction error in each 
	simulation step. The strong friction counter-acts this by remembering the small error and
	taking it to account during the next simulation step.

	However, in some cases the strong friction could cause problems, and this is why it is
	possible to disable the strong friction feature by setting this flag. One example is
	raycast vehicles, that are sliding fast across the surface, but still need a precise
	steering behavior. It may be a good idea to reenable the strong friction when objects
	are coming to a rest, to prevent them from slowly creeping down inclines.

	Note: This flag only has an effect if the NX_MF_DISABLE_FRICTION bit is 0.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxWheelShape
	*/
	NX_MF_DISABLE_STRONG_FRICTION = 1 << 5,

	//Note: Bits 16-31 are reserved for internal use!
	};

/**
Flag that determines the combine mode. When two actors come in contact with each other, they each have
materials with various coefficients, but we only need a single set of coefficients for the pair.

Physics doesn't have any inherent combinations because the coefficients are determined empirically on a case by case
basis. However, simulating this with a pairwise lookup table is often impractical.

For this reason the following combine behaviors are available:

NX_CM_AVERAGE
NX_CM_MIN
NX_CM_MULTIPLY
NX_CM_MAX

The effective combine mode for the pair is max(material0.combineMode, material1.combineMode).

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxMaterial NxMaterialDesc NxMaterialDesc.frictionCombineMode NxMaterialDesc.restitutionCombineMode
*/
enum NxCombineMode
	{
	NX_CM_AVERAGE = 0,		//!< Average: (a + b)/2
	NX_CM_MIN = 1,			//!< Minimum: min(a,b)
	NX_CM_MULTIPLY = 2,		//!< Multiply: a*b
	NX_CM_MAX = 3,			//!< Maximum: max(a,b)
	NX_CM_N_VALUES = 4,	//!< This is not a valid combine mode, it is a sentinel to denote the number of possible values. We assert that the variable's value is smaller than this.
	NX_CM_PAD_32 = 0xffffffff //!< This is not a valid combine mode, it is to assure that the size of the enum type is big enough.
	};


/**
\brief Descriptor of #NxMaterial.

@see NxMaterial NxScene.createMaterial()
*/
class NxMaterialDesc
	{
	public:
	/**
	coefficient of dynamic friction -- should be in [0, +inf]. If set to greater than staticFriction, the effective value of staticFriction will be increased to match.
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)

	<b>Range:</b> [0,inf]<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags frictionCombineMode
	*/
	NxReal	dynamicFriction;
	
	/**
	coefficient of static friction -- should be in [0, +inf]
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)

	<b>Range:</b> [0,inf]<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags frictionCombineMode
	*/
	NxReal	staticFriction;

	/**
	coefficient of restitution --  0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce.
	Note that values close to or above 1 may cause stability problems and/or increasing energy.
	<b>Range:</b> [0,1]<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags restitutionCombineMode
	*/
	NxReal	restitution;

	/**
	anisotropic dynamic friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags & NX_MF_ANISOTROPIC is set.

	<b>Range:</b> [0,inf]<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags dynamicFriction
	*/
	NxReal dynamicFrictionV;

	/**
	anisotropic static  friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags & NX_MF_ANISOTROPIC is set.

	<b>Range:</b> [0,inf]<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags staticFriction
	*/
	NxReal staticFrictionV;
	
	/**
	shape space direction (unit vector) of anisotropy.
	This is only used if flags & NX_MF_ANISOTROPIC is set.

	<b>Range:</b> direction vector<br>
	<b>Default:</b> 1.0f,0.0f,0.0f

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags staticFrictionV dynamicFrictionV
	*/
	NxVec3 dirOfAnisotropy;

	/**
	Flags, a combination of the bits defined by the enum ::NxMaterialFlag . 

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterialFlag
	*/
	NxU32 flags;

	/**
	Friction combine mode. See the enum ::NxCombineMode .

	<b>Default:</b> NX_CM_AVERAGE

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCombineMode staticFriction dynamicFriction
	*/
	NxCombineMode frictionCombineMode;

	/**
	Restitution combine mode. See the enum ::NxCombineMode .

	<b>Default:</b> NX_CM_AVERAGE

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCombineMode restitution
	*/
	NxCombineMode restitutionCombineMode;

	/**
	Not used.
	*/
	NxSpringDesc * spring;

	/**
	\brief constructor sets to default.
	*/
	NX_INLINE NxMaterialDesc();	
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

NX_INLINE NxMaterialDesc::NxMaterialDesc()
	{
	setToDefault();
	}

NX_INLINE	void NxMaterialDesc::setToDefault()
	{
	dynamicFriction	= 0.0f;
	staticFriction	= 0.0f;
	restitution		= 0.0f;


	dynamicFrictionV= 0.0f;
	staticFrictionV = 0.0f;

	dirOfAnisotropy.set(1,0,0);
	flags = 0;
	frictionCombineMode = NX_CM_AVERAGE;
	restitutionCombineMode = NX_CM_AVERAGE;
	spring = 0;
	}

NX_INLINE	bool NxMaterialDesc::isValid()	const
	{
	if(dynamicFriction < 0.0f) 
		return false;
	if(staticFriction < 0.0f) 
		return false;
	if(restitution < 0.0f || restitution > 1.0f) 
		return false;


	if (flags & NX_MF_ANISOTROPIC)
		{
		NxReal ad = dirOfAnisotropy.magnitudeSquared();
		if (ad < 0.98f || ad > 1.03f)
			return false;
		if(dynamicFrictionV < 0.0f) 
			return false;
		if(staticFrictionV < 0.0f) 
			return false;
		}
	/*
	if (flags & NX_MF_MOVING_SURFACE)
		{
		NxReal md = dirOfMotion.magnitudeSquared();
		if (md < 0.98f || md > 1.03f)
			return false;
		}
	*/
	if (frictionCombineMode >= NX_CM_N_VALUES)
		return false;
	if (restitutionCombineMode >= NX_CM_N_VALUES)
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
