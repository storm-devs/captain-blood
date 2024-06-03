#ifndef NX_PHYSICS_NXBOXCONTROLLER
#define NX_PHYSICS_NXBOXCONTROLLER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxCharacter.h"
#include "NxController.h"

/**
\brief Descriptor for a box character controller.

@see NxBoxController NxControllerDesc
*/
class NxBoxControllerDesc : public NxControllerDesc
	{
	public:
	/**
	\brief constructor sets to default.
	*/
	NX_INLINE								NxBoxControllerDesc();
	NX_INLINE virtual						~NxBoxControllerDesc();

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
						\brief The extents of the box controller.

						The extents of the controller specify the half width/height/depth for each axis.

						<b>Default:</b> [0.5,1.0,0.5]
						*/
						NxVec3				extents;
	};

NX_INLINE NxBoxControllerDesc::NxBoxControllerDesc() : NxControllerDesc(NX_CONTROLLER_BOX)
	{
	setToDefault();
	}

NX_INLINE NxBoxControllerDesc::~NxBoxControllerDesc()
	{
	}

NX_INLINE void NxBoxControllerDesc::setToDefault()
	{
	NxControllerDesc::setToDefault();
	extents.x = 0.5f;
	extents.y = 1.0f;
	extents.z = 0.5f;
	}

NX_INLINE bool NxBoxControllerDesc::isValid() const
	{
	if(!NxControllerDesc::isValid())	return false;
	if(extents.x<=0.0f)					return false;
	if(extents.y<=0.0f)					return false;
	if(extents.z<=0.0f)					return false;
	return true;
	}

/**
\brief Box character controller.

@see NxBoxControllerDesc NxController
*/
class NxBoxController : public NxController
	{
	protected:
	NX_INLINE					NxBoxController()	{}
	virtual						~NxBoxController()	{}

	public:

	/**
	\brief Gets controller's extents.

	\return The extents of the controller.

	@see NxBoxControllerDesc.extents setExtents()
	*/
	virtual		const NxVec3&	getExtents() const = 0;

	/**
	\brief Resets controller's extents.

	\warning this doesn't check for collisions.

	\param[in] extents The new extents for the controller.
	\return Currently always true.

	@see NxBoxControllerDesc.extents getExtents()
	*/
	virtual		bool			setExtents(const NxVec3& extents) = 0;

	/**
	\brief Sets the step height/offset for the controller.

	\param[in] offset The new step offset.

	@see NxControllerDesc.stepOffset NxController.stepOffset
	*/
	virtual	    void			setStepOffset(const float offset) =0;

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
