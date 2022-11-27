

#include "Common.h"


class PhysicsService;

class PhysTriangleMesh : public IPhysTriangleMesh
{
	friend class PhysicsService;
public:
	PhysTriangleMesh(NxTriangleMesh * trgMesh, IPhysics * srv);
	virtual ~PhysTriangleMesh();

	NxTriangleMesh * TriangleMesh();

private:
	NxTriangleMesh * triangleMesh;
};