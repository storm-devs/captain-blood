#ifndef NX_PHYSICS_NXD6JOINTDESC
#define NX_PHYSICS_NXD6JOINTDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJointDesc.h"
#include "NxJointLimitSoftPairDesc.h"
#include "NxBitField.h"
#include "NxMotorDesc.h"

/** \addtogroup physics
  @{
*/





///////////////////////////////////////////////////////////

/**
\brief Descriptor class for the D6Joint. See #NxD6Joint.

 <p>In the D6Joint, the axes are assigned as follows:
 <ul>
 <li>x-axis = joint axis</li>
 <li>y-axis = joint normal axis</li>
 <li>z-axis = x-axis cross y-axis</li>
 </ul>
 These are defined relative to the parent body (0) of the joint.</p>

 <p>
 Swing is defined as the rotation of the x-axis with respect to the y- and z-axis.
 </p>

 <p>
 Twist is defined as the rotation about the x-axis.
 </p>

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Partial(some features not supported)
\li PS3  : Yes
\li XB360: Yes

 @see NxD6Joint NxJointDesc NxScene.createJoint() 
*/

class NxD6JointDesc : public NxJointDesc
{
public:
	
/* Constraints */
	/**
	\brief Define the linear degrees of freedom

	<b>Default:</b> NX_D6JOINT_MOTION_FREE

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxD6JointMotion xMotion yMotion zMotion
	*/
	NxD6JointMotion			xMotion, yMotion, zMotion; 

	/**
	\brief Define the angular degrees of freedom

	<b>Default:</b> NX_D6JOINT_MOTION_FREE

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxD6JointMotion swing1Motion swing2Motion twistMotion
	*/
	NxD6JointMotion			swing1Motion, swing2Motion, twistMotion;

	/**
	\brief If some linear DOF are limited, linearLimit defines the characteristics of these limits

	<b>Range:</b> See #NxJointLimitSoftDesc<br>
	<b>Default:</b> See #NxJointLimitSoftDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointLimitSoftDesc swing1Limit swing2Limit twistLimit
	*/
	NxJointLimitSoftDesc		linearLimit;

	/**
	\brief If swing1Motion is NX_D6JOINT_MOTION_LIMITED, swing1Limit defines the characteristics of the limit

	<b>Range:</b> See #NxJointLimitSoftDesc<br>
	<b>Default:</b> See #NxJointLimitSoftDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointLimitSoftDesc linearLimit swing2Limit twistLimit
	*/
	NxJointLimitSoftDesc		swing1Limit; 
	
	/**
	\brief If swing2Motion is NX_D6JOINT_MOTION_LIMITED, swing2Limit defines the characteristics of the limit

	<b>Range:</b> See #NxJointLimitSoftDesc<br>
	<b>Default:</b> See #NxJointLimitSoftDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointLimitSoftDesc linearLimit swing1Limit twistLimit
	*/
	NxJointLimitSoftDesc		swing2Limit;
	
	/**
	\brief If twistMotion is NX_D6JOINT_MOTION_LIMITED, twistLimit defines the characteristics of the limit

	<b>Range:</b> See #NxJointLimitSoftPairDesc<br>
	<b>Default:</b> See #NxJointLimitSoftDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointLimitSoftDesc linearLimit swing1Limit swing2imit
	*/
	NxJointLimitSoftPairDesc	twistLimit;

/* drive */
	
	/**
	\brief Drive the three linear DOF

	<b>Range:</b> See #NxJointDriveDesc<br>
	<b>Default:</b> See #NxJointDriveDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDriveDesc xDrive yDrive zDrive drivePosition
	*/
	NxJointDriveDesc		xDrive, yDrive, zDrive;

	/**
	\brief These drives are used if the flag NX_D6JOINT_SLERP_DRIVE is not set

	<b>Range:</b> See #NxJointDriveDesc<br>
	<b>Default:</b> See #NxJointDriveDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDriveDesc swingDrive twistDrive driveOrientation
	*/
	NxJointDriveDesc		swingDrive, twistDrive;

	/**
	\brief This drive is used if the flag NX_D6JOINT_SLERP_DRIVE is set

	<b>Range:</b> See #NxJointDriveDesc<br>
	<b>Default:</b> See #NxJointDriveDesc

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDriveDesc driveOrientation
	*/
	NxJointDriveDesc		slerpDrive;

	
	/**
	\brief If the type of xDrive (yDrive,zDrive) is NX_D6JOINT_DRIVE_POSITION, drivePosition defines the goal position

	<b>Range:</b> position vector<br>
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see xDrive yDrive zDrive NxD6Joint.setDrivePosition
	*/
	NxVec3					drivePosition;

	/**
	\brief If the type of swingDrive or twistDrive is NX_D6JOINT_DRIVE_POSITION, driveOrientation defines the goal orientation

	<b>Range:</b> unit quaternion<br>
	<b>Default:</b> Identity Quaternion

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see slerpDrive swingDrive twistDrive NxD6Joint.setDriveOrientation()
	*/
	NxQuat					driveOrientation;

	/**
	\brief If the type of xDrive (yDrive,zDrive) is NX_D6JOINT_DRIVE_VELOCITY, driveLinearVelocity defines the goal linear velocity

	<b>Range:</b> velocity vector<br>
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see xDrive yDrive zDrive NxD6Joint.setDriveLinearVelocity()
	*/
	NxVec3					driveLinearVelocity;
	
	/**
	\brief If the type of swingDrive or twistDrive is NX_D6JOINT_DRIVE_VELOCITY, driveAngularVelocity defines the goal angular velocity
	\li driveAngularVelocity.x - goal angular velocity about the twist axis
	\li driveAngularVelocity.y - goal angular velocity about the swing1 axis
	\li driveAngularVelocity.z - goal angular velocity about the swing2 axis

	<b>Range:</b> angular velocity vector<br>
	<b>Default:</b> Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see slerpDrive swingDrive twistDrive NxD6Joint.setDriveAngularVelocity()
	*/
	NxVec3					driveAngularVelocity;

	/**
	\brief If projectionMode is NX_JPM_NONE, projection is disabled. If NX_JPM_POINT_MINDIST, bodies are projected to limits leaving an linear error of projectionDistance and an angular error of projectionAngle

	<b>Default:</b> NX_JPM_NONE

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointProjectionMode projectionDistance projectionAngle
	*/
	NxJointProjectionMode	projectionMode;
	
	/**
	\brief The distance above which to project the joint.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 0.1

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see projectionMode projectionAngle
	*/
	NxReal					projectionDistance;	
	
	/**
	\brief The angle above which to project the joint.

	<b>Range:</b> (0,inf)<br>
	<b>Default:</b> 0.0872f (about 5 degrees in radians)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see projectionMode ProjectionDistance
	*/
	NxReal					projectionAngle;	

	/**
	\brief when the flag NX_D6JOINT_GEAR_ENABLED is set, the angular velocity of the second actor is driven towards the angular velocity of the first actor times gearRatio (both w.r.t. their primary axis)

	<b>Range:</b> (-inf,inf)<br>
	<b>Default:</b> 1.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flags
	*/
	NxReal					gearRatio;

	/**
	\brief This is a combination of the bits defined by ::NxD6JointFlag

	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial (Slerp drive not supported)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxD6JointFlag
	*/
	NxU32 flags;

	/**
	\brief constructor sets to default.
	*/

	NX_INLINE			NxD6JointDesc();

	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE	void	setToDefault();

	/**
	\brief Returns true if the descriptor is valid.

	\return returns true if the current settings are valid
	*/

	NX_INLINE	bool	isValid() const;
};

NxD6JointDesc::NxD6JointDesc() : NxJointDesc(NX_JOINT_D6)
{
	setToDefault();
}

void NxD6JointDesc::setToDefault() 
{
	NxJointDesc::setToDefault();

	xMotion = NX_D6JOINT_MOTION_FREE;
	yMotion = NX_D6JOINT_MOTION_FREE;
	zMotion = NX_D6JOINT_MOTION_FREE;
	twistMotion = NX_D6JOINT_MOTION_FREE;
	swing1Motion = NX_D6JOINT_MOTION_FREE;
	swing2Motion = NX_D6JOINT_MOTION_FREE;

	drivePosition.set(0,0,0);
	driveOrientation.id();

	driveLinearVelocity.set(0,0,0);
	driveAngularVelocity.set(0,0,0);

	projectionMode = NX_JPM_NONE;	// choose NX_JPM_POINT_MINDIST to turn projection on
	projectionDistance = 0.1f;
	projectionAngle = 0.0872f;	//about 5 degrees in radians.

	flags = 0;
	gearRatio = 1.0f;
}

bool NxD6JointDesc::isValid() const
{
	if (flags & NX_D6JOINT_SLERP_DRIVE) {	// only possible with all angular DOF available
		if (swing1Motion == NX_D6JOINT_MOTION_LOCKED ||
			swing2Motion == NX_D6JOINT_MOTION_LOCKED ||
			twistMotion == NX_D6JOINT_MOTION_LOCKED) return false;
	}

	// swing limits are symmetric, thus their range is 0..180 degrees
	if (swing1Motion == NX_D6JOINT_MOTION_LIMITED) {
		if (swing1Limit.value < 0.0f) return false;
		if (swing1Limit.value > NxPi) return false;
	}
	if (swing2Motion == NX_D6JOINT_MOTION_LIMITED) {
		if (swing2Limit.value < 0.0f) return false;
		if (swing2Limit.value > NxPi) return false;
	}

	// twist limits are asymmetric with -180 <= low < high <= 180 degrees
	if (twistMotion == NX_D6JOINT_MOTION_LIMITED) {
		if (twistLimit.low.value < -NxPi) return false;
		if (twistLimit.high.value > NxPi) return false;
		if (twistLimit.low.value > twistLimit.high.value) return false;
	}

	// in angular limited-free mode, only -90..90 swings are possible
	if (swing1Motion == NX_D6JOINT_MOTION_LIMITED && swing2Motion == NX_D6JOINT_MOTION_FREE) 
		if (swing1Limit.value > NxHalfPi) return false; 
	if (swing2Motion == NX_D6JOINT_MOTION_LIMITED && swing1Motion == NX_D6JOINT_MOTION_FREE) 
		if (swing2Limit.value > NxHalfPi) return false; 

	if (flags & NX_D6JOINT_GEAR_ENABLED)		// gear only with twist motion enabled
		if (twistMotion == NX_D6JOINT_MOTION_LOCKED) return false;

	return NxJointDesc::isValid();
}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
