#ifndef NX_COLLISION_CONTROLLER
#define NX_COLLISION_CONTROLLER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

/* Exclude from documentation */
/** \cond */

//#include "NxAllocateable.h"
#include "NxController.h"
#include "CharacterController.h"

class NxScene;
class NxActor;
class NxBounds3;
class CharacterControllerManager;

class Controller //: public NxAllocateable
	{
	public:
										Controller(const NxControllerDesc& desc, NxScene* scene);
	virtual								~Controller();

	virtual	bool						getWorldBox(NxExtendedBounds3& box)	const	= 0;
	virtual	NxController*				getNxController()						= 0;
	virtual	NxActor*					getActor()					const	{ return kineActor; };

			NxControllerType			type;
			NxCCTInteractionFlag		interactionFlag;

	// User params
			NxHeightFieldAxis			upDirection;
			NxF32						slopeLimit;
			NxF32						skinWidth;
			NxF32						stepOffset;
			NxUserControllerHitReport*	callback;
			void*						userData;

	// Internal data
			SweepTest					cctModule;			// Internal CCT object. Optim test for Ubi.
			NxActor*					kineActor;			// Associated kinematic actor
			NxExtendedVec3				position;			// Current position
			NxExtendedVec3				filteredPosition;	// Current position after feedback filter
			NxExtendedVec3				exposedPosition;	// Position visible from the outside at any given time
			Extended					memory;				// Memory variable for feedback filter
			NxScene*					scene;				// Handy scene owner
			CharacterControllerManager*	manager;			// Owner manager
			bool						handleSlope;		// True to handle walkable parts according to slope

	protected:
	// Internal methods
			bool						setPos(const NxExtendedVec3& pos);
			void						setCollision(bool enabled);
			void						move(SweptVolume& volume, const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness, const NxGroupsMask* groupsMask, bool constrainedClimbingMode);
			void						setInteraction(NxCCTInteractionFlag flag)	{ interactionFlag = flag;	}
			NxCCTInteractionFlag		getInteraction()					const	{ return interactionFlag;	}
	};

/** \endcond */

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
