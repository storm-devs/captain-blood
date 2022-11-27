
class IAnimation;

class IPhysRagdoll : public IPhysBase
{
protected:
	IPhysRagdoll(IPhysicsScene * _scene) : IPhysBase(_scene){};
	virtual ~IPhysRagdoll(){};
public:
	//Установить блендер для анимацию
	virtual void SetBlendStage(IAnimation * ani, dword level) = null;
	//Удалить блендер для анимации
	virtual void RemoveBlendStage(IAnimation * ani) = null;
	//Включить блэндер
	virtual void Activate(float blendTime, const Matrix & transform) = null;
	//Выключить блэндер
	virtual void Deactivate(float blendTime, Matrix & transform) = null;
	//Установить линейную скорость
	virtual void SetVelocity(const Vector & vel, bool isRecursive = true) = null;
	//Установить угловую скорость
	virtual void SetRotation(float ay) = null;
	//Применить силу к рутовой кости
	virtual void ApplyForce(const Vector & force) = null;
	//Применить импульс к рутовой кости
	virtual void ApplyImpulse(const Vector & imp) = null;
	//Приложить вращающий момент к рутовой кости в глобальном системе
	virtual void ApplyTorque(const Vector & imp) = null;
	//Приложить вращающий момент к рутовой кости в локальной системе
	virtual void ApplyLocalTorque(const Vector & imp) = null;

	//Изменить групппу
	virtual void SetGroup(PhysicsCollisionGroup group) = null;

	//Убивает физику, оставляя последние параметры костей
	virtual void Freeze() = null;
	//Текущие состояние регдолла - заморожен ли
	virtual bool IsFreezed() = null;
};


class IPhysEditableRagdoll : public IPhysRagdoll
{
public:

	//Параметры сферического сустава
	struct SphericalJointParams
	{
		Vector worldJointPosition;			//Мировая позиция кости
		Vector swingAxisInParentSystem;		//Направление соединения вокруг которого образуется конус
		float swingLimit;					//Угол в радианах, ограничивающего конуса 0...PI
		float swingSpring;					//Сила тянущяя сустав к направлению соединения
		float swingDamper;					//Коэфициент демпфирования для разгибания
		Vector twistAxisInParentSystem;		//Направление нескрученого сустава
		float twistMin;						//Минимальный угол скручивания -PI...PI
		float twistMax;						//Максимальный угол скручивания -PI...PI
		float twistSpring;					//Сила возврата в нескрученое состояние
		float twistDamper;					//Коэфициент демпфирования для скручивания
	};

	//Параметры шарнирного сустава
	struct RevoluteJointParams
	{
		Vector worldJointPosition;			//Мировая позиция кости
		Vector axisInParentSystem;			//Ось в системе родителя вокруг которой вращается сустав
		Vector normalInParentSystem;		//Нормаль в системе родителя вокруг которой задаются лимиты
		float minAngle;						//Минимальный угол отклонения -PI...PI
		float maxAngle;						//Максимальный угол отклонения -PI...PI
		float spring;						//Сила тянущяя сустав к направлению соединения
		float damper;						//Коэфициент демпфирования для разгибания
	};

	//Представление кости
	class IBone
	{
	protected:
		IBone(){};
		virtual ~IBone(){};

	//---------------------------------------------------------------------------
	//Доступ к иерархии
	//---------------------------------------------------------------------------
	public:
		//Получить родителя
		virtual IBone * GetParent() = null;
		//Получить количество детей
		virtual long ChildCount() = null;
		//Получить ребёнка
		virtual IBone & GetChild(long index) = null;
		//Добавить ребёнка
		virtual IBone & AddChild() = null;
		//Установить имя кости
		virtual void SetBoneName(const char * name) = null;

	//---------------------------------------------------------------------------
	//Установить коллидер кости
	//---------------------------------------------------------------------------
	public:
		//Представить кость ящиком
		virtual void SetShape(const Matrix & worldTransform, const Matrix & localTransform, const Vector & size, float mass) = null;
		//Представить кость капсулой
		virtual void SetShape(const Matrix & worldTransform, const Matrix & localTransform, float height, float radius, float mass) = null;

	//---------------------------------------------------------------------------
	//Суставы
	//---------------------------------------------------------------------------
	public:
		//Создать сферический сустав между текущей костью и ребёнком ребёнком
		virtual void CreateJoint(IBone & child, const SphericalJointParams & params) = null;
		//Создать шарнирный сустав между текущей костью и ребёнком ребёнком
		virtual void CreateJoint(IBone & child, const RevoluteJointParams & params) = null;

	//---------------------------------------------------------------------------
	//Расное
	//---------------------------------------------------------------------------
	public:
		//Получить мировые координаты кости
		virtual void GetWorldTransform(Matrix & mtx) = null;
		//Приложить силу к кости в заданной локальной точке
		virtual void ApplyForce(const Vector & worldForce, const Vector & localPosition) = null;

		//Изменить групппу
		virtual void SetGroup(PhysicsCollisionGroup group) = null;
	};

protected:
	IPhysEditableRagdoll(IPhysicsScene * _scene) : IPhysRagdoll(_scene){};
	virtual ~IPhysEditableRagdoll(){};

public:
	//Очистить для создания нового
	virtual void Clear() = null;
	//Получить рутовую кость
	virtual IBone & GetRootBone() = null;
	//Получить кость по идентификатору рэйкаста
	virtual IBone * GetBone(PhysRaycastId id) = null;

	//Сохранить параметры рэгдола в массив
	virtual bool BuildSaveData(array<byte> & buffer) = null;


};


