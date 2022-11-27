//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Delay	
//============================================================================================
			

#include "Delay.h"
#include "MissionReloader.h"

//============================================================================================

Delay::Delay()
{
	waitTime = 0.0f;
	deltaTime = 0.0f;
	isInstant = false;
	isProcess = false;
}

Delay::~Delay()
{

}

//============================================================================================

//Инициализировать объект
bool Delay::Create(MOPReader & reader)
{	
	initTime = waitTime = reader.Float();
	deltaTime = reader.Float();
	waitTime += Rnd(deltaTime);
	bool isActivate = reader.Bool();
	isInstant = reader.Bool();
	isProcess = false;
	event.Init(reader);
	Activate(isActivate);
	return true;
}

//Активировать
void Delay::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		if(!isInstant)
		{
			waitTime = initTime + Rnd(deltaTime);
			SetUpdate(&Delay::Work, ML_TRIGGERS);
		}else{
			waitTime = 0.0f;
			Work(0.0f, 0);
		}
	}else{
		LogicDebug("Deactivate");
		DelUpdate(&Delay::Work);
	}
}

//Инициализировать объект
bool Delay::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool Delay::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Получить размеры описывающего ящика
void Delay::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}

//Работа
void _cdecl Delay::Work(float dltTime, long level)
{
	waitTime -= dltTime;
	if(waitTime > 0.0f) return;
	waitTime = -1.0f;
	Activate(false);
	if(isProcess)
	{
		LogicDebugError("Delay already in process! Critical error!");
		return;
	}
	isProcess = true;
	LogicDebug("Triggering");
	event.Activate(Mission(), false);
	isProcess = false;
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTG(Delay, "Delay", '1.00', 0x0fffffff, "Logic")
	MOP_FLOATEXC("Wait time", 10.0f, 0.0f, 1000000.0f, "Delay time in seconds")
	MOP_FLOATEXC("Random delta time", 0.0f, 0.0f, 1000000.0f, "Random time value add to wait time")
	MOP_BOOL("Active", false)
	MOP_BOOLC("Instant trigger", false, "Skip time params and execute instant triggers")
	MOP_MISSIONTRIGGERG("Events", "")
MOP_ENDLIST(Delay)



