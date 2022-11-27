//============================================================================================
// Spirenkov Maxim, 2007
//============================================================================================
// Mission objects
//============================================================================================
// MultiDelay	
//============================================================================================

#include "MultiDelay.h"
#include "MissionReloader.h"

//============================================================================================

MultiDelay::MultiDelay() : elements(_FL_)
{
}

MultiDelay::~MultiDelay()
{

}

//============================================================================================

//Инициализировать объект
bool MultiDelay::Create(MOPReader & reader)
{	
	currentIndex = 0;
	elements.DelAll();
	elements.AddElements(reader.Array());
	for(long i = 0; i < elements; i++)
	{
		Element & el = elements[i];
		el.initTime = reader.Float();
		el.deltaTime = reader.Float();
		el.waitTime = el.initTime + Rnd(el.deltaTime);
		el.event.Init(reader);
	}
	isChain = reader.Bool();
	bool isActivate = reader.Bool();
	isPause = false;
	Activate(isActivate);
	return true;
}

//Активировать
void MultiDelay::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		for(long i = 0; i < elements; i++)
		{
			Element & el = elements[i];
			el.waitTime = el.initTime + Rnd(el.deltaTime);
		}
		currentIndex = 0;
		if(!isChain)
		{
			SetUpdate(&MultiDelay::ParalelWork, ML_TRIGGERS);
		}else{
			SetUpdate(&MultiDelay::ChainWork, ML_TRIGGERS);
		}
		isPause = false;
	}else{
		LogicDebug("Deactivate");
		DelUpdate();
	}
}

//Обработчик команд для объекта
void MultiDelay::Command(const char * id, dword numParams, const char ** params)
{
	if(!id)
	{
		return;
	}
	if(string::IsEqual(id, "pause"))
	{
		isPause = true;
		LogicDebug("Multidelay is pause");
	}else
	if(string::IsEqual(id, "resume"))
	{
		isPause = false;
		LogicDebug("Multidelay is resume");
	}
}

//Инициализировать объект
bool MultiDelay::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool MultiDelay::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Получить размеры описывающего ящика
void MultiDelay::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}

//Работа
void _cdecl MultiDelay::ParalelWork(float dltTime, long level)
{
	if(isPause)
	{
		return;
	}
	long workCount = 0;
	for(long i = 0; i < elements; i++)
	{
		Element & el = elements[i];
		if(el.waitTime < -1.0f) continue;
		workCount++;
		el.waitTime -= dltTime;
		if(el.waitTime > 0.0f) continue;
		el.waitTime = -10.0f;
		LogicDebug("Triggering %i delay", i);
		el.event.Activate(Mission(), false);
	}
	if(!workCount)
	{
		Activate(false);
	}
}

//Работа
void _cdecl MultiDelay::ChainWork(float dltTime, long level)
{
	if(isPause)
	{
		return;
	}
	if(currentIndex >= elements)
	{
		Activate(false);
		return;
	}
	Element & el = elements[currentIndex];
	el.waitTime -= dltTime;
	if(el.waitTime > 0.0f) return;
	el.waitTime = -10.0f;
	LogicDebug("Triggering %i delay", currentIndex);
	currentIndex++;
	el.event.Activate(Mission(), false);
}

//Пересоздать объект
void MultiDelay::Restart()
{
	ReCreate();
};

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MultiDelay, "Multi delay", '1.00', 0x0fffffff, "Commands: pause - pause delay work;\n           resume - continue work after pause;\nActivate: true - restart object and resume work,\n        false - stop working", "Logic")
	MOP_ARRAYBEG("Delleis", 1, 100)
		MOP_FLOATEXC("Wait time", 10.0f, 0.0f, 1000000.0f, "Delay time in seconds from activate")
		MOP_FLOATEXC("Random delta time", 0.0f, 0.0f, 1000000.0f, "Random time value add to wait time")
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
	MOP_BOOLC("Chain", false, "Chain or paralel work")
	MOP_BOOL("Active", false)	
MOP_ENDLIST(MultiDelay)



