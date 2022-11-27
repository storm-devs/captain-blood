#pragma once
#include "RigidBodyProxyHelper.h"

class PhysicsScene;
class CombinedProxy : public IPhysCombined, public IProxy
{
private:
	struct ObjectDesc
	{
		Matrix	mTransform;
		float	fMass;

		Vector	size;			// box extents
		float	radius;			// sphere or capsule radius
		float	height;			// capsule height
	};

	IPhysCombined *						m_realCombined;
	RigidBodyProxyHelper<IPhysCombined>	m_helper;
	
	DECLARE_MCALLS(IPhysCombined)
	//array<IDeferrer<IPhysCombined>*>	m_calls;

	//PhysicsScene&						m_scene;
	bool								m_isBuilt;
	bool								m_haveBuildCommand;

	Vector								m_velocity;
	array<ObjectDesc>					m_descs;
	float								m_totalMass;
	float								m_density;

	void AddObjectDesc(const Matrix & transform, float fMass, const Vector & size, float radius = 0.0f, float height = 0.0f);

public:
	CombinedProxy(const char * cppfile, long cppline, const Matrix & transform, bool isDynamic, float density, PhysicsScene& _scene);
	virtual ~CombinedProxy(void);

	//////////////////////////////////////////////////////////////////////////
	// IPhysRigidBody
	//////////////////////////////////////////////////////////////////////////
	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx) {	m_helper.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx);
	//Установить массу
	virtual void SetMass(float mass) { m_helper.SetMass(mass); }
	//Получить массу
	virtual float GetMass();
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm) {	m_helper.SetCenterMass(cm); }
	//Установить центр масс
	virtual Vector GetCenterMass();
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group) { m_helper.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup();

	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition);

	//Приложить импульс в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition);
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque);

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping );
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping );

	// включить-выключить гравитацию
	virtual void EnableGravity ( bool bEnable );
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable);

	//Включить-выключить объект
	virtual void Activate(bool isActive);
	
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const;

	// установить материал
	virtual void SetPhysMaterial(IPhysMaterial * material);
	// получить материал
	virtual IPhysMaterial * GetPhysMaterial() const;
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const;


	//////////////////////////////////////////////////////////////////////////
	// IPhysCombined
	//////////////////////////////////////////////////////////////////////////
	//Добавить ящик
	virtual void AddBox(const Vector & size, const Matrix & transform);
	//Добавить шар
	virtual void AddSphere(float radius, const Matrix & transform);
	//Добавить капсулу
	virtual void AddCapsule(float radius, float height, const Matrix & transform);
	//Установить массу элемента
	virtual bool SetMass(unsigned int index, float mass);
	//Получить текущее число фигур для билда
	virtual unsigned int GetCountForBuild();
	//Сконструировать объект
	virtual void Build();

public:
	//Получить количество фигур
	virtual long GetCount();
	//Установить локальную позицию
	virtual bool SetLocalTransform(long index, const Matrix & transform);
	//Получить локальную позицию
	virtual bool GetLocalTransform(long index, Matrix & transform);
	//Получить тип
	virtual Type GetType(long index);
	//Получить параметры ящика
	virtual bool GetBox(long index, Vector & size);
	//Получить параметры шара
	virtual bool GetSphere(long index, float & radius);
	//Получить параметры капсулы
	virtual bool GetCapsule(long index, float & radius, float & height);
	//Установить глобальную позицию
	virtual bool SetGlobalTransform(unsigned int index, const Matrix & transform);
	//Получить глобальную позицию
	virtual bool GetGlobalTransform(unsigned int index, Matrix & transform);
	//Включить/выключить коллизии
	virtual void EnableCollision(bool enable);
	virtual void EnableCollision(unsigned int index, bool enable);
	//Включить/выключить реакцию на коллизии
	virtual void EnableResponse(unsigned int index, bool enable);
	//Включить/выключить рэйкаст
	virtual void EnableRaycast(unsigned int index, bool enable);
	//Включить/выключить визуализацию
	virtual void EnableVisualization(unsigned int index, bool enable);
	//Включить/выключить визуализацию для актера
	virtual void EnableVisualization(bool enable);
	// установить кол-во итерация солвера для актера
	virtual void SetSolverIterations(unsigned int count);

	//Протестировать элемент на пересечение с лучом
	virtual bool Raycast(unsigned int index, const Vector& from, const Vector& to, RaycastResult * details = null);
	//Протестировать элемент на пересечение с боксом
	virtual bool OverlapBox(unsigned int index, const Vector& size, const Matrix& transform );
	//Протестировать элемент на пересечение со сферой
	virtual bool OverlapSphere(unsigned int index, const Vector& center, float radius);

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
