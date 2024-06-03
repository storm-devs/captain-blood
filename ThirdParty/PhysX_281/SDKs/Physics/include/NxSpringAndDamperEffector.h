#ifndef NX_PHYSICS_NXSPRINGANDDAMPEREFFECTOR
#define NX_PHYSICS_NXSPRINGANDDAMPEREFFECTOR
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
#include "NxEffector.h"

class NxActor;
class NxEffector;
class NxSpringAndDamperEffectorDesc;

/**
\brief  Represents a spring and damper element, which exerts a force between two bodies,
 proportional to the relative positions and the relative velocities of the bodies.

\note The spring and damper element is deprecated and the distance joint should be used in its place, for the following reasons:

\li Spring and Damper elements are not as numerically stable as genuine joints.
\li Joints have proper sleep behavior, bodies fall asleep and wake up at appropriate times with regard to forces being transfered across the joints spring. 


 <h3>Creation</h3>

 Example:

 \include NxSpringAndDamperEffector_Create.cpp

 @see NxScene.createSpringAndDamperEffector() NxScene.releaseEffector()
*/
class NxSpringAndDamperEffector: public NxEffector
	{
	public:
	/**
	\brief Writes all of the effector's spring attributes to the description, as well
	as setting the 2 actor connection points.

	\param[out] desc The descriptor used to retrieve the state of the effector.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void saveToDesc(NxSpringAndDamperEffectorDesc &desc) = 0;

	/**
	\brief Sets the two bodies which are connected by the element.

	You may set one of the bodies to NULL to signify that the effect is between a body and the static 
	environment.

	Setting both of the bodies to NULL is invalid.

	Each body parameter is followed by a point defined in the global coordinate frame, which will
	move with the respective body. This is the point where the respective end of the spring and damper
	element is attached to the body.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] body1 First Body.
	\param[in] global1 Attachment point for spring in the global frame. <b>Range:</b> position vector
	\param[in] body2 Second Body
	\param[in] global2 Attachment point for spring in the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void setBodies(NxActor* body1, const NxVec3  & global1, NxActor* body2, const NxVec3  & global2) = 0;

	/**
	\brief Sets the properties of the linear spring.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.
	
	The first three parameters are stretch distances between the end points:
	
	The following has to hold: 0 <= distCompressSaturate <= distRelaxed <= distStretchSaturate and
	0 <= maxCompressForce, 0 <= maxStretchForce.

	The last two parameters are maximal force magnitudes. Set maxCompressForce to zero to disable the 
	compress phase. Set maxStretchForce  to zero to disable the stretch phase.


	\param[in] distCompressSaturate is the distance at which the repulsive spring force magnitude no 
	longer increases, but stays constant. <b>Range:</b> [0,springDistRelaxed]

	\param[in] distRelaxed is the distance at which the spring is relaxed, and there is no spring force 
	applied. <b>Range:</b> [springDistCompressSaturate,springDistStretchSaturate]

	\param[in] distStretchSaturate is the distance at which the attractive spring force magnitude no 
	longer increases, but stays constant. <b>Range:</b> [springDistCompressSaturate,inf)

	\param[in] maxCompressForce is the force applied when distCompressSaturate is reached. The force 
	ramps up linearly until this value, starting at zero at distRelaxed. <b>Range:</b> [0,inf)
	
	\param[in] maxStretchForce  is the force applied when distStretchSaturate  is reached. The force 
	ramps up linearly until this value, starting at zero at distRelaxed. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void setLinearSpring(NxReal distCompressSaturate, NxReal distRelaxed, NxReal distStretchSaturate, NxReal maxCompressForce, NxReal maxStretchForce) = 0;

	/**
	\brief Retrieves the spring properties. See #setLinearSpring.

	\param[out] distCompressSaturate is the distance at which the repulsive spring force magnitude no longer increases, but stays constant.
	\param[out] distRelaxed is the distance at which the spring is relaxed, and there is no spring force applied.
	\param[out] distStretchSaturate is the distance at which the attractive spring force magnitude no longer increases, but stays constant.
	
	\param[out] maxCompressForce is the force applied when distCompressSaturate is reached. The force ramps up linearly until this value, starting at zero at distRelaxed.
	\param[out] maxStretchForce  is the force applied when distStretchSaturate  is reached. The force ramps up linearly until this value, starting at zero at distRelaxed.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void getLinearSpring(NxReal & distCompressSaturate, NxReal & distRelaxed, NxReal & distStretchSaturate, NxReal & maxCompressForce, NxReal & maxStretchForce) = 0;

	/**
	\brief Sets the properties of the linear damper.

	The first two parameters are relative body velocities and the last two parameters are maximal force 
	magnitudes.

	The following has to hold: velCompressSaturate <= 0 <= velStretchSaturate and 0 <= maxCompressForce; 0 <= maxStretchForce

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	\param[in] velCompressSaturate is the negative (compression direction) velocity where the damping 
	force magnitude no longer increases, but stays constant. <b>Range:</b> (-inf,0]
	
	\param[in] velStretchSaturate  is the positive (stretch direction) velocity where the the damping 
	force magnitude no longer increases, but stays constant. <b>Range:</b> [0,inf)

	\param[in] maxCompressForce is the force applied when velCompressSaturate is reached. The force 
	ramps up linearly until this value, starting at zero at vrel == 0. <b>Range:</b> [0,inf)

	\param[in] maxStretchForce  is the force applied when velStretchSaturate  is reached. The force 
	ramps up linearly until this value, starting at zero at vrel == 0. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void setLinearDamper(NxReal velCompressSaturate, NxReal velStretchSaturate, NxReal maxCompressForce, NxReal maxStretchForce) = 0;

	/**
	\brief Retrieves the damper properties.

	Retrieves the damper properties. See #setLinearDamper.

	\param[out] velCompressSaturate is the negative (compression direction) velocity where the damping force magnitude no longer increases, but stays constant. 
	\param[out] velStretchSaturate  is the positive (stretch direction) velocity where the the damping force magnitude no longer increases, but stays constant. 

	\param[out] maxCompressForce is the force applied when velCompressSaturate is reached. The force ramps up linearly until this value, starting at zero at vrel == 0. 
	\param[out] maxStretchForce  is the force applied when velStretchSaturate  is reached. The force ramps up linearly until this value, starting at zero at vrel == 0.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void getLinearDamper(NxReal & velCompressSaturate, NxReal & velStretchSaturate, NxReal & maxCompressForce, NxReal & maxStretchForce) = 0;
	
	protected:
	virtual ~NxSpringAndDamperEffector(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
