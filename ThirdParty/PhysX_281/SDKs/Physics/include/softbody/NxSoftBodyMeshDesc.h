#ifndef NX_PHYSICS_NX_SOFTBODYMESHDESC
#define NX_PHYSICS_NX_SOFTBODYMESHDESC
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

/**
\brief Soft body mesh flags.

@see NxSoftBodyMeshDesc.flags
*/
enum NxSoftBodyMeshFlags
	{
	NX_SOFTBODY_MESH_16_BIT_INDICES	=	(1<<1),	//<! Denotes the use of 16-bit vertex indices

	/**
	Not supported in current release.

	\brief Specifies whether extra space is allocated for tearing on the PPU.
	If this flag is not set, less memory is needed on the PPU but tearing is not possible.
	*/
	NX_SOFTBODY_MESH_TEARABLE	=	(1<<2),
	};

/**
\brief Soft body vertex flags.

@see NxSoftBodyMeshDesc.vertexFlags
*/
enum NxSoftBodyVertexFlags
	{
	/**
	\brief Specifies whether a soft body vertex can be torn.
	*/
	NX_SOFTBODY_VERTEX_TEARABLE	=	(1<<7),
	};

/*----------------------------------------------------------------------------*/

/**
\brief Descriptor class for #NxSoftBodyMesh.

The mesh data is *copied* when an NxSoftBodyMesh object is created from this descriptor. 
After the creation the user may discard the basic mesh data.

@see NxSoftBodyMesh
*/
class NxSoftBodyMeshDesc
{
public:
	NxU32 numVertices;				//!< Number of vertices.
	NxU32 numTetrahedra;			//!< Number of tetrahedra.
	NxU32 vertexStrideBytes;		//!< Offset between vertex points in bytes.
	NxU32 tetrahedronStrideBytes;	//!< Offset between tetrahedra in bytes.

	/**
	\brief Pointer to first vertex point.
	
	Caller may add vertexStrideBytes bytes to the pointer to access the next point.
	*/
	const void* vertices;

	/**
	\brief Pointer to first tetrahedron.
	
	Caller may add tetrahedronStrideBytes bytes to the pointer to access the next tetrahedron.
	
	These are quadruples of 0 based indices:
	vert0 vert1 vert2 vert3
	vert0 vert1 vert2 vert3
	vert0 vert1 vert2 vert3
	...

	where vertex is either a 32 or 16 bit unsigned integer. There are numTetrahedra*4 indices.

	This is declared as a void pointer because it is actually either an NxU16 or a NxU32 pointer.
	*/
	const void* tetrahedra;

	/**
	\brief Flags bits, combined from values of the enum ::NxSoftBodyMeshFlags
	*/
	NxU32 flags;


	NxU32 vertexMassStrideBytes;		//!< Offset between vertex masses in bytes.
	NxU32 vertexFlagStrideBytes;		//!< Offset between vertex flags in bytes.
	
	/**
	\brief Pointer to first vertex mass.
	
	Caller may add vertexMassStrideBytes bytes to the pointer to access the next vertex mass.
	*/
	const void* vertexMasses;

	/**
	\brief Pointer to first vertex flag. Flags are of type #NxSoftBodyVertexFlags
	
	Caller may add vertexFlagStrideBytes bytes to the pointer to access the next vertex flag.
	*/
	const void* vertexFlags;

	/**
	\brief Constructor sets to default.
	*/
	NX_INLINE NxSoftBodyMeshDesc();

	/**
	\brief (Re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();

	/**
	\brief Returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
};

/*----------------------------------------------------------------------------*/

NX_INLINE NxSoftBodyMeshDesc::NxSoftBodyMeshDesc()
{
	setToDefault();
}

/*----------------------------------------------------------------------------*/

NX_INLINE void NxSoftBodyMeshDesc::setToDefault()
{
	numVertices				= 0;
	numTetrahedra			= 0;
	vertexStrideBytes		= 0;
	tetrahedronStrideBytes	= 0;
	vertices				= NULL;
	tetrahedra				= NULL;
	flags					= 0;

	vertexMassStrideBytes	= sizeof(NxReal);
	vertexFlagStrideBytes	= sizeof(NxU32);
	vertexMasses			= NULL;
	vertexFlags				= NULL;
}

/*----------------------------------------------------------------------------*/

NX_INLINE bool NxSoftBodyMeshDesc::isValid() const
{
	// Check geometry
	if(numVertices > 0xffff && flags & NX_SOFTBODY_MESH_16_BIT_INDICES)
		return false;
	if(!vertices)
		return false;
	if(vertexStrideBytes < sizeof(NxPoint))	//should be at least one point's worth of data
		return false;

	if(!tetrahedra) 
		return false;

	if(flags & NX_SOFTBODY_MESH_16_BIT_INDICES)
		{
		if((tetrahedronStrideBytes < sizeof(NxU16)*3))
			return false;
		}
	else
		{
		if((tetrahedronStrideBytes < sizeof(NxU32)*3))
			return false;
		}
		
	if(vertexMasses && (vertexMassStrideBytes < sizeof(NxReal)))
		return false;
	if(vertexFlags && (vertexFlagStrideBytes < sizeof(NxU32)))
		return false;

	return true;
}

/*----------------------------------------------------------------------------*/
/** @} */
#endif

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
