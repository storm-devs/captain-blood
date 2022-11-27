//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionReloader
//============================================================================================

#include "MissionReloader.h"

MissionReloader::ActionRecord MissionReloader::records[] =
{
	{act_load, ConstString("Load mission"), "Load mission \"%s\""},
	{act_unload, ConstString("Unload mission"), "Unload mission \"%s\""},
	{act_unload_all, ConstString("Unload all missions"), "Unload all missions"},
	{act_lock, ConstString("Lock mission"), "Lock mission \"%s\""},
	{act_unlock, ConstString("Unlock mission"), "Unlock mission \"%s\""},
	{act_start, ConstString("Start mission"), "Start mission \"%s\""},
	{act_start_excl, ConstString("Exclusive start mission"), "Exclusive start mission \"%s\""},
	{act_stop, ConstString("Stop mission"), "Stop mission \"%s\" and return to previous mission"},
	{act_restart, ConstString("Restart mission"), "Restart mission \"%s\""},
	{act_restart_all, ConstString("Restart all missions"), "Restart missions"},
};


MissionReloader::MissionReloader() : commands(_FL_)
{
}

MissionReloader::~MissionReloader()
{
}

//Инициализировать объект
bool MissionReloader::Create(MOPReader & reader)
{
	commands.DelAll();
	commands.AddElements(reader.Array());
	for(long i = 0; i < commands; i++)
	{
		ConstString act = reader.Enum();
		for(long j = 0; j < ARRSIZE(records); j++)
		{
			if(records[j].str == act)
			{
				break;
			}
		}
		Assert(j < ARRSIZE(records));
		commands[i].act = records[j].act;
		commands[i].name = reader.String().c_str();
		commands[i].ld_comment = records[j].ld_comment;
	}
	hideTimer = 0.0f;
	MissionObject::Activate(false);
	return true;
}

//Активировать/деактивировать объект
void MissionReloader::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(isActive)
	{
		MissionsManager * mng = MissionsManager::Ptr();
		if(mng)
		{
			LogicDebug("Activate");
			for(long i = 0; i < commands; i++)
			{
				Command & cmd = commands[i];
				const char * name = cmd.name;
				switch(commands[i].act)
				{
				case act_load:					
					mng->LoadMission(cmd.name);
					break;
				case act_unload:
					mng->UnloadMission(cmd.name);
					break;
				case act_unload_all:
					mng->UnloadAllMissions();
					break;
				case act_start:
					mng->StartMission(cmd.name, false);
					break;
				case act_start_excl:
					mng->StartMission(cmd.name, true);
					break;
				case act_stop:
					if(!cmd.name || !cmd.name[0])
					{
						mng->StopMission(Mission().GetMissionName());
						name = Mission().GetMissionName();
					}else{
						mng->StopMission(cmd.name);
					}					
					break;
				case act_restart:
					if(!cmd.name || !cmd.name[0])
					{						
						name = Mission().GetMissionName();
					}
					mng->Restart(name);
					break;
				case act_restart_all:
					mng->Restart(null);
					break;
				default:
					{
						Assert(false);
					}
				};
				LogicDebug(cmd.ld_comment, name);
			}
		}else{
			LogicDebugError("Can't do reload operations becouse MissionsManager not loaded");
			SetUpdate(&MissionReloader::Draw, ML_GUI5);
			hideTimer = 0.0f;
		}
	}else{
		DelUpdate(&MissionReloader::Draw);
	}
}

//Написать сообщение в случае отсутствия мэнеджера
void _cdecl MissionReloader::Draw(float dltTime, long level)
{
	hideTimer += dltTime;
	if(hideTimer > 7.0f)
	{
		hideTimer = 0.0f;
		DelUpdate(&MissionReloader::Draw);
		return;
	}
	const RENDERVIEWPORT & wp = Render().GetViewport();
	IFont * font = Render().GetSystemFont();
	if(!font)
	{
		DelUpdate(&MissionReloader::Draw);
		return;
	}
	const char * str = "Mission reloader activated, but manager not found... (Editior mode?)";
	float w = font->GetLength(str);
	float h = font->GetHeight();
	Color c(1.0f, 1.0f, 1.0f, 1.0f);
	if(hideTimer > 5.0f)
	{
		c.a = 1.0f - (hideTimer - 5.0f)*(1.0f/(7.0f - 5.0f));
	}
	Render().Print((wp.Width - w)*0.5f, (wp.Height - h)*0.5f, c.GetDword(), str);	
}

//Инициализировать объект
bool MissionReloader::EditMode_Create(MOPReader & reader)
{
	return true;
}

const char * MissionReloader::comment =
"Mission reloader provide missions managment.\n"
"Actions: \n"
"Load mission: load mission into memory pool\n"
"       param: mission name\n"
"Unload mission: unload mission\n"
"       param: mission name\n"
"Unload all missions: unload all mission and clear execute stack [clear]\n"
"Lock mission: lock defined mission for unload, locked mission can't be unload\n"
"       param: mission name\n"
"Unlock mission: unlock defined mission for unload\n"
"       param: mission name\n"
"Start mission: start preloaded mission with pause previous [push started]\n"
"       param: mission name\n"
"Exclusive start mission: start preloaded mission with stop previous [push started]\n"
"       param: mission name\n"
"Stop mission: remove from stack defined mission and continue top mission [remove defined]\n"
"       param: mission name\n"
"Restart mission: restart all mission objects\n"
"       param: mission name (current mission for empty field)\n"
"Restart all missions: restart all loaded missions\n"
" \n"
"Where is [execute stack operation]\n"
" \n"
" ";

MOP_BEGINLISTCG(MissionReloader, "Mission reloader", '1.00', 0, MissionReloader::comment, "Managment")
	MOP_ENUMBEG("Action")
		MOP_ENUMELEMENT(MissionReloader::records[0].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[1].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[2].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[3].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[4].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[5].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[6].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[7].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[8].str.c_str())
		MOP_ENUMELEMENT(MissionReloader::records[9].str.c_str())
	MOP_ENUMEND
	MOP_ARRAYBEG("Commands", 1, 100)
		MOP_ENUM("Action", "Action")
		MOP_STRINGC("Mission", "", "Mission name, if needed")
	MOP_ARRAYEND
MOP_ENDLIST(MissionReloader)

