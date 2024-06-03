#ifndef NX_PHYSICS_NXCONTROLLER
#define NX_PHYSICS_NXCONTROLLER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxCharacter.h"
#include "Nxp.h"
#include "NxExtended.h"

/**
\brief The type of controller, eg box, sphere or capsule.
*/
enum NxControllerType
	{
	/**
	\brief A box controller.

	@see NxBoxController NxBoxControllerDesc
	*/
	NX_CONTROLLER_BOX,

	/**
	\brief A capsule controller

	@see NxCapsuleController NxCapsuleControllerDesc
	*/
	NX_CONTROLLER_CAPSULE,

	NX_CONTROLLER_FORCE_DWORD = 0x7fffffff
	};

#include "NxTriangleMesh.h"

class NxShape;
class NxScene;
class NxController;
class NxActor;

/**
\brief specifies how a CCT interacts with other CCTs.

This member controls if a character controller will collide with another controller. There are 3 options:
always collide, never collide and collide based on the shape group.
This flag only affects other controllers when they move; when this controller moves, the flag is ignored
and the flags of the other controllers determine collision.
*/
enum NxCCTInteractionFlag
	{
	NXIF_INTERACTION_INCLUDE,		//!< Always collide character controllers.
	NXIF_INTERACTION_EXCLUDE,		//!< Never collide character controllers.

	/**
	\brief Collide based on the shape group.

	The groups to collide against are passed in the activeGroups member of #NxController::move(). The active
	groups flags are combined with the shape flags for the controllers kinematic actor to determine if a 
	collision should occur:

	    activeGroups & ( 1 << shape->getGroup())

	So to set the shape flags the NxController::getActor() method can be called, then the getShapes() method to
	retrieve the shape. Then NxShape::setGroup() method is used to set the shape group.

	@see NxController.move() NxController.getActor()
	*/
	NXIF_INTERACTION_USE_FILTER,	
	};

/**
\brief specifies which sides a character is colliding with.
*/
enum NxControllerFlag
	{
	NXCC_COLLISION_SIDES	= (1<<0),	//!< Character is colliding to the sides.
	NXCC_COLLISION_UP		= (1<<1),	//!< Character has collision above.
	NXCC_COLLISION_DOWN		= (1<<2),	//!< Character has collision below.
	};

/**
\brief Controller actions.
*/
enum NxControllerAction
	{
	NX_ACTION_NONE,				//!< Don't apply forces to touched actor
	NX_ACTION_PUSH,				//!< Automatically compute & apply forces to touched actor (push)
	};

/**
\brief Describes a controller shape hit. Passed to onShapeHit()

@see NxUserControllerHitReport.onShapeHit()
*/
struct NxControllerShapeHit
	{
	NxController*	controller;		//!< Current controller
	NxShape*		shape;			//!< Touched shape
	NxExtendedVec3	worldPos;		//!< Contact position in world space
	NxVec3			worldNormal;	//!< Contact normal in world space
	NxU32			id;				//!< Reserved for future use
	NxVec3			dir;			//!< Motion direction
	NxF32			length;			//!< Motion length
	};

/**
\brief Describe a controller hit. Passed to onControllerHit().

@see NxUserControllerHitReport.onControllerHit()
*/
struct NxControllersHit
	{
	NxController*	controller;		//!< Current controller
	NxController*	other;			//!< Touched controller
	};

/**
\brief User callback class for character controller events.

\note Character controller hit reports are only generated when move is called.

@see NxControllerDesc.callback
*/
class NxUserControllerHitReport
	{
	public:

	/**
	\brief Called when current controller hits a shape.

	\param[in] hit Provides information about the contact with the touched shape.
	\return Specifies if to automatically apply forces to the touched actor.

	@see NxControllerAction NxControllerShapeHit
	*/
	virtual NxControllerAction  onShapeHit(const NxControllerShapeHit& hit) = 0;

	/**
	\brief Called when current controller hits another controller.

	\param[in] hit Provides information about the touched controller.
	\return Specifies if to automatically apply forces to the touched controller.

	@see NxControllerAction NxControllersHit
	*/
	virtual NxControllerAction  onControllerHit(const NxControllersHit& hit) = 0;

	protected:
	virtual ~NxUserControllerHitReport(){};
	};

/**
\brief Descriptor class for a character controller.

@see NxBoxController NxCapsuleController
*/
class NxControllerDesc
	{
	protected:
	
	/**
	\brief Not used.
	*/
	NxU32						version;
	const NxControllerType		type;		//!< The type of the controller. This gets set by the derived class' ctor, the user should not have to change it.
	
	/**
	\brief constructor sets to default.
	*/
	NX_INLINE										NxControllerDesc(NxControllerType);
	NX_INLINE virtual								~NxControllerDesc();
	public:

	/**
	\brief (re)sets the structure to the default.
	*/
	NX_INLINE virtual	void						setToDefault();
	/**
	\brief returns true if the current settings are valid

	\return True if the descriptor is valid.
	*/
	NX_INLINE virtual	bool						isValid()		const;

	/**
	\brief Not used.

	\return The version number.
	*/
	NX_INLINE			NxU32						getVersion()	const	{ return version;	}

	/**
	\brief Returns the character controller type

	\return The controllers type.

	@see NxControllerType NxCapsuleControllerDesc NxBoxControllerDesc
	*/
	NX_INLINE			NxControllerType			getType()		const	{ return type;		}

						/**
						\brief The position of the character

						<b>Default:</b> Zero
						*/
						NxExtendedVec3				position;

						/**
						\brief Specifies the 'up' direction
						
						In order to provide stepping functionality the SDK must be informed about the up direction.

						\li NX_X => (1, 0, 0)
						\li NX_Y => (0, 1, 0)
						\li NX_Z => (0, 0, 1)

						<b>Default:</b> NX_Y

						@see NxHeightFieldAxis
						*/
						NxHeightFieldAxis			upDirection;

						/**
						\brief The maximum slope which the character can walk up.

						In general it is desirable to limit where the character can walk, in particular it is unrealistic
						for the character to be able to climb arbitary slopes.

						The limit is expressed as the cosine of desired limit angle. A value of 0 disables this feature.

						<b>Default:</b> 0.707

						@see upDirection
						*/
						NxF32						slopeLimit;

						/**
						\brief The skin width used by the controller.

						A "skin" around the controller is necessary to avoid numerical precision issues.

						This is dependant on the scale of the users world, but should be a small, positive 
						non zero value.

						<b>Default:</b> 0.1
						*/
						NxF32						skinWidth;

						/**
						\brief Defines the maximum height of an obstacle which the character can climb.

						A small value will mean that the character gets stuck and cannot walk up stairs etc, 
						a value which is too large will mean that the character can climb over unrealistically 
						high obstacles.

						<b>Default:</b> 0.5

						@see upDirection 
						*/
						NxF32						stepOffset;

						/**
						\brief Specifies a user callback interface.

						This callback interface is called when the character collides with shapes and other characters.

						Setting this to NULL disables callbacks.

						<b>Default:</b> NULL

						@see NxUserControllerHitReport
						*/
						NxUserControllerHitReport*	callback;

						/**
						\brief The interaction flag controls if a character controller collides with other controllers.

						The default is to collide controllers.

						<b>Default:</b> NXIF_INTERACTION_INCLUDE

						@see NxCCTInteractionFlag
						*/
						NxCCTInteractionFlag		interactionFlag;

						/**
						\brief User specified data associated with the controller.

						<b>Default:</b> NULL
						*/
						void*						userData;
	};

NX_INLINE NxControllerDesc::NxControllerDesc(NxControllerType t) : type(t)
	{
	setToDefault();
	}

NX_INLINE NxControllerDesc::~NxControllerDesc()
	{
	}

NX_INLINE void NxControllerDesc::setToDefault()
	{
	version			= 0;
	upDirection		= NX_Y;
	slopeLimit		= 0.707f;
	skinWidth		= 0.1f;
	stepOffset		= 0.5f;
	callback		= NULL;
	userData		= NULL;
	interactionFlag	= NXIF_INTERACTION_INCLUDE;
	position.zero();
	}

NX_INLINE bool NxControllerDesc::isValid() const
	{
	if (slopeLimit<0)	return false;
	if (stepOffset<0)	return false;
	if (skinWidth<0)	return false;
	return true;
	}


/**
\brief Base class for character controllers.

@see NxCapsuleController NxBoxController
*/
	class NxController
	{
	protected:
	NX_INLINE							NxController()					{}
	virtual								~NxController()					{}

	public:

	/**
	\brief Moves the character using a "collide-and-slide" algorithm.

	\param disp	a displacement vector
	\param activeGroups	a filtering mask for collision groups. If a bit is set, corresponding group is active.
	\param minDist the minimum travelled distance to consider. If travelled distance is smaller, the character doesn't move. 
	This is used to stop the recursive motion algorithm when remaining distance to travel is small.
	\param collisionFlags returned collision flags, collection of ::NxControllerFlag
	\param sharpness to prevent sudden height changes due to the autostep feature, the motion can be smoothed using a feedback filter.
	This coefficient defines the amount of smoothing. The smaller, the smoother. (1.0 means no smoothing).
	\param groupsMask Alternative mask used to filter shapes, see NxScene::overlapAABBShapes().
	*/
	virtual		void					move(const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness=1.0f, const NxGroupsMask* groupsMask=NULL)	= 0;

	/**
	\brief Resets controller's position.

	\warning this is a 'teleport' function, it doesn't check for collisions.

	To move the character under normal conditions use the #move() function.

	\param[in] position The new positon for the controller.
	\return Currently always returns true.

	@see NxControllerDesc.position getPosition() move()
	*/
	virtual		bool					setPosition(const NxExtendedVec3& position) = 0;
	/**
	\brief Retrieve the raw position of the controller.

	The position and filtered position are updated by calls to move(). Calling this method without calling
	move() will result in the last position or the initial position of the controller.

	\return The controllers position

	@see NxControllerDesc.position setPositon() move()
	*/
	virtual		const NxExtendedVec3&	getPosition()			const	= 0;
	/**
	\brief Retrieve the filtered position of the controller.

	\return The filtered position.

	Note: Deprecated - equivalent to getPosition().

	@see move() getPosition()
	*/
	virtual		const NxExtendedVec3&	getFilteredPosition()	const	= 0;

	/**
	\brief Currently returns the position of the character.

	\return The characters position.

	@see getPosition()
	*/
	virtual		const NxExtendedVec3&	getDebugPosition()		const	= 0;

	/**
	\brief Get the actor associated with this controller (see PhysX documentation). 
	The behavior upon manually altering this actor is undefined, you should primarily 
	use it for reading const properties.

	\return the actor associated with the controller.
	*/
	virtual		NxActor*				getActor()				const	= 0;

	/**
	\brief The step height.

	\param[in] offset The new step offset for the controller.

	@see NxControllerDesc.stepOffset
	*/
	virtual	    void					setStepOffset(const float offset) =0;

	/**
	\brief Enable/Disable collisions for this controller/actor.

	Enable/Disable collision between the scene and the kinematic actor associated with the controller.

	This function simply controls collision detection using clearActorFlag(NX_AF_DISABLE_COLLISION) or
	raiseActorFlag(NX_AF_DISABLE_COLLISION) on the kinematic actor associated with the controller.

	NOTE: This method does not affect collisions detected as part of the character movement. Instead it controls
	collisions between the actor and other objects. For example a box will block the movement of the controller,
	but the box can fall through the controller as part of the simulation.

	\param[in] enabled True to enable collision with the controller.
	*/
	virtual		void					setCollision(bool enabled)		= 0;

	/**
	\brief Sets the interaction flag for the CCT.

	\param[in] flag The new value of the interaction flag.

	\see NxCCTInteractionFlag
	*/
	virtual		void					setInteraction(NxCCTInteractionFlag flag)	= 0;

	/**
	\brief Retrieves the interaction flag for the CCT.

	\return The current interaction flag.

	\see NxCCTInteractionFlag
	*/
	virtual		NxCCTInteractionFlag	getInteraction()				const		= 0;

	/**
	\brief The character controller uses caching in order to speed up collision testing, this caching can not detect when static objects have changed in the scene. You need to call this method when such changes have been made.
	*/
	virtual		void					reportSceneChanged()			= 0;

	/**
	\brief Returns the user data associated with this controller.

	\return The user pointer associated with the controller.

	@see NxControllerDesc.userData
	*/
	virtual		void*					getUserData()		const		= 0;

	/**
	\brief Return the type of controller

	@see NxControllerType
	*/
	virtual		NxControllerType		getType()						= 0;
	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
