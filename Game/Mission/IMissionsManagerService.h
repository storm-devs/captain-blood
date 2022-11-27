//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionsManager
//============================================================================================

#ifndef _IMissionsManagerService_h_
#define _IMissionsManagerService_h_

#include "..\..\common_h\mission.h"


class IMissionsManagerService : public Service
{
public:
	IMissionsManagerService();
	virtual ~IMissionsManagerService();
	
	//Запустить исполнение сервиса
	virtual void Activate() = null;
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime) = null;

#ifndef NO_TOOLS
#ifndef _XBOX
	//Создать миссию
	virtual IMission * CreateMission(const char * name) = null;
	//Удалить миссию
	virtual void DeleteMission(IMission * mis) = null;
	//Удалить мэнеджер
	virtual void DeleteManager() = null;
#endif
#endif
};

#endif
