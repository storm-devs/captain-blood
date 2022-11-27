//================================================================================================
//Система бонусов
//================================================================================================

#ifndef _Bonuses_base_h_
#define _Bonuses_base_h_

#include "mission.h"

//================================================================================================

//Описание общих параметров бонусов
#define BONUSBASE_PARAMS     \
MOP_GROUPBEGC("Drop FX", "Effects for drop moment") \
	MOP_STRING("Particles | Drop |", "") \
	MOP_STRING("Sound | Drop |", "") \
MOP_GROUPEND() \
MOP_GROUPBEGC("Wait FX", "Effects for waiting pickup state") \
	MOP_STRING("Particles | Wait |", "") \
	MOP_STRING("Sound | Wait |", "") \
MOP_GROUPEND() \
MOP_GROUPBEGC("Jump FX", "Effects for phtysics reflection from sides, floor, etc...") \
	MOP_STRING("Particles | Jump |", "") \
	MOP_STRING("Sound | Jump |", "") \
MOP_GROUPEND() \
MOP_GROUPBEGC("Pickup fly FX", "Effects for start flying bonus before pickup") \
	MOP_STRING("Particles | Fly |", "") \
	MOP_STRING("Sound | Fly |", "") \
MOP_GROUPEND() \
MOP_GROUPBEGC("Pickup FX", "Effects for pickup moment") \
	MOP_STRING("Particles | Pickup |", "") \
	MOP_STRING("Sound | Pickup |", "") \
MOP_GROUPEND() \
MOP_GROUPBEGC("Fade FX", "Effects for fade moment, if bonus don't taked") \
	MOP_STRING("Particles | Fade |", "") \
	MOP_STRING("Sound | Fade |", "") \
MOP_GROUPEND() \
MOP_STRINGC("Model", "", "Bonus's model name") \
MOP_FLOATC("Model scale", 1.0f, "Scale of model") \
MOP_POSITIONC("Position", Vector(0.0f), "Local model position") \
MOP_ANGLESC("Angles", Vector(0.0f), "Local model orientation")  \
MOP_FLOATEXC("Radius", 0.2f, 0.001f, 1000.0f, "Bonus's radius in meters") \
MOP_FLOATEXC("Pickup radius", 0.8f, 0.001f, 1000.0f, "Bonus pickup radius in meters") \
MOP_FLOATEXC("Live time", 0.0f, 0.0f, 1000.0f, "Bonus live time in seconds. If is 0 live time is infinite") \
MOP_GROUPBEGC("Movement", "Parameters for bonus movement") \
	MOP_FLOATC("Height", 0.5f, "Height of model lift up") \
	MOP_FLOATC("Rotate speed", 0.5f, "Model rotate speed (turn/sec)") \
	MOP_FLOATC("Amplidute", 0.2f, "Amplitude of lifting" ) \
	MOP_FLOATC("Lift speed", 0.4f, "Speed of lifting (m/sec)" ) \
MOP_GROUPEND() \
MOP_FLOATC("Glow size",0.1f,"Bonus's glow size (horizontal and vertical)") \
MOP_COLORC("Glow color", 0xFFFFFFFF, "Bonus's glow color") \
MOP_LONGEXC("Glow frame", -1, -1, 3, "Bonus's glow frame index into glow texture (2x2: top left = index 0)")


//Параметры задающие режим использования бонуса
#define BONUSBASE_WORKTIME	MOP_FLOATEXC("Work time", 0.0f, 0.0f, 1000.0f, "Bonus work time in seconds. If is 0 work time is infinite")
#define BONUSBASE_FREQ		MOP_FLOATEXC("Work frequency", 2.0f, 0.0f, 1000.0f, "Bonus work frequency in ticks per secont. If freq. is bonus work is one time.")
#define BONUSBASE_APPCOUNT	MOP_LONGEXC("Work ticks", 1, 1, 1000000, "Bonus work ticks count.")





//!!! Для бонусов нужно установить данный уровень создания
#define BONUSBASE_LEVEL		1


//================================================================================================

class IGreedy
{
protected:
	virtual ~IGreedy() {}

public:
	virtual void SetPosition(const Vector& pos) = null;
	virtual const Vector & GetPosition() = null;
	virtual MissionObject* OwnerMO() = null;

	virtual void Release() = null;
};


//Базовый класс бонуса, который требует расширения до базового класса
//под конкретную разновидность игрока в виде реализации функций
//float GetKoefProbability() и bool CollisionCheck(const Vector & pos, float radius)
//Далее расширенный под игрока класс используется в качестве базового для
//бонусов создаваемых для данного игрока
class BonusBase : public MissionObject
{
public:
	struct FX
	{
		FX()
		{
		};

		ConstString particles;
		ConstString sound;
	};

	struct CreationParams
	{
		CreationParams()
		{
			radius = 1.0f;			
			liveTime = 0.0f;
			applyCount = 1;
			workTime = 0.0f;
			freq = 0.0f;
		};

		FX drop;					//Эффект выподания
		FX wait;					//Эффект сопровождающий бонус при ожидании
		FX jump;					//Эффект при отскоке бонуса от стенки
		FX pickupfly;				//Эффект начала поднятия бонуса (начало полёта)
		FX pickup;					//Эффект поднятия бонуса
		FX fade;					//Эффект исчезновения при таймауте
		const char * modelName;		//Имя модельки, отображающий бонус
		float radius;				//Радиус бонуса
		float pickupRadius;			//Радиус в котором берётся бонус
		float liveTime;				//Время жизни бонуса, если <= 0 то бесконечное время
		long applyCount;			//Количество вызовов apply, если <= 0 то учитывается только время работы и частоту
		float workTime;				//Время работы бонуса при взятии, если <= 0 то бесконечное время
		float freq;					//С какой приблизительно частотой вызывать apply при работе, если <= 0 то только раз сработает
		Matrix localModelTransform;	//Локальное положение модели в бонусе
		float moveheight;			//Высота поднятия бонуса
		float moverotspeed;			//скорость вращения бонуса
		float moveamplitude;		//амплитуда перемещения бонуса(вверх/вниз)
		float moveliftspeed;		//скорость перемещения бонуса(вверх/вниз)
		float modelscale;			//масштаб модельки бонуса
		float glowSize;				//размер глоу (ширина=высота)
		dword glowColor;			//цвет глоу
		long glowFrame;				//кадр билборда из текстуры глоу (-1 = нет глоу)
	};

	class UserBonus
	{
	public:
		//Пораметры для модификации
		struct UpdateParams
		{
			IRender * render;	//in
			float dltTime;		//in
			Matrix mtx;			//in-out
			float alpha;		//in-out
			float scale;		//in-out
			bool isLive;		//in-out
		};

	protected:
		UserBonus(){};
		virtual ~UserBonus(){};

	public:
		virtual void Init(const Matrix & initMatrix) = null;
		virtual void Release() = null;
		virtual void Update(UpdateParams & params) = null;
	};

//--------------------------------------------------------------------------------------------
//Работа бонуса
//--------------------------------------------------------------------------------------------
public:
	//Готов ли для взятия этим объектом
	virtual bool ReadyForPickup(IGreedy* greedy)
	{
		return true;
	}

	//Применить данный бонус к игроку, если по каким либо причинам надо досрочно прервать работу бонуса то надо вернуть false
	virtual bool Apply(IGreedy* greedy) = null;

//--------------------------------------------------------------------------------------------
//Методы для адаптации бонуса под данный класс плеера
//--------------------------------------------------------------------------------------------
public:	
	//Получить относительное количество поинтов у игрока 0..1 (0 == 0%, 1 == 100%)
	virtual float GetPoints() = null;
	
	//Найти пересечение колижена и отрезка, если нет пересечения вернуть false
	virtual bool CollisionLine(const Vector & start, const Vector & end, Vector & p, Vector & n)
	{
		p = start;
		n = Vector(0.0f, 1.0f, 0.0f);
		return true;
	}
	//Проверить детально пересечение сферы бонуса и игрока
	virtual bool CollisionCheck(const Vector & pos, float radius)
	{
		return true;
	}


//--------------------------------------------------------------------------------------------
//Дополнительные возможности
//--------------------------------------------------------------------------------------------
public:
	//Создать дополнительный объект управления бонусом
	virtual UserBonus * CreateUserBonus()
	{
		return null;
	};
	
	//Указать количество создаваемых бонусов при дропе
	virtual long DropsCount()
	{
		return 1;
	}

//--------------------------------------------------------------------------------------------
//Утилитные функции
//--------------------------------------------------------------------------------------------
protected:
	//Прочитать параметры из стандартного списка
	void ReadStdParams(MOPReader & reader)
	{
		creationParams.drop.particles = reader.String();
		creationParams.drop.sound = reader.String();
		creationParams.wait.particles = reader.String();
		creationParams.wait.sound = reader.String();
		creationParams.jump.particles = reader.String();
		creationParams.jump.sound = reader.String();
		creationParams.pickupfly.particles = reader.String();
		creationParams.pickupfly.sound = reader.String();
		creationParams.pickup.particles = reader.String();
		creationParams.pickup.sound = reader.String();
		creationParams.fade.particles = reader.String();
		creationParams.fade.sound = reader.String();
		creationParams.modelName = reader.String().c_str();
		creationParams.modelscale = reader.Float();
		Vector locPos = reader.Position();
		Vector locAng = reader.Angles();
		creationParams.localModelTransform.Build(locAng, locPos);
		creationParams.radius = reader.Float();
		creationParams.pickupRadius = reader.Float();
		creationParams.liveTime = reader.Float();
		CacheModel(creationParams.modelName);
		creationParams.moveheight = reader.Float();
		creationParams.moverotspeed = reader.Float();
		creationParams.moveamplitude = reader.Float();
		creationParams.moveliftspeed = reader.Float();
		creationParams.glowSize = reader.Float();
		creationParams.glowColor = reader.Colors().GetDword();
		creationParams.glowFrame = reader.Long();
	}

public:
	inline const CreationParams & GetCreationParams() const
	{
		return creationParams;
	}

	MO_IS_FUNCTION(BonusBase, MissionObject);

	void CacheModel(const char * modelName);

//--------------------------------------------------------------------------------------------
protected:
	CreationParams creationParams;
};



//================================================================================================
//Бонус при выподании создаётся с помощью статической функции BonusesManager::CreateBonus()
//================================================================================================

//Менеджер бонусов
class BonusesManager : public MissionObject
{
public:
	struct DropParams
	{
		float minVy, maxVy;
		float minVxz, maxVxz;
		float minAy, maxAy;
		float friction;
		float skipLifeTime;
	};

public:
	//Cоздать бонус в соответствии с таблицей (может и не создатся если кубики не выпадут)
	static void CreateBonus(IMission & mis, const Vector & pos, const ConstString & tableName, DropParams * crParams = null)
	{
		BonusesManager * bm = CreateBonusManager(mis);
		if(bm)
		{
			bm->CreateBonusInside(pos, tableName, crParams);
		}
	};

	static void ResetAllBonuses(IMission & mis)
	{
		BonusesManager * bm = CreateBonusManager(mis);
		if(bm)
		{
			bm->ResetAllBonusesInside();
		}
	}

	static void CacheModel(IMission & mis, const char * modelName)
	{
		BonusesManager * bm = CreateBonusManager(mis);
		if(bm)
		{
			bm->CacheModelInside(modelName);
		}
	}

	static IGreedy* CreateGreedy(MissionObject* pOwnerMO)
	{
		if( !pOwnerMO ) return null;
		BonusesManager * bm = CreateBonusManager( pOwnerMO->Mission() );
		if(bm)
		{
			return bm->CreateGreedyInside( pOwnerMO );
		}
		return null;
	}


private:
	__forceinline static BonusesManager * CreateBonusManager(IMission & mis)
	{
		MOSafePointer sp;
		static const ConstString obj("_BonusesManagerImp_");
		mis.CreateObject(sp, "BonusesManagerImp", obj);
		return (BonusesManager *)sp.SPtr();
	}

protected:
	virtual void CreateBonusInside(const Vector & pos, const ConstString & tableName, DropParams * crParams) = null;
	virtual void ResetAllBonusesInside() = null;
	virtual void CacheModelInside(const char * modelName) = null;
	virtual IGreedy* CreateGreedyInside(MissionObject* pOwnerMO) = null;
};


__forceinline void BonusBase::CacheModel(const char * modelName)
{
	if(string::NotEmpty(modelName) && !EditMode_IsOn())
	{
		BonusesManager::CacheModel(Mission(), modelName);
	}	
}

#endif

