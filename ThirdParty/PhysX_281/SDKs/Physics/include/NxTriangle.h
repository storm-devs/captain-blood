#ifndef NX_COLLISION_NXTRIANGLE
#define NX_COLLISION_NXTRIANGLE
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

	#define	NX_INV3		0.33333333333333333333f		//!< 1/3

/**
\brief Triangle class.

@see NxTriangleMesh NxTriangleMeshDesc
*/
	class NxTriangle
	{
		public:
		/**
		\brief Constructor
		*/
		NX_INLINE			NxTriangle()
			{
			}
		/**
		\brief Constructor

		\param[in] p0 Point 0
		\param[in] p1 Point 1
		\param[in] p2 Point 2
		*/
		NX_INLINE			NxTriangle(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2)
			{
				verts[0] = p0;
				verts[1] = p1;
				verts[2] = p2;
			}
		/**
		\brief Copy constructor

		\param[in] triangle Tri to copy
		*/
		NX_INLINE			NxTriangle(const NxTriangle& triangle)
			{
				verts[0] = triangle.verts[0];
				verts[1] = triangle.verts[1];
				verts[2] = triangle.verts[2];
			}
		/**
		\brief Destructor
		*/
		NX_INLINE			~NxTriangle()
			{
			}

		/**
		\brief Array of Vertices.
		*/
				NxVec3		verts[3];

		/**
		\brief Compute the center of the NxTriangle.

		\param[out] center Retrieve center (average) point of triangle.
		*/
		NX_INLINE	void	center(NxVec3& center) const
			{
				center = (verts[0] + verts[1] + verts[2])*NX_INV3;
			}

		/**
		\brief Compute the normal of the NxTriangle.

		\param[out] _normal Triangle normal.
		*/
		NX_INLINE	void	normal(NxVec3& _normal) const
			{
				_normal = (verts[1]-verts[0])^(verts[2]-verts[0]);
				_normal.normalize();
			}

		/**
		\brief Makes a fat triangle

		\param[in] fatCoeff Amount to inflate triangle by.
		\param[in] constantBorder True - Add same width independent of triangle size. False - Add bigger borders to biggers triangles.
		*/
		NX_INLINE	void	inflate(float fatCoeff, bool constantBorder)
			{
			// Compute triangle center
			NxVec3 triangleCenter;
			center(triangleCenter);

			// Don't normalize?
			// Normalize => add a constant border, regardless of triangle size
			// Don't => add more to big triangles
			for(unsigned i=0;i<3;i++)
				{
				NxVec3 v = verts[i] - triangleCenter;

				if(constantBorder)	v.normalize();

				verts[i] += v * fatCoeff;
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
