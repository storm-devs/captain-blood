
class IPhysicsScene;
struct PhysInternal;

class IPhysRigidBody : public IPhysBase
{
protected:
	IPhysRigidBody(IPhysicsScene * _scene) : IPhysBase(_scene)
	{
		material = pmtlid_air;
	}

	virtual ~IPhysRigidBody() {}

public:
	//Устоновить мировую позицию
	virtual void SetTransform(const Matrix & mtx) = null;
	//Получить позицию и ориентацию в мире
	virtual void GetTransform(Matrix & mtx) = null;
	//Установить плотность
	virtual void SetMass(float mass) = null;
	//Получить массу
	virtual float GetMass() = null;
	//Установить центр масс
	virtual void SetCenterMass(const Vector & cm) = null;
	//Установить центр масс
	virtual Vector GetCenterMass() = null;
	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group) = null;
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup() = null;

	//Приложить силу к кости в заданной локальной точке
	virtual void ApplyForce(const Vector & force, const Vector & localPosition) = null;

	//Приложить импульс в заданной локальной точке
	virtual void ApplyImpulse(const Vector & force, const Vector & localPosition) = null;
	// приложить вращающий момент в СК актера
	virtual void ApplyLocalTorque(const Vector& torque) = null;
    
	// установить затухание движения
	virtual void SetMotionDamping ( float fDamping ) = null;
	virtual void SetMotionDamping ( float fLinDamping, float fAngDamping ) = null;

	// включить-выключить коллизию
	virtual void EnableCollision ( bool bEnable ) = null;
	// включить-выключить гравитацию
	virtual void EnableGravity ( bool bEnable ) = null;
	// включить-выключить кинематическое поведение
	virtual void EnableKinematic(bool bEnable) = null;

	//Включить-выключить объект
	virtual void Activate(bool isActive) = null;

	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const = null;

	// установить материал объекта
	virtual void SetMaterial(PhysTriangleMaterialID mtl) { material = mtl; };
	// получить материал объекта
	virtual PhysTriangleMaterialID GetMaterial() { return material; }

	// установить физический материал
	virtual void SetPhysMaterial(IPhysMaterial * material) = null;
	// получить физический материал
	virtual IPhysMaterial * GetPhysMaterial() const = null;

	// получить линейную составляющую скорости
	virtual Vector GetLinearVelocity() const = null;

private:
	PhysTriangleMaterialID material;
};

//Плоскость
class IPhysPlane : public IPhysRigidBody
{
protected:
	IPhysPlane(IPhysicsScene * _scene) : IPhysRigidBody(_scene){}
	virtual ~IPhysPlane(){}
public:
	//Удалить
	virtual bool Release(){ return IPhysRigidBody::Release(); }
	//Установить плоскость
	virtual void SetPlane(Plane & p) = null;
	//Получить плоскость
	virtual Plane GetPlane() = null;

	//Установить группу
	virtual void SetGroup(PhysicsCollisionGroup group) = null;
	//Получить группы
	virtual PhysicsCollisionGroup GetGroup() = null;
};

//Ящик
class IPhysBox : public IPhysRigidBody
{
protected:
	IPhysBox(IPhysicsScene * _scene) : IPhysRigidBody(_scene){}
	virtual ~IPhysBox(){}
public:
	//Установить размер ящика
	virtual void SetSize(const Vector & size) = null;
	//Получить размер ящика
	virtual Vector GetSize() = null;

};

//Сфера
class IPhysSphere : public IPhysRigidBody
{
protected:
	IPhysSphere(IPhysicsScene * _scene) : IPhysRigidBody(_scene){}
	virtual ~IPhysSphere(){}
public:
	//Установить радиус
	virtual void SetRadius(float radius) = null;
	//Получить радиус
	virtual float GetRadius() = null;	
};

//Капсула
class IPhysCapsule : public IPhysRigidBody
{
protected:
	IPhysCapsule(IPhysicsScene * _scene) : IPhysRigidBody(_scene){}
	virtual ~IPhysCapsule(){}
public:
	//Установить радиус
	virtual void SetRadius(float radius) = null;
	//Получить радиус
	virtual float GetRadius() = null;
	//Установить высоту
	virtual void SetHeight(float height) = null;
	//Получить высоту
	virtual float GetHeight() = null;

};

//Объект состоящий из сетки треугольников
class IPhysMesh : public IPhysRigidBody
{
protected:
	IPhysMesh(IPhysicsScene * _scene) : IPhysRigidBody(_scene){}
	virtual ~IPhysMesh(){}
public:


};

//Комбинированный объект
class IPhysCombined : public IPhysRigidBody
{
protected:
	IPhysCombined(IPhysicsScene * _scene) : IPhysRigidBody(_scene){}
	virtual ~IPhysCombined(){}

public:
	//Тип фигуры
	enum Type
	{
		t_error,
		t_box,
		t_sphere,
		t_capsule,
		t_forcedword = 0x7fffffff
	};

	typedef unsigned long MaterialID;

	struct RaycastResult
	{
		Vector position;			//Мировая позиция точки пересечения
		Vector normal;				//Нормаль к поверхности в мировой системе
		float distance;				//Дистанция до точки
		MaterialID mtl;				//Идентификатор материала
	};

public:
	//Добавить ящик
	virtual void AddBox(const Vector & size, const Matrix & transform) = null;
	//Добавить шар
	virtual void AddSphere(float radius, const Matrix & transform) = null;
	//Добавить капсулу
	virtual void AddCapsule(float radius, float height, const Matrix & transform) = null;
	//Установить массу элемента
	using IPhysRigidBody::SetMass; // вносим перегрузки из базового
	virtual bool SetMass(unsigned int index, float mass) = null;
	//Получить текущее число фигур для билда
	virtual unsigned int GetCountForBuild() = null;
	//Сконструировать объект
	virtual void Build() = null;

public:
	//Получить количество фигур
	virtual long GetCount() = null;
	//Установить локальную позицию
	virtual bool SetLocalTransform(long index, const Matrix & transform) = null;
	//Получить локальную позицию
	virtual bool GetLocalTransform(long index, Matrix & transform) = null;
	//Получить тип
	virtual Type GetType(long index) = null;
	//Получить параметры ящика
	virtual bool GetBox(long index, Vector & size) = null;
	//Получить параметры шара
	virtual bool GetSphere(long index, float & radius) = null;
	//Получить параметры капсулы
	virtual bool GetCapsule(long index, float & radius, float & height) = null;
	//Установить глобальную позицию
	virtual bool SetGlobalTransform(unsigned int index, const Matrix & transform) = null;
	//Получить глобальную позицию
	virtual bool GetGlobalTransform(unsigned int index, Matrix & transform) = null;
	//Включить/выключить коллизии
	using IPhysRigidBody::EnableCollision;
	virtual void EnableCollision(unsigned int index, bool enable) = null;
	//Включить/выключить реакцию на коллизии
	virtual void EnableResponse(unsigned int index, bool enable) = null;
	//Включить/выключить рэйкаст
	virtual void EnableRaycast(unsigned int index, bool enable) = null;
	//Включить/выключить визуализацию
	virtual void EnableVisualization(unsigned int index, bool enable) = null;
	//Включить/выключить визуализацию для актера
	virtual void EnableVisualization(bool enable) = null;
	// установить кол-во итерация солвера для актера
	virtual void SetSolverIterations(unsigned int count) = null;

	//Протестировать элемент на пересечение с лучом
	virtual bool Raycast(unsigned int index, const Vector& from, const Vector& to, RaycastResult * details = null) = null;
	//Протестировать элемент на пересечение с боксом
	virtual bool OverlapBox(unsigned int index, const Vector& size, const Matrix& transform ) = null;
	//Протестировать элемент на пересечение со сферой
	virtual bool OverlapSphere(unsigned int index, const Vector& center, float radius) = null;
};

class IPhysRigidBodyConnector : public IPhysBase
{
protected:
	IPhysRigidBodyConnector(IPhysicsScene * _scene) : IPhysBase(_scene){}
	virtual ~IPhysRigidBodyConnector(){}
public:
	typedef void (_cdecl Object::* BrokeEvent)(IPhysRigidBodyConnector * joint);

public:
	//Сломано соединение или нет
	virtual bool IsBroke() = null;
	//Поставить обработчик
	virtual void SetEventHandler(BrokeEvent * event) = null;
	//Получить первый объект к которому присоеденены
	virtual IPhysRigidBody * GetLeft() = null;
	//Получить второй объект к которому присоеденены
	virtual IPhysRigidBody * GetRight() = null;

};


