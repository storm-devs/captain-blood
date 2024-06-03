#ifndef NX_PHYSICS_NXP
#define NX_PHYSICS_NXP
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

//this header should be included first thing in all headers in physics/include
#ifndef NXP_DLL_EXPORT
	#if defined NX_PHYSICS_DLL
             #if defined(WIN32)
		#define NXP_DLL_EXPORT __declspec(dllexport)
		//new: default foundation to static lib:
		#define NXF_DLL_EXPORT //__declspec(dllimport)
             #elif defined(LINUX) && defined(NX_LINUX_USE_VISIBILITY)
		#define NXP_DLL_EXPORT __attribute__ ((visibility ("default")))
		#define NXF_DLL_EXPORT 
             #else
		#define NXP_DLL_EXPORT 
		#define NXF_DLL_EXPORT 
             #endif
	#elif defined NX_PHYSICS_STATICLIB
		#define NXP_DLL_EXPORT
		#define NXF_DLL_EXPORT
	#elif defined NX_USE_SDK_DLLS
             #if defined(WIN32)
		#define NXP_DLL_EXPORT __declspec(dllimport)
		//new: default foundation to static lib:
		#define NXF_DLL_EXPORT //__declspec(dllimport)
             #elif defined(LINUX) && defined(NX_LINUX_USE_VISIBILITY)
		#define NXP_DLL_EXPORT __attribute__ ((visibility ("default")))
		#define NXF_DLL_EXPORT 
             #else
		#define NXP_DLL_EXPORT 
		#define NXF_DLL_EXPORT 
             #endif
	#elif defined NX_USE_SDK_STATICLIBS

		#define NXP_DLL_EXPORT
		#define NXF_DLL_EXPORT

	#else

             #if defined(WIN32)
		//#error Please define either NX_USE_SDK_DLLS or NX_USE_SDK_STATICLIBS in your project settings depending on the kind of libraries you use!
		//new: default foundation to static lib:
		#define NXP_DLL_EXPORT __declspec(dllimport)
		#define NXF_DLL_EXPORT //__declspec(dllimport)
             #elif defined(LINUX) && defined(NX_LINUX_USE_VISIBILITY)
		#define NXP_DLL_EXPORT __attribute__ ((visibility ("default")))
		#define NXF_DLL_EXPORT
             #else
		#define NXP_DLL_EXPORT 
		#define NXF_DLL_EXPORT 
             #endif
  			
	#endif
#endif

#include "Nxf.h"
#include "NxVec3.h"
#include "NxQuat.h"
#include "NxMat33.h"
#include "NxMat34.h"

#include "NxVersionNumber.h"
/**
Pass the constant NX_PHYSICS_SDK_VERSION to the NxCreatePhysicsSDK function. 
This is to ensure that the application is using the same header version as the
library was built with.
*/

#define NX_PHYSICS_SDK_VERSION ((   NX_SDK_VERSION_MAJOR   <<24)+(NX_SDK_VERSION_MINOR    <<16)+(NX_SDK_VERSION_BUGFIX    <<8) + 0)
//2.1.1 Automatic scheme via VersionNumber.h on July 9, 2004.
//2.1.0 (new scheme: major.minor.build.configCode) on May 12, 2004. ConfigCode can be i.e. 32 vs. 64 bit.
//2.3 on Friday April 2, 2004, starting ag. changes.
//2.2 on Friday Feb 13, 2004
//2.1 on Jan 20, 2004

/*
Note: users can't change these defines as it would need the libraries to be recompiled!

AM: PLEASE MAKE SURE TO HAVE AN 'NX_' PREFIX ON ALL NEW DEFINES YOU ADD HERE!!!!!

*/

#define NX_FIX_TTP_1922				1
#define NX_SUPPORT_SWEEP_API		1
#define NX_HAS_CCD_SKELETONS
#define NX_ENABLE_HW_PARSER			0
//#define SUPPORT_INTERNAL_RADIUS	// For raycast CCD only
//#define NX_SUPPORT_MESH_SCALE		// Experimental mesh scale support

//#define NX_DISABLE_FLUIDS
//#define NX_DISABLE_CLOTH
//#define NX_DISABLE_SOFTBODY

#ifdef NX_DISABLE_FLUIDS
	#define NX_USE_FLUID_API  0
	//#define NX_USE_SDK_FLUIDS 0
#else
	// If we are exposing the Fluid API.
	#define NX_USE_FLUID_API 1

	// If we are compiling in support for Fluid.
	// We check to see if this is already defined
	// by the Project or Makefile.
	//#ifndef NX_USE_SDK_FLUIDS
		//#define NX_USE_SDK_FLUIDS 1
	//#endif

	#if (defined(__CELLOS_LV2__) || defined(_XBOX))
		#define NX_USE_LL_SW_FLUIDS 1
	#else
		#define NX_USE_LL_SW_FLUIDS 0
	#endif
	#if NX_USE_LL_SW_FLUIDS
		// Experimental code path which allows to put fluids in the primary scene
		//#define NX_FLUID_IN_PRIMARY_SCENE
	#endif
#endif /* NX_DISABLE_FLUIDS */

// Exposing of the Cloth API
#ifdef NX_DISABLE_CLOTH
	#define NX_USE_CLOTH_API 0
#else
	#define NX_USE_CLOTH_API 1
#endif /* NX_DISABLE_CLOTH */

// Exposing of the SoftBody API
#ifdef NX_DISABLE_SOFTBODY
	#define NX_USE_SOFTBODY_API 0
#else
	#define NX_USE_SOFTBODY_API 1
#endif /* NX_DISABLE_SOFTBODY */

#ifdef _XBOX
    #define XBOX_MULT_THREAD_DEFORMABLE  1
	#define XBOX_DEFORMABLE_BATCH_SIZE   32
#endif

// a bunch of simple defines used in several places:

typedef NxU16 NxActorGroup;
typedef NxU16 NxDominanceGroup;		// Must be < 32, NxU16 to be symmetric with NxCollisionGroup, NxActorGroup.
typedef NxU16 NxCollisionGroup;		// Must be < 32
typedef NxU16 NxMaterialIndex;
typedef NxU16 NxForceFieldVariety;
typedef NxU16 NxForceFieldMaterial;

typedef NxU32 NxTriangleID;

////// moved enums here that are used in Core so we don't have to include headers such as NxBoxShape in core!!

/**
\brief Identified a particular shape type.

@see NxShape NxShapeDesc
*/
enum NxShapeType
	{
	/**
	\brief A physical plane
    @see NxPlaneShape
	*/
	NX_SHAPE_PLANE,

	/**
	\brief A physical sphere
	@see NxSphereShape
	*/
	NX_SHAPE_SPHERE,

	/**
	\brief A physical box (OBB)
	@see NxBoxShape
	*/
	NX_SHAPE_BOX,

	/**
	\brief A physical capsule (LSS)
	@see NxCapsuleShape
	*/
	NX_SHAPE_CAPSULE,

	/**
	\brief A wheel for raycast cars
	@see NxWheelShape
	*/
	NX_SHAPE_WHEEL,

	/**
	\brief A physical convex mesh
	@see NxConvexShape NxConvexMesh
	*/
	NX_SHAPE_CONVEX,

	/**
	\brief A physical mesh
	@see NxTriangleMeshShape NxTriangleMesh
	*/
	NX_SHAPE_MESH,

	/**
	\brief A physical height-field
	@see NxHeightFieldShape NxHeightField
	*/
	NX_SHAPE_HEIGHTFIELD,

	/**
	\brief internal use only!

	*/
	NX_SHAPE_RAW_MESH,

	NX_SHAPE_COMPOUND,

	NX_SHAPE_COUNT,

	NX_SHAPE_FORCE_DWORD = 0x7fffffff
	};

enum NxMeshShapeFlag
	{
	/**
	\brief Select between "normal" or "smooth" sphere-mesh/raycastcapsule-mesh contact generation routines.

	The 'normal' algorithm assumes that the mesh is composed from flat triangles. 
	When a ball rolls or a raycast capsule slides along the mesh surface, it will experience small,
	sudden changes in velocity as it rolls from one triangle to the next. The smooth algorithm, on
	the other hand, assumes that the triangles are just an approximation of a surface that is smooth.  
	It uses the Gouraud algorithm to smooth the triangles' vertex normals (which in this 
	case are particularly important). This way the rolling sphere's/capsule's velocity will change
	smoothly over time, instead of suddenly. We recommend this algorithm for simulating car wheels
	on a terrain.

	@see NxSphereShape NxTriangleMeshShape NxHeightFieldShape
	*/
	NX_MESH_SMOOTH_SPHERE_COLLISIONS	= (1<<0),
	NX_MESH_DOUBLE_SIDED				= (1<<1)	//!< The mesh is double-sided. This is currently only used for raycasting.
	};

/**
\brief Lets the user specify how to handle mesh paging to the PhysX card.

It is always possible to manually upload mesh pages, also in the modes called "automatic".
Actually the "automatic" modes should preferrably be used in a half-automatic manner, 
having the automaticy as a fallback if there are pages missing.

Although it is possible to manually map pages, you can not override the automatic
page mapping by unmapping an automatically mapped page. The pages that are mapped
on the PhysX card have two reference counters; one for the user (manual mapping) and
one for the automatic mapping. Only when both references are removed is the page
removed.
*/
enum NxMeshPagingMode
	{
	/**
	\brief Manual mesh paging is the default, and it is up to the user to see to it that all
	needed mesh pages are available on the PhysX card when they are needed.

	This mode should be the default choice, as it lets the developer prepare for scene
	switches, by uploading new mesh data continously.
	*/
	NX_MESH_PAGING_MANUAL,

	/**
	\brief [Experimental] An automatic fallback to SW collision testing when mesh pages are missing on the PhysX card.

	This mode uses SW collision methods when an interaction between a shape and a mesh shape is missing mesh pages.
	The kind of constraints that are generated are both slower to generate, and takes more time to simulate.

	\warning This mesh paging method is still an experimental feature
	*/
	NX_MESH_PAGING_FALLBACK,

	/**
	\brief [Experimental] Automatic paging of missing mesh pages.

	This mode checks which mesh pages are needed for each interaction between a mesh shape and another shape.
	If the page is not available on the PhysX card, then it is uploaded. If it is not possible to upload the
	mesh page (e.g. because the PhysX card is out of memory), then the SW fallback (see NX_MESH_PAGING_FALLBACK)
	is used instead.
	\warning This mesh paging method is still an experimental feature
	*/
	NX_MESH_PAGING_AUTO
	};

enum NxCapsuleShapeFlag
	{
	/**
	\brief If this flag is set, the capsule shape represents a moving sphere, 
	moving along the ray defined by the capsule's positive Y axis.

	Currently this behavior is only implemented for points (zero radius spheres).

	Note:Used only in conjunction with the old-style capsule based wheel shape (see
	the Guide for more information).
	*/
	NX_SWEPT_SHAPE	= (1<<0)
	};

/**
\brief Parameter to addForce*() calls, determines the exact operation that is carried out.

@see NxActor.addForce() NxActor.addTorque()
*/
enum NxForceMode
	{
	NX_FORCE,                   //!< parameter has unit of mass * distance/ time^2, i.e. a force
	NX_IMPULSE,                 //!< parameter has unit of mass * distance /time
	NX_VELOCITY_CHANGE,			//!< parameter has unit of distance / time, i.e. the effect is mass independent: a velocity change.
	NX_SMOOTH_IMPULSE,          //!< same as NX_IMPULSE but the effect is applied over all substeps. Use this for motion controllers that repeatedly apply an impulse.
	NX_SMOOTH_VELOCITY_CHANGE,	//!< same as NX_VELOCITY_CHANGE but the effect is applied over all substeps. Use this for motion controllers that repeatedly apply an impulse.
	NX_ACCELERATION				//!< parameter has unit of distance/ time^2, i.e. an acceleration. It gets treated just like a force except the mass is not divided out before integration.
	};

#define NX_SLEEP_INTERVAL (20.0f*0.02f)		// Corresponds to 20 frames for the standard time step.
//#define NX_SLEEP_INTERVAL (999999999.0f)


/**
\brief Collection of flags describing the behavior of a dynamic rigid body.

@see NxBodyDesc NxActor NxActorDesc
*/
enum NxBodyFlag
	{
	/**
	\brief Set if gravity should not be applied on this body

	@see NxBodyDesc.flags NxScene.setGravity()
	*/
	NX_BF_DISABLE_GRAVITY	= (1<<0),
	
	/**	
	\brief Enable/disable freezing for this body/actor. 

	\note This is an EXPERIMENTAL feature which doesn't always work on in all situations, e.g. 
	for actors which have joints connected to them.
	
	To freeze an actor is a way to simulate that it is static. The actor is however still simulated
	as if it was dynamic, it's position is just restored after the simulation has finished. A much
	more stable way to make an actor temporarily static is to raise the NX_BF_KINEMATIC flag.
	*/
	NX_BF_FROZEN_POS_X		= (1<<1),
	NX_BF_FROZEN_POS_Y		= (1<<2),
	NX_BF_FROZEN_POS_Z		= (1<<3),
	NX_BF_FROZEN_ROT_X		= (1<<4),
	NX_BF_FROZEN_ROT_Y		= (1<<5),
	NX_BF_FROZEN_ROT_Z		= (1<<6),
	NX_BF_FROZEN_POS		= NX_BF_FROZEN_POS_X|NX_BF_FROZEN_POS_Y|NX_BF_FROZEN_POS_Z,
	NX_BF_FROZEN_ROT		= NX_BF_FROZEN_ROT_X|NX_BF_FROZEN_ROT_Y|NX_BF_FROZEN_ROT_Z,
	NX_BF_FROZEN			= NX_BF_FROZEN_POS|NX_BF_FROZEN_ROT,


	/**
	\brief Enables kinematic mode for the actor.
	
	Kinematic actors are special dynamic actors that are not 
	influenced by forces (such as gravity), and have no momentum. They are considered to have infinite
	mass and can be moved around the world using the moveGlobal*() methods. They will push 
	regular dynamic actors out of the way. Kinematics will not collide with static or other kinematic objects.
	
	Kinematic actors are great for moving platforms or characters, where direct motion control is desired.

	You can not connect Reduced joints to kinematic actors. Lagrange joints work ok if the platform
	is moving with a relatively low, uniform velocity.

	@see NxActor NxActor.raiseActorFlag()
	*/
	NX_BF_KINEMATIC			= (1<<7),		//!< Enable kinematic mode for the body.

	/**
	\brief Enable debug renderer for this body

	@see NxScene.getDebugRenderable() NxDebugRenderable NxParameter
	*/
	NX_BF_VISUALIZATION		= (1<<8),

	NX_BF_DUMMY_0			= (1<<9), // deprecated flag placeholder

	/**
	\brief Filter velocities used keep body awake. The filter reduces rapid oscillations and transient spikes.
	@see NxActor.isSleeping()
	*/
	NX_BF_FILTER_SLEEP_VEL	= (1<<10),

	/**
	\brief Enables energy-based sleeping algorithm.
	@see NxActor.isSleeping() NxBodyDesc.sleepEnergyThreshold 
	*/
	NX_BF_ENERGY_SLEEP_TEST	= (1<<11),
	};


/**
\brief Flags which affect the behavior of NxShapes.

@see NxShape NxShapeDesc NxShape.setFlag()
*/
enum NxShapeFlag
	{
	/**
	\brief Trigger callback will be called when a shape enters the trigger volume.

	@see NxUserTriggerReport NxScene.setUserTriggerReport()
	*/
	NX_TRIGGER_ON_ENTER				= (1<<0),
	
	/**
	\brief Trigger callback will be called after a shape leaves the trigger volume.

	@see NxUserTriggerReport NxScene.setUserTriggerReport()
	*/
	NX_TRIGGER_ON_LEAVE				= (1<<1),
	
	/**
	\brief Trigger callback will be called while a shape is intersecting the trigger volume.

	@see NxUserTriggerReport NxScene.setUserTriggerReport()
	*/
	NX_TRIGGER_ON_STAY				= (1<<2),

	/**
	\brief Combination of all the other trigger enable flags.

	@see NxUserTriggerReport NxScene.setUserTriggerReport()
	*/
	NX_TRIGGER_ENABLE				= NX_TRIGGER_ON_ENTER|NX_TRIGGER_ON_LEAVE|NX_TRIGGER_ON_STAY,

	/**
	\brief Enable debug renderer for this shape

	@see NxScene.getDebugRenderable() NxDebugRenderable NxParameter
	*/
	NX_SF_VISUALIZATION				= (1<<3),

	/**
	\brief Disable collision detection for this shape (counterpart of NX_AF_DISABLE_COLLISION)

	\warning IMPORTANT: this is only used for compound objects! Use NX_AF_DISABLE_COLLISION otherwise.
	*/
	NX_SF_DISABLE_COLLISION			= (1<<4),

	/**
	\brief Enable feature indices in contact stream.

	@see NxUserContactReport NxContactStreamIterator NxContactStreamIterator.getFeatureIndex0()
	*/
	NX_SF_FEATURE_INDICES			= (1<<5),

	/**
	\brief Disable raycasting for this shape
	*/
	NX_SF_DISABLE_RAYCASTING		= (1<<6),

	/**
	\brief Enable contact force reporting per contact point in contact stream (otherwise we only report force per actor pair)
	*/
	NX_SF_POINT_CONTACT_FORCE		= (1<<7),

	NX_SF_FLUID_DRAIN				= (1<<8),	//!< Sets the shape to be a fluid drain.
	NX_SF_FLUID_DISABLE_COLLISION	= (1<<10),	//!< Disable collision with fluids.
	NX_SF_FLUID_TWOWAY				= (1<<11),	//!< Enables the reaction of the shapes actor on fluid collision.

	/**
	\brief Disable collision response for this shape (counterpart of NX_AF_DISABLE_RESPONSE)

	\warning not supported by cloth / soft bodies
	*/
	NX_SF_DISABLE_RESPONSE			= (1<<12),

	/**
	\brief Enable dynamic-dynamic CCD for this shape. Used only when CCD is globally enabled and shape have a CCD skeleton.
	*/
	NX_SF_DYNAMIC_DYNAMIC_CCD		= (1<<13),

	/**
	\brief Disable participation in ray casts, overlap tests and sweeps.

	NOTE: Setting this flag for static non-trigger shapes may cause incorrect behavior for 
	colliding fluid and cloth.
	*/
	NX_SF_DISABLE_SCENE_QUERIES			= (1<<14),

	NX_SF_CLOTH_DRAIN					= (1<<15),	//!< Sets the shape to be a cloth drain.
	NX_SF_CLOTH_DISABLE_COLLISION		= (1<<16),	//!< Disable collision with cloths.

	/**
	\brief  Enables the reaction of the shapes actor on cloth collision.
	\warning Compound objects cannot use a different value for each constituent shape.
	*/
	NX_SF_CLOTH_TWOWAY					= (1<<17),	

	NX_SF_SOFTBODY_DRAIN				= (1<<18),	//!< Sets the shape to be a soft body drain.
	NX_SF_SOFTBODY_DISABLE_COLLISION	= (1<<19),	//!< Disable collision with soft bodies.

	/**
	\brief  Enables the reaction of the shapes actor on soft body collision.
	\warning Compound objects cannot use a different value for each constituent shape.
	*/
	NX_SF_SOFTBODY_TWOWAY				= (1<<20),
	};

/**
\brief For compatibility with previous SDK versions before 2.1.1

@see NxShapeFlag
*/
typedef NxShapeFlag	NxTriggerFlag;


/**
\brief Specifies which axis is the "up" direction for a heightfield.
*/
enum NxHeightFieldAxis
	{
	NX_X				= 0, //!< X Axis
	NX_Y				= 1, //!< Y Axis
	NX_Z				= 2, //!< Z Axis
	NX_NOT_HEIGHTFIELD	= 0xff //!< Not a heightfield
	};

/**
\brief Compatability/readability typedef.

@see NxVec3
*/
typedef NxVec3 NxPoint;

/**
\brief Structure used to store indices for a triangles points.
*/
struct NxTriangle32
	{
	NX_INLINE	NxTriangle32()							{}
	NX_INLINE	NxTriangle32(NxU32 a, NxU32 b, NxU32 c) { v[0] = a; v[1] = b; v[2] = c; }

	NxU32 v[3];	//vertex indices
	};

/**
\brief Structure used to store actor group pair flags.
*/

struct NxActorGroupPair
	{
	NxActorGroup group0;
	NxActorGroup group1; 
	NxU32 flags;
	};

/**
\brief Expresses the dominance relationship of a constraint.
For the time being only three settings are permitted:

(1.0f, 1.0f), (0.0f, 1.0f), and (1.0f, 0.0f).
*/	
	
struct NxConstraintDominance
	{
	NxConstraintDominance(NxReal a, NxReal b) : dominance0(a), dominance1(b) {}
	NxReal dominance0;
	NxReal dominance1;
	};

/**
\brief Typedef for submesh indexing.
@see NxTriangleMesh NxConvexMesh
*/
typedef NxU32 NxSubmeshIndex;

/**
\brief Enum to access internal data structures for triangle meshes and convex meshes.

@see NxTriangleMesh.getFormat() NxConvexMesh.getFormat()
*/
enum NxInternalFormat
	{
	NX_FORMAT_NODATA,		//!< No data available
	NX_FORMAT_FLOAT,		//!< Data is in floating-point format
	NX_FORMAT_BYTE,			//!< Data is in byte format (8 bit)
	NX_FORMAT_SHORT,		//!< Data is in short format (16 bit)
	NX_FORMAT_INT,			//!< Data is in int format (32 bit)
	};

/**
\brief Enum to access internal data structures for triangle meshes and convex meshes.

@see NxTriangleMesh.getBase() NxConvexMesh.getBase()
*/
enum NxInternalArray
	{
	NX_ARRAY_TRIANGLES,			//!< Array of triangles (index buffer). One triangle = 3 vertex references in returned format.
	NX_ARRAY_VERTICES,			//!< Array of vertices (vertex buffer). One vertex = 3 coordinates in returned format.
	NX_ARRAY_NORMALS,			//!< Array of vertex normals. One normal = 3 coordinates in returned format.
	NX_ARRAY_HULL_VERTICES,		//!< Array of hull vertices. One vertex = 3 coordinates in returned format.
	NX_ARRAY_HULL_POLYGONS,		//!< Array of hull polygons
	NX_ARRAY_TRIANGLES_REMAP,	//!< Array of triangle remap indices. One triangle index = 1 original triangle index in returned format. (NxTriangleMesh only).
	};

/**
\brief Identifies each type of joint.

@see NxJoint NxJointDesc NxScene.createJoint()
*/
enum NxJointType
	{
	/**
	\brief Permits a single translational degree of freedom.

	@see NxPrismaticJoint
	*/
	NX_JOINT_PRISMATIC,

	/**
	\brief Also known as a hinge joint, permits one rotational degree of freedom.

	@see NxRevoluteJoint
	*/
	NX_JOINT_REVOLUTE,

	/**
	\brief Formerly known as a sliding joint, permits one translational and one rotational degree of freedom.

	@see NxCylindricalJoint
	*/
	NX_JOINT_CYLINDRICAL,

	/**
	\brief Also known as a ball or ball and socket joint.

	@see NxSphericalJoint
	*/
	NX_JOINT_SPHERICAL,

	/**
	\brief A point on one actor is constrained to stay on a line on another.

	@see NxPointOnLineJoint
	*/
	NX_JOINT_POINT_ON_LINE,

	/**
	\brief A point on one actor is constrained to stay on a plane on another.

	@see NxPointInPlaneJoint
	*/
	NX_JOINT_POINT_IN_PLANE,

	/**
	\brief A point on one actor maintains a certain distance range to another point on another actor.

	@see NxDistanceJoint
	*/
	NX_JOINT_DISTANCE,

	/**
	\brief A pulley joint.

	@see NxPulleyJoint
	*/
	NX_JOINT_PULLEY,

	/**
	\brief A "fixed" connection.

	@see NxFixedJoint
	*/
	NX_JOINT_FIXED,

	/**
	\brief A 6 degree of freedom joint

	@see NxD6Joint
	*/
	NX_JOINT_D6,

	NX_JOINT_COUNT,				//!< Just to track the number of available enum values. Not a joint type.
	NX_JOINT_FORCE_DWORD = 0x7fffffff
	};

/**
\brief Describes the state of the joint.

@see NxJoint
*/
enum NxJointState
	{
	/**
	\brief Not used.
	*/
	NX_JS_UNBOUND,

	/**
	\brief The joint is being simulated under normal conditions. I.e. it is not broken or deleted.
	*/
	NX_JS_SIMULATING,

	/**
	\brief Set when the joint has been broken or one of the actors connected to the joint has been remove.
	@see NxUserNotify.onJointBreak() NxJoint.setBreakable()
	*/
	NX_JS_BROKEN
	};

/**
\brief Joint flags.

@see NxJoint NxJointDesc
*/
enum NxJointFlag
    {
	/**
	\brief Raised if collision detection should be enabled between the bodies attached to the joint.

	By default collision constraints are not generated between pairs of bodies which are connected by joints.
	*/
    NX_JF_COLLISION_ENABLED	= (1<<0),

	/**
	\brief Enable debug renderer for this joint

	@see NxScene.getDebugRenderable() NxDebugRenderable NxParameter
	*/
    NX_JF_VISUALIZATION		= (1<<1),
    };

/**
\brief Joint projection modes.

Joint projection is a method for correcting large joint errors.

It is also necessary to set the distance above which projection occurs.

@see NxRevoluteJointDesc.projectionMode NxRevoluteJointDesc.projectionDistance NxRevoluteJointDesc.projectionAngle
@see NxSphericalJointDesc.projectionMode
@see NxD6JointDesc.projectionMode
*/
enum NxJointProjectionMode
	{
	NX_JPM_NONE  = 0,				//!< don't project this joint
	NX_JPM_POINT_MINDIST = 1,		//!< linear and angular minimum distance projection
	NX_JPM_LINEAR_MINDIST = 2,		//!< linear only minimum distance projection
	//there may be more modes later
	};

/**
\brief Flags to control the behavior of revolute joints.

@see NxRevoluteJoint NxRevoluteJointDesc
*/
enum NxRevoluteJointFlag
	{
	/**
	\brief true if limits is enabled

	@see NxRevoluteJointDesc.limit
	*/
	NX_RJF_LIMIT_ENABLED = 1 << 0,
	
	/**
	\brief true if the motor is enabled

	@see NxRevoluteJoint.motor
	*/
	NX_RJF_MOTOR_ENABLED = 1 << 1,
	
	/**
	\brief true if the spring is enabled. The spring will only take effect if the motor is disabled.

	@see NxRevoluteJoint.spring
	*/
	NX_RJF_SPRING_ENABLED = 1 << 2,
	};

/**
\brief Flags to control the behavior of pulley joints.

@see NxPulleyJoint NxPulleyJointDesc NxPulleyJoint.setFlags()
*/
enum NxPulleyJointFlag
	{
	/**
	\brief true if the joint also maintains a minimum distance, not just a maximum.
	*/
	NX_PJF_IS_RIGID = 1 << 0,

	/**
	\brief true if the motor is enabled

	@see NxPulleyJointDesc.motor
	*/
	NX_PJF_MOTOR_ENABLED = 1 << 1
	};

/**
\brief Flags which control the behavior of distance joints.

@see NxDistanceJoint NxDistanceJointDesc NxDistanceJointDesc.flags
*/
enum NxDistanceJointFlag
	{
	/**
	\brief true if the joint enforces the maximum separate distance.
	*/
	NX_DJF_MAX_DISTANCE_ENABLED = 1 << 0,

	/**
	\brief true if the joint enforces the minimum separate distance.
	*/
	NX_DJF_MIN_DISTANCE_ENABLED = 1 << 1,

	/**
	\brief true if the spring is enabled
	
	@see NxDistanceJointDesc.spring
	*/
	NX_DJF_SPRING_ENABLED		= 1 << 2,
	};

/**
\brief Used to specify the range of motions allowed for a DOF in a D6 joint.

@see NxD6Joint NxD6JointDesc
@see NxD6Joint.xMotion NxD6Joint.swing1Motion
*/
enum NxD6JointMotion
	{
	NX_D6JOINT_MOTION_LOCKED,	//!< The DOF is locked, it does not allow relative motion.
	NX_D6JOINT_MOTION_LIMITED,  //!< The DOF is limited, it only allows motion within a specific range.
	NX_D6JOINT_MOTION_FREE		//!< The DOF is free and has its full range of motions.
	};


/**
\brief Used to specify a particular drive method. i.e. Having a position based goal or a velocity based goal.
*/
enum NxD6JointDriveType
	{
	/**
	\brief Used to set a position goal when driving.

	Note: the appropriate target positions/orientations should be set.

	@see NxD6JointDesc.xDrive NxD6Joint.swingDrive NxD6JointDesc.drivePosition
	*/
	NX_D6JOINT_DRIVE_POSITION	= 1<<0,

	/**
	\brief Used to set a velocity goal when driving.

	Note: the appropriate target velocities should beset.

	@see NxD6JointDesc.xDrive NxD6Joint.swingDrive NxD6JointDesc.driveLinearVelocity
	*/
	NX_D6JOINT_DRIVE_VELOCITY	= 1<<1
	};

/**
\brief Flags which control the general behavior of D6 joints.

@see NxD6Joint NxD6JointDesc NxD6JointDesc.flags
*/
enum NxD6JointFlag
	{
	/**
	\brief Drive along the shortest spherical arc.

	@see NxD6JointDesc.slerpDrive
	*/
	NX_D6JOINT_SLERP_DRIVE = 1<<0,
	/**
   	\brief Apply gearing to the angular motion, e.g. body 2s angular motion is twice body 1s etc.
   
   	@see NxD6JointDesc.gearRatio
   	*/
	NX_D6JOINT_GEAR_ENABLED = 1<<1
	};

/**
\brief Flags which control the behavior of an actor.

@see NxActor NxActorDesc NxActor.raiseBodyFlag()
*/
enum NxActorFlag
	{
	/**
	\brief Enable/disable collision detection

	Turn off collision detection, i.e. the actor will not collide with other objects. Please note that you might need to wake 
	the actor up if it is sleeping, this depends on the result you wish to get when using this flag. (If a body is asleep it 
	will not start to fall through objects unless you activate it).

	Note: Also excludes the actor from overlap tests!
	*/
	NX_AF_DISABLE_COLLISION			= (1<<0),

	/**
	\brief Enable/disable collision response (reports contacts but don't use them)

	@see NxUserContactReport
	*/
	NX_AF_DISABLE_RESPONSE			= (1<<1), 

	/**
	\brief Disables COM update when computing inertial properties at creation time.

	When sdk computes inertial properties, by default the center of mass will be calculated too.  However, if lockCOM is set to a non-zero (true) value then the center of mass will not be altered.
	*/
	NX_AF_LOCK_COM					= (1<<2),

	/**
	\brief Enable/disable collision with fluid.
	*/
	NX_AF_FLUID_DISABLE_COLLISION	= (1<<3),

	/**
	\brief Turn on contact modification callback for the actor.

	@see NxScene.setUserContactModify(), NX_NOTIFY_CONTACT_MODIFICATION
	*/
	NX_AF_CONTACT_MODIFICATION		= (1<<4),


	/**
	\brief Force cone friction to be used for this actor.	

	This ensures that all contacts involving the actor will use cone friction, rather than the default
	simplified scheme. This will however have a negative impact on performance in software scenes. Use this
	flag if sliding objects show an affinity for moving along the world axes.

	\note Only applies to software scenes. Hardware scenes always force cone friction.

	Cone friction may also be applied wholesale to a scene using the NX_SF_FORCE_CONE_FRICTION 
	flag, see #NxSceneFlags.
	*/
	NX_AF_FORCE_CONE_FRICTION		= (1<<5),

	/**
	\brief Enable/disable custom contact filtering. 

	When enabled the user will be queried for contact filtering for all contacts involving this actor.
	*/
	NX_AF_USER_ACTOR_PAIR_FILTERING	= (1<<6)
	};

/**
\brief Flags which control the properties of a scene compartment.

@see NxScene NxCompartmentDesc
*/
enum NxCompartmentFlag
	{
	/**
	\brief Enable/disable sleep event reporting

	If sleep notification is enabled for the compartment, onWake and onSleep will be called on the NxUserNotify associated with the master scene, if non-null. 
	Note that onWake and onSleep will be called one time for each compartment with sleep notification enabled, 
	i.e. not only once with all sleeping/awake bodies from all compartments.

	@see NxUserNotify
	*/
	NX_CF_SLEEP_NOTIFICATION			= (1<<0),

	/**
	\brief Enable/disable continuous collision detection on the compartment.

	@see NxPhysicsSDK.createCCDSkeleton()
	*/
	NX_CF_CONTINUOUS_CD					= (1<<1),

	/**
	\brief Enable Restricted Scene on the compartment.

	\note Only applies to hardware scenes.
	\note This flag is immutable and can hence only be set at creation.

	@see NX_SF_RESTRICTED_SCENE
	*/
	NX_CF_RESTRICTED_SCENE				= (1<<2),

	/**
	\brief Inherit NX_CF_CONTINUOUS_CD and NX_CF_RESTRICTED_SCENE.

	Override the NX_CF_CONTINUOUS_CD and NX_CF_RESTRICTED_SCENE and inherit these settings from the SDK parameters and the
	primary scene respectively (i.e. NX_CONTINUOUS_CD and NX_SF_RESTRICTED_SCENE).
	*/
	NX_CF_INHERIT_SETTINGS				= (1<<3)
	};

/**
\brief Flags which control the behavior of spherical joints.

@see NxSphericalJoint NxSphericalJointDesc NxSphericalJointDesc.flags
*/
enum NxSphericalJointFlag
	{
	/**
	\brief true if the twist limit is enabled

	@see NxSphericalJointDesc.twistLimit
	*/
	NX_SJF_TWIST_LIMIT_ENABLED = 1 << 0,
	
	/**
	\brief true if the swing limit is enabled

	@see NxSphericalJointDesc.swingLimit
	*/
	NX_SJF_SWING_LIMIT_ENABLED = 1 << 1,
	
	/**
	\brief true if the twist spring is enabled

	@see NxSphericalJointDesc.twistSpring
	*/
	NX_SJF_TWIST_SPRING_ENABLED= 1 << 2,
	
	/**
	\brief true if the swing spring is enabled

	@see NxSphericalJointDesc.swingSpring
	*/
	NX_SJF_SWING_SPRING_ENABLED= 1 << 3,
	
	/**
	\brief true if the joint spring is enabled

	@see NxSphericalJointDesc.jointSpring

	*/
	NX_SJF_JOINT_SPRING_ENABLED= 1 << 4,

	/**
	\brief Add additional constraints to linear movement.

	Constrain movements along directions perpendicular to the distance vector defined by the two anchor points.

	\note Setting this flag can increase the stability of the joint but the computation will be more expensive.
	*/
    NX_SJF_PERPENDICULAR_DIR_CONSTRAINTS	= 1 << 5,
	};

/**
\brief Used to control contact queries.

@see NxTriangleMeshShape.overlapAABBTriangles()
*/
enum NxQueryFlags
	{
	NX_QUERY_WORLD_SPACE	= (1<<0),	//!< world-space parameter, else object space
	NX_QUERY_FIRST_CONTACT	= (1<<1),	//!< returns first contact only, else returns all contacts
	};

/**
\brief Flags which are set for convex edges.

Must be the 3 first ones to be indexed by (flags & (1<<edge_index))

@see NxTriangleMeshShape.getTriangle()
*/
enum NxTriangleFlags
	{
	NXTF_ACTIVE_EDGE01		= (1<<0),
	NXTF_ACTIVE_EDGE12		= (1<<1),
	NXTF_ACTIVE_EDGE20		= (1<<2),
	NXTF_DOUBLE_SIDED		= (1<<3),
	NXTF_BOUNDARY_EDGE01	= (1<<4),
	NXTF_BOUNDARY_EDGE12	= (1<<5),
	NXTF_BOUNDARY_EDGE20	= (1<<6),
	};

/*
NOTE: Parameters should NOT be conditionally compiled out. Even if a particular feature is not available.
Otherwise the parameter values get shifted about and the numeric values change per platform. This causes problems
when trying to serialize parameters.

New parameters should also be added to the end of the list for this reason. Also make sure to update 
NX_PARAM_NUM_VALUES, which should be one higher than the maximum value in the enum.
*/

/**
\brief Parameters enums to be used as the 1st arg to setParameter or getParameter.

@see NxPhysicsSDK.setParameter() NxPhysicsSDK.getParameter()
*/
enum NxParameter
	{
/* RigidBody-related parameters  */
	
	/**
	\brief DEPRECATED! Do not use!

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_PENALTY_FORCE			= 0,
	
	/**
	\brief Default value for ::NxShapeDesc::skinWidth, see for more info.
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0.025<br>
	<b>Unit:</b> distance.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapeDesc.skinWidth
	*/
	NX_SKIN_WIDTH = 1,

	
	/**
	\brief The default linear velocity, squared, below which objects start going to sleep. 
	Note: Only makes sense when the NX_BF_ENERGY_SLEEP_TEST is not set.
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> (0.15*0.15)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Sleep behavior on hardware is different)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_DEFAULT_SLEEP_LIN_VEL_SQUARED = 2,
	
	/**
	\brief The default angular velocity, squared, below which objects start going to sleep. 
	Note: Only makes sense when the NX_BF_ENERGY_SLEEP_TEST is not set.
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> (0.14*0.14)
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Sleep behavior on hardware is different)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_DEFAULT_SLEEP_ANG_VEL_SQUARED = 3,

	
	/**
	\brief A contact with a relative velocity below this will not bounce.	
	
	<b>Range:</b> (-inf, 0]<br>
	<b>Default:</b> -2

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterial
	*/
	NX_BOUNCE_THRESHOLD = 4,

	/**
	\brief This lets the user scale the magnitude of the dynamic friction applied to all objects.	
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 1

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterial
	*/
	NX_DYN_FRICT_SCALING = 5,
	
	/**
	\brief This lets the user scale the magnitude of the static friction applied to all objects.
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 1
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterial
	*/
	NX_STA_FRICT_SCALING = 6,

	
	/**
	\brief See the comment for NxActor::setMaxAngularVelocity() for details.
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 7

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc.setMaxAngularVelocity()
	*/
	NX_MAX_ANGULAR_VELOCITY = 7,

/* Collision-related parameters:  */

	
	/**
	\brief Enable/disable continuous collision detection (0.0f to disable)

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPhysicsSDK.createCCDSkeleton()
	*/
	NX_CONTINUOUS_CD = 8,
	
	/**
	\brief This overall visualization scale gets multiplied with the individual scales. Setting to zero turns ignores all visualizations. Default is 0.

	The below settings permit the debug visualization of various simulation properties. 
	The setting is either zero, in which case the property is not drawn. Otherwise it is a scaling factor
	that determines the size of the visualization widgets.

	Only bodies and joints for which visualization is turned on using setFlag(VISUALIZE) are visualized.
	Contacts are visualized if they involve a body which is being visualized.
	Default is 0.

	Notes:
	- to see any visualization, you have to set NX_VISUALIZATION_SCALE to nonzero first.
	- the scale factor has been introduced because it's difficult (if not impossible) to come up with a
	good scale for 3D vectors. Normals are normalized and their length is always 1. But it doesn't mean
	we should render a line of length 1. Depending on your objects/scene, this might be completely invisible
	or extremely huge. That's why the scale factor is here, to let you tune the length until it's ok in
	your scene.
	- however, things like collision shapes aren't ambiguous. They are clearly defined for example by the
	triangles & polygons themselves, and there's no point in scaling that. So the visualization widgets
	are only scaled when it makes sense.

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (only a subset of visualizations are supported)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZATION_SCALE = 9,

	
	/**
	\brief Visualize the world axes.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_WORLD_AXES = 10,
	
/* Body visualizations */

	/**
	\brief Visualize a bodies axes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor.globalPose NxActor
	*/
	NX_VISUALIZE_BODY_AXES = 11,
	
	/**
	\brief Visualize a body's mass axes.

	This visualization is also useful for visualizing the sleep state of bodies. Sleeping bodies are drawn in
	black, while awake bodies are drawn in white. If the body is sleeping and part of a sleeping group, it is
	drawn in red.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc.massLocalPose NxActor
	*/
	NX_VISUALIZE_BODY_MASS_AXES = 12,
	
	/**
	\brief Visualize the bodies linear velocity.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc.linearVelocity NxActor
	*/
	NX_VISUALIZE_BODY_LIN_VELOCITY = 13,
	
	/**
	\brief Visualize the bodies angular velocity.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBodyDesc.angularVelocity NxActor
	*/
	NX_VISUALIZE_BODY_ANG_VELOCITY = 14,
	
/*
	Obsolete parameters
	NX_VISUALIZE_BODY_LIN_MOMENTUM = 15,
	NX_VISUALIZE_BODY_ANG_MOMENTUM = 16,
	NX_VISUALIZE_BODY_LIN_ACCEL = 17,
	NX_VISUALIZE_BODY_ANG_ACCEL = 18,
	NX_VISUALIZE_BODY_LIN_FORCE = 19,
	NX_VISUALIZE_BODY_ANG_FORCE = 20,
	NX_VISUALIZE_BODY_REDUCED = 21,
*/

	/**
	\brief Visualize joint groups 
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_BODY_JOINT_GROUPS = 22,

/* Obsolete parameters
	NX_VISUALIZE_BODY_CONTACT_LIST = 23,
	NX_VISUALIZE_BODY_JOINT_LIST = 24,
	NX_VISUALIZE_BODY_DAMPING = 25,
	NX_VISUALIZE_BODY_SLEEP = 26,
*/

/* Joint visualisations */
	/**
	\brief Visualize local joint axes (including drive targets, if any)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJointDesc.localAxis NxJoint
	*/
	NX_VISUALIZE_JOINT_LOCAL_AXES = 27,
	
	/**
	\brief Visualize joint world axes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint
	*/
	NX_VISUALIZE_JOINT_WORLD_AXES = 28,
	
	/**
	\brief Visualize joint limits.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint
	*/
	NX_VISUALIZE_JOINT_LIMITS = 29,
	
/* Obsolete parameters
	NX_VISUALIZE_JOINT_ERROR = 30,
	NX_VISUALIZE_JOINT_FORCE = 31,
	NX_VISUALIZE_JOINT_REDUCED = 32,
*/

/* Contact visualisations */

	/**
	\brief  Visualize contact points. Will enable contact information.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_CONTACT_POINT = 33,
	
	/**
	\brief Visualize contact normals. Will enable contact information.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_CONTACT_NORMAL = 34,
	
	/**
	\brief  Visualize contact errors. Will enable contact information.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_CONTACT_ERROR = 35,
	
	/**
	\brief Visualize Contact forces. Will enable contact information.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_CONTACT_FORCE = 36,

	
	/**
	\brief Visualize actor axes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor
	*/
	NX_VISUALIZE_ACTOR_AXES = 37,

	
	/**
	\brief Visualize bounds (AABBs in world space)
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_COLLISION_AABBS = 38,
	
	/**
	\brief Shape visualization

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape
	*/
	NX_VISUALIZE_COLLISION_SHAPES = 39,
	
	/**
	\brief Shape axis visualization

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape
	*/
	NX_VISUALIZE_COLLISION_AXES = 40,
	
	/**
	\brief Compound visualization (compound AABBs in world space)
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_COLLISION_COMPOUNDS = 41,
	
	/**
	\brief Mesh & convex vertex normals

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMesh NxConvexMesh
	*/
	NX_VISUALIZE_COLLISION_VNORMALS = 42,
	
	/**
	\brief Mesh & convex face normals

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMesh NxConvexMesh
	*/
	NX_VISUALIZE_COLLISION_FNORMALS = 43,
	
	/**
	\brief Active edges for meshes

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxTriangleMesh
	*/
	NX_VISUALIZE_COLLISION_EDGES = 44,
	
	/**
	\brief Bounding spheres
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_COLLISION_SPHERES = 45,
	
/* Obsolete parameter
	NX_VISUALIZE_COLLISION_SAP = 46,
*/
	
	/**
	\brief Static pruning structures
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_COLLISION_STATIC = 47,
	
	/**
	\brief Dynamic pruning structures
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_COLLISION_DYNAMIC = 48,
	
	/**
	\brief "Free" pruning structures
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_COLLISION_FREE = 49,
	
	/**
	\brief Visualize the CCD tests

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPhysicsSDK.createCCDSkeleton()
	*/
	NX_VISUALIZE_COLLISION_CCD = 50,	
	
	/**
	\brief Visualize CCD Skeletons

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPhysicsSDK.createCCDSkeleton()
	*/
	NX_VISUALIZE_COLLISION_SKELETONS = 51,

/* Fluid visualizations */
	/**
	\brief Emitter visualization.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_FLUID_EMITTERS = 52,
	
	/**
	\brief Particle position visualization.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_FLUID_POSITION = 53,
	
	/**
	\brief Particle velocity visualization.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_FLUID_VELOCITY = 54,
	
	/**
	\brief Particle kernel radius visualization.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_FLUID_KERNEL_RADIUS = 55,
	
	/**
	\brief Fluid AABB visualization.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_VISUALIZE_FLUID_BOUNDS = 56,

	/**
	\brief Fluid Packet visualization.
	*/
	NX_VISUALIZE_FLUID_PACKETS = 57,
	
	/**
	\brief Fluid motion limit visualization.
	*/
	NX_VISUALIZE_FLUID_MOTION_LIMIT = 58,

	/**
	\brief Fluid dynamic collision visualization.
	*/
	NX_VISUALIZE_FLUID_DYN_COLLISION = 59,

	/**
	\brief Fluid static collision visualization.
	*/
	NX_VISUALIZE_FLUID_STC_COLLISION = 60,

	/**
	\brief Fluid available mesh packets visualization.
	*/
	NX_VISUALIZE_FLUID_MESH_PACKETS = 61,

	/**
	\brief Fluid drain shape visualization.
	*/
	NX_VISUALIZE_FLUID_DRAINS = 62,

	/**
	\brief Fluid Packet Data visualization.
	*/
	NX_VISUALIZE_FLUID_PACKET_DATA = 90,

/* Cloth visualizations */
	/**
	\brief Cloth mesh visualization.
	*/
	NX_VISUALIZE_CLOTH_MESH = 63,

	/**
	\brief Cloth rigid body collision visualization.
	*/
	NX_VISUALIZE_CLOTH_COLLISIONS = 64,

	/**
	\brief Cloth cloth collision visualization.
	*/
	NX_VISUALIZE_CLOTH_SELFCOLLISIONS = 65,
	
	/**
	\brief Cloth clustering for the PPU simulation visualization.
	*/
	NX_VISUALIZE_CLOTH_WORKPACKETS = 66,
	
	/**
	\brief Cloth overall sleeping visualization.
	*/
	NX_VISUALIZE_CLOTH_SLEEP = 67,

	/**
	\brief Cloth per vertex sleeping visualization.
	*/
	NX_VISUALIZE_CLOTH_SLEEP_VERTEX = 94,

	/**
	\brief Cloth tearable vertices visualization.
	*/
	NX_VISUALIZE_CLOTH_TEARABLE_VERTICES = 80,

	/**
	\brief Cloth tearing visualization.
	*/
	NX_VISUALIZE_CLOTH_TEARING = 81,

	/**
	\brief Cloth attachments visualization.
	*/
	NX_VISUALIZE_CLOTH_ATTACHMENT = 82,

	/**
	\brief Cloth valid bounds visualization.
	*/
	NX_VISUALIZE_CLOTH_VALIDBOUNDS = 92,

/* SoftBody visualizations */
	/**
	\brief Soft body mesh visualization.
	*/
	NX_VISUALIZE_SOFTBODY_MESH = 83,

	/**
	\brief Soft body rigid body collision visualization.
	*/
	NX_VISUALIZE_SOFTBODY_COLLISIONS = 84,

	/**
	\brief Soft body clustering for the PPU simulation visualization.
	*/
	NX_VISUALIZE_SOFTBODY_WORKPACKETS = 85,

	/**
	\brief Soft body overall sleeping visualization.
	*/
	NX_VISUALIZE_SOFTBODY_SLEEP = 86,

	/**
	\brief Soft body per vertex sleeping visualization.
	*/
	NX_VISUALIZE_SOFTBODY_SLEEP_VERTEX = 95,

	/**
	\brief Soft body tearable vertices visualization.
	*/
	NX_VISUALIZE_SOFTBODY_TEARABLE_VERTICES = 87,

	/**
	\brief Soft body tearing visualization.
	*/
	NX_VISUALIZE_SOFTBODY_TEARING = 88,

	/**
	\brief Soft body attachments visualization.
	*/
	NX_VISUALIZE_SOFTBODY_ATTACHMENT = 89,

	/**
	\brief Soft body valid bounds visualization.
	*/
	NX_VISUALIZE_SOFTBODY_VALIDBOUNDS = 93,

/* General parameters and new parameters */

	/**
	\brief Used to enable adaptive forces to accelerate convergence of the solver.

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 1.0
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_ADAPTIVE_FORCE = 68,
	
	/**
	\brief Controls default filtering for jointed bodies. True means collision is disabled.

	<b>Range:</b> {true, false}<br>
	<b>Default:</b> true

	@see NX_JF_COLLISION_ENABLED
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_COLL_VETO_JOINTED = 69,
	
	/**
	\brief Controls whether two touching triggers generate a callback or not.

	<b>Range:</b> {true, false}<br>
	<b>Default:</b> true

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserTriggerReport
	*/
	NX_TRIGGER_TRIGGER_CALLBACK = 70,
	
	/**
	\brief Internal, used for debugging and testing. Not to be used.
	*/
	NX_SELECT_HW_ALGO = 71,
	
	/**
	\brief Internal, used for debugging and testing. Not to be used.
	*/
	NX_VISUALIZE_ACTIVE_VERTICES = 72,

	/**
	\brief Distance epsilon for the CCD algorithm.  

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0.01
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_CCD_EPSILON = 73,

	/**
	\brief Used to accelerate solver.

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0
	
	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No
	*/
	NX_SOLVER_CONVERGENCE_THRESHOLD = 74,

	/**
	\brief Used to accelerate HW Broad Phase.
	
	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0.001

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No
	*/
	NX_BBOX_NOISE_LEVEL = 75,

	/**
	\brief Used to set the sweep cache size.

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 5.0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_IMPLICIT_SWEEP_CACHE_SIZE = 76,

	/**
	\brief The default sleep energy threshold. Objects with an energy below this threshold are allowed
	to go to sleep. 
	Note: Only used when the NX_BF_ENERGY_SLEEP_TEST flag is set.

	<b>Range:</b> [0, inf)<br>
	<b>Default:</b> 0.005

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_DEFAULT_SLEEP_ENERGY = 77,

	/**
	\brief Constant for the maximum number of packets per fluid. Used to compute the fluid packet buffer size in NxFluidPacketData.

	<b>Range:</b> [925, 925]<br>
	<b>Default:</b> 925

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxFluidPacketData
	*/
	NX_CONSTANT_FLUID_MAX_PACKETS = 78,

	/**
	\brief Constant for the maximum number of new fluid particles per frame.
	<b>Range:</b> [4096, 4096]<br>
	<b>Default:</b> 4096

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_CONSTANT_FLUID_MAX_PARTICLES_PER_STEP = 79,

	/**
	\brief Force field visualization.
	*/
	NX_VISUALIZE_FORCE_FIELDS = 91,

	/**
	\brief [Experimental] Disables scene locks when creating/releasing meshes.

	Prevents the SDK from locking all scenes when creating and releasing triangle meshes, convex meshes, height field 
	meshes, softbody meshes and cloth meshes, which is the default behavior. Can be used to improve parallelism but beware
	of possible side effects.

	\warning Experimental feature.
	*/
	NX_ASYNCHRONOUS_MESH_CREATION = 96,

	/**
	\brief Epsilon for custom force field kernels.
	
	This epsilon is used in custom force field kernels (NxSwTarget). Methods like recip()
	or recipSqrt() evaluate to zero if their input is smaller than this epsilon.
	
	<br>
	*/
	NX_FORCE_FIELD_CUSTOM_KERNEL_EPSILON = 97,

	/**
	\brief Enable/disable improved spring solver for joints and wheelshapes
	
	This parameter allows to enable/disable an improved version of the spring solver for joints and wheelshapes.

	\warning 
	The parameter is introduced for legacy purposes only and will be removed in future versions (such that
	the improved spring solver will always be used).

	\note 
	Changing the parameter will only affect newly created scenes but not existing ones.

	<b>Range:</b> {0: disabled, 1: enabled}<br>
	<b>Default:</b> 1

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No (disabled by default)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_IMPROVED_SPRING_SOLVER = 98,

	NX_FAST_MASSIVE_BP_VOLUME_DELETION = 99,

	/**
	\brief This is not a parameter, it just records the current number of parameters (as max(NxParameter)+1) for use in loops.
	When a new parameter is added this number should be assigned to it and then incremented.
	*/
	NX_PARAMS_NUM_VALUES = 100,

	NX_PARAMS_FORCE_DWORD = 0x7fffffff
	};

	/**
	\brief 128-bit mask used for collision filtering.

	The collision filtering equation for 2 shapes S0 and S1 is:

	<pre> (G0 op0 K0) op2 (G1 op1 K1) == b </pre>

	with

	<ul>
	<li> G0 = NxGroupsMask for shape S0. See ::setGroupsMask </li>
	<li> G1 = NxGroupsMask for shape S1. See ::setGroupsMask </li>
	<li> K0 = filtering constant 0. See ::setFilterConstant0 </li>
	<li> K1 = filtering constant 1. See ::setFilterConstant1 </li>
	<li> b = filtering boolean. See ::setFilterBool </li>
	<li> op0, op1, op2 = filtering operations. See ::setFilterOps </li>
	</ul>

	If the filtering equation is true, collision detection is enabled.

	@see NxScene::setFilterOps()
	*/
	class NxGroupsMask
		{
		public:
		NX_INLINE	NxGroupsMask()	{}
		NX_INLINE	~NxGroupsMask()	{}

		NxU32		bits0, bits1, bits2, bits3;
		};

	/**
	\brief Collision filtering operations.
	
	@see NxGroupsMask
	*/
	enum NxFilterOp
		{
		NX_FILTEROP_AND,
		NX_FILTEROP_OR,
		NX_FILTEROP_XOR,
		NX_FILTEROP_NAND,
		NX_FILTEROP_NOR,
		NX_FILTEROP_NXOR,
        //UBISOFT : FILTERING
        NX_FILTEROP_SWAP_AND
        };

		/**
	Identifies each possible seperating axis for a pair of oriented bounding boxes.

	@see NxSeparatingAxis()
	*/
	enum NxSepAxis
	{
		NX_SEP_AXIS_OVERLAP,

		NX_SEP_AXIS_A0,
		NX_SEP_AXIS_A1,
		NX_SEP_AXIS_A2,

		NX_SEP_AXIS_B0,
		NX_SEP_AXIS_B1,
		NX_SEP_AXIS_B2,

		NX_SEP_AXIS_A0_CROSS_B0,
		NX_SEP_AXIS_A0_CROSS_B1,
		NX_SEP_AXIS_A0_CROSS_B2,

		NX_SEP_AXIS_A1_CROSS_B0,
		NX_SEP_AXIS_A1_CROSS_B1,
		NX_SEP_AXIS_A1_CROSS_B2,

		NX_SEP_AXIS_A2_CROSS_B0,
		NX_SEP_AXIS_A2_CROSS_B1,
		NX_SEP_AXIS_A2_CROSS_B2,

		NX_SEP_AXIS_FORCE_DWORD	= 0x7fffffff
	};

	/**
	Identifies the version of hardware present in the machine.
	*/

	enum NxHWVersion
		{
		NX_HW_VERSION_NONE = 0,
		NX_HW_VERSION_ATHENA_1_0 = 1
		};

	/**
	\brief Describes the format of height field samples.
	@see NxHeightFieldDesc.format NxHeightFieldDesc.samples
	*/
	enum NxHeightFieldFormat
		{
		/**
		\brief Height field height data is 16 bit signed integers, followed by triangle materials. 
		
		Each sample is 32 bits wide arranged as follows:
		
		\image html heightFieldFormat_S16_TM.png

		1) First there is a 16 bit height value.
		2) Next, two one byte material indices, with the high bit of each byte reserved for special use.
		(so the material index is only 7 bits).
		The high bit of material0 is the tess-flag.
		The high bit of material1 is reserved for future use.
		
		There are zero or more unused bytes before the next sample depending on NxHeightFieldDesc.sampleStride, 
		where the application may eventually keep its own data.

		This is the only format supported at the moment.

		<b>Platform:</b>
		\li PC SW: Yes
		\li PPU  : Yes (Software fallback)
		\li PS3  : Yes
		\li XB360: Yes

		@see NxHeightFieldDesc.format NxHeightFieldDesc.samples
		*/
		NX_HF_S16_TM = (1 << 0),
		};

	/** 
	\brief Determines the tessellation of height field cells.
	@see NxHeightFieldDesc.format NxHeightFieldDesc.samples
	*/
	enum NxHeightFieldTessFlag
		{
		/**
		\brief This flag determines which way each quad cell is subdivided.

		The flag lowered indicates subdivision like this: (the 0th vertex is referenced by only one triangle)
		
		\image html heightfieldTriMat2.PNG

		<pre>
		+--+--+--+---> column
		| /| /| /|
		|/ |/ |/ |
		+--+--+--+
		| /| /| /|
		|/ |/ |/ |
		+--+--+--+
		|
		|
		V row
		</pre>
		
		The flag raised indicates subdivision like this: (the 0th vertex is shared by two triangles)
		
		\image html heightfieldTriMat1.PNG

		<pre>
		+--+--+--+---> column
		|\ |\ |\ |
		| \| \| \|
		+--+--+--+
		|\ |\ |\ |
		| \| \| \|
		+--+--+--+
		|
		|
		V row
		</pre>
		
		<b>Platform:</b>
		\li PC SW: Yes
		\li PPU  : Yes (Software fallback)
		\li PS3  : Yes
		\li XB360: Yes

		@see NxHeightFieldDesc.format NxHeightFieldDesc.samples
		*/
		NX_HF_0TH_VERTEX_SHARED = (1 << 0)
		};

	/**
	\brief Enum with flag values to be used in NxHeightFieldDesc.flags.
	*/
	enum NxHeightFieldFlags
		{
		/**
		\brief Disable collisions with height field with boundary edges.
		
		Raise this flag if several terrain patches are going to be placed adjacent to each other, 
		to avoid a bump when sliding across.

		This flag is ignored in contact generation with sphere and capsule shapes.

		<b>Platform:</b>
		\li PC SW: Yes
		\li PPU  : Yes (Software fallback)
		\li PS3  : Yes
		\li XB360: Yes

		@see NxHeightFieldDesc.flags
		*/
		NX_HF_NO_BOUNDARY_EDGES = (1 << 0),
		};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
