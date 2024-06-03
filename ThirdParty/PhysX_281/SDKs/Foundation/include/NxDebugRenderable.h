#ifndef NX_FOUNDATION_NXDEBUGRENDERABLE
#define NX_FOUNDATION_NXDEBUGRENDERABLE
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
#include "NxBox.h"
#include "NxBounds3.h"

/**
\brief Default color values used for debug rendering.
*/
enum NxDebugColor
	{
	NX_ARGB_BLACK	= 0xff000000,
	NX_ARGB_RED		= 0xffff0000,
	NX_ARGB_GREEN	= 0xff00ff00,
	NX_ARGB_BLUE	= 0xff0000ff,
	NX_ARGB_YELLOW	= 0xffffff00,
	NX_ARGB_MAGENTA	= 0xffff00ff,
	NX_ARGB_CYAN	= 0xff00ffff,
	NX_ARGB_WHITE	= 0xffffffff,
	};

/**
\brief Used to store a single point and colour for debug rendering.
*/
struct NxDebugPoint
	{
	NxVec3	p;
	NxU32	color;
	};

/**
\brief Used to store a single line and colour for debug rendering.
*/
struct NxDebugLine
	{
	NxVec3	p0;
	NxVec3	p1;
	NxU32	color;
	};

/**
\brief Used to store a single triangle and colour for debug rendering.
*/
struct NxDebugTriangle
	{
	NxVec3	p0;
	NxVec3	p1;
	NxVec3	p2;
	NxU32	color;
	};

/**
\brief This class references buffers with points, lines, and triangles.  They represent visualizations
of SDK objects to help with debugging the user's code.  

The user should not have to instance this class.

<h3>Example</h3>

\include NxUserDebugRenderer_Example.cpp
*/
class NxDebugRenderable
	{
	public:
	NX_INLINE NxDebugRenderable(NxU32 np, const NxDebugPoint* p, NxU32 nl, const NxDebugLine* l, NxU32 nt, const NxDebugTriangle* t)
		:  numPoints(np), numLines(nl), numTriangles(nt), points(p), lines(l), triangles(t)		{		}

	/**
	\brief Retrieve the number of points to render.
	\return Point count.
	*/
	NX_INLINE NxU32 getNbPoints() const						{ return numPoints; }

	/**
	\brief Retrieve an array of points.
	\return Array of #NxDebugPoint
	*/
	NX_INLINE const NxDebugPoint* getPoints() const			{ return points; }



	/**
	\brief Retrieve the number of lines to render.
	\return Line Count.
	*/
	NX_INLINE NxU32 getNbLines() const						{ return numLines; }

	/**
	\brief Retrieve an array of lines to render.
	\return Array of #NxDebugLine
	*/
	NX_INLINE const NxDebugLine* getLines() const			{ return lines; }

	/**
	\brief Retrieve the number of triangles to render.
	\return Array of #NxDebugTriangle
	*/
	NX_INLINE NxU32 getNbTriangles() const					{ return numTriangles; }

	/**
	\brief Retrieve an array of triangles to render.
	\return Array of #NxDebugTriangle
	*/
	NX_INLINE const NxDebugTriangle* getTriangles() const	{ return triangles; }

	private:
	NxU32 numPoints;
	NxU32 numLines;
	NxU32 numTriangles;

	const NxDebugPoint* points;
	const NxDebugLine* lines;
	const NxDebugTriangle* triangles;
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
