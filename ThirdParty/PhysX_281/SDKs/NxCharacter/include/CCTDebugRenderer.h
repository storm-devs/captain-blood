#ifndef NX_CHARACTER_DEBUGRENDERER
#define NX_CHARACTER_DEBUGRENDERER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxDebugRenderable.h"
#include "CCTAllocator.h"
#include "NxArray.h"

struct NxExtendedBounds3;

	class CCTDebugData
	{
		public:
											CCTDebugData();
											~CCTDebugData();

				void						clear();

				NxU32						getNbPoints()		const	{ return pointsArray.size();		}
				const NxDebugPoint*			getPoints()			const	{ return pointsArray.begin();		}

				NxU32						getNbLines()		const	{ return linesArray.size();			}
				const NxDebugLine*			getLines()			const	{ return linesArray.begin();		}

				NxU32						getNbTriangles()	const	{ return trianglesArray.size();		}
				const NxDebugTriangle*		getTriangles()		const	{ return trianglesArray.begin();	}

				void						addPoint	(const NxVec3& p, NxU32 color);
				void						addLine		(const NxVec3& p0, const NxVec3& p1, NxU32 color);
				void						addTriangle	(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2, NxU32 color);
				void						addOBB		(const NxBox& box, NxU32 color, bool renderFrame);
				void						addAABB		(const NxBounds3& bounds, NxU32 color, bool renderFrame);
				void						addAABB		(const NxExtendedBounds3& bounds, NxU32 color);
		private:
				NxArray<NxDebugPoint, CCTAllocator>		pointsArray;
				NxArray<NxDebugLine, CCTAllocator>		linesArray;
				NxArray<NxDebugTriangle, CCTAllocator>	trianglesArray;
	};

#endif

