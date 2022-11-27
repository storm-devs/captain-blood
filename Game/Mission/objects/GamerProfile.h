
#ifndef _GamerProfile_h_
#define _GamerProfile_h_


#include "..\..\..\Common_h\mission\Mission.h"


class SaveGamerProfile : public MissionObject
{
public:
	SaveGamerProfile();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	// ждать завершения отложенной записи
	void _cdecl Update(float dltTime, long level);

private:
	MissionTrigger successful;
	MissionTrigger failed;
	bool block;
};

class SaveOptions : public MissionObject
{
public:
	SaveOptions();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	// ждать завершения отложенной записи
	void _cdecl Update(float dltTime, long level);

private:
	MissionTrigger successful;
	MissionTrigger failed;
	bool block;
};

class LoadGamerProfile : public MissionObject
{
public:
	LoadGamerProfile();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	MissionTrigger successful;
	MissionTrigger failed;
	bool block;
};


#endif

