

#include "MissionStarter.h"


MissionStarter::MissionStarter()
{
}

MissionStarter::~MissionStarter()
{
}

//Инициализировать объект
bool MissionStarter::Create(MOPReader & reader)
{
	trigger.Init(reader);
	return true;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void MissionStarter::PostCreate()
{
	if(!trigger.IsEmpty())
	{
		LogicDebug("First start process");
		trigger.Activate(Mission(), false);
	}	
}

//Пересоздать объект
void MissionStarter::Restart()
{
	if(!trigger.IsEmpty())
	{
		LogicDebug("Set to restart process");
		SetUpdate(&MissionStarter::PostRestart, ML_SCENE_START + 1);
	}
}

//Отложенное срабатывание
void _cdecl MissionStarter::PostRestart(float dltTime, long level)
{
	DelUpdate(&MissionStarter::PostRestart);
	LogicDebug("Restart process");
	trigger.Activate(Mission(), false);
}


MOP_BEGINLISTCG(MissionStarter, "Mission starter", '1.00', 0x7ffffff, "Activate when all mission objects is created", "Logic")
	MOP_MISSIONTRIGGER("")
MOP_ENDLIST(MissionStarter)



