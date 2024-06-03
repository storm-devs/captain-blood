#ifndef NX_PHYSICS_NXCAPSULECONTROLLER
#define NX_PHYSICS_NXCAPSULECONTROLLER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxCharacter.h"
#include "NxController.h"

enum NxCapsuleClimbingMode
	{
	CLIMB_EASY,			//!< Standard mode, let the capsule climb over surfaces according to impact normal
	CLIMB_CONSTRAINED,	//!< Constrained mode, try to limit climbing according to the step offset

	CLIMB_LAST
	};

/**
\brief A descriptor for a capsule character controller.

@see NxCapsuleController NxControllerDesc
*/
class NxCapsuleControllerDesc : public NxControllerDesc
	{
	public:
	/**
	\brief constructor sets to default.
	*/
	NX_INLINE								NxCapsuleControllerDesc ();
	NX_INLINE virtual						~NxCapsuleControllerDesc ();

	/**
	\brief (re)sets the structure to the default.
	*/
	NX_INLINE virtual	void				setToDefault();
	/**
	\brief returns true if the current settings are valid

	\return True if the descriptor is valid.
	*/
	NX_INLINE virtual	bool				isValid()		const;

						/**
						\brief The radius of the capsule

						<b>Default:</b> 0.0

						@see NxCapsuleController
						*/
						NxF32				radius;

						/**
						\brief The height of the controller

						<b>Default:</b> 0.0

						@see NxCapsuleController
						*/
						NxF32				height;

						/**
						\brief The climbing mode

						<b>Default:</b> CLIMB_CONSTRAINED

						@see NxCapsuleController
						*/
						NxCapsuleClimbingMode	climbingMode;
	};

NX_INLINE NxCapsuleControllerDesc::NxCapsuleControllerDesc () : NxControllerDesc(NX_CONTROLLER_CAPSULE)
	{
	setToDefault();
	}

NX_INLINE NxCapsuleControllerDesc::~NxCapsuleControllerDesc()
	{
	}

NX_INLINE void NxCapsuleControllerDesc::setToDefault()
	{
	NxControllerDesc::setToDefault();
	radius = height = 0.0f;
	climbingMode = CLIMB_CONSTRAINED;
	}

NX_INLINE bool NxCapsuleControllerDesc::isValid() const
	{
	if(!NxControllerDesc::isValid())	return false;
	if(radius<=0.0f)					return false;
	if(height<=0.0f)					return false;
	return true;
	}
/**
\brief A capsule character controller.

	The capsule is defined as a position, a vertical height, and a radius.
	The height is the same height as for NxCapsuleShape objects, i.e. the distance
	between the two sphere centers at the end of the capsule. In other words:

	p = pos (returned by controller)<br>
	h = height<br>
	r = radius<br>

	p = center of capsule<br>
	top sphere center = p.y + h*0.5<br>
	bottom sphere center = p.y - h*0.5<br>
	top capsule point = p.y + h*0.5 + r<br>
	bottom capsule point = p.y - h*0.5 - r<br>
*/
class NxCapsuleController : public NxController
	{
	protected:
	NX_INLINE					NxCapsuleController()	{}
	virtual						~NxCapsuleController()	{}

	public:

	/**
	\brief Gets controller's radius.

	\return The radius of the controller.

	@see NxCapsuleControllerDesc.radius setRadius()
	*/
	virtual		NxF32			getRadius() const = 0;

	/**
	\brief Sets controller's radius.

	\warning this doesn't check for collisions.

	\param[in] radius The new radius for the controller.
	\return Currently always true.

	@see NxCapsuleControllerDesc.radius getRadius()
	*/
	virtual		bool			setRadius(NxF32 radius) = 0;

	/**
	\brief Gets controller's height.

	\return The height of the capsule controller.

	@see NxCapsuleControllerDesc.height setHeight()
	*/
	virtual		NxF32			getHeight() const = 0;

	/**
	\brief Gets controller's climbing mode.

	\return The capsule controller's climbing mode.

	@see NxCapsuleControllerDesc.climbingMode setClimbingMode()
	*/
	virtual		NxCapsuleClimbingMode	getClimbingMode()	const	= 0;

	/**
	\brief Resets controller's height.

	\warning this doesn't check for collisions.

	\param[in] height The new height for the controller.
	\return Currently always true.

	@see NxCapsuleControllerDesc.height getHeight()
	*/
	virtual		bool			setHeight(NxF32 height) = 0;

	/**
	\brief Sets the step height/offset for the controller.

	\param[in] offset The new step offset.

	@see NxControllerDesc.stepOffset NxController.stepOffset
	*/
	virtual	    void			setStepOffset(const float offset) =0;

	/**
	\brief Sets controller's climbing mode.

	\param[in] mode The capsule controller's climbing mode.

	@see NxCapsuleControllerDesc.climbingMode getClimbingMode()
	*/
	virtual		bool			setClimbingMode(NxCapsuleClimbingMode mode)	= 0;

	/**
	\brief The character controller uses caching in order to speed up collision testing, this caching can not detect when static objects have changed in the scene. You need to call this method when such changes have been made.
	*/
	virtual		void			reportSceneChanged() = 0;
	};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
