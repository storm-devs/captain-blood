#pragma once

#include "PxRigidActor.h"
#include "PxJoint.h"
#include "PxFiltering.h"
#include "PxShape.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "PxShapeExt.h"
#include "PxRigidBodyExt.h"
#include "foundation/PxSimpleTypes.h"

#include "WrapperLock.h"

/** Corresponds to 20 frames for a time step of 0.02 */
#define DEFAULT_WAKE_UP_COUNTER 0.4f

namespace physx
{
//--------------------------------------------------------------
//
// Helper routines
//
//--------------------------------------------------------------

inline PxReal degToRad(PxReal d)
{
	return d * PxPi / 180.0f;
}

inline PxReal operator|(const PxVec3& left, const PxVec3& right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

// Special version allowing 2D quads
inline PxReal volume(const PxVec3& extents)
{
	PxReal v = 1.0f;
	if (extents.x != 0.0f)	v *= extents.x;
	if (extents.y != 0.0f)	v *= extents.y;
	if (extents.z != 0.0f)	v *= extents.z;
	return v;
}

// Sphere
inline PxReal computeSphereRatio(PxReal radius) { return (4.0f / 3.0f) * PxPi * radius * radius * radius; }
inline PxReal computeSphereMass(PxReal radius, PxReal density) { return density * computeSphereRatio(radius); }
inline PxReal computeSphereDensity(PxReal radius, PxReal mass) { return mass / computeSphereRatio(radius); }

// Box
inline PxReal computeBoxRatio(const PxVec3& extents) { return volume(extents); }
inline PxReal computeBoxMass(const PxVec3& extents, PxReal density) { return density * computeBoxRatio(extents); }
inline PxReal computeBoxDensity(const PxVec3& extents, PxReal mass) { return mass / computeBoxRatio(extents); }

// Ellipsoid
inline PxReal computeEllipsoidRatio(const PxVec3& extents) { return (4.0f / 3.0f) * PxPi * volume(extents); }
inline PxReal computeEllipsoidMass(const PxVec3& extents, PxReal density) { return density * computeEllipsoidRatio(extents); }
inline PxReal computeEllipsoidDensity(const PxVec3& extents, PxReal mass) { return mass / computeEllipsoidRatio(extents); }

// Cylinder
inline PxReal computeCylinderRatio(PxReal r, PxReal l) { return PxPi * r * r * (2.0f * l); }
inline PxReal computeCylinderMass(PxReal r, PxReal l, PxReal density) { return density * computeCylinderRatio(r, l); }
inline PxReal computeCylinderDensity(PxReal r, PxReal l, PxReal mass) { return mass / computeCylinderRatio(r, l); }

inline PxReal distanceSquared(const PxVec3& v1, const PxVec3& v2)
	{
	PxReal dx = v1.x - v2.x;
	PxReal dy = v1.y - v2.y;
	PxReal dz = v1.z - v2.z;
	return dx * dx + dy * dy + dz * dz;
	}


enum NxBodyFlag
{
	NX_BF_KINEMATIC = (1 << 7), //!< Enable kinematic mode for the body.
};


class NxActor : public PxActor
{
public:

	PxU32 getNbShapes() const
	{
		const PxRigidActor*	rigidActor	= isRigidActor();
		if (!rigidActor)
			return 0;
		PHYSX3_LOCK_READ(rigidActor->getScene());
		return rigidActor->getNbShapes();
	}

	PxShape** getShapes() const
	{
		const PxRigidActor* rigidActor = isRigidActor();
		if (!rigidActor)
			return nullptr;
		PHYSX3_LOCK_READ(rigidActor->getScene());
		PxU32 nbShapes = rigidActor->getNbShapes();
		
		// FIX_PX3 add SAMPLE_ALLOC or similar
		PxShape** shapes;
		//PxShape** shapes = (PxShape**)SAMPLE_ALLOC(sizeof(PxShape*) * nbShapes);
		//rigidActor->getShapes(shapes, nbShapes);
		
		return shapes;
	}

	bool updateMassFromShapes(PxReal density, PxReal totalMass, bool includeNonSimShapes = false)
	{
		PxRigidBody* rigidBody = isRigidBody();

		PX_ASSERT(density == 0.0f);
		if (rigidBody && density == 0.0f)
		{
			PHYSX3_LOCK_WRITE(rigidBody->getScene());
			// don't call setMass, we need update inertia and cmass at same time.
			PxRigidBodyExt::setMassAndUpdateInertia(*rigidBody, totalMass, NULL, includeNonSimShapes);

			return true;
		}
		return false;
	}

	bool readBodyFlag(NxBodyFlag bodyFlag)  const
	{
		bool ret = false;
		if (bodyFlag != NX_BF_KINEMATIC)
		{
			PX_ALWAYS_ASSERT();
			return ret;
		}
		const PxRigidDynamic* rigidDynamic = isRigidDynamic();
		PxScene *scene = rigidDynamic ? rigidDynamic->getScene() : NULL;
		PHYSX3_LOCK_READ(scene);
		if (rigidDynamic && (rigidDynamic->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC))
		{
			ret = true;
		}
		return ret;
	}

	bool raiseBodyFlag(NxBodyFlag bodyFlag)
	{
		if (bodyFlag != NX_BF_KINEMATIC)
		{
			PX_ALWAYS_ASSERT();
			return false;
		}

		PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC, true);
			return true;
		}

		return false;
	}

	bool clearBodyFlag(NxBodyFlag bodyFlag)
	{
		if (bodyFlag != NX_BF_KINEMATIC)
		{
			PX_ALWAYS_ASSERT();
			return false;
		}

		PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC, false);
			return true;
		}

		return false;
	}

	bool isDynamic() const
	{
		return !!isRigidDynamic();
	}

	bool isSleeping() const
	{
		const PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_READ(rigidDynamic->getScene());
			return rigidDynamic->isSleeping();
		}
		return false;
	}

	void setAngularDamping(PxReal angDamp)
	{
		PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setAngularDamping(angDamp);
		}
	}

	void setContactReportThreshold(PxReal threshold)
	{
		PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setContactReportThreshold(threshold);
		}
	}

	void setLinearDamping(PxReal linDamp)
	{
		PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setLinearDamping(linDamp);
		}
	}

	void setSolverIterationCounts(PxU32 minPositionIters, PxU32 minVelocityIters = 1)
	{
		PxRigidDynamic* rigidDynamic = isRigidDynamic();
		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setSolverIterationCounts(minPositionIters, minVelocityIters);
		}
	}

	void addTorque(const PxVec3& torque, PxForceMode::Enum mode = PxForceMode::eFORCE, bool autowake = true)
	{
		PxRigidBody* rigidBody = isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->addTorque(torque, mode, autowake);
		}
	}

	PxVec3 getCMassGlobalPosition() const
	{
		const PxRigidBody* rigidBody = isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_READ(rigidBody->getScene());
			return (rigidBody->getGlobalPose()*rigidBody->getCMassLocalPose()).p;
		}
		return PxVec3(.0f);
	}

	PxTransform getGlobalPose() const
	{
		const PxRigidBody* rigidBody = isRigidBody();
		PxTransform	value;
		if (rigidBody)
		{
			PHYSX3_LOCK_READ(rigidBody->getScene());
			value = rigidBody->getGlobalPose();
		}
		return value;
	}

	void setGlobalPose(const PxTransform& pose, bool autowake = true)
	{
		PxRigidBody* rigidBody = isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->setGlobalPose(pose, autowake);
		}
	}

	void moveKinematic(const PxTransform& destination)
	{
		PxRigidDynamic* rigidDynamic = isRigidDynamic();

		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setKinematicTarget(destination);
		}
	}

	void wakeUp(PxReal wakeCounterValue = DEFAULT_WAKE_UP_COUNTER)
	{
		PxRigidDynamic*	rigidDynamic	= isRigidDynamic();

		if (rigidDynamic)
		{
			PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			if ( !(rigidDynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC ) )
			{
				rigidDynamic->wakeUp();
				rigidDynamic->setWakeCounter(wakeCounterValue);
			}
		}
	}
	
	PxReal getMass()  const
	{
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_READ(rigidBody->getScene());
			return rigidBody->getMass();
		}

		return 0.0f;
	}

	void setCMassLocalPose(const PxTransform& pose)
	{
		PxRigidBody* rigidBody = isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->setCMassLocalPose(pose);
		}
	}

	PxVec3 getLinearVelocity() const
	{
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_READ(rigidBody->getScene());
			return rigidBody->getLinearVelocity();
		}
		return PxVec3(0.0f);
	}
	void setLinearVelocity(const PxVec3& linVel, bool autowake = true)
	{
		PxRigidBody* rigidBody = isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->setLinearVelocity(linVel, autowake);
		}
	}
	PxVec3 getAngularVelocity() const
	{
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_READ(rigidBody->getScene());
			return rigidBody->getAngularVelocity();
		}
		return PxVec3(0.0f);
	}
	void setAngularVelocity(const PxVec3& angVel, bool autowake = true)
	{
		PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->setAngularVelocity(angVel, autowake);
		}
	}

	PxVec3 getPointVelocity(const PxVec3& worldPosition) const
	{
		PxVec3 vel(0.0f);
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			PHYSX3_LOCK_READ(rigidBody->getScene());
			vel = PxRigidBodyExt::getVelocityAtPos(*rigidBody, worldPosition);
		}
		return vel;
	}

	NxActor() : PxActor(PxBaseFlag::eIS_RELEASABLE | PxBaseFlag::eOWNS_MEMORY) {}
};

PX_INLINE PxShape* getShape(const NxActor& actor, PxU32 index)
{
	const PxRigidActor*	rigidActor = actor.isRigidActor();

	if (!rigidActor)
	{
		return NULL;
	}
	PHYSX3_LOCK_READ(rigidActor->getScene());
	PxShape* shape;
	rigidActor->getShapes(&shape, 1, index);

	return (PxShape*)shape;
}


class NxBox
	{
	public:
	/**
	\brief Constructor
	*/
	inline NxBox()
		{
		}

	/**
	\brief Constructor

	\param _center Center of the OBB
	\param _extents Extents/radii of the obb.
	\param _rot rotation to apply to the obb.
	*/
	inline NxBox(const physx::PxVec3& _center, const physx::PxVec3& _extents, const physx::PxMat33& _rot) : center(_center), extents(_extents), rot(_rot)
		{
		}

	/**
	\brief Destructor
	*/
	inline ~NxBox()
		{
		}

	/**
	 \brief Setups an empty box.
	*/
	inline void setEmpty()
		{
		center	= physx::PxVec3(0);
		extents = physx::PxVec3(-PX_MAX_REAL, -PX_MAX_REAL, -PX_MAX_REAL);
		rot		= physx::PxMat33::createIdentity();
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 \brief Tests if a point is contained within the box

	 See #NxBoxContainsPoint().

	 \param		p	[in] the world point to test
	 \return	true if inside the box
	*/
	inline bool containsPoint(const physx::PxVec3& p) const
		{
		return NxBoxContainsPoint(*this, p);
		}

	/**
	 \brief Builds a box from AABB and a world transform.

	 See #NxCreateBox().

	 \param		aabb	[in] the aabb
	 \param		mat		[in] the world transform
	*/
	inline void create(const NxBounds3& aabb, const NxMat34& mat)
		{
		NxCreateBox(*this, aabb, mat);
		}
#endif
	/**
	 \brief Recomputes the box after an arbitrary transform by a 4x4 matrix.

	 \param		mtx		[in] the transform matrix
	 \param		obb		[out] the transformed OBB
	*/
	/**
	 \brief Checks the box is valid.

	 \return	true if the box is valid
	*/
	inline bool isValid() const
		{
		// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
		if (extents.x < 0.0f)	return false;
		if (extents.y < 0.0f)	return false;
		if (extents.z < 0.0f)	return false;
		return true;
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 \brief Computes the obb planes.

	 See #NxComputeBoxPlanes().

	 \param		planes	[out] 6 box planes
	 \return	true if success
	*/
	inline bool computePlanes(NxPlane* planes) const
		{
		return NxComputeBoxPlanes(*this, planes);
		}

	/**
	 \brief Computes the obb points.

	 See #NxComputeBoxPoints().

	 \param		pts	[out] 8 box points
	 \return	true if success
	*/
	inline bool computePoints(physx::PxVec3* pts) const
		{
		return NxComputeBoxPoints(*this, pts);
		}

	/**
	 \brief Computes vertex normals.

	 See #NxComputeBoxVertexNormals().

	 \param		pts	[out] 8 box points
	 \return	true if success
	*/
	inline bool computeVertexNormals(physx::PxVec3* pts) const
		{
		return NxComputeBoxVertexNormals(*this, pts);
		}

	/**
	 \brief Returns edges.

	 See #NxGetBoxEdges().

	 \return	24 indices (12 edges) indexing the list returned by ComputePoints()
	*/
	inline const NxU32* getEdges() const
		{
		return NxGetBoxEdges();
		}

		/**
		\brief Return edge axes indices.

		 See #NxgetBoxEdgeAxes().

		 \return Array of edge axes indices.
		*/

	inline const NxI32* getEdgesAxes() const
		{
		return NxGetBoxEdgesAxes();
		}

	/**
	 \brief Returns triangles.

	 See #NxGetBoxTriangles().


	 \return 36 indices (12 triangles) indexing the list returned by ComputePoints()
	*/
	inline const NxU32* getTriangles() const
		{
		return NxGetBoxTriangles();
		}

	/**
	 \brief Returns local edge normals.

	 See #NxGetBoxLocalEdgeNormals().

	 \return edge normals in local space
	*/
	inline const physx::PxVec3* getLocalEdgeNormals() const
		{
		return NxGetBoxLocalEdgeNormals();
		}

	/**
	 \brief Returns world edge normal

	 See #NxComputeBoxWorldEdgeNormal().

	 \param		edge_index		[in] 0 <= edge index < 12
	 \param		world_normal	[out] edge normal in world space
	*/
	inline void computeWorldEdgeNormal(NxU32 edge_index, physx::PxVec3& world_normal) const
		{
		NxComputeBoxWorldEdgeNormal(*this, edge_index, world_normal);
		}

	/**
	 \brief Computes a capsule surrounding the box.

	 See #NxComputeCapsuleAroundBox().

	 \param		capsule	[out] the capsule
	*/
	inline void computeCapsule(NxCapsule& capsule) const
		{
		NxComputeCapsuleAroundBox(*this, capsule);
		}

	/**
	 \brief Checks the box is inside another box

	 See #NxIsBoxAInsideBoxB().

	 \param		box		[in] the other box
	 \return	TRUE if we're inside the other box
	*/
	inline bool isInside(const NxBox& box) const
		{
		return NxIsBoxAInsideBoxB(*this, box);
		}
#endif
	// Accessors

		/**
		\brief Return center of box.

		\return Center of box.
		*/
	inline const physx::PxVec3& GetCenter() const
		{
		return center;
		}

		/**
		\brief Return extents(radii) of box.

		\return Extents of box.
		*/

	inline const physx::PxVec3& GetExtents() const
		{
		return extents;
		}

		/**
		\brief return box rotation.

		\return Box Rotation.
		*/

	inline const physx::PxMat33& GetRot() const
		{
		return rot;
		}

/*	inline	void GetRotatedExtents(NxMat33& extents) const
		{
		extents = mRot;
		extents.Scale(mExtents);
		}*/

	physx::PxVec3	center;
	physx::PxVec3	extents;
	physx::PxMat33	rot;
	};


/**
\brief Represents an infinite ray as an origin and direction.

The direction should be normalized.
*/
class NxRay
	{
	public:
	/**
	Constructor
	*/
	inline NxRay()
		{
		}

	/**
	Constructor
	*/
	inline NxRay(const PxVec3& _orig, const PxVec3& _dir) : orig(_orig), dir(_dir)
		{
		}

	/**
	Destructor
	*/
	inline ~NxRay()
		{
		}
#ifdef FOUNDATION_EXPORTS

	inline PxF32 distanceSquared(const PxVec3& point, PxF32* t=NULL) const
		{
		return NxComputeDistanceSquared(*this, point, t);
		}

	inline PxF32 distance(const PxVec3& point, PxF32* t=NULL) const
		{
		return sqrtf(distanceSquared(point, t));
		}
#endif

	PxVec3	orig;	//!< Ray origin
	PxVec3	dir;	//!< Normalized direction
	};

	inline void ComputeReflexionVector(PxVec3& reflected, const PxVec3& incoming_dir, const PxVec3& outward_normal)
	{
		reflected = incoming_dir - outward_normal * 2.0f * incoming_dir.dot(outward_normal);
	}

	inline void ComputeReflexionVector(PxVec3& reflected, const PxVec3& source, const PxVec3& impact, const PxVec3& normal)
	{
		PxVec3 V = impact - source;
		reflected = V - normal * 2.0f * V.dot(normal);
	}

	inline void ComputeNormalCompo(PxVec3& normal_compo, const PxVec3& outward_dir, const PxVec3& outward_normal)
	{
		normal_compo = outward_normal * outward_dir.dot(outward_normal);
	}

	inline void ComputeTangentCompo(PxVec3& outward_dir, const PxVec3& outward_normal)
	{
		outward_dir -= outward_normal * outward_dir.dot(outward_normal);
	}

	inline void DecomposeVector(PxVec3& normal_compo, PxVec3& tangent_compo, const PxVec3& outward_dir, const PxVec3& outward_normal)
	{
		normal_compo = outward_normal * outward_dir.dot(outward_normal);
		tangent_compo = outward_dir - normal_compo;
	}


	/**
\brief Represents a sphere defined by its center point and radius.
*/
class NxSphere
	{
	public:
	/**
	\brief Constructor
	*/
	inline NxSphere()
		{
		}

	/**
	\brief Constructor
	*/
	inline NxSphere(const PxVec3& _center, PxF32 _radius) : center(_center), radius(_radius)
		{
		}
#ifdef FOUNDATION_EXPORTS
	/**
	\brief Constructor
	*/
	inline NxSphere(unsigned nb_verts, const PxVec3* verts)
		{
		NxComputeSphere(*this, nb_verts, verts);
		}
#endif
	/**
	\brief Copy constructor
	*/
	inline NxSphere(const NxSphere& sphere) : center(sphere.center), radius(sphere.radius)
		{
		}
#ifdef FOUNDATION_EXPORTS

	/**
	\brief Union of spheres
	*/
	inline NxSphere(const NxSphere& sphere0, const NxSphere& sphere1)
		{
		NxMergeSpheres(*this, sphere0, sphere1);
		}
#endif
	/**
	\brief Destructor
	*/
	inline ~NxSphere()
		{
		}
#ifdef FOUNDATION_EXPORTS

	inline NxBSphereMethod compute(unsigned nb_verts, const PxVec3* verts)
		{
		return NxComputeSphere(*this, nb_verts, verts);
		}

	inline bool fastCompute(unsigned nb_verts, const PxVec3* verts)
		{
		return NxFastComputeSphere(*this, nb_verts, verts);
		}
#endif
	/**
	\brief Checks the sphere is valid.

	\return		true if the sphere is valid
	*/
	inline bool IsValid() const
		{
		// Consistency condition for spheres: Radius >= 0.0f
		return radius >= 0.0f;
		}

	/**
	\brief Tests if a point is contained within the sphere.

	\param[in] p the point to test
	\return	true if inside the sphere
	*/
	inline bool Contains(const PxVec3& p) const
		{
		return distanceSquared(center, p) <= radius*radius;
		}

	/**
	\brief Tests if a sphere is contained within the sphere.

	\param		sphere	[in] the sphere to test
	\return		true if inside the sphere
	*/
	inline bool Contains(const NxSphere& sphere)	const
		{
		// If our radius is the smallest, we can't possibly contain the other sphere
		if (radius < sphere.radius)	return false;
		// So r is always positive or null now
		float r = radius - sphere.radius;
		return distanceSquared(center, sphere.center) <= r*r;
		}

	/**
	\brief Tests if a box is contained within the sphere.

	\param		min		[in] min value of the box
	\param		max		[in] max value of the box
	\return		true if inside the sphere
	*/
	inline bool Contains(const PxVec3& min, const PxVec3& max) const
		{
		// I assume if all 8 box vertices are inside the sphere, so does the whole box.
		// Sounds ok but maybe there's a better way?
		PxF32 R2 = radius * radius;
		PxVec3 p;
		p.x=max.x; p.y=max.y; p.z=max.z;	if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;
		p.x=max.x; p.y=min.y;				if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;
		p.x=max.x; p.y=max.y; p.z=min.z;	if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;
		p.x=max.x; p.y=min.y;				if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;

		return true;
		}

	/**
	\brief Tests if the sphere intersects another sphere

	\param		sphere	[in] the other sphere
	\return		true if spheres overlap
	*/
	inline bool Intersect(const NxSphere& sphere) const
		{
		PxF32 r = radius + sphere.radius;
		return distanceSquared(center, sphere.center) <= r*r;
		}

	PxVec3	center;		//!< Sphere's center
	PxF32	radius;		//!< Sphere's radius
	};
} // namespace physx
