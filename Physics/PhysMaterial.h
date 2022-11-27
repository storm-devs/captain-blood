#pragma once
#include "common.h"

class PhysMaterial : public IPhysMaterial
{
	NxMaterial * m_nxMaterial;
public:
	PhysMaterial(const char * filename, long fileline, IPhysicsScene * scene, NxMaterial * mat);
	virtual ~PhysMaterial(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysMaterial
	//////////////////////////////////////////////////////////////////////////
	virtual void SetRestitution(float restitution) { m_nxMaterial->setRestitution(restitution); }
	virtual float GetRestitution() const { return m_nxMaterial->getRestitution(); }
	virtual void SetDynamicFriction(float friction) { m_nxMaterial->setDynamicFriction(friction); }
	virtual float GetDynamicFriction() const { return m_nxMaterial->getDynamicFriction(); }
	virtual void SetStaticFriction(float friction) { m_nxMaterial->setStaticFriction(friction); }
	virtual float GetStaticFriction() const { return m_nxMaterial->getStaticFriction(); }
	virtual unsigned int GetIndex() const { return m_nxMaterial->getMaterialIndex(); }
};
