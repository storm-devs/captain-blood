#pragma once

class IPhysMaterial : public IPhysBase
{
public:
	IPhysMaterial(IPhysicsScene* scene) : IPhysBase(scene) {}

	virtual void SetRestitution(float restitution) = 0;
	virtual float GetRestitution() const = 0;
	virtual void SetDynamicFriction(float friction) = 0;
	virtual float GetDynamicFriction() const = 0;
	virtual void SetStaticFriction(float friction) = 0;
	virtual float GetStaticFriction() const = 0;
	virtual unsigned int GetIndex() const = 0;
};