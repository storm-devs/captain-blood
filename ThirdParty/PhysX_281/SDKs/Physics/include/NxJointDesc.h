#ifndef NX_PHYSICS_NXJOINTDESC
#define NX_PHYSICS_NXJOINTDESC
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
#include "NxJoint.h"
#include "NxActor.h"
#include "PhysXLoader.h"
#include "NxUtilLib.h"

/**
\brief Descriptor class for the #NxJoint class.

Joint descriptors for all the different joint types are derived from this class.

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes
\li PS3  : Yes
\li XB360: Yes

@see NxJoint NxScene.createJoint()
@see NxCylindricalJointDesc NxD6JointDesc NxDistanceJointDesc NxFixedJointDesc NxPointInPlaneJointDesc
NxPointOnLineJointDesc NxPrismaticJointDesc NxPulleyJointDesc NxRevoluteJointDesc NxSphericalJointDesc
*/
class NxJointDesc
	{
	protected:

	/**
	\brief The type of joint. This is set by the c'tor of the derived class.
	*/
	NxJointType type;
	public:	

	/**
	\brief The two actors connected by the joint.
	
	The actors must be in the same scene as this joint.
	
	At least one of the two pointers must be a dynamic actor.
	
	One of the two may be NULL to indicate the world frame. Neither may be a static actor!

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor
	*/
	NxActor * actor[2];

	/**
	\brief X axis of joint space, in actor[i]'s space, orthogonal to localAxis[i]

	#localAxis and localNormal should be unit length and at right angles to each other, i.e. 
	dot(localNormal[0],localAxis[0])==0 and dot(localNormal[1],localAxis[1])==0.

	<b>Range:</b> direction vector<br>
	<b>Default:</b> [0] 1.0f,0.0f,0.0f<br>
	<b>Default:</b> [1] 1.0f,0.0f,0.0f

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see localAxis setGlobalAxis()
	*/
	NxVec3 localNormal[2];

	/**
	\brief Z axis of joint space, in actor[i]'s space. This is the primary axis of the joint.

	localAxis and #localNormal should be unit length and at right angles to each other, i.e. 
	dot(localNormal[0],localAxis[0])==0 and dot(localNormal[1],localAxis[1])==0.

	<b>Range:</b> direction vector<br>
	<b>Default:</b> [0] 0.0f,0.0f,1.0f<br>
	<b>Default:</b> [1] 0.0f,0.0f,1.0f

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see localNormal setGlobalAxis()
	*/
	NxVec3 localAxis[2];
	
	/**
	\brief Attachment point of joint in actor[i]'s space

	<b>Range:</b> position vector<br>
	<b>Default:</b> [0] Zero<br>
	<b>Default:</b> [1] Zero

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalAnchor()
	*/
	NxVec3 localAnchor[2];

	/**
	Maximum linear force that the joint can withstand before breaking, must be positive.
	
	<b>Range:</b> (0,inf]<br>
	<b>Default:</b> NX_MAX_REAL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint.setBreakable() NxUserNotify.onJointBreak()
	*/
	NxReal maxForce;

	/**
	\brief Maximum angular force (torque) that the joint can withstand before breaking, must be positive.
	
	<b>Range:</b> (0,inf]<br>
	<b>Default:</b> NX_MAX_REAL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint.setBreakable() NxUserNotify.onJointBreak()
	*/
	NxReal maxTorque;

	/**
	\brief Extrapolation factor for solving joint constraints.

	This parameter can be used to build stronger joints and increase the solver convergence. Higher values
	lead to stronger joints.

	\note Setting the value too high can decrease the joint stability.

	\note Currently, this feature is supported for D6, Revolute and Spherical Joints only.
	
	<b>Range:</b> [0.5,2]<br>
	<b>Default:</b> 1

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint.setSolverExtrapolationFactor(), NxJoint.getSolverExtrapolationFactor()
	*/
	NxReal solverExtrapolationFactor;

	/**
	\brief Switch to acceleration based spring.

	This parameter can be used to switch between acceleration and force based spring. Acceleration
	based springs do not take the mass of the attached objects into account, i.e., the spring/damping
	behaviour will be independent of the load.

	\note Currently, this feature is supported for D6, Revolute and Spherical Joints only.
	
	<b>Range:</b> {0: use force spring, 1: use acceleration spring}<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint.setUseAccelerationSpring(), NxJoint.getUseAccelerationSpring()
	*/
	NxU32 useAccelerationSpring;
	
	/**
	\brief Will be copied to NxJoint::userData.

	<b>Default:</b> NULL

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	void* userData;

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
	\brief This is a combination of the bits defined by ::NxJointFlag .

	<b>Default:</b> NX_JF_VISUALIZATION

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NxU32 jointFlags;


	NX_INLINE virtual		~NxJointDesc();
	
	/**
	\brief (re)sets the structure to the default.	
	*/
	NX_INLINE virtual void setToDefault();
	
	/**
	\brief Returns true if the descriptor is valid.

	\return true if the current settings are valid
	*/
	NX_INLINE virtual bool isValid() const;


	/**
	\brief Set the localAnchor[] members using a world space point.

	sets the members localAnchor[0,1] by transforming the passed world space
	vector into actor1 resp. actor2's local space. The actor pointers must already be set!

	\param[in] wsAnchor Global frame anchor point. <b>Range:</b> position vector

	@see setGlobalAxis() localAxis
	*/
	NX_INLINE void setGlobalAnchor(const NxVec3 & wsAnchor);

	/**
	\brief Set the local axis/normal using a world space axis.

	sets the members localAxis[0,1] by transforming the passed world space
	vector into actor1 resp. actor2's local space, and finding arbitrary orthogonals for localNormal[0,1].
	The actor pointers must already be set!

	\param[in] wsAxis Global frame axis. <b>Range:</b> direction vector

	@see setGlobalAnchor() localAnchor
	*/
	NX_INLINE void setGlobalAxis(const NxVec3 & wsAxis);

	/**
	\brief Retrieves the joint type.

	\return The type of joint this descriptor describes. See #NxJointType.

	@see NxJointType
	*/
	NX_INLINE NxJointType	getType()	const	{ return type; }
	protected:
	/**
	\brief Constructor sets to default.

	\param t Joint type
	*/
	NX_INLINE NxJointDesc(NxJointType t);	
	};


NX_INLINE NxJointDesc::NxJointDesc(NxJointType t) : type(t)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE NxJointDesc::~NxJointDesc()
	{
	}

NX_INLINE void NxJointDesc::setToDefault()
	{
	for (int i=0; i<2; i++)
		{
		actor[i] = 0;
		localAxis[i].set(0,0,1);
		localNormal[i].set(1,0,0);
		localAnchor[i].zero();
		}

	maxForce	= NX_MAX_REAL;
	maxTorque	= NX_MAX_REAL;
	solverExtrapolationFactor = 1.0f;
	useAccelerationSpring = 0;
	userData	= NULL;
	name		= NULL;
	jointFlags	= NX_JF_VISUALIZATION;
	}

NX_INLINE bool NxJointDesc::isValid() const
	{
	if (actor[0] == actor[1])
		return false;
	if (!(actor[0] || actor[1]))
		return false;
	//non-null pointers must be dynamic:
	if (actor[0] && ! actor[0]->isDynamic())
		return false;
	if (actor[1] && ! actor[1]->isDynamic())
		return false;

	if (type >= NX_JOINT_COUNT)
		return false;
	for (int i=0; i<2; i++)
		{
		if (fabsf(localAxis[i].magnitudeSquared() - 1.0f) > 0.1f) return false;
		if (fabsf(localNormal[i].magnitudeSquared() - 1.0f) > 0.1f) return false;
		//check orthogonal pairs
		if (fabsf(localAxis[i].dot(localNormal[i])) > 0.1f) return false;
		}
	if (maxForce <= 0)
		return false;
	if (maxTorque <= 0)
		return false;
	if ((solverExtrapolationFactor < 0.5f) || (solverExtrapolationFactor > 2.0f))
		return false;
	if (useAccelerationSpring > 1)
		return false;

	return true;
	}

NX_INLINE void NxJointDesc::setGlobalAnchor(const NxVec3 & wsAnchor)
	{
	NxGetUtilLib()->NxJointDesc_SetGlobalAnchor(*this, wsAnchor);
	}

NX_INLINE void NxJointDesc::setGlobalAxis(const NxVec3 & wsAxis)
	{
	NxGetUtilLib()->NxJointDesc_SetGlobalAxis(*this, wsAxis);
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
