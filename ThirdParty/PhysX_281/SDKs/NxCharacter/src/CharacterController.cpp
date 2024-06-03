/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "CharacterController.h"
#include "SweptBox.h"
#include "SweptCapsule.h"
#include "NxController.h"
#include "NxRay.h"
#include "PhysXLoader.h"
#include "CCTDebugRenderer.h"

#define ASSERT		assert
#define INVALID_ID	0xffffffff

static NxUtilLib* gUtilLib = NULL;

#define	MAX_ITER	10

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	NX_INLINE void CollisionResponse(NxExtendedVec3& target_position, const NxExtendedVec3& current_position, const NxVec3& current_dir, const NxVec3& hit_normal, NxF32 bump, NxF32 friction, bool normalize=false)
	{
		// Compute reflect direction
		NxVec3 ReflectDir;
		ComputeReflexionVector(ReflectDir, current_dir, hit_normal);
		ReflectDir.normalize();

		// Decompose it
		NxVec3 NormalCompo, TangentCompo;
		DecomposeVector(NormalCompo, TangentCompo, ReflectDir, hit_normal);

		// Compute new destination position
		const Extended Amplitude = target_position.distance(current_position);

		target_position = current_position;
		if(bump!=0.0f)
		{
			if(normalize)	NormalCompo.normalize();
			target_position += NormalCompo*float(bump*Amplitude);
		}
		if(friction!=0.0)
		{
			if(normalize)	TangentCompo.normalize();
			target_position += TangentCompo*float(friction*Amplitude);
		}
	}

NX_INLINE void RelocateBox(NxBox& box, const NxExtendedVec3& center, const NxVec3& extents, const NxExtendedVec3& origin)
{
	box.center.x	= float(center.x - origin.x);
	box.center.y	= float(center.y - origin.y);
	box.center.z	= float(center.z - origin.z);
	box.extents		= extents;
	box.rot.id();
}

NX_INLINE void RelocateCapsule(NxCapsule& capsule, const SweptCapsule* sc, NxU32 up_direction, const NxExtendedVec3& center, const NxExtendedVec3& origin)
{
	sc->GetLocalCapsule(capsule, up_direction);
	capsule.p0.x += float(center.x - origin.x);
	capsule.p0.y += float(center.y - origin.y);
	capsule.p0.z += float(center.z - origin.z);
	capsule.p1.x += float(center.x - origin.x);
	capsule.p1.y += float(center.y - origin.y);
	capsule.p1.z += float(center.z - origin.z);
}

static bool SweepBoxUserBox(const SweepTest*, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_BOX);
	ASSERT(geom->mType==TOUCHED_USER_BOX);
	const SweptBox* SB = static_cast<const SweptBox*>(volume);
	const TouchedUserBox* TC = static_cast<const TouchedUserBox*>(geom);

	NxBox Box0;	// To precompute
	RelocateBox(Box0, center, SB->mExtents, TC->mOffset);

	NxBox Box1;
	TC->Relocate(Box1);

	NxVec3 Hit, Normal;
	float t;
	if(!gUtilLib->NxSweepBoxBox(Box0, Box1, dir, impact.mDistance, Hit, Normal, t))	return false;
	if(t>=impact.mDistance)															return false;

	impact.mWorldNormal = Normal;
	impact.mDistance	= t;
	impact.mIndex		= INVALID_ID;
	impact.mWorldPos.x	= Hit.x + TC->mOffset.x;
	impact.mWorldPos.y	= Hit.y + TC->mOffset.y;
	impact.mWorldPos.z	= Hit.z + TC->mOffset.z;
	return true;
}

static bool SweepBoxUserCapsule(const SweepTest*, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_BOX);
	ASSERT(geom->mType==TOUCHED_USER_CAPSULE);
	const SweptBox* SB = static_cast<const SweptBox*>(volume);
	const TouchedUserCapsule* TC = static_cast<const TouchedUserCapsule*>(geom);

	NxBox Box0;	// To precompute
	RelocateBox(Box0, center, SB->mExtents, TC->mOffset);

	NxCapsule Capsule;
	TC->Relocate(Capsule);

	float d;
	NxVec3 nrm;
	if(!gUtilLib->NxSweepBoxCapsule(Box0, Capsule, -dir, impact.mDistance, d, nrm))	return false;
	if(d>=impact.mDistance)															return false;

	impact.mDistance	= d;
	impact.mWorldNormal	= -nrm;
	impact.mIndex		= INVALID_ID;
	if(d==0.0f)
	{
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
// ### this fixes the bug on box-capsule but I'm not sure it's valid:
// - when the capsule is moving, it's ok to return false
// - when the box is moving, it's not! because it means the motion is completely free!!
		return false;
	}
	else
	{
		// ### check this
		float t;
		NxVec3 p;
		float d = gUtilLib->NxSegmentOBBSqrDist(Capsule, Box0.center, Box0.extents, Box0.rot, &t, &p);
		Box0.rot.multiply(p,p);
		impact.mWorldPos.x = p.x + Box0.center.x + TC->mOffset.x;
		impact.mWorldPos.y = p.y + Box0.center.y + TC->mOffset.y;
		impact.mWorldPos.z = p.z + Box0.center.z + TC->mOffset.z;
	}
	return true;
}

static bool SweepBoxMesh(const SweepTest* sweep_test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_BOX);
	ASSERT(geom->mType==TOUCHED_MESH);
	const SweptBox* SB = static_cast<const SweptBox*>(volume);
	const TouchedMesh* TM = static_cast<const TouchedMesh*>(geom);

	NxU32 NbTris = TM->mNbTris;
	if(!NbTris)	return false;

	// Fetch triangle data for current mesh (the stream may contain triangles from multiple meshes)
	const NxTriangle* T		= &sweep_test->mWorldTriangles[TM->mIndexWorldTriangles];
	const NxTriangle* ET	= &sweep_test->mWorldEdgeNormals[TM->mIndexWorldEdgeNormals];
	const NxU32* EdgeFlags	= &sweep_test->mEdgeFlags[TM->mIndexEdgeFlags];

	NxBounds3 Box;
	Box.setCenterExtents(NxVec3(float(center.x - TM->mOffset.x), float(center.y - TM->mOffset.y), float(center.z - TM->mOffset.z)), SB->mExtents);	// Precompute

	// PT: this only really works when the CCT collides with a single mesh, but that's the most common case. When it doesn't, there's just no speedup but it still works.
	NxU32 CachedIndex = sweep_test->mCachedTriIndex[sweep_test->mCachedTriIndexIndex];
	if(CachedIndex>=NbTris)	CachedIndex=0;

	NxVec3 Hit, Normal;
	float t;
	NxU32 Index;
	if(gUtilLib->NxSweepBoxTriangles(NbTris, T, ET, EdgeFlags, Box, dir, impact.mDistance, Hit, Normal, t, Index, &CachedIndex))
	{
		if(t>=impact.mDistance)			return false;

		impact.mDistance	= t;
		impact.mWorldNormal	= Normal;
		impact.mWorldPos.x	= Hit.x + TM->mOffset.x;
		impact.mWorldPos.y	= Hit.y + TM->mOffset.y;
		impact.mWorldPos.z	= Hit.z + TM->mOffset.z;

		// Returned index is only between 0 and NbTris, i.e. it indexes the array of cached triangles, not the original mesh.
		assert(Index<NbTris);
		sweep_test->mCachedTriIndex[sweep_test->mCachedTriIndexIndex] = Index;

		// The CCT loop will use the index from the start of the cache...
		impact.mIndex = Index + TM->mIndexWorldTriangles;
		return true;
	}
	return false;
}

static bool SweepBoxBox(const SweepTest*, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_BOX);
	ASSERT(geom->mType==TOUCHED_BOX);
	const SweptBox* SB = static_cast<const SweptBox*>(volume);
	const TouchedBox* TB = static_cast<const TouchedBox*>(geom);

	NxBox Box0;	// To precompute
	RelocateBox(Box0, center, SB->mExtents, TB->mOffset);

	NxVec3 Hit, Normal;
	float t;
	if(!gUtilLib->NxSweepBoxBox(Box0, TB->mBox, dir, impact.mDistance, Hit, Normal, t))	return false;
	if(t>=impact.mDistance)																return false;

	impact.mWorldNormal = Normal;
	impact.mDistance	= t;
	impact.mIndex		= INVALID_ID;
	impact.mWorldPos.x	= Hit.x + TB->mOffset.x;
	impact.mWorldPos.y	= Hit.y + TB->mOffset.y;
	impact.mWorldPos.z	= Hit.z + TB->mOffset.z;
	return true;
}

static bool SweepBoxSphere(const SweepTest*, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_BOX);
	ASSERT(geom->mType==TOUCHED_SPHERE);
	const SweptBox* SB = static_cast<const SweptBox*>(volume);
	const TouchedSphere* TS = static_cast<const TouchedSphere*>(geom);

	NxBox Box0;	// To precompute
	RelocateBox(Box0, center, SB->mExtents, TS->mOffset);

	float d;
	NxVec3 nrm;
	if(!gUtilLib->NxSweepBoxSphere(Box0, TS->mSphere, -dir, impact.mDistance, d, nrm))	return false;
	if(d>=impact.mDistance)																return false;

	impact.mDistance	= d;
	impact.mWorldNormal	= -nrm;
	impact.mIndex		= INVALID_ID;
	if(d==0.0f)
	{
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
		return false;
	}
	else
	{
		// The sweep test doesn't compute the impact point automatically, so we have to do it here.
		NxVec3 NewSphereCenter = TS->mSphere.center - d * dir;
		NxVec3 Closest;
		gUtilLib->NxPointOBBSqrDist(NewSphereCenter, Box0.center, Box0.extents, Box0.rot, &Closest);
		// Compute point on the box, after sweep
		Box0.rot.multiply(Closest, Closest);
		impact.mWorldPos.x = TS->mOffset.x + Closest.x + Box0.center.x + d * dir.x;
		impact.mWorldPos.y = TS->mOffset.y + Closest.y + Box0.center.y + d * dir.y;
		impact.mWorldPos.z = TS->mOffset.z + Closest.z + Box0.center.z + d * dir.z;

		impact.mWorldNormal.setNegative();
	}
	return true;
}

static bool SweepBoxCapsule(const SweepTest*, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_BOX);
	ASSERT(geom->mType==TOUCHED_CAPSULE);
	const SweptBox* SB = static_cast<const SweptBox*>(volume);
	const TouchedCapsule* TC = static_cast<const TouchedCapsule*>(geom);

	NxBox Box0;	// To precompute
	RelocateBox(Box0, center, SB->mExtents, TC->mOffset);

	float d;
	NxVec3 nrm;
	if(!gUtilLib->NxSweepBoxCapsule(Box0, TC->mCapsule, -dir, impact.mDistance, d, nrm))	return false;
	if(d>=impact.mDistance)																	return false;

	impact.mDistance	= d;
	impact.mWorldNormal	= -nrm;
	impact.mIndex		= INVALID_ID;
	if(d==0.0f)
	{
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
// ### this fixes the bug on box-capsule but I'm not sure it's valid:
// - when the capsule is moving, it's ok to return false
// - when the box is moving, it's not! because it means the motion is completely free!!
		return false;
	}
	else
	{
		float t;
		NxVec3 p;
		float d = gUtilLib->NxSegmentOBBSqrDist(TC->mCapsule, Box0.center, Box0.extents, Box0.rot, &t, &p);
		Box0.rot.multiply(p,p);
		impact.mWorldPos.x = p.x + Box0.center.x + TC->mOffset.x;
		impact.mWorldPos.y = p.y + Box0.center.y + TC->mOffset.y;
		impact.mWorldPos.z = p.z + Box0.center.z + TC->mOffset.z;
	}
	return true;
}

static bool SweepCapsuleMesh(const SweepTest* sweep_test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_CAPSULE);
	ASSERT(geom->mType==TOUCHED_MESH);
	const SweptCapsule* SC = static_cast<const SweptCapsule*>(volume);
	const TouchedMesh* TM = static_cast<const TouchedMesh*>(geom);

	NxU32 NbTris = TM->mNbTris;
	if(!NbTris)	return false;

	// Fetch triangle data for current mesh (the stream may contain triangles from multiple meshes)
	const NxTriangle* T		= &sweep_test->mWorldTriangles[TM->mIndexWorldTriangles];
//	const NxTriangle* ET	= &sweep_test->mWorldEdgeNormals[TM->mIndexWorldEdgeNormals];
	const NxU32* EdgeFlags	= &sweep_test->mEdgeFlags[TM->mIndexEdgeFlags];

	NxVec3 C0(float(center.x - TM->mOffset.x), float(center.y - TM->mOffset.y), float(center.z - TM->mOffset.z));

	// PT: this only really works when the CCT collides with a single mesh, but that's the most common case. When it doesn't, there's just no speedup but it still works.
	NxU32 CachedIndex = sweep_test->mCachedTriIndex[sweep_test->mCachedTriIndexIndex];
	if(CachedIndex>=NbTris)	CachedIndex=0;

	NxVec3 Hit, Normal;
	float t;
	NxU32 Index;
	if(gUtilLib->NxSweepCapsuleTriangles(sweep_test->mUpDirection, NbTris, T, EdgeFlags,
							C0, SC->mRadius, SC->mHeight,
							dir, impact.mDistance,
							Hit, Normal, t, Index, &CachedIndex))
	{
		if(t>=impact.mDistance)			return false;

		impact.mDistance	= t;
		impact.mWorldNormal	= Normal;
		impact.mWorldPos.x	= Hit.x + TM->mOffset.x;
		impact.mWorldPos.y	= Hit.y + TM->mOffset.y;
		impact.mWorldPos.z	= Hit.z + TM->mOffset.z;

		// Returned index is only between 0 and NbTris, i.e. it indexes the array of cached triangles, not the original mesh.
		assert(Index<NbTris);
		sweep_test->mCachedTriIndex[sweep_test->mCachedTriIndexIndex] = Index;

		// The CCT loop will use the index from the start of the cache...
		impact.mIndex = Index + TM->mIndexWorldTriangles;

		return true;
	}
	return false;
}

static bool SweepCapsuleBox(const SweepTest* test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_CAPSULE);
	ASSERT(geom->mType==TOUCHED_BOX);
	const SweptCapsule* SC = static_cast<const SweptCapsule*>(volume);
	const TouchedBox* TB = static_cast<const TouchedBox*>(geom);

	NxCapsule Capsule;
	RelocateCapsule(Capsule, SC, test->mUpDirection, center, TB->mOffset);

	// The box and capsule coordinates are relative to the center of the cached bounding box
	float d;
	NxVec3 nrm;
	if(!gUtilLib->NxSweepBoxCapsule(TB->mBox, Capsule, dir, impact.mDistance, d, nrm))	return false;
	if(d>=impact.mDistance)																return false;

	impact.mDistance	= d;
	impact.mWorldNormal	= nrm;
	impact.mIndex		= INVALID_ID;

	if(d==0.0f)
	{
	// ### this part makes the capsule goes through the box sometimes
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
	// ### this fixes the bug on box-capsule but I'm not sure it's valid:
	// - when the capsule is moving, it's ok to return false
	// - when the box is moving, it's not! because it means the motion is completely free!!
		return false;
	}
	else
	{
		float t;
		NxVec3 p;
		float d = gUtilLib->NxSegmentOBBSqrDist(Capsule, TB->mBox.center, TB->mBox.extents, TB->mBox.rot, &t, &p);
		TB->mBox.rot.multiply(p,p);
		p += TB->mBox.center;
		impact.mWorldPos.x = p.x + TB->mOffset.x;
		impact.mWorldPos.y = p.y + TB->mOffset.y;
		impact.mWorldPos.z = p.z + TB->mOffset.z;
	}
	return true;
}

static bool SweepCapsuleSphere(const SweepTest* test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_CAPSULE);
	ASSERT(geom->mType==TOUCHED_SPHERE);
	const SweptCapsule* SC = static_cast<const SweptCapsule*>(volume);
	const TouchedSphere* TS = static_cast<const TouchedSphere*>(geom);

	NxCapsule Capsule;
	RelocateCapsule(Capsule, SC, test->mUpDirection, center, TS->mOffset);

	float d;
	NxVec3 nrm;
	NxVec3 ip;
	if(!gUtilLib->NxSweepSphereCapsule(TS->mSphere, Capsule, -dir, impact.mDistance, d, ip, nrm))	return false;
	if(d>=impact.mDistance)																			return false;

	impact.mDistance	= d;
	impact.mWorldPos.x	= ip.x;
	impact.mWorldPos.y	= ip.y;
	impact.mWorldPos.z	= ip.z;
	impact.mWorldNormal	= nrm;
	impact.mIndex		= INVALID_ID;
	if(d==0.0f)
	{
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
		return false;
	}
	else
	{
		impact.mWorldPos.x += TS->mOffset.x;
		impact.mWorldPos.y += TS->mOffset.y;
		impact.mWorldPos.z += TS->mOffset.z;
	}
	return true;
}


static bool SweepCapsuleCapsule(const SweepTest* test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_CAPSULE);
	ASSERT(geom->mType==TOUCHED_CAPSULE);
	const SweptCapsule* SC = static_cast<const SweptCapsule*>(volume);
	const TouchedCapsule* TC = static_cast<const TouchedCapsule*>(geom);

	NxCapsule Capsule;
	RelocateCapsule(Capsule, SC, test->mUpDirection, center, TC->mOffset);

	float d;
	NxVec3 nrm;
	NxVec3 ip;
	if(!gUtilLib->NxSweepCapsuleCapsule(Capsule, TC->mCapsule, -dir, impact.mDistance, d, ip, nrm))	return false;
	if(d>=impact.mDistance)																			return false;

	impact.mDistance	= d;
	impact.mWorldPos.x	= ip.x;
	impact.mWorldPos.y	= ip.y;
	impact.mWorldPos.z	= ip.z;
	impact.mWorldNormal	= nrm;
	impact.mIndex		= INVALID_ID;
	if(d==0.0f)
	{
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
		return false;
	}
	else
	{
		impact.mWorldPos.x += TC->mOffset.x;
		impact.mWorldPos.y += TC->mOffset.y;
		impact.mWorldPos.z += TC->mOffset.z;
	}
	return true;
}

static bool SweepCapsuleUserCapsule(const SweepTest* test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_CAPSULE);
	ASSERT(geom->mType==TOUCHED_USER_CAPSULE);
	const SweptCapsule* SC = static_cast<const SweptCapsule*>(volume);
	const TouchedUserCapsule* TC = static_cast<const TouchedUserCapsule*>(geom);

	NxCapsule Capsule;
	RelocateCapsule(Capsule, SC, test->mUpDirection, center, TC->mOffset);

	NxCapsule TouchedCapsule;
	TC->Relocate(TouchedCapsule);

	float d;
	NxVec3 nrm;
	NxVec3 ip;
	if(!gUtilLib->NxSweepCapsuleCapsule(Capsule, TouchedCapsule, -dir, impact.mDistance, d, ip, nrm))	return false;
	if(d>=impact.mDistance)																				return false;

	impact.mDistance	= d;
	impact.mWorldPos.x	= ip.x;
	impact.mWorldPos.y	= ip.y;
	impact.mWorldPos.z	= ip.z;
	impact.mWorldNormal	= nrm;
	impact.mIndex		= INVALID_ID;
	if(d==0.0f)
	{
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
		return false;
	}
	else
	{
		impact.mWorldPos.x += TC->mOffset.x;
		impact.mWorldPos.y += TC->mOffset.y;
		impact.mWorldPos.z += TC->mOffset.z;
	}
	return true;
}

static bool SweepCapsuleUserBox(const SweepTest* test, const SweptVolume* volume, const TouchedGeom* geom, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	ASSERT(volume->GetType()==SWEPT_CAPSULE);
	ASSERT(geom->mType==TOUCHED_USER_BOX);
	const SweptCapsule* SC = static_cast<const SweptCapsule*>(volume);
	const TouchedUserBox* TB = static_cast<const TouchedUserBox*>(geom);

	NxCapsule Capsule;
	RelocateCapsule(Capsule, SC, test->mUpDirection, center, TB->mOffset);

	NxBox Box;
	TB->Relocate(Box);

	float d;
	NxVec3 nrm;
	if(!gUtilLib->NxSweepBoxCapsule(Box, Capsule, dir, impact.mDistance, d, nrm))	return false;
	if(d>=impact.mDistance)															return false;

	impact.mDistance	= d;
	impact.mWorldNormal	= nrm;
	impact.mIndex		= INVALID_ID;

	if(d==0.0f)
	{
	// ### this part makes the capsule goes through the box sometimes
		impact.mWorldPos.zero();
		impact.mWorldNormal.zero();
	// ### this fixes the bug on box-capsule but I'm not sure it's valid:
	// - when the capsule is moving, it's ok to return false
	// - when the box is moving, it's not! because it means the motion is completely free!!
		return false;
	}
	else
	{
		// ### check this
		float t;
		NxVec3 p;
		float d = gUtilLib->NxSegmentOBBSqrDist(Capsule, Box.center, Box.extents, Box.rot, &t, &p);
		p += Box.center;
		impact.mWorldPos.x = p.x + TB->mOffset.x;
		impact.mWorldPos.y = p.y + TB->mOffset.y;
		impact.mWorldPos.z = p.z + TB->mOffset.z;
	}
	return true;
}

typedef bool (*SweepFunc) (const SweepTest*, const SweptVolume*, const TouchedGeom*, const NxExtendedVec3&, const NxVec3&, SweptContact&);

static SweepFunc gSweepMap[SWEPT_LAST][TOUCHED_LAST] = {
	// Box funcs
	SweepBoxUserBox,
	SweepBoxUserCapsule,
	SweepBoxMesh,
	SweepBoxBox,
	SweepBoxSphere,
	SweepBoxCapsule,

	// Capsule funcs
	SweepCapsuleUserBox,
	SweepCapsuleUserCapsule,
	SweepCapsuleMesh,
	SweepCapsuleBox,
	SweepCapsuleSphere,
	SweepCapsuleCapsule,
};
NX_COMPILE_TIME_ASSERT(sizeof(gSweepMap)==SWEPT_LAST*TOUCHED_LAST*sizeof(SweepFunc));

static bool CollideGeoms(const SweepTest* sweep_test, const SweptVolume& volume, const IntArray& geom_stream, const NxExtendedVec3& center, const NxVec3& dir, SweptContact& impact)
{
	impact.mIndex	= INVALID_ID;
	impact.mGeom	= NULL;

	static const NxU32 GeomSizes[] = 
	{
		sizeof(TouchedUserBox),
		sizeof(TouchedUserCapsule),
		sizeof(TouchedMesh),
		sizeof(TouchedBox),
		sizeof(TouchedSphere),
		sizeof(TouchedCapsule),
	};

	bool Status = false;
	const NxU32* Data = geom_stream.begin();
	const NxU32* Last = geom_stream.end();
	while(Data!=Last)
	{
		TouchedGeom* CurrentGeom = (TouchedGeom*)Data;

		SweepFunc ST = gSweepMap[volume.GetType()][CurrentGeom->mType];
		if(ST)
		{
			SweptContact C;
			C.mDistance = impact.mDistance;	// Initialize with current best distance
			C.mIndex	= INVALID_ID;
			if((ST)(sweep_test, &volume, CurrentGeom, center, dir, C))
			{
				if(C.mDistance<impact.mDistance)
				{
					impact = C;
					impact.mGeom = CurrentGeom;
					Status = true;
				}
			}
		}

		NxU8* ptr = (NxU8*)Data;
		ptr += GeomSizes[CurrentGeom->mType];
		Data = (const NxU32*)ptr;
	}
	return Status;
}





SweepTest::SweepTest() :
	debugData			(NULL),
	mValidTri			(false),
	mValidateCallback	(false),
	mNormalizeResponse	(false)
{
	mCachedTBV.setEmpty();
	mCachedTriIndexIndex	= 0;
	mCachedTriIndex[0] = mCachedTriIndex[1] = mCachedTriIndex[2] = 0;
	mNbCachedStatic = 0;
	mNbCachedT		= 0;
	mNbCachedEN		= 0;
	mNbCachedF		= 0;
	mHandleSlope	= false;
	mSlopeLimit		= 0.0f;
	mSkinWidth		= 0.0f;
	mStepOffset		= 0.0f;
	mUpDirection	= 0;
//	mVolumeGrowth	= 1.2f;	// Must be >1.0f and not too big
	mVolumeGrowth	= 1.5f;	// Must be >1.0f and not too big
//	mVolumeGrowth	= 2.0f;	// Must be >1.0f and not too big
	mHitNonWalkable	= false;
	mWalkExperiment	= false;
	mMaxIter		= MAX_ITER;
	mFirstUpdate	= false;
}

SweepTest::~SweepTest()
{
}

void SweepTest::FindTouchedCCTs(	NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
									NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
									const NxExtendedBounds3& world_box)
{
	NxExtendedVec3 Origin;	// Will be TouchedGeom::mOffset
	world_box.getCenter(Origin);

	// Find touched boxes, i.e. other box controllers
	for(NxU32 i=0;i<nb_boxes;i++)
	{
		if(!world_box.intersect(boxes[i]))
			continue;

		TouchedUserBox* UserBox = (TouchedUserBox*)reserve(mGeomStream, sizeof(TouchedUserBox)/sizeof(NxU32));
		UserBox->mType		= TOUCHED_USER_BOX;
		UserBox->mUserData	= box_user_data[i];
		UserBox->mOffset	= Origin;
		UserBox->mBox		= boxes[i];
	}

	// Find touched capsules, i.e. other capsule controllers
	NxExtendedVec3 Center;
	NxVec3 Extents;
	world_box.getCenter(Center);
	world_box.getExtents(Extents);
	NxMat33 Idt;
	Idt.id();
	for(NxU32 i=0;i<nb_capsules;i++)
	{
		// Do a quick AABB check first, to avoid calling the SDK too much
		const NxF32 r = capsules[i].radius;
		if((capsules[i].p0.x - r > world_box.max.x) || (world_box.min.x > capsules[i].p1.x + r)) continue;
		if((capsules[i].p0.y - r > world_box.max.y) || (world_box.min.y > capsules[i].p1.y + r)) continue;
		if((capsules[i].p0.z - r > world_box.max.z) || (world_box.min.z > capsules[i].p1.z + r)) continue;

		// Do a box-capsule intersect, or skip it? => better to skip it, not really useful now
/*		NxCapsule tmp;
		tmp.radius = capsules[i].radius;
		tmp.p0.x = float(capsules[i].p0.x);
		tmp.p0.y = float(capsules[i].p0.y);
		tmp.p0.z = float(capsules[i].p0.z);
		tmp.p1.x = float(capsules[i].p1.x);
		tmp.p1.y = float(capsules[i].p1.y);
		tmp.p1.z = float(capsules[i].p1.z);
		float d2 = gUtilLib->NxSegmentOBBSqrDist(tmp, NxVec3(float(Center.x), float(Center.y), float(Center.z)), 
											Extents, 
											Idt, NULL, NULL);
		if(d2<capsules[i].radius*capsules[i].radius)*/
		{
			TouchedUserCapsule* UserCapsule = (TouchedUserCapsule*)reserve(mGeomStream, sizeof(TouchedUserCapsule)/sizeof(NxU32));
			UserCapsule->mType		= TOUCHED_USER_CAPSULE;
			UserCapsule->mUserData	= capsule_user_data[i];
			UserCapsule->mOffset	= Origin;
			UserCapsule->mCapsule	= capsules[i];
		}
	}
}

static NxU32 gNbIters = 0;
static NxU32 gNbFullUpdates = 0;
static NxU32 gNbPartialUpdates = 0;

void SweepTest::UpdateTouchedGeoms(	void* user_data, const SweptVolume& swept_volume,
									NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
									NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
									NxU32 group_flags, const NxExtendedBounds3& world_box, const NxGroupsMask* groupsMask)
{
	/*
	- if this is the first iteration (new frame) we have to redo the dynamic objects & the CCTs. The static objects can
	be cached.
	- if this is not, we can cache everything
	*/

	// PT: using "world_box" instead of "mCachedTBV" seems to produce TTP 6207
//#define DYNAMIC_BOX	world_box
#define DYNAMIC_BOX	mCachedTBV

	bool NewCachedBox = false;

	// If the input box is inside the cached box, nothing to do
	if(world_box.isInside(mCachedTBV))
	{
		if(mFirstUpdate)
		{
			mFirstUpdate = false;

			// Only redo the dynamic
			mGeomStream.erase(&mGeomStream[mNbCachedStatic]);
			mWorldTriangles.erase(&mWorldTriangles[mNbCachedT]);
			mWorldEdgeNormals.erase(&mWorldEdgeNormals[mNbCachedEN]);
			mEdgeFlags.erase(&mEdgeFlags[mNbCachedF]);

			FindTouchedGeometry(user_data, DYNAMIC_BOX, mWorldTriangles, swept_volume.GetType()==SWEPT_BOX ? &mWorldEdgeNormals : NULL, mEdgeFlags, mGeomStream, group_flags, false, true, groupsMask);
			FindTouchedCCTs(
				nb_boxes, boxes, box_user_data,
				nb_capsules, capsules, capsule_user_data,
				DYNAMIC_BOX
				);
			gNbPartialUpdates++;
		}
	}
	else
	{
		NewCachedBox = true;

		// Cache BV used for the query
		mCachedTBV = world_box;

		// Grow the volume a bit. The temporal box here doesn't take sliding & collision response into account.
		// In bad cases it is possible to eventually touch a portion of space not covered by this volume. Just
		// in case, we grow the initial volume slightly. Then, additional tests are performed within the loop
		// to make sure the TBV is always correct. There's a tradeoff between the original (artificial) growth
		// of the volume, and the number of TBV recomputations performed at runtime...
		if(1)
		{
			mCachedTBV.scale(mVolumeGrowth);
		}
		else
		{
			NxExtendedVec3 center;	mCachedTBV.getCenter(center);
			NxVec3 extents;	mCachedTBV.getExtents(extents);
/*			NxVec3 scale(mVolumeGrowth, mVolumeGrowth, mVolumeGrowth);
			scale[mUpDirection] = 1.0f;*/
/*			NxVec3 scale(1.0f, 1.0f, 1.0f);
			scale[mUpDirection] = mVolumeGrowth;
			extents.x *= scale.x;
			extents.y *= scale.y;
			extents.z *= scale.z;*/
			extents.x *= 1.8f;
			extents.y += 1.0f;
			extents.z *= 1.8f;
			mCachedTBV.setCenterExtents(center, extents);
		}

		// Gather triangles touched by this box. This covers multiple meshes.
		mWorldTriangles.clear();
		mWorldEdgeNormals.clear();
		mEdgeFlags.clear();
		mGeomStream.clear();
		mCachedTriIndexIndex	= 0;
		mCachedTriIndex[0] = mCachedTriIndex[1] = mCachedTriIndex[2] = 0;

		gNbFullUpdates++;
		FindTouchedGeometry(user_data, mCachedTBV, mWorldTriangles, swept_volume.GetType()==SWEPT_BOX ? &mWorldEdgeNormals : NULL, mEdgeFlags, mGeomStream, group_flags, true, false, groupsMask);
		mNbCachedStatic = mGeomStream.size();
		mNbCachedT = mWorldTriangles.size();
		mNbCachedEN = mWorldEdgeNormals.size();
		mNbCachedF = mEdgeFlags.size();

		FindTouchedGeometry(user_data, DYNAMIC_BOX, mWorldTriangles, swept_volume.GetType()==SWEPT_BOX ? &mWorldEdgeNormals : NULL, mEdgeFlags, mGeomStream, group_flags, false, true, groupsMask);
		// We can't early exit when no tris are touched since we also have to handle the boxes

		FindTouchedCCTs(
			nb_boxes, boxes, box_user_data,
			nb_capsules, capsules, capsule_user_data,
			DYNAMIC_BOX
			);

		mFirstUpdate = false;
	}

	if(debugData)
	{
		debugData->addAABB(mCachedTBV, NewCachedBox ? NX_ARGB_RED : NX_ARGB_GREEN);
		debugData->addAABB(world_box, NX_ARGB_YELLOW);
	}
}

// This is the generic sweep test for all swept volumes, but not character-controller specific
bool SweepTest::DoSweepTest(void* user_data,
							void* user_data2,
							NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
							NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
							SweptVolume& swept_volume,
							const NxVec3& direction, NxU32 max_iter, NxU32* nb_collisions,
							NxU32 group_flags, float min_dist, const NxGroupsMask* groupsMask, bool down_pass)
{
	// Early exit when motion is zero. Since the motion is decomposed into several vectors
	// and this function is called for each of them, it actually happens quite often.
	if(direction.isZero())
		return false;

	bool HasMoved = false;
	mValidTri = false;

	NxExtendedVec3 CurrentPosition = swept_volume.mCenter;
	NxExtendedVec3 TargetPosition = swept_volume.mCenter;
	TargetPosition += direction;

	NxU32 NbCollisions = 0;
	while(max_iter--)
	{
		gNbIters++;
		// Compute current direction
		NxVec3 CurrentDirection = TargetPosition - CurrentPosition;

		// Make sure the new TBV is still valid
		{
			// Compute temporal bounding box. We could use a capsule or an OBB instead:
			// - the volume would be smaller
			// - but the query would be slower
			// Overall it's unclear whether it's worth it or not.
			// TODO: optimize this part ?
			NxExtendedBounds3 TemporalBox;
			swept_volume.ComputeTemporalBox(*this, TemporalBox, CurrentPosition, CurrentDirection);

			// Gather touched geoms
			UpdateTouchedGeoms(user_data, swept_volume,
								nb_boxes, boxes, box_user_data,
								nb_capsules, capsules, capsule_user_data,
								group_flags, TemporalBox, groupsMask);
		}

		const float Length = CurrentDirection.magnitude();
		if(Length<min_dist)	break;

		CurrentDirection /= Length;

		// From Quake2: "if velocity is against the original velocity, stop dead to avoid tiny occilations in sloping corners"
		if((CurrentDirection|direction) <= 0.0f)	break;

		// From this point, we're going to update the position at least once
		HasMoved = true;

		// Find closest collision
		SweptContact C;
		C.mDistance = Length + mSkinWidth;

		if(!CollideGeoms(this, swept_volume, mGeomStream, CurrentPosition, CurrentDirection, C))
		{
			// no collision found => move to desired position
			CurrentPosition = TargetPosition;
			break;
		}

		ASSERT(C.mGeom);	// If we reach this point, we must have touched a geom

		if(C.mGeom->mType==TOUCHED_USER_BOX || C.mGeom->mType==TOUCHED_USER_CAPSULE)
		{
			// We touched a user object, typically another CCT
			if(mValidateCallback)	UserHitCallback(user_data2, C, CurrentDirection, Length);

			// Trying to solve the following problem:
			// - by default, the CCT "friction" is infinite, i.e. a CCT will not slide on a slope (this is by design)
			// - this produces bad results when a capsule CCT stands on top of another capsule CCT, without sliding. Visually it looks
			//   like the character is standing on the other character's head, it looks bad. So, here, we would like to let the CCT
			//   slide away, i.e. we don't want friction.
			// So here we simply increase the number of iterations (== let the CCT slide) when the first down collision is with another CCT.
			if(down_pass && !NbCollisions)
				max_iter += 9;
		}
		else
		{
			// We touched a normal object
#ifdef USE_CONTACT_NORMAL_FOR_SLOPE_TEST
			mValidTri = true;
			mCN = C.mWorldNormal;
#else
			if(C.mIndex!=INVALID_ID)
			{
				mValidTri = true;
				mTouched = mWorldTriangles[C.mIndex];
			}
#endif
			{
				if(mValidateCallback)	ShapeHitCallback(user_data2, C, CurrentDirection, Length);
			}
		}

		NbCollisions++;
		mContactPointHeight = (float)C.mWorldPos[mUpDirection];	// UBI

		const float DynSkin = mSkinWidth;

		if(C.mDistance>DynSkin/*+0.01f*/)
			CurrentPosition += CurrentDirection*(C.mDistance-DynSkin);

		NxVec3 WorldNormal = C.mWorldNormal;
		if(mWalkExperiment)
		{
			// Make sure the auto-step doesn't bypass this !
			WorldNormal[mUpDirection]=0.0f;
			WorldNormal.normalize();
		}

		const float Bump = 0.0f;	// ### doesn't work when !=0 because of Quake2 hack!
		const float Friction = 1.0f;
		CollisionResponse(TargetPosition, CurrentPosition, CurrentDirection, WorldNormal, Bump, Friction, mNormalizeResponse);
	}

	if(nb_collisions)	*nb_collisions = NbCollisions;

	// Final box position that should be reflected in the graphics engine
	swept_volume.mCenter = CurrentPosition;

	// If we didn't move, don't update the box position at all (keeping possible lazy-evaluated structures valid)
	return HasMoved;
}

// ### have a return code to tell if we really moved or not

// Using swept code & direct position update (no physics engine)
// This function is the generic character controller logic, valid for all swept volumes
void SweepTest::MoveCharacter(
					void* user_data,
					void* user_data2,
					SweptVolume& volume,
					const NxVec3& direction,
					NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
					NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
					NxU32 groups, float min_dist,
					NxU32& collision_flags,
					const NxGroupsMask* groupsMask,
					bool constrainedClimbingMode
					 )
{
	mHitNonWalkable = false;
	NxU32 CollisionFlags = 0;
	const NxU32 MaxIter = mMaxIter;	// 1 for "collide and stop"
	const NxU32 MaxIterUp = MaxIter;
	const NxU32 MaxIterSides = MaxIter;
//	const NxU32 MaxIterDown = gWalkExperiment ? MaxIter : 1;
	const NxU32 MaxIterDown = 1;

	// ### this causes the artificial gap on top of chars
	float StepOffset = mStepOffset;	// Default step offset can be cancelled in some cases.

	// Save initial height
	Extended OriginalHeight = volume.mCenter[mUpDirection];
	Extended OriginalBottomPoint = OriginalHeight - volume.mHalfHeight;	// UBI

	// TEST! Disable auto-step when flying. Not sure this is really useful.
	if(direction[mUpDirection]>0.0f)
		StepOffset = 0.0f;

	// Decompose motion into 3 independent motions: up, side, down
	// - if the motion is purely down (gravity only), the up part is needed to fight accuracy issues. For example if the
	// character is already touching the geometry a bit, the down sweep test might have troubles. If we first move it above
	// the geometry, the problems disappear.
	// - if the motion is lateral (character moving forward under normal gravity) the decomposition provides the autostep feature
	// - if the motion is purely up, the down part can be skipped

	NxVec3 UpVector(0.0f, 0.0f, 0.0f);
	NxVec3 DownVector(0.0f, 0.0f, 0.0f);

	if(direction[mUpDirection]<0.0f)	DownVector[mUpDirection] = direction[mUpDirection];
	else								UpVector[mUpDirection] = direction[mUpDirection];

	NxVec3 SideVector = direction;
	SideVector[mUpDirection] = 0.0f;

	// If the side motion is zero, i.e. if the character is not really moving, disable auto-step.
	if(!SideVector.isZero())
		UpVector[mUpDirection] += StepOffset;

	// ==========[ Initial volume query ]===========================
	if(1)
	{
		NxVec3 MotionExtents = UpVector;
		MotionExtents.max(SideVector);
		MotionExtents.max(DownVector);

		NxExtendedBounds3 TemporalBox;
		volume.ComputeTemporalBox(*this, TemporalBox, volume.mCenter, MotionExtents);

		// Gather touched geoms
		UpdateTouchedGeoms(user_data, volume,
							nb_boxes, boxes, box_user_data,
							nb_capsules, capsules, capsule_user_data,
							groups, TemporalBox, groupsMask);
	}

	// ==========[ UP PASS ]===========================

	mCachedTriIndexIndex = 0;
	const bool PerformUpPass = true;
	NxU32 NbCollisions=0;

	if(PerformUpPass)
	{
		// Prevent user callback for up motion. This up displacement is artificial, and only needed for auto-stepping.
		// If we call the user for this, we might eventually apply upward forces to objects resting on top of us, even
		// if we visually don't move. This produces weird-looking motions.
		mValidateCallback = false;

		// In the walk-experiment we explicitely want to ban any up motions, to avoid characters climbing slopes they shouldn't climb.
		// So let's bypass the whole up pass.
		if(!mWalkExperiment)
		{
			// ### MaxIter here seems to "solve" the V bug
			if(DoSweepTest(user_data,
				user_data2,
				nb_boxes, boxes, box_user_data,
				nb_capsules, capsules, capsule_user_data,
				volume, UpVector, MaxIterUp, &NbCollisions, groups, min_dist, groupsMask))
			{
				if(NbCollisions)
				{
					CollisionFlags |= NXCC_COLLISION_UP;

					// Clamp step offset to make sure we don't undo more than what we did
					Extended Delta = volume.mCenter[mUpDirection] - OriginalHeight;
					if(Delta<StepOffset)
					{
						StepOffset=float(Delta);
					}
				}
			}
		}
	}

	// ==========[ SIDE PASS ]===========================

	mCachedTriIndexIndex = 1;
	mValidateCallback = true;
	const bool PerformSidePass = true;

	if(PerformSidePass)
	{
		NbCollisions=0;
		if(DoSweepTest(user_data,
			user_data2,
			nb_boxes, boxes, box_user_data,
			nb_capsules, capsules, capsule_user_data,
			volume, SideVector, MaxIterSides, &NbCollisions, groups, min_dist, groupsMask))
		{
			if(NbCollisions)	CollisionFlags |= NXCC_COLLISION_SIDES;
		}
	}

	// ==========[ DOWN PASS ]===========================

	mCachedTriIndexIndex = 2;
	const bool PerformDownPass = true;

	if(PerformDownPass)
	{
		NbCollisions=0;

		if(!SideVector.isZero())	// We disabled that before so we don't have to undo it in that case
			DownVector[mUpDirection] -= StepOffset;	// Undo our artificial up motion

		mValidTri = false;

		// min_dist actually makes a big difference :(
		// AAARRRGGH: if we get culled because of min_dist here, mValidTri never becomes valid!
		if(DoSweepTest(user_data,
			user_data2,
			nb_boxes, boxes, box_user_data,
			nb_capsules, capsules, capsule_user_data,
			volume, DownVector, MaxIterDown, &NbCollisions, groups, min_dist, groupsMask, true))
		{
			if(NbCollisions)
			{
				CollisionFlags |= NXCC_COLLISION_DOWN;
				if(mHandleSlope)	// PT: I think the following fix shouldn't be performed when mHandleSlope is false.
				{
					// PT: the following code is responsible for a weird capsule behaviour,
					// when colliding against a highly tesselated terrain:
					// - with a large direction vector, the capsule gets stuck against some part of the terrain
					// - with a slower direction vector (but in the same direction!) the capsule manages to move
					// I will keep that code nonetheless, since it seems to be useful for them.

					// constrainedClimbingMode
					if ( constrainedClimbingMode && mContactPointHeight > OriginalBottomPoint + StepOffset)
					{
						mHitNonWalkable = true;
						if(!mWalkExperiment)
							return;
					}
					//~constrainedClimbingMode
				}
			}
		}

		// TEST: do another down pass if we're on a non-walkable poly
		// ### kind of works but still not perfect
		// ### could it be because we zero the Y impulse later?
		// ### also check clamped response vectors
		if(mHandleSlope && mValidTri && direction[mUpDirection]<0.0f)
		{
			NxVec3 Normal;
		#ifdef USE_CONTACT_NORMAL_FOR_SLOPE_TEST
			Normal = mCN;
		#else
			mTouched.normal(Normal);
		#endif
		//	if(fabsf(Normal|NxVec3(0.0f, 1.0f, 0.0f))<cosf(45.0f*DEGTORAD))
			if(Normal[mUpDirection]>=0.0f && Normal[mUpDirection]<mSlopeLimit)
			{
				mHitNonWalkable = true;
				// Early exit if we're going to run this again anyway...
				if(!mWalkExperiment)	return;
		/*		CatchScene()->GetRenderer()->AddLine(mTouched.mVerts[0], mTouched.mVerts[1], ARGB_YELLOW);
				CatchScene()->GetRenderer()->AddLine(mTouched.mVerts[0], mTouched.mVerts[2], ARGB_YELLOW);
				CatchScene()->GetRenderer()->AddLine(mTouched.mVerts[1], mTouched.mVerts[2], ARGB_YELLOW);
		*/

				// ==========[ WALK EXPERIMENT ]===========================

				mNormalizeResponse=true;

				Extended Delta = volume.mCenter[mUpDirection] > OriginalHeight ? volume.mCenter[mUpDirection] - OriginalHeight : 0.0f;
				Delta += fabsf(direction[mUpDirection]);
				Extended Recover = Delta;

				NbCollisions=0;
				const Extended MD = Recover < min_dist ? Recover/float(MaxIter) : min_dist;

				NxVec3 RecoverPoint(0,0,0);
				RecoverPoint[mUpDirection]=-float(Recover);

				if(DoSweepTest(user_data,
					user_data2,
					nb_boxes, boxes, box_user_data,
					nb_capsules, capsules, capsule_user_data,
					volume, RecoverPoint, MaxIter, &NbCollisions, groups, float(MD), groupsMask))
				{
		//			if(NbCollisions)	CollisionFlags |= COLLISION_Y_DOWN;
					// PT: why did we do this ? Removed for now. It creates a bug (non registered event) when we land on a steep poly.
					// However this might have been needed when we were sliding on those polygons, and we didn't want the land anim to
					// start while we were sliding.
		//			if(NbCollisions)	CollisionFlags &= ~NXCC_COLLISION_DOWN;
				}
				mNormalizeResponse=false;
			}
		}
	}

	// Setup new collision flags
	collision_flags = CollisionFlags;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is an interface between NX users and the internal character controller module.

#include "Controller.h"
#include "BoxController.h"
#include "CapsuleController.h"
#include "CharacterControllerManager.h"
#include "NxActor.h"

NX_INLINE NxF64 feedbackFilter(NxF64 val, NxF64& memory, NxF64 sharpness)
{
	NX_ASSERT(sharpness>=0.0 && sharpness<=1.0f && "Invalid sharpness value in feedback filter");
			if(sharpness<0.0)	sharpness = 0.0;
	else	if(sharpness>1.0)	sharpness = 1.0;
	return memory = val * sharpness + memory * (1.0 - sharpness);
}

void Controller::move(SweptVolume& volume, const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness, const NxGroupsMask* groupsMask, bool constrainedClimbingMode)
	{
	// Dynamic-load the utility library
	if(!gUtilLib)
		gUtilLib = NxGetUtilLib();
	assert(gUtilLib);
	if(!gUtilLib)	return;

	SweepTest* ST = &cctModule;

	// Init CCT with per-controller settings
	ST->debugData		= manager->debugData;
	ST->mSkinWidth		= skinWidth;
	ST->mStepOffset		= stepOffset;
	ST->mUpDirection	= upDirection;
	ST->mHandleSlope	= handleSlope;
	ST->mSlopeLimit		= slopeLimit;
	ST->mFirstUpdate	= true;

	///////////

	Controller** boxUserData = NULL;
	NxExtendedBounds3* boxes = NULL;
	NxU32 nbBoxes = 0;

	Controller** capsuleUserData = NULL;
	NxExtendedCapsule* capsules = NULL;
	NxU32 nbCapsules = 0;

	if(1)
		{
		// Experiment - to do better
		NxU32 nbControllers = manager->getNbControllers();
		Controller** controllers = manager->getControllers();

		boxes = (NxExtendedBounds3*)NxAlloca(nbControllers*sizeof(NxExtendedBounds3));
		capsules = (NxExtendedCapsule*)NxAlloca(nbControllers*sizeof(NxExtendedCapsule));	// It's evil to waste that ram
		boxUserData = (Controller**)NxAlloca(nbControllers*sizeof(Controller*));
		capsuleUserData = (Controller**)NxAlloca(nbControllers*sizeof(Controller*));

		while(nbControllers--)
			{
			Controller* currentController = *controllers++;
			if(currentController==this)	continue;

			NxActor* pActor = currentController->getActor();
			int nbShapes = pActor->getNbShapes();
			NX_ASSERT( nbShapes == 1 );
			NxShape* pCurrentShape= pActor->getShapes()[0];

			// Depending on user settings the current controller can be:
			// - discarded
			// - always kept
			// - or tested against filtering flags
			NxCCTInteractionFlag interactionFlag = currentController->getInteraction();
			bool keepController = true;
			if(interactionFlag==NXIF_INTERACTION_EXCLUDE)			keepController = false;
			else if(interactionFlag==NXIF_INTERACTION_USE_FILTER)	keepController = (activeGroups & ( 1 << pCurrentShape->getGroup()))!=0;

			if(keepController)
				{
				if(currentController->type==NX_CONTROLLER_BOX)
					{
					currentController->getWorldBox(boxes[nbBoxes]);
					boxUserData[nbBoxes++] = currentController;
					}
				else if(currentController->type==NX_CONTROLLER_CAPSULE)
					{
					CapsuleController* CC = static_cast<CapsuleController*>(currentController);
					NxExtendedVec3 p0 = CC->position;
					NxExtendedVec3 p1 = CC->position;
					p0[ST->mUpDirection] -= CC->height*0.5f;
					p1[ST->mUpDirection] += CC->height*0.5f;
					capsules[nbCapsules].p0 = p0;
					capsules[nbCapsules].p1 = p1;
					capsules[nbCapsules].radius = CC->radius;
					capsuleUserData[nbCapsules++] = currentController;
					}
				else ASSERT(0);
				}
			}
		}

	///////////

	ST->mWalkExperiment = false;

	NxExtendedVec3 Backup = volume.mCenter;
	ST->MoveCharacter(scene,
		(Controller*)this,
		volume, disp,
		nbBoxes, nbBoxes ? boxes : NULL, nbBoxes ? (const void**)boxUserData : NULL,
		nbCapsules, nbCapsules ? capsules : NULL, nbCapsules ? (const void**)capsuleUserData : NULL,
		activeGroups, minDist, collisionFlags, groupsMask, constrainedClimbingMode);

	if(ST->mHitNonWalkable)
		{
		// A bit slow, but everything else I tried was less convincing...
		ST->mWalkExperiment = true;
		volume.mCenter = Backup;
		ST->MoveCharacter(scene,
			(Controller*)this,
			volume, disp,
			nbBoxes, nbBoxes ? boxes : NULL, nbBoxes ? (const void**)boxUserData : NULL,
			nbCapsules, nbCapsules ? capsules : NULL, nbCapsules ? (const void**)capsuleUserData : NULL,
			activeGroups, minDist, collisionFlags, groupsMask, constrainedClimbingMode);
		ST->mWalkExperiment = false;
		}

if(sharpness<0.0f)
volume.mCenter = Backup;

	// Copy results back
	position = volume.mCenter;

	NxVec3 Delta = Backup - volume.mCenter;
	NxF32 deltaM2 = Delta.magnitudeSquared();
	if(deltaM2!=0.0f)
		{
		// Update kinematic actor
		if(kineActor)
			kineActor->moveGlobalPosition(NxVec3((float)position.x, (float)position.y, (float)position.z));
		}

	filteredPosition = position;

sharpness = fabsf(sharpness);

	// Apply feedback filter if needed
	if(sharpness<1.0f)
		filteredPosition[upDirection] = feedbackFilter(position[upDirection], memory, sharpness);

//	if(manager->debugData)
//		manager->debugData->addAABB(cctModule.mCachedTBV, NX_ARGB_YELLOW);
	}

void BoxController::move(const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness, const NxGroupsMask* groupsMask)
	{
	// Create internal swept box
	SweptBox sweptBox;
	sweptBox.mCenter		= position;
	sweptBox.mExtents		= extents;
	sweptBox.mHalfHeight	= extents[upDirection];	// UBI
	Controller::move(sweptBox, disp, activeGroups, minDist, collisionFlags, sharpness, groupsMask, false);
	}

void CapsuleController::move(const NxVec3& disp, NxU32 activeGroups, NxF32 minDist, NxU32& collisionFlags, NxF32 sharpness, const NxGroupsMask* groupsMask)
	{
	// Create internal swept capsule
	SweptCapsule sweptCapsule;
	sweptCapsule.mCenter		= position;
	sweptCapsule.mRadius		= radius;
	sweptCapsule.mHeight		= height;
	sweptCapsule.mHalfHeight	= height/2.0f + radius;	// UBI
	Controller::move(sweptCapsule, disp, activeGroups, minDist, collisionFlags, sharpness, groupsMask, climbingMode==CLIMB_CONSTRAINED);
	}

#if defined(_WIN32) && !defined(_XBOX)
#include <windows.h>
#endif
#include <stdio.h>
void CharacterControllerManager::printStats()
{
    static volatile bool bPrintThis = false;
    if ( bPrintThis )
    {
        char buffer[256];
        sprintf(buffer, "%d - %d - %d\n", gNbIters, gNbFullUpdates, gNbPartialUpdates);
//      OutputDebugString(buffer);
        printf(buffer);
    }
    gNbIters = 0;
    gNbFullUpdates = 0;
    gNbPartialUpdates = 0;
}
