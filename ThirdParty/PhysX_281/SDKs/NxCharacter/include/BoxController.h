#ifndef NX_COLLISION_BOXCONTROLLER
#define NX_COLLISION_BOXCONTROLLER
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
#include "NxBoxController.h"

class BoxController : public NxBoxController, public Controller
	{
	public:
									BoxController(const NxControllerDesc& desc, NxScene* scene);
	virtual							~BoxController();

	virtual	NxController*			getNxController()								{ return this;						}
	virtual	NxControllerType		getType()										{ return type;						}

	virtual	void					move(const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness, const NxGroupsMask* groupsMask);

	virtual	bool					setPosition(const NxExtendedVec3& position)	{ return setPos(position);				}
	virtual	NxActor*				getActor()							const	{ return Controller::getActor();		}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// NxBoxController
	virtual	const NxVec3&			getExtents()						const;
	virtual	bool					setExtents(const NxVec3& extents);
	//~ NxBoxController

	virtual	void					setStepOffset(const float offset);

	virtual	const NxExtendedVec3&	getPosition()						const	{ return exposedPosition;				}
	virtual	const NxExtendedVec3&	getFilteredPosition()				const	{ return exposedPosition;				}
	virtual const NxExtendedVec3&	getDebugPosition()					const	{ return position;						}
	virtual	bool					getWorldBox(NxExtendedBounds3& box) const;
	virtual	void					setCollision(bool enabled)					{ Controller::setCollision(enabled);	}
	virtual	void					setInteraction(NxCCTInteractionFlag flag)	{ Controller::setInteraction(flag);		}
	virtual	NxCCTInteractionFlag	getInteraction()					const	{ return Controller::getInteraction();	}

	virtual	void					reportSceneChanged();
	virtual	void*					getUserData()						const	{ return userData;						}

			NxVec3					extents;
	};

/** \endcond */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
