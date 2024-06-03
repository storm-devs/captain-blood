#ifndef NX_PHYSICS_NX_SOFTBODY
#define NX_PHYSICS_NX_SOFTBODY
/** \addtogroup softbody
  @{
*/
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxSoftBodyDesc.h"

class NxRay;
class NxScene;
class NxActor;
class NxShape;
class NxBounds3;
class NxStream;
class NxCompartment;

class NxSoftBody
{
protected:
	NX_INLINE NxSoftBody() : userData(NULL) {}
	virtual ~NxSoftBody() {}

public:
	/**
	\brief Saves the soft body descriptor.

	\param[out] desc The descriptor used to retrieve the state of the object.
	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc
	*/
	virtual	bool saveToDesc(NxSoftBodyDesc& desc) const = 0;

	/**
	\brief Returns a pointer to the corresponding soft body mesh.

	\return The soft body mesh associated with this soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.softBodyMesh
	*/
	virtual	NxSoftBodyMesh* getSoftBodyMesh() const = 0;

	/**
	\brief Sets the soft body volume stiffness in the range from 0 to 1.
 
	\param[in] stiffness The volume stiffness of this soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.volumeStiffness getVolumeStiffness()
	*/
	virtual void setVolumeStiffness(NxReal stiffness) = 0;

	/**
	\brief Retrieves the soft body volume stiffness.

	\return Volume stiffness of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.volumeStiffness setVolumeStiffness()
	*/
	virtual NxReal getVolumeStiffness() const = 0;

	/**
	\brief Sets the soft body stretching stiffness in the range from 0 to 1.

	Note: The stretching stiffness must be larger than 0.

	\param[in] stiffness Stiffness of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.stretchingStiffness getStretchingStiffness()
	*/
	virtual void setStretchingStiffness(NxReal stiffness) = 0;

	/**
	\brief Retrieves the soft body stretching stiffness.

	\return stretching stiffness of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.stretchingStiffness setStretchingStiffness()
	*/
	virtual NxReal getStretchingStiffness() const = 0;

	/**
	\brief Sets the damping coefficient in the range from 0 to 1.

	\param[in] dampingCoefficient damping coefficient of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.dampingCoefficient getDampingCoefficient()
	*/
	virtual void setDampingCoefficient(NxReal dampingCoefficient) = 0;

	/**
	\brief Retrieves the damping coefficient.

	\return damping coefficient of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxSoftBodyDesc.dampingCoefficient setDampingCoefficient()
	*/
	virtual NxReal getDampingCoefficient() const = 0;

	/**
	\brief Sets the soft body friction coefficient in the range from 0 to 1.

	\param[in] friction The friction of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.friction getFriction()
	*/
	virtual void setFriction(NxReal friction) = 0;

	/**
	\brief Retrieves the soft body friction coefficient.

	\return Friction coefficient of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see NxSoftBodyDesc.friction setFriction()
	*/
	virtual NxReal getFriction() const = 0;

	/**
	\brief Sets the soft body tear factor (must be larger than one).

	\param[in] factor The tear factor for the soft body

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.tearFactor getTearFactor()
	*/
	virtual void setTearFactor(NxReal factor) = 0;

	/**
	\brief Retrieves the soft body tear factor.

	\return tear factor of the soft body.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.tearFactor setTearFactor()
	*/
	virtual NxReal getTearFactor() const = 0;

	/**
	\brief Sets the soft body attachment tear factor (must be larger than one).

	\param[in] factor The attachment tear factor for the soft body

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.attachmentTearFactor getAttachmentTearFactor()
	*/
	virtual void setAttachmentTearFactor(NxReal factor) = 0;

	/**
	\brief Retrieves the attachment soft body tear factor.

	\return tear attachment factor of the soft body.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.attachmentTearFactor setAttachmentTearFactor()
	*/
	virtual NxReal getAttachmentTearFactor() const = 0;

	/**
	\brief Sets the soft body particle radius (must be positive).

	\param[in] particleRadius The particle radius of the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.particleRadius getParticleRadius()
	*/
	virtual void setParticleRadius(NxReal particleRadius) = 0;

	/**
	\brief Gets the soft body particle radius.

	\return particle radius of the soft body.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.particleRadius setParticleRadius()
	*/
	virtual NxReal getParticleRadius() const = 0;

	/**
	\brief Gets the soft body density.

	\return density of the soft body.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.density
	*/
	virtual NxReal getDensity() const = 0;

	/**
	\brief Gets the relative grid spacing for the broad phase.
	The soft body is represented by a set of world aligned cubical cells in broad phase.
	The size of these cells is determined by multiplying the length of the diagonal
	of the AABB of the initial soft body size with this constant.

	\return relative grid spacing.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.relativeGridSpacing
	*/
	virtual NxReal getRelativeGridSpacing() const = 0;

	/**
	\brief Retrieves the soft body solver iterations.

	\return solver iterations of the soft body.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.solverIterations setSolverIterations()
	*/
	virtual NxU32 getSolverIterations() const = 0;

	/**
	\brief Sets the soft body solver iterations.

	\param[in] iterations The new solver iteration count for the soft body.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.solverIterations getSolverIterations()
	*/
	virtual void setSolverIterations(NxU32 iterations) = 0;

	/**
	\brief Returns a world space AABB enclosing all soft body particles.

	\param[out] bounds Retrieves the world space bounds.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3
	*/
	virtual void getWorldBounds(NxBounds3& bounds) const = 0;

	/**
	\brief Attaches the soft body to a shape. All soft body vertices currently inside the shape are attached.

	\note This method only works with primitive and convex shapes. Since the inside of a general 
	triangle mesh is not clearly defined.
	\note Collisions with attached shapes are automatically switched off to increase the stability.

	\param[in] shape Shape to which the soft body should be attached to.
	\param[in] attachmentFlags One or two way interaction, tearable or non-tearable
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyAttachmentFlag freeVertex() attachToCollidingShapes()
	*/
	virtual void attachToShape(const NxShape *shape, NxU32 attachmentFlags) = 0;

	/**
	\brief Attaches the soft body to all shapes, currently colliding. 

	\note This method only works with primitive and convex shapes. Since the inside of a general 
	triangle mesh is not clearly defined.
	\note Collisions with attached shapes are automatically switched off to increase the stability.

	\param[in] attachmentFlags One or two way interaction, tearable or non-tearable

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyAttachmentFlag NxSoftBodyDesc.attachmentTearFactor NxSoftBodyDesc.attachmentResponseCoefficient freeVertex()
	*/
	virtual void attachToCollidingShapes(NxU32 attachmentFlags) = 0;

	/**
	\brief Detaches the soft body from a shape it has been attached to before. 

	If the soft body has not been attached to the shape before, the call has no effect.

	\param[in] shape Shape from which the soft body should be detached.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyAttachmentFlag NxSoftBodyDesc.attachmentTearFactor NxSoftBodyDesc.attachmentResponseCoefficient freeVertex() attachToShape()
	*/
	virtual void detachFromShape(const NxShape *shape) = 0;

	/**
	\brief Attaches a soft body vertex to a local position within a shape.

	\param[in] vertexId Index of the vertex to attach.
	\param[in] shape Shape to attach the vertex to.
	\param[in] localPos The position relative to the pose of the shape.
	\param[in] attachmentFlags One or two way interaction, tearable or non-tearable

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxShape freeVertex() NxSoftBodyAttachmentFlag attachToShape()
	*/
	virtual void attachVertexToShape(NxU32 vertexId, const NxShape *shape, const NxVec3 &localPos, NxU32 attachmentFlags) = 0;

	/**
	\brief Attaches a soft body vertex to a position in world space.

	\param[in] vertexId Index of the vertex to attach.
	\param[in] pos The position in world space.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyAttachmentFlag NxSoftBodyDesc.attachmentTearFactor NxSoftBodyDesc.attachmentResponseCoefficient freeVertex() attachToShape()
	*/
	virtual void attachVertexToGlobalPosition(const NxU32 vertexId, const NxVec3 &pos) = 0;

	/**
	\brief Frees a previously attached soft body vertex.

	\param[in] vertexId Index of the vertex to free.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see attachVertexToGlobalPosition() attachVertexToShape() detachFromShape()
	*/
	virtual void freeVertex(const NxU32 vertexId) = 0;

	/**
	\note Experimental feature, not yet fully supported.

	\brief [Experimental] Tears the soft body at a given vertex. 

	First the vertex is duplicated. The tetrahedra on one side of the split plane keep 
	the original vertex. For all tetrahedra on the opposite side the original vertex is 
	replaced by the new one. The split plane is defined by the world location of the 
	vertex and the normal provided by the user.

	Note: TearVertex performs a user defined vertex split in contrast to an automatic split
	that is performed when the flag NX_SBF_TEARABLE is set. Therefore, tearVertex works 
	even if NX_SBF_TEARABLE is not set in NxSoftBodyDesc.flags.

	Note: For tearVertex to work in hardware mode, the softBodyMesh has to be cooked with the
	flag NX_SOFTBODY_MESH_TEARABLE set in NxSoftBodyMeshDesc.flags.

	\param[in] vertexId Index of the vertex to tear.
	\param[in] normal The normal of the split plane.
	\return true if the split had an effect (i.e. there were tetrahedra on both sides of the split plane)

	@see NxSoftBodyFlag, NxSoftBodyMeshFlags, NxSoftBodyDesc.flags

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool tearVertex(const NxU32 vertexId, const NxVec3 &normal) = 0;

	/**
	\brief Executes a raycast against the soft body.

	\param[in] worldRay The ray in world space.
	\param[out] hit The hit position.
	\param[out] vertexId Index to the nearest vertex hit by the raycast.

	\return true if the ray hits the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual bool raycast(const NxRay& worldRay, NxVec3 &hit, NxU32 &vertexId) = 0;

	/**
	\brief Sets which collision group this soft body is part of.

	\param[in] collisionGroup The collision group for this soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCollisionGroup
	*/
	virtual void setGroup(NxCollisionGroup collisionGroup) = 0;

	/**
	\brief Retrieves the value set with #setGroup().

	\return The collision group this soft body belongs to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCollisionGroup
	*/
	virtual NxCollisionGroup getGroup() const = 0;

	/**
	\brief Sets 128-bit mask used for collision filtering.

	\param[in] groupsMask The group mask to set for the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getGroupsMask() NxGroupsMask
	*/
	virtual void setGroupsMask(const NxGroupsMask& groupsMask) = 0;

	/**
	\brief Sets 128-bit mask used for collision filtering.

	\return The group mask for the soft body.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setGroupsMask() NxGroupsMask
	*/
	virtual const NxGroupsMask getGroupsMask() const = 0;

	/**
	\brief Sets the user buffer wrapper for the soft body mesh.

	\param[in,out] meshData User buffer wrapper.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMeshData NxSoftBodyDesc.meshData
	*/
	virtual void setMeshData(NxMeshData& meshData) = 0;

	/**
	\brief Returns a copy of the user buffer wrapper for the soft body mesh.
	
	\return User buffer wrapper.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxMeshData setMeshData() NxSoftBodyDesc.meshData
	*/
	virtual NxMeshData getMeshData() = 0;

	/**
	\brief Sets the user buffer wrapper for the soft body split pairs.

	\param[in,out] splitPairData User buffer wrapper.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodySplitPairData NxSoftBodyDesc.splitPairData
	*/
	virtual	void setSplitPairData(NxSoftBodySplitPairData& splitPairData) = 0;

	/**
	\brief Returns a copy of the user buffer wrapper for the soft body split pairs.

	\return User buffer wrapper.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodySplitPairData setSplitPairData() NxSoftBodyDesc.splitPairData
	*/
	virtual	NxSoftBodySplitPairData	getSplitPairData() = 0;

	/**
	Note: Valid bounds do not have an effect on soft bodies in the current version.

	\brief Sets the valid bounds of the soft body in world space.

	If the flag NX_SBF_VALIDBOUNDS is set, these bounds defines the volume
	outside of which soft body particles are automatically removed from the simulation. 

	\param[in] validBounds The valid bounds.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.validBounds getValidBounds() NxBounds3
	*/
	virtual void setValidBounds(const NxBounds3& validBounds) = 0;

	/**
	Note: Valid bounds do not have an effect on soft bodies in the current version.

	\brief Returns the valid bounds of the soft body in world space.

	\param[out] validBounds The valid bounds.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.validBounds setValidBounds() NxBounds3
	*/
	virtual void getValidBounds(NxBounds3& validBounds) const = 0;

	/**
	\brief Sets the position of a particular vertex of the soft body.

	\param[in] position New position of the vertex.
	\param[in] vertexId Index of the vertex.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getPosition() setPositions() getPositions() setVelocity() getVelocity() getNumberOfParticles()
	*/
	virtual void setPosition(const NxVec3& position, NxU32 vertexId) = 0;

	/**
	\brief Sets the positions of the soft body.

	The user must supply a buffer containing all positions (i.e same number of elements as number of vertices).

	\param[in] buffer The user supplied buffer containing all positions for the soft body.
	\param[in] byteStride The stride in bytes between the position vectors in the buffer. Default is size of NxVec3.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getPositions() setVelocities() getVelocities() getNumberOfParticles()
	*/
	virtual void setPositions(void* buffer, NxU32 byteStride = sizeof(NxVec3)) = 0;

	/**
	\brief Gets the position of a particular vertex of the soft body.

	\param[in] vertexId Index of the vertex.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setPosition() setPositions() getPositions() setVelocity() getVelocity() getNumberOfParticles()
	*/
	virtual NxVec3 getPosition(NxU32 vertexId) const = 0;

	/**
	\brief Gets the positions of the soft body.

	The user must supply a buffer large enough to hold all positions (i.e same number of elements as number of particles).

	\param[in] buffer The user supplied buffer to hold all positions of the soft body.
	\param[in] byteStride The stride in bytes between the position vectors in the buffer. Default is size of NxVec3.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setPositions() setVelocities() getVelocities() getNumberOfParticles()
	*/
	virtual void getPositions(void* buffer, NxU32 byteStride = sizeof(NxVec3)) = 0;

	/**
	\brief Sets the velocity of a particular vertex of the soft body.

	\param[in] position New velocity of the vertex.
	\param[in] vertexId Index of the vertex.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setPosition() getPosition() getVelocity() setVelocities() getVelocities()  getNumberOfParticles()
	*/
	virtual void setVelocity(const NxVec3& velocity, NxU32 vertexId) = 0;

	/**
	\brief Sets the velocities of the soft body.

	The user must supply a buffer containing all velocities (i.e same number of elements as number of vertices).

	\param[in] buffer The user supplied buffer containing all velocities for the soft body.
	\param[in] byteStride The stride in bytes between the velocity vectors in the buffer. Default is size of NxVec3.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getVelocities() setPositions() getPositions() getNumberOfParticles()
	*/
	virtual void setVelocities(void* buffer, NxU32 byteStride = sizeof(NxVec3)) = 0;

	/**
	\brief Gets the velocity of a particular vertex of the soft body.

	\param[in] vertexId Index of the vertex.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setPosition() getPosition() setVelocity() setVelocities() getVelocities()  getNumberOfParticles()
	*/
	virtual NxVec3 getVelocity(NxU32 vertexId) const = 0;

	/**
	\brief Gets the velocities of the soft body.

	The user must supply a buffer large enough to hold all velocities (i.e same number of elements as number of vertices).

	\param[in] buffer The user supplied buffer to hold all velocities of the soft body.
	\param[in] byteStride The stride in bytes between the velocity vectors in the buffer. Default is size of NxVec3.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setVelocities() setPositions() getPositions() getNumberOfParticles()
	*/
	virtual void getVelocities(void* buffer, NxU32 byteStride = sizeof(NxVec3)) = 0;

	/**
	\brief Gets the number of soft body particles.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setVelocities() getVelocities() setPositions() getPositions() 
	*/
	virtual NxU32 getNumberOfParticles() = 0;

	/**
	\brief Queries the soft body for the currently interacting shapes. Must be called prior to saveStateToStream in order for attachments and collisons to be saved.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getShapePointers() setShapePointers() saveStateToStream() loadStateFromStream()
	*/
	virtual NxU32 queryShapePointers() = 0;

	/**
	\brief Gets the byte size of the current soft body state.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getNumberOfParticles()
	*/
	virtual NxU32 getStateByteSize() = 0;

	/**
	\brief Saves pointers to the currently interacting shapes to memory

	\param[in] shapePointers The user supplied array to hold the shape pointers.
	\param[in] flags The optional user supplied array to hold the attachment shape flags for the various shapes.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see queryShapePointers() setShapePointers() saveStateToStream() loadStateFromStream()
	*/
	virtual void getShapePointers(NxShape** shapePointers,NxU32 *flags) = 0;

	/**
	\brief Loads pointers to the currently interacting shapes from memory.

	\param[in] shapePointers The user supplied array that holds the shape pointers. Must be in the exact same order as the shapes were retrieved by getShapePointers.
	\param[in] numShapes The size of the supplied array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see queryShapePointers() getShapePointers() saveStateToStream() loadStateFromStream()
	*/
	virtual void setShapePointers(NxShape** shapePointers,unsigned int numShapes) = 0;

	/**
	\brief Saves the current soft body state to a stream. 
	
	queryShapePointers must be called prior to this function in order for attachments and collisions to be saved. 
	Tearable soft bodies are currently not supported.
	A saved state contains platform specific data and can thus only be loaded on back on the same platform.

	\param[in] stream The user supplied stream to hold the soft body state.
	\param[in] permute If true, the order of the vertices output will correspond to that of the associated
	NxSoftBodyMesh's saveToDesc mehod; if false (the default), it will correspond to the original NxSoftBodyMesh descriptor
	used to create the mesh. These may differ due to cooking.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see loadStateFromStream() queryShapePointers() getShapePointers() setShapePointers() 
	*/
	virtual void saveStateToStream(NxStream& stream, bool permute = false) = 0;

	/**
	\brief Loads the current soft body state from a stream. 
	
	setShapePointers must be called prior to this function if attachments and collisions are to be loaded. 
	Tearable soft bodies are currently not supported.
	A saved state contains platform specific data and can thus only be loaded on back on the same platform.

	\param[in] stream The user supplied stream that holds the soft body state.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see saveStateToStream() queryShapePointers() getShapePointers() setShapePointers() 
	*/
	virtual void loadStateFromStream(NxStream& stream) = 0;

	/**
	\brief Sets the collision response coefficient.

	\param[in] coefficient The collision response coefficient (0 or greater).
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.collisionResponseCoefficient getCollisionResponseCoefficient()
	*/
	virtual void setCollisionResponseCoefficient(NxReal coefficient) = 0;

	/**
	\brief Retrieves the collision response coefficient.

	\return The collision response coefficient.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.collisionResponseCoefficient setCollisionResponseCoefficient()
	*/
	virtual NxReal getCollisionResponseCoefficient() const = 0;

	/**
	\brief Sets the attachment response coefficient

	\param[in] coefficient The attachment response coefficient in the range from 0 to 1.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.attachmentResponseCoefficient getAttachmentResponseCoefficient()
	*/
	virtual void setAttachmentResponseCoefficient(NxReal coefficient) = 0;

	/**
	\brief Retrieves the attachment response coefficient

	\return The attachment response coefficient.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.attachmentResponseCoefficient setAttachmentResponseCoefficient()
	*/
	virtual NxReal getAttachmentResponseCoefficient() const = 0;

	/**
	\brief Sets the response coefficient for collisions from fluids to this soft body

	\param[in] coefficient The response coefficient 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.fromFluidResponseCoefficient getFromFluidResponseCoefficient()
	*/
	virtual void setFromFluidResponseCoefficient(NxReal coefficient) = 0;

	/**
	\brief Retrieves response coefficient for collisions from fluids to this soft body

	\return The response coefficient.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.fromFluidResponseCoefficient setFromFluidResponseCoefficient()
	*/
	virtual NxReal getFromFluidResponseCoefficient() const = 0;

	/**
	\brief Sets the response coefficient for collisions from this soft body to fluids

	\param[in] coefficient The response coefficient 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.toFluidResponseCoefficient getToFluidResponseCoefficient()
	*/
	virtual void setToFluidResponseCoefficient(NxReal coefficient) = 0;

	/**
	\brief Retrieves response coefficient for collisions from this soft body to fluids

	\return The response coefficient.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.toFluidResponseCoefficient setToFluidResponseCoefficient()
	*/
	virtual NxReal getToFluidResponseCoefficient() const = 0;

	/**
	\brief Sets an external acceleration which affects all non attached particles of the soft body

	\param[in] acceleration The acceleration vector (unit length / s^2)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.externalAcceleration getExternalAcceleration()
	*/
	virtual void setExternalAcceleration(NxVec3 acceleration) = 0;

	/**
	\brief Retrieves the external acceleration which affects all non attached particles of the soft body

	\return The acceleration vector (unit length / s^2)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.externalAcceleration setExternalAcceleration()
	*/
	virtual NxVec3 getExternalAcceleration() const = 0;

	/**
	\brief If the NX_SBF_ADHERE flag is set the soft body moves partially in the frame 
	of the attached actor. 

	This feature is useful when the soft body is attached to a fast moving shape.
	In that case the soft body adheres to the shape it is attached to while only 
	velocities below the parameter minAdhereVelocity are used for secondary effects.

	\param[in] velocity The minimal velocity for the soft body to adhere (unit length / s)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.minAdhereVelocity getMinAdhereVelocity()
	*/
	virtual void setMinAdhereVelocity(NxReal velocity) = 0;

	/**
	\brief If the NX_SBF_ADHERE flag is set the soft body moves partially in the frame 
	of the attached actor. 

	This feature is useful when the soft body is attached to a fast moving shape.
	In that case the soft body adheres to the shape it is attached to while only 
	velocities below the parameter minAdhereVelocity are used for secondary effects.

	\return Returns the minimal velocity for the soft body to adhere (unit length / s)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.minAdhereVelocity setMinAdhereVelocity()
	*/
	virtual NxReal getMinAdhereVelocity() const = 0;

	/**
	\brief Returns true if this soft body is sleeping.

	When a soft body does not move for a period of time, it is no longer simulated in order to save time. This state
	is called sleeping. However, because the object automatically wakes up when it is either touched by an awake object,
	or one of its properties is changed by the user, the entire sleep mechanism should be transparent to the user.
	
	If a soft body is asleep after the call to NxScene::fetchResults() returns, it is guaranteed that the position of the soft body 
	vertices was not changed. You can use this information to avoid updating dependent objects.
	
	\return True if the soft body is sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isSleeping() getSleepLinearVelocity() wakeUp() putToSleep()
	*/
	virtual bool isSleeping() const = 0;

	/**
	\brief Returns the linear velocity below which a soft body may go to sleep.
	
	A soft body whose linear velocity is above this threshold will not be put to sleep.
    
    @see isSleeping

	\return The threshold linear velocity for sleeping.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isSleeping() getSleepLinearVelocity() wakeUp() putToSleep() setSleepLinearVelocity()
	*/
    virtual NxReal getSleepLinearVelocity() const = 0;

    /**
	\brief Sets the linear velocity below which a soft body may go to sleep.
	
	A soft body whose linear velocity is above this threshold will not be put to sleep.
	
	If the threshold value is negative,	the velocity threshold is set using the NxPhysicsSDK's 
	NX_DEFAULT_SLEEP_LIN_VEL_SQUARED parameter.
    
	\param[in] threshold Linear velocity below which a soft body may sleep. <b>Range:</b> (0,inf]

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isSleeping() getSleepLinearVelocity() wakeUp() putToSleep()
	*/
    virtual void setSleepLinearVelocity(NxReal threshold) = 0;

	/**
	\brief Wakes up the soft body if it is sleeping.  

	The wakeCounterValue determines how long until the soft body is put to sleep, a value of zero means 
	that the soft body is sleeping. wakeUp(0) is equivalent to NxSoftBody::putToSleep().

	\param[in] wakeCounterValue New sleep counter value. <b>Range:</b> [0,inf]
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isSleeping() getSleepLinearVelocity() putToSleep()
	*/
	virtual void wakeUp(NxReal wakeCounterValue = NX_SLEEP_INTERVAL) = 0;

	/**
	\brief Forces the soft body to sleep. 
	
	The soft body will fall asleep.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see isSleeping() getSleepLinearVelocity() wakeUp()
	*/
	virtual void putToSleep() = 0;

	/**
	\brief Sets the flags, a combination of the bits defined by the enum ::NxSoftBodyFlag.

	\param[in] flags #NxSoftBodyFlag combination.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.flags NxSoftBodyFlag getFlags()
	*/
	virtual void setFlags(NxU32 flags) = 0;

	/**
	\brief Retrieves the flags.

	\return The soft body flags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxSoftBodyDesc.flags NxSoftBodyFlag setFlags()
	*/
	virtual NxU32 getFlags() const = 0;

	/**
	\brief Sets a name string for the object that can be retrieved with getName().
	
	This is for debugging and is not used by the SDK. The string is not copied by 
	the SDK, only the pointer is stored.

	\param[in] name String to set the objects name to.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see getName()
	*/
	virtual void setName(const char* name) = 0;

	/**
	\brief Applies a force (or impulse) defined in the global coordinate frame, to a particular 
	vertex of the soft body. 

	Because forces are reset at the end of every timestep, 
	you can maintain a total external force on an object by calling this once every frame.

    ::NxForceMode determines if the force is to be conventional or impulsive.

	\param[in] force Force/impulse to add, defined in the global frame. <b>Range:</b> force vector
	\param[in] vertexId Number of the vertex to add the force at. <b>Range:</b> position vector
	\param[in] mode The mode to use when applying the force/impulse 
	(see #NxForceMode, supported modes are NX_FORCE, NX_IMPULSE, NX_ACCELERATION, NX_VELOCITY_CHANGE)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	*/
	virtual	void addForceAtVertex(const NxVec3& force, NxU32 vertexId, NxForceMode mode = NX_FORCE) = 0;

	/**
	\brief Applies a radial force (or impulse) at a particular position. All vertices
	within radius will be affected with a quadratic drop-off. 

	Because forces are reset at the end of every timestep, 
	you can maintain a total external force on an object by calling this once every frame.

    ::NxForceMode determines if the force is to be conventional or impulsive.

	\param[in] position Position to apply force at.
	\param[in] magnitude Magnitude of the force/impulse to apply.
	\param[in] radius The sphere radius in which particles will be affected. <b>Range:</b> position vector
	\param[in] mode The mode to use when applying the force/impulse 
	(see #NxForceMode, supported modes are NX_FORCE, NX_IMPULSE, NX_ACCELERATION, NX_VELOCITY_CHANGE).

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxForceMode 
	*/
	virtual	void addForceAtPos(const NxVec3& position, NxReal magnitude, NxReal radius, NxForceMode mode = NX_FORCE) = 0;

	/**
	\brief Finds tetrahedra touching the input bounds.

	\warning This method returns a pointer to an internal structure using the indices member. Hence the
	user should use or copy the indices before calling any other API function.

	\param[in] bounds Bounds to test against in world space. <b>Range:</b> See #NxBounds3
	\param[out] nb Retrieves the number of tetrahedral indices touching the AABB.
	\param[out] indices Returns an array of touching tetrahedra indices. 
	The tetrahedral indices correspond to the tetrahedra referenced to by NxSoftBodyDesc.meshdata (#NxMeshData).
	Tetrahedron i has the vertices 4i, 4i+1, 4i+2 and 4i+3  in the array NxMeshData.indicesBegin.
	\return True if there is an overlap.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxBounds3 NxSoftBodyDesc NxMeshData
	*/
	virtual	bool overlapAABBTetrahedra(const NxBounds3& bounds, NxU32& nb, const NxU32*& indices) const = 0;

	/**
	\brief Retrieves the scene which this soft body belongs to.

	\return Owner Scene.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxScene
	*/
	virtual NxScene& getScene() const = 0;

	/**
	\brief Retrieves the name string set with setName().

	\return Name string associated with object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see setName()
	*/
	virtual const char* getName() const = 0;

	/**
	\brief Retrieves the soft body's simulation compartment, as specified by the user at creation time.
	\return NULL if the soft body is not simulated in a compartment or if it was specified to run in 
	the default soft body compartment, otherwise the simulation compartment.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see NxCompartment
	*/
	virtual NxCompartment *			getCompartment() const = 0;

	/**
	\brief Retrieves the actor's force field material index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual NxForceFieldMaterial	getForceFieldMaterial() const = 0;

	/**
	\brief Sets the actor's force field material index, default index is 0

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes [SW fallback]
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual void					setForceFieldMaterial(NxForceFieldMaterial)  = 0;

	//public variables:
public:
	void* userData; //!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
};
/** @} */
#endif

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
