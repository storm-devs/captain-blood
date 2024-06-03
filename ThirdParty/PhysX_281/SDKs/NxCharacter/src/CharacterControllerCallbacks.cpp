/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxTriangle.h"

#include "NxScene.h"
#include "NxBox.h"
#include "NxCapsule.h"
#include "NxActor.h"
#include "NxShape.h"
#include "NxBoxShape.h"
#include "NxSphereShape.h"
#include "NxCapsuleShape.h"
#include "NxConvexShape.h"
#include "NxConvexMesh.h"
#include "NxTriangleMeshShape.h"
#include "NxTriangleMesh.h"
#include "NxTriangleMeshDesc.h"
#include "NxHeightFieldShape.h"
#include "NxHeightField.h"
#include "CharacterController.h"
#include "Controller.h"

//#define VISUALIZE_CCT_TRIS

#ifdef VISUALIZE_CCT_TRIS
#include "NxPhysicsSDK.h"
#include "NxDebugRenderable.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static void outputSphereToStream(NxSphereShape* sphereShape, NxShape* shape, IntArray& geom_stream, const NxExtendedVec3& origin)
	{
	NxExtendedSphere WorldSphere;
		{
		NxSphere s_tmp;
		sphereShape->getWorldSphere(s_tmp);	// LOSS OF ACCURACY
		WorldSphere.radius = s_tmp.radius;
		WorldSphere.center.x = s_tmp.center.x;
		WorldSphere.center.y = s_tmp.center.y;
		WorldSphere.center.z = s_tmp.center.z;
		}

	TouchedSphere* touchedSphere	= (TouchedSphere*)reserve(geom_stream, sizeof(TouchedSphere)/sizeof(NxU32));
	touchedSphere->mType			= TOUCHED_SPHERE;
	touchedSphere->mUserData		= shape;
	touchedSphere->mOffset			= origin;
	touchedSphere->mSphere.radius	= WorldSphere.radius;
	touchedSphere->mSphere.center.x	= float(WorldSphere.center.x - origin.x);
	touchedSphere->mSphere.center.y	= float(WorldSphere.center.y - origin.y);
	touchedSphere->mSphere.center.z	= float(WorldSphere.center.z - origin.z);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void outputCapsuleToStream(NxCapsuleShape* capsuleShape, NxShape* shape, IntArray& geom_stream, const NxExtendedVec3& origin)
	{
	NxExtendedCapsule WorldCapsule;
		{
		NxCapsule c_tmp;
		capsuleShape->getWorldCapsule(c_tmp);	// LOSS OF ACCURACY
		WorldCapsule.radius	= c_tmp.radius;
		WorldCapsule.p0.x	= c_tmp.p0.x;
		WorldCapsule.p0.y	= c_tmp.p0.y;
		WorldCapsule.p0.z	= c_tmp.p0.z;
		WorldCapsule.p1.x	= c_tmp.p1.x;
		WorldCapsule.p1.y	= c_tmp.p1.y;
		WorldCapsule.p1.z	= c_tmp.p1.z;
		}

	TouchedCapsule* touchedCapsule	= (TouchedCapsule*)reserve(geom_stream, sizeof(TouchedCapsule)/sizeof(NxU32));
	touchedCapsule->mType			= TOUCHED_CAPSULE;
	touchedCapsule->mUserData		= shape;
	touchedCapsule->mOffset			= origin;
	touchedCapsule->mCapsule.radius	= WorldCapsule.radius;
	touchedCapsule->mCapsule.p0.x	= float(WorldCapsule.p0.x - origin.x);
	touchedCapsule->mCapsule.p0.y	= float(WorldCapsule.p0.y - origin.y);
	touchedCapsule->mCapsule.p0.z	= float(WorldCapsule.p0.z - origin.z);
	touchedCapsule->mCapsule.p1.x	= float(WorldCapsule.p1.x - origin.x);
	touchedCapsule->mCapsule.p1.y	= float(WorldCapsule.p1.y - origin.y);
	touchedCapsule->mCapsule.p1.z	= float(WorldCapsule.p1.z - origin.z);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void outputBoxToStream(NxBoxShape* boxShape, NxShape* shape, IntArray& geom_stream, const NxExtendedVec3& origin)
	{
	NxExtendedBox WorldBox;
		{
		NxBox b_tmp;
		boxShape->getWorldOBB(b_tmp);	// LOSS OF ACCURACY
		WorldBox.rot		= b_tmp.rot;
		WorldBox.extents	= b_tmp.extents;
		WorldBox.center.x	= b_tmp.center.x;
		WorldBox.center.y	= b_tmp.center.y;
		WorldBox.center.z	= b_tmp.center.z;
		}

	TouchedBox* touchedBox		= (TouchedBox*)reserve(geom_stream, sizeof(TouchedBox)/sizeof(NxU32));
	touchedBox->mType			= TOUCHED_BOX;
	touchedBox->mUserData		= shape;
	touchedBox->mOffset			= origin;

	touchedBox->mBox.extents	= WorldBox.extents;
	touchedBox->mBox.rot		= WorldBox.rot;
	touchedBox->mBox.center.x	= float(WorldBox.center.x - origin.x);
	touchedBox->mBox.center.y	= float(WorldBox.center.y - origin.y);
	touchedBox->mBox.center.z	= float(WorldBox.center.z - origin.z);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void outputMeshToStream(
	NxTriangleMeshShape* meshShape,
	NxShape* shape,
	IntArray& geom_stream,
	TriArray& world_triangles,
	TriArray* world_edge_normals,
	IntArray& edge_flags,
	const NxExtendedVec3& origin,
	const NxBounds3& tmpBounds
	)
	{
	// Do AABB-mesh query

	NxU32 Nb;
	const NxU32* TF;

	// Collide AABB against current mesh
	if(!meshShape->overlapAABBTriangles(tmpBounds, NX_QUERY_WORLD_SPACE, Nb, TF))
		return;

	NxTriangleMeshDesc meshDesc;
	meshShape->getTriangleMesh().saveToDesc(meshDesc);
	bool ReverseWinding = (meshDesc.heightFieldVerticalAxis != NX_NOT_HEIGHTFIELD && meshDesc.heightFieldVerticalExtent > 0);

	NxVec3 tmp = shape->getGlobalPosition();	// LOSS OF ACCURACY
	NxVec3 MeshOffset;
	MeshOffset.x = float(tmp.x - origin.x);
	MeshOffset.y = float(tmp.y - origin.y);
	MeshOffset.z = float(tmp.z - origin.z);

	TouchedMesh* touchedMesh			= (TouchedMesh*)reserve(geom_stream, sizeof(TouchedMesh)/sizeof(NxU32));
	touchedMesh->mType					= TOUCHED_MESH;
	touchedMesh->mUserData				= shape;
	touchedMesh->mOffset				= origin;
	touchedMesh->mNbTris				= Nb;
	touchedMesh->mIndexWorldTriangles	= world_triangles.size();
	touchedMesh->mIndexWorldEdgeNormals	= world_edge_normals ? world_edge_normals->size() : 0;
	touchedMesh->mIndexEdgeFlags		= edge_flags.size();

	// Reserve memory for incoming triangles
	NxTriangle* TouchedTriangles = reserve(world_triangles, Nb);
	NxTriangle* EdgeTriangles = world_edge_normals ? reserve(*world_edge_normals, Nb) : NULL;

	// Loop through touched triangles
	while(Nb--)
		{
		NxU32 Index = *TF++;
		// Compute triangle in world space, add to array
		NxTriangle& CurrentTriangle = *TouchedTriangles++;

		NxTriangle edgeTri;
		NxU32 edgeFlags;
		meshShape->getTriangle(CurrentTriangle, &edgeTri, &edgeFlags, Index, false);
		CurrentTriangle.verts[0] += MeshOffset;
		CurrentTriangle.verts[1] += MeshOffset;
		CurrentTriangle.verts[2] += MeshOffset;

		if(EdgeTriangles)
			{
			*EdgeTriangles++ = edgeTri;
			}

		edge_flags.pushBack(edgeFlags);

		if (ReverseWinding)
			{
			NxVec3 tmp = CurrentTriangle.verts[1];
			CurrentTriangle.verts[1] = CurrentTriangle.verts[2];
			CurrentTriangle.verts[2] = tmp;
			}

#ifdef VISUALIZE_CCT_TRIS
		// Visualize debug triangles
		{
//		PhysicsSDK::getInstance().getDebugRenderable()->addTriangle((const NxVec3&)CurrentTriangle.mVerts[0], (const NxVec3&)CurrentTriangle.mVerts[1], (const NxVec3&)CurrentTriangle.mVerts[2], NX_ARGB_GREEN);
		NxGetPhysicsSDK()->getDebugRenderable()->addTriangle((const NxVec3&)CurrentTriangle.mVerts[0], (const NxVec3&)CurrentTriangle.mVerts[1], (const NxVec3&)CurrentTriangle.mVerts[2], NX_ARGB_GREEN);

//		PhysicsSDK::getInstance().getDebugRenderable()->addLine((const NxVec3&)CurrentTriangle.mVerts[0], (const NxVec3&)CurrentTriangle.mVerts[1], NX_ARGB_GREEN);
//		PhysicsSDK::getInstance().getDebugRenderable()->addLine((const NxVec3&)CurrentTriangle.mVerts[1], (const NxVec3&)CurrentTriangle.mVerts[2], NX_ARGB_GREEN);
//		PhysicsSDK::getInstance().getDebugRenderable()->addLine((const NxVec3&)CurrentTriangle.mVerts[2], (const NxVec3&)CurrentTriangle.mVerts[0], NX_ARGB_GREEN);
		}
#endif
	}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void outputHeightFieldToStream(
	NxHeightFieldShape* hfShape,
	NxShape* shape,
	IntArray& geom_stream,
	TriArray& world_triangles,
	TriArray* world_edge_normals,
	IntArray& edge_flags,
	const NxExtendedVec3& origin,
	const NxBounds3& tmpBounds
	)
	{
	// Do AABB-mesh query

	NxU32 Nb;
	const NxU32* TF;

	// Collide AABB against current mesh
	if(!hfShape->overlapAABBTriangles(tmpBounds, NX_QUERY_WORLD_SPACE, Nb, TF))
		return;

	NxVec3 tmp = shape->getGlobalPosition();	// LOSS OF ACCURACY
	NxVec3 MeshOffset;
	MeshOffset.x = float(tmp.x - origin.x);
	MeshOffset.y = float(tmp.y - origin.y);
	MeshOffset.z = float(tmp.z - origin.z);

	TouchedMesh* touchedMesh			= (TouchedMesh*)reserve(geom_stream, sizeof(TouchedMesh)/sizeof(NxU32));
	touchedMesh->mType					= TOUCHED_MESH; // ptchernev: seems to work
	touchedMesh->mUserData				= shape;
	touchedMesh->mOffset				= origin;
	touchedMesh->mNbTris				= Nb;
	touchedMesh->mIndexWorldTriangles	= world_triangles.size();
	touchedMesh->mIndexWorldEdgeNormals	= world_edge_normals ? world_edge_normals->size() : 0;
	touchedMesh->mIndexEdgeFlags		= edge_flags.size();

	// Reserve memory for incoming triangles
	NxTriangle* TouchedTriangles = reserve(world_triangles, Nb);
	NxTriangle* EdgeTriangles = world_edge_normals ? reserve(*world_edge_normals, Nb) : NULL;

	// Loop through touched triangles
	while(Nb--)
		{
		NxU32 Index = *TF++;
		// Compute triangle in world space, add to array
		NxTriangle& CurrentTriangle = *TouchedTriangles++;

		NxTriangle edgeTri;
		NxU32 edgeFlags;
		hfShape->getTriangle(CurrentTriangle, &edgeTri, &edgeFlags, Index, false);
		CurrentTriangle.verts[0] += MeshOffset;
		CurrentTriangle.verts[1] += MeshOffset;
		CurrentTriangle.verts[2] += MeshOffset;

		if(EdgeTriangles)
			{
			*EdgeTriangles++ = edgeTri;
			}

		edge_flags.pushBack(edgeFlags);

#ifdef VISUALIZE_CCT_TRIS
		// Visualize debug triangles
			{
			//		PhysicsSDK::getInstance().getDebugRenderable()->addTriangle((const NxVec3&)CurrentTriangle.mVerts[0], (const NxVec3&)CurrentTriangle.mVerts[1], (const NxVec3&)CurrentTriangle.mVerts[2], NX_ARGB_GREEN);
			NxGetPhysicsSDK()->getDebugRenderable()->addTriangle((const NxVec3&)CurrentTriangle.mVerts[0], (const NxVec3&)CurrentTriangle.mVerts[1], (const NxVec3&)CurrentTriangle.mVerts[2], NX_ARGB_GREEN);

			//		PhysicsSDK::getInstance().getDebugRenderable()->addLine((const NxVec3&)CurrentTriangle.mVerts[0], (const NxVec3&)CurrentTriangle.mVerts[1], NX_ARGB_GREEN);
			//		PhysicsSDK::getInstance().getDebugRenderable()->addLine((const NxVec3&)CurrentTriangle.mVerts[1], (const NxVec3&)CurrentTriangle.mVerts[2], NX_ARGB_GREEN);
			//		PhysicsSDK::getInstance().getDebugRenderable()->addLine((const NxVec3&)CurrentTriangle.mVerts[2], (const NxVec3&)CurrentTriangle.mVerts[0], NX_ARGB_GREEN);
			}
#endif
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void outputConvexToStream(
	NxConvexShape* convexShape,
	NxShape* shape,
	IntArray& geom_stream,
	TriArray& world_triangles,
	TriArray* world_edge_normals,
	IntArray& edge_flags,
	const NxExtendedVec3& origin,
	const NxBounds3& tmpBounds
	)
	{
	// Do AABB-mesh query

	NxU32 Nb;
	const NxU32* TF;

	// Collide AABB against current mesh
	// The overlap function doesn't exist for convexes so let's just dump all tris
	NxConvexMesh& cm = convexShape->getConvexMesh();
	Nb = cm.getCount(0, NX_ARRAY_TRIANGLES);
	NX_ASSERT(cm.getFormat(0, NX_ARRAY_TRIANGLES)==NX_FORMAT_INT);
	TF = (const NxU32*)cm.getBase(0, NX_ARRAY_TRIANGLES);
	NX_ASSERT(cm.getFormat(0, NX_ARRAY_VERTICES)==NX_FORMAT_FLOAT);
	const NxVec3* verts = (const NxVec3*)cm.getBase(0, NX_ARRAY_VERTICES);

	NxMat34 absPose = shape->getGlobalPose();

	NxVec3 tmp = shape->getGlobalPosition();	// LOSS OF ACCURACY
	NxVec3 MeshOffset;
	MeshOffset.x = float(tmp.x - origin.x);
	MeshOffset.y = float(tmp.y - origin.y);
	MeshOffset.z = float(tmp.z - origin.z);

	TouchedMesh* touchedMesh			= (TouchedMesh*)reserve(geom_stream, sizeof(TouchedMesh)/sizeof(NxU32));
	touchedMesh->mType					= TOUCHED_MESH;
	touchedMesh->mUserData				= shape;
	touchedMesh->mOffset				= origin;
	touchedMesh->mNbTris				= Nb;
	touchedMesh->mIndexWorldTriangles	= world_triangles.size();
	touchedMesh->mIndexWorldEdgeNormals	= world_edge_normals ? world_edge_normals->size() : 0;
	touchedMesh->mIndexEdgeFlags		= edge_flags.size();

	// Reserve memory for incoming triangles
	NxTriangle* TouchedTriangles = reserve(world_triangles, Nb);
	NxTriangle* EdgeTriangles = world_edge_normals ? reserve(*world_edge_normals, Nb) : NULL;

	// Loop through touched triangles
	while(Nb--)
		{
		// Compute triangle in world space, add to array
		NxTriangle& CurrentTriangle = *TouchedTriangles++;

		NxU32 vref0 = *TF++;
		NxU32 vref1 = *TF++;
		NxU32 vref2 = *TF++;

		NxVec3 v0 = verts[vref0];
		NxVec3 v1 = verts[vref1];
		NxVec3 v2 = verts[vref2];
		absPose.M.multiply(v0, v0);
		absPose.M.multiply(v1, v1);
		absPose.M.multiply(v2, v2);

		CurrentTriangle.verts[0] = v0;
		CurrentTriangle.verts[1] = v1;
		CurrentTriangle.verts[2] = v2;

		CurrentTriangle.verts[0] += MeshOffset;
		CurrentTriangle.verts[1] += MeshOffset;
		CurrentTriangle.verts[2] += MeshOffset;

		if(EdgeTriangles)
			{
			// #### hmmm
			NxTriangle edgeTri;
			edgeTri.verts[0].zero();
			edgeTri.verts[1].zero();
			edgeTri.verts[2].zero();

			*EdgeTriangles++ = edgeTri;
			}

		// #### hmmm
		NxU32 edgeFlags = 7;
		edge_flags.pushBack(edgeFlags);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FindTouchedGeometry(
	void* user_data,
	const NxExtendedBounds3& worldBounds,		// ### we should also accept other volumes

	TriArray& world_triangles,
	TriArray* world_edge_normals,
	IntArray& edge_flags,
	IntArray& geom_stream,

	NxU32 group_flags,
	bool static_shapes, bool dynamic_shapes, const NxGroupsMask* groupsMask)
	{
	NX_ASSERT(user_data);
	NxScene* scene = (NxScene*)user_data;

	NxExtendedVec3 Origin;	// Will be TouchedGeom::mOffset
	worldBounds.getCenter(Origin);

	// Reserve a stack buffer big enough to hold all shapes in the world. This is a lazy approach that is
	// acceptable here since the total number of shapes is limited to 64K anyway, which would "only" consume
	// 256 Kb on the stack (hence, stack overflow is unlikely).
	// ### TODO: the new callback mechanism would allow us to use less memory here
//	NxU32 total = scene->getNbStaticShapes() + scene->getNbDynamicShapes();
	NxU32 total = scene->getTotalNbShapes();
	NxShape** buffer = (NxShape**)NxAlloca(total*sizeof(NxShape*));

	// Find touched *boxes* i.e. touched objects' AABBs in the world
	// We collide against dynamic shapes too, to get back dynamic boxes/etc
	// TODO: add active groups in interface!

	NxU32 Flags = 0;
	if(static_shapes)	Flags |= NX_STATIC_SHAPES;
	if(dynamic_shapes)	Flags |= NX_DYNAMIC_SHAPES;

	// ### this one is dangerous
	NxBounds3 tmpBounds;	// LOSS OF ACCURACY
	tmpBounds.min.x = (float)worldBounds.min.x;
	tmpBounds.min.y = (float)worldBounds.min.y;
	tmpBounds.min.z = (float)worldBounds.min.z;
	tmpBounds.max.x = (float)worldBounds.max.x;
	tmpBounds.max.y = (float)worldBounds.max.y;
	tmpBounds.max.z = (float)worldBounds.max.z;
	NxU32 nbTouchedBoxes = scene->overlapAABBShapes(tmpBounds, NxShapesType(Flags), total, buffer, NULL, group_flags, groupsMask);

	// Early exit if no AABBs found
	if(!nbTouchedBoxes)	return false;
	NX_ASSERT(nbTouchedBoxes<=total);	// Else we just trashed some stack memory

	// Loop through touched world AABBs
	NxShape** touched = buffer;
	while(nbTouchedBoxes--)
		{
		// Get current shape
		NxShape* shape = *touched++;

		// Filtering

		// Discard all CCT shapes, i.e. kinematic actors we created ourselves. We don't need to collide with them since they're surrounded
		// by the real CCT volume - and collisions with those are handled elsewhere. We use the userData field for filtering because that's
		// really our only valid option (filtering groups are already used by clients and we don't have control over them, clients might
		// create other kinematic actors that we may want to keep here, etc, etc)
		if(size_t(shape->userData)=='CCTS')
			continue;

		// Discard if not collidable
		// PT: this shouldn't be possible at this point since:
		// - the SF flag is only used for compounds
		// - the AF flag is already tested in scene query
		// - we shouldn't get compound shapes here
		if(shape->getFlag(NX_SF_DISABLE_COLLISION))
			continue;

		// Ubi (EA) : Discarding Triggers :
		if ( shape->getFlag(NX_TRIGGER_ENABLE) )
			continue;

		// PT: here you might want to disable kinematic objects.

		// Output shape to stream
		NxShapeType type = shape->getType();
				if(type==NX_SHAPE_SPHERE)		outputSphereToStream((NxSphereShape*)shape, shape, geom_stream, Origin);
		else	if(type==NX_SHAPE_CAPSULE)		outputCapsuleToStream((NxCapsuleShape*)shape, shape, geom_stream, Origin);
		else	if(type==NX_SHAPE_BOX)			outputBoxToStream((NxBoxShape*)shape, shape, geom_stream, Origin);
		else	if(type==NX_SHAPE_MESH)			outputMeshToStream((NxTriangleMeshShape*)shape, shape, geom_stream, world_triangles, world_edge_normals, edge_flags, Origin, tmpBounds);
		else	if(type==NX_SHAPE_HEIGHTFIELD)	outputHeightFieldToStream((NxHeightFieldShape*)shape, shape, geom_stream, world_triangles, world_edge_normals, edge_flags, Origin, tmpBounds);
		else	if(type==NX_SHAPE_CONVEX)		outputConvexToStream((NxConvexShape*)shape, shape, geom_stream, world_triangles, world_edge_normals, edge_flags, Origin, tmpBounds);
		}

	return true;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #### hmmm, in the down case, isn't reported length too big ? It contains our artificial up component,
// that might confuse the user

void ShapeHitCallback(void* user_data, const SweptContact& contact, const NxVec3& dir, float length)
{
	Controller* controller = (Controller*)user_data;

	if(controller->callback)
	{
		NxControllerShapeHit hit;
		hit.shape			= (NxShape*)contact.mGeom->mUserData;
		hit.worldPos.x		= contact.mWorldPos.x;
		hit.worldPos.y		= contact.mWorldPos.y;
		hit.worldPos.z		= contact.mWorldPos.z;
		hit.worldNormal.x	= contact.mWorldNormal.x;
		hit.worldNormal.y	= contact.mWorldNormal.y;
		hit.worldNormal.z	= contact.mWorldNormal.z;
		hit.dir.x			= dir.x;
		hit.dir.y			= dir.y;
		hit.dir.z			= dir.z;
		hit.length			= length;
		hit.controller		= controller->getNxController();

		// PB: the index is only indexing the limited set of returned triangles, which is useless for the user!
//		hit.faceID = tri_index;	// Remap this!

		NxControllerAction action = controller->callback->onShapeHit(hit);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserHitCallback(void* user_data, const SweptContact& contact, const NxVec3& dir, float length)
{
	Controller* controller = (Controller*)user_data;

	if(controller->callback)
	{
		Controller* other = (Controller*)contact.mGeom->mUserData;

		NxControllersHit hit;
		hit.controller	= controller->getNxController();
		hit.other		= other->getNxController();

		NxControllerAction action = NX_ACTION_NONE;
		action = controller->callback->onControllerHit(hit);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
