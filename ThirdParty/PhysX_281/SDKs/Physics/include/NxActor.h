#ifndef NX_PHYSICS_NX_ACTOR
#define NX_PHYSICS_NX_ACTOR
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
#include "NxArray.h"
#include "NxBounds3.h"
#include "NxActorDesc.h"
#include "NxPhysicsSDK.h"
#include "NxUserEntityReport.h"

class NxBodyDesc;
class NxShapeDesc;
class NxJoint;
class NxShape;

#if NX_SUPPORT_SWEEP_API
	struct NxSweepQueryHit;

	class NxSweepCache
		{
		protected:
													NxSweepCache()	{}
		virtual										~NxSweepCache()	{}
		public:
		virtual		void							setVolume(const NxBox& box)	= 0;
		};
#endif

/**
\brief NxActor is the main simulation object in the physics SDK.

The actor is owned by and contained in a NxScene.

An actor may optionally encapsulate a dynamic rigid body by setting the body member of the
actor's descriptor when it is created. Otherwise the actor will be static (fixed in the world).

<h3>Creation</h3>
Instances of this class are created by calling #NxScene::createActor() and deleted with #NxScene::releaseActor().

See #NxActorDescBase for a more detailed description of the parameters which can be set when creating an actor.

Example (Static Actor):

\include NxActor_CreateStatic.cpp

Example (Dynamic Actor):

\include NxActor_CreateDynamic.cpp

<h3>Visualizations</h3>
\li #NX_VISUALIZE_ACTOR_AXES
\li #NX_VISUALIZE_BODY_AXES
\li #NX_VISUALIZE_BODY_MASS_AXES
\li #NX_VISUALIZE_BODY_LIN_VELOCITY
\li #NX_VISUALIZE_BODY_ANG_VELOCITY
\li #NX_VISUALIZE_BODY_JOINT_GROUPS


@see NxActorDesc NxBodyDesc NxScene.createActor() NxScene.releaseActor()
*/

class NxActor
	{
	protected:
	NX_INLINE					NxActor() : userData(NULL)
											{}
	virtual						~NxActor()	{}

	public:
	/**
	\brief Retrieves the scene which this actor belongs to.

	\return Owner Scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual		NxScene&		getScene()	const = 0;

	// Runtime modifications

	/**
	\brief Saves the state of the actor to the passed descriptor.

	This method does not save out any shapes belonging to the actor to the descriptor's
	shape vector, nor does it write to its body member. You have to iterate through
	the shapes of the actor and save them manually. In addition for dynamic actors you 
	have to call the #saveBodyToDesc() method.

	\param[out] desc	Descriptor to save object state to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActorDesc NxActorDescBase
	*/
	virtual		void			saveToDesc(NxActorDescBase& desc) = 0;

	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK. The string is not copied by the SDK, 
	only the pointer is stored.

	\param[in] name String to set the objects name to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getName()
	*/
	virtual		void			setName(const char* name)		= 0;

	/**
	\brief Retrieves the name string set with setName().

	\return Name string associated with object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setName()
	*/
	virtual		const char*		getName()			const	= 0;

/************************************************************************************************/
/** @name Global Pose Manipulation
*/
//@{

	/**
	\brief Methods for setting a dynamic actor's pose in the world.

	These methods instantaneously change the actor space to world space transformation. 

	One should exercise restraint in making use of these methods. 

	Static actors should not be moved at all. There are various internal data structures for static actors
	which may need to be recomputed when one moves. Also, moving static actors will not interact correctly
	with dynamic actors or joints. If you would like to directly control an actor's position and would like to 
	have it correctly interact with dynamic bodies and joints, you should create a dynamic body with the 
	NX_BF_KINEMATIC flag, and then use the moveGlobal*() commands to move it along a path!

	When briefly moving dynamic actors, one should not:
	
	\li Move actors into other actors, thus causing interpenetration (an invalid physical state)
	
	\li Move an actor  that is connected by a joint to another away from the other (thus causing joint error)

	\li When moving jointed actors the joints' cached transform information is destroyed and recreated next frame;
	thus this call is expensive for jointed actors.
	
	setGlobalPose(m) has the same effect as calling	
	setGlobalOrientation(m.M);	and setGlobalPosition(m.t); 
	but setGlobalPose() may be faster as it doesn't recompute some internal values twice.
	
	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Transformation from the actors local frame to the global frame. <b>Range:</b> rigid body transform.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGlobalPose() setGlobalPosition() setGlobalOrientation() getGlobalPose()
	*/
	virtual		void			setGlobalPose(const NxMat34& mat)			= 0;

	/**
	\brief Sets a dynamic actor's position in the world.

	see ::setGlobalPose() for information.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] vec New position for the actors frame relative to the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalPose() setGlobalOrientation() getGlobalPosition()
	*/
	virtual		void			setGlobalPosition(const NxVec3& vec)		= 0;

	/**
	\brief Sets a dynamic actor's orientation in the world.

	see ::setGlobalPose() for information.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat New orientation for the actors frame. <b>Range:</b> rotation matrix.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalPose() setGlobalPosition() getGlobalOrientation() setGlobalOrientationQuat()
	*/
	virtual		void			setGlobalOrientation(const NxMat33& mat)	= 0;

	/**
	\brief Sets a dynamic actor's orientation in the world.

	see ::setGlobalPose() for information.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat New orientation for the actors frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalOrientation() getGlobalOrientation() setGlobalPose()
	*/
	virtual		void			setGlobalOrientationQuat(const NxQuat& mat)	= 0;

	/**
	\brief Retrieves the actors world space transform.

	The getGlobal*() methods retrieve the actor's current actor space to world space transformation.

	\return Global pose matrix of object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalPose() getGlobalPosition() getGlobalOrientation()
	*/
	virtual		NxMat34 		getGlobalPose()			  const	= 0;

	/**
	\brief Retrieves the actors world space position.

	The getGlobal*() methods retrieve the actor's current actor space to world space transformation.

	\return Global position of object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGlobalPosition() getGlobalPose() getGlobalOrientation()
	*/
	virtual		NxVec3 			getGlobalPosition()		  const	= 0;

	/**
	\brief Retrieves the actors world space orientation.

	The getGlobal*() methods retrieve the actor's current actor space to world space transformation.

	\return Global orientation of object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGlobalOrientationQuat() setGlobalOrientation() getGlobalPose() getGlobalPosition()
	*/
	virtual		NxMat33 		getGlobalOrientation()	  const	= 0; 

	/**
	\brief Retrieves the actors world space orientation.

	The getGlobal*() methods retrieve the actor's current actor space to world space transformation.

	\return Global orientation of the actor as a quaternion.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGlobalOrientation() setGlobalOrientation() getGlobalPose() getGlobalPosition()
	*/
	virtual		NxQuat 			getGlobalOrientationQuat()const	= 0;

/************************************************************************************************/
//@}

/** @name Kinematic Actors
*/
//@{

	/**
	\brief The moveGlobal* calls serve to move kinematically controlled dynamic actors through the game world.

	You set a dynamic actor to be kinematic using the NX_BF_KINEMATIC body flag,
	used either in the NxBodyDesc or with raiseBodyFlag().
	
	The move command will result in a velocity that, when successfully carried 
	out (i.e. the motion is not blocked due to joints or collisions) inside run*(),
	will move the body into the desired pose. After the move is carried out during
	a single time step, the velocity is returned to zero. Thus, you must
	continuously call this in every time step for kinematic actors so that they 
	move along a path.

	These functions simply store the move destination until run*() is called,
	so consecutive calls will simply overwrite the stored target variable.

	Note that in the future we will provide a mechanism for the motion to be blocked
	in certain cases (such as when a box jams in an automatic door), but currently
	the motion is always fully carried out.	

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat The desired pose for the kinematic actor, in the global frame. <b>Range:</b> rigid body transform.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see moveGlobalPosition() moveGlobalOrientation() moveGlobalOrientationQuat() NxBodyFlag raiseBodyFlag() NxBodyDesc.flags
	*/
	virtual		void			moveGlobalPose(const NxMat34& mat)			= 0;

	/**
	\brief The moveGlobal* calls serve to move kinematically controlled dynamic actors through the game world.

	See ::moveGlobalPose() for more information.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] vec The desired position for the kinematic actor, in the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see moveGlobalPose() moveGlobalOrientation() moveGlobalOrientationQuat() NxBodyFlag raiseBodyFlag() NxBodyDesc.flags
	*/
	virtual		void			moveGlobalPosition(const NxVec3& vec)		= 0;

	/**
	\brief The moveGlobal* calls serve to move kinematically controlled dynamic actors through the game world.

	See ::moveGlobalPose() for more information.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat The desired orientation for the kinematic actor, in the global frame. <b>Range:</b> rotation matrix.

	\note Although it is possible to only specify the orientation, it might be needed to also specify the body position, using moveGlobalPosition(). 
	If you don't do this, the actor position can start to drift, because of numerical imprecision.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see moveGlobalOrientationQuat() moveGlobalPosition() moveGlobalPose() NxBodyFlag raiseBodyFlag() NxBodyDesc.flags
	*/
	virtual		void			moveGlobalOrientation(const NxMat33& mat)	= 0;

	/**
	\brief The moveGlobal* calls serve to move kinematically controlled dynamic actors through the game world.

	See ::moveGlobalPose() for more information.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] quat The desired orientation quaternion for the kinematic actor, in the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see moveGlobalPosition() moveGlobalPose() NxBodyFlag raiseBodyFlag() NxBodyDesc.flags
	*/
	virtual		void			moveGlobalOrientationQuat(const NxQuat& quat)	= 0;

/************************************************************************************************/
//@}

/** @name Shapes
*/
//@{

	/**
	\brief Creates a new shape and adds it to the list of shapes of this actor.
	
	This invalidates the pointer returned by getShapes().

	\note Mass properties of dynamic actors will not automatically be recomputed
	to reflect the new mass distribution implied by the shape. Follow 
	this call with a call to updateMassFromShapes() to do that.

	\note Creating compounds with a very large number of shapes may adversly affect performance and stability.
	When performing collision tests between a pair of actors containing multiple shapes, a collision check is
	performed between each pair of shapes. This results in N^2 running time.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	Only a subset of the shape types are supported in hardware scenes (others will fall back to running in software):-

	Fluids:

	\li Compounds are supported
	\li #NxBoxShape
	\li #NxCapsuleShape
	\li #NxSphereShape
	\li #NxConvexShape

	Hardware Rigid bodies:

	\li Compounds are supported
	\li #NxBoxShape
	\li #NxSphereShape
	\li #NxCapsuleShape
	\li #NxConvexShape (software fallback for > 32 vertices or faces)
	\li #NxTriangleMeshShape
	\li #NxPlaneShape

	In addition mesh pages must be mapped into PPU memory for hardware scenes. No collision detection will be performed
	with portions of the mesh which have not been mapped to PPU memory. See #NxTriangleMeshShape.mapPageInstance()

	\param[in] desc The descriptor for the new shape. See e.g. #NxSphereShapeDesc.
	\return The newly create shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback in some cases)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape NxShapeDesc
	@see NxBoxShape NxCapsuleShape NxConvexShape NxPlaneShape NxSphereShape NxTriangleMeshShape NxWheelShape
	*/
	virtual		NxShape*		createShape(const NxShapeDesc& desc)	= 0;

	/**
	\brief Deletes the specified shape.
	
	This invalidates the pointer returned by getShapes().
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).
	Note that mass properties for the actor are unchanged by this call unless #updateMassFromShapes is also called.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] shape Shape to be released.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape
	@see NxBoxShape NxCapsuleShape NxConvexShape NxPlaneShape NxSphereShape NxTriangleMeshShape NxWheelShape
	*/
	virtual		void			releaseShape(NxShape& shape) = 0;

	/**
	\brief Returns the number of shapes assigned to the actor.

	You can use #getShapes() to retrieve an array of shape pointers.

	For static actors it is not possible to release all actors associated with the shape.
	An attempt to remove the last shape will be ignored.

	\return Number of shapes associated with this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape getShapes()
	*/
	virtual		NxU32			getNbShapes()		const	= 0;


	/**
	\brief Returns an array of shape pointers belonging to the actor.

	These are the shapes used by the actor for collision detection.

	You can retrieve the number of shape pointers by calling #getNbShapes()

	Note: Adding or removing shapes with #createShape() or #releaseShape() will invalidate the pointer.

	\return Array of shapes which are associated with this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape getNbShapes() createShape() releaseShape()
	*/
	virtual		NxShape*const *	getShapes()			const	= 0;

/************************************************************************************************/
//@}

	/**
	\brief Assigns the actor to a user defined group of actors.
	
	NxActorGroup is a 16 bit group identifier.

	This is similar to #NxShape groups, except those are only five bits and serve a different purpose.
	
	The NxScene::setActorGroupPairFlags() lets you set certain behaviors for pairs of actor groups.
	By default every actor is created in group 0.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] actorGroup The actor group flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroup() NxActorGroup
	*/
	virtual		void			setGroup(NxActorGroup actorGroup)		 = 0;
	
	/**
	\brief Retrieves the value set with setGroup().

	\return The group ID of this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroup() NxActorGroup
	*/
	virtual		NxActorGroup	getGroup() const			 = 0;


	/**
	\brief Assigns dynamic actors a dominance group identifier.
	
	NxDominanceGroup is a 5 bit group identifier (legal range from 0 to 31).

	This is similar to #NxShape groups, except those serve a different purpose.
	
	The NxScene::setDominanceGroupPair() lets you set certain behaviors for pairs of dominance groups.
	By default every actor is created in group 0.  Static actors must stay in group 0; thus you can only 
	call this on dynamic actors.

	<b>Sleeping:</b> Changing the dominance group does <b>NOT</b> wake the actor up automatically.

	@see getDominanceGroup() NxDominanceGroup NxScene::setDominanceGroupPair()
	*/
	virtual		void			setDominanceGroup(NxDominanceGroup dominanceGroup)		 = 0;
	
	
	/**
	\brief Retrieves the value set with setDominanceGroup().

	\return The dominance group of this actor.

	@see setDominanceGroup() NxDominanceGroup NxScene::setDominanceGroupPair()
	*/
	virtual		NxDominanceGroup	getDominanceGroup() const			 = 0;


	/**
	\brief Raises a particular actor flag.
	
	See the list of flags #NxActorFlag

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] actorFlag The actor flag to raise(set). See #NxActorFlag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActorFlag clearActorFlag() readActorFlag() NxActorDesc.flags
	*/
	virtual		void			raiseActorFlag(NxActorFlag actorFlag)			= 0;
	
	/**
	\brief Clears a particular actor flag.
	
	See the list of flags #NxActorFlag

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] actorFlag The actor flag to clear. See #NxActorFlag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActorFlag raiseActorFlag() readActorFlag() NxActorDesc.flags
	*/
	virtual		void			clearActorFlag(NxActorFlag actorFlag)			= 0;
	
	/**
	\brief Reads a particular actor flag.
	
	See the list of flags #NxActorFlag

	\param[in] actorFlag The actor flag to retrieve. See #NxActorFlag. 

	\return The value of the actor flag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActorFlag raiseActorFlag() clearActorFlag() NxActorDesc.flags
	*/
	virtual		bool			readActorFlag(NxActorFlag actorFlag)	const	= 0;

	/**
	\brief Reset the user actor pair filtering state for this actor. This will cause filtering
	callbacks to be called again for any pairs involving this actor. Use this method
	when you wish to change the filtering policy of an actor that may already be in contact
	with other actors.
	
	@see NxUserActorPairFiltering
	*/
	virtual		void			resetUserActorPairFiltering() = 0;


	/**
	\brief Returns true if the actor is dynamic.

	\return True if this is a dynamic actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc
	*/
	virtual		bool			isDynamic()	const			= 0;

/************************************************************************************************/

/** @name Mass Manipulation
*/
//@{

	/**
	\brief The setCMassOffsetLocal*() methods set the pose of the center of mass relative to the actor.	
	
	Methods that automatically compute the center of mass such as updateMassFromShapes() as well as computing
	the mass and inertia using the actors shapes, will set this pose automatically.
	
	The actor must be dynamic.

	\note Changing this transform will not move the actor in the world!

	\note Setting an unrealistic center of mass which is a long way from the body can make it difficult for
	the SDK to solve constraints. Perhaps leading to instability and jittering bodies.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Mass  frame offset transform relative to the actor frame. <b>Range:</b> rigid body transform.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassOffsetLocalPosition() setCMassOffsetLocalOrientation() setCMassOffsetGlobalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassOffsetLocalPose(const NxMat34& mat)			= 0;

	/**
	\brief The setCMassOffsetLocal*() methods set the pose of the center of mass relative to the actor.

	See ::setCMassOffsetLocalPose() for more information.

	\note Setting an unrealistic center of mass which is a long way from the body can make it difficult for
	the SDK to solve constraints. Perhaps leading to instability and jittering bodies.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] vec Mass frame offset relative to the actor frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassOffsetLocalPose() setCMassOffsetLocalOrientation() setCMassOffsetGlobalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassOffsetLocalPosition(const NxVec3& vec)		= 0;

	/**
	\brief The setCMassOffsetLocal*() methods set the pose of the center of mass relative to the actor.

	See ::setCMassOffsetLocalPose() for more information.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Mass frame orientation offset relative to the actor frame. <b>Range:</b> rotation matrix.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassOffsetLocalPose() setCMassOffsetLocalPosition() setCMassOffsetGlobalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassOffsetLocalOrientation(const NxMat33& mat)	= 0;

	/**
	\brief The setCMassOffsetGlobal*() methods set the pose of the center of mass relative to world space.

	Note that this will simply transform the parameter to actor space and then call 
	setCMassLocal*(). In other words it only shifts the center of mass but does not move the actor.
	
	The actor must be dynamic.

	\note Setting an unrealistic center of mass which is a long way from the body can make it difficult for
	the SDK to solve constraints. Perhaps leading to instability and jittering bodies.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Mass frame offset transform relative to the global frame. <b>Range:</b> rigid body transform.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassOffsetGlobalPosition() setCMassOffsetGlobalOrientation()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassOffsetGlobalPose(const NxMat34& mat)		= 0;

	/**
	\brief The setCMassOffsetGlobal*() methods set the pose of the center of mass relative to world space.

	See ::setCMassOffsetGlobalPose() for more information.

	\note Setting an unrealistic center of mass which is a long way from the body can make it difficult for
	the SDK to solve constraints. Perhaps leading to instability and jittering bodies.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] vec Mass frame offset relative to the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassOffsetGlobalPose() setCMassOffsetGlobalOrientation()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassOffsetGlobalPosition(const NxVec3& vec)		= 0;

	/**
	\brief The setCMassOffsetGlobal*() methods set the pose of the center of mass relative to world space.

	See ::setCMassOffsetGlobalPose() for more information.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Mass frame orientation offset relative to the global frame. <b>Range:</b> rotation matrix.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassOffsetGlobalPose() setCMassOffsetGlobalPosition() 
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassOffsetGlobalOrientation(const NxMat33& mat)	= 0;

	/**
	\brief The setCMassGlobal*() methods move the actor by setting the pose of the center of mass.

	Here the transform between the center of mass and the actor frame is held fixed and the actor
	to world transform is updated.
	
	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Actors new pose, from the transformation of the mass frame to the global frame. <b>Range:</b> rigid body transform.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassGlobalPosition() setCMassGlobalOrientation() getCMassLocalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassGlobalPose(const NxMat34& mat)			= 0;

	/**
	\brief The setCMassGlobal*() methods move the actor by setting the pose of the center of mass.

	See ::setCMassGlobalPose() for more information.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] vec Actors new position, from the transformation of the mass frame to the global frame. <b>Range:</b> position vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassGlobalPose() setCMassGlobalOrientation() getCMassLocalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassGlobalPosition(const NxVec3& vec)		= 0;

	/**
	\brief The setCMassGlobal*() methods move the actor by setting the pose of the center of mass.

	See ::setCMassGlobalPose() for more information.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] mat Actors new orientation, from the transformation of the mass frame to the global frame. <b>Range:</b> rotation matrix.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCMassGlobalPose() setCMassGlobalPosition() getCMassLocalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		void			setCMassGlobalOrientation(const NxMat33& mat)	= 0;

	/**
	\brief The getCMassLocal*() methods retrieve the center of mass pose relative to the actor.

	The actor must be dynamic.

	\return The center of mass pose relative to the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCMassLocalPosition() getCMassLocalOrientation() getCMassGlobalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		NxMat34 		getCMassLocalPose()					const	= 0;

	/**
	\brief The getCMassLocal*() methods retrieve the center of mass pose relative to the actor.

	The actor must be dynamic.

	\return The center of mass position relative to the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCMassLocalPose() getCMassLocalOrientation() getCMassGlobalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		NxVec3 			getCMassLocalPosition()				const	= 0; 

	/**
	\brief The getCMassLocal*() methods retrieve the center of mass pose relative to the actor.

	The actor must be dynamic.

	\return The mass orientation relative to the actors frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCMassLocalPose() getCMassLocalPosition() getCMassGlobalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		NxMat33 		getCMassLocalOrientation()			const	= 0;

	/**
	\brief The getCMassGlobal*() methods retrieve the center of mass pose in world space.

	The actor must be dynamic.

	\return The Mass transform for this actor relative to the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCMassGlobalPosition() getCMassGlobalOrientation() getCMassLocalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		NxMat34 		getCMassGlobalPose()				const  = 0;

	/**
	\brief The getCMassGlobal*() methods retrieve the center of mass pose in world space.

	The actor must be dynamic.

	\return The position of the center of mass relative to the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCMassGlobalPose() getCMassGlobalOrientation() getCMassLocalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		NxVec3 			getCMassGlobalPosition()			const  = 0;

	/**
	\brief The getCMassGlobal*() methods retrieve the center of mass pose in world space.

	The actor must be dynamic.

	\return The orientation of the mass frame relative to the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCMassGlobalPose() getCMassGlobalPosition() getCMassLocalPose()
	@see NxBodyDesc.massLocalPose
	*/
	virtual		NxMat33 		getCMassGlobalOrientation()			const = 0;

	/**
	\brief Sets the mass of a dynamic actor.
	
	The mass must be positive and the actor must be dynamic.
	
	setMass() does not update the inertial properties of the body, to change the inertia tensor
	use setMassSpaceInertiaTensor() or updateMassFromShapes().

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] mass New mass value for the actor. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getMass() NxBodyDesc.mass setMassSpaceInertiaTensor() updateMassFromShapes()
	*/
	virtual		void			setMass(NxReal mass) = 0;

	/**
	\brief Retrieves the mass of the actor.
	
	Static actors will always return 0.

	\return The mass of this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setMass() NxBodyDesc.mass setMassSpaceInertiaTensor()
	*/
	virtual		NxReal			getMass() const = 0;

	/**
	\brief Sets the inertia tensor, using a parameter specified in mass space coordinates.
	
	Note that such matrices are diagonal -- the passed vector is the diagonal.

	If you have a non diagonal world/actor space inertia tensor(3x3 matrix). Then you need to
	diagonalize it and set an appropriate mass space transform. See #setCMassOffsetLocalPose().
	
	The actor must be dynamic.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] m New mass space inertia tensor for the actor. <b>Range:</b> inertia vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc.massSpaceInertia getMassSpaceInertia() setMass() setCMassOffsetLocalPose()
	*/
	virtual		void			setMassSpaceInertiaTensor(const NxVec3& m) = 0;

	/**
	\brief  Retrieves the diagonal inertia tensor of the actor relative to the mass coordinate frame.

	This method retrieves a mass frame inertia vector. If you want a global frame inertia tensor(3x3 matrix),
	then see #getGlobalInertiaTensor().

	The actor must be dynamic.

	\return The mass space inertia tensor of this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc.massSpaceInertia setMassSpaceInertiaTensor() setMass() CMassOffsetLocalPose()
	*/
	virtual		NxVec3			getMassSpaceInertiaTensor()			const = 0;

	/**
	\brief Retrieves the inertia tensor of the actor relative to the world coordinate frame.

	The actor must be dynamic.

	\return The global frame inertia tensor of this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGlobalInertiaTensorInverse() NxBodyDesc.massSpaceInertia setMassSpaceInertiaTensor()
	*/
	virtual		NxMat33			getGlobalInertiaTensor()			const = 0;

	/**
	\brief Retrieves the inverse of the inertia tensor of the actor relative to the world coordinate frame.

	The actor must be dynamic.

	\return The inverse of the inertia tensor in the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGlobalInertiaTensor() NxBodyDesc.massSpaceInertia setMassSpaceInertiaTensor()
	*/
	virtual		NxMat33			getGlobalInertiaTensorInverse()		const = 0;

	/**
	\brief Recomputes a dynamic actor's mass properties from its shapes

	Given a constant density or total mass, the actors mass properties can be recomputed
	using the shapes attached to the actor. If the actor has no shapes, then only the totalMass
	parameter can be used. If all shapes in the actor are trigger shapes (non-physical), the call
	will fail.

	The mass of each shape is either the shape's local density (as specified in the #NxShapeDesc; default 1.0) 
	multiplied by the shape's volume or a directly specified shape mass. 

	The inertia tensor, mass frame and center of mass will always be recomputed. If there are no
	shapes in the actor, the mass will be totalMass, and the mass frame will be set to the center
	of the actor.

	If you supply a non-zero total mass, the actor's mass and inertia will first be computed as
	above and then scaled to fit this total mass.

	If you supply a non-zero density, the actor's mass and inertia will first be computed as above
	and then scaled by this factor.

	Either totalMass or density must be non-zero.

	The actor must be dynamic.

	\param[in] density Density scale factor of the shapes belonging to the actor. <b>Range:</b> [0,inf)
	\param[in] totalMass Total mass of the actor(or zero). <b>Range:</b> [0,inf)

	\return True if successful.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActorDesc NxBodyDesc NxBodyDesc.mass NxActorDesc.density NxActorDesc.lockCOM
	*/
	virtual		bool			updateMassFromShapes(NxReal density, NxReal totalMass)		= 0;

//@}
/************************************************************************************************/
/** @name Damping
*/
//@{

	/**
	\brief Sets the linear damping coefficient.
	
	Zero represents no damping. The damping coefficient must be nonnegative.
	
	The actor must be dynamic.

	<b>Default:</b> 0.
	
	\param[in] linDamp Linear damping coefficient. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLinearDamping() setAngularDamping() NxBodyDesc.linearDamping
	*/
	virtual		void			setLinearDamping(NxReal linDamp) = 0;

	/**
	\brief Retrieves the linear damping coefficient.

	The actor must be dynamic.

	\return The linear damping coefficient associated with this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLinearDamping() getAngularDamping() NxBodyDesc.linearDamping
	*/
	virtual		NxReal			getLinearDamping() const = 0;

	/**
	\brief Sets the angular damping coefficient.
	
	Zero represents no damping.
	
	The angular damping coefficient must be nonnegative.
	
	The actor must be dynamic.

	<b>Default:</b> 0.05

	\param[in] angDamp Angular damping coefficient. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getAngularDamping() NxBodyDesc.angularDamping setLinearDamping()
	*/
	virtual		void			setAngularDamping(NxReal angDamp) = 0;

	/**
	\brief Retrieves the angular damping coefficient.

	The actor must be dynamic.

	\return The angular damping coefficient associated with this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setAngularDamping() NxBodyDesc.angularDamping getLinearDamping()
	*/
	virtual		NxReal			getAngularDamping() const = 0;

//@}
/************************************************************************************************/
/** @name Velocity
*/
//@{

	/**
	\brief Sets the linear velocity of the actor.
	
	Note that if you continuously set the velocity of an actor yourself, 
	forces such as gravity or friction will not be able to manifest themselves, because forces directly
	influence only the velocity/momentum of an actor.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] linVel New linear velocity of actor. <b>Range:</b> velocity vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLinearVelocity() setAngularVelocity() NxBodyDesc.linearVelocity
	*/
	virtual		void			setLinearVelocity(const NxVec3& linVel) = 0;

	/**
	\brief Sets the angular velocity of the actor.
	
	Note that if you continuously set the angular velocity of an actor yourself, 
	forces such as friction will not be able to rotate the actor, because forces directly influence only the velocity/momentum.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] angVel New angular velocity of actor. <b>Range:</b> angular velocity vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getAngularVelocity() setLinearVelocity() NxBodyDesc.angularVelocity
	*/
	virtual		void			setAngularVelocity(const NxVec3& angVel) = 0;

	/**
	\brief Retrieves the linear velocity of an actor.

	The actor must be dynamic.

	\return The linear velocity of the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLinearVelocity() getAngularVelocity() NxBodyDesc.linearVelocity
	*/
	virtual		NxVec3			getLinearVelocity()		const = 0;

	/**
	\brief Retrieves the angular velocity of the actor.

	The actor must be dynamic.

	\return The angular velocity of the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setAngularVelocity() getLinearVelocity() NxBodyDesc.angularVelocity
	*/
	virtual		NxVec3			getAngularVelocity()	const = 0;

	/**
	\brief Lets you set the maximum angular velocity permitted for this actor.
	
	Because for various internal computations, very quickly rotating actors introduce error 
	into the simulation, which leads to undesired results.

	With NxPhysicsSDK::setParameter(NX_MAX_ANGULAR_VELOCITY) you can set the default maximum velocity for actors created
	after the call. Bodies' high angular velocities are clamped to this value. 

	However, because some actors, such as car wheels, should be able to rotate quickly, you can override the default setting
	on a per-actor basis with the below call. Note that objects such as wheels which are approximated with spherical or 
	other smooth collision primitives can be simulated with stability at a much higher angular velocity than, say, a box that
	has corners.

	Note: The angular velocity is clamped to the set value <i>before</i> the solver, which means that
	the limit may still be momentarily exceeded.

	The actor must be dynamic.

	\param[in] maxAngVel Max allowable angular velocity for actor. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getMaxAngularVelocity() NxBodyDesc.maxAngularVelocity
	*/
	virtual		void			setMaxAngularVelocity(NxReal maxAngVel) = 0;

	/**
	\brief Retrieves the maximum angular velocity permitted for this actor.

	The actor must be dynamic.

	\return The maximum allowed angular velocity for this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setMaxAngularVelocity NxBodyDesc.maxAngularVelocity
	*/
	virtual		NxReal			getMaxAngularVelocity()	const = 0; 

//@}
/************************************************************************************************/
/** @name CCD
*/
//@{

	/**
	\brief Sets the CCD Motion Threshold.
	
	If CCD is globally enabled (parameter NX_CONTINUOUS_CD), it is still skipped for bodies
	which have no point on any of their shapes moving more than CCDMotionThreshold distance in 
	one time step.  
	
	Hence, CCD is always performed if the threshold is 0.

	The actor must be dynamic and the CCD motion threshold must be non-negative.

	<h3>Visualizations:</h3>
	\li #NX_VISUALIZE_COLLISION_CCD
	\li #NX_VISUALIZE_COLLISION_SKELETONS

	\param[in] thresh CCD Motion threshold. <b>Range:</b> [0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCCDMotionThreshold NxParameter
	*/
	virtual		void			setCCDMotionThreshold(NxReal thresh) = 0;

	/**
	\brief Retrieves the CCD Motion threshold for this actor.

	The actor must be dynamic.

	\return The CCD threshold for the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setCCDMotionThreshold NxParameter
	*/
	virtual		NxReal			getCCDMotionThreshold()	const = 0; 

//@}
/************************************************************************************************/

/** @name Momentum
*/
//@{

	/**
	\brief Sets the linear momentum of the actor. 
	
	Note that if you continuously set the linear momentum of an actor yourself, 
	forces such as gravity or friction will not be able to manifest themselves, because forces directly
	influence only the velocity/momentum of a actor.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] linMoment New linear momentum. <b>Range:</b> momentum vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLinearMomentum() setAngularMomentum()
	*/
	virtual		void			setLinearMomentum(const NxVec3& linMoment) = 0;

	/**
	\brief Sets the angular momentum of the actor.
	
	Note that if you continuously set the angular velocity of an actor yourself, 
	forces such as friction will not be able to rotate the actor, because forces directly
	influence only the velocity of actor.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] angMoment New angular momentum. <b>Range:</b> angular momentum vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getAngularMomentum() setLinearMomentum() 
	*/
	virtual		void			setAngularMomentum(const NxVec3& angMoment) = 0;

	/**
	\brief Retrieves the linear momentum of an actor.
	
	The momentum is equal to the velocity times the mass.

	The actor must be dynamic.

	\return The linear momentum for the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setLinearMomentum() getAngularMomentum()
	*/
	virtual		NxVec3			getLinearMomentum()		const = 0;

	/**
	\brief Retrieves the angular momentum of an actor.
	
	The angular momentum is equal to the angular velocity times the global space inertia tensor.

	The actor must be dynamic.

	\return The angular momentum for the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setAngularMomentum() getLinearMomentum() 
	*/
	virtual		NxVec3			getAngularMomentum()	const = 0;

//@}

/************************************************************************************************/

/** @name Forces
*/
//@{

	/**
	\brief Applies a force (or impulse) defined in the global coordinate frame, acting at a particular 
	point in global coordinates, to the actor. 

	Note that if the force does not act along the center of mass of the actor, this
	will also add the corresponding torque. Because forces are reset at the end of every timestep, 
	you can maintain a total external force on an object by calling this once every frame.

    ::NxForceMode determines if the force is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] force Force/impulse to add, defined in the global frame. <b>Range:</b> force vector
	\param[in] pos Position in the global frame to add the force at. <b>Range:</b> position vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode)
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	@see addForceAtLocalPos() addLocalForceAtPos() addLocalForceAtLocalPos() addForce() addLocalForce()
	*/
	virtual		void			addForceAtPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies a force (or impulse) defined in the global coordinate frame, acting at a particular 
	point in local coordinates, to the actor. 

	Note that if the force does not act along the center of mass of the actor, this
	will also add the corresponding torque. Because forces are reset at the end of every timestep, you can maintain a
	total external force on an object by calling this once every frame.

	::NxForceMode determines if the force is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] force Force/impulse to add, defined in the global frame. <b>Range:</b> force vector
	\param[in] pos Position in the local frame to add the force at. <b>Range:</b> position vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode)
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	@see addForceAtPos() addLocalForceAtPos() addLocalForceAtLocalPos() addForce() addLocalForce()
	*/
	virtual		void			addForceAtLocalPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies a force (or impulse) defined in the actor local coordinate frame, acting at a 
	particular point in global coordinates, to the actor. 

	Note that if the force does not act along the center of mass of the actor, this
	will also add the corresponding torque. Because forces are reset at the end of every timestep, you can maintain a
	total external force on an object by calling this once every frame.

	::NxForceMode determines if the force is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] force Force/impulse to add, defined in the local frame. <b>Range:</b> force vector
	\param[in] pos Position in the global frame to add the force at. <b>Range:</b> position vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode)
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	@see addForceAtPos() addForceAtLocalPos() addLocalForceAtLocalPos() addForce() addLocalForce()
	*/
	virtual		void			addLocalForceAtPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies a force (or impulse) defined in the actor local coordinate frame, acting at a 
	particular point in local coordinates, to the actor. 

	Note that if the force does not act along the center of mass of the actor, this
	will also add the corresponding torque. Because forces are reset at the end of every timestep, you can maintain a
	total external force on an object by calling this once every frame.

	::NxForceMode determines if the force is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] force Force/impulse to add, defined in the local frame. <b>Range:</b> force vector
	\param[in] pos Position in the local frame to add the force at. <b>Range:</b> position vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode)
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	@see addForceAtPos() addForceAtLocalPos() addLocalForceAtPos() addForce() addLocalForce()
	*/
	virtual		void			addLocalForceAtLocalPos(const NxVec3& force, const NxVec3& pos, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies a force (or impulse) defined in the global coordinate frame to the actor.

	<b>This will not induce a torque</b>.

	::NxForceMode determines if the force is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] force Force/Impulse to apply defined in the global frame. <b>Range:</b> force vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode)
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	@see addForceAtPos() addForceAtLocalPos() addLocalForceAtPos() addLocalForceAtLocalPos() addLocalForce()
	*/
	virtual		void			addForce(const NxVec3& force, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies a force (or impulse) defined in the actor local coordinate frame to the actor.

	<b>This will not induce a torque</b>.

	::NxForceMode determines if the force is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] force Force/Impulse to apply defined in the local frame. <b>Range:</b> force vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode)
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	@see addForceAtPos() addForceAtLocalPos() addLocalForceAtPos() addLocalForceAtLocalPos() addForce()
	*/
	virtual		void			addLocalForce(const NxVec3& force, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies an impulsive torque defined in the global coordinate frame to the actor.

	::NxForceMode determines if the torque is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] torque Torque to apply defined in the global frame. <b>Range:</b> torque vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode).
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode addLocalTorque() addForce()
	*/
	virtual		void			addTorque(const NxVec3& torque, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

	/**
	\brief Applies an impulsive torque defined in the actor local coordinate frame to the actor.

	::NxForceMode determines if the torque is to be conventional or impulsive.

	The actor must be dynamic.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping and the wakeup parameter is true (default).

	\param[in] torque Torque to apply defined in the local frame. <b>Range:</b> torque vector
	\param[in] mode The mode to use when applying the force/impulse(see #NxForceMode).
	\param[in] wakeup Specify if the call should wake up the actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode addTorque() addForce()
	*/
	virtual		void			addLocalTorque(const NxVec3& torque, NxForceMode mode = NX_FORCE, bool wakeup = true) = 0;

//@}
/************************************************************************************************/

	/**
	\brief Computes the total kinetic (rotational and translational) energy of the object.

	The actor must be dynamic.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	\return The kinetic energy of the actor.
	*/
	virtual		NxReal			computeKineticEnergy() const = 0;

/************************************************************************************************/

/** @name Point Velocity
*/
//@{

	/**
	\brief Computes the velocity of a point given in world coordinates if it were attached to the 
	actor and moving with it.

	The actor must be dynamic.

	\param[in] point Point we wish to determine the velocity for, defined in the global frame. <b>Range:</b> position vector
	\return The velocity of point in the global frame.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getLocalPointVelocity() NxBodyDesc.linearVelocity NxBodyDesc.angularVelocity
	*/
	virtual		NxVec3			getPointVelocity(const NxVec3& point)	const		= 0;

	/**
	\brief Computes the velocity of a point given in body local coordinates as if it were attached to the 
	actor and moving with it.

	The actor must be dynamic.

	\param[in] point Point we wish to determine the velocity of, defined in the body local frame. <b>Range:</b> position vector
	\return The velocity, in the global frame, of the point.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getPointVelocity() NxBodyDesc.linearVelocity NxBodyDesc.angularVelocity
	*/
	virtual		NxVec3			getLocalPointVelocity(const NxVec3& point)	const	= 0;

//@}
/************************************************************************************************/

/** @name Sleeping
*/
//@{

	/**
	\brief Returns true if this body and all the actors it is touching or is linked to with joints are sleeping.

	When an actor does not move for a period of time, it is no longer simulated in order to save time. This state
	is called sleeping. However, because the object automatically wakes up when it is either touched by an awake object,
	or one of its properties is changed by the user, the entire sleep mechanism should be transparent to the user.
	
	(Note: From version 2.5 this method is identical to isSleeping())
	
	The actor must be dynamic.

	\return True if the actor's group is sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() getSleepEnergyThreshold()
	*/
	virtual		bool			isGroupSleeping() const = 0;

	/**
	\brief Returns true if this body is sleeping.

	When an actor does not move for a period of time, it is no longer simulated in order to save time. This state
	is called sleeping. However, because the object automatically wakes up when it is either touched by an awake object,
	or one of its properties is changed by the user, the entire sleep mechanism should be transparent to the user.
	
	If an actor is asleep after the call to NxScene::fetchResults() returns, it is guaranteed that the pose of the actor 
	was not changed. You can use this information to avoid updating the transforms of associated of dependent objects.
	
	The actor must be dynamic.

	\return True if the actor is sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep()  getSleepEnergyThreshold()
	*/
	virtual		bool			isSleeping() const = 0;

	/**
	\brief Returns the linear velocity below which an actor may go to sleep.
	
	Actors whose linear velocity is above this threshold will not be put to sleep.
    
    The actor must be dynamic.

    @see isSleeping

	\return The threshold linear velocity for sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() setSleepLinearVelocity() setSleepEnergyThreshold() getSleepEnergyThreshold()
	*/
    virtual		NxReal			getSleepLinearVelocity() const = 0;

    /**
	\brief Sets the linear velocity below which an actor may go to sleep.
	
	Actors whose linear velocity is above this threshold will not be put to sleep.
	
	If the threshold value is negative,	the velocity threshold is set using the NxPhysicsSDK's 
	NX_DEFAULT_SLEEP_LIN_VEL_SQUARED parameter.
    
	Setting the sleep angular/linear velocity only makes sense when the NX_BF_ENERGY_SLEEP_TEST is not set. In
	version 2.5 and later a new method is used by default which uses the kinetic energy of the body to control
	sleeping.


    The actor must be dynamic.

	\param[in] threshold Linear velocity below which an actor may sleep. <b>Range:</b> (0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() setSleepEnergyThreshold() getSleepEnergyThreshold()
	*/
    virtual		void			setSleepLinearVelocity(NxReal threshold) = 0;

	/**
	\brief Returns the angular velocity below which an actor may go to sleep.
	
	Actors whose angular velocity is above this threshold will not be put to sleep. 
    
    The actor must be dynamic.

    \return The threshold angular velocity for sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() setSleepAngularVelocity() setSleepEnergyThreshold() getSleepEnergyThreshold()
	*/
    virtual		NxReal			getSleepAngularVelocity() const = 0;

 	/**
	\brief Sets the angular velocity below which an actor may go to sleep.
	
	Actors whose angular velocity is above this threshold will not be put to sleep.

	If the threshold value is negative,	the velocity threshold is set using the NxPhysicsSDK's 
	NX_DEFAULT_SLEEP_LIN_VEL_SQUARED parameter.

	Setting the sleep angular/linear velocity only makes sense when the NX_BF_ENERGY_SLEEP_TEST is not set. In
	version 2.5 and later a new method is used by default which uses the kinetic energy of the body to control
	sleeping.
    
    The actor must be dynamic.

	\param[in] threshold Angular velocity below which an actor may go to sleep. <b>Range:</b> (0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

    @see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() setSleepLinearVelocity() setSleepEnergyThreshold() getSleepEnergyThreshold()
	*/
    virtual		void			setSleepAngularVelocity(NxReal threshold) = 0;

	/**
	\brief Returns the energy below which an actor may go to sleep.

	Actors whose energy is above this threshold will not be put to sleep. 

	The actor must be dynamic.

	\return The energy threshold for sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() setSleepAngularVelocity() NxBodyDesc.sleepEnergyThreshold
	*/
	virtual		NxReal				getSleepEnergyThreshold() const = 0;

 	/**
	\brief Sets the energy threshold below which an actor may go to sleep.

	Actors whose kinematic energy is above this threshold will not be put to sleep.

	If the threshold value is negative,	the velocity threshold is set using the NxPhysicsSDK's 
	NX_DEFAULT_SLEEP_ENERGY parameter.

	Setting the sleep energy threshold only makes sense when the NX_BF_ENERGY_SLEEP_TEST is set. There
	are also other types of sleeping that uses the linear and angular velocities directly instead of the 
	energy.
    
    The actor must be dynamic.

	\param[in] threshold Energy below which an actor may go to sleep. <b>Range:</b> (0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepEnergyThreshold() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp() putToSleep() setSleepLinearVelocity() setSleepAngularVelocity() NxBodyDesc.sleepEnergyThreshold
	*/
	virtual		void				setSleepEnergyThreshold(NxReal threshold) = 0;

	/**
	\brief Wakes up the actor if it is sleeping.  

	The wakeCounterValue determines how long until the body is put to sleep, a value of zero means 
	that the body is sleeping. wakeUp(0) is equivalent to NxActor::putToSleep().

	The actor must be dynamic.

	\param[in] wakeCounterValue New sleep counter value. <b>Range:</b> [0,inf]
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() putToSleep()
	*/
	virtual		void			wakeUp(NxReal wakeCounterValue=NX_SLEEP_INTERVAL)	= 0;

	/**
	\brief Forces the actor to sleep. 
	
	The actor will stay asleep until the next call to simulate, and will not wake up until then even when otherwise 
	it would (for example a force is applied to it). It can however wake up during
	the next simulate call.

	The actor must be dynamic.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isGroupSleeping() isSleeping() getSleepLinearVelocity() getSleepAngularVelocity() wakeUp()
	*/
	virtual		void			putToSleep()	= 0;
//@}
/************************************************************************************************/

	/**
	\brief Raises a particular body flag.
	
	See the actors body flags. See #NxBodyFlag for a list of flags.

	The actor must be dynamic.

	\param[in] bodyFlag Body flag to raise(set). See #NxBodyFlag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial (supports NX_BF_KINEMATIC, NX_BF_DISABLE_GRAVITY)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyFlag clearBodyFlag() readBodyFlag() NxBodyDesc.flags
	*/
	virtual		void			raiseBodyFlag(NxBodyFlag bodyFlag)				= 0;
	
	/**
	\brief Clears a particular body flag.
	
	See #NxBodyFlag for a list of flags.

	The actor must be dynamic.

	\param[in] bodyFlag Body flag to clear. See #NxBodyFlag.

	Sleeping: Does NOT wake the actor up automatically.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial (supports NX_BF_KINEMATIC, NX_BF_DISABLE_GRAVITY)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyFlag raiseBodyFlag() readBodyFlag() NxBodyDesc.flags
	*/
	virtual		void			clearBodyFlag(NxBodyFlag bodyFlag)				= 0;
	/**
	\brief Reads a particular body flag.
	
	See #NxBodyFlag for a list of flags.

	The actor must be dynamic.

	\param[in] bodyFlag Body flag to retrieve. See #NxBodyFlag.
	\return The value of the body flag specified by bodyFlag.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Partial (supports NX_BF_KINEMATIC, NX_BF_DISABLE_GRAVITY)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyFlag raiseBodyFlag() clearBodyFlag() NxBodyDesc.flags
	*/
	virtual		bool			readBodyFlag(NxBodyFlag bodyFlag)		const	= 0;

	/**
	\brief Saves the body information of a dynamic actor to the passed body descriptor.

	This method only save the dynamic(body) state for the actor. The user should use #saveToDesc()
	to save the state common between static and dynamic actors. Plus manually saving the shapes
	belonging to the actor.

	The actor must be dynamic.

	\param[out] bodyDesc Descriptor to save the state of the body to.
	\return True for a dynamic body. Otherwise False.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc saveToDesc() getShape()
	*/
	virtual		bool			saveBodyToDesc(NxBodyDesc& bodyDesc) = 0;

	/**
	\brief Sets the solver iteration count for the body. 
	
	The solver iteration count determines how accurately joints and contacts are resolved. 
	If you are having trouble with jointed bodies oscillating and behaving erratically, then
	setting a higher solver iteration count may improve their stability.

	The actor must be dynamic.

	\param[in] iterCount Number of iterations the solver should perform for this body. <b>Range:</b> [1,255]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSolverIterationCount() NxBodyDesc.solverIterationCount
	*/
	virtual		void			setSolverIterationCount(NxU32 iterCount) = 0;

	/**
	\brief Retrieves the solver iteration count.

	See #setSolverIterationCount().

	The actor must be dynamic.

	\return The solver iteration count for this body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setSolverIterationCount() NxBodyDesc.solverIterationCount
	*/
	virtual		NxU32			getSolverIterationCount() const = 0;

	/**
	\brief Retrieves the force threshold for contact reports.

	The contact report threshold is a force threshold. If the force between 
	two actors exceeds this threshold for either of the two actors, a contact report 
	will be generated according to the union of both actors' contact report threshold flags.
	See #getContactReportFlags().

	The actor must be dynamic. The threshold used for a collision between a dynamic actor
	and the static environment is the threshold of the dynamic actor, and all contacts with
	static actors are summed to find the total normal force.

	\return Force threshold for contact reports.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setContactReportThreshold getContactReportFlags NxContactPairFlag NxBodyDesc::contactReportThreshold
	*/
	virtual NxReal					getContactReportThreshold() const = 0;

	/**
	\brief Sets the force threshold for contact reports.

	See #getContactReportThreshold().

	The actor must be dynamic.

	\param[in] threshold Force threshold for contact reports. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getContactReportThreshold getContactReportFlags NxContactPairFlag NxBodyDesc::contactReportThreshold
	*/
	virtual void					setContactReportThreshold(NxReal threshold) = 0;

	/**
	\brief Retrieves the actor's contact report flags.

	See #setContactReportFlags().

	\return The contact reporting flags associated with this actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setContactReportFlags NxContactPairFlag NxActorDesc::contactReportFlags
	*/
	virtual NxU32					getContactReportFlags() const = 0;

	/**
	\brief Sets the actor's contact report flags.

	These flags are used to determine the kind of report that is generated for interactions with other
	actors.

	The following flags are permitted:

	NX_NOTIFY_ON_START_TOUCH
	NX_NOTIFY_ON_END_TOUCH
	NX_NOTIFY_ON_TOUCH	
	NX_NOTIFY_ON_IMPACT
	NX_NOTIFY_ON_ROLL
	NX_NOTIFY_ON_SLIDE
	NX_NOTIFY_FORCE
	NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD
	NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD
	NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD

	Please note: If the actor is part of an interacting pair for which the contact report generation
	is controlled already through any other mechanism (for example by use of NxScene::setActorPairFlags)
	then the union of all the specified contact report flags will be used to generate the report.

	See #getContactReportFlags().

	\param[in] flags Flags to control contact reporting. See #NxContactPairFlag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getContactReportFlags NxContactPairFlag NxActorDesc::contactReportFlags
	*/
	virtual void					setContactReportFlags(NxU32 flags) = 0;

#if NX_SUPPORT_SWEEP_API
	/**
	\brief Performs a linear sweep through space with the actor.

	\param[in] motion Length and direction of the sweep
	\param[in] flags Flags controlling the mode of the sweep
	\param[in] userData User data to impart to the returned data struct
	\param[in] nbShapes Maximum number of shapes to report <b>Range:</b> [1,NX_MAX_U32]
	\param[out] shapes Pointer to buffer for reported shapes
	\param[in] callback Callback function invoked on the closest hit (if any)
	\param[in] sweepCache Sweep cache to use with the query

	The function sweeps the entire actor, with all its shapes, through space and reports any shapes in the scene
	with which they intersect. Apart from the number of shapes intersected in this way, and the shapes
	intersected, information on the closest intersection is put in an #NxSweepQueryHit structure which 
	can be processed in the callback function if provided.
	Which shapes in the scene are regarded is specified through the flags parameter.
	For persistent sweeps, a sweep cache may be used to improve performance. A sweep cache may be created 
	through NxScene::createSweepCache().
	Note that trigger shapes possibly contained in the actor are automatically filtered out.

	\return The number of hits reported.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSweepQueryHit NxSweepFlags NxUserEntityReport NxScene
	*/
	virtual		NxU32			linearSweep(const NxVec3& motion, NxU32 flags, void* userData, NxU32 nbShapes, NxSweepQueryHit* shapes, NxUserEntityReport<NxSweepQueryHit>* callback, const NxSweepCache* sweepCache=NULL)	= 0;
#endif

	/**
	\brief Retrieves the actor's simulation compartment, if any.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartment
	*/
	virtual NxCompartment *			getCompartment() const = 0;

	/**
	\brief Retrieves the actor's force field material index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual NxForceFieldMaterial	getForceFieldMaterial() const = 0;

	/**
	\brief Sets the actor's force field material index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	*/
	virtual void					setForceFieldMaterial(NxForceFieldMaterial)  = 0;

	//public variables:
				void*			userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
