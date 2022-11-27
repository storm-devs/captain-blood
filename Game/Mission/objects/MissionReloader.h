//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionReloader
//============================================================================================

#ifndef _MissionReloader_h_
#define _MissionReloader_h_


#include "..\MissionsManager.h"


class MissionReloader : public MissionObject
{
public:
	enum Action
	{
		act_error,
		act_load,
		act_unload,
		act_unload_all,
		act_lock,
		act_unlock,
		act_start,
		act_start_excl,
		act_stop,
		act_restart,
		act_restart_all
	};

	struct ActionRecord
	{
		Action act;
		ConstString str;
		const char * ld_comment;
	};

	struct Command
	{
		Action act;
		const char * name;
		const char * ld_comment;
	};

public:
	MissionReloader();
	virtual ~MissionReloader();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Написать сообщение в случае отсутствия мэнеджера
	void _cdecl Draw(float dltTime, long level);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);

private:
	array<Command> commands;
	float hideTimer;

public:
	static ActionRecord records[];
	static const char * comment;
};


#endif



