

#include "Common.h"


class PhysicsService;

class PhysTriangleMesh : public IPhysTriangleMesh
{
	friend class PhysicsService;
public:
	PhysTriangleMesh(PxTriangleMesh * trgMesh, IPhysics * srv);
	virtual ~PhysTriangleMesh();

	PxTriangleMesh * TriangleMesh();

private:
	PxTriangleMesh * triangleMesh;
};