/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxController.h"
#include "Controller.h"
#include "CapsuleController.h"
#include "NxPhysics.h"
#include "NxCapsuleShapeDesc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CapsuleController::CapsuleController(const NxControllerDesc& desc, NxScene* s) : Controller(desc, s)
	{
	type = NX_CONTROLLER_CAPSULE;

	const NxCapsuleControllerDesc& cc = (const NxCapsuleControllerDesc&)desc;

	radius			= cc.radius;
	height			= cc.height;
	climbingMode	= cc.climbingMode;

	// Create kinematic actor under the hood
	if(1)
		{
		NxBodyDesc bodyDesc;
		bodyDesc.flags			|= NX_BF_KINEMATIC;

		NxCapsuleShapeDesc capsuleDesc;
		capsuleDesc.userData	= (void*)'CCTS';	// Mark as a "CCT Shape"

		// PT: we don't disable raycasting or CD because:
		// - raycasting is needed for visibility queries (the SDK otherwise doesn't know about the CCTS)
		// - collision is needed because the only reason we create actors there is to handle collisions with dynamic shapes
		// So it's actually wrong to disable any of those, and we'll have to filter out the CCT shapes using the 'CCTS' mark.
//		capsuleDesc.shapeFlags	|= NX_SF_DISABLE_RAYCASTING;	// You don't want to report those to users...
//		capsuleDesc.shapeFlags	|= NX_SF_DISABLE_COLLISION;

		capsuleDesc.radius		= radius * 0.8f;
		capsuleDesc.height		= height * 0.8f;

		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&capsuleDesc);
		actorDesc.body			= &bodyDesc;
		actorDesc.density		= 10000.0f;	// ### expose this ?
		actorDesc.density		= 10.0f;	// ### expose this ?
		actorDesc.globalPose.t.x  = (float)position.x;
		actorDesc.globalPose.t.y  = (float)position.y;
		actorDesc.globalPose.t.z  = (float)position.z;

		if(desc.upDirection==NX_Z)
			actorDesc.globalPose.M.rotX(NxHalfPiF32);

		kineActor = scene->createActor(actorDesc);
  }
}

CapsuleController::~CapsuleController()
	{
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapsuleController::reportSceneChanged()
	{
	cctModule.VoidTestCache();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CapsuleController::getWorldBox(NxExtendedBounds3& box) const
	{
//	box.setCenterExtents(position, NxVec3(radius, height, radius));
	box.setCenterExtents(position, NxVec3(radius, radius+height*0.5f, radius));
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NxF32 CapsuleController::getRadius() const
	{
	return radius;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NxF32 CapsuleController::getHeight() const
	{
	return height;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CapsuleController::setRadius(NxF32 r)
	{
	radius	= r;
	if(kineActor)
		{
		NxShape* S = *kineActor->getShapes();
		NX_ASSERT(S && static_cast<NxCapsuleShape*>(S));
		NxCapsuleShape* CS = static_cast<NxCapsuleShape*>(S);
		CS->setRadius(r * 0.8f);
		}
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CapsuleController::setHeight(NxF32 h)
	{
	height	= h;
	if(kineActor)
		{
		NxShape* S = *kineActor->getShapes();
		NX_ASSERT(S && static_cast<NxCapsuleShape*>(S));
		NxCapsuleShape* CS = static_cast<NxCapsuleShape*>(S);
		CS->setHeight(h * 0.8f);
		}
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapsuleController::setStepOffset(const float offset)
	{
    stepOffset = offset;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NxCapsuleClimbingMode CapsuleController::getClimbingMode() const
	{
	return climbingMode;
	}

bool CapsuleController::setClimbingMode(NxCapsuleClimbingMode mode)
	{
	if(mode>=CLIMB_LAST)
		return false;
	climbingMode = mode;
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
