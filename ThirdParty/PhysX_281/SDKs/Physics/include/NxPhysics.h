#ifndef NX_PHYSICS_NXPHYSICS
#define NX_PHYSICS_NXPHYSICS
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

/**
This is the main include header for the Physics SDK, for users who
want to use a single #include file.

Alternatively, one can instead directly #include a subset of the below files.
*/

#include "NxFoundation.h"		//include the all of the foundation SDK 


//////////////general:

#include "NxScene.h"
#include "NxSceneDesc.h"

#include "NxCompartment.h"

#include "NxActor.h"
#include "NxActorDesc.h"

#include "NxMaterial.h"
#include "NxMaterialDesc.h"

#include "NxContactStreamIterator.h"

#include "NxUserContactReport.h"
#include "NxUserNotify.h"
#include "NxFluidUserNotify.h"
#include "NxClothUserNotify.h"
#include "NxSoftBodyUserNotify.h"
#include "NxUserRaycastReport.h"
#include "NxUserEntityReport.h"

#include "NxBodyDesc.h"

#include "NxEffector.h"
#include "NxEffectorDesc.h"

#include "NxSpringAndDamperEffector.h"
#include "NxSpringAndDamperEffectorDesc.h"

#include "NxForceField.h"
#include "NxForceFieldDesc.h"
#include "NxForceFieldShapeGroup.h"
#include "NxForceFieldShapeGroupDesc.h"
#include "NxForceFieldKernel.h"
#include "NxForceFieldLinearKernel.h"
#include "NxForceFieldLinearKernelDesc.h"

#include "NxForceFieldShape.h"
#include "NxForceFieldShapeDesc.h"

#include "NxBoxForceFieldShape.h"
#include "NxBoxForceFieldShapeDesc.h"
#include "NxSphereForceFieldShape.h"
#include "NxSphereForceFieldShapeDesc.h"
#include "NxCapsuleForceFieldShape.h"
#include "NxCapsuleForceFieldShapeDesc.h"
#include "NxConvexForceFieldShape.h"
#include "NxConvexForceFieldShapeDesc.h"

#include "NxScheduler.h"

#if NX_USE_FLUID_API
#include "fluids/NxFluid.h"
#include "fluids/NxFluidDesc.h"
#include "fluids/NxFluidEmitter.h"
#include "fluids/NxFluidEmitterDesc.h"
#endif

#if NX_USE_CLOTH_API
#include "cloth/NxCloth.h"
#include "cloth/NxClothDesc.h"
#endif

#if NX_USE_SOFTBODY_API
#include "softbody/NxSoftBody.h"
#include "softbody/NxSoftBodyDesc.h"
#endif

#include "NxCCDSkeleton.h"
#include "NxTriangle.h"
#include "NxScheduler.h"
#include "NxSceneStats.h"
#include "NxSceneStats2.h"
/////////////joints:

#include "NxJoint.h"

#include "NxJointLimitDesc.h"
#include "NxJointLimitPairDesc.h"
#include "NxMotorDesc.h"
#include "NxSpringDesc.h"

#include "NxPointInPlaneJoint.h"
#include "NxPointInPlaneJointDesc.h"

#include "NxPointOnLineJoint.h"
#include "NxPointOnLineJointDesc.h"

#include "NxRevoluteJoint.h"
#include "NxRevoluteJointDesc.h"

#include "NxPrismaticJoint.h"
#include "NxPrismaticJointDesc.h"

#include "NxCylindricalJoint.h"
#include "NxCylindricalJointDesc.h"

#include "NxSphericalJoint.h"
#include "NxSphericalJointDesc.h"

#include "NxFixedJoint.h"
#include "NxFixedJointDesc.h"

#include "NxDistanceJoint.h"
#include "NxDistanceJointDesc.h"

#include "NxPulleyJoint.h"
#include "NxPulleyJointDesc.h"

#include "NxD6Joint.h"
#include "NxD6JointDesc.h"

//////////////shapes:

#include "NxShape.h"
#include "NxShapeDesc.h"

#include "NxBoxShape.h"
#include "NxBoxShapeDesc.h"

#include "NxCapsuleShape.h"
#include "NxCapsuleShapeDesc.h"

#include "NxPlaneShape.h"
#include "NxPlaneShapeDesc.h"

#include "NxSphereShape.h"
#include "NxSphereShapeDesc.h"

#include "NxTriangleMesh.h"
#include "NxTriangleMeshDesc.h"

#include "NxTriangleMeshShape.h"
#include "NxTriangleMeshShapeDesc.h"

#include "NxConvexMesh.h"
#include "NxConvexMeshDesc.h"

#include "NxConvexShape.h"
#include "NxConvexShapeDesc.h"

#include "NxHeightField.h"
#include "NxHeightFieldDesc.h"

#include "NxHeightFieldShape.h"
#include "NxHeightFieldShapeDesc.h"
#include "NxHeightFieldSample.h"

#include "NxWheelShape.h"
#include "NxWheelShapeDesc.h"
//////////////utils:

#include "NxInertiaTensor.h"
#include "NxIntersectionBoxBox.h"
#include "NxIntersectionPointTriangle.h"
#include "NxIntersectionRayPlane.h"
#include "NxIntersectionRaySphere.h"
#include "NxIntersectionRayTriangle.h"
#include "NxIntersectionSegmentBox.h"
#include "NxIntersectionSegmentCapsule.h"
#include "NxIntersectionSweptSpheres.h"
#include "NxPMap.h"
#include "NxSmoothNormals.h"
#include "NxAllocateable.h"
#include "NxExportedUtils.h"

#include "PhysXLoader.h"

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
