#ifndef NX_COLLISION_NXWHEELSHAPEDESC
#define NX_COLLISION_NXWHEELSHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxShapeDesc.h"
#include "NxSpringDesc.h"
class NxUserWheelContactModify;

/**
\brief Class used to describe tire properties for a #NxWheelShape

A tire force function takes a measure for tire slip as input, (this is computed differently for lateral and longitudal directions)
and gives a measure of grip as output.

The exact interpretation of the output value will depend on the model used.

In the default model the frictional constraints are modeled as springs.
The output from the tire force function is interpreted as a normalized spring coefficient.
The actual spring coefficient is computed by scaling the output with the normal impulse 
and the stiffness parameter.
Note that this implies that the material of the "ground" and the wheel do not affect the grip level.

In the clamped friction model the output from the tire force function is interpreted as friction
coefficients. The maximum friction impulse available to maintain the constraint is computed 
by scaling the output with the normal impulse.
Note that the stiffness parameter is ignored in this model.
Note that this implies that the material of the "ground" and the wheel do not affect the grip level.
See #NX_WF_CLAMPED_FRICTION.

In the legacy friction model the output from the tire force functions is ignored.
The combined material properties of the wheel and the "ground" determine the traction budget.
See #NX_WF_EMULATE_LEGACY_WHEEL.

The default model and the clamped friction model use a different mechanism for low speed friction.
The low speed friction model is activated if the contact point with the ground is within skinWidth 
of its previous position. In this mode the wheel contacts are modeled as static friction contacts.
<br>The maximum friction force in each direction is:
<br>(friction force) = mu * (normal force).
<br>Where,
<br>mu = NxTireFunctionDesc::extremumValue * NxTireFunctionDesc::stiffness (for the default model)
<br>mu = NxTireFunctionDesc::extremumValue (for the clamped friction model)

The curve is approximated by a two piece cubic Hermite spline.  The first section goes from (0,0) to (extremumSlip, extremumValue), at which 
point the curve's tangent is zero.

The second section goes from (extremumSlip, extremumValue) to (asymptoteSlip, asymptoteValue), at which point the curve's tangent is again zero.

\image html wheelGraph.png


<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes (Software fallback for collision)
\li PS3  : Yes
\li XB360: Yes

See #NxWheelShape.
*/

/*
Force
^		extremum
|    _*_
|   ~   \     asymptote
|  /     \~__*______________
| /
|/
---------------------------> Slip
*/
class NxTireFunctionDesc
	{
	public:

	virtual ~NxTireFunctionDesc(){};
	
	/**
	\brief extremal point of curve.  Both values must be positive.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0
	*/
	NxReal extremumSlip;

	/**
	\brief extremal point of curve.  Both values must be positive.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 0.02
	*/
	NxReal extremumValue;

	/**
	\brief point on curve at which for all x > minumumX, function equals minimumY.  Both values must be positive.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 2.0
	*/
	NxReal asymptoteSlip;
	
	/**
	\brief point on curve at which for all x > minumumX, function equals minimumY.  Both values must be positive.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 0.01
	*/
	NxReal asymptoteValue;


	/**
	\brief Scaling factor for tire force.
	
	This is an additional overall positive scaling that gets applied to the tire forces before passing 
	them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may 
	need to lower this. A setting of zero will disable all friction in this direction.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1000000.0 (quite stiff by default)
	*/
	NxReal stiffnessFactor;


	/**
	\brief Scaling factor for tire force.
	
	This is an additional overall positive scaling that gets applied to the tire forces before passing 
	them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may 
	need to lower this. A setting of zero will disable all friction in this direction.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1000000.0 (quite stiff by default)
	*/
	//NxReal stiffnessFactor;


	/**
	constructor sets to default.
	*/
	NX_INLINE					NxTireFunctionDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;

	/**
	evaluates the Force(Slip) function
	*/
	NX_INLINE NxReal hermiteEval(NxReal t) const;
	};

enum NxWheelShapeFlags
	{
	/**
	\brief Determines whether the suspension axis or the ground contact normal is used for the suspension constraint.

	*/
	NX_WF_WHEEL_AXIS_CONTACT_NORMAL = 1 << 0,
	
	/**
	\brief If set, the laterial slip velocity is used as the input to the tire function, rather than the slip angle.

	*/
	NX_WF_INPUT_LAT_SLIPVELOCITY = 1 << 1,
	
	/**
	\brief If set, the longutudal slip velocity is used as the input to the tire function, rather than the slip ratio.  
	*/
	NX_WF_INPUT_LNG_SLIPVELOCITY = 1 << 2,

	/**
	\brief If set, does not factor out the suspension travel and wheel radius from the spring force computation.  This is the legacy behavior from the raycast capsule approach.
	*/
	NX_WF_UNSCALED_SPRING_BEHAVIOR = 1 << 3, 

	/**
	\brief If set, the axle speed is not computed by the simulation but is rather expected to be provided by the user every simulation step via NxWheelShape::setAxleSpeed().
	*/
	NX_WF_AXLE_SPEED_OVERRIDE = 1 << 4,

	/**
	\brief If set, the NxWheelShape will emulate the legacy raycast capsule based wheel.
	See #NxTireFunctionDesc
	*/
	NX_WF_EMULATE_LEGACY_WHEEL = 1 << 5,

	/**
	\brief If set, the NxWheelShape will clamp the force in the friction constraints.
	See #NxTireFunctionDesc
	*/
	NX_WF_CLAMPED_FRICTION = 1 << 6,
	};


/**
 \brief Descriptor for an #NxWheelShape.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes (Software fallback for collision)
\li PS3  : Yes
\li XB360: Yes

 @see NxWheelShape NxActor.createActor()
*/
class NxWheelShapeDesc : public NxShapeDesc		//TODO: this nor other desc classes can be assigned with = operator, we need to define copy ctors. 
	{
	public:

	virtual ~NxWheelShapeDesc(){};

//geometrical constants:
	
	/**
	\brief distance from wheel axle to a point on the contact surface.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0

	@see NxWheelShape.getRadius() NxWheelShape.setRadius()
	*/
	NxReal radius;
	
	/**
	\brief maximum extension distance of suspension along shape's -Y axis.
	
	The minimum extension is always 0.  

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 1.0

	@see NxWheelShape.setSuspensionTravel() NxWheelShape.getSuspensionTravel()
	*/
	NxReal suspensionTravel;

//(In the old model the capsule height was the sum of the two members above.)
//^^^ this may be redundant together with suspension.targetValue, not sure yet.

//simulation constants:
	
	/**
	\brief data intended for car wheel suspension effects.  
	
	targetValue must be in [0, 1], which is the rest length of the suspension along the suspensionTravel.  
	0 is the default, which maps to the tip of the ray cast.

	<b>Range:</b> .spring [0,inf)<br>
	<b>Range:</b> .damper [0,inf)<br>
	<b>Range:</b> .targetValue [0,1]<br>
	<b>Default:</b> See #NxSpringDesc


	@see NxSpringDesc NxWheelShape.setSuspension() NxWheelShape.getSuspension()
	*/
	NxSpringDesc suspension;

	/**
	\brief cubic hermite spline coefficients describing the longitudal tire force curve.

	<b>Range:</b> See #NxTireFunctionDesc<br>
	<b>Default:</b> See #NxTireFunctionDesc

	@see NxWheelShape.getLongitudalTireForceFunction() NxWheelShape.setLongitudalTireForceFunction()
	*/
	NxTireFunctionDesc longitudalTireForceFunction;
	
	/**
	\brief cubic hermite spline coefficients describing the lateral tire force curve.

	<b>Range:</b> See #NxTireFunctionDesc<br>
	<b>Default:</b> See #NxTireFunctionDesc

	@see NxWheelShape.getLateralTireForceFunction() NxWheelShape.setLateralTireForceFunction()
	*/
	NxTireFunctionDesc lateralTireForceFunction;
	
	/**
	\brief inverse mass of the wheel.
	
	Determines the wheel velocity that wheel torques can achieve.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 1.0

	@see NxWheelShape.setInverseWheelMass() NxWheelShape.getInverseWheelMass()
	*/
	NxReal	inverseWheelMass;
	
	/**
	\brief flags from NxWheelShapeFlags

	<b>Default:</b> 0

	@see NxWheelShapeFlags NxWheelShape.getWheelFlags() NxWheelShape.setWheelFlags()
	*/
	NxU32 wheelFlags;

//dynamic inputs:
	
	/**
	\brief Sum engine torque on the wheel axle.
	
	Positive or negative depending on direction.

	<b>Range:</b> (-inf,inf)<br>
	<b>Default:</b> 0.0

	@see NxWheelShape.setMotorTorque() NxWheelShape.getMotorTorque() brakeTorque
	*/
	NxReal motorTorque;
	
	/**
	\brief The amount of torque applied for braking.
	
	Must be nonnegative.  Very large values should lock wheel but should be stable.

	<b>Range:</b> [0,inf)<br>
	<b>Default:</b> 0.0

	@see NxWheelShape.setBrakeTorque() NxWheelShape.getBrakeTorque() motorTorque
	*/
	NxReal brakeTorque;
	
	/**
	\brief steering angle, around shape Y axis.

	<b>Range:</b> (-PI,PI)<br>
	<b>Default:</b> 0.0

	@see NxWheelShape.setSteerAngle() NxWheelShape.getSteerAngle()
	*/
	NxReal steerAngle;

	/**
	\brief callback used for modifying the wheel contact point before the wheel constraints are created.

	<b>Default:</b> NULL

	@see NxUserWheelContactModify
	*/
	NxUserWheelContactModify* wheelContactModify;

	/**
	constructor sets to default.
	*/
	NX_INLINE					NxWheelShapeDesc();	
	/**
	\brief (re)sets the structure to the default.
	 \param[in] fromCtor Avoid redundant work if called from constructor.
	*/
	NX_INLINE virtual	void	setToDefault(bool fromCtor = false);
	/**
	\brief returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};


NX_INLINE NxTireFunctionDesc::NxTireFunctionDesc()
	{
	setToDefault();
	}

NX_INLINE void NxTireFunctionDesc::setToDefault()
	{
	extremumSlip = 1.0f;
	extremumValue = 0.02f;
	asymptoteSlip = 2.0f;
	asymptoteValue = 0.01f;	
	stiffnessFactor = 1000000.0f;	//quite stiff by default.
	}

NX_INLINE bool NxTireFunctionDesc::isValid() const
	{
	if(!(0.0f < extremumSlip))			return false;
	if(!(extremumSlip < asymptoteSlip))	return false;
	if(!(0.0f < extremumValue))			return false;
	if(!(0.0f < asymptoteValue))		return false;
	if(!(0.0f <= stiffnessFactor))		return false;

	return true;
	}


NX_INLINE NxReal NxTireFunctionDesc::hermiteEval(NxReal t) const
	{
	
	// This fix for TTP 3429 & 3675 is from Sega.
	// Assume blending functions (look these up in a graph):
	// H0(t) =  2ttt - 3tt + 1
	// H1(t) = -2ttt + 3tt
	// H2(t) =   ttt - 2tt + t
	// H3(t) =   ttt -  tt 

	NxReal v = NxMath::abs(t);
	NxReal s = (t>=0) ? 1.0f : -1.0f;

	NxReal F;

	if(v<extremumSlip)
		{
		// For t in the interval 0 < t < extremumSlip
		// We normalize t:
		// a = t/extremumSlip;
		// and use H1 + H2 to compute F:
		// F = extremumValue * ( H1(a) + H2(a) )

		NxReal a = v/extremumSlip;
		NxReal a2 = a*a;
		NxReal a3 = a*a2;

		F = extremumValue * (-a3 + a2 + a);
		}
	else
		{
		if(v<asymptoteSlip)
			{
			// For the interval extremumSlip <= t < asymtoteSlip
			// We normalize and remap t:
			// a = (t-extremumSlip)/(asymptoteSlip - extremumSlip)
			// and use H0 to compute F:
			// F = extremumValue + (extremumValue - asymtoteValue) * H0(a)
			// note that the above differs from the actual expression but this is how it looks with H0 factorized.
				
			NxReal a = (v-extremumSlip)/(asymptoteSlip - extremumSlip);
			NxReal a2 = a*a;
			NxReal a3 = a*a2;

			NxReal diff = asymptoteValue - extremumValue;
			F = -2.0f*diff*a3 + 3.0f*diff*a2 + extremumValue;
			}
		else
			{
			F = asymptoteValue;
			}
		}
	return s*F;
	}


NX_INLINE NxWheelShapeDesc::NxWheelShapeDesc() : NxShapeDesc(NX_SHAPE_WHEEL)	//constructor sets to default
	{
	setToDefault(true);
	}

	NX_INLINE void NxWheelShapeDesc::setToDefault(bool fromCtor)
	{
	NxShapeDesc::setToDefault();

	radius = 1.0f;				
	suspensionTravel = 1.0f;	
	//simulation constants:
	inverseWheelMass = 1.0f;
	wheelFlags = 0;
	//dynamic inputs:
	motorTorque = 0.0f;
	brakeTorque = 0.0f;
	steerAngle = 0.0f;
	wheelContactModify = NULL;
 	if (!fromCtor)
 		{
 		suspension.setToDefault();
 		longitudalTireForceFunction.setToDefault();
 		lateralTireForceFunction.setToDefault();
 		}
	}

NX_INLINE bool NxWheelShapeDesc::isValid() const
	{
	if(!NxMath::isFinite(radius))	return false;
	if(radius<=0.0f)				return false;
	if(!NxMath::isFinite(suspensionTravel))	return false;
	if(suspensionTravel<0.0f)				return false;
	if(!NxMath::isFinite(inverseWheelMass))	return false;
	if(inverseWheelMass<=0.0f)				return false;
	if(!NxMath::isFinite(motorTorque))	return false;
	if(!NxMath::isFinite(brakeTorque))	return false;
	if(brakeTorque<0.0f)				return false;
	if(!NxMath::isFinite(steerAngle))	return false;

	if (!suspension.isValid()) return false;
	if (!longitudalTireForceFunction.isValid()) return false;
	if (!lateralTireForceFunction.isValid()) return false;


	return NxShapeDesc::isValid();
	}
#endif

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
