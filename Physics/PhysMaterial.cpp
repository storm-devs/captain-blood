#include "PhysMaterial.h"

PhysMaterial::PhysMaterial(const char * filename, long fileline, IPhysicsScene * scene, PxMaterial * mat) :
IPhysMaterial(scene),
m_nxMaterial(mat)
{
	SetFileLine(filename, fileline);
	Assert(m_nxMaterial);
}

PhysMaterial::~PhysMaterial(void)
{
}
