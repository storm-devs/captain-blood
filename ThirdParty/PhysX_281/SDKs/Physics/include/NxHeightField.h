#ifndef NX_COLLISION_NXHEIGHTFIELD
#define NX_COLLISION_NXHEIGHTFIELD
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

class NxHeightFieldDesc;

/**
\brief A height field object.  

Height fields work in a similar way as triangle meshes specified to act as height fields, with some important differences:

Triangle meshes can be made of nonuniform geometry, while height fields are regular, rectangular grids.
This means that with NxHeightField, you sacrifice flexibility in return for improved performance and decreased memory consumption.

Like Convexes and TriangleMeshes, HeightFields are referenced by shape instances of type NxHeightFieldShape.

To avoid duplicating data when you have several instances of a particular 
height field differently, you do not use this class to represent a 
height field object directly. Instead, you create an instance of this height field 
via the NxHeightFieldShape class.

<h3>Creation</h3>

To create an instance of this class call NxPhysicsSDK::createHeightField(),
and NxPhysicsSDK::releaseHeightField() to delete it. This is only possible
once you have released all of its NxHeightFiedShape instances.

Example:

\include NxHeightField_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_COLLISION_AABBS
\li #NX_VISUALIZE_COLLISION_SHAPES
\li #NX_VISUALIZE_COLLISION_AXES
\li #NX_VISUALIZE_COLLISION_VNORMALS
\li #NX_VISUALIZE_COLLISION_FNORMALS
\li #NX_VISUALIZE_COLLISION_EDGES
\li #NX_VISUALIZE_COLLISION_SPHERES

@see NxHeightFieldDesc NxHeightFieldShape NxPhysicsSDK.createHeightField()
*/

class NxHeightField
	{
	public:

	/**
	\brief Saves the HeightField descriptor.
	
	This does not save out the cells member of the desc because the user must provide destination memory for that.
	Instead, use the saveCells method obtain the sample data.

	\param[out] desc The descriptor used to retrieve the state of the object.

	\return True on success.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		bool						saveToDesc(NxHeightFieldDesc& desc)	const	= 0;

	/**
	\brief Load the height field from a description.

    \param[in] desc The descriptor to load the object from.

	\return True if the height field was successfully loaded.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc
	*/
	virtual		bool						loadFromDesc(const NxHeightFieldDesc& desc)			= 0;

	/**
    \brief Writes out the sample data array.
	
	The user provides destBufferSize bytes storage at destBuffer.
	The data is formatted and arranged as NxHeightFieldDesc.samples.

	\param[out] destBuffer The destination buffer for the sample data.
	\param[in] destBufferSize The size of the destination buffer.
	\return The number of bytes written.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.samples
	*/
    virtual		NxU32						saveCells(void * destBuffer, NxU32 destBufferSize) const = 0;

	/**
	\brief Retrieves the number of sample rows in the samples array.

	\return The number of sample rows in the samples array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.nbRows
	*/
	virtual		NxU32						getNbRows()					const = 0;

	/**
	\brief Retrieves the number of sample columns in the samples array.

	\return The number of sample columns in the samples array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.nbColumns
	*/
	virtual		NxU32						getNbColumns()				const = 0;

	/**
	\brief Retrieves the format of the sample data.
	
	\return The format of the sample data.
	
	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.format NxHeightFieldFormat
	*/
	virtual		NxHeightFieldFormat			getFormat()					const = 0;

	/**
	\brief Retrieves the offset in bytes between consecutive samples in the array.

	\return The offset in bytes between consecutive samples in the array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.sampleStride
	*/
	virtual		NxU32						getSampleStride()			const = 0;

	/**
	\brief Deprecated: Retrieves the extent of the height volume in the vertical direction.

	\return The extent of the height volume in the vertical direction.

	<b>Platform:</b>
	\li PC SW: Deprecated
	\li PPU  : Deprecated
	\li PS3  : Deprecated
	\li XB360: Deprecated

	@see NxHeightFieldDesc.verticalExtent
	*/
	virtual		NxReal						getVerticalExtent()			const = 0;

	/**
	\brief Retrieves the thickness of the height volume in the vertical direction.

	\return The thickness of the height volume in the vertical direction.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.thickness
	*/
	virtual		NxReal						getThickness()			const = 0;

	/**
	\brief Retrieves the convex edge threshold.

	\return The convex edge threshold.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.convexEdgeThreshold
	*/
	virtual		NxReal						getConvexEdgeThreshold()	const = 0;

	/**
	\brief Retrieves the flags bits, combined from values of the enum ::NxHeightFieldFlags.

	\return The flags bits, combined from values of the enum ::NxHeightFieldFlags.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.flags NxHeightFieldFlags
	*/
	virtual		NxU32						getFlags()					const = 0;

	/**
	\brief Retrieves the height at the given coordinates in grid space.
	\return The height at the given coordinates or 0 if the coordinates are out of range.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxReal						getHeight(NxReal x, NxReal z) const = 0;

	/**
	\brief Returns a read only pointer directly to the samples array.
	The data format is identical to that in NxHeightFieldDesc.samples.

	\return A const void pointer to the samples array.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes (Software fallback)
	\li PS3  : Yes
	\li XB360: Yes

	@see NxHeightFieldDesc.samples
	*/
	virtual		const void*					getCells()					const = 0;

	/**
	\brief Returns the reference count for shared meshes.

	\return the current reference count, not used in any shapes if the count is 0.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	virtual		NxU32						getReferenceCount()					= 0;


	protected:
	virtual ~NxHeightField(){};
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
