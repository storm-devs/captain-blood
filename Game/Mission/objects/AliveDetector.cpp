//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// AliveDetector	
//============================================================================================
			

#include "AliveDetector.h"

//============================================================================================

AliveDetector::AliveDetector() : aiObjects(_FL_)
{
	isDown = false;
	autoReset = false;
	time = -1.0f;
	delayTime = 0.0f;
	watchDieFact = false;
	deadState = true;
}

AliveDetector::~AliveDetector()
{
}


//============================================================================================


//Инициализировать объект
bool AliveDetector::Create(MOPReader & reader)
{
	aiObjects.DelAll();
	isDown = false;
	autoReset = false;
	time = -1.0f;
	delayTime = 0.0f;
	watchDieFact = false;
	deadState = true;
	if(!DetectorObject::Create(reader)) return false;
	isDown = false;
	Show(false);
	//Составляем список объектов
	aiObjects.Empty();
	long objs = reader.Array();
	AIElement el;
	for(long i = 0; i < objs; i++)
	{
		FindObject(reader.String(), el.object);
		el.isLive = false;
		el.isDead = false;
		aiObjects.Add(el);
	}
	if(aiObjects <= 0)
	{
		Mission().LogicDebugError("AliveDetector \"%s\" not created -> AIObjects not found", GetObjectID().c_str());
		return false;
	}	
	delayTime = reader.Float();
	watchDieFact = reader.Bool();
	deadState = reader.Bool();
	inverseLogic = reader.Bool();
	//Инициализируем тригер события
	event.Init(reader);
	Activate(reader.Bool());
	return true;
}

//Активировать
void AliveDetector::Activate(bool isActive)
{
	DetectorObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			LogicDebug("Activate");
			SetUpdate(&AliveDetector::Work, ML_TRIGGERS);
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&AliveDetector::Work);
		}
	}else{
		DelUpdate(&AliveDetector::Work);
	}
}

//Инициализировать объект
bool AliveDetector::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool AliveDetector::EditMode_Update(MOPReader & reader)
{
	return true;
}

//============================================================================================

//Работа детектора
void _cdecl AliveDetector::Work(float dltTime, long level)
{
	long aliveCount = 0;
	for(long i = 0; i < aiObjects; i++)
	{
		AIElement & el = aiObjects[i];
		//Проверяем правильность объекта
		if(!el.object.Validate())
		{
			continue;
		}
		if(watchDieFact)
		{
			//Если жив
			if(el.isLive)
			{
				if(!el.isDead)
				{
					if(IsAlive(el.object.Ptr()))
					{
						aliveCount++;
					}else{
						el.isDead = true;
					}
				}
			}else{
				aliveCount++;
				if(IsAlive(el.object.Ptr()))
				{
					el.isLive = true;
				}
			}
		}else{
			if(IsAlive(el.object.Ptr()))
			{
				aliveCount++;
			}
		}
	}
	if(!aliveCount && time < -0.5f)
	{
		time = delayTime;
		LogicDebug("Start timer in time: %f", time);
	}
	if(time > -0.5f)
	{
		time -= dltTime;
		if(time <= 0.0f)
		{
			LogicDebug("Triggering");
			time = -1.0f;
			Activate(false);
			event.Activate(Mission(), false);
			for(long i = 0; i < aiObjects; i++)
			{
				AIElement & el = aiObjects[i];
				el.isLive = false;
				el.isDead = false;
			}
		}
	}
}

//Проверить жив или нет
bool AliveDetector::IsAlive(MissionObject * obj)
{
	bool dead;
	if(deadState)
	{
		dead = obj->IsDead();
	}else{
		dead = obj->IsDie() | obj->IsDead();
	}
	if(inverseLogic)
	{
		return dead;
	}
	return !dead;
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(AliveDetector, "Alive detector", '1.00', 0x0fffffff, "Alive detector look for some AI mission object,\nand triggering if all objects is dead.", "Logic")
	MOP_ARRAYBEG("Alive object", 0, 1000)
		MOP_STRING("Object id", "Player")
	MOP_ARRAYEND
	MOP_FLOATEXC("Delay time", 5.0f, 0.0f, 100000.0f, "Delay time in seconds before activate trigger")
	MOP_BOOLC("Triggering", true, "Triggering at die fact or look dead objects")
	MOP_BOOLC("Look dead", true, "Look dead or die objects")
	MOP_BOOLC("Inverse logic", false, "Inverse logic -> look live objects or birth fact")
	MOP_MISSIONTRIGGERG("Events", "")
	MOP_BOOL("Active", true)
MOP_ENDLIST(AliveDetector)





