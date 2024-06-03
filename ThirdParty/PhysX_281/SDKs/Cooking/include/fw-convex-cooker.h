/*----------------------------------------------------------------------
    This Software and Related Documentation are Proprietary to Ageia
    Technologies, Inc.

    Copyright 2005 Ageia Technologies, Inc. St. Louis, MO
    Unpublished -
    All Rights Reserved Under the Copyright Laws of the United States.

    Restricted Rights Legend:  Use, Duplication, or Disclosure by
    the Government is Subject to Restrictions as Set Forth in
    Paragraph (c)(1)(ii) of the Rights in Technical Data and
    Computer Software Clause at DFARS 252.227-7013.  Ageia
    Technologies Inc.
-----------------------------------------------------------------------*/

#ifndef FW_MESH_COOKER_H
#define FW_MESH_COOKER_H 1

#include "NxPhysics.h"

class ConvexMesh;
void makeFwConvex(NxU8 *addr, const ConvexMesh *mesh);

#ifdef DEFINE_CONVEX_COOKER 
//#include "PhysicsSDK.h"
#include "Physics.h"
#include "TriangleMesh.h"
#include "TriangleMeshShape.h"
#include "ConvexMesh.h"
#include "ConvexShape.h"
#include "Scene.h"
// #include "Stream.h"
#include "ConvexHull.h"
#include "ConvexMesh.h"
#include "fw-mesh-utils.h"
#include "fw-convex.h"


// convex is required to be at least MAX_FWCONVEX_SIZE

void makeFwConvex(NxU8 *convex, const ConvexMesh *mesh)
{
	NxU32 i,j;

	NxU32 nVerts = ((ConvexMeshRuntime *)mesh)->GetNbVerts();
	NxU32 nFaces = ((ConvexMeshRuntime *)mesh)->GetNbPolygons();
	NxU32 nEdges = nVerts + nFaces - 2;

	ASSERT(nVerts <= FW_MAX_CONVEX_VERT);
	ASSERT(nFaces <= FW_MAX_CONVEX_FACE);

	// Allocate memory

	fw_convex_shape *shape = (fw_convex_shape *) convex;
	shape->numVerts = nVerts;
	shape->numFaces = nFaces;

	shape->centroid = FwV3(0,0,0);
	FwV3 bbMin = FwV3(FLT_MAX,FLT_MAX,FLT_MAX);
	FwV3 bbMax = FwV3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	// Copy verts

	FwV3 *verts			= getConvexVerts(convex);
	for (i = 0; i < nVerts; i++)
	{
		verts[i] = *(FwV3 *)&(((ConvexMeshRuntime *)mesh)->GetVerts()[i]);
		shape->centroid += verts[i];
		bbMin = bbMin.min(verts[i]);
		bbMax = bbMax.max(verts[i]);
	}

	shape->centroid /= (NxReal)nVerts;
	shape->bbCentre = (bbMin + bbMax)/2;
	shape->bbRadius = (bbMax - bbMin)/2;

	fw_convex_face *faces	= getConvexFaces(convex);
	fw_convex_loop *loops	= getConvexLoops(convex);

	// Copy faces

	NxU32 loopIndex = 0;
	for (i = 0; i < nFaces; i++)
	{
		const PxHullPolygonData & Poly = ((ConvexMeshRuntime *)mesh)->GetPolygon(i);

		faces[i].plane.n = *(FwV3 *)&Poly.mPlane[0];
		faces[i].plane.d = Poly.mPlane[3];
		faces[i].loopIndex = loopIndex;
		faces[i].numVerts = Poly.mNbVerts;

		FwV3 fn = (verts[Poly.mVRef8[1]]-verts[Poly.mVRef8[0]]).cross(verts[Poly.mVRef8[2]]-verts[Poly.mVRef8[1]]);


		// Copy loops such that cross product of successive faces is always outward facing

		if(fn.dot(faces[i].plane.n)>0)
		{
			for (j = 0; j < Poly.mNbVerts; j++)
			{
				loops[loopIndex].vIndex = Poly.mVRef8[j];
				loops[loopIndex].eIndex = static_cast<u8> (Poly.mERef16[j]);  // TODO: cull parallel edges
				loopIndex++;
			}
		}
		else
		{
			for (j = 0; j < Poly.mNbVerts; j++)
			{
				loops[loopIndex].vIndex = Poly.mVRef8[Poly.mNbVerts-1-j];
				loops[loopIndex].eIndex = static_cast<u8> (Poly.mERef16[Poly.mNbVerts-1-j]);  // TODO: cull parallel edges
				loopIndex++;
			}
		}
	}

	NxReal normErr = 0;
	for(i=0;i<nFaces;i++)
	{
		fw_convex_loop *loop		= loops + faces[i].loopIndex;
		NxU8			v0			= loop[faces[i].numVerts-1].vIndex;
		NxU8			v1;

		for(j=0; j<faces[i].numVerts; j++, v0=v1)
		{
			v1 = loop[j].vIndex;
			FwV3 e = (verts[v1]-verts[v0]).unit();
			NxReal k = fabs(faces[i].plane.n.dot(e));
			if(k>normErr)
				normErr = k;
		}
	}

//	printf("********** Normal error %f\n",normErr);


	/* fill in per-edge face info - somewhat clunky */

	NxU8 *edgeTable = (NxU8 *) _alloca(nEdges * 2 * 3);
	NxU8 *edgePtr = edgeTable;

	for(i=0;i<nFaces;i++)
	{
		fw_convex_loop *loop		= loops + faces[i].loopIndex;
		NxU8			v0			= loop[faces[i].numVerts-1].vIndex;
		NxU8			v1;

		for(j=0; j<faces[i].numVerts; j++, v0=v1)
		{
			v1 = loop[j].vIndex;
			*edgePtr++ = v0;
			*edgePtr++ = v1;
			*edgePtr++ = i;
		}
	}


	for(i=0;i<nFaces;i++)
	{
		fw_convex_loop *loop		= loops + faces[i].loopIndex;
		NxU8			v0			= loop[faces[i].numVerts-1].vIndex;
		NxU8			v1;

		for(j=0; j<faces[i].numVerts; j++, v0=v1)
		{
			NxU32 k;
			v1 = loop[j].vIndex;
			for(k=0;k<nEdges*2 && !(edgeTable[k*3]==v1 && edgeTable[k*3+1]==v0); k++);
			assert(k!=nEdges*2);
			loop[j].adjFace = edgeTable[k*3+2];
		}
	}

	{
		shape->ofsLoops = static_cast<u16> ((sizeof(fw_convex_shape) + sizeof(fw_convex_face) * nFaces) >> 2);
		shape->ofsVerts = shape->ofsLoops +
					static_cast<u8> ((sizeof(fw_convex_loop) * nEdges * 2) >> 2);
		shape->cSize = shape->ofsVerts + ((12 * shape->numVerts) >> 2);
		shape->cSize = (shape->cSize + 7) & ~7;
	}
}

#endif
#endif
