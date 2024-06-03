/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "CCTDebugRenderer.h"
#include "NxExtended.h"

CCTDebugData::CCTDebugData()
{
}

CCTDebugData::~CCTDebugData()
{
}

void CCTDebugData::clear()
{
	pointsArray.clear();
	linesArray.clear();
	trianglesArray.clear();
}

void CCTDebugData::addPoint(const NxVec3& p, NxU32 color)
{
	NxDebugPoint tmp;
	tmp.p		= p;
	tmp.color	= color;
	pointsArray.pushBack(tmp);
}

void CCTDebugData::addLine(const NxVec3& p0, const NxVec3& p1, NxU32 color)
{
	NxDebugLine tmp;
	tmp.p0		= p0;
	tmp.p1		= p1;
	tmp.color	= color;
	linesArray.pushBack(tmp);
}

void CCTDebugData::addTriangle(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2, NxU32 color)
{
	NxDebugTriangle tmp;
	tmp.p0		= p0;
	tmp.p1		= p1;
	tmp.p2		= p2;
	tmp.color	= color;
	trianglesArray.pushBack(tmp);
}

static bool computeBoxPoints(const NxBox& box, NxVec3* pts)
{
	// Checkings
	if(!pts)	return false;

	NxVec3 Axis0; box.rot.getColumn(0, Axis0);
	NxVec3 Axis1; box.rot.getColumn(1, Axis1);
	NxVec3 Axis2; box.rot.getColumn(2, Axis2);

	// "Rotated extents"
	Axis0 *= box.extents.x;
	Axis1 *= box.extents.y;
	Axis2 *= box.extents.z;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	// Original code: 24 vector ops
/*	pts[0] = box.center - Axis0 - Axis1 - Axis2;
	pts[1] = box.center + Axis0 - Axis1 - Axis2;
	pts[2] = box.center + Axis0 + Axis1 - Axis2;
	pts[3] = box.center - Axis0 + Axis1 - Axis2;
	pts[4] = box.center - Axis0 - Axis1 + Axis2;
	pts[5] = box.center + Axis0 - Axis1 + Axis2;
	pts[6] = box.center + Axis0 + Axis1 + Axis2;
	pts[7] = box.center - Axis0 + Axis1 + Axis2;*/

	// Rewritten: 12 vector ops
	pts[0] = pts[3] = pts[4] = pts[7] = box.center - Axis0;
	pts[1] = pts[2] = pts[5] = pts[6] = box.center + Axis0;

	NxVec3 Tmp = Axis1 + Axis2;
	pts[0] -= Tmp;
	pts[1] -= Tmp;
	pts[6] += Tmp;
	pts[7] += Tmp;

	Tmp = Axis1 - Axis2;
	pts[2] += Tmp;
	pts[3] += Tmp;
	pts[4] -= Tmp;
	pts[5] -= Tmp;

	return true;
}

static const NxU32* getBoxEdges()
{
	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	static NxU32 Indices[] = {
	0, 1,	1, 2,	2, 3,	3, 0,
	7, 6,	6, 5,	5, 4,	4, 7,
	1, 5,	6, 2,
	3, 7,	4, 0
	};
	return Indices;
}

void CCTDebugData::addOBB(const NxBox& box, NxU32 color, bool renderFrame)
{
	// Compute box vertices
	NxVec3 pp[8];
	//box.computePoints(pp);
	computeBoxPoints(box, pp);

	// Draw all lines
	const NxU32* Indices = getBoxEdges();
	for(NxU32 i=0;i<12;i++)
		{
		NxU32 VRef0 = *Indices++;
		NxU32 VRef1 = *Indices++;
		addLine(pp[VRef0], pp[VRef1], color);
		}

	// Render frame if needed
	if(renderFrame)
		{
		NxVec3 Axis0; box.rot.getColumn(0, Axis0);
		NxVec3 Axis1; box.rot.getColumn(1, Axis1);
		NxVec3 Axis2; box.rot.getColumn(2, Axis2);
		
		addLine(box.center, box.center + Axis0, 0x00ff0000);
		addLine(box.center, box.center + Axis1, 0x0000ff00);
		addLine(box.center, box.center + Axis2, 0x000000ff);
		}
}

void CCTDebugData::addAABB(const NxBounds3& bounds, NxU32 color, bool renderFrame)
{
	// Reuse OBB code...
	NxVec3 center;	bounds.getCenter(center);
	NxVec3 extents;	bounds.getExtents(extents);
	NxMat33 id;	id.id();
	addOBB(NxBox(center, extents, id), color, renderFrame);
}

void CCTDebugData::addAABB(const NxExtendedBounds3& bounds, NxU32 color)
{
	NxExtendedVec3 center;
	NxVec3 extents;
	bounds.getCenter(center);
	bounds.getExtents(extents);

	NxBounds3 tmp;
	tmp.setCenterExtents(NxVec3((float)center.x, (float)center.y, (float)center.z), extents);

	addAABB(tmp, color, false);
}
