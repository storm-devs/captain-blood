//============================================================================================
// Spirenkov Maxim, 2009
//============================================================================================
// Mission objects
//============================================================================================
// SetLanguage
//============================================================================================

#pragma once

//#include "..\..\..\Common_h\Mission.h"
#include "..\Mission.h"

class SetLanguage : public MissionObject
{
public:
	SetLanguage();
	virtual ~SetLanguage();
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);
	//Пересоздать объект
	virtual void Restart();

private:
	const char * langName;
};


class SetLanguageBlockService : public Service
{
public:
	SetLanguageBlockService();
	virtual ~SetLanguageBlockService();

	static void SetNewLanguage(const char * name);

private:
	virtual bool Init();
	virtual void EndFrame(float dltTime);

private:
	string languageName;
	static SetLanguageBlockService * ptr;
};


