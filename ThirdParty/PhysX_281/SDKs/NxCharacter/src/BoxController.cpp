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
#include "NxPhysics.h"
#include "NxBoxShapeDesc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BoxController::BoxController(const NxControllerDesc& desc, NxScene* s) : Controller(desc, s)
	{
	type = NX_CONTROLLER_BOX;

	const NxBoxControllerDesc& bc = (const NxBoxControllerDesc&)desc;

	extents	= bc.extents;

	// Create kinematic actor under the hood
	if(1)
		{
		NxBodyDesc bodyDesc;
		bodyDesc.flags			|= NX_BF_KINEMATIC;

		NxBoxShapeDesc boxDesc;
		boxDesc.userData		= (void*)'CCTS';	// Mark as a "CCT Shape"

		// PT: we don't disable raycasting or CD because:
		// - raycasting is needed for visibility queries (the SDK otherwise doesn't know about the CCTS)
		// - collision is needed because the only reason we create actors there is to handle collisions with dynamic shapes
		// So it's actually wrong to disable any of those, and we'll have to filter out the CCT shapes using the 'CCTS' mark.
//		boxDesc.shapeFlags		|= NX_SF_DISABLE_RAYCASTING;	// You don't want to report those to users...
//		boxDesc.shapeFlags		|= NX_SF_DISABLE_COLLISION;

//		boxDesc.dimensions		= extents*0.9f;
		boxDesc.dimensions		= extents*0.8f;
//		boxDesc.dimensions		= extents*1.1f;
//		boxDesc.dimensions		= extents*2.0f;
//		boxDesc.dimensions		= extents*0.5f;
//		boxDesc.dimensions		= extents;
//		boxDesc.dimensions		= extents*1.5f;
//		boxDesc.dimensions		= extents*0.05f;

//		boxDesc.dimensions		= extents;
//		boxDesc.dimensions.y	+= stepOffset*1.1f;


		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&boxDesc);
		actorDesc.body			= &bodyDesc;
		actorDesc.density		= 10000.0f;	// ### expose this ?
		actorDesc.density		= 10.0f;	// ### expose this ?
		// LOSS OF ACCURACY
		actorDesc.globalPose.t.x  = (float)position.x;
		actorDesc.globalPose.t.y  = (float)position.y;
		actorDesc.globalPose.t.z  = (float)position.z;
		kineActor = scene->createActor(actorDesc);
  }
}

BoxController::~BoxController()
	{
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BoxController::reportSceneChanged()
	{
	cctModule.VoidTestCache();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BoxController::getWorldBox(NxExtendedBounds3& box) const
	{
	box.setCenterExtents(position, extents);
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const NxVec3& BoxController::getExtents() const
	{
	return extents;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BoxController::setExtents(const NxVec3& e)
	{
	extents	= e;
	if(kineActor)
		{
		NxShape* S = *kineActor->getShapes();
		NX_ASSERT(S && static_cast<NxBoxShape*>(S));
		NxBoxShape* BS = static_cast<NxBoxShape*>(S);
		BS->setDimensions(e * 0.8f);
		}
	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BoxController::setStepOffset(const float offset)
	{
    stepOffset = offset;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

