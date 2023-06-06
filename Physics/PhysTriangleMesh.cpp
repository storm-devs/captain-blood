
#include "PhysTriangleMesh.h"


PhysTriangleMesh::PhysTriangleMesh(PxTriangleMesh* trgMesh, IPhysics * srv) : IPhysTriangleMesh(srv)
{
	Assert(trgMesh);
	triangleMesh = trgMesh;
}

PhysTriangleMesh::~PhysTriangleMesh()
{
}

PxTriangleMesh* PhysTriangleMesh::TriangleMesh()
{
	return triangleMesh;
}




