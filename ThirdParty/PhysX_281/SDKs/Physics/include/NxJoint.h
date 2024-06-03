#ifndef NX_PHYSICS_NXJOINT
#define NX_PHYSICS_NXJOINT
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

class NxActor;
class NxScene;
class NxRevoluteJoint;
class NxPointInPlaneJoint;
class NxPointOnLineJoint;
class NxPrismaticJoint;
class NxCylindricalJoint;
class NxSphericalJoint;
class NxFixedJoint;
class NxDistanceJoint;
class NxPulleyJoint;
class NxD6Joint;

/**
 \brief Abstract base class for the different types of joints.

 All joints are used to connect two dynamic actors, or an actor and the environment.

 A NULL actor represents the environment. Whenever the below comments mention two actors,
 one of them may always be the environment (NULL).

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_JOINT_LOCAL_AXES
\li #NX_VISUALIZE_JOINT_WORLD_AXES
\li #NX_VISUALIZE_JOINT_LIMITS

<b>Platform:</b>
\li PC SW: Yes
\li PPU  : Yes (Up to 64k per scene)
\li PS3  : Yes
\li XB360: Yes

 @see NxJointDesc NxScene.createJoint()
 @see NxCylindricalJoint NxD6Joint NxDistanceJoint NxFixedJoint NxPointInPlaneJoint
NxPointOnLineJoint NxPrismaticJoint NxPulleyJoint NxRevoluteJointDesc NxSphericalJoint
*/
class NxJoint
	{
	protected:
	NX_INLINE					NxJoint() : userData(NULL), appData(NULL)
											{}
	virtual						~NxJoint()	{}

	public:

	/**
	\brief Retrieves the Actors involved.

	\param[out] actor1 First actor associated with joint.
	\param[out] actor2 Second actor associated with joint.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.actors NxActor
	*/
	virtual void getActors(NxActor** actor1, NxActor** actor2) = 0;

	/**
	\brief Sets the point where the two actors are attached, specified in global coordinates.

	Set this after setting the actors of the joint.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] vec Point the actors are attached at, specified in the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.setGlobalAnchor() getGlobalAnchor()
	*/
	virtual void setGlobalAnchor(const NxVec3 &vec) = 0;

	/**
	\brief Sets the direction of the joint's primary axis, specified in global coordinates.

	The direction vector should be normalized to unit length.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] vec Direction of primary axis in the global frame. <b>Range:</b> direction vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.setGlobalAxis() getGlobalAxis()
	*/
	virtual void setGlobalAxis(const NxVec3 &vec) = 0;

	/**
	\brief Retrieves the joint anchor.

	\return The joints anchor point in the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalAnchor() getGlobalAxis()
	*/
	virtual	NxVec3	getGlobalAnchor()	const  = 0;

	/**
	\brief Retrieves the joint axis.

	\return The joints axis in the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalAxis() getGlobalAnchor()
	*/
	virtual	NxVec3	getGlobalAxis()		const = 0;

	/**
	\brief Returns the state of the joint.

	Joints are created in the NX_JS_UNBOUND state. Making certain changes to the simulation or the joint 
	can also make joints become unbound.
	Unbound joints are automatically bound the next time Scene::run() is called, and this changes their
	state to NX_JS_SIMULATING. NX_JS_BROKEN means that a breakable joint has broken due to a large force
	or one of its actors has been deleted. In either case the joint was removed from the simulation, 
	so it should be released by the user to free up its memory.

	\return The state of the joint. See #NxJointState.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointState setBreakable()
	*/
	virtual NxJointState getState() = 0;
	
	/**
	\brief Sets the maximum force magnitude that the joint is able to withstand without breaking.
	
	If the joint force rises above this threshold, the joint breaks, and becomes disabled. Additionally,
	the jointBreakNotify() method of the scene's user notify callback will be called.
	(You can set this with NxScene::setUserNotify()).

	There are two values, one for linear forces, and one for angular forces. Both values are used directly
	as a value for the maximum impulse tolerated by the joint constraints. 

	Both force values are NX_MAX_REAL by default. This setting makes the joint unbreakable. 
	The values should always be nonnegative.

	The distinction between maxForce and maxTorque is dependent on how the joint is implemented internally, 
	which may not be obvious. For example what appears to be an angular degree of freedom may be constrained 
	indirectly by a linear constraint.

    So in most practical applications the user should set both maxTorque and maxForce to low values.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] maxForce Maximum force the joint can withstand without breaking. <b>Range:</b> (0,inf]
	\param[in] maxTorque Maximum torque the joint can withstand without breaking. <b>Range:</b> (0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.maxForce NxJointDesc.maxTorque getState() getBreakable()
	*/
	virtual void setBreakable(NxReal maxForce, NxReal maxTorque) = 0;

	/**
	\brief Retrieves the max forces of a breakable joint. See #setBreakable().

	\param[out] maxForce Retrieves the maximum force the joint can withstand without breaking.
	\param[out] maxTorque Retrieves the maximum torque the joint can withstand without breaking.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setBreakable NxJointDesc.maxForce NxJointDesc.maxTorque getState()
	*/
	virtual void getBreakable(NxReal & maxForce, NxReal & maxTorque) = 0;

	/**
	\brief Sets the solver extrapolation factor.

	\param[in] solverExtrapolationFactor The solver extrapolation factor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.solverExtrapolationFactor
	*/

	virtual void setSolverExtrapolationFactor(NxReal solverExtrapolationFactor) = 0;

	/**
	\brief Retrieves the solver extrapolation factor.

	\return The solver extrapolation factor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.solverExtrapolationFactor
	*/
	virtual NxReal getSolverExtrapolationFactor() const = 0;

	/**
	\brief Switch between acceleration and force based spring.

	\param[in] b {true: use acceleration spring, false: use force spring}.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.useAccelerationSpring
	*/
	virtual void setUseAccelerationSpring(bool b) = 0;

	/**
	\brief Checks whether acceleration spring is used.

	\return True if acceleration spring is used else false.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.useAccelerationSpring
	*/
	virtual bool getUseAccelerationSpring() const = 0;

/************************************************************************************************/

/** @name Limits
*/
//@{

	/**
	\brief Sets the limit point.
	
	The point is specified in the global coordinate frame.

	All types of joints may be limited with the same system:
	You may elect a point attached to one of the two actors to act as the limit point.
	You may also specify several planes attached to the other actor.

	The points and planes move together with the actor they are attached to.

	The simulation then makes certain that the pair of actors only move relative to each other 
	so that the limit point stays on the positive side of all limit planes.

	the default limit point is (0,0,0) in the local frame of actor2.
	Calling this deletes all existing limit planes.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] point The limit reference point defined in the global frame. <b>Range:</b> position vector
	\param[in] pointIsOnActor2 if true the point is attached to the second actor. Otherwise it is attached to the first.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLimitPoint() addLimitPlane()
	*/

	virtual void setLimitPoint(const NxVec3 & point, bool pointIsOnActor2 = true) = 0;

	/**
	\brief Retrieves the global space limit point.
	
	Returns true if the point is fixed on actor2.

	\param[out] worldLimitPoint Used to store the global frame limit point. 
	\return True if the point is fixed to actor 2 otherwise the point is fixed to actor 1.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLimitPoint() addLimitPlane()
	*/
	virtual bool getLimitPoint(NxVec3 & worldLimitPoint) = 0;

	/**
	\brief Adds a limit plane.
	
	Both of the parameters are given in global coordinates. see setLimitPoint() for the meaning of limit planes.
	
	The plane is affixed to the	actor that does not have the limit point.

	The normal of the plane points toward the positive side of the plane, and thus toward the
	limit point. If the normal points away from the limit point at the time of this call, the
	method returns false and the limit plane is ignored.

	\note This function always returns true and adds the limit plane unlike earlier versions. This behavior
	was changed to allow the joint to be serialized easily.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] normal Normal for the limit plane in global coordinates. <b>Range:</b> direction vector
	\param[in] pointInPlane Point in the limit plane in global coordinates. <b>Range:</b> position vector
	\param[in] restitution Restitution of the limit plane.
	<b>Range:</b> [0.0, 1.0]
	<b>Default:</b> 0.0
	\return Always true.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLimitPoint() purgeLimitPlanes() getNextLimitPlane()
	*/
	virtual bool addLimitPlane(const NxVec3 & normal, const NxVec3 & pointInPlane, NxReal restitution = 0.0f) = 0;

	/**
	\brief deletes all limit planes added to the joint.
	
	Invalidates limit plane iterator.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actor up automatically.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see addLimitPlane() getNextLimitPlane()
	*/
	virtual void purgeLimitPlanes() = 0;

	/**
	\brief Restarts the limit plane iteration.
	
	Call before starting to iterate. This method may be used together with
	the below two methods to enumerate the limit planes.
	This iterator becomes invalid when planes
	are added or removed, or the plane iterator mechanism is
	invoked on another joint.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see hasMoreLimitPlanes() getNextLimitPlane()
	*/
	virtual void resetLimitPlaneIterator() = 0;

	/**
	\brief Returns true until the iterator reaches the end of the set of limit planes.
	
	Adding or removing elements does not reset the iterator.

	\return True if the iterator has not reached the end of the sequence of limit planes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see resetLimitPlaneIterator() getNextLimitPlane()
	*/
	virtual bool hasMoreLimitPlanes() = 0;

	/**
	\brief Returns the next element pointed to by the limit plane iterator, and increments the iterator.

	Places the global frame plane equation (consisting of normal and d, the 4th
	element) coefficients in the argument references. The plane equation is of the form:

	dot(n,p) + d == 0 (n = normal, p = a point on the plane)

	\note This convention for the plane equation differs from the convention used by #NxPlaneShape

	\param[out] planeNormal Used to store the plane normal.
	\param[out] planeD Used to store the plane 'D'.
	\param[out] restitution Optional, used to store restitution of the limit plane.
	\return Returns true if the limit plane is satisfied.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see resetLimitPlaneIterator() hasMoreLimitPlanes()
	*/
	virtual bool getNextLimitPlane(NxVec3 & planeNormal, NxReal & planeD, NxReal * restitution = NULL) = 0;
//@}
/************************************************************************************************/

	/**
	\brief Retrieve the type of this joint.
	\return The type of joint.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Only D6 joints are supported in hardware)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointType
	*/
	virtual NxJointType  getType() const = 0;

/************************************************************************************************/

/** @name Is... Joint Type
*/
//@{

	/**
	\brief Type casting operator. The result may be cast to the desired subclass type.

	\param[in] type Used to query for a specific joint type.
	\return NULL if the object if not of type(see #NxJointType). Otherwise a pointer to this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointType
	*/
	virtual void* is(NxJointType type) = 0;

	/**
	\brief Attempts to perform a cast to a #NxRevoluteJoint.

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxRevoluteJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxRevoluteJoint
	*/
	NX_INLINE NxRevoluteJoint* isRevoluteJoint() { return (NxRevoluteJoint*)is(NX_JOINT_REVOLUTE);}

	/**
	\brief Attempts to perform a cast to a #NxPointInPlaneJoint.

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxPointInPlaneJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxPointInPlaneJoint
	*/
	NX_INLINE NxPointInPlaneJoint* isPointInPlaneJoint() { return (NxPointInPlaneJoint*)is(NX_JOINT_POINT_IN_PLANE);}

	/**
	\brief Attempts to perform a cast to a #NxPointOnLineJoint.

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxPointOnLineJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxPointOnLineJoint
	*/
	NX_INLINE NxPointOnLineJoint* isPointOnLineJoint() { return (NxPointOnLineJoint*)is(NX_JOINT_POINT_ON_LINE);}

	/**
	\brief Attempts to perform a cast to a #NxD6Joint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxD6Joint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxD6Joint
	*/
	NX_INLINE NxD6Joint* isD6Joint() { return (NxD6Joint*)is(NX_JOINT_D6);}

	/**
	\brief Attempts to perform a cast to a #NxPrismaticJoint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxPrismaticJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxPrismaticJoint
	*/
	NX_INLINE NxPrismaticJoint* isPrismaticJoint() { return (NxPrismaticJoint*)is(NX_JOINT_PRISMATIC);}

	/**
	\brief Attempts to perform a cast to a #NxCylindricalJoint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxCylindricalJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxCylindricalJoint
	*/
	NX_INLINE NxCylindricalJoint* isCylindricalJoint() { return (NxCylindricalJoint*)is(NX_JOINT_CYLINDRICAL);}

	/**
	\brief Attempts to perform a cast to a #NxSphericalJoint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxSphericalJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxSphericalJoint
	*/
	NX_INLINE NxSphericalJoint* isSphericalJoint() { return (NxSphericalJoint*)is(NX_JOINT_SPHERICAL);}

	/**
	\brief Attempts to perform a cast to a #NxFixedJoint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxFixedJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxFixedJoint
	*/
	NX_INLINE NxFixedJoint* isFixedJoint() { return (NxFixedJoint*)is(NX_JOINT_FIXED);}

	/**
	\brief Attempts to perform a cast to a #NxDistanceJoint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxDistanceJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxDistanceJoint
	*/
	NX_INLINE NxDistanceJoint* isDistanceJoint() { return (NxDistanceJoint*)is(NX_JOINT_DISTANCE);}

	/**
	\brief Attempts to perform a cast to a #NxPulleyJoint

	Returns NULL if this object is not of the appropriate type.

	\return NULL if this object is not a #NxPulleyJoint. Otherwise a pointer to this.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see is NxPulleyJoint
	*/
	NX_INLINE NxPulleyJoint* isPulleyJoint() { return (NxPulleyJoint*)is(NX_JOINT_PULLEY);}
//@}
/************************************************************************************************/

	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK. The string is not copied by the SDK, only the pointer is stored.
	
	\param[in] name String to set the objects name to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getName()
	*/
	virtual	void			setName(const char* name)		= 0;

	/**
	\brief Retrieves the name string set with setName().

	\return The name string for this object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setName()
	*/
	virtual	const char*		getName()			const	= 0;

	/**
	\brief Retrieves owner scene.

	\return The scene which owns this joint.

	<b>Platform:</b>
	
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual	NxScene&		getScene() const = 0;

	void*			userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	void*			appData;	//!< used internally, do not change.
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
