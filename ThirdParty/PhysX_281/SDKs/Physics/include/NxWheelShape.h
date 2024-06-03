#ifndef NX_COLLISION_NXWHEELSHAPE
#define NX_COLLISION_NXWHEELSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxShape.h"
#include "NxSpringDesc.h"
#include "NxWheelShapeDesc.h"

class NxShape;
class NxWheelShapeDesc;

/**
\brief Contact information used by NxWheelShape

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxWheelShape NxWheelShape.getContact()
*/
class NxWheelContactData
	{
	public:
	/**
	\brief The point of contact between the wheel shape and the ground.

	*/
	NxVec3 contactPoint;

	/**
	\brief The normal at the point of contact.

	*/
	NxVec3 contactNormal;
	
	/**
	\brief The direction the wheel is pointing in.
	*/
	NxVec3 longitudalDirection;

	/**
	\brief The sideways direction for the wheel(at right angles to the longitudinal direction).
	*/
	NxVec3 lateralDirection;

	/**
	\brief The magnitude of the force being applied for the contact.
	*/
	NxReal contactForce;

	/**
	\brief What these exactly are depend on NX_WF_INPUT_LAT_SLIPVELOCITY and NX_WF_INPUT_LNG_SLIPVELOCITY flags for the wheel.
	*/
	NxReal longitudalSlip, lateralSlip;

	/**
	\brief the clipped impulses applied at the wheel.
	*/
	NxReal longitudalImpulse, lateralImpulse;

	/**
	\brief The material index of the shape in contact with the wheel.

	@see NxMaterial NxMaterialIndex
	*/
	NxMaterialIndex otherShapeMaterialIndex;

	/**
	\brief The distance on the spring travel distance where the wheel would end up if it was resting on the contact point.
	*/
	NxReal contactPosition;
	};

/**
\brief A special shape used for simulating a car wheel.

The -Y axis should be directed toward the ground.  A ray is cast from the shape's origin along the -Y axis.  

When the ray strikes something, and the distance is:

	\li less than wheelRadius from the shape origin: a hard contact is created
	\li between wheelRadius and (suspensionTravel + wheelRadius): a soft suspension contact is created
	\li greater than (suspensionTravel + wheelRadius): no contact is created.

Thus at the point of greatest possible suspension compression the wheel axle will pass through at the shape's origin.
At the point greatest suspension extension the wheel axle will be a distance of suspensionTravel from the shape's origin.

The suspension's targetValue is 0 for real cars, which means that the suspension tries to extend all the way.  Otherwise one can specify values
[0,1] for suspensions which have a spring to pull the wheel up when it is extended too far.  0.5 will then fall halfway along suspensionTravel.

The +Z axis is the 'forward' direction of travel for the wheel.  -Z is backwards.
The wheel rolls forward when rotating around the positive direction around the X axis.

A positive wheel steering angle corresponds to a positive rotation around the shape's Y axis.  (Castor angles are not modeled.)

The coordinate frame of the shape is rigidly fixed on the car.  

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes (Software fallback for collision)
\li PS3  : Yes
\li XB360: Yes

<h3>Visualizations</h3>
\li NX_VISUALIZE_COLLISION_AABBS
\li NX_VISUALIZE_COLLISION_SHAPES
\li NX_VISUALIZE_COLLISION_AXES

@see NxShape NxWheelShapeDesc
*/

class NxWheelShape : public NxShape
	{
	public:

	/**
	\brief Saves the state of the shape object to a descriptor.

	\param[out] desc Descriptor to retrieve shape properties.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxWheelShapeDesc
	*/
	virtual	void	saveToDesc(NxWheelShapeDesc& desc)		const = 0;

//accessors for all the stuff in the descriptor go here.
//geometrical constants:
	
	/**
	\brief distance from wheel axle to a point on the contact surface.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] radius The new wheel radius. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getRadius()
	*/
	virtual	void setRadius(NxReal radius) = 0;
	
	/**
	\brief maximum extension distance of suspension along shape's -Y axis.  The minimum extension is always 0.  

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] travel The suspension travel. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSuspensionTravel()
	*/
	virtual	void setSuspensionTravel(NxReal travel) = 0;


	/**
	\brief distance from wheel axle to a point on the contact surface.

	\return The wheel radius.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setRadius()
	*/
	virtual	NxReal getRadius() const = 0;
	
	/**
	\brief maximum extension distance of suspension along shape's -Y axis.  The minimum extension is always 0.  

	\return The suspension travel.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setSuspensionTravel()
	*/
	virtual	NxReal getSuspensionTravel() const = 0;

	//simulation constants:
	/**
	\brief data intended for car wheel suspension effects.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] spring Suspension spring properties. 
	<b>Range:</b> .spring [0,inf)<br>
	<b>Range:</b> .damper [0,inf)<br>
	<b>Range:</b> .targetValue [0,1]<br>

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSuspension() NxSpringDesc
	*/
	virtual	void setSuspension(NxSpringDesc spring) = 0;
	
	/**
	\brief cubic Hermite spline coefficients describing the longitudinal tire force curve.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] tireFunc Description of the longitudinal tire forces. <b>Range:</b> See #NxTireFunctionDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLongitudalTireForceFunction NxTireFunctionDesc
	*/
	virtual	void setLongitudalTireForceFunction(NxTireFunctionDesc tireFunc) = 0;
	
	/**
	\brief cubic Hermite spline coefficients describing the lateral tire force curve.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] tireFunc Description of the lateral tire forces. <b>Range:</b> See #NxTireFunctionDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLateralTireForceFunction() NxTireFunctionDesc
	*/
	virtual	void setLateralTireForceFunction(NxTireFunctionDesc tireFunc) = 0;
	
	/**
	\brief inverse mass of the wheel.  Determines the wheel velocity that wheel torques can achieve.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] invMass The inverse wheel mass. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getInverseWheelMass()
	*/
	virtual	void setInverseWheelMass(NxReal invMass) = 0;
	
	/**
	\brief flags from NxWheelShapeFlags

	\param[in] flags The new wheel flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxWheelShapeFlags getWheelFlags()
	*/
	virtual	void setWheelFlags(NxU32 flags) = 0;

	/**
	\brief data intended for car wheel suspension effects.

	\return Description of the suspension spring.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setSuspension() NxSpringDesc
	*/
	virtual	NxSpringDesc getSuspension() const = 0;
	
	/**
	\brief cubic Hermite spline coefficients describing the longitudinal tire force curve.

	\return Description of the longitudinal tire force.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTireFunctionDesc setLongitudalTireForceFunction()
	*/
	virtual	NxTireFunctionDesc getLongitudalTireForceFunction() const = 0;
	
	/**
	\brief cubic Hermite spline coefficients describing the lateral tire force curve.

	\return Description of the lateral tire force.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTireFunctionDesc setLateralTireForceFunction()
	*/
	virtual	NxTireFunctionDesc getLateralTireForceFunction() const = 0;
	
	/**
	\brief inverse mass of the wheel.  Determines the wheel velocity that wheel torques can achieve.

	\return Inverse wheel mass. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setInverseWheelMass()
	*/
	virtual	NxReal	getInverseWheelMass() const = 0;
	
	/**
	\brief flags from NxWheelShapeFlags

	\return The wheel flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxWheelShapeFlags setWheelFlags()
	*/
	virtual	NxU32	getWheelFlags() const = 0;

//dynamic inputs:
	/**
	\brief Sum engine torque on the wheel axle.  Positive or negative depending on direction.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] torque The motor torque to apply. <b>Range:</b> (-inf,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getMotorTorque()
	*/
	virtual	void setMotorTorque(NxReal torque) = 0;
	
	/**
	\brief Must be nonnegative.  Very large values should lock wheel but should be stable.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] torque The brake torque to apply. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getBrakeTorque()
	*/
	virtual	void setBrakeTorque(NxReal torque) = 0;
	
	/**
	\brief steering angle, around shape Y axis.

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] angle The new steering angle(in radians). <b>Range:</b> (-PI,PI)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSteerAngle()
	*/
	virtual	void setSteerAngle(NxReal angle) = 0;

	/**
	\brief Sum engine torque on the wheel axle.  Positive or negative depending on direction.

	\return The current motor torque.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setMotorTorque()
	*/
	virtual	NxReal getMotorTorque() const = 0;
	
	/**
	\brief Must be positive.  Very large values should lock wheel but should be stable.

	\return The current brake torque.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setBrakeTorque()
	*/
	virtual	NxReal getBrakeTorque() const = 0;
	
	/**
	\brief steering angle, around shape Y axis.

	\return The current steering angle(in radians)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setSteerAngle()
	*/
	virtual	NxReal getSteerAngle() const = 0;


//setting of internal state variables -- you normally don't want to do this!
	
	/**
	\brief Current axle rotation speed  NOTE: NX_WF_AXLE_SPEED_OVERRIDE flag must be raised for this to have effect!

	<b>Sleeping:</b> Does <b>NOT</b> wake any actors associated with the shape.

	\param[in] speed The new axle speed. <b>Range:</b> (-inf,inf)

	NOTE: NX_WF_AXLE_SPEED_OVERRIDE flag must be raised for this to have effect!

	An overridden axle speed of course renders the axle motor and brake torques ineffective.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getAxleSpeed()
	*/
	virtual	void setAxleSpeed(NxReal speed) = 0;

//readout of internal state variables:

	/**
	\brief Current axle rotation speed

	\return The current axle speed.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setAxleSpeed()
	*/
	virtual	NxReal getAxleSpeed() const = 0;

	/**
	\brief Return contact information for the wheel.

	when getContact() returns nonzero the wheel is in contact with another shape. (if there are several objects, this is the closest one hit by the wheel.)
	returns most recent contact normal and contact force for the wheel.  
	longitudalDirection and lateralDirection are tangential to the returned shape, and form a basis together with the contactNormal.
	otherShapeMaterialIndex is the material of the returned shape at the point of contact.  Because the tire model does not automatically respond to the material
	of objects it is in contact with (because it is an inherently different kind of material simulation) any adaptation of the tire force curves must be done by the
	user in response to the ground material returned here.

	\param[out] dest Description of the contact. (only valid if there is a contact)
	\return Returns the shape the wheel is in contact with. Or NULL if there is no contact.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback for collision)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxWheelContactData NxShape
	*/
	virtual NxShape * getContact(NxWheelContactData & dest) const = 0;	

	/**
	\brief Sets a callback function for wheel contact modification. NULL deactivates this functionality for the WheelShape.

	\param callback The callback function

	@see NxWheelShapeDesc.wheelContactModify NxUserWheelContactModify
	*/
	virtual		void							setUserWheelContactModify(NxUserWheelContactModify* callback) = 0;

	/**
	\brief Returns the callback function used for wheel contact modification on the WheelShape.

	\return The current wheel contact modification callback. NULL if no contact modification is being used.

	@see NxWheelShapeDesc.wheelContactModify NxUserWheelContactModify
	*/
	virtual		NxUserWheelContactModify*		getUserWheelContactModify() = 0;
	};


#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
