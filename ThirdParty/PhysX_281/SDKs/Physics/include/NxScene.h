#ifndef NX_PHYSICS_NX_SCENE
#define NX_PHYSICS_NX_SCENE
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

#include "Nxp.h"
#include "NxUserRaycastReport.h"
#include "NxUserEntityReport.h"
#include "NxSceneQuery.h"
#include "NxSceneDesc.h"
#include "NxSceneStats.h"
#include "NxSceneStats2.h"
#include "NxArray.h"
#include "NxProfiler.h"
#if NX_USE_FLUID_API
#include "fluids/NxFluid.h"
class NxUserFluidContactReport;
#endif

#if NX_USE_CLOTH_API
#include "cloth/NxCloth.h"
#include "cloth/NxClothMesh.h"
#endif

#if NX_USE_SOFTBODY_API
#include "softbody/NxSoftBody.h"
#include "softbody/NxSoftBodyMesh.h"
#endif

class NxActor;
class NxActorDescBase;
class NxJoint;
class NxJointDesc;
class NxEffector;
class NxEffectorDesc;
class NxSpringAndDamperEffector;
class NxSpringAndDamperEffectorDesc;
class NxMaterialDesc;
class NxMaterial;
class NxUserNotify;
class NxUserTriggerReport;
class NxUserContactReport;
class NxThread;
class NxTriangle;
class NxDebugRenderable;
class NxCompartment;
class NxCompartmentDesc;
class NxPhysicsSDK;

class NxForceField;
class NxForceFieldDesc;
class NxForceFieldLinearKernel;
class NxForceFieldLinearKernelDesc;
class NxForceFieldShapeGroup;
class NxForceFieldShapeGroupDesc;

/**
\brief Struct used by NxScene::getPairFlagArray().

The high bit of each 32 bit	flag field denotes whether a pair is a shape or an actor pair.
The flags are defined by the enum NxContactPairFlag.

@see NxScene.getPairFlagArray
*/
class NxPairFlag
	{
	public:
	void*	objects[2];
	NxU32	flags;

	NX_INLINE NxU32 isActorPair() const { return flags & 0x80000000;	}
	};


/**
\brief Enum describing synchronization conditions.
*/
enum NxStandardFences
	{
	NX_FENCE_RUN_FINISHED,
	/*NX_SYNC_RAYCAST_FINISHED,*/
	NX_NUM_STANDARD_FENCES,
	};

/**
 enum to check if a certain part of the SDK has finished.
 used in:
 bool checkResults(NxSimulationStatus, bool block = false)
 bool fetchResults(NxSimulationStatus, bool block = false)

 @see NxScene.checkResults() NxScene.fetchResults()
*/
enum NxSimulationStatus
	{
	/**
	\brief Refers to the primary scene and all compartments having finished, the new results being readable, and everything being writeable.
	*/
	NX_RIGID_BODY_FINISHED	= (1<<0),
	NX_ALL_FINISHED			= (1<<0),	//an alias as the above is misnomer
	/**
	\brief Refers to the primary scene having finished.  The scene will still be locked for writing until you call fetchResults(NX_RIGID_BODY_FINISHED).
	*/
	NX_PRIMARY_FINISHED		= (1<<1),
	};

/**
\brief Polling result for SDK managed threading.

@see NxScene.pollForWork()
*/
enum NxThreadPollResult
	{
	/**
	\brief There is no work to execute at the time the function was called.
	*/
	NX_THREAD_NOWORK				= 0,
	
	/**
	\brief There may be more work waiting for execution.
	*/
	NX_THREAD_MOREWORK				= 1,

	/**
	\brief The function returned because the simulation tick finished.
	*/
	NX_THREAD_SIMULATION_END		= 2,

	/**
	\brief The function returned because the user call shutdownWorkerThreads()

	When the user calls NxScene.shutdownWorkerThreads() the SDK releases all blocked threads.
	Threads which are released return NX_THREAD_SHUTDOWN.	
	*/
	NX_THREAD_SHUTDOWN				= 3,

	NX_THREAD_FORCE_DWORD = 0x7fffffff,
	};

enum NxThreadWait
	{
	/**
	\brief The poll function will return immediately if there is no work available.

	Valid for pollForWork() and pollForBackgroundWork()

	@see NxScene.pollForWork() NxScene.pollForBackgroundWork()
	*/
	NX_WAIT_NONE				= 0,

	/**
	\brief The poll function will wait until the end of the simulation tick for work.

	Valid for pollForWork()

	@see NxScene.pollForWork()
	*/
	NX_WAIT_SIMULATION_END		= 1,

	/**
	\brief Wait until NxScene.shutdownWorkerThreads() is called.

	@see NxScene.shutdownWorkerThreads
	*/
	NX_WAIT_SHUTDOWN			= 2,

	NX_WAIT_FORCE_DWORD = 0x7fffffff,
	};

#if NX_SUPPORT_SWEEP_API
enum NxSweepFlags
	{
	NX_SF_STATICS		= (1<<0),	//!< Sweep vs static objects
	NX_SF_DYNAMICS		= (1<<1),	//!< Sweep vs dynamic objects
	NX_SF_ASYNC			= (1<<2),	//!< Asynchronous sweeps (else synchronous) (Note: Currently disabled)
	NX_SF_ALL_HITS		= (1<<3),	//!< Reports all hits rather than just closest hit

	NX_SF_DEBUG_SM		= (1<<5),	//!< DEBUG - temp - don't use
	NX_SF_DEBUG_ET		= (1<<6),	//!< DEBUG - temp - don't use
	};

/**
 \brief Struct used with the Sweep API.
 Related methods: NxScene::linearOBBSweep(), NxScene::linearCapsuleSweep(), 
 NxActor::linearSweep().

 @see NxScene, NxActor
 */
struct NxSweepQueryHit
	{
	NxF32		t;					//!< Distance to hit expressed as a percentage of the source motion vector ([0,1] coeff)
	NxShape*	hitShape;			//!< Hit shape
	NxShape*	sweepShape;			//!< Only nonzero when using NxActor::linearSweep. Shape from NxActor that hits the hitShape.
	void*		userData;			//!< User-defined data
	NxU32		internalFaceID;		//!< ID of touched triangle (internal)
	NxU32		faceID;				//!< ID of touched triangle (external)
	NxVec3		point;				//!< World-space impact point
	NxVec3		normal;				//!< World-space impact normal
	};

	class NxSweepCache;
#endif


/**
 \brief Data struct for use with Active Transform Notification.
 Used with NxScene::getActiveTransforms().
 
 @see NxScene
*/
struct NxActiveTransform
{
	NxActor*	actor;				//!< Affected actor
	void*		userData;			//!< User data of the actor
	NxMat34		actor2World;		//!< Actor-to-world transform of the actor
};

	
/** 
\brief Names for a number of profile zones that should always be available.  Can be used with NxProfileData::getNamedZone()

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No
*/
enum NxProfileZoneName
	{
	NX_PZ_CLIENT_FRAME,		//!< Clock start is in client thread, just before scene thread was kicked off; clock end is when client calls fetchResults().
	NX_PZ_CPU_SIMULATE,		//!< Maximum time of simulation thread ((Nf)Scene::simulate(NxU32)), over all CPU scenes inside the NxScene.
	NX_PZ_PPU0_SIMULATE,	//!< Maximum time of simulation thread over all PPU# scenes inside the NxScene.
	NX_PZ_PPU1_SIMULATE,
	NX_PZ_PPU2_SIMULATE,
	NX_PZ_PPU3_SIMULATE,
	NX_PZ_TOTAL_SIMULATION = 0x10,	//!< Clock start is in client thread, just before scene thread was kicked off; clock end is in simulation thread when it finishes.
	};


/**
\brief Array of profiling data. 

 profileZones points to an array of numZones profile zones.  Zones are sorted such that the parent zones always come before their children.  
 Some zones have multiple parents (code called from multiple places) in which case only the relationship to the first parent is displayed.
 returned by NxScene::readProfileData().
*/
class NxProfileData : public NxProfilerData
	{
	protected:
	NX_INLINE					NxProfileData() {}
	virtual						~NxProfileData(){}

	public:
	/**
	\brief Returns some named profile zones.  
	
	Note: though copies of these named zones do appear in the above array, this function may 
	return pointers to objects not included above.  May return NULL if the zone is not available.
	*/
	virtual const NxProfileZone * getNamedZone(NxProfileZoneName) const = 0;
	};

/** 
 \brief A scene is a collection of bodies, constraints, and effectors which can interact.

 The scene simulates the behavior of these objects over time. Several scenes may exist 
 at the same time, but each body, constraint, or effector object is specific to a scene 
 -- they may not be shared.

 For example, attempting to create a joint in one scene and then using it to attach
 bodies from a different scene results in undefined behavior.

 <h3>Creation</h3>

 Example:

 \include NxScene_Create.cpp

 @see NxSceneDesc NxPhysicsSDK.createScene() NxPhysicsSDK.releaseScene()
*/
class NxScene
	{
	protected:
										NxScene(): userData(0), extLink(0)	{}
	virtual								~NxScene()	{}

	public:


	/**
	\brief Saves the Scene descriptor.

	\param[out] desc The descriptor used to retrieve the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneDesc
	*/
	virtual	bool				saveToDesc(NxSceneDesc& desc)	const	= 0;

	/**
	\brief Get the scene flags.

	\return The scene flags. See #NxSceneFlags

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneFlags
	*/
	virtual		NxU32		getFlags() const = 0;

	/**
	\brief Get the simulation type.

	\return The simulation type. See #NxSimulationType

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: No

	@see NxSceneDesc.simType
	*/
	virtual		NxSimulationType		getSimType() const = 0;


	/**
	\brief Gets a private interface to an internal debug object.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void *						getInternal(void) = 0;

	/**
	\brief Sets a constant gravity for the entire scene.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] vec A new gravity vector(e.g. NxVec3(0.0f,-9.8f,0.0f) ) <b>Range:</b> force vector

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneDesc.gravity getGravity()
	*/
	virtual void						setGravity(const NxVec3& vec) = 0;

	/**
	\brief Retrieves the current gravity setting.

	\param[out] vec Used to retrieve the current gravity for the scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGravity() NxSceneDesc.gravity
	*/
	virtual void						getGravity(NxVec3& vec) = 0;

/************************************************************************************************/

/** @name Create/Release Objects
*/
//@{

	/**
	\brief Creates an actor in this scene.
	
	NxActorDesc::isValid() must return true.

	<b>Sleeping:</b> This call wakes the actors if they are sleeping.

	\param[in] desc Descriptor for actor to create. See #NxActorDescBase
	\return The new actor.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Limits on numbers and types of actors)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActor NxActorDesc NxActorDescBase releaseActor()
	*/
	virtual NxActor*					createActor(const NxActorDescBase& desc) = 0;

	/**
	\brief Deletes the specified actor.
	
	Also releases any body and/or shapes associated with the actor.

	The actor must be in this scene.
	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	Note: deleting a static actor will not wake up any sleeping objects that were
	sitting on it. Use a kinematic actor instead to get this behavior.

	Releasing an actor will affect any joints that are connected to the actor.
	Such joints will be moved to a list of "dead joints" and automatically deleted upon 
	scene deletion, or explicitly by the user by calling NxScene::releaseJoint(). It is
	recommended to always remove all joints that reference actors before the actors
	themselves are removed. It is not possible to retrieve the list of dead joints.

	<b>Sleeping:</b> This call will awaken any sleeping actors contacting the deleted actor (directly or indirectly).

	\param[in] actor The actor to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createActor() NxActor
	*/
	virtual void						releaseActor(NxActor& actor) = 0;

	/**
	\brief Creates a joint.
	
	The joint type depends on the type of joint desc passed in.

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] jointDesc The descriptor for the joint to create. E.g. #NxSphericalJointDesc,#NxRevoluteJointDesc etc
	\return The new joint.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Up to 64k per scene)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxJoint NxJointDesc releaseJoint() NxJoint
	@see NxRevoluteJoint NxSphericalJoint NxPrismaticJoint NxCylindricalJoint NxD6Joint NxDistanceJoint
	NxFixedJoint NxPointInPlaneJoint NxPointOnLineJoint
	*/
	virtual NxJoint *					createJoint(const NxJointDesc &jointDesc) = 0;

	/**
	\brief Deletes the specified joint.
	
	The joint must be in this scene.
	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	<b>Sleeping:</b> This call wakes the actor(s) if they are sleeping.

	\param[in] joint The joint to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						releaseJoint(NxJoint &joint) = 0;

	/**
	\brief Deprecated.  Use createEffector() instead.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] springDesc The descriptor for the spring and damper effector to create. See #NxSpringAndDamperEffectorDesc.
	\return The new spring and damper.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffectorDesc NxSpringAndDamperEffector releaseEffector()
	*/
	virtual NxSpringAndDamperEffector*	createSpringAndDamperEffector(const NxSpringAndDamperEffectorDesc& springDesc) = 0;

	/**
	\brief Creates an effector.

	\param[in] desc The descriptor for the effector to create. See #NxEffectorDesc.
	\return The new effector.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSpringAndDamperEffectorDesc NxSpringAndDamperEffector releaseEffector()
	*/
	virtual NxEffector*	createEffector(const NxEffectorDesc& desc) = 0;

	/**
	\brief Deletes the effector passed.

	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] effector The effector to delete.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createSpringAndDamperEffector NxSpringAndDamperEffector
	*/
	virtual void						releaseEffector(NxEffector& effector) = 0;

	/**
	\brief Creates a force field.

	\param[in] forceFieldDesc The descriptor for the force field to create. See #NxForceFieldDesc.
	\return The new force field.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see releaseForceField NxForceField NxForceFieldDesc
	*/
	virtual NxForceField*				createForceField(const NxForceFieldDesc& forceFieldDesc) = 0;

	/**
	\brief Deletes the force field passed.

	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param[in] forceField The effector to delete.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes

	@see createForceField NxForceField
	*/
		virtual void						releaseForceField(NxForceField& forceField) = 0;

	/**
	\brief Gets the number of force fields in the scene.

	\return The force field count.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxU32					getNbForceFields()		const	= 0;

	/**
	\brief Gets the force fields in the scene.

	\return Array of pointers to NxForceField objects. Use #getNbForceFields to find the size of the array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxForceField**			getForceFields()				= 0;

	/**
	\brief creates a forcefield kernel which uses the same linear function as pre 2.8 force fields

	\param[in] kernelDesc The linear kernel desc to use to create a linear kernel for force fields. See #NxForceFieldLinearKernelDesc.
	\return NxForceFieldLinearKernel. See #NxForceFieldLinearKernel

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxForceFieldLinearKernel*	createForceFieldLinearKernel(const NxForceFieldLinearKernelDesc& kernelDesc)	= 0;

	/**
	\brief releases a linear force field kernel
	\param[in] kernel to be released.
	\return NxForceFieldLinearKernel. See #NxForceFieldLinearKernel

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void						releaseForceFieldLinearKernel(NxForceFieldLinearKernel& kernel)							= 0;

	/**
	\brief Returns the number of linear kernels in the scene. 

	\return number of linear kernels in the scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32						getNbForceFieldLinearKernels() const													= 0; 

	/**
	\brief Restarts the linear kernels iterator so that the next call to getNextForceFieldLinearKernel(). 

	\return The first shape group in the force scene.  

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						resetForceFieldLinearKernelsIterator()													= 0; 

	/**
	\brief Retrieves the next linear kernel when iterating. 

	\return NxForceFieldLinearKernel  

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldLinearKernel*	getNextForceFieldLinearKernel()															= 0; 

	/**
	\brief Creates a new force field shape group.  

	\param[in] desc The force field group descriptor. See #NxForceFieldShapeGroupDesc.
	\return NxForceFieldShapeGroup. See #NxForceFieldShapeGroup

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldShapeGroup*		createForceFieldShapeGroup(const NxForceFieldShapeGroupDesc& desc)						= 0;
	
	/**
	\brief Releases a force field shape group.

	\param[in] group The group which is to be relased. See #NxForceFieldShapeGroup.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						releaseForceFieldShapeGroup(NxForceFieldShapeGroup& group)									= 0;

	/**
	\brief Returns the number of shape groups in the scene.

	\return The number of shape groups in the scene.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxU32						getNbForceFieldShapeGroups() const														= 0; 

	/**
	\brief Restarts the shape groups iterator so that the next call to getNextForceFieldShapeGroup() returns the first shape group in the force scene. 
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						resetForceFieldShapeGroupsIterator()													= 0; 

	/**
	\brief Retrieves the next shape group when iterating.
	\return NxForceFieldShapeGroup. See #NxForceFieldShapeGroup

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldShapeGroup*		getNextForceFieldShapeGroup()															= 0; 

	/**
	\brief Creates a new variety index for force fields to access the scaling table, creates a new row in the scaling table.
	\return NxForceFieldVariety. See #NxForceFieldVariety & #setForceFieldScale 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldVariety			createForceFieldVariety()																= 0;
	
	/**
	\brief Returns the highest allocated force field variety.
	\return Highest variety index See #NxForceFieldVariety

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldVariety			getHighestForceFieldVariety() const														= 0;

	/**
	\brief Releases a forcefield variety index and the related row in the scaling table.
	\param[in] mat The variery index to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						releaseForceFieldVariety(NxForceFieldVariety var)										= 0;

	/**
	\brief Creates a new index for objects(actor, fluid, cloth, softbody) to access the scaling table, creates a new column in the scaling table.
	\return NxForceFieldMaterial See #NxForceFieldMaterial

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldMaterial		createForceFieldMaterial()																= 0;

	/**
	\brief Returns the highest allocated force field material.
	\return The highest allocated force field material. See #NxForceFieldMaterial

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldMaterial		getHighestForceFieldMaterial() const													= 0;

	/**
	\brief Releases a forcefield material index and the related column in the scaling table.
	\param[in] mat The material index to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						releaseForceFieldMaterial(NxForceFieldMaterial mat)										= 0;

	/**
	\brief Get the scaling value for a given variety/material pair.
	\return The scaling value for a given variety/material pair.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	NxReal						getForceFieldScale(NxForceFieldVariety var, NxForceFieldMaterial mat)					= 0;

	/**
	\brief Set the scaling value for a given variety/material pair.
	\param[in] var A Variety index.
	\param[in] mat A Material index.
	\param[in] val The value to set at the variety/material coordinate in the table. Setting the value to big or to low may cause invalid floats in the kernel output.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void						setForceFieldScale(NxForceFieldVariety var, NxForceFieldMaterial mat, NxReal val)		= 0;

	/**
	\brief Creates a new NxMaterial.

	The material library consists of an array of material objects. Each
	material has a well defined index that can be used to refer to it.
	If an object (shape or triangle) references an undefined material,
	the default material with index 0 is used instead.

	\param[in] matDesc The material desc to use to create a material. See #NxMaterialDesc.
	\return The new material.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterial NxMaterialDesc releaseMaterial()
	*/
	virtual NxMaterial *				createMaterial(const NxMaterialDesc &matDesc) = 0;

	/**
	\brief Deletes the specified material.
	
	The material must be in this scene.
	Do not keep a reference to the deleted instance.
	If you release a material while shapes or meshes are referencing its material index,
	the material assignment of these objects becomes undefined.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param[in] material The material to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createMaterial() NxMaterial
	*/
	virtual void						releaseMaterial(NxMaterial &material) = 0;


	/**
	\brief Creates a scene compartment.

	A scene compartment is a portion of the scene that can
	be simulated on a different hardware device than other parts of the scene.
	See also the User's Guide on Compartments.

	\param[in] compDesc The NxCompartment descriptor to use to create a compartment.
	\return the new compartment.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartment
	*/
	virtual NxCompartment *		createCompartment(const NxCompartmentDesc &compDesc) = 0;

	/**
	\brief Returns the number of compartments created in the scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartment, getCompartmentArray()
	*/
	virtual NxU32		getNbCompartments() const = 0;

	/**
	\brief Writes the scene's array of NxCompartment pointers to a user buffer.

	bufferSize is the number of pointers (not bytes) that the buffer can hold.
	usersIterator is an iterator that the user should initialize to 0 to start copying the array from the beginning.
	Once the first call succeeds, the SDK will have changed the value of the iterator (in some data structure specific way) such that the
	next get*Array() call will return the next batch of values.  This way a large internal array can be read out with several calls into 
	a smaller user side buffer.

	Returns the number of pointers written, this should be less or equal to bufferSize.

	The ordering of the compartments in the array is not specified.

	\param[out] userBuffer The buffer to receive compartment pointers.
	\param[in] bufferSize The number of compartment pointers which can be stored in the buffer.
	\param[in,out] usersIterator Cookie used to continue iteration from the last position.
	\return The number of compartment pointers written to userBuffer.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbCompartments() NxCompartment
	*/
	virtual NxU32		getCompartmentArray(NxCompartment ** userBuffer, NxU32 bufferSize, NxU32 & usersIterator) const = 0;
//@}
/************************************************************************************************/

/** @name Collision Filtering and Grouping
*/
//@{

	/**
	\brief Sets the pair flags for the given pair of actors.

	The pair flags are a combination of bits
	defined by ::NxContactPairFlag. Calling this on an actor that has no shape(s) has no effect.
	The two actor references must not reference the same actor.

	It is important to note that the SDK stores pair flags per shape, even for actor pair flags. 
	This means that shapes should be created before actor pair flags are set, otherwise the pair
	flags will be ignored.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actors up automatically.

	\param[in] actorA Actor A
	\param[in] actorB Actor B
	\param[in] nxContactPairFlag New set of contact pair flags. See #NxContactPairFlag

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getActorPairFlags() NxContactPairFlag
	*/
	virtual void						setActorPairFlags(NxActor& actorA, NxActor& actorB, NxU32 nxContactPairFlag) = 0;

	/**
	\brief Retrieves the pair flags for the given pair of actors.
	
	The pair flags are a combination of bits
	defined by ::NxContactPairFlag. If no pair record is found, zero is returned.
	The two actor references must not reference the same actor.

	\param[in] actorA Actor A
	\param[in] actorB Actor B
	\return The actor pair flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see setActorPairFlags() NxContactPairFlag
	*/
	virtual NxU32						getActorPairFlags(NxActor& actorA, NxActor& actorB) const = 0;

	/**
	\brief Similar to #setActorPairFlags(), but for a pair of shapes.
	
	NX_IGNORE_PAIR is the only thing allowed
	as a shape pair flag.  All of the NX_NOTIFY flags should be used at the actor level.
	The two shape references must not reference the same shape.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actors up automatically.

	\param[in] shapeA Shape A
	\param[in] shapeB Shape B
	\param[in] nxContactPairFlag New set of shape contact pair flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getShapePairFlags() NxContactPairFlag
	*/
	virtual	void						setShapePairFlags(NxShape& shapeA, NxShape& shapeB, NxU32 nxContactPairFlag) = 0;

	/**
	\brief Similar to #getActorPairFlags(), but for a pair of shapes.

	The two shape references must not reference the same shape.

	\param[in] shapeA Shape A
	\param[in] shapeB SHape B
	\return The shape pair flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setShapePairFlags() NxContactPairFlag
	*/
	virtual	NxU32						getShapePairFlags(NxShape& shapeA, NxShape& shapeB) const = 0;

	/**
	\brief Returns the number of pairs for which pairFlags are defined. 
	Note that this includes compartments.

	This includes actor and shape pairs.

	\return The number of pairs.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxContactPairFlag setShapePairFlags() setActorPairFlags() getPairFlagArray()
	*/
	virtual NxU32						getNbPairs() const = 0;

	/**
	\brief Retrieves the pair flag data.
	
	The high bit of each 32 bit flag field denotes whether a pair is a shape or
	an actor pair. numPairs is the number of pairs the buffer can hold. The user is responsible for
	allocating and deallocating the buffer. Call ::getNbPairs() to check what the number of pairs should be.
	

	\param[out] userArray Pointer to user array to receive pair flags. should be at least sizeof(NxPairFlag)*numPairs in size.
	\param[in] numPairs Number of pairs the user buffer can hold.
	\return The number of pairs written to userArray.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxContactPairFlag setShapePairFlags() setActorPairFlags() getNbPairs()
	*/
	virtual NxU32						getPairFlagArray(NxPairFlag* userArray, NxU32 numPairs) const = 0;

	/**
	\brief Specifies if collision should be performed by a pair of shape groups.
	
	It is possible to assign each shape to a collision groups using #NxShape::setGroup().
	With this method one can set whether collisions should be detected between shapes 
	belonging to a given pair of groups. Initially all pairs are enabled.

	Collision detection between two shapes a and b occurs if: 
	getGroupCollisionFlag(a->getGroup(), b->getGroup()) && isEnabledPair(a,b) is true.

	Fluids can be assigned to collision groups as well.

	NxCollisionGroup is an integer between 0 and 31.

	<b>Sleeping:</b> Does <b>NOT</b> wake the associated actors up automatically.

	\param[in] group1 First group.
	\param[in] group2 Second group.
	\param[in] enable True to enable collision between the groups.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCollisionGroup getGroupCollisionFlag() NxShape.setGroup() NxShape.getGroup()
	*/
	virtual void setGroupCollisionFlag(NxCollisionGroup group1, NxCollisionGroup group2, bool enable) = 0;

	/**
	\brief Determines if collision detection is performed between a pair of groups.

	NxCollisionGroup is an integer between 0 and 31.

	\param[in] group1 First Group.
	\param[in] group2 Second Group.
	\return True if the groups could collide.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroupCollisionFlag() NxCollisionGroup NxShape.setGroup() NxShape.getGroup()
	*/
	virtual bool getGroupCollisionFlag(NxCollisionGroup group1, NxCollisionGroup group2) const = 0;
	
	/**
	\brief Specifies the dominance behavior of constraints between two actors with two certain dominance groups.
	
	It is possible to assign each actor to a dominance groups using #NxActor::setDominanceGroup().

	With dominance groups one can have all constraints (contacts and joints) created 
	between actors act in one direction only. This is useful if you want to make sure that the movement of the rider
	of a vehicle or the pony tail of a character doesn't influence the object it is attached to, while keeping the motion of 
	both inherently physical.  
	
	Whenever a constraint (i.e. joint or contact) between two actors (a0, a1) needs to be solved, the groups (g0, g1) of both
	actors are retrieved.  Then the NxConstraintDominance setting for this group pair is retrieved with getDominanceGroupPair(g0, g1).  
	
	In the constraint, NxConstraintDominance::dominance0 becomes the dominance setting for a0, and 
	NxConstraintDominance::dominance1 becomes the dominance setting for a1.  A dominanceN setting of 1.0f, the default, 
	will permit aN to be pushed or pulled by a(1-N) through the constraint.  A dominanceN setting of 0.0f, will however 
	prevent aN to be pushed or pulled by a(1-N) through the constraint.  Thus, a NxConstraintDominance of (1.0f, 0.0f) makes 
	the interaction one-way.
	
	
	The matrix sampled by getDominanceGroupPair(g1, g2) is initialised by default such that:
	
	if g1 == g2, then (1.0f, 1.0f) is returned
	if g1 <  g2, then (0.0f, 1.0f) is returned
	if g1 >  g2, then (1.0f, 0.0f) is returned
	
	In other words, we permit actors in higher groups to be pushed around by actors in lower groups by default.
		
	These settings should cover most applications, and in fact not overriding these settings may likely result in higher performance.
	
	It is not possible to make the matrix asymetric, or to change the diagonal.  In other words: 
	
	* it is not possible to change (g1, g2) if (g1==g2)	
	* if you set 
	
	(g1, g2) to X, then (g2, g1) will implicitly and automatically be set to ~X, where:
	
	~(1.0f, 1.0f) is (1.0f, 1.0f)
	~(0.0f, 1.0f) is (1.0f, 0.0f)
	~(1.0f, 0.0f) is (0.0f, 1.0f)
	
	These two restrictions are to make sure that constraints between two actors will always evaluate to the same dominance
	setting, regardless of which order the actors are passed to the constraint.
	
	Dominance settings are currently specified as floats 0.0f or 1.0f because in the future we may permit arbitrary 
	fractional settings to express 'partly-one-way' interactions.
		
	<b>Sleeping:</b> Does <b>NOT</b> wake actors up automatically.

	@see getDominanceGroupPair() NxDominanceGroup NxConstraintDominance NxActor::setDominanceGroup() NxActor::getDominanceGroup()
	*/
	virtual void setDominanceGroupPair(NxDominanceGroup group1, NxDominanceGroup group2, NxConstraintDominance & dominance) = 0;

	/**
	\brief Samples the dominance matrix.

	@see setDominanceGroupPair() NxDominanceGroup NxConstraintDominance NxActor::setDominanceGroup() NxActor::getDominanceGroup()
	*/
	virtual NxConstraintDominance getDominanceGroupPair(NxDominanceGroup group1, NxDominanceGroup group2) const = 0;
	
	/**
	\brief With this method one can set contact reporting flags between actors belonging to a pair of groups.

	It is possible to assign each actor to a group using NxActor::setGroup(). This is a different
	set of groups from the shape groups despite the similar name. Here up to 0xffff different groups are permitted,
	With this method one can set contact reporting flags between actors belonging to a pair of groups.

	The following flags are permitted:

	NX_NOTIFY_ON_START_TOUCH
	NX_NOTIFY_ON_END_TOUCH	
	NX_NOTIFY_ON_TOUCH
	NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD
	NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD
	NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD
	NX_NOTIFY_ON_IMPACT		
	NX_NOTIFY_ON_ROLL		
	NX_NOTIFY_ON_SLIDE	

	See ::NxContactPairFlag.

	Note that finer grain control of pairwise flags is possible using the functions
	NxScene::setShapePairFlags() and NxScene::setActorPairFlags().

	<b>Sleeping:</b> Does <b>NOT</b> wake the actors up automatically.

	\param[in] group1 First group.
	\param[in] group2 Second group
	\param[in] flags Flags to control contact reporting. See #NxContactPairFlag.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getActorGroupPairFlags() NxActorGroup NxActor.getGroup() NxActor.setGroup()
	*/
	virtual void setActorGroupPairFlags(NxActorGroup group1, NxActorGroup group2, NxU32 flags) = 0;

	/**
	\brief This reads the value set with #setActorGroupPairFlags.

	\param[in] group1 First Group
	\param[in] group2 Second Group
	\return The contact reporting flags associated with this actor pair. See #setActorGroupPairFlags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setActorPairFlags() NxActorGroup NxActor.getGroup() NxActor.setGroup()
	*/
	virtual NxU32 getActorGroupPairFlags(NxActorGroup group1, NxActorGroup group2) const = 0;

	/**
	\brief Gets the number of actor group flags (as set by setActorGroupPairFlags).

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see NxActorGroupPair, getActorGroupPairFlags(), getActorGroupPairArray()
	*/
	virtual NxU32 getNbActorGroupPairs() const = 0;

	/**
	\brief Writes the scene's array of actor group flags (as set by setActorGroupPairFlags) to a user buffer.

	bufferSize is the number of NxActorGroupPairs (not bytes) that the buffer can hold.
	usersIterator is an iterator that the user should initialize to 0 to start copying the array from the beginning.
	Once the first call succeeds, the SDK will have changed the value of the iterator (in some data structure specific way) such that the
	next get*Array() call will return the next batch of values.  This way a large internal array can be read out with several calls into 
	a smaller user side buffer.

	\return the number of pairs written, this should be less or equal to bufferSize.

	The ordering of the elements in the array is not specified.

	\param[out] userBuffer The buffer to receive NxActorGroupPairs.
	\param[in] bufferSize The number of NxActorGroupPairs which can be stored in the buffer.
	\param[in,out] userIterator Cookie used to continue iteration from the last position.
	\return The number of pairs written to userBuffer.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbActorGroupPairs() NxActorGroupPair
	*/
	virtual	NxU32 getActorGroupPairArray(NxActorGroupPair * userBuffer, NxU32 bufferSize, NxU32 & userIterator) const = 0;


	/**
	\brief Setups filtering operations. See comments for ::NxGroupsMask

	<b>Sleeping:</b> Does <b>NOT</b> wake the actors up automatically.

	\param[in] op0 Filter op 0.
	\param[in] op1 Filter op 1.
	\param[in] op2 Filter op 2.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterBool() setFilterConstant0() setFilterConstant1()
	*/
	virtual	void	setFilterOps(NxFilterOp op0, NxFilterOp op1, NxFilterOp op2)	= 0;

	/**
	\brief Setups filtering's boolean value. See comments for ::NxGroupsMask

	<b>Sleeping:</b> Does <b>NOT</b> wake the actors up automatically.

	\param[in] flag Boolean value for filter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterOps() setFilterConstant0() setFilterConstant1()
	*/
	virtual	void	setFilterBool(bool flag)										= 0;

	/**
	\brief Setups filtering's K0 value. See comments for ::NxGroupsMask

	<b>Sleeping:</b> Does <b>NOT</b> wake the actors up automatically.

	\param[in] mask The new group mask. See #NxGroupsMask.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterOps() setFilterBool() setFilterConstant1()
	*/
	virtual	void	setFilterConstant0(const NxGroupsMask& mask)					= 0;

	/**
	\brief Setups filtering's K1 value. See comments for ::NxGroupsMask

	<b>Sleeping:</b> Does <b>NOT</b> wake the actors up automatically.

	\param[in] mask The new group mask. See #NxGroupsMask.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterOps() setFilterBool() setFilterConstant0()
	*/
	virtual	void	setFilterConstant1(const NxGroupsMask& mask)					= 0;

	/**
	\brief Retrieves filtering operation. See comments for ::NxGroupsMask

	\param[out] op0 First filter operator.
	\param[out] op1 Second filter operator.
	\param[out] op2 Third filter operator.

	See the user guide page "Contact Filtering" for more details.	

	\return the filter operation requested

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterOps() setFilterBool() setFilterConstant0() setFilterConstant1()
	*/
	virtual	void	getFilterOps(NxFilterOp& op0, NxFilterOp& op1, NxFilterOp& op2)const	= 0;

	/**
	\brief Retrieves filtering's boolean value. See comments for ::NxGroupsMask

	\return flag Boolean value for filter.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterBool() setFilterConstant0() setFilterConstant1()
	*/
	virtual	bool 						getFilterBool() const						= 0;

	/**
	\brief Gets filtering constant K0. See comments for ::NxGroupsMask

	\return the filtering constant, as a mask. See #NxGroupsMask.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterOps() setFilterBool() setFilterConstant0() setFilterConstant1() getFilterConstant1()
	*/
	virtual	NxGroupsMask				getFilterConstant0() const			= 0;

	/**
	\brief Gets filtering constant K1. See comments for ::NxGroupsMask

	\return the filtering constant, as a mask. See #NxGroupsMask.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setFilterOps() setFilterBool() setFilterConstant0() setFilterConstant1() getFilterConstant0()
	*/
	virtual	NxGroupsMask				getFilterConstant1() const			= 0;

//@}
/************************************************************************************************/

/** @name Enumeration
*/
//@{

	/**
	\brief Retrieve the number of actors in the scene.
	\return the number of actors.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getActors()
	*/
	virtual	NxU32						getNbActors()		const	= 0;

	/**
	\brief Retrieve an array of all the actors in the scene.

	\return an array of actor pointers with size getNbActors().

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbActors()
	*/
	virtual	NxActor**					getActors()					= 0;

	/**
	\brief Queries the NxScene for a list of the NxActors whose transforms have been 
	updated during the previous simulation step

	Note: NX_SF_ENABLE_ACTIVETRANSFORMS must be set.

	\param[out] nbTransformsOut The number of transforms returned.

	\return A pointer to the list of NxActiveTransforms generated during the last call to fetchResults().

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxActiveTransform
	*/

	virtual NxActiveTransform*			getActiveTransforms(NxU32 &nbTransformsOut) = 0;

	/**
	\brief Returns the number of static shapes in the scene.
	Note that this includes compartments and mirrored shapes in compartments.

	\return The number of static shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbDynamicShapes();
	@see getTotalNbShapes()
	*/
	virtual	NxU32						getNbStaticShapes()		const	= 0;

	/**
	\brief Returns the number of dynamic shapes in the scene.
	Note that this includes compartments and mirrored shapes in compartments.

	\return the number of dynamic shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbStaticShapes()
	@see getTotalNbShapes()
	*/
	virtual	NxU32						getNbDynamicShapes()	const	= 0;

	/**
	\brief Returns the total number of shapes in the scene, including compounds' sub-shapes.
	Note that this also includes compartments and mirrored shapes in compartments.

	\return the total number of shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbStaticShapes()
	@see getNbDynamicShapes()
	*/
	virtual	NxU32						getTotalNbShapes()	const	= 0;

	/**
	\brief Returns the number of joints in the scene (excluding "dead" joints).
	Note that this includes compartments.

	\return the number of joints in this scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNextJoint()
	*/
	virtual NxU32						getNbJoints()		const	= 0;

	/**

	\brief Restarts the joint iterator so that the next call to ::getNextJoint() returns the first joint in the scene.

	Creating or deleting joints resets the joint iterator.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see resetJointIterator() getNbJoints()
	*/
	virtual void						resetJointIterator()	 = 0;

	/**
	\brief Retrieves the next joint when iterating.
	
	First call resetJointIterator(), then call this method repeatedly until it returns
	zero. After a call to resetJointIterator(), repeated calls to getNextJoint() should return a sequence of getNbJoints()
	joint pointers. The getNbJoints()+1th call will return 0.
	Creating or deleting joints resets the joint iterator.

	@see resetJointIterator.

	\return The next joint in the iteration sequence. Or NULL when the end of the list of joints is reached.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see resetJointIterator() getNbJoints()
	*/
	virtual NxJoint *					getNextJoint()	 = 0;

	/**
	\brief Returns the number of effectors in the scene.

	\return the number of effectors in this scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNextEffector()
	*/
	virtual NxU32						getNbEffectors()		const	= 0;

	/**
    \brief Restarts the effector iterator so that the next call to ::getNextEffector() returns the first effector in the scene.
	
	Creating or deleting effectors resets the joint iterator.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNextEffector() getNbEffectors()
	*/
	virtual void						resetEffectorIterator()	 = 0;

	/**
	\brief Retrieves the next effector when iterating through the effectors in the scene.
	
	First call resetEffectorIterator(), then call this method repeatedly until it returns
	zero. After a call to resetEffectorIterator(), repeated calls to getNextEffector() should return a sequence of getNbEffectors()
	effector pointers. The getNbEffectors()+1th call will return 0.
	Creating or deleting effectors resets the joint iterator.

	@see resetEffectorIterator.

	\return The next effector in the iteration sequence. Or NULL when the end of the list of joints is reached.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see resetEffectorIterator() getNbEffectors()
	*/
	virtual NxEffector *				getNextEffector() = 0;


	/**
	\brief Returns an upper bound for the number of actors in the collision island of a certain actor

    A collision island is a group of objects that are connected through bounds overlaps or joint constraints.

	\param[in] actor The actor for which to return island information

	\return The maximum size of the internal actor array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getIslandArrayFromActor()
	*/
	virtual NxU32 getBoundForIslandSize(NxActor& actor) = 0;

	/**
	\brief Writes pointers to the actors making up the collision island of a certain actor to a user buffer

	A collision island is a group of objects that are connected through bounds overlaps or joint constraints.

	bufferSize is the number of pointers (not bytes) that the buffer can hold.
	usersIterator is an iterator that the user should initialize to 0 to start copying the array from the beginning.
	Once the first call succeeds, the SDK will have changed the value of the iterator (in some data structure specific way) such that the
	next get*Array() call will return the next batch of values.  This way a large internal array can be read out with several calls into 
	a smaller user side buffer.

	Returns the number of pointers written, this should be less or equal to bufferSize.  This will include the specified actor.

	The ordering of the actors in the array is not specified.
	Note that this call is invalid while the scene is simulating (in between simulate() and fetchResults() calls).
	Also, the island is based on the state just before the last simulation step, which means the islands may be somewhat inaccurate, especially for fast moving objects.

	Usage example:
	\code
	NxActor * ptrs[3];
	NxU32 iterator = 0;
	NxU32 actorCount;
	while (actorCount = s->getIslandArrayFromActor(actor, ptrs, 3, iterator))
		while(actorCount--) processActor(ptrs[actorCount]);

	\endcode

	\param[in] actor The actor for which to return island information
	\param[out] userBuffer The buffer to receive actor pointers.
	\param[in] bufferSize The number of actor pointers which can be stored in the buffer.
	\param[in,out] userIterator Cookie used to continue iteration from the last position.
	\return The number of actor pointers written to userBuffer.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getBoundForIslandSize() NxActor
	*/
	virtual NxU32 getIslandArrayFromActor(NxActor& actor, NxActor** userBuffer, NxU32 bufferSize, NxU32& userIterator) = 0;

//@}
/************************************************************************************************/

/** @name Materials
*/
//@{

	/**
	\brief Return the number of materials in the scene.

	Note that the returned value is not related to material indices (NxMaterialIndex).
	Those may not be allocated continuously, and its values may be higher than getNbMaterials().
	This will also include the default material which exists without having to be created.

	\return The size of the internal material array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getMaterialArray()
	*/
	virtual NxU32						getNbMaterials() const = 0;

	/**
	\brief Writes the scene's array of material pointers to a user buffer.
	
	bufferSize is the number of pointers (not bytes) that the buffer can hold.
	usersIterator is an iterator that the user should initialize to 0 to start copying the array from the beginning.
	Once the first call succeeds, the SDK will have changed the value of the iterator (in some data structure specific way) such that the
	next get*Array() call will return the next batch of values.  This way a large internal array can be read out with several calls into 
	a smaller user side buffer.
	
	Returns the number of pointers written, this should be less or equal to bufferSize.  This will also return the default material which exists without having to be created.

	The ordering of the materials in the array is not specified.

	Usage example:
	\code
	NxMaterial * ptrs[3];
	NxU32 iterator = 0;
	NxU32 materialCount;
	while (materialCount = s->getMaterialArray(ptrs, 3, iterator))
		while(materialCount--) processMaterial(ptrs[materialCount]);

	\endcode

	\param[out] userBuffer The buffer to receive material pointers.
	\param[in] bufferSize The number of material pointers which can be stored in the buffer.
	\param[in,out] usersIterator Cookie used to continue iteration from the last position.
	\return The number of material pointers written to userBuffer.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbMaterials() NxMaterial
	*/
	virtual	NxU32						getMaterialArray(NxMaterial ** userBuffer, NxU32 bufferSize, NxU32 & usersIterator) = 0;

	/**
	\brief Returns current highest valid material index.
	
	Note that not all indices below this are valid if some of them belong to meshes that
	have been freed.

	\return The highest material index.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterial NxMaterialIndex NxScene.createMaterial()
	*/

	virtual NxMaterialIndex				getHighestMaterialIndex() const = 0;

	/**
	\brief Retrieves the material with the given material index.
	
	There is always at least one material in the Scene, the default material (index 0). If the
	specified material index is out of range (larger than getHighestMaterialIndex) or belongs
	to a material that has been released, then the default material is returned, but no error
	is reported. 
	
	You can always get a pointer to the default material by specifying index 0. You can change
	the properties of the default material by changing the properties directly on the material.
	It is not possible to release the default material, calling releaseMaterial(defaultMaterial)
	has no effect.

	\param[in] matIndex Material index to retrieve.
	\return The associated material.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMaterial NxMaterialIndex NxScene.createMaterial() getHighestMaterialIndex()
	*/
	virtual	NxMaterial *				getMaterialFromIndex(NxMaterialIndex matIndex) = 0;

//@}
/************************************************************************************************/

	/**
	\brief Flush the scene's command queue for processing.

	Flushes any buffered commands so that they get executed.
	Ensures that commands buffered in the system will continue to make forward progress until completion.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setTiming() simulate() fetchResults() checkResults()
	*/
	virtual void						flushStream() = 0;

	/**
 	\brief Sets simulation timing parameters used in simulate(elapsedTime).

	If method is NX_TIMESTEP_FIXED, elapsedTime(simulate() parameter) is internally subdivided into up to
	maxIter substeps no larger than maxTimestep.
	
	If elapsedTime is not a multiple of maxTimestep then any remaining time is accumulated 
	to be added onto the elapsedTime for the next time step.

	If more sub steps than maxIter are needed to advance the simulation by elapsed time, then
	the remaining time is also accumulated for the next call to simulate().
	
	The timestep method of TIMESTEP_FIXED is strongly preferred for stable, reproducible simulation.

	Alternatively NX_TIMESTEP_VARIABLE can be used, in this case the first two parameters
	are not used.	See also ::NxTimeStepMethod.

	\param[in] maxTimestep Maximum size of a substep. <b>Range:</b> (0,inf)
	\param[in] maxIter Maximum number of iterations to divide a timestep into.
	\param[in] method Method to use for timestep (either variable time step or fixed). See #NxTimeStepMethod.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see flushStream() simulate() fetchResults() checkResults()
	*/
	virtual void						setTiming(NxReal maxTimestep=1.0f/60.0f, NxU32 maxIter=8, NxTimeStepMethod method=NX_TIMESTEP_FIXED) = 0;

	/**
	\brief Retrieves simulation timing parameters.

	\param[in] maxTimestep Maximum size to divide a substep into. <b>Range:</b> (0,inf)
	\param[in] maxIter Maximum number of iterations to divide a timestep into.
	\param[in] method Method to use for timestep (either variable time step or fixed). See #NxTimeStepMethod.
	\param[in] numSubSteps The number of sub steps the time step will be divided into.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setTiming()
	*/
	virtual void						getTiming(NxReal& maxTimestep, NxU32& maxIter, NxTimeStepMethod& method, NxU32* numSubSteps=NULL) const = 0;

	/**
	\brief Retrieves the debug renderable.
	
	This will contain the results of any active visualization for this scene.
	\return The debug renderable.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxDebugRenderable
	*/
	virtual const NxDebugRenderable *	getDebugRenderable() = 0;

	/**
	\brief Call this method to retrieve the Physics SDK.

	\return The physics SDK this scene is associated with.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPhysicsSDK
	*/
	virtual	NxPhysicsSDK&				getPhysicsSDK() = 0;

	/**
	\brief Call this method to retrieve statistics about the current scene.

	\param[out] stats Used to retrieve statistics for the scene. See #NxSceneStats.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : No
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneStats getLimits()
	*/
	virtual	void						getStats(NxSceneStats& stats) const = 0;
#ifdef NX_ENABLE_SCENE_STATS2
	/**
	\brief Call this method to retrieve extended statistics about the current scene.

	\return Used to retrieve statistics for the scene. See #NxSceneStats2. 
	Note that this is a pointer to data that changes with each simulation call; 
	the relevant data must be copied in order to save it.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	const NxSceneStats2 *				getStats2() const = 0;
#endif

	/**
	\brief Call to retrieve the expected object count limits set in the scene descriptor.

	\param[out] limits Used to retrieve the limits for the scene(e.g. maximum number of actors). See #NxSceneLimits.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneLimits getStats() NxSceneStats
	*/
	virtual	void						getLimits(NxSceneLimits& limits) const = 0;


	/**
	\brief Not yet implemented!
	
	Call to set the maximum CPU for use when load-balancing. 

	The SDK may choose to balance physics load by simulating assets on the CPU which could 
	otherwise be simulated on the PPU. Use this function to allocate a portion of the CPU
	for this purpose. Note that the SDK only load-balances assets created in compartments 
	with the attribute NX_DC_PPU_AUTO_ASSIGN, and that the default CPU allocated is zero.

	\param[in] cpuFraction The maximum fraction of the total host CPU to use - for example 
	0.2 would equate to 20% of a single core or might use 10% of each core or 20% of one 
	core on a dual-core CPU.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxDeviceCode getMaxCPUForLoadBalancing()
	*/
	virtual	void						setMaxCPUForLoadBalancing(NxReal cpuFraction) = 0;

	/**
	\brief Call to get the maximum CPU for use when load-balancing.

	@see setMaxCPUForLoadBalancing()
	*/
	virtual	NxReal						getMaxCPUForLoadBalancing() = 0;


/************************************************************************************************/

/** @name Callbacks
*/
//@{

	/**
	\brief Sets a user notify object which receives special simulation events when they occur.

	\param[in] callback User notification callback. See #NxUserNotify.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserNotify getUserNotify
	*/
	virtual void						setUserNotify(NxUserNotify* callback) = 0;

	/**
	\brief Retrieves the userNotify pointer set with setUserNotify().

	\return The current user notify pointer. See #NxUserNotify.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserNotify setUserNotify()
	*/
	virtual NxUserNotify*				getUserNotify() const = 0;

#if NX_USE_FLUID_API
	/**
	\brief Sets a user notify object which receives special simulation events when they occur.

	\param[in] callback User fluid notification callback. See #NxFluidUserNotify.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxFluidUserNotify getFluidUserNotify
	*/
	virtual void						setFluidUserNotify(NxFluidUserNotify* callback) = 0;


	/**
	\brief Retrieves the NxFluidUserNotify pointer set with setfluidUserNotify().

	\return The current user fluid notify pointer. See #NxFluidUserNotify.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxFluidUserNotify setFluidUserNotify()
	*/
	virtual NxFluidUserNotify*			getFluidUserNotify() const = 0;

#endif //NX_USE_FLUID_API

#if NX_USE_CLOTH_API
	/**
	\brief Sets a user notify object which receives special simulation events when they occur.

	\param[in] callback User cloth notification callback. See #NxClothUserNotify.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxClothUserNotify getClothUserNotify
	*/
	virtual void						setClothUserNotify(NxClothUserNotify* callback) = 0;


	/**
	\brief Retrieves the NxClothUserNotify pointer set with setClothUserNotify().

	\return The current user cloth notify pointer. See #NxClothUserNotify.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxClothUserNotify setClothUserNotify()
	*/
	virtual NxClothUserNotify*			getClothUserNotify() const = 0;
#endif //NX_USE_CLOTH_API

#if NX_USE_SOFTBODY_API
	/**
	\brief Sets a user notify object which receives special simulation events when they occur.

	\param[in] callback User softbody notification callback. See #NxSoftBodyUserNotify.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxSoftBodyUserNotify getSoftBodyUserNotify
	*/
	virtual void						setSoftBodyUserNotify(NxSoftBodyUserNotify* callback) = 0;


	/**
	\brief Retrieves the NxSoftBodyUserNotify pointer set with setSoftBodyUserNotify().

	\return The current user softbody notify pointer. See #NxSoftBodyUserNotify.

	<b>Platform:</b>
	\li PC SW: No
	\li PPU  : No
	\li PS3  : No
	\li XB360: No

	@see NxSoftBodyUserNotify setSoftBodyUserNotify()
	*/
	virtual NxSoftBodyUserNotify*		getSoftBodyUserNotify() const = 0;
#endif //NX_USE_SOFTBODY_API

	/**
	\brief Sets a user callback object, which receives callbacks on all contacts generated for specified actors.

	\param[in] callback Asynchronous user contact modification callback. See #NxUserContactModify.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void						setUserContactModify(NxUserContactModify* callback) = 0;

	/**
	\brief Retrieves the NxUserContactModify pointer set with setUserContactModify().

	\return The current user contact modify callback pointer. See #NxUserContactModify.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserContactModify setUserContactModify()
	*/
	virtual NxUserContactModify*				getUserContactModify() const = 0;

	/**
	\brief Sets a trigger report object which receives collision trigger events.

	\param[in] callback User trigger callback. See #NxUserTriggerReport.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserTriggerReport getUserTriggerReport()
	*/
	virtual	void						setUserTriggerReport(NxUserTriggerReport* callback) = 0;

	/**
	\brief Retrieves the callback pointer set with setUserTriggerReport().

	\return The current user trigger pointer. See #NxUserTriggerReport.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserTriggerReport setUserTriggerReport()
	*/
	virtual	NxUserTriggerReport*		getUserTriggerReport() const = 0;

	/**
	\brief Sets a contact report object which receives collision contact events.

	\param[in] callback User contact callback. See #NxUserContactReport.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserContactReport getUserContactReport()
	*/
	virtual	void						setUserContactReport(NxUserContactReport* callback) = 0;

	/**
	\brief Retrieves the callback pointer set with setUserContactReport().

	\return The current user contact reporter. See #NxUserContactReport.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxUserContactReport setUserContactReport()
	*/
	virtual	NxUserContactReport*		getUserContactReport() const = 0;

	/**
	\brief Sets the custom actor pair filtering to use for this scene.

	\param callback Filtering class that defines the callback to use for custom pair filtering.

	@see NxUserActorPairFiltering getUserActorPairFiltering NxActor::resetUserActorPairFiltering
	*/
	virtual	void						setUserActorPairFiltering(NxUserActorPairFiltering* callback) = 0;

	/**
	\brief Gets the custom actor pair filtering in use for this scene.

	\return Filtering class that defines the callback used for custom pair filtering.

	@see NxUserActorPairFiltering setUserActorPairFiltering NxActor::resetUserActorPairFiltering
	*/
	virtual	NxUserActorPairFiltering*	getUserActorPairFiltering() const = 0;

//@}
/************************************************************************************************/

/** @name Raycasting
*/
//@{

	/**
	\brief Returns true if any axis aligned bounding box enclosing a shape of type shapeType is intersected by the ray.

	\note Make certain that the direction vector of NxRay is normalized.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldRay The ray to cast in the global frame. <b>Range:</b> See #NxRay
	\param[in] shapesType Choose if to raycast against static, dynamic or both types of shape. See #NxShapesType.
	\param[in] groups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] maxDist Max distance to check along the ray for intersecting bounds. <b>Range:</b> (0,inf)
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return true if any axis aligned bounding box enclosing a shape of type shapeType is intersected by the ray

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapesType NxRay NxShape.setGroup()  raycastAnyShape()
	*/
	virtual bool						raycastAnyBounds		(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, const NxGroupsMask* groupsMask=NULL) const = 0;

	/**
	\brief Returns true if any shape of type ShapeType is intersected by the ray.

	\note Make certain that the direction vector of NxRay is normalized.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldRay The ray to cast in the global frame. <b>Range:</b> See #NxRay
	\param[in] shapesType Choose if to raycast against static, dynamic or both types of shape. See #NxShapesType.
	\param[in] groups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] maxDist Max distance to check along the ray for intersecting objects. <b>Range:</b> (0,inf) 
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask
	\param[in] cache Possible cache for persistent raycasts, filled out by the SDK.

	\return Returns true if any shape of type ShapeType is intersected by the ray.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShapesType NxRay NxShape.setGroup()  raycastAnyBounds()
	*/
	virtual bool						raycastAnyShape			(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, const NxGroupsMask* groupsMask=NULL, NxShape** cache=NULL) const = 0;

	/**
	\brief Calls the report's hitCallback() method for all the axis aligned bounding boxes enclosing shapes of type ShapeType intersected by the ray.
	
	The point of impact is provided as a parameter to hitCallback().
	hintFlags is a combination of ::NxRaycastBit flags.
	Returns the number of shapes hit.

	\note Make certain that the direction vector of NxRay is normalized.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldRay The ray to cast in the global frame. <b>Range:</b> See #NxRay
	\param[in] report User callback, to be called when an intersection is encountered.
	\param[in] shapesType Choose if to raycast against static, dynamic or both types of shape. See #NxShapesType.
	\param[in] groups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] maxDist Max distance to check along the ray for intersecting objects. <b>Range:</b> (0,inf) 
	\param[in] hintFlags Allows the user to specify which field of #NxRaycastHit they are interested in.
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return the number of shapes hit.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see raycastAnyBounds() raycastAllShapes() NxRay NxUserRaycastReport NxShapesType NxShape.setGroup() NxRaycastHit
	*/
	virtual NxU32						raycastAllBounds		(const NxRay& worldRay, NxUserRaycastReport& report, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL) const = 0;

	/**
	\brief Calls the report's hitCallback() method for all the shapes of type ShapeType intersected by the ray.
	
	hintFlags is a combination of ::NxRaycastBit flags.
	Returns the number of shapes hit. The point of impact is provided as a parameter to hitCallback().

	\note Make certain that the direction vector of NxRay is normalized.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	<h3>Example</h3>

	\include NxUserRaycastReport_Usage.cpp


	\param[in] worldRay The ray to cast in the global frame. <b>Range:</b> See #NxRay
	\param[in] report User callback, to be called when an intersection is encountered.
	\param[in] shapesType Choose if to raycast against static, dynamic or both types of shape. See #NxShapesType.
	\param[in] groups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] maxDist Max distance to check along the ray for intersecting objects. <b>Range:</b> (0,inf) 
	\param[in] hintFlags Allows the user to specify which field of #NxRaycastHit they are interested in. See #NxRaycastBit
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return the number of shapes hit

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see raycastAnyShape() raycastAllBounds() NxRay NxUserRaycastReport NxShapesType NxShape.setGroup() NxRaycastHit
	*/
	virtual NxU32						raycastAllShapes		(const NxRay& worldRay, NxUserRaycastReport& report, NxShapesType shapesType, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL) const = 0;

	/**
	\brief Returns the first axis aligned bounding box enclosing a shape of type shapeType that is hit along the ray.
	
	The world space intersection point, and the distance along the ray are also provided.
	hintFlags is a combination of ::NxRaycastBit flags.

	\note Make certain that the direction vector of NxRay is normalized.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldRay The ray to cast in the global frame. <b>Range:</b> See #NxRay
	\param[in] shapeType Choose if to raycast against static, dynamic or both types of shape. See #NxShapesType.
	\param[out] hit Description of the intersection. See #NxRaycastHit.
	\param[in] groups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] maxDist Max distance to check along the ray for intersecting objects. <b>Range:</b> (0,inf) 
	\param[in] hintFlags Allows the user to specify which field of #NxRaycastHit they are interested in. See #NxRaycastBit
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return The shape which is hit.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see raycastAllBounds() NxRay NxShapesType NxRaycastHit NxShape.setGroup() NxRaycastBit
	*/
	virtual NxShape*					raycastClosestBounds	(const NxRay& worldRay, NxShapesType shapeType, NxRaycastHit& hit, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL) const = 0;

	/**
	\brief Returns the first shape of type shapeType that is hit along the ray.
	
	The world space intersection point, and the distance along the ray are also provided.
	hintFlags is a combination of ::NxRaycastBit flags.

	\note Make certain that the direction vector of NxRay is normalized.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldRay The ray to cast in the global frame. <b>Range:</b> See #NxRay
	\param[in] shapeType Choose if to raycast against static, dynamic or both types of shape. See #NxShapesType.
	\param[out] hit Description of the intersection. See #NxRaycastHit.
	\param[in] groups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] maxDist Max distance to check along the ray for intersecting objects. <b>Range:</b> (0,inf) 
	\param[in] hintFlags Allows the user to specify which field of #NxRaycastHit they are interested in. See #NxRaycastBit
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask
	\param[in] cache Possible cache for persistent raycasts, filled out by the SDK.

	\return The shape which is hit.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see raycastAllShapes() NxRay NxShapesType NxRaycastHit NxShape.setGroup() NxRaycastBit
	*/
	virtual NxShape*					raycastClosestShape		(const NxRay& worldRay, NxShapesType shapeType, NxRaycastHit& hit, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask* groupsMask=NULL, NxShape** cache=NULL) const = 0;

//@}
/************************************************************************************************/

/** @name Overlap Testing
*/
//@{

	/**
	\brief Returns the set of shapes overlapped by the world-space sphere.
	
	You can test against static and/or dynamic objects by adjusting 'shapeType'. 
	Shapes are written to the static array 'shapes', which should be big enough to hold 'nbShapes'.
	An alternative is to use the ::NxUserEntityReport callback mechanism.

	The function returns the total number of collided shapes.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldSphere Sphere description in world space. <b>Range:</b> See #NxSphere
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] nbShapes Number of shapes that the buffer shapes can hold.
	\param[out] shapes Buffer to store intersecting shapes. Should be at least sizeof(NxShape *) * nbShapes.
	\param[in] callback Alternative method to retrieve overlapping shapes. Is called with sets of overlapping shapes.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask
	\param[in] accurateCollision True to test the sphere against the actual shapes, false to test against the AABBs only.

	\return the total number of collided shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphere NxShapesType overlapAABBShapes NxUserEntityReport NxShape.checkOverlapSphere()
	*/
	virtual	NxU32						overlapSphereShapes		(const NxSphere& worldSphere, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, bool accurateCollision=false) = 0;

	/**
	\brief Returns the set of shapes overlapped by the world-space AABB.
	
	You can test against static and/or dynamic objects by adjusting 'shapeType'. 
	Shapes are written to the static array 'shapes', which should be big enough to hold 'nbShapes'.
	An alternative is to use the ::NxUserEntityReport callback mechanism.

	The function returns the total number of collided shapes.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldBounds Axis Aligned Bounding Box in world space. <b>Range:</b> See #NxBounds3
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] nbShapes Number of shapes that the buffer shapes can hold.
	\param[out] shapes Buffer to store intersecting shapes. Should be at least sizeof(NxShape *) * nbShapes.
	\param[in] callback Alternative method to retrieve overlapping shapes. Is called with sets of overlapping shapes.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask
	\param[in] accurateCollision True to test the AABB against the actual shapes, false to test against the AABBs only.

	\return the total number of collided shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxShapesType overlapAABBShapes NxUserEntityReport NxShape.checkOverlapAABB()
	*/
	virtual	NxU32						overlapAABBShapes		(const NxBounds3& worldBounds, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, bool accurateCollision=false) = 0;

	/**
	\brief Returns the set of shapes overlapped by the world-space OBB.
	
	You can test against static and/or dynamic objects by adjusting 'shapeType'. 
	Shapes are written to the static array 'shapes', which should be big enough to hold 'nbShapes'.
	An alternative is to use the ::NxUserEntityReport callback mechanism.

	The function returns the total number of collided shapes.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldBox Oriented Bounding Box in world space. <b>Range:</b> See #NxBox
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] nbShapes Number of shapes that the buffer shapes can hold.
	\param[out] shapes Buffer to store intersecting shapes. Should be at least sizeof(NxShape *) * nbShapes.
	\param[in] callback Alternative method to retrieve overlapping shapes. Is called with sets of overlapping shapes.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask
	\param[in] accurateCollision True to test the OBB against the actual shapes, false to test against the AABBs only.

	\return the total number of collided shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxShapesType overlapOBBShapes NxUserEntityReport NxShape.checkOverlapOBB()
	*/
	virtual	NxU32						overlapOBBShapes		(const NxBox& worldBox, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, bool accurateCollision=false) = 0;

	/**
	\brief Returns the set of shapes overlapped by the world-space capsule.
	
	You can test against static and/or dynamic objects by adjusting 'shapeType'. 
	Shapes are written to the static array 'shapes', which should be big enough to hold 'nbShapes'.
	An alternative is to use the ::NxUserEntityReport callback mechanism.

	The function returns the total number of collided shapes.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldCapsule capsule in world space. <b>Range:</b> See #NxCapsule
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] nbShapes Number of shapes that the buffer shapes can hold.
	\param[out] shapes Buffer to store intersecting shapes. Should be at least sizeof(NxShape *) * nbShapes.
	\param[in] callback Alternative method to retrieve overlapping shapes. Is called with sets of overlapping shapes.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask
	\param[in] accurateCollision True to test the capsule against the actual shapes, false to test against the AABBs only.

	\return the total number of collided shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxShapesType overlapCapsuleShapes NxUserEntityReport NxShape.checkOverlapCapsule()
	*/
	virtual	NxU32						overlapCapsuleShapes		(const NxCapsule& worldCapsule, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL, bool accurateCollision=false) = 0;

#if NX_SUPPORT_SWEEP_API
	/**
	\brief Creates a sweep cache, for use with NxActor::linearSweep(). See the Guide, "Sweep API" section for 
	more information.
	@see NxActor
	*/
	virtual	NxSweepCache*				createSweepCache() = 0;
	/**
	\brief Deletes a sweep cache. See the Guide, "Sweep API" section, for more
	information on sweep caches.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).
	@see NxActor
	*/
	virtual	void						releaseSweepCache(NxSweepCache* cache)	= 0;

	/**
	\brief Performs a linear sweep through space with an oriented box.

	The function sweeps an oriented box through space and reports any shapes in the scene
	which it intersects. Apart from the number of shapes intersected in this way, and the shapes
	intersected, information on the closest intersection is put in an #NxSweepQueryHit structure which 
	can be processed in the callback function if provided.
	Which shapes in the scene are regarded is specified through the flags parameter.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldBox The oriented box (#NxBox object) that is to be swept
	\param[in] motion Length and direction of the sweep
	\param[in] flags Flags controlling the mode of the sweep
	\param[in] userData User data to impart to the returned data struct
	\param[in] nbShapes Maximum number of shapes to report <b>Range:</b> [1,NX_MAX_U32]
	\param[out] shapes Pointer to buffer for reported shapes
	\param[in] callback Callback function invoked on the closest hit (if any)
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return The number of hits reported.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBox NxShape NxSweepQueryHit NxSweepFlags NxUserEntityReport NxScene
	*/
	virtual NxU32						linearOBBSweep				(const NxBox& worldBox, const NxVec3& motion, NxU32 flags, void* userData, NxU32 nbShapes, NxSweepQueryHit* shapes, NxUserEntityReport<NxSweepQueryHit>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL)			= 0;
	/**
	\brief Performs a linear sweep through space with an oriented capsule.

	The function a capsule through space and reports any shapes in the scene
	which it intersects. Apart from the number of shapes intersected in this way, and the shapes
	intersected, information on the closest intersection is put in an #NxSweepQueryHit structure which 
	can be processed in the callback function if provided.
	Which shapes in the scene are regarded is specified through the flags parameter.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldCapsule The oriented capsule (#NxCapsule object) that is to be swept
	\param[in] motion Length and direction of the sweep
	\param[in] flags Flags controlling the mode of the sweep
	\param[in] userData User data to impart to the returned data struct
	\param[in] nbShapes Maximum number of shapes to report <b>Range:</b> [1,NX_MAX_U32]
	\param[out] shapes Pointer to buffer for reported shapes
	\param[in] callback Callback function invoked on the closest hit (if any)
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return The number of hits reported.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsule NxShape NxSweepQueryHit NxSweepFlags NxUserEntityReport NxScene
	*/
	virtual	NxU32						linearCapsuleSweep			(const NxCapsule& worldCapsule, const NxVec3& motion, NxU32 flags, void* userData, NxU32 nbShapes, NxSweepQueryHit* shapes, NxUserEntityReport<NxSweepQueryHit>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL)	= 0;
#endif

	/**
	\brief Returns the set of shapes which are in the negative half space of a number of planes.
	
	This function returns the set of shapes whose axis aligned bounding volumes are in the negative 
	half space(side the normal points away from) of all the planes passed in.
	
	However the set of shapes returned is not conservative, ie additional shapes may be returned which
	do not actually intersect the union of the planes negative half space.

	You can test against static and/or dynamic objects by adjusting 'shapeType'.
	Shapes are written to the static array 'shapes', which should be big enough to hold 'nbShapes'.
	An alternative is to use the ::NxUserEntityReport callback mechanism.

	The function returns the total number of collided shapes.

	This function can be used for view-frustum culling by passing the 6 camera planes to the function.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\warning Passing more than 32 planes to this function is unsupported and may result in undefined behavior.
	
	\param[in] nbPlanes Number of planes to test. (worldPlanes should contain this many planes)
	\param[in] worldPlanes Set of planes to test. <b>Range:</b> See #NxPlane
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] nbShapes Number of shapes that the buffer shapes can hold.
	\param[out] shapes Buffer to store intersecting shapes. Should be at least sizeof(NxShape *) * nbShapes.
	\param[in] callback Alternative method to retrieve overlapping shapes. Is called with sets of overlapping shapes.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return the total number of collided shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxPlane overlapAABBShapes() overlapSphereShapes() NxShapesType NxUserEntityReport NxShape.setGroup()
	*/

	virtual	NxU32						cullShapes				(NxU32 nbPlanes, const NxPlane* worldPlanes, NxShapesType shapeType, NxU32 nbShapes, NxShape** shapes, NxUserEntityReport<NxShape*>* callback, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL) = 0;

	/**
	\brief Checks whether a world-space sphere overlaps a shape or not.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldSphere Sphere description in world space. <b>Range:</b> See #NxSphere
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return True if the sphere overlaps a shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSphere NxShapesType overlapSphereShapes NxShape.checkOverlapSphere()
	*/
	virtual bool						checkOverlapSphere		(const NxSphere& worldSphere, NxShapesType shapeType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL)	= 0;

	/**
	\brief Checks whether a world-space AABB overlaps a shape or not.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldBounds Axis Aligned Bounding Box in world space. <b>Range:</b> See #NxBounds3
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return True if the AABB overlaps a shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxShapesType overlapAABBShapes NxShape.checkOverlapAABB()
	*/
	virtual bool						checkOverlapAABB		(const NxBounds3& worldBounds, NxShapesType shapeType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL)	= 0;

	/**
	\brief Checks whether a world-space OBB overlaps a shape or not.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldBox Oriented Bounding Box in world space. <b>Range:</b> See #NxBounds3
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return True if the OBB overlaps a shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxShapesType overlapOBBShapes NxShape.checkOverlapOBB()
	*/
	virtual bool						checkOverlapOBB			(const NxBox& worldBox, NxShapesType shapeType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL)	= 0;

	/**
	\brief Checks whether a world-space capsule overlaps something or not.

	\note Because the SDK double buffers shape state, a shape will not be updated until a simulation step is 
	taken. For example the result of setting the global pose is not immediatly visible.

	\param[in] worldCapsule Capsule description in world space. <b>Range:</b> See #NxCapsule
	\param[in] shapeType Choose if to intersect with static, dynamic or both types of shape. See #NxShapesType.
	\param[in] activeGroups Mask used to filter shape objects. See #NxShape::setGroup
	\param[in] groupsMask Alternative mask used to filter shapes. See #NxShape::setGroupsMask

	\return True if the capsule overlaps a shape.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCapsule NxShapesType NxShape.setGroup NxShape.setGroupsMask
	*/
	virtual bool						checkOverlapCapsule		(const NxCapsule& worldCapsule, NxShapesType shapeType=NX_ALL_SHAPES, NxU32 activeGroups=0xffffffff, const NxGroupsMask* groupsMask=NULL)	= 0;

	//virtual	NxU32						overlapAABBTriangles	(const NxBounds3& worldBounds, NxArraySDK<NxTriangle>& worldTriangles) = 0;
//@}
/************************************************************************************************/







/** @name Fluids
*/
//@{

#if NX_USE_FLUID_API
	
	/**
	\brief Creates a fluid in this scene. 
	
	NxFluidDesc::isValid() must return true.

	\param[in] fluidDesc Description of the fluid object to create. See #NxFluidDesc.
	\return The new fluid.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see releaseFluid()
	*/
	virtual NxFluid*					createFluid(const NxFluidDescBase& fluidDesc) = 0;

	/**
	\brief Deletes the specified fluid.	The fluid must be in this scene.

	Also releases any screen surface meshes created from the fluid.

	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param[in] fluid Fluid to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see createFluid()
	*/
	virtual void						releaseFluid(NxFluid& fluid)			= 0;

	/**
	\brief Get the number of fluids belonging to the scene.

	\return the number of fluids.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getFluids()
	*/
	virtual	NxU32						getNbFluids()		const		= 0;

	/**
	\brief Get an array of fluids belonging to the scene.

	\return an array of fluid pointers with size getNbFluids().

	\return An array of fluid objects belonging to this scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbFluids()
	*/
	virtual	NxFluid**					getFluids()						= 0;


	/**
	\brief Pre-cooks all triangles from static NxTriangleMeshShapes of the scene which are intersecting with the given bounds.
	
	The pre-cooking will only be valid for Fluids which share the specified parameters (see NxFluidDesc)
	
	\param[in] bounds The volume whose contents should be pre-cooked
	\param[in] packetSizeMultiplier
	\param[in] restParticlesPerMeter 
	\param[in] kernelRadiusMultiplier
	\param[in] motionLimitMultiplier
	\param[in] collisionDistanceMultiplier
	\param[in] compartment The specific compartment to perform the pre-cooking for.
	\param[in] forceStrictCookingFormat Forces specified cooking parameters. Otherwise they might internaly be reinterpreted depending on created fluids. Not implemented yet.

	\return Operation succeeded.
	@see NxFluidDesc
	*/
	virtual bool						cookFluidMeshHotspot(const NxBounds3& bounds, NxU32 packetSizeMultiplier, NxReal restParticlesPerMeter, NxReal kernelRadiusMultiplier, NxReal motionLimitMultiplier, NxReal collisionDistanceMultiplier, NxCompartment* compartment = NULL, bool forceStrictCookingFormat = false) = 0;

#endif
//@}
/************************************************************************************************/
	
//@}
/************************************************************************************************/
/** @name Cloth
*/
//@{

#if NX_USE_CLOTH_API
	
	/**
	Creates a cloth in this scene. NxClothDesc::isValid() must return true.

	\param clothDesc Description of the cloth object to create. See #NxClothDesc.
	\return The new cloth.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxClothDesc NxCloth
	*/
	virtual NxCloth*					createCloth(const NxClothDesc& clothDesc) = 0;

	/**
	Deletes the specified cloth. The cloth must be in this scene.
	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param cloth Cloth to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCloth
	*/
	virtual void						releaseCloth(NxCloth& cloth) = 0;

	/**
	\return the number of cloths.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getCloths()
	*/
	virtual	NxU32						getNbCloths() const = 0;

	/**
	\brief Returns an array of cloth objects.

	\return an array of cloth pointers with size getNbCloths().

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbCloths()
	*/
	virtual	NxCloth**					getCloths() = 0;

#endif
//@}

/************************************************************************************************/
/** @name SoftBody
*/
//@{

#if NX_USE_SOFTBODY_API
	
	/**
	Creates a soft body in this scene. NxSoftBodyDesc::isValid() must return true.

	\param softBodyDesc Description of the soft body object to create. See #NxSoftBodyDesc.
	\return The new soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc NxSoftBody
	*/
	virtual NxSoftBody*					createSoftBody(const NxSoftBodyDesc& softBodyDesc) = 0;

	/**
	Deletes the specified soft body. The soft body must be in this scene.
	Do not keep a reference to the deleted instance.
	Avoid release calls while the scene is simulating (in between simulate() and fetchResults() calls).

	\param softBody Soft body to release.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBody
	*/
	virtual void						releaseSoftBody(NxSoftBody& softBody) = 0;

	/**
	\return the number of soft bodies.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getSoftBodies()
	*/
	virtual	NxU32						getNbSoftBodies() const = 0;

	/**
	\brief Returns an array of soft body objects.

	\return an array of soft body pointers with size getNbSoftBodies().

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNbSoftBodies()
	*/
	virtual	NxSoftBody**					getSoftBodies() = 0;

#endif
//@}

/************************************************************************************************/

	/**
	\brief This is a query to see if the scene is in a state that allows the application to update scene state.

	\return True if the scene can be written by the application.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	bool						isWritable()	= 0;

	/**
 	\brief Advances the simulation by an elapsedTime time.
	
	If elapsedTime is large, it is internally subdivided according to parameters provided with the 
	#setTiming() method. See #setTiming() for a more detailed discussion of the elapsedTime parameter 
	and time stepping behavior.
 
 	Calls to simulate() should pair with calls to fetchResults():
 	Each fetchResults() invocation corresponds to exactly one simulate()
 	invocation; calling simulate() twice without an intervening fetchResults()
 	or fetchResults() twice without an intervening simulate() causes an error
 	condition.
 
 	scene->simulate();
 	...do some processing until physics is computed...
 	scene->fetchResults();
 	...now results of run may be retrieved.
 
 	Applications should not modify physics objects between calls to
 	simulate() and fetchResults();
 
  	This method replaces startRun().
	

	\param[in] elapsedTime Amount of time to advance simulation by. <b>Range:</b> (0,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see fetchResults() checkResults()
	*/
	virtual	void						simulate(NxReal elapsedTime)		= 0;
	
	/**
	\brief This checks to see if the part of the simulation run whose results you are interested in has completed.

	This does not cause the data available for reading to be updated with the results of the simulation, it is simply a status check.
	The bool will allow it to either return immediately or block waiting for the condition to be met so that it can return true

	This method replaces wait()
	
	\param[in] status The part of the simulation to check (eg NX_RIGID_BODY_FINISHED). See #NxSimulationStatus.
	\param[in] block When set to true will block until the condition is met.
	\return True if the results are available.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see simulate() fetchResults()
	*/
	virtual	bool						checkResults(NxSimulationStatus status, bool block = false)	= 0;

	/**
	This is the big brother to checkResults() it basically does the following:
	
	\code
	if ( checkResults(enum, block) )
	{
		fire appropriate callbacks
		swap buffers
		if (CheckResults(all_enums, false))
			make IsWritable() true
		return true
	}
	else
		return false

	\endcode

	\param[in] status The part of the simulation to fetch results for (eg NX_RIGID_BODY_FINISHED). See #NxSimulationStatus.
	\param[in] block When set to true will block until the condition is met.
	\param[out] errorState Used to retrieve hardware error codes. A non zero value indicates an error.
	\return True if the results have been fetched.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see simulate() checkResults()
	*/
	virtual	bool						fetchResults(NxSimulationStatus status, bool block = false, NxU32 *errorState = 0)	= 0;

	/**
	Flush internal caches.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual	void						flushCaches()	= 0;

	/**
	Accesses internal profiler.  If clearData is true, the profile counters are reset.  You will most likely want to call readProfileData(true)
	right after calling fetchResults().  If that is not the time you want to read out the data, just call readProfileData(false) at some other place
	in your code.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual const NxProfileData *		readProfileData(bool clearData)	= 0;

	/**
	 \brief Poll for work to execute on the current thread.
	
	Returns NX_THREAD_SIMULATION_END if the simulation step ended, 
	NX_THREAD_NOWORK if no work is available, NX_THREAD_MOREWORK if
	there may be work remaining to execute.

	The function waits for work to execute according the the waitType argument.

	The function will continue to return NX_THREAD_SIMULATION_END until the 
	#resetPollForWork() method is called.
	
	\param[in] waitType Specifies how long the function should wait for work.
	\return Informs the user about the reason for returning.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see resetPollForWork() NxThreadWait NxThreadPollResult
	*/
	virtual NxThreadPollResult pollForWork(NxThreadWait waitType)=0;

	/**
	\brief Reset parallel simulation

	Resets parallel simulation so that threads calling pollForWork do not 
	return NX_THREAD_SIMULATION_END, but instead wait for a new simulation
	step to start.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes


	@see pollForWork() NxThreadPollResult NxThreadWait
	*/

	virtual void resetPollForWork()=0;

	/**
	\brief Polls for background work.
	
	Returns NX_THREAD_NOWORK if no work is available, NX_THREAD_MOREWORK if
	there may be work remaining to execute or NX_THREAD_SHUTDOWN if
	the thread was released by shutdownWorkerThreads().

	The function waits for work to execute according the the waitType argument.

	The NX_WAIT_SIMULATION_END does not make sense for background tasks, as they are not 
	associated with a particular simulation step.

	\param[in] waitType Specifies how long the function should wait for work.
	\return Informs the user about the reason for returning.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see resetPollForWork() NxThreadWait NxThreadPollResult
	*/
	virtual NxThreadPollResult pollForBackgroundWork(NxThreadWait waitType)=0;

	/**
	\brief Release threads which are blocking to allow the SDK to be destroyed safely.

	Note: This also applies to SDK managed threads. So calling shutdownWorkerThreads will release 
	all threads which are waiting using NX_WAIT_SHUTDOWN, which includes internal SDK threads.

	When internal threads are released by shutdownWorkerThreads() they will exit.

	In a typical application, the user should call shutdownWorkerThreads prior to destroying the
	scene. Then they will block until all user owned threads have left the API. It is then 
	safe to release the scene.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see pollForBackgroundWork()
	*/
	virtual void shutdownWorkerThreads()=0;

	/**
	\brief Blocks all parallel raycast/overlap queries.

	This method should be used to lock raycasts from other threads while the SDK state is being updated in a way
	which will affect raycasting/overlap queries.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see unlockQueries()
	*/
	virtual void lockQueries()=0;

	/**
	\brief Unlock parallel raycast/overlap queries.

	See #lockQueries() for more details.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : No
	\li XB360: Yes

	@see lockQueries()
	*/
	virtual void unlockQueries()=0;

	// BATCHED_RAYCASTS

	/**
	\brief Create a batched query object.

	\param[in] desc Descriptor used to modify the created query object.
	\return A new query object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (software)
	\li PS3  : Yes
	\li XB360: Yes

	@see releaseSceneQuery() NxSceneQueryDesc NxSceneQuery
	*/
	virtual	NxSceneQuery*	createSceneQuery(const NxSceneQueryDesc& desc)	= 0;

	/**

	\brief Release a scene query object.

	\param[in] query The query object to release.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (software)
	\li PS3  : Yes
	\li XB360: Yes

	@see createSceneQuery() NxSceneQuery 
	*/
	virtual	bool			releaseSceneQuery(NxSceneQuery& query)			= 0;
	//~ BATCHED_RAYCASTS

	/**
	\brief Sets the rebuild rate of the dynamic tree pruning structure.

	\param[in] dynamicTreeRebuildRateHint Rebuild rate of the dynamic tree pruning structure.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneDesc.dynamicTreeRebuildRateHint getDynamicTreeRebuildRateHint()
	*/
	virtual	void			setDynamicTreeRebuildRateHint(NxU32 dynamicTreeRebuildRateHint) = 0;

	/**
	\brief Retrieves the rebuild rate of the dynamic tree pruning structure.

	\return The rebuild rate of the dyamic tree pruning structure.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSceneDesc.dynamicTreeRebuildRateHint setDynamicTreeRebuildRateHint()
	*/
	virtual NxU32			getDynamicTreeRebuildRateHint() const = 0;

	/**
	\brief Sets the number of actors required to spawn a separate rigid body solver thread.

	\note If internal multi threading is disabled (see #NX_SF_ENABLE_MULTITHREAD) this call will
	have no effect.

	\param[in] solverBatchSize Number of actors required to spawn a separate rigid body solver thread.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Not applicable
	\li PS3  : Not applicable
	\li XB360: Yes

	@see NxSceneDesc.solverBatchSize getSolverBatchSize()
	*/
	virtual	void			setSolverBatchSize(NxU32 solverBatchSize) = 0;

	/**
	\brief Retrieves the number of actors required to spawn a separate rigid body solver thread.

	\return Current number of actors required to spawn a separate rigid body solver thread.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Not applicable
	\li PS3  : Not applicable
	\li XB360: Yes

	@see NxSceneDesc.solverBatchSize setSolverBatchSize()
	*/
	virtual NxU32			getSolverBatchSize() const = 0;


	void*	userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	void*	extLink;	//!< reserved for linkage with other Ageia components. Applications and SDK should not modify
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
