#ifndef NX_PHYSICS_NXHEIGHTFIELDSAMPLE
#define NX_PHYSICS_NXHEIGHTFIELDSAMPLE
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

/**
\brief Heightfield sample format.

This format corresponds to the #NxHeightFieldFormat member NX_HF_S16_TM.

An array of heightfield samples are used when creating a NxHeightField to specify
the elevation of the heightfield points. In addition the material and tessellation of the adjacent 
triangles are specified.

@see NxHeightField NxHeightFieldDesc NxHeightFieldDesc.samples
*/
struct NxHeightFieldSample
	{
	/**
	\brief The height of the heightfield sample

	This value is scaled by NxHeightFieldShapeDesc::heightScale.

	@see NxHeightFieldShapeDesc
	*/
	NxI16			height			: 16;

	/**
	\brief The low 7 bits of a triangle material index.

	These low bits are concatenated with the high bits from NxHeightFieldShapeDesc::materialIndexHighBits,
	to produce an index into the scene's material array. This index determines the material of the lower
	of the quad's two triangles (i.e. the quad whose upper-left corner is this sample, see the Guide for illustrations).

	@see NxHeightFieldShapeDesc materialIndex1
	*/
	NxU8			materialIndex0	: 7;

	/**
	\brief Tessellation flag.

	This flag specifies which way the quad is split whose upper left corner is this sample.
	If the flag is set, the diagonal of the quad will run from this sample to the opposite vertex; if not,
	it will run between the other two vertices (see the Guide for illustrations).
	*/
	NxU8			tessFlag		: 1;

	/**
	\brief The low 7 bits of a triangle material index.

	These low bits are concatenated with the high bits from NxHeightFieldShapeDesc::materialIndexHighBits,
	to produce an index into the scene's material array. This index determines the material of the upper
	of the quad's two triangles (i.e. the quad whose upper-left corner is this sample, see the Guide for illustrations).

	@see NxHeightFieldShapeDesc materialIndex0
	*/
	NxU8			materialIndex1	: 7;

	/**
	\brief Reserved for future use. Should be set to zero.
	*/
	NxU8			unused			: 1;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
