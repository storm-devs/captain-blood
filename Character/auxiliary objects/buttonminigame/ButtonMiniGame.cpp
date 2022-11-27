
#include "ButtonMiniGame.h"



ButtonMiniGameParams::ButtonMiniGameParams():games(_FL_)
{
	debug = false;
	always_win = false;
}

//Инициализировать объект
bool ButtonMiniGameParams::Create(MOPReader & reader)
{
	InitParams(reader);		
	return true;
}

//Обновить параметры
bool ButtonMiniGameParams::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

void ButtonMiniGameParams::InitParams(MOPReader &reader)
{
	games.DelAll();

	int count = reader.Array();

	for (int i=0;i<count;i++)
	{
		MiniGame_Desc* game = &games[games.Add()];

		game->id = reader.String().c_str();

		const char* type = reader.Enum().c_str();
		if (type[0] == 'N')
		{
			game->type = mg_button_normal;
		}
		else
		{
			game->type = mg_button_cycled;
		}
		
		game->button = reader.String().c_str();
		
		int memory_count = reader.Array();

		for (int j=0;j<memory_count;j++)
		{
			rnd_memory_params& params = game->memory_params[game->memory_params.Add()];

			params.difficulty = reader.String().c_str();
			params.remember_button = reader.Bool();
		}		

		game->difficulty = reader.String().c_str();
		game->autowin_difficulty = reader.String().c_str();

		game->iTimes = reader.Long();
		game->fDuration = reader.Float();

		game->wait_end_mg = reader.Bool();
		game->end_node = reader.String().c_str();

		game->wintriger = reader.String();
		game->losetriger = reader.String();
		game->buttontriger = reader.String();

		for (int j=0;j<16;j++)
		{
			game->widgets[j] = reader.String();
		}
	}

	debug = reader.Bool();
	always_win = reader.Bool();
	mission_time = reader.String();
}

bool ButtonMiniGameParams::StartGame(Character* owner,const char* id)
{
	for (int i=0;i<(int)games.Size();i++)	
	{
		MiniGame_Desc& game = games[i];

		if (string::IsEqual(id,game.id))
		{						
			owner->StartButtonMG(game,always_win);
			return true;
		}		
	}

	return false;
}

MissionObject* ButtonMiniGameParams::GetMissionTime(bool force_debug)
{
	MOSafePointer mt;		
	
	if (debug || force_debug)
	{
		FindObject(mission_time,mt);		
	}

	return mt.Ptr();
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(ButtonMiniGameParams, "Button MiniGame", '1.00', 50, "Button MiniGame", "Character objects")

	MOP_ENUMBEG("MG Type")
		MOP_ENUMELEMENT("Normal")
		MOP_ENUMELEMENT("Cycled")		
	MOP_ENUMEND

	MOP_ARRAYBEGC("Games", 0, 10000, "List of ")
		
		MOP_STRING("ID", "")				
		MOP_ENUM("MG Type", "Type")	
		
		MOP_STRING("Button", "")		

		MOP_ARRAYBEGC("Remembering button", 1, 5, "Remembering button depending from difficulty")
			MOP_STRING("Difficulty", "")
			MOP_BOOL("Remember Button", true)
		MOP_ARRAYEND
		
		
		MOP_STRING("Difficulty", "")
		MOP_STRING("Auto Win Difficulty", "")
		MOP_LONGEXC("Num Pressing", 1,1,1000,"Number of button pressing that needed to win")		
		MOP_FLOATEXC("Duration", 0.2f,0.05f,10.0f,"How fast player must press button")

		MOP_BOOL("Wait End MG Event", false)
		MOP_STRINGC("Final Node", "", "If not set, current node is final")
		
		MOP_STRING("Win Triger", "")
		MOP_STRING("Lose Triger", "")
		MOP_STRING("Button Triger", "")

		MOP_STRING("MngA", "Wgt_MngA")
		MOP_STRING("MngB", "Wgt_MngB")
		MOP_STRING("MngC", "Wgt_MngC")
		MOP_STRING("MngD", "Wgt_MngD")
		MOP_STRING("MngLeft", "Wgt_MngLeft")
		MOP_STRING("MngRight", "Wgt_MngRight")
		MOP_STRING("MngUp", "Wgt_MngUp")
		MOP_STRING("MngDown", "Wgt_MngDown")
		
		MOP_STRING("MngA anim", "Wgt_MngA_anim")
		MOP_STRING("MngB anim", "Wgt_MngB_anim")
		MOP_STRING("MngC anim", "Wgt_MngC_anim")
		MOP_STRING("MngD anim", "Wgt_MngD_anim")
		MOP_STRING("MngLeft anim", "Wgt_MngLeft_anim")
		MOP_STRING("MngRight anim", "Wgt_MngRight_anim")
		MOP_STRING("MngUp anim", "Wgt_MngUp_anim")
		MOP_STRING("MngDown anim", "Wgt_MngDown_anim")		

	MOP_ARRAYEND

	MOP_BOOL("Debug Mode", false)
	MOP_BOOL("AutoWin", false)
	MOP_STRING("Mission Time", "")

MOP_ENDLIST(ButtonMiniGameParams)


