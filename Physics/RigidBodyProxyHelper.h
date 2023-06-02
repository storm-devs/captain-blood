#pragma once
#include "common.h"
#include "IProxy.h"
#include "Deferrer.h"

template <class T>
class RigidBodyProxyHelper : public IPhysRigidBody
{
	array<IDeferrer<T>*>&	m_calls;

public:
	Matrix					m_transform;
	Vector					m_cm;	
	float					m_mass;
	PhysicsCollisionGroup	m_physicscollisiongroup;

	RigidBodyProxyHelper(array<IDeferrer<T>*>& calls) :	IPhysRigidBody(NULL), m_calls(calls) 
	{
		m_cm = 0.0f;
		m_mass = 1.0f;
		m_physicscollisiongroup = (PhysicsCollisionGroup)0;
	}
	virtual ~RigidBodyProxyHelper(void) {}

	//////////////////////////////////////////////////////////////////////////
	// IPhysRigidBody
	//////////////////////////////////////////////////////////////////////////

	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx)
	{
		m_transform = mtx;
		m_calls.Add
		(
			MakeDeferrer<T, void, const Matrix&>(DEF_FREF(IPhysRigidBody::SetTransform), mtx)
		);
	}
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx) { mtx = m_transform; }//Assert(!"do not call it for that class"); }
	//Установить плотность
	virtual void SetMass(float mass)
	{
		m_mass = mass;
		m_calls.Add
		(
			MakeDeferrer<T, void, float>(DEF_FREF(IPhysRigidBody::SetMass), mass)
		);
	}
	//Получить массу
	virtual float GetMass() { return m_mass; } //Assert(!"do not call it for that class"); return 0.0f; }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm)
	{
		m_cm = cm;
		m_calls.Add
		(
			MakeDeferrer<T, void, const Vector&>(DEF_FREF(IPhysRigidBody::SetCenterMass), cm)
		);
	}
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group)
	{
		m_physicscollisiongroup = group;

		m_calls.Add
		(
			MakeDeferrer<T, void, PhysicsCollisionGroup>(DEF_FREF(IPhysRigidBody::SetGroup), group)
		);
	}
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup() { return m_physicscollisiongroup; } //Assert(!"do not call it for that class"); return phys_nocollision; }

	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition)
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, const Vector&, const Vector&>(DEF_FREF(IPhysRigidBody::ApplyForce), force, localPosition)
		);
	}

	//Приложить импульс в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition)
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, const Vector&, const Vector&>(DEF_FREF(IPhysRigidBody::ApplyImpulse), force, localPosition)
		);
	}
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque)
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, const Vector&>(DEF_FREF(IPhysRigidBody::ApplyLocalTorque), torque)
		);
	}

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping )
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, float>(DEF_FREF(IPhysRigidBody::SetMotionDamping), fDamping)
		);
	}
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping )
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, float, float>(DEF_FREF(IPhysRigidBody::SetMotionDamping), fLinDamping, fAngDamping)
		);
	}

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable )
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, bool>(DEF_FREF(IPhysRigidBody::EnableCollision), bEnable)
		);
	}
	// включить-выключить гравитацию
	virtual void EnableGravity ( bool bEnable )
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, bool>(DEF_FREF(IPhysRigidBody::EnableGravity), bEnable)
		);
	}
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable)
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, bool>(DEF_FREF(IPhysRigidBody::EnableKinematic), bEnable)
		);
	}

	//Включить-выключить объект
	virtual void Activate(bool isActive)
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, bool>(DEF_FREF(IPhysRigidBody::Activate), isActive)
		);
	}


	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { Assert(!"do not call it for that class"); }

	// установить материал
	virtual void SetPhysMaterial(IPhysMaterial * material)
	{
		m_calls.Add
		(
			MakeDeferrer<T, void, IPhysMaterial *>(DEF_FREF(IPhysRigidBody::SetPhysMaterial), material)
		);
	}
	// получить материал
	virtual IPhysMaterial * GetPhysMaterial() const { Assert(!"do not call it for that class"); return NULL; }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { Assert(!"do not call it for that class"); return Vector(0.0f); }
};
