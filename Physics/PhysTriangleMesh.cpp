
#include "PhysTriangleMesh.h"


PhysTriangleMesh::PhysTriangleMesh(NxTriangleMesh * trgMesh, IPhysics * srv) : IPhysTriangleMesh(srv)
{
	Assert(trgMesh);
	triangleMesh = trgMesh;
}

PhysTriangleMesh::~PhysTriangleMesh()
{
}

NxTriangleMesh * PhysTriangleMesh::TriangleMesh()
{
	return triangleMesh;
}




