#pragma once

#include "Common.h"
#include "PhysInternal.h"

//Базовые свойства актёра
class PhysRigidBodyActor
{
public:
	PhysRigidBodyActor(float density);
	~PhysRigidBodyActor();

	// удалить актера - после этой функции больше актером пользоваться нельзя
	void Release();

	//Устоновить мировую позицию
	void SetTransform(const Matrix & mtx);
	//Получить позицию и ориентацию в мире
	void GetTransform(Matrix & mtx) const;
	//Установить массу
	void SetMass(float mass);
	//Установить массу
	float GetMass() const;
	//Установить центр масс
	void SetCenterMass(const Vector & cm);
	//Установить группу
	void SetGroup(PhysicsCollisionGroup group);
	//Получить группы
	PhysicsCollisionGroup GetGroup() const;
	//Приложить силу к кости в заданной локальной точке
	void ApplyForce(const Vector & force, const Vector & localPosition);

	//Приложить импульс к кости в заданной локальной точке
	void ApplyImpulse(const Vector & force, const Vector & localPosition);
	// приложить вращающий момент в СК актера
	void ApplyLocalTorque(const Vector& torque);

	// установить затухание движения
	void SetMotionDamping ( float fDamping );
	void SetMotionDamping ( float fLinDamping, float fAngDamping );

	// включить-выключить коллизию
	void EnableCollision ( bool bEnable );
	// включить-выключить гравитацию
	void EnableGravity ( bool bEnable );
	// включить-выключить кинематическое поведение
	void EnableKinematic(bool bEnable);

	//Включить-выключить объект
	void Activate(bool isActive);

	//получить владение
	void SetNxActor(const char * filename, long fileline, NxActor* nxActor);
	//Получить ссылку на актёра
	NxActor & GetNxActor() const;

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material);
	// получить материал
	IPhysMaterial * GetPhysMaterial() const;

	// получить линейную составляющую скорости
	Vector GetLinearVelocity() const;

	// Нормализовать глобальную матрицу чтобы не искажалась
	int Normalize();

	__forceinline void SetFileLine(const char * filename, long fileline);
	__forceinline const char * GetFileName() const { return m_fileName; }
	__forceinline long GetFileLine() const { return m_fileLine; }

private:
	void sys_SetGroup(PhysicsCollisionGroup group);

	NxActor * actor;
	PhysicsCollisionGroup group;
	IPhysMaterial * material;
	float	m_density;
	bool	bKinematic;
	
	const char	* m_fileName;
	long		m_fileLine;
};

//Плоскость
class PhysPlane : public IPhysPlane
{
public:
	PhysPlane(const char * filename, long fileline, const Vector & n, float d, IPhysicsScene * _scene);
	virtual ~PhysPlane();
	//Установить плоскость
	virtual void SetPlane(Plane & p);
	//Получить плоскость
	virtual Plane GetPlane();

	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx){ }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx){ }
	//Установить массу
	virtual void SetMass(float mass){ }
	//Получить массу
	virtual float GetMass() { return 0.0f; }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm){ }
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group){ rbactor.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup(){ return rbactor.GetGroup(); }
	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition){ }

	//Приложить импульс к кости в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition){}
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque){}
	

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ){ rbactor.SetMotionDamping ( fDamping ); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { rbactor.SetMotionDamping(fLinDamping, fAngDamping); }

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable ){ rbactor.EnableCollision ( bEnable ); }
	// включить-выключить гравитацию
	virtual void EnableGravity( bool bEnable ) { rbactor.EnableGravity( bEnable ); }
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable) { rbactor.EnableKinematic(bEnable); }

	//Включить-выключить объект
	virtual void Activate(bool isActive){ rbactor.Activate(isActive); scene->SceneChanged(); }

	//Получить доступ к физическому актёру
	virtual NxActor * GetActor(){ return &rbactor.GetNxActor(); }
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.actor = &rbactor.GetNxActor(); }

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material) { rbactor.SetPhysMaterial(material); }
	// получить материал
	IPhysMaterial * GetPhysMaterial() const { return rbactor.GetPhysMaterial(); }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { return rbactor.GetLinearVelocity(); }

	// Нормализовать физический объект, чтобы матрицы не искажались
	virtual int Normalize() { return rbactor.Normalize(); };

protected:
	PhysRigidBodyActor rbactor;
};

//Ящик
class PhysBox : public IPhysBox
{
public:
	PhysBox(const char * filename, long fileline, const Vector & size, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene);
	virtual ~PhysBox();
public:
	//Установить размер ящика
	virtual void SetSize(const Vector & size);
	//Получить размер ящика
	virtual Vector GetSize();

	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx){ rbactor.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx){ rbactor.GetTransform(mtx); }
	//Установить массу
	virtual void SetMass(float mass){ rbactor.SetMass(mass); }
	//Получить массу
	virtual float GetMass() { return rbactor.GetMass(); }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm){ rbactor.SetCenterMass(cm); }
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group)
	{ 
		rbactor.SetGroup(group);
	}
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup(){ return rbactor.GetGroup(); }
	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition){ rbactor.ApplyForce(force, localPosition); }

	//Приложить импульс к кости в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition){ rbactor.ApplyImpulse(force, localPosition); }
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) { rbactor.ApplyLocalTorque(torque); }

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ){ rbactor.SetMotionDamping ( fDamping ); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { rbactor.SetMotionDamping(fLinDamping, fAngDamping); }

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable ){ rbactor.EnableCollision ( bEnable ); }
	// включить-выключить гравитацию
	virtual void EnableGravity( bool bEnable ) { rbactor.EnableGravity( bEnable ); }
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable) { rbactor.EnableKinematic(bEnable); }

	//Включить-выключить объект
	virtual void Activate(bool isActive){ rbactor.Activate(isActive); scene->SceneChanged(); }

	//Получить доступ к физическому актёру
	virtual NxActor * GetActor(){ return &rbactor.GetNxActor(); }
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.actor = &rbactor.GetNxActor(); }

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material) { rbactor.SetPhysMaterial(material); }
	// получить материал
	IPhysMaterial * GetPhysMaterial() const { return rbactor.GetPhysMaterial(); }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { return rbactor.GetLinearVelocity(); }

	// Нормализовать физический объект, чтобы матрицы не искажались
	virtual int Normalize() { return rbactor.Normalize(); };

protected:
	PhysRigidBodyActor rbactor;
};

//Сфера
class PhysSphere : public IPhysSphere
{
public:
	PhysSphere(const char * filename, long fileline, float radius, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene);
	virtual ~PhysSphere();
public:
	//Установить радиус
	virtual void SetRadius(float radius);
	//Получить радиус
	virtual float GetRadius();

	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx){ rbactor.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx){ rbactor.GetTransform(mtx); }
	//Установить массу
	virtual void SetMass(float mass){ rbactor.SetMass(mass); }
	//Получить массу
	virtual float GetMass() { return rbactor.GetMass(); }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm){ rbactor.SetCenterMass(cm); }
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group){ rbactor.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup(){ return rbactor.GetGroup(); }
	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition){ rbactor.ApplyForce(force, localPosition); }

	//Приложить импульс к кости в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition){ rbactor.ApplyImpulse(force, localPosition); }
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) { rbactor.ApplyLocalTorque(torque); }

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ){ rbactor.SetMotionDamping ( fDamping ); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { rbactor.SetMotionDamping(fLinDamping, fAngDamping); }

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable ){ rbactor.EnableCollision ( bEnable ); }
	// включить-выключить гравитацию
	virtual void EnableGravity( bool bEnable ) { rbactor.EnableGravity( bEnable ); }
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable) { rbactor.EnableKinematic(bEnable); }

	//Включить-выключить объект
	virtual void Activate(bool isActive){ rbactor.Activate(isActive); scene->SceneChanged(); }

	//Получить доступ к физическому актёру
	virtual NxActor * GetActor(){ return &rbactor.GetNxActor(); }
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.actor = &rbactor.GetNxActor(); }

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material) { rbactor.SetPhysMaterial(material); }
	// получить материал
	IPhysMaterial * GetPhysMaterial() const { return rbactor.GetPhysMaterial(); }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { return rbactor.GetLinearVelocity(); }

	// Нормализовать физический объект, чтобы матрицы не искажались
	virtual int Normalize() { return rbactor.Normalize(); };

protected:
	PhysRigidBodyActor rbactor;
};

//Капсула
class PhysCapsule : public IPhysCapsule
{
public:
	PhysCapsule(const char * filename, long fileline, float radius, float height, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene);
	virtual ~PhysCapsule();
public:
	//Установить радиус
	virtual void SetRadius(float radius);
	//Получить радиус
	virtual float GetRadius();
	//Установить высоту
	virtual void SetHeight(float height);
	//Получить высоту
	virtual float GetHeight();

	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx){ rbactor.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx){ rbactor.GetTransform(mtx); }
	//Установить массу
	virtual void SetMass(float mass){ rbactor.SetMass(mass); }
	//Получить массу
	virtual float GetMass() { return rbactor.GetMass(); }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm){ rbactor.SetCenterMass(cm); }
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group){ rbactor.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup(){ return rbactor.GetGroup(); }
	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition){ rbactor.ApplyForce(force, localPosition); }

	//Приложить импульс к кости в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition){ rbactor.ApplyImpulse(force, localPosition); }
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) { rbactor.ApplyLocalTorque(torque); }

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ){ rbactor.SetMotionDamping ( fDamping ); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { rbactor.SetMotionDamping(fLinDamping, fAngDamping); }

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable ) { rbactor.EnableCollision ( bEnable ); }
	// включить-выключить гравитацию
	virtual void EnableGravity( bool bEnable ) { rbactor.EnableGravity( bEnable ); }
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable) { rbactor.EnableKinematic(bEnable); }

	//Включить-выключить объект
	virtual void Activate(bool isActive){ rbactor.Activate(isActive); scene->SceneChanged(); }

	//Получить доступ к физическому актёру
	virtual NxActor * GetActor(){ return &rbactor.GetNxActor(); }
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.actor = &rbactor.GetNxActor(); }

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material) { rbactor.SetPhysMaterial(material); }
	// получить материал
	IPhysMaterial * GetPhysMaterial() const { return rbactor.GetPhysMaterial(); }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { return rbactor.GetLinearVelocity(); }

	// Нормализовать физический объект, чтобы матрицы не искажались
	virtual int Normalize() { return rbactor.Normalize(); };

protected:
	PhysRigidBodyActor rbactor;
};

//Объект состоящий из сетки треугольников
class PhysTriangleMesh;
class PhysMesh : public IPhysMesh
{
public:
	PhysMesh(const char * filename, long fileline, const IPhysicsScene::MeshInit * meshes, dword numMeshes, bool isDynamic, float density, IPhysicsScene * _scene);
	virtual ~PhysMesh();
public:

	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx){ rbactor.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx){ rbactor.GetTransform(mtx); }
	//Установить массу
	virtual void SetMass(float mass){ rbactor.SetMass(mass); }
	//Получить массу
	virtual float GetMass() { return rbactor.GetMass(); }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm){ rbactor.SetCenterMass(cm); }
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group){ rbactor.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup(){ return rbactor.GetGroup(); }
	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition){ rbactor.ApplyForce(force, localPosition); }

	//Приложить импульс к кости в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition){ rbactor.ApplyImpulse(force, localPosition); }
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) { rbactor.ApplyLocalTorque(torque); }

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ){ rbactor.SetMotionDamping ( fDamping ); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { rbactor.SetMotionDamping(fLinDamping, fAngDamping); }

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable );// { rbactor.EnableCollision ( bEnable ); }
	// включить-выключить гравитацию
	virtual void EnableGravity( bool bEnable ) { rbactor.EnableGravity( bEnable ); }
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable);// { rbactor.EnableKinematic(bEnable); }

	//Включить-выключить объект
	virtual void Activate(bool isActive){ rbactor.Activate(isActive); scene->SceneChanged(); }

	//Получить доступ к физическому актёру
	virtual NxActor * GetActor(){ return &rbactor.GetNxActor(); }
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.actor = &rbactor.GetNxActor(); }

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material) { rbactor.SetPhysMaterial(material); }
	// получить материал
	IPhysMaterial * GetPhysMaterial() const { return rbactor.GetPhysMaterial(); }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { return rbactor.GetLinearVelocity(); }

protected:
	PhysRigidBodyActor rbactor;
	array<PhysTriangleMesh *> refMeshes;
};

//Комбинированный объект
class PhysCombined : public IPhysCombined
{
	struct ObjectDesc
	{
		ObjectDesc();
		~ObjectDesc();
		NxActorDesc actorDesc;
		array<NxShapeDesc *> descs;
		NxBodyDesc bodyDesc;
	};
public:
	PhysCombined(const char * filename, long fileline, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene);
	virtual ~PhysCombined();

public:
	//Добавить ящик
	virtual void AddBox(const Vector & size, const Matrix & transform);
	//Добавить шар
	virtual void AddSphere(float radius, const Matrix & transform);
	//Добавить капсулу
	virtual void AddCapsule(float radius, float height, const Matrix & transform);
	//Установить массу элемента
	virtual bool SetMass(unsigned int index, float mass);
	//Получить массу
	virtual float GetMass() { return rbactor.GetMass(); }
	//Получить текущее число фигур для билда
	virtual unsigned int GetCountForBuild();
	//Сконструировать объект
	virtual void Build();

public:
	//Получить количество фигур
	virtual long GetCount();
	//Получить локальную позицию
	virtual bool SetLocalTransform(long index, const Matrix & transform);
	//Установить локальную позицию
	virtual bool GetLocalTransform(long index, Matrix & transform);
	//Получить тип
	virtual Type GetType(long index);
	//Получить параметры ящика
	virtual bool GetBox(long index, Vector & size);
	//Получить параметры шара
	virtual bool GetSphere(long index, float & radius);
	//Получить параметры капсулы
	virtual bool GetCapsule(long index, float & radius, float & height);
	//Установить глобальную позицию для элемента
	virtual bool SetGlobalTransform(unsigned int index, const Matrix & transform);
	//Получить глобальную позицию для элемента
	virtual bool GetGlobalTransform(unsigned int index, Matrix & transform);
	//Включить/выключить коллизию для элемента
	virtual void EnableCollision(unsigned int index, bool enable);
	//Включить/выключить коллизию для актера
	virtual void EnableCollision(bool enable);
	//Включить/выключить реакцию на коллизии для элемента
	virtual void EnableResponse(unsigned int index, bool enable);
	//Включить/выключить рэйкаст для элемента
	virtual void EnableRaycast(unsigned int index, bool enable);
	//Включить/выключить визуализацию для элемента
	virtual void EnableVisualization(unsigned int index, bool enable);
	//Включить/выключить визуализацию для актера
	virtual void EnableVisualization(bool enable);
	// включить-выключить гравитацию
	virtual void EnableGravity( bool bEnable );
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable);

	// установить кол-во итерация солвера для актера
	virtual void SetSolverIterations(unsigned int count);

	//Протестировать элемент на пересечение с лучом
	virtual bool Raycast(unsigned int index, const Vector& from, const Vector& to, RaycastResult * details = null);
	//Протестировать элемент на пересечение с боксом
	virtual bool OverlapBox(unsigned int index, const Vector& size, const Matrix& transform );
	//Протестировать элемент на пересечение со сферой
	virtual bool OverlapSphere(unsigned int index, const Vector& center, float radius);
	

	//Установить мировую позицию
	virtual void SetTransform(const Matrix & mtx){ rbactor.SetTransform(mtx); }
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx){ rbactor.GetTransform(mtx); }
	//Установить массу
	virtual void SetMass(float mass){ rbactor.SetMass(mass); }
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm){ rbactor.SetCenterMass(cm); }
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group){ rbactor.SetGroup(group); }
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup(){ return rbactor.GetGroup(); }
	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition){ rbactor.ApplyForce(force, localPosition); }

	//Приложить импульс к кости в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition){ rbactor.ApplyImpulse(force, localPosition); }
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) { rbactor.ApplyLocalTorque(torque); }

	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ){ rbactor.SetMotionDamping ( fDamping ); }
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) { rbactor.SetMotionDamping(fLinDamping, fAngDamping); }

	//Включить-выключить объект
	virtual void Activate(bool isActive){ rbactor.Activate(isActive); scene->SceneChanged(); }

	//Получить доступ к физическому актёру
	virtual NxActor * GetActor(){ return &rbactor.GetNxActor(); }
	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.actor = &rbactor.GetNxActor(); }

	// установить материал
	void SetPhysMaterial(IPhysMaterial * material) { rbactor.SetPhysMaterial(material); }
	// получить материал
	IPhysMaterial * GetPhysMaterial() const { return rbactor.GetPhysMaterial(); }
	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const { return rbactor.GetLinearVelocity(); }

	// Нормализовать физический объект, чтобы матрицы не искажались
	virtual int Normalize() { return rbactor.Normalize(); };

protected:
	PhysRigidBodyActor rbactor;
	ObjectDesc * desc;
};


class PhysRigidBodyConnector : public IPhysRigidBodyConnector
{
public:
	PhysRigidBodyConnector(const char * filename, long fileline, IPhysicsScene * _scene, IPhysRigidBody * _left, IPhysRigidBody * _right, float brokeForce);
	virtual ~PhysRigidBodyConnector();

public:

	//////////////////////////////////////////////////////////////////////////
	// IPhysRigidBodyConnector
	//////////////////////////////////////////////////////////////////////////
	//Сломано соединение или нет
	virtual bool IsBroke();
	//Поставить обработчик
	virtual void SetEventHandler(BrokeEvent * event);
	//Получить первый объект к которому присоеденены
	virtual IPhysRigidBody * GetLeft();
	//Получить второй объект к которому присоеденены
	virtual IPhysRigidBody * GetRight();

protected:
	IPhysRigidBody * left;
	IPhysRigidBody * right;
	NxFixedJoint * joint;
};



