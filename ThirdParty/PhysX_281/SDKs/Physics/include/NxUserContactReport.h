#ifndef NX_COLLISION_NXUSERCONTACTREPORT
#define NX_COLLISION_NXUSERCONTACTREPORT
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
#include "NxShape.h"
#include "NxContactStreamIterator.h"

class NxActor;
class NxShape;


/**
\brief Contact pair flags.

@see NxUserContactReport.onContactNotify() NxActor::setContactReportThreshold
*/
enum NxContactPairFlag
	{
	NX_IGNORE_PAIR								= (1<<0),	//!< Disable contact generation for this pair

	NX_NOTIFY_ON_START_TOUCH					= (1<<1),	//!< Pair callback will be called when the pair starts to be in contact
	NX_NOTIFY_ON_END_TOUCH						= (1<<2),	//!< Pair callback will be called when the pair stops to be in contact
	NX_NOTIFY_ON_TOUCH							= (1<<3),	//!< Pair callback will keep getting called while the pair is in contact
	NX_NOTIFY_ON_IMPACT							= (1<<4),	//!< [Not yet implemented] pair callback will be called when it may be appropriate for the pair to play an impact sound
	NX_NOTIFY_ON_ROLL							= (1<<5),	//!< [Not yet implemented] pair callback will be called when the pair is in contact and rolling.
	NX_NOTIFY_ON_SLIDE							= (1<<6),	//!< [Not yet implemented] pair callback will be called when the pair is in contact and sliding (and not rolling).
	NX_NOTIFY_FORCES							= (1<<7),	//!< The (summed total) friction force and normal force will be given in the NxContactPair variable in the contact report.
	NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD	= (1<<8),	//!< Pair callback will be called when the contact force between two actors exceeds one of the actor-defined force thresholds
	NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD		= (1<<9),	//!< Pair callback will be called when the contact force between two actors falls below the actor-defined force thresholds
	NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD			= (1<<10),	//!< Pair callback will keep getting called while the contact force between two actors exceeds one of the actor-defined force thresholds

	NX_NOTIFY_CONTACT_MODIFICATION				= (1<<16),	//!< Generate a callback for all associated contact constraints, making it possible to edit the constraint. This flag is not included in NX_NOTIFY_ALL for performance reasons. \see NxUserContactModify

	NX_NOTIFY_ALL								= (NX_NOTIFY_ON_START_TOUCH|NX_NOTIFY_ON_END_TOUCH|NX_NOTIFY_ON_TOUCH|NX_NOTIFY_ON_IMPACT|NX_NOTIFY_ON_ROLL|NX_NOTIFY_ON_SLIDE|NX_NOTIFY_FORCES|
												   NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD|NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD|NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD)
	};

/**
\brief An instance of this class is passed to NxUserContactReport::onContactNotify().
It contains a contact stream which may be parsed using the class NxContactStreamIterator.

@see NxUserContactReport.onContactNotify()
*/
class NxContactPair
	{
	public:
		NX_INLINE	NxContactPair() : stream(NULL)	{}

	/**
	\brief The two actors that make up the pair.

	\note The actor pointers might reference deleted actors. Check the #isDeletedActor member to see
	      whether that is the case. Do not dereference a pointer to a deleted actor. The pointer to a
		  deleted actor is only provided such that user data structures which might depend on the pointer
		  value can be updated.

	@see NxActor
	*/
	NxActor*				actors[2];

	/**
	\brief Use this to create stream iter. See #NxContactStreamIterator.

	@see NxConstContactStream
	*/
	NxConstContactStream	stream;

	/**
	\brief The total contact normal force that was applied for this pair, to maintain nonpenetration constraints. You should set NX_NOTIFY_FORCES in order to receive this value.
	*/
	NxVec3					sumNormalForce;

	/**
	\brief The total tangential force that was applied for this pair. You should set NX_NOTIFY_FORCES in order to receive this value.
	*/
	NxVec3					sumFrictionForce;

	/**
	\brief Specifies for each actor of the pair if the actor has been deleted.

	Before dereferencing the actor pointers of the contact pair you might want to use this member
	to check if the pointers reference deleted actors. This will be the case if an actor for which
	NX_NOTIFY_ON_END_TOUCH or NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD events were requested gets deleted.

	@see actors
	*/
	bool					isDeletedActor[2];
	};

/**
\brief The user needs to implement this interface class in order to be notified when
certain contact events occur.

Once you pass an instance of this class to #NxScene::setUserContactReport(), 
its  #onContactNotify() method will be called for each pair of actors which comes into contact, 
for which this behavior was enabled.

You request which events are reported using NxScene::setActorPairFlags(), 
#NxScene::setShapePairFlags(), #NxScene::setActorGroupPairFlags() or #NxActor::setContactReportFlags()

Please note: Kinematic actors will not generate contact reports when in contact with other kinematic actors.

 <b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
 user thread.

<h3>Example</h3>

\include NxUserContactReport_Example.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_CONTACT_POINT
\li #NX_VISUALIZE_CONTACT_NORMAL
\li #NX_VISUALIZE_CONTACT_ERROR
\li #NX_VISUALIZE_CONTACT_FORCE

@see NxScene.setUserContactReport() NxScene.getUserNotify()
*/
class NxUserContactReport
	{
	public:
 	/**
	Called for a pair in contact. The events parameter is a combination of:

	<ul>
	<li>NX_NOTIFY_ON_START_TOUCH,</li>
	<li>NX_NOTIFY_ON_END_TOUCH,</li>
	<li>NX_NOTIFY_ON_TOUCH,</li>
	<li>NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD,</li>
	<li>NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD,</li>
	<li>NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD,</li>
	<li>NX_NOTIFY_ON_IMPACT,	//unimplemented!</li>
	<li>NX_NOTIFY_ON_ROLL,		//unimplemented!</li>
	<li>NX_NOTIFY_ON_SLIDE,		//unimplemented!</li>
	</ul>

	See the documentation of #NxContactPairFlag for an explanation of each. You request which events 
	are reported using #NxScene::setActorPairFlags(), #NxScene::setActorGroupPairFlags(),
	#NxScene::setShapePairFlags() or #NxActor::setContactReportFlags(). Do not keep a reference to 
	the passed object, as it will be invalid after this function returns.

	\note SDK state should not be modified from within onContactNotify(). In particular objects should not
	be created or destroyed. If state modification is needed then the changes should be stored to a buffer
	and performed after the simulation step.

	\param[in] pair The contact pair we are being notified of. See #NxContactPair.
	\param[in] events Flags raised due to the contact. See #NxContactPairFlag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxContactPair NxContactPairFlag
	*/
	virtual void  onContactNotify(NxContactPair& pair, NxU32 events) = 0;

	protected:
	virtual ~NxUserContactReport(){};
	};

/**
\brief The user needs to implement this interface class in order to be notified when trigger events
occur. 

Once you pass an instance of this class to #NxScene::setUserTriggerReport(), shapes
which have been marked as triggers using NxShape::setFlag(NX_TRIGGER_ENABLE,true) will call the
#onTrigger() method when their trigger status changes.

<b>Threading:</b> It is not necessary to make this class thread safe as it will only be called in the context of the
user thread.

Example:

\include NxUserTriggerReport_Usage.cpp

<h3>Visualizations</h3>
\li NX_VISUALIZE_COLLISION_SHAPES

@see NxScene.setUserTriggerReport() NxScene.getUserTriggerReport() NxShapeFlag NxShape.setFlag()
*/
class NxUserTriggerReport
	{
	public:
	/**
	\brief Called when a trigger shape reports a trigger event.

	\note SDK state should not be modified from within onTrigger(). In particular objects should not
	be created or destroyed. If state modification is needed then the changes should be stored to a buffer
	and performed after the simulation step.

	\param[in] triggerShape is the shape that has been marked as a trigger.
	\param[in] otherShape is the shape causing the trigger event.
	\param[in] status is the type of trigger event.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriggerFlag
	*/
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status) = 0;

	protected:
	virtual ~NxUserTriggerReport(){};
	};

/**
\brief An interface class that the user can implement in order to modify contact constraints.

<b>Threading:</b> It is <b>necessary</b> to make this class thread safe as it will be called in the context of the
simulation thread. It might also be necessary to make it reentrant, since some calls can be made by multi-threaded
parts of the physics engine.

You can enable the use of this contact modification callback in two ways:
1. Raise the flag NX_AF_CONTACT_MODIFICATION on a per-actor basis.
or
2. Set the flag NX_NOTIFY_CONTACT_MODIFICATION on a per-actor-pair basis.

Please note: 
+ It is possible to raise the contact modification flags at any time. But the calls will not wake the actors up.
+ It is not possible to turn off the performance degradation by simply removing the callback from the scene, all flags need to be removed as well.
+ The contacts will only be reported as long as the actors are awake. There will be no callbacks while the actors are sleeping.

@see NxScene.setUserContactModify() NxScene.getUserContactModify()
*/
class NxUserContactModify
{
public:
	//enum to identify what changes have been made
	/**
	\brief This enum is used for marking changes made to contact constraints in the NxUserContactModify callback. OR the values together when making multiple changes on the same contact.
	*/
	enum NxContactConstraintChange {
		NX_CCC_NONE					= 0,		//!< No changes made

		NX_CCC_MINIMPULSE			= (1<<0),	//!< Min impulse value changed
		NX_CCC_MAXIMPULSE			= (1<<1),	//!< Max impulse value changed
		NX_CCC_ERROR				= (1<<2),	//!< Error vector changed
		NX_CCC_TARGET				= (1<<3),	//!< Target vector changed

		NX_CCC_LOCALPOSITION0		= (1<<4),	//!< Local attachment position in shape 0 changed
		NX_CCC_LOCALPOSITION1		= (1<<5),	//!< Local attachment position in shape 1 changed
		NX_CCC_LOCALORIENTATION0	= (1<<6),	//!< Local orientation (normal, friction direction) in shape 0 changed
		NX_CCC_LOCALORIENTATION1	= (1<<7),	//!< Local orientation (normal, friction direction) in shape 1 changed

		NX_CCC_STATICFRICTION0		= (1<<8),	//!< Static friction parameter 0 changed. (Note: 0 does not have anything to do with shape 0/1)
		NX_CCC_STATICFRICTION1		= (1<<9),	//!< Static friction parameter 1 changed. (Note: 1 does not have anything to do with shape 0/1)
		NX_CCC_DYNAMICFRICTION0		= (1<<10),	//!< Dynamic friction parameter 0 changed. (Note: 0 does not have anything to do with shape 0/1)
		NX_CCC_DYNAMICFRICTION1		= (1<<11),	//!< Dynamic friction parameter 1 changed. (Note: 1 does not have anything to do with shape 0/1)
		NX_CCC_RESTITUTION			= (1<<12),	//!< Restitution value changed.

		NX_CCC_FORCE32				= (1<<31)	//!< Not a valid flag value, used by the enum to force the size to 32 bits.
	};

	//The data that can be changed by the callback
	struct NxContactCallbackData {
		NxReal minImpulse;			//!< Minimum impulse value that the solver can apply. Normally this should be 0, negative amount gives sticky contacts.
		NxReal maxImpulse;			//!< Maximum impulse value that the solver can apply. Normally this is FLT_MAX. If you set this to 0 (and the min impulse value is 0) then you will void contact effects of the constraint.
		NxVec3 error;				//!< Error vector. This is the current error that the solver should try to relax.
		NxVec3 target;				//!< Target velocity. This is the relative target velocity of the two bodies.

		/**
		\brief Constraint attachment point for shape 0. 
		
		If the shape belongs to a dynamic actor, then localpos0 is relative to the body frame of the actor. 
		Alternatively it is relative to the world frame for a static actor. 
		*/
		NxVec3 localpos0;

		/**
		\brief Constraint attachment point for shape 1. 
		
		If the shape belongs to a dynamic actor, then localpos1 is relative to the body frame of the actor. 
		Alternatively it is relative to the world frame for a static actor. 
		*/
		NxVec3 localpos1;

		/**
		\brief Constraint orientation quaternion for shape 0 relative to shape 0s body frame for dynamic 
		actors and relative to the world frame for static actors. 
		
		The constraint axis (normal) is along the x-axis of the quaternion. 
		The Y axis is the primary friction axis and the Z axis the secondary friction axis. 
		*/
		NxQuat localorientation0;
		
		/**
		\brief Constraint orientation quaternion for shape 1 relative to shape 1s body frame for dynamic 
		actors and relative to the world frame for static actors. 
		
		The constraint axis (normal) is along the x-axis of the quaternion. 
		The Y axis is the primary friction axis and the Z axis the secondary friction axis. 
		*/
		NxQuat localorientation1;

		/**
		\brief Static friction parameter 0.

		\note 0 does not have anything to do with shape 0/1, but is related to anisotropic friction, 
		0 is the primary friction axis.
		*/
		NxReal staticFriction0;

		/**
		\brief Static friction parameter 1.

		\note 1 does not have anything to do with shape 0/1, but is related to anisotropic friction, 
		0 is the primary friction axis.
		*/
		NxReal staticFriction1;
		
		/**
		\brief Dynamic friction parameter 0.

		\note 0 does not have anything to do with shape 0/1, but is related to anisotropic friction, 
		0 is the primary friction axis.
		*/		
		NxReal dynamicFriction0;
		
		/**
		\brief Dynamic friction parameter 1.

		\note 1 does not have anything to do with shape 0/1, but is related to anisotropic friction, 
		0 is the primary friction axis.
		*/
		NxReal dynamicFriction1;
		NxReal restitution;			//!< Restitution value.
	};

	/**
	\brief This is called when a contact constraint is generated. Modify the parameters in order to affect the generated contact constraint.
	This callback needs to be both thread safe and reentrant.

	\param changeFlags when making changes to the contact point, you must mark in this flag what changes have been made, see NxContactConstraintChange.
	\param shape0 one of the two shapes in contact
	\param shape1 the other shape
	\param featureIndex0 feature on the first shape, which is in contact with the other shape
	\param featureIndex1 feature on the second shape, which is in contact with the other shape
	\param data contact constraint properties, for the user to change. Changes in this also requires changes in the changeFlags parameter.

	\return true if the contact point should be kept, false if it should be discarded.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool onContactConstraint(
		NxU32& changeFlags, 
		const NxShape* shape0, 
		const NxShape* shape1, 
		const NxU32 featureIndex0, 
		const NxU32 featureIndex1,
		NxContactCallbackData& data) = 0;

protected:
	virtual ~NxUserContactModify(){};
};

/**
\brief An interface class that the user can implement in order to modify the contact point on which the 
WheelShape base its simulation constraints.

<b>Threading:</b> It is <b>necessary</b> to make this class thread safe as it will be called in the context of the
simulation thread. It might also be necessary to make it reentrant, since some calls can be made by multi-threaded
parts of the physics engine.

You enable the use of this callback by specifying a callback function in NxWheelShapeDesc.wheelContactModify 
or by setting a callback function through NxWheelShape.setUserWheelContactModify().

Please note: 
+ There will only be callbacks if the WheelShape finds a contact point. Increasing the suspensionTravel value
gives a longer raycast and increases the chance of finding a contact point (but also gives a potentially slower 
simulation).

@see NxWheelShapeDesc.wheelContactModify NxWheelShape.setUserWheelContactModify() NxWheelShape.getUserWheelContactModify()
*/
class NxUserWheelContactModify {
public:

	/**
	\brief This callback is called once for each wheel and sub step before the wheel constraints are setup
	and fed to the SDK. The values passed in the parameters can be adjusted to affect the vehicle simulation.
	The most interesting values are contactPosition, contactPoint, and contactNormal. The contactPosition value
	specifies how far on the travel distance the contactPoint was found. If you want to simulate a bumpy road,
	then this is the main parameter to change. It is also good to adjust the contactPoint variable, so that the
	wheel forces are applied in the correct position. 

	\param wheelShape The WheelShape that is being processed.
	\param contactPoint The contact point (in world coordinates) that is being used for the wheel.
	\param contactNormal The normal of the geometry at the contact point.
	\param contactPosition The distance on the spring travel distance where the wheel would end up if it was resting on the contact point.
	\param normalForce The normal force on the wheel from the last simulation step.
	\param otherShape The shape with which the wheel is in contact.
	\param otherShapeMaterialIndex The material on the other shape in the position where the wheel is in contact. Currently has no effect on the simulation.
	\param otherShapeFeatureIndex The feature on the other shape in the position where the wheel is in contact.

	\return Return true to keep the contact (with the possibly edited values) or false to drop the contact.
	*/
	virtual bool onWheelContact(NxWheelShape* wheelShape, NxVec3& contactPoint, NxVec3& contactNormal, NxReal& contactPosition, NxReal& normalForce, NxShape* otherShape, NxMaterialIndex& otherShapeMaterialIndex, NxU32 otherShapeFeatureIndex) = 0;

protected:
	virtual ~NxUserWheelContactModify() {}
};


/**
\brief An actor pair used by filtering.
*/
class NxActorPairFilter       
	{
	public:
	NxActor*    actor[2];  //!< Pair of actors that are candidates for contact generation
	bool        filtered;  //!< Set to true in order to filter out this pair from contact generation
	};

/**
\brief An interface class that the user can implement in order to apply custom contact filtering.
*/
class NxUserActorPairFiltering 
	{
	public:
	/**
	\brief Callback to allow the user to decide whether to filter a certain actor pair.

	Use the actor member of the NxActorPairFilter objects to decide whether to filter out the contact
	between the pair. Set the filtered member to true to apply filtering.

	\param filterArray An array of actor pairs for which filtering is to be decided.
	\param arraySize The number of elements in filterArray.

	@see NxActorPairFilter NxActor::resetUserActorPairFiltering NxScene::setUserActorPairFiltering
	*/
	virtual void onActorPairs(NxActorPairFilter* filterArray, NxU32 arraySize) = 0;

	protected:
	virtual ~NxUserActorPairFiltering() {}      
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
