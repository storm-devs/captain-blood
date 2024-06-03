#ifndef NX_COLLISION_CAPSULECONTROLLER
#define NX_COLLISION_CAPSULECONTROLLER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

/* Exclude from documentation */
/** \cond */

#include "Controller.h"
#include "NxCapsuleController.h"

class CapsuleController : public NxCapsuleController, public Controller
	{
	public:
									CapsuleController(const NxControllerDesc& desc, NxScene* scene);
	virtual							~CapsuleController();

	virtual	NxController*			getNxController()								{ return this;						}
	virtual	NxControllerType		getType()										{ return type;						}

	virtual	void					move(const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness, const NxGroupsMask* groupsMask);

	virtual	bool					setPosition(const NxExtendedVec3& position)	{ return setPos(position);				}

	virtual	void					setStepOffset(const float offset);

	virtual	NxActor*				getActor()							const	{ return Controller::getActor();		}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// NxCapsuleController
	virtual	NxF32					getRadius()							const;
	virtual	NxF32					getHeight()							const;
	virtual	NxCapsuleClimbingMode	getClimbingMode()					const;
	virtual	bool					setRadius(NxF32 radius);
	virtual	bool					setHeight(NxF32 height);
	virtual	bool					setClimbingMode(NxCapsuleClimbingMode);
	//~ NxCapsuleController

	virtual	const NxExtendedVec3&	getPosition()						const	{ return exposedPosition;				}
	virtual	const NxExtendedVec3&	getFilteredPosition()				const	{ return exposedPosition;				}
	virtual const NxExtendedVec3&	getDebugPosition()					const	{ return position;						}
	virtual	bool					getWorldBox(NxExtendedBounds3& box) const;
	virtual	void					setCollision(bool enabled)					{ Controller::setCollision(enabled);	}
	virtual	void					setInteraction(NxCCTInteractionFlag flag)	{ Controller::setInteraction(flag);		}
	virtual	NxCCTInteractionFlag	getInteraction()					const	{ return Controller::getInteraction();	}

	virtual	void					reportSceneChanged();
	virtual	void*					getUserData()						const	{ return userData;						}

			NxF32					radius;
			NxF32					height;
			NxCapsuleClimbingMode	climbingMode;
	};

/** \endcond */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
