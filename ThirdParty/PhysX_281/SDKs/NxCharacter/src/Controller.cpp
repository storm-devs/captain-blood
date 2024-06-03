/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxController.h"
#include "Controller.h"
#include "BoxController.h"
#include "NxScene.h"
#include "NxActor.h"
#include "NxBoxShapeDesc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Controller::Controller(const NxControllerDesc& desc, NxScene* s) : manager(NULL), scene(s)
	{
	type				= NX_CONTROLLER_FORCE_DWORD;
	interactionFlag		= desc.interactionFlag;

	upDirection			= desc.upDirection;
	slopeLimit			= desc.slopeLimit;
	skinWidth			= desc.skinWidth;
	stepOffset			= desc.stepOffset;
	callback			= desc.callback;
	userData			= desc.userData;

	kineActor			= NULL;
	position			= desc.position;
	filteredPosition	= desc.position;
	exposedPosition		= desc.position;
	memory				= desc.position[upDirection];
	handleSlope			= desc.slopeLimit!=0.0f; 
	}

Controller::~Controller()
	{
	if(scene && kineActor)
		scene->releaseActor(*kineActor);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Controller::setPos(const NxExtendedVec3& pos)
	{
	position = filteredPosition = exposedPosition = pos;
	memory = pos[upDirection];

	// Update kinematic actor
	if(kineActor)
		kineActor->moveGlobalPosition(NxVec3(NxReal(position.x), NxReal(position.y), NxReal(position.z)));	// LOSS OF ACCURACY
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Controller::setCollision(bool enabled)
	{
	if(kineActor)
		{
		if(enabled)	kineActor->clearActorFlag(NX_AF_DISABLE_COLLISION);
		else		kineActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
