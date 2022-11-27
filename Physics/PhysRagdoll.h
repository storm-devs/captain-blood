

#include "..\Common_h\Physics.h"
#include "..\Common_h\Animation.h"
#include "PhysicsScene.h"

class PhysRagdoll : public IPhysEditableRagdoll
{
	class PhysBone : public IBone
	{
	public:
		PhysBone(PhysRagdoll & r, PhysBone * p);
		virtual ~PhysBone();


	//---------------------------------------------------------------------------
	//Утилитные
	//---------------------------------------------------------------------------
	public:
		bool CheckId(PhysRaycastId id);
		PhysBone * Find(const char * bname, dword bhash);
		void Activate(bool isActive);

	//---------------------------------------------------------------------------
	//Доступ к иерархии
	//---------------------------------------------------------------------------
	public:
		//Получить родителя
		virtual IBone * GetParent();
		//Получить количество детей
		virtual long ChildCount();
		//Получить ребёнка
		virtual IBone & GetChild(long index);
		//Добавить ребёнка
		virtual IBone & AddChild();
		//Установить имя кости
		virtual void SetBoneName(const char * name);
		//Получить имя кости
		const char * GetBoneName();

	//---------------------------------------------------------------------------
	//Установить коллидер кости
	//---------------------------------------------------------------------------
	public:
		//Представить кость ящиком
		virtual void SetShape(const Matrix & worldTransform, const Matrix & localTransform, const Vector & size, float mass);
		//Представить кость капсулой
		virtual void SetShape(const Matrix & worldTransform, const Matrix & localTransform, float height, float radius, float mass);

	//---------------------------------------------------------------------------
	//Суставы
	//---------------------------------------------------------------------------
	public:
		//Создать сферический сустав между текущей костью и ребёнком ребёнком
		virtual void CreateJoint(IBone & child, const SphericalJointParams & params);
		//Создать шарнирный сустав между текущей костью и ребёнком ребёнком
		virtual void CreateJoint(IBone & child, const RevoluteJointParams & params);

	//---------------------------------------------------------------------------
	public:
		//Устоновить кости позицию в мире
		void SetWorldTransform(const Matrix & mtx);
		//Получить мировые координаты кости
		virtual void GetWorldTransform(Matrix & mtx);
		//Приложить силу к кости в заданной локальной точке
		virtual void ApplyForce(const Vector & force, const Vector & localPosition);
		//Приложить импульс к кости в заданной локальной точке
		virtual void ApplyImpulse(const Vector & imp, const Vector & localPosition);
		//Приложить вращающий момент к рутовой кости в глобальном системе
		virtual void ApplyTorque(const Vector & imp);
		//Приложить вращающий момент к рутовой кости в локальной системе
		virtual void ApplyLocalTorque(const Vector & imp);
		//Установить линейную скорость
		void SetVelocity(const Vector & v, bool isRecursive = true);
		//Установить скорость вращения		
		void SetRotation(float ay);

		//Изменить групппу
		virtual void SetGroup(PhysicsCollisionGroup group);

	//---------------------------------------------------------------------------
	protected:
		//Удалить коллижен шейп
		void ReleaseActor();
		//Убить суставы у себя и детей
		void ReleaseJoint();

	//---------------------------------------------------------------------------
	protected:
		PhysBone * parent;					//Указатель на родителя
		NxActor * actor;					//Указатель на актёра, представляющего кость
		NxSphericalJoint * sphereJoint;		//Сферический сустав
		NxRevoluteJoint * revoluteJoint;	//Шаргнирный сустав
		PhysRagdoll & ragdoll;				//Кому принадлежим
		string name;						//Имя анимационной кости
		dword hash;							//Хэшь для имени анимационной кости
		array<PhysBone *> child;			//Дети
	};

	class BlendStage : public IAniBlendStage
	{
	public:
		BlendStage(PhysRagdoll & r, IAnimation * ani);
		virtual ~BlendStage();
		//Обновить текущий уровень
		void UpdateLevel(dword lvl);
		//Установить знак и время блендинга
		void SetBlendTime(float time, const Matrix & transform);
		//Текущие состояние стадии
		bool IsActive();
		//Заморозить стадию 
		void Freeze();

	private:
		//Получить коэфициент блендинга для интересующей кости
		virtual float GetBoneBlend(long boneIndex);
		//Получить трансформацию кости
		virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale);
		//Обновить
		virtual void Update(float dltTime);
	private:
		struct BSBone
		{
			PhysBone * physBone;
			Vector position;
			Quaternion rotation;
		};
		dword level;
		PhysRagdoll & ragdoll;
		float currentBlend;
		float dltBlend;
		Matrix iworld;
		array<BSBone> bones;
		bool isActive;
	};

#pragma pack(push, 1)

	/*
	SaveDataHeader
	{
		SaveDataBone
		SaveDataBox / SaveDataCapsule
		SaveDataSphericalJoint / SaveDataRevoluteJoint
		byte[SaveDataBone.namelength]
	}[SaveDataHeader.bonesCount]
	*/


	struct SaveDataHeader
	{
		byte id[16];		//Идентификатор
		byte ver[4];		//Версия
		dword bonesCount;	//Количество костей
	};

	struct SaveDataBone
	{
		enum ShapeType
		{
			st_unkown = 0,
			st_box = 1,
			st_capsule = 2
		};
		enum JointType
		{
			jt_unkown = 0,
			jt_spherical = 1,
			jt_revolute = 2
		};
		short parent;		//Индекс родительской кости
		byte shapeType;		//Тип фигуры
		byte jointType;		//Тип сустава
		dword namelength;	//Длинна имени вместе с 0
	};

	struct SaveDataBox
	{
		float worldTransform[12];
		float localTransform[12];
		float size[3];
		float mass;
	};

	struct SaveDataCapsule
	{
		float worldTransform[12];
		float localTransform[12];
		float height;
		float radius;
		float mass;
	};

	//Параметры сферического сустава
	struct SaveDataSphericalJoint
	{
		float worldJointPosition[3];		//Мировая позиция кости
		float swingAxisInParentSystem[3];	//Направление соединения вокруг которого образуется конус
		float swingLimit;					//Угол в радианах, ограничивающего конуса 0...PI
		float swingSpring;					//Сила тянущяя сустав к направлению соединения
		float swingDamper;					//Коэфициент демпфирования для разгибания
		float twistAxisInParentSystem[3];	//Направление нескрученого сустава
		float twistMin;						//Минимальный угол скручивания -PI...PI
		float twistMax;						//Максимальный угол скручивания -PI...PI
		float twistSpring;					//Сила возврата в нескрученое состояние
		float twistDamper;					//Коэфициент демпфирования для скручивания
	};

	//Параметры шарнирного сустава
	struct SaveDataRevoluteJoint
	{
		float worldJointPosition[3];		//Мировая позиция кости
		float axisInParentSystem[3];		//Ось в системе родителя вокруг которой вращается сустав
		float normalInParentSystem[3];		//Нормаль в системе родителя вокруг которой задаются лимиты
		float minAngle;						//Минимальный угол отклонения -PI...PI
		float maxAngle;						//Максимальный угол отклонения -PI...PI
		float spring;						//Сила тянущяя сустав к направлению соединения
		float damper;						//Коэфициент демпфирования для разгибания
	};

#pragma pack(pop)

	struct SaveDataInfo
	{
		void Init();
		void Release();

		IBone * bone;
		SaveDataBox * boxData;
		SaveDataCapsule * capsuleData;
		SaveDataSphericalJoint * sphericalJoint;
		SaveDataRevoluteJoint * revoluteJoint;
	};

public:
	struct ExceptionObj
	{
		const char * error;
	};

public:
	PhysRagdoll(IPhysBase * proxy, const char * filename, long fileline, IPhysicsScene * _scene);
	PhysRagdoll(IPhysBase * proxy, const char * filename, long fileline, const void * data, dword size, IPhysicsScene * _scene);
	virtual ~PhysRagdoll();

	//Убивает физику, оставляя последние параметры костей
	virtual void Freeze();
	//Текущие состояние регдолла - заморожен ли
	virtual bool IsFreezed();

	//Очистить для создания нового
	virtual void Clear();
	//Получить рутовую кость
	virtual IBone & GetRootBone();
	//Получить кость по идентификатору рэйкаста
	virtual IBone * GetBone(PhysRaycastId id);

	//Установить блендер для анимацию
	virtual void SetBlendStage(IAnimation * ani, dword level);
	//Удалить блендер для анимации
	virtual void RemoveBlendStage(IAnimation * ani);
	//Включить блэндер
	virtual void Activate(float blendTime, const Matrix & transform);
	//Выключить блэндер
	virtual void Deactivate(float blendTime, Matrix & transform);

	//Установить линейную скорость
	virtual void SetVelocity(const Vector & vel, bool isRecursive);
	//Установить угловую скорость
	virtual void SetRotation(float ay);
	//Применить силу к рутовой кости
	virtual void ApplyForce(const Vector & force);
	//Применить импульс к рутовой кости
	virtual void ApplyImpulse(const Vector & imp);
	//Приложить вращающий момент к рутовой кости в глобальном системе
	virtual void ApplyTorque(const Vector & imp);
	//Приложить вращающий момент к рутовой кости в локальной системе
	virtual void ApplyLocalTorque(const Vector & imp);

	//Изменить групппу
	virtual void SetGroup(PhysicsCollisionGroup group);

	//Сохранить параметры рэгдола в массив
	virtual bool BuildSaveData(array<byte> & buffer);

	//Загрузился ли регдол
	bool IsLoaded();

public:
	//Получить сцену, которой принадлежим
	NxScene & Scene();
	//Добавить кость
	void AddBone(IBone * bone);
	//Установить ящик для кости
	void SetBoneShape(IBone * bone, const Matrix & worldTransform, const Matrix & localTransform, const Vector & size, float mass);
	//Установить капсулу для кости
	void SetBoneShape(IBone * bone, const Matrix & worldTransform, const Matrix & localTransform, float height, float radius, float mass);
	//Установить сферический сустав для ребёнка и его родителя
	void SetBoneJoint(IBone * bone, const SphericalJointParams & params);
	//Установить шарнирный сустав для ребёнка и его родителя
	void SetBoneJoint(IBone * bone, const RevoluteJointParams & params);
	//Создать ящик опираясь на сохранённые данные
	void CreateBoxFromSaveData(IBone * bone, const SaveDataBox & data);
	//Создать капсулу опираясь на сохранённые данные
	void CreateCapsuleFromSaveData(IBone * bone, const SaveDataCapsule & data);
	//Создать сферический сустав опираясь на сохранённые данные
	void CreateSphericalJointFromSaveData(IBone * bone, const SaveDataSphericalJoint & data);
	//Создать шарнирный сустав опираясь на сохранённые данные
	void CreateRevoluteJointFromSaveData(IBone * bone, const SaveDataRevoluteJoint & data);
	
	//Обновить состояние рэгдола
	void UpdateActiveState();

	// отписаться от всех активных анимационных блендеров
	void RemoveBlendStages();

private:
	//Получить описания кости по указателю
	SaveDataInfo & GetSaveDataInfo(IBone * bone);
	//Сконвертировать вектор
	static void ConvertVector(float v[3], const Vector & vct);
	//Сконвертировать вектор
	static void ConvertVector(Vector & vct, const float v[3]);
	//Сконвертировать матрицу
	static void ConvertMatrix(float m[12], const Matrix & mtx);
	//Сконвертировать матрицу
	static void ConvertMatrix(Matrix & mtx, const float m[12]);
	//Добавить данные в сейв
	static void AddSaveData(array<byte> & buffer, const void * ptr, dword size);
	//Прочитать данные
	template <class T> static const T & GetSaveData(dword & ptr, const void * data, dword dataSize);
	//Прочитать данные
	static const void * GetSaveData(dword & ptr, const void * data, dword dataSize, dword size);
	//Активировать/деактивировать рэгдол
	void Activate(bool isAct);


private:
	PhysBone * root;
	array<BlendStage *> stages;
	array<SaveDataInfo> * bones;
	bool isActive;
	bool isFreezed;
};

