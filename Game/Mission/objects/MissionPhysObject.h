//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// MissionPhysObject
//============================================================================================

#ifndef _MissionPhysObject_h_
#define _MissionPhysObject_h_


#include "MissionPhysObjPattern.h"
#include "..\..\..\Common_h\IMissionPhysObject.h"
#include "..\..\..\Common_h\ITips.h"
#include "..\..\..\Common_h\bonuses.h"

class MissionPhysObject : public IMissionPhysObject
{
	enum DamageType
	{		
		dt_sword = DamageReceiver::ds_sword,
		dt_bomb = DamageReceiver::ds_bomb,
		dt_bullet = DamageReceiver::ds_bullet,
		dt_cannon = DamageReceiver::ds_cannon,
		dt_flame = DamageReceiver::ds_flame,
		dt_shooter = DamageReceiver::ds_shooter,
		dt_check = DamageReceiver::ds_check,
		dt_unknown = DamageReceiver::ds_unknown,		
		dt_pickup = ds_pickup,
	};

	enum Flags
	{
		f_part_isCollision = 1,
		f_part_isDrawInAlphaLevel = 2,

		f_isBroken = 1,						//Состояние объекта
		f_isShock = 2,						//
		f_isShowTips = 4,					//Показывать ли типсы
		f_isFadeProcess = 8,				//Фэйдиться
		f_isDead = 16,						//Помер
		f_needCalcBox = 32,					//Нужно пересчитать ящик
		f_drawDebugBoxes = 64,				//Нарисовать коллизион боксы
		f_isCheckLod = 128,					//Проверять ли дистанцию скрытия
		f_hackHide = 512,					//Флажёк отключения отрисовки физических объектов при состоянии включено

	};

	struct Part
	{		
		IPhysCombined * physObject;
		IGMXScene * model;
		IGMXScene * loadedModel;
		Matrix mtx;
		Vector impPos;
		Vector impDir;
		long count;
		Vector effectsPos;
		long effectsCount;
		float alpha;
		float curAlpha;
		long descIndex;
		ITip * tip;
		dword flags;
		dword sndMatMask;									//Маска использования материалов
		float sndCooldown;									//Время блокировки запуска следующего звука
	};

	struct ABB
	{
		Vector min;
		Vector max;
	};


//--------------------------------------------------------------------------------------------
public:
	MissionPhysObject();
	virtual ~MissionPhysObject();

//--------------------------------------------------------------------------------------------
private:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Пересоздать объект
	virtual void Restart();
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Устоновить состояние подписки
	void SetShow(bool isShow);
	//Обновить состояние типсов
	void ShowTips();
	//Установить состояние подсказки
	void SetTipsState(array<Part> & obj, bool isActive);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Обновление позиций
	void _cdecl Update(float dltTime, long level);
	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);
	//Нарисовать модельку для тени
	void _cdecl ShadowInfo(const char * group, MissionObject * sender);
	//Нарисовать модельку для тени
	void _cdecl ShadowDraw(const char * group, MissionObject * sender);
	//Нарисовать отражёную модельку
	void _cdecl SeaReflection(const char * group, MissionObject * sender);
	//Нарисовать модельку
	void _cdecl EditMode_Draw(float dltTime, long level);
	//Отсчёт исчезновения
	void _cdecl HideTimer(float dltTime, long level);
	//Процесс исчезновения, или другого окончания объекта
	void _cdecl Fader(float dltTime, long level);

	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	void ApplyImpulse(Part & p, const Matrix & boxTransform, const Vector & spherePos, float sphereRadius, float hp, DamageType dtype);
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	void ApplyImpulse(Part & p, const Matrix & boxTransform, const Vector & from, const Vector & to, float hp, DamageType dtype);
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);
	void ApplyImpulse(Part & p, const Matrix & boxTransform, const Vector vrt[4], float hp, DamageType dtype);
	//Применить импульс к части зная параметры импульса
	__forceinline void ApplyImpulseResult(Part & p, const Vector & impulsePos, const Vector & impulseDir, float kAttenuation, float hp, DamageType dtype, const Vector & effectsPos);
	//Применить импульсы
	void ApplyImpulses(DamageType type);
	//Умирает
	virtual bool IsDie();
	//Мёртв
	virtual bool IsDead();

	//Проверить на пересечение отрезка и ящиков описывающих объекты
	virtual bool OverlapLine(const Vector & v1, const Vector & v2, float skin);

public:
	//Патерн удаляется
	void DeletePattern();

private:
	//Прочитать параметры
	void ReadParameters(MOPReader & reader);
	//Получить поинтер на патерн
	void UpdatePatternPointer();
	//Отиграть эффект падения в воду
	void WaterDrop(Part & part, Vector & pos1, Vector & pos2, float level);


public:
	//Перестроить данные с паттерна
	void UpdatePatternData();

private:
	//Создать физические объекты
	void MakePhysics(array<Part> & obj);
	//Удалить физические объекты
	void ReleasePhysics(array<Part> & obj);
	//Удалить модельки
	void ReleaseModels(array<Part> & obj);
	//Удалить данные
	void Release();

	//Сломать
	void Broke();
	//Модифицировать наносимое повреждение с учётом коэфициентов демеджа
	float ModifyHp(DamageType type, float hp);
	//Нанести повреждение
	bool ApplyDamage(float damage, DamageType type);

	//Найти видимый ABB
	void FindVisibleABB(array<Part> & obj, ABB & abb);
	//Найти для физики ABB
	void FindCollidersABB(array<Part> & obj, ABB & abb);

private:
	static DamageType DetectType(dword source);

private:
	//Переместить целую часть в новую позицию
	virtual bool SetPartMatrix(long partIndex, const Matrix & mtx);
	//Получить текущую матрицу целой части объекта
	virtual bool GetPartMatrix(long partIndex, Matrix & mtx);
	//Получить пивот целой части
	virtual bool GetPartPivot(long partIndex, Matrix & mtx);
	//Применить импульс в мировых координатах
	virtual void Impulse(const Vector & dir);
	//Сломать объект
	virtual void BrokeObject();
	//Показать-скрыть подсказки
	virtual void ShowTips(bool isShow);
	//Найти индекс локатора по имени, -1 если нет такого
	virtual long GetLocatorIndexByName(const ConstString & name);
	//Получить позицию локатора по индексу
	virtual bool GetLocator(long index, Matrix & mtx);

	static __forceinline void ResetFlag(dword & f, dword value);
	static __forceinline void SetFlag(dword & f, dword value);
	static __forceinline bool CheckFlag(dword f, dword value);


//--------------------------------------------------------------------------------------------
private:
	MOSafePointerType<MissionPhysObjPattern> pattern;	//Исходный объект
	float HP;											//Жизни
	dword flags;										//Флаги
	Vector boxMin, boxMax;								//Локальный ящик
	float hideTime;										//Время ожидания до исчезновения
	ConstString patternName;							//Имя исходного объекта
	Matrix initMtx;										//Матрица начального положения
	long chacheIndex;									//Индекс проверки указателя
//	float globalAlpha;									//Текущая прозрачность
	float fadeOutTime;									//Счётчик протухания
	float kShockTime;									//Время тряски
	float shockTime;									//Время тряски
	float shockWaitTime;								//Время между импульсами тряски
	float shockWaitTimeMid;								//Время между импульсами тряски
	float shockForce;									//Сила тряски
	array<Part> solid;									//Цельный объект
	array<Part> broken;									//Сломаный объект
	ABB visAbb;											//Видимый описывающий ящик
	ABB colAbb;											//Коллидер верхнего уровня
	MOSafePointer waterLevel;							//Сохранёный объект получения уровня
	MissionTrigger brokeEvent;							//Триггер, срабатывающий при поломке объекта
	IMissionQTObject * aiColider;						//Препятствие для AI
	
	
	
	/*
	
	struct DLine
	{
		DLine(){}

		DLine(const Vector & _from, dword _cf, const Vector & _to, dword _ct)
		{
			from = _from;
			to = _to;
			cf = _cf;
			ct = _ct;
		}

		Vector from, to;
		dword cf, ct;
	};
	
	
	array<DLine> lines;

	*/

};

#endif

