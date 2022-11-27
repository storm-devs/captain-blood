#include "ControlsManager.h"
/*
static char _key[] = "Profile.Controls.active1";
static long _key_index = 23;
*/
static const char *not_binded = "[[]";

ControlsManager:: ControlsManager() :
	items	(_FL_),
	profiles(_FL_)
{
}

ControlsManager::~ControlsManager()
{
}

bool ControlsManager::Create		 (MOPReader &reader)
{
	InitParams(reader);

	Show(true);

	return true;
}

bool ControlsManager::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void ControlsManager::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&ControlsManager::Draw,ML_ALPHA5);
	else
		DelUpdate(&ControlsManager::Draw);
}

void ControlsManager::Update()
{
	if( !profile )
	{
		LogicDebugError("Can't update profile: no active profile.");
		return;
	}

	Profile &prof = profiles[curProfile];

	if( prof.stat )
	{
		LogicDebugError("Can't update profile: profile %s is static.",profile);
		return;
	}

/*	string name = "resource\\ini\\controls\\profiles\\";

	name += profile;
	name += ".ini";

	#ifndef _XBOX

	char path[MAX_PATH];

	GetCurrentDirectory(MAX_PATH,path);

	crt_strcat(path,MAX_PATH,"\\");
	crt_strcat(path,MAX_PATH,name);

//	api->Trace(path);

	SetFileAttributes(path,FILE_ATTRIBUTE_NORMAL);

	#endif

	IEditableIniFile *file = Files().OpenEditableIniFile(name,file_create_always,_FL_);

	const char *section = "aliases";

	if( file )
		file->AddSection(section);

	int index = 0;

	const array<long> &ctls = prof.ctls;

//	for( int i = 0 ; i < items ; i++ )
	for( int i = 0 ; i < ctls  ; i++ )
	{
		long k = ctls[i];

		if( k >= items )
		{
			LogicDebugError("Invalid control index %d in profile %s.",k,profile);
			continue;
		}

	//	Item &item = items[i];
		Item &item = items[k];

		if( Controls().UpdateAlias(item.alias,item.name))
		{
			static char buf[100];

			if( file )
			{
				if( item.name && item.name[0] == '[' )
					sprintf_s(buf,100,"%s, %s",item.alias,item.name + 1);
				else
					sprintf_s(buf,100,"%s, %s",item.alias,item.name);

				file->SetString(section,"assign",buf,index++);
			}
		}
	}

	if( file )
		file->Release();*/

	static char buf[128];

	crt_snprintf(buf,128,"Game.Controls.single.%s",profile);

	ICoreStorageFolder *folder = api->Storage().GetItemFolder(buf,_FL_);

	const array<long> &ctls = prof.ctls;

//	for( int i = 0 ; i < items ; i++ )
	for( int i = 0 ; i < ctls  ; i++ )
	{
		long k = ctls[i];

		if( k >= items )
		{
			LogicDebugError("Invalid control index %d in profile %s.",k,profile);
			continue;
		}

	//	Item &item = items[i];
		Item &item = items[k];

		if( Controls().UpdateAlias(item.alias.c_str(),item.name))
		{
			if( item.name && item.name[0] == '[' )
				folder->SetString(item.alias.c_str(),item.name + 1);
			else
				folder->SetString(item.alias.c_str(),item.name);
		}
	}

	folder->Release();
}

void ControlsManager::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	//
}

void _cdecl ControlsManager::Draw(float dltTime, long level)
{
	if( !updated )
	{
		SetProfile(profile,number); updated = true;
	}

	if( curItem < 0 )
	{
		if( Controls().Locked() && m_locked && !Controls().GetActiveControl(null))
			Controls().Unlock();
	}
	else
	{
		if( IsActive() && !EditMode_IsOn())
		{
			Item &item = items[curItem];

		//	if( Controls().GetReleasedControl(null))
		//		Controls().Lock();

			const char *name = Controls().GetPressedControl(item.alias.c_str());

			if( m_locked && Controls().LockCount() < 2 && name &&
				!string::IsEqual(name,"MouseDeltaH") &&
				!string::IsEqual(name,"MouseDeltaV") &&
				!string::IsEqual(name,"MouseLDouble"))
			{
				Assert(profile)

				Profile &prof = profiles[curProfile];

			//	long index = -1;

				for( int i = 0 ; i < prof.ctls ; i++ )
				{
					long n = prof.ctls[i];

					if( n >= 0 && n < items )
					{
						if( n != curItem )
						{
							const char *alias = Controls().UpdateAlias(items[n].alias.c_str(),null);

							if( alias && string::IsEqual(alias,name))
							{
							//	index = n;

								Item &prev = items[n];

								const char *s = not_binded;

								Controls().UpdateAlias(prev.alias.c_str(),s + 1);

								MOSafePointer p;

								if( FindObject(ConstString(prev.value),p))
								{
									p.Ptr()->Command("SetString",1,&s);
								}
								else
									LogicDebugError("Object %s not found.",prev.value.c_str());

								prev.name = s;
							}
						}
					}
					else
					{
						LogicDebugError(
							"Profile %s: control[%d] has invalid value (%d).",profile,i,n);
					}
				}

			/*	if( index >= 0 )
				{
				//	пытаемся прибиндить уже используемый контрол

				//	;

			//		return;

					Item &prev = items[index];

					const char *s = not_binded;

					Controls().UpdateAlias(prev.alias,s + 1);

					MissionObject *p = (MissionObject *)FindObject(prev.value);

					if( p )
					{
						p->Command("SetString",1,&s);
					}
					else
						LogicDebugError("Object %s not found.",prev.value.c_str());

					prev.name = s;
				}*/
			//	else
				{
					newTrigger.Activate(Mission(),false);

					LogicDebug("Set alias: %s = %s.",item.alias.c_str(),name);

					const char *tt = Controls().UpdateAlias(item.alias.c_str(),name);
				//	api->Trace("    GET: %s = %s, %s",item.alias,name,tt);

					MOSafePointer p;

					if( FindObject(ConstString(item.value),p))
					{
						static char buf[100];

					//	sprintf_s(buf,100,"[%s] %s",name,name);
						sprintf_s(buf,100,"[%s]",name);

						const char *s = buf;

						p.Ptr()->Command("SetString",1,&s);
					}
					else
						LogicDebugError("Object %s not found.",item.value.c_str());

					item.name = name;

					curItem = -1;
				}
			}
			else
			{
				MOSafePointer p;

				if( FindObject(ConstString(item.value),p))
				{
					curTime += dltTime;

					if( curTime > 1.0f )
						curTime = curTime - 1.0f;

					const char *s = curTime < 0.5f ? "" : " _";

					p.Ptr()->Command("SetString",1,&s);
				}
			}
		}
	}
}

void ControlsManager::SetProfile(const char *name, int index, bool reset)
{
	if( name )
	{
		const Profile &prof = profiles[curProfile];

		if( Controls().LoadProfile(prof.player,prof.single,index,name,reset))
			profile = name;
		else
			profile = null;
	}
	else
		profile = null;

/*	_key[_key_index] = '0' + index;

	if( profile )
	{
		GameState().SetString(_key,profile ? profile : "");

		api->Trace("SETSTATE: %s = %s",_key,profile ? profile : "[none]");
	}*/

//	api->Trace("\n    MANAGER: set profile %s.\n",name);

	for( int i = 0 ; i < items ; i++ )
	{
		Item &item = items[i];

		MOSafePointer p;

		if( FindObject(ConstString(item.value),p))
		{
			const char *name = profile ? Controls().UpdateAlias(item.alias.c_str(),null) : null;

			if( name && name[0] == '_' )
			{
				name = not_binded;

				p.Ptr()->Command("SetString",1,&name);
			}
			else
			{
				static char buf[100];

			//	sprintf_s(buf,100,name ? "[%s] %s" : " _ERROR_",name,name);
				sprintf_s(buf,100,name ? "[%s]" : " _ERROR_",name);

				const char *s = buf;

				p.Ptr()->Command("SetString",1,&s);

			//	api->Trace("    %s = %s",item.value,name);
			}

			item.name = name;
		}
	}

	MOSafePointer p;

	if( FindObject(ConstString(title),p))
	{
		if( name )
		{
			const char *s = profiles[curProfile].title;

			p.Ptr()->Command("SetString",1,&s);
		}
	}
}

void ControlsManager::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"select"))
	{
	//	long index = numParams ? atol(params[0]) : 0;

		if( numParams < 3 )
		{
			LogicDebugError("Command <select> error. Not enought parameters.");
			return;
		}

		long player = atol(params[0]);
		bool single = atol(params[1]) == 0;
		long index	= atol(params[2]);

	/*	_key[_key_index] = '0' + (char)index;

		const char *name = GameState().GetString(_key,"");

		api->Trace("GETSTATE: %s = %s",_key,name[0] ? name : "[none]");*/

		const char *curr = Controls().CurrentProfile(player,single,index);

		for( int i = 0 ; i < profiles ; i++ )
		{
			Profile &prof = profiles[i];

		//	if( Controls().CurrentProfile() == prof.name )
		//	if( string::IsEqual(name,prof.name))
			if( prof.player == player &&
				prof.single == single &&
				prof.index  == index  && string::IsEqual(curr,prof.name))
			{
				curProfile = i;
				prof.onSelect.Activate(Mission(),false);

			//	profile = profiles[curProfile].name;
			//	number	= profiles[curProfile].index;

			//	api->Trace("PROFILE: %s",profiles[curProfile].name);

				return;
			}
		}

		defTrigger.Activate(Mission(),false);
	}
	else
	if( string::IsEqual(id,"set"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <set> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <set> error. Index not specified.");
		}

		long index = atol(params[0]);

		if( index < profiles && profiles[index].name[0] )
		{
			curProfile = index;

			profile = profiles[curProfile].name;
			number	= profiles[curProfile].index;

			SetProfile(profile,number);

			LogicDebug(
				"Command <set>. Setting profile %s as active.",profile);
		}
		else
		{
			LogicDebugError(
				"Command <set> error. Index value %i is not in range [0, %i].",
				index,profiles.Size() - 1);
			return;
		}
	}
	else
	if( string::IsEqual(id,"set_default"))
	{
		if( numParams < 3 )
		{
			LogicDebugError("Command <set_default> error. Not enought parameters.");
			return;
		}

		long player = atol(params[0]);
		bool single = atol(params[1]) == 0;
		long index	= atol(params[2]);

		const char *name = Controls().CurrentProfile(player,single,index);

		if( Controls().LoadProfile(player,single,index,name))
		{
			LogicDebug(
				"Command <set_default>. Loading profile %s to group [%d].",name,index);
		}
		else
		{
			LogicDebugError(
				"Command <set_default> error. No default profile in group [%d].",index);
		}
	}
	else
	if( string::IsEqual(id,"dec"))
	{
		if( curProfile >= 0 )
		{
			if( curProfile )
			{
				curProfile--;

				profile = profiles[curProfile].name;
				number	= profiles[curProfile].index;

				SetProfile(profile,number);

				LogicDebug(
					"Command <dec>. Setting profile %s as active.",profile);
			}
		}
	}
	else
	if( string::IsEqual(id,"inc"))
	{
		if( curProfile >= 0 )
		{
			if( curProfile < profiles - 1 )
			{
				curProfile++;

				profile = profiles[curProfile].name;
				number	= profiles[curProfile].index;

				SetProfile(profile,number);

				LogicDebug(
					"Command <inc>. Setting profile %s as active.",profile);
			}
		}
	}
	else
	if( string::IsEqual(id,"get"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <get> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <get> error. Index not specified.");
		}

		long index = atol(params[0]);

		if( index < items )
		{
			if( curItem < 0 )
			{
				if( profile && !profiles[curProfile].stat )
				{
					Controls().Lock(); m_locked = true;

					curItem = index;

					LogicDebug(
						"Command <get>. Setting %s to %s.",
						items[index].alias.c_str(),"###");
				}
			}
			else
				LogicDebugError(
					"Command <get> error. Is alrady in waiting state.");
		}
		else
		{
			LogicDebugError(
				"Command <get> error. Index value %i is not in range [0, %i].",
				index,items.Size() - 1);
			return;
		}
	}
	else
	if( string::IsEqual(id,"update"))
	{
		if( profile )
		{
			Update();

			LogicDebug("Command <update>. Profile %s update complete.",profile);
		}
		else
			LogicDebugError("Command <update> error. No active profile.");
	}
	else
	if( string::IsEqual(id,"cancel"))
	{
		if( curProfile >= 0 && curProfile < profiles && profiles[curProfile].name[0] )
		{
			profile = profiles[curProfile].name;
			number	= profiles[curProfile].index;

			SetProfile(profile,number);

			LogicDebug(
				"Command <cancel>. Profile %s is restored to previous settings.",profile);
		}
		else
			LogicDebugError("Command <cancel> error. No active profile.");
	}
	else
	if( string::IsEqual(id,"default"))
	{
		if( curProfile >= 0 && curProfile < profiles && profiles[curProfile].name[0] )
		{
			profile = profiles[curProfile].name;
			number	= profiles[curProfile].index;

			SetProfile(profile,number,true);

			LogicDebug(
				"Command <default>. Profile %s is reseted to default settings.",profile);
		}
		else
			LogicDebugError("Command <default> error. No active profile.");
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

void __declspec(dllexport) ControlsManager::InitParams(MOPReader &reader)
{
	items.DelAll();

	int n = reader.Array();

	for( int i = 0 ; i < n ; i++ )
	{
		Item &item = items[items.Add()];

		item.alias = reader.String();
		item.value = reader.String();
	}

	profiles.DelAll();

	n = reader.Array();

	for( int i = 0 ; i < n ; i++ )
	{
		Profile &prof = profiles[profiles.Add()];

		prof.player = reader.Long();
		prof.single = reader.Bool();

		prof.index = reader.Long();

		prof.name  = reader.String().c_str();
		prof.title = reader.String().c_str();

		prof.stat = reader.Bool();

		int k = reader.Array();

		for( int j = 0 ; j < k ; j++ )
		{
			prof.ctls.Add(reader.Long());
		}

		prof.onSelect.Init(reader);
	}

	defTrigger.Init(reader);
	newTrigger.Init(reader);

	title = reader.String().c_str();

	curProfile = reader.Long();

	if( curProfile >= 0 && curProfile < profiles && profiles[curProfile].name[0] )
	{
		profile = profiles[curProfile].name;
		number	= profiles[curProfile].index;
	}
	else
	{
		profile = null;
		number	= 0;
	}

	Activate(reader.Bool());

	curItem = -1;
	curTime = 0.0f;

	updated = false;

	m_locked = false;

//	SetProfile(profile);
}

static char desc[] =

"ControlsManager\n\n"
"    Use to setup character controller\n\n"
"Commands:\n\n"
"    select - select default profile\n\n"
"        param[0] - player index\n"
"        param[1] - single/second (0/1)\n"
"        param[2] - group index\n\n"
"    set - set current profile\n\n"
"        param[0] - profile index\n\n"
"    set_default - select default profile (inner init only)\n\n"
"        param[0] - player index\n"
"        param[1] - single/second (0/1)\n"
"        param[2] - group index\n\n"
"    get - get control value\n\n"
"        param[0] - control index\n\n"
"    update - save changes to current profile\n\n"
"    cancel - cancel last changes\n\n"
"    defaut - load default settings";

MOP_BEGINLISTCG(ControlsManager, "ControlsManager", '1.00', 2000, desc, "Interface")

	MOP_ARRAYBEG("Controls", 0, 100)

		MOP_STRING("Alias", "")
		MOP_STRING("Value", "")

	MOP_ARRAYEND

	MOP_ARRAYBEG("Profiles", 0, 100)

		MOP_LONGEX("Player", 0, 0, 10)
		MOP_BOOL  ("Single", true)

		MOP_LONGEX("Index", 0, 0, 100)

		MOP_STRING("Name" , "")
		MOP_STRING("Title", "")

		MOP_BOOL("Static", true)

		MOP_ARRAYBEG("Controls", 0, 100)

			MOP_LONGEX("Index", 0, 0, 100)

		MOP_ARRAYEND

		MOP_MISSIONTRIGGER("OnSelect")

	MOP_ARRAYEND

	MOP_MISSIONTRIGGER("Default")
	MOP_MISSIONTRIGGER("Defined")

	MOP_STRING("Title widget", "")

	MOP_LONG("Start profile", 0)

	MOP_BOOL("Active", false)

MOP_ENDLIST(ControlsManager)
