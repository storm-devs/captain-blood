#pragma once
#include "RigidBodyProxyHelper.h"

class PhysicsScene;

class BoxProxy : public IPhysBox, public IProxy
{
	RigidBodyProxyHelper<IPhysBox>	m_helper;
	DECLARE_MCALLS(IPhysBox)
	//array<IDeferrer<IPhysBox>*>		m_calls;
	IPhysBox*						m_realBox;
	//PhysicsScene&					m_scene;
	
	Vector	m_size;
	bool	m_isDynamic;
	float	m_density;

public:
	BoxProxy(const Vector & size, const Matrix & transform, bool isDynamic, float density, PhysicsScene& scene);
	virtual ~BoxProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysRigidBody
	//////////////////////////////////////////////////////////////////////////
	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx) { m_helper.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx);
	//Установить плотность
	virtual void SetMass(float mass) { m_helper.SetMass(mass); }
	//Получить массу
	virtual float GetMass();
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm) { m_helper.SetCenterMass(cm); }
	//Установить центр масс
	virtual Vector GetCenterMass();
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group) { m_helper.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup();

	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition) { m_helper.ApplyForce(force, localPosition); }

	//Приложить импульс в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition) { m_helper.ApplyImpulse(force, localPosition); }
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) { m_helper.ApplyLocalTorque(torque); }

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ) { m_helper.SetMotionDamping(fDamping); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { m_helper.SetMotionDamping(fLinDamping, fAngDamping); }

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable ) { m_helper.EnableCollision(bEnable); }
	// включить-выключить гравитацию
	virtual void EnableGravity ( bool bEnable ) { m_helper.EnableGravity(bEnable); }
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable) { m_helper.EnableKinematic(bEnable); }

	//Включить-выключить объект
	virtual void Activate(bool isActive) { m_helper.Activate(isActive); }

	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const;

	// установить материал
	virtual void SetPhysMaterial(IPhysMaterial * material) { m_helper.SetPhysMaterial(material); }
	// получить материал
	virtual IPhysMaterial * GetPhysMaterial() const;
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const;

	//////////////////////////////////////////////////////////////////////////
	// IPhysBox
	//////////////////////////////////////////////////////////////////////////
	//Установить размер ящика
	virtual void SetSize(const Vector & size);
	//Получить размер ящика
	virtual Vector GetSize();

	//////////////////////////////////////////////////////////////////////////
	// IPhysBase
	//////////////////////////////////////////////////////////////////////////
	virtual bool Release();

	//////////////////////////////////////////////////////////////////////////
	// IProxy
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSyncCreate();
	virtual void OnSyncCalls();
	virtual bool OnSyncRelease();
	virtual void OnSyncTrace();
};
