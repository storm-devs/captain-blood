#ifndef NX_CHARACTER_CONTROLLER
#define NX_CHARACTER_CONTROLLER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

//#define USE_CONTACT_NORMAL_FOR_SLOPE_TEST

#include "NxBox.h"
#include "NxSphere.h"
#include "NxCapsule.h"
#include "NxTriangle.h"
#include "NxExtended.h"
#include "NxArray.h"
#include "CCTAllocator.h"

	template<class T>
	NX_INLINE T* reserve(NxArray<T, CCTAllocator>& array, NxU32 nb)
	{
		NxU32 currentSize = array.size();
		array.insert(array.begin() + currentSize, nb, T());
		return array.begin() + currentSize;
	}

// Sigh. The function above doesn't work with typedefs apparently
//typedef NxArray<NxTriangle, CCTAllocator>	TriArray;
//typedef NxArray<NxTriangle, NxU32>		IntArray;

#define TriArray	NxArray<NxTriangle, CCTAllocator>
#define IntArray	NxArray<NxU32, CCTAllocator>

/* Exclude from documentation */
/** \cond */

	enum TouchedGeomType
	{
		TOUCHED_USER_BOX,
		TOUCHED_USER_CAPSULE,
		TOUCHED_MESH,
		TOUCHED_BOX,
		TOUCHED_SPHERE,
		TOUCHED_CAPSULE,

		TOUCHED_LAST,

		TOUCHED_FORCE_DWORD	= 0x7fffffff
	};

	class SweptVolume;

// PT: apparently stupid .Net aligns some of them on 8-bytes boundaries for no good reason. This is bad.
#pragma pack(4)

	struct TouchedGeom
	{
		TouchedGeomType		mType;
		const void*			mUserData;	// NxController or NxShape NxVec3er
		NxExtendedVec3		mOffset;	// Local origin, typically the center of the world bounds around the character. We translate both
										// touched shapes & the character so that they are nearby this NxVec3, then add the offset back to
										// computed "world" impacts.
	};

	struct TouchedUserBox : public TouchedGeom
	{
		NxExtendedBounds3		mBox;

		NX_INLINE	void	Relocate(NxBox& box)	const
		{
			box.center.x = float(mBox.getCenter(0) - mOffset.x);
			box.center.y = float(mBox.getCenter(1) - mOffset.y);
			box.center.z = float(mBox.getCenter(2) - mOffset.z);

			box.extents.x = (float)mBox.getExtents(0);
			box.extents.y = (float)mBox.getExtents(1);
			box.extents.z = (float)mBox.getExtents(2);

			box.rot.id();
		}
	};
	NX_COMPILE_TIME_ASSERT(sizeof(TouchedUserBox)==sizeof(TouchedGeom)+sizeof(NxExtendedBounds3));

	struct TouchedUserCapsule : public TouchedGeom
	{
		NxExtendedCapsule		mCapsule;

		NX_INLINE	void	Relocate(NxCapsule& capsule)	const
		{
			capsule.radius = mCapsule.radius;
			capsule.p0.x = float(mCapsule.p0.x - mOffset.x);
			capsule.p0.y = float(mCapsule.p0.y - mOffset.y);
			capsule.p0.z = float(mCapsule.p0.z - mOffset.z);
			capsule.p1.x = float(mCapsule.p1.x - mOffset.x);
			capsule.p1.y = float(mCapsule.p1.y - mOffset.y);
			capsule.p1.z = float(mCapsule.p1.z - mOffset.z);
		}
	};
	NX_COMPILE_TIME_ASSERT(sizeof(TouchedUserCapsule)==sizeof(TouchedGeom)+sizeof(NxExtendedCapsule));

	struct TouchedMesh : public TouchedGeom
	{
		NxU32			mNbTris;
		NxU32			mIndexWorldTriangles;
		NxU32			mIndexWorldEdgeNormals;
		NxU32			mIndexEdgeFlags;
	};

	struct TouchedBox : public TouchedGeom
	{
		NxBox			mBox;
	};
	NX_COMPILE_TIME_ASSERT(sizeof(TouchedBox)==sizeof(TouchedGeom)+sizeof(NxBox));

	struct TouchedSphere : public TouchedGeom
	{
		NxSphere		mSphere;
	};
	NX_COMPILE_TIME_ASSERT(sizeof(TouchedSphere)==sizeof(TouchedGeom)+sizeof(NxSphere));

	struct TouchedCapsule : public TouchedGeom
	{
		NxCapsule		mCapsule;
	};
	NX_COMPILE_TIME_ASSERT(sizeof(TouchedCapsule)==sizeof(TouchedGeom)+sizeof(NxCapsule));

#pragma pack()

	enum SweptContactType
	{
		SWEPT_CTC_SHAPE,		// We touched another shape
		SWEPT_CTC_CONTROLLER,	// We touched another controller
	};

	struct SweptContact
	{
		NxExtendedVec3		mWorldPos;		// Contact position in world space
		NxVec3				mWorldNormal;	// Contact normal in world space
		NxF32				mDistance;		// Contact distance
		NxU32				mIndex;			// Feature identifier (e.g. triangle index for meshes)
		TouchedGeom*		mGeom;
	};

	class NxGroupsMask;
	class CCTDebugData;

	class SweepTest
	{
		public:
									SweepTest();
									~SweepTest();

				void				MoveCharacter(
										void* user_data,
										void* user_data2,
										SweptVolume& volume,
										const NxVec3& direction,
										NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
										NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
										NxU32 groups, NxF32 min_dist,
										NxU32& collision_flags,
										const NxGroupsMask* groupsMask,
										bool constrainedClimbingMode
										);

				bool				DoSweepTest(
										void* user_data,
										void* user_data2,
										NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
										NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
										SweptVolume& swept_volume,
										const NxVec3& direction, NxU32 max_iter,
										NxU32* nb_collisions, NxU32 group_flags, NxF32 min_dist, const NxGroupsMask* groupsMask, bool down_pass=false);

				void				FindTouchedCCTs(
										NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
										NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
										const NxExtendedBounds3& world_box
									);

				void				VoidTestCache()	{ mCachedTBV.setEmpty();	}

//		private:
				CCTDebugData*		debugData;
				TriArray			mWorldTriangles;
				TriArray			mWorldEdgeNormals;
				IntArray			mEdgeFlags;
				IntArray			mGeomStream;
				NxExtendedBounds3	mCachedTBV;
				NxU32				mCachedTriIndexIndex;
		mutable	NxU32				mCachedTriIndex[3];
				NxU32				mNbCachedStatic;
				NxU32				mNbCachedT;
				NxU32				mNbCachedEN;
				NxU32				mNbCachedF;
		public:
#ifdef USE_CONTACT_NORMAL_FOR_SLOPE_TEST
				NxVec3				mCN;
#else
				NxTriangle			mTouched;
#endif
				NxF32				mSlopeLimit;
				NxF32				mSkinWidth;
				NxF32				mStepOffset;
				NxF32				mVolumeGrowth;	// Must be >1.0f and not too big
				NxF32				mContactPointHeight;	// UBI
				NxU32				mUpDirection;
				NxU32				mMaxIter;
				bool				mHitNonWalkable;
				bool				mWalkExperiment;
				bool				mHandleSlope;
				bool				mValidTri;
				bool				mValidateCallback;
				bool				mNormalizeResponse;
				bool				mFirstUpdate;
		private:
				void				UpdateTouchedGeoms(void* user_data, const SweptVolume& swept_volume,
												NxU32 nb_boxes, const NxExtendedBounds3* boxes, const void** box_user_data,
												NxU32 nb_capsules, const NxExtendedCapsule* capsules, const void** capsule_user_data,
												NxU32 group_flags, const NxExtendedBounds3& world_box, const NxGroupsMask* groupsMask);
	};

	bool FindTouchedGeometry(void* user_data,
							const NxExtendedBounds3& world_aabb,

							TriArray& world_triangles,
							TriArray* world_edge_normals,
							IntArray& edge_flags,
							IntArray& geom_stream,

							NxU32 group_flags,
							bool static_shapes, bool dynamic_shapes, const NxGroupsMask* groupsMask);

	void ShapeHitCallback(void* user_data2, const SweptContact& contact, const NxVec3& dir, NxF32 length);
	void UserHitCallback(void* user_data2, const SweptContact& contact, const NxVec3& dir, NxF32 length);

/** \endcond */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
