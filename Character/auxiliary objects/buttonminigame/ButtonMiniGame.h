
#ifndef _ButtonMiniGame_h_
#define _ButtonMiniGame_h_

#include "..\..\..\Common_h\Mission.h"
#include "..\..\Character\Character.h"
#include "..\..\Character\Components\ButtonMG.h"


class ButtonMiniGameParams : public MissionObject
{	
	array<MiniGame_Desc> games;	

	bool  debug;
	bool  always_win;
	ConstString mission_time;

public:

	ButtonMiniGameParams();	

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);	
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);		

	void InitParams(MOPReader &reader);	

	bool StartGame(Character* owner,const char* id);

	MO_IS_FUNCTION(ButtonMiniGameParams, MissionObject);

	MissionObject* GetMissionTime(bool force_debug = false);
};

#endif











