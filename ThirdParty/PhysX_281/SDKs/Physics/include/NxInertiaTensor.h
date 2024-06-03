#ifndef NX_PHYSICS_NXINERTIATENSOR
#define NX_PHYSICS_NXINERTIATENSOR
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
#include "PhysXLoader.h"

	/**
	\brief Computes mass of a homogeneous sphere according to sphere density.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] radius Radius of the sphere. <b>Range:</b> (0,inf)
	\param[in] density Density of the sphere. <b>Range:</b> (0,inf)

	\return The mass of the sphere.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeSphereMass			(NxReal radius, NxReal density)
		{
		return NxGetUtilLib()->NxComputeSphereMass(radius,density);
		}

	/**
	\brief Computes density of a homogeneous sphere according to sphere mass.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] radius Radius of the sphere. <b>Range:</b> (0,inf)
	\param[in] mass Mass of the sphere. <b>Range:</b> (0,inf)

	\return The density of the sphere.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeSphereDensity		(NxReal radius, NxReal mass)
		{
		return NxGetUtilLib()->NxComputeSphereDensity(radius,mass);
		}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous box according to box density.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] extents The extents/radii, that is the full side length along each axis, of the box. <b>Range:</b> direction vector
	\param[in] density The density of the box. <b>Range:</b> (0,inf)

	\return The mass of the box.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeBoxMass			(const NxVec3& extents, NxReal density)
		{
		return NxGetUtilLib()->NxComputeBoxMass(extents,density);
		}
	
	/**
	\brief Computes density of a homogeneous box according to box mass.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] extents The extents/radii, that is the full side length along each axis, of the box. <b>Range:</b> direction vector
	\param[in] mass The mass of the box. <b>Range:</b> (0,inf)

	\return  The density of the box.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeBoxDensity			(const NxVec3& extents, NxReal mass)
		{
		return NxGetUtilLib()->NxComputeBoxDensity(extents,mass);
		}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous ellipsoid according to ellipsoid density.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] extents The extents/radii of the ellipsoid. <b>Range:</b> direction vector
	\param[in] density The density of the ellipsoid. <b>Range:</b> (0,inf)

	\return The mass of the ellipsoid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeEllipsoidMass		(const NxVec3& extents, NxReal density)
		{
		return NxGetUtilLib()->NxComputeEllipsoidMass(extents,density);
		}
	
	/**
	\brief Computes density of a homogeneous ellipsoid according to ellipsoid mass.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] extents The extents/radii of the ellipsoid. <b>Range:</b> direction vector
	\param[in] mass The mass of the ellipsoid. <b>Range:</b> (0,inf)

	\return The density of the ellipsoid.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeEllipsoidDensity	(const NxVec3& extents, NxReal mass)
		{
		return NxGetUtilLib()->NxComputeEllipsoidDensity(extents,mass);
		}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous cylinder according to cylinder density.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] radius The radius of the cylinder. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] density The density. <b>Range:</b> (0,inf)

	\return The mass.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeCylinderMass		(NxReal radius, NxReal length, NxReal density)
		{
		return NxGetUtilLib()->NxComputeCylinderMass(radius,length,density);
		}
	
	/**
	\brief Computes density of a homogeneous cylinder according to cylinder mass.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] radius The radius of the cylinder. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] mass The mass. <b>Range:</b> (0,inf)

	\return The density.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeCylinderDensity	(NxReal radius, NxReal length, NxReal mass)
		{
		return NxGetUtilLib()->NxComputeCylinderDensity(radius,length,mass);
		}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes mass of a homogeneous cone according to cone density.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] radius The radius of the cone. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] density The density. <b>Range:</b> (0,inf)

	\return The mass.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeConeMass			(NxReal radius, NxReal length, NxReal density)
		{
		return NxGetUtilLib()->NxComputeConeMass(radius,length,density);
		}
	
	/**
	\brief Computes density of a homogeneous cone according to cone mass.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[in] radius The radius of the cone. <b>Range:</b> (0,inf)
	\param[in] length The length. <b>Range:</b> (0,inf)
	\param[in] mass The mass. <b>Range:</b> (0,inf)

	\return The density.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE NxReal NxComputeConeDensity		(NxReal radius, NxReal length, NxReal mass)
		{
		return NxGetUtilLib()->NxComputeConeDensity(radius,length,mass);
		}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	\brief Computes diagonalized inertia tensor for a box.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[out] diagInertia The diagonalized inertia tensor.
	\param[in] mass The mass of the box. <b>Range:</b> (0,inf)
	\param[in] xlength The width of the box. <b>Range:</b> (-inf,inf)
	\param[in] ylength The height. <b>Range:</b> (-inf,inf)
	\param[in] zlength The depth. <b>Range:</b> (-inf,inf)

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE void NxComputeBoxInertiaTensor	(NxVec3& diagInertia, NxReal mass, NxReal xlength, NxReal ylength, NxReal zlength)
		{
		NxGetUtilLib()->NxComputeBoxInertiaTensor(diagInertia,mass,xlength,ylength,zlength);
		}
	
	/**
	\brief Computes diagonalized inertia tensor for a sphere.

	\warning #NxCreatePhysicsSDK() must be called before using this function.

	\param[out] diagInertia The diagonalized inertia tensor.
	\param[in] mass The mass. <b>Range:</b> (0,inf)
	\param[in] radius The radius. <b>Range:</b> (-inf,inf)
	\param[in] hollow True to treat the sphere as a hollow shell. 

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	*/
	NX_INLINE void NxComputeSphereInertiaTensor(NxVec3& diagInertia, NxReal mass, NxReal radius, bool hollow)
		{
		NxGetUtilLib()->NxComputeSphereInertiaTensor(diagInertia,mass,radius,hollow);
		}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
