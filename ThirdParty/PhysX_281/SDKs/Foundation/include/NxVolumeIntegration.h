#ifndef NX_FOUNDATION_NXVOLUMEINTEGRATION
#define NX_FOUNDATION_NXVOLUMEINTEGRATION
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup foundation
  @{
*/


#include "Nx.h"
#include "NxVec3.h"
#include "NxMat33.h"

class NxSimpleTriangleMesh;

/**
\brief Data structure used to store mass properties.
*/
struct NxIntegrals
	{
	NxVec3 COM;					//!< Center of mass
	NxF64 mass;						//!< Total mass
	NxF64 inertiaTensor[3][3];		//!< Inertia tensor (mass matrix) relative to the origin
	NxF64 COMInertiaTensor[3][3];	//!< Inertia tensor (mass matrix) relative to the COM

	/**
	\brief Retrieve the inertia tensor relative to the center of mass.

	\param inertia Inertia tensor.
	*/
	void getInertia(NxMat33& inertia)
	{
		for(NxU32 j=0;j<3;j++)
		{
			for(NxU32 i=0;i<3;i++)
			{
				inertia(i,j) = (NxF32)COMInertiaTensor[i][j];
			}
		}
	}

	/**
	\brief Retrieve the inertia tensor relative to the origin.

	\param inertia Inertia tensor.
	*/
	void getOriginInertia(NxMat33& inertia)
	{
		for(NxU32 j=0;j<3;j++)
		{
			for(NxU32 i=0;i<3;i++)
			{
				inertia(i,j) = (NxF32)inertiaTensor[i][j];
			}
		}
	}
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
