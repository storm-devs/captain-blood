#ifndef NX_PHYSICS_NXSPRINGANDDAMPEREFFECTORDESC
#define NX_PHYSICS_NXSPRINGANDDAMPEREFFECTORDESC
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
#include "NxEffectorDesc.h"

class NxActor;

/* TODO: This is not yet/anymore used for anything useful!! */

/**
\brief Desc class for NxSpringAndDamperEffector. 

<h3>Example</h3>

\include NxSpringAndDamperEffector_Create.cpp

@see NxSpringAndDamperEffector
*/
class NxSpringAndDamperEffectorDesc : public NxEffectorDesc
	{
	public:

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxSpringAndDamperEffectorDesc();	
	
	/**
	\brief (re)sets the structure to the default.	
	*/
	virtual NX_INLINE void setToDefault();
	
	/**
	\brief Returns true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	virtual NX_INLINE bool isValid() const;

	/**
	\brief First attached body.

	<b>Default:</b>	NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setBodies()
	*/
	NxActor* body1;

	/**
	\brief Second attached body.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setBodies()
	*/
	NxActor* body2;

	/**
	\brief First attachment point.

	<b>Range:</b> position vector<br>
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setBodies()
	*/
	NxVec3 pos1;

	/**
	\brief Second attachment point.

	<b>Range:</b> position vector<br>
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setBodies()
	*/
	NxVec3 pos2;

//linear spring parameters:
	/**
	\brief The distance at which the maximum repulsive force is attained (at shorter distances it remains the same).

	<b>Range:</b> [0,springDistRelaxed]<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearSpring() NxSpringAndDampererEffector.getLinearSpring()
	*/
	NxReal springDistCompressSaturate;
	
	/**
	\brief The distance at which the spring force is zero.

	<b>Range:</b> [springDistCompressSaturate,springDistStretchSaturate]<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearSpring() NxSpringAndDampererEffector.getLinearSpring()
	*/
	NxReal springDistRelaxed;
	
	/**
	\brief The distance at which the attractive spring force attains its maximum (farther away it remains the same).

	<b>Range:</b> [springDistCompressSaturate,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearSpring() NxSpringAndDampererEffector.getLinearSpring()
	*/
	NxReal springDistStretchSaturate;
	
	/**
	\brief The maximum repulsive spring force, attained at distance springDistCompressSaturate.

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearSpring() NxSpringAndDampererEffector.getLinearSpring()
	*/
	NxReal springMaxCompressForce;
	
	/**
	\brief The maximum attractive spring force, attained at distance springDistStretchSaturate.

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearSpring() NxSpringAndDampererEffector.getLinearSpring()
	*/
	NxReal springMaxStretchForce;

//linear damper parameters:
	
	/**
	\brief The relative velocity (negative) at which the repulsive damping force attains its maximum.

	<b>Range:</b> (-inf,0]<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearDamper() NxSpringAndDampererEffector.getLinearDamper()
	*/
	NxReal damperVelCompressSaturate;
	
	/**
	\brief The relative velocity at which the attractive damping force attains its maximum.
	
	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearDamper() NxSpringAndDampererEffector.getLinearDamper()
	*/
	NxReal damperVelStretchSaturate;
	
	/**
	\brief The maximum repulsive damping force, attained at relative velocity damperVelCompressSaturate

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearDamper() NxSpringAndDampererEffector.getLinearDamper()
	*/
	NxReal damperMaxCompressForce;
	
	/**
	\brief The maximum attractive damping force, attained at relative velocity damperVelStretchSaturate

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffector.setLinearDamper() NxSpringAndDampererEffector.getLinearDamper()
	*/
	NxReal damperMaxStretchForce;
	};

NX_INLINE NxSpringAndDamperEffectorDesc::NxSpringAndDamperEffectorDesc() : NxEffectorDesc(NX_EFFECTOR_SPRING_AND_DAMPER)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxSpringAndDamperEffectorDesc::setToDefault()
	{
	NxEffectorDesc::setToDefault();

	body1 = NULL;
    body2 = NULL;

	pos1.zero();
	pos2.zero();

	springDistCompressSaturate = 0;
	springDistRelaxed = 0;
	springDistStretchSaturate = 0;
	springMaxCompressForce = 0;
	springMaxStretchForce = 0;

	damperVelCompressSaturate = 0;
    damperVelStretchSaturate = 0;
	damperMaxCompressForce = 0;
	damperMaxStretchForce = 0;
	}

NX_INLINE bool NxSpringAndDamperEffectorDesc::isValid() const
	{
	return NxEffectorDesc::isValid();
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
