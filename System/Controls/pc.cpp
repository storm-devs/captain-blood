#include "pc.h"
#include "..\..\common_h\systemmsg.h"

#include "XBoxDevice.h"
#include "..\..\common_h\InputSrvCmds.h"

#include "XBoxKeyboard.h"

const unsigned int XBoxGamepadCount = 4;

CREATE_SERVICE_NAMED("ControlsService", Controls, 25)

#ifndef _XBOX

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#include <XInput.h>

#pragma comment(lib, "Xinput.lib")

#include "MouseDevice.h"
#include "KeyboardDevice.h"
#include "JoystickDevice.h"

#include <delayimp.h>
#include <cmath>

#else  // _XBOX

#include <xtl.h>

#endif // _XBOX

#include "JoystickManager.h"
#include "utils.h"

#include "..\..\common_h\FileService.h"
#include "..\..\common_h\Render.h"
#include "..\..\common_h\LocStrings.h"

#include "..\..\common_h\data_swizzle.h"

static IFileService  *storage;
static IRender		 *render;

//////////////////////////////////////////////////////////////////////////
// ControlFilter
//////////////////////////////////////////////////////////////////////////

dword mmmax = 0;

const ConstString MouseV("MouseV");
const ConstString MouseH("MouseH");
const ConstString MouseDeltaH("MouseDeltaH");
const ConstString MouseDeltaV("MouseDeltaV");
const ConstString MouseLDouble("mouseldouble");
const ConstString MouseRDouble("mouserdouble");

ControlFilter::ControlFilter(Controls *ctrl, const char *name, float scale, bool isInverted) : m_Items(_FL_)
{
	m_ctrl		 = ctrl;

	SetName(name);
	//m_name		 = name;
	//mmmax = Max(m_name.Len(), mmmax);
	m_scale		 = scale;
	m_isInverted = isInverted;

	m_deviceIndex  = INVALID_CODE;
	m_controlIndex = INVALID_CODE;

	m_prev = 0.0f;
	m_curr = 0.0f;

	m_last = -1;
}

ControlFilter::ControlFilter(Controls *ctrl) : m_Items(_FL_)
{
	m_ctrl		 = ctrl;

	m_scale		 = 1.0f;
	m_isInverted = false;

	m_deviceIndex  = INVALID_CODE;
	m_controlIndex = INVALID_CODE;

	m_prev = 0.0f;
	m_curr = 0.0f;

	m_last = -1;
}

void ControlFilter::SetName(const char * name)
{
	crt_strcpy(m_fname, sizeof(m_fname), name);
	m_isMouseVorH = string::IsEqual(m_fname, "MouseV") || string::IsEqual(m_fname, "MouseH");
	m_isMouseDeltaVorH = string::IsEqual(m_fname, "MouseDeltaV") || string::IsEqual(m_fname, "MouseDeltaH");
}

bool ControlFilter::operator ==(const ControlFilter &other) const
{
	return (string::IsEqual(m_fname, other.m_fname));
}

const ControlFilter &ControlFilter::operator =(const ControlFilter &other)
{
	m_ctrl		 = other.m_ctrl;
	SetName(other.m_fname);
	//m_name		 = other.m_name;
	//mmmax = Max(m_name.Len(), mmmax);
	m_isInverted = other.m_isInverted;
	m_scale		 = other.m_scale;

	m_deviceIndex  = other.m_deviceIndex;
	m_controlIndex = other.m_controlIndex;

	m_Items = other.m_Items;

	return *this;
}

float ControlFilter::GetValue()
{
	Assert(m_ctrl);

	bool isUpdated = frameUpdated == m_ctrl->GetCurFrame();

	if( !isUpdated )
		m_prev = m_curr;

	float value = 0.0f;

	if( m_deviceIndex  != INVALID_CODE &&
		m_controlIndex != INVALID_CODE )
	{
		value  = m_ctrl->GetDeviceValue(m_deviceIndex, m_controlIndex);
		value *= m_scale;

		if( m_isInverted )
			value *= -1.0f;

		m_last = -1;

		if (!isUpdated)
			m_curr = value;

		frameUpdated = m_ctrl->GetCurFrame();

		return value;
	}

	bool addNext = false;

	for( dword i = 0; i < m_Items.Size() ; i++ )
	{
		ControlFilter &f = m_ctrl->GetFilter(m_Items[i].filter);

		float next_value = 0.0f;

		if( string::IsEqual(m_fname, f.m_fname) && !m_isMouseVorH)
		{
			api->Trace("CONTROLS: alias '%s' is looped!!!",m_fname);
		}
		else
		{
			next_value = f.GetValue();
		}

		if( m_Items[i].isInverted )
			next_value *= -1.0f;

		next_value *= m_Items[i].scale;

		if( addNext )
		{
			value *= next_value;
		}
		else
		{			
			value  = next_value;
		}		

		addNext = m_Items[i].isSumNext;

		if( fabs(value) > 0.0222f && !addNext )
		{
			m_last = i; break;
		}
	}

	value *= m_scale;

	if( m_isInverted )
		value *= -1;

	if (!isUpdated)
		m_curr = value;

	frameUpdated = m_ctrl->GetCurFrame();

	return value;
}

float ControlFilter::GetValueLast()
{
	Assert(m_ctrl);

	float value = 0.0f;

	if( m_deviceIndex  != INVALID_CODE &&
		m_controlIndex != INVALID_CODE )
	{
		value = (frameUpdated == m_ctrl->GetCurFrame()) ? m_prev : m_curr;

		return value;
	}

	bool addNext = false;	
	
	for( dword i = 0; i < m_Items.Size() ; i++ )
	{
#ifdef _XBOX
		// оптимизация на прекеширование следующего фильтра из списка
		if (i+1 < m_Items.Size())
		{
			ControlFilter & nextFilter = m_ctrl->GetFilter(m_Items[i].filter);
			__dcbt(0, &nextFilter);
		}
#endif
		ControlFilter &f = m_ctrl->GetFilter(m_Items[i].filter);

		float next_value = 0.0f;

		if( string::IsEqual(m_fname, f.m_fname) && !m_isMouseVorH)
		{
			api->Trace("CONTROLS: alias '%s' is looped!!!", m_fname);
		}
		else
		{
			next_value = f.GetValueLast();
		}

		if( m_Items[i].isInverted )
			next_value *= -1.0f;

		next_value *= m_Items[i].scale;

		if( addNext )
		{
			value *= next_value;
		}
		else
		{			
			value  = next_value;
		}		

		addNext = m_Items[i].isSumNext;

		if( fabs(value) > 0.0222f && !addNext )
		{
			break;
		}
	}

	value *= m_scale;

	if( m_isInverted )
		value *= -1;

	return value;
}

//////////////////////////////////////////////////////////////////////////
// GameControl
//////////////////////////////////////////////////////////////////////////

GameControl::GameControl(Controls* ctrl, const string & name, bool isReverse) :
	ControlFilter(ctrl,name,1.0f,false)
{
	m_isReverse = isReverse;

	m_stateType = CST_INACTIVE;
	m_fValue	= 0.0f;

	//m_isUpdated = false;
	m_enabled	= true;

	m_locked = false;
	m_curFrame = -1;
}

GameControl::GameControl() :
	ControlFilter(NULL)
{
	m_isReverse = false;

	m_stateType = CST_INACTIVE;
	m_fValue	= 0.0f;

	//m_isUpdated = false;
	m_enabled	= true;

	m_locked = false;
	m_curFrame = -1;
}

__inline bool GameControl::IsUpdated() const
{
	return (m_curFrame == m_ctrl->GetCurFrame());
}

void GameControl::Update()
{
	//m_isUpdated = true;
	m_curFrame = m_ctrl->GetCurFrame();

	if( !m_enabled )
	{
		m_fValue	= 0.0f;
		m_stateType = CST_INACTIVE;

		return;
	}

	float oldValue = ControlFilter::GetValueLast();
	float newValue = ControlFilter::GetValue();

	const float thr = 0.6f; // порог срабатывания

	bool oldBoolState = oldValue > thr;
	bool newBoolState = newValue > thr;

	if( m_locked )
	{
		if( !newBoolState )
			m_locked = false;

		newValue = 0.0f;

		m_stateType = CST_INACTIVE;
	}
	else
	{
		if( newBoolState && oldBoolState )
			m_stateType = m_isReverse ? CST_INACTIVE	: CST_ACTIVE;

		if( !newBoolState && oldBoolState )
			m_stateType = m_isReverse ? CST_ACTIVATED	: CST_INACTIVATED;

		if( newBoolState && !oldBoolState )
			m_stateType = m_isReverse ? CST_INACTIVATED : CST_ACTIVATED;

		if( !newBoolState && !oldBoolState )
			m_stateType = m_isReverse ? CST_ACTIVE		: CST_INACTIVE;
	}

	m_fValue = newValue;
}

//////////////////////////////////////////////////////////////////////////
// Controls
//////////////////////////////////////////////////////////////////////////

const int forces_count = 32;

// получить свободную силу мз пула
Force *Controls::GetFreeForce()
{
	// профайл не джойстик - тогда ничего не даем. джойстику незачем вибрировать
	const char* pcCurProfileName = api->Storage().GetString("game.controls.single.land");
	if( !pcCurProfileName || (pcCurProfileName[0]!='j' && pcCurProfileName[0]!='J') )
		return null;

	for( int i = 0 ; i < forces ; i++ )
		if( !forces[i].busy )
			return &forces[i];

	if( forces < forces_count )
		return &forces[forces.Add()];

	return null;
}

Controls::Controls() : forces(_FL_,forces_count),
#ifndef _XBOX

m_DI(NULL),

#endif
	m_groups		(_FL_, 16),
	m_devices		(_FL_),
	m_gameControls	(_FL_, 224),
	m_tableControls	(_FL_),
	m_controlFilters(_FL_, 512),
	m_tableFilters	(_FL_),
	m_Keys			(_FL_),
	m_forceList		(_FL_, 48),
	m_forceHash		(_FL_),
	m_inst			(_FL_),
	m_tableGroups	(_FL_)
{
	m_curFrame = 0;

	m_lockMouse = false;
	m_freeMouse = false;

	m_showCursor = true;
	m_clipCursor = false;

	m_prev_act = false;
	m_preserve = false;

	m_joyMrg	= null;

	target = null;
	
	m_version = 0;

	images = null;

	m_locked = 0;

	m_debug = false;

	m_vibrate = null;

	m_forceVibrate = false;
}

Controls::~Controls()
{
	if( m_joyMrg )
	{
		// стопорим все моторы
		for( long i = 0 ; i < 4 ; i++ )
		{
			float sp[] = {0.0f,0.0f};

			m_joyMrg->SetJoystickValues(IJoystick::FFRotorSpeed,sp,2*sizeof(float),i);
		}
	}

	for( int i = 0; i < m_devices ; i++ )
	{
		delete m_devices[i];
			   m_devices[i] = null;
	}

	for( int i = 0 ; i < m_forceList ; i++ )
	{
		m_forceList[i].Release();
	}

	m_devices.DelAll();

#ifndef _XBOX

	if( m_DI )
	{
		m_DI->Release();
		m_DI = null;
	}

	ClipCursor(null);

#endif

	RELEASE(target)

	RELEASE(images)

	bool empty = true;
	bool first = true;

	api->Trace("\n=============================");
	api->Trace("\nControls service shutdown start");

	for( int i = 0 ; i < m_inst ; i++ )
	{
		InstanceInfo &info = m_inst[i];

		if( info.used )
		{
			if( first )
			{
				api->Trace("");

				first = false;
				empty = false;
			}

			api->Trace("    Unreleased instance: %s [%d]",info.file,info.line);
		}
	}

	if( !empty )
		api->Trace("");

	api->Trace("Controls service shutdown end");
	api->Trace("\n=============================\n");

	RELEASE(m_vibrate)

	m_patcher.Release();
}

#ifndef _XBOX

BOOL __stdcall DeviceEnumer(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
	Controls *ctrls = (Controls *)pvRef;

	if(((lpddi->dwDevType & 0xFF) == DI8DEVTYPE_MOUSE))
		ctrls->m_devices.Add(
			NEW MouseDevice(ctrls->m_DI,*lpddi));

	if(((lpddi->dwDevType & 0xFF) == DI8DEVTYPE_KEYBOARD))
		ctrls->m_devices.Add(
			NEW KeyboardDevice(*ctrls,ctrls->m_DI,*lpddi));

	if(((lpddi->dwDevType & 0xFF) == DI8DEVTYPE_JOYSTICK ||
		(lpddi->dwDevType & 0xFF) == DI8DEVTYPE_GAMEPAD) &&	!IsXInputDevice(&lpddi->guidProduct))
		ctrls->m_joyMrg->AddJoystick(
			NEW JoystickDevice(ctrls->m_DI,*lpddi,ctrls->m_iniParser));

	return DIENUM_CONTINUE;
}

BOOL __stdcall JoystickEnumer(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
	Controls *ctrls = (Controls *)pvRef;

	if( !IsXInputDevice(&lpddi->guidProduct))
		ctrls->m_iniParser.AddJoystickMappingTable(lpddi->tszProductName);

	return DIENUM_CONTINUE;
}

static LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep, int &error)
{
	// If this is a Delay-load problem, ExceptionInformation[0] points
	// to a DelayLoadInfo structure that has detailed error info
	PDelayLoadInfo pdli = PDelayLoadInfo(pep->ExceptionRecord->ExceptionInformation[0]);

	switch( pep->ExceptionRecord->ExceptionCode )
	{
		case VcppException(ERROR_SEVERITY_ERROR,ERROR_MOD_NOT_FOUND):
			// The DLL was not found
			error = 1;
			return EXCEPTION_EXECUTE_HANDLER;

		case VcppException(ERROR_SEVERITY_ERROR,ERROR_PROC_NOT_FOUND):
			// The DLL was found but it doesn't contain the function
			error = 2;
			return EXCEPTION_EXECUTE_HANDLER;

		default:
			// We don't recognize this exception
			error = 3;
			return EXCEPTION_CONTINUE_SEARCH;
			break;
	}
}

static bool TryInitXInput()
{
	int errorCode = 0; bool res = true;

	__try
	{
		XINPUT_STATE state; XInputGetState(0,&state);
	}
	__except( DelayLoadDllExceptionFilter(GetExceptionInformation(),errorCode))
	{
		res = false;
	}

	return res;
}

#endif // !_XBOX

bool Controls::Init()
{
	storage = (IFileService *)api->GetService("FileService");
	render	= (IRender *)	  api->GetService("DX9Render");

	for( int i = 0 ; i < m_devices ; i++ )
	{
		delete m_devices[i];
			   m_devices[i] = NULL;
	}

	m_devices.DelAll();

	if( !render )
	{
		const char *err = "ControlsService: Render service not found.";

		api->Trace(err);
		Error(1000302,err);

		return false;
	}

	// добавляем группу дефолтного маппинга (на случай если нету таблицы для какого-то джойстика)
	m_iniParser.AddJoystickMappingTable("Default");

	m_joyMrg = NEW JoystickManager();
	m_devices.Add(m_joyMrg);

#ifndef _XBOX

	// выясняем какие джойстики подключены, сохраняем их названия в парсере маппинга
	HINSTANCE hInst = (HINSTANCE)api->Storage().GetLong("system.hInstance");

	HRESULT hr = DirectInput8Create(hInst,DIRECTINPUT_VERSION,IID_IDirectInput8A,(void**)&m_DI,NULL);

	if( FAILED(hr))
	{
		const char *err = "ControlsService: Can't create DirectInput8 device.";

		api->Trace(err);
		Error(1000300,err);

		return false;
	}

	if( TryInitXInput() == false )
	{
		const char *err = "ControlsService: Can't initialize XInput library. Install the latest version of DirectX.";

		api->Trace(err);
		Error(1000301,err);

		return false;
	}

	m_DI->EnumDevices(DI8DEVCLASS_GAMECTRL,JoystickEnumer,this,DIEDFL_ALLDEVICES);

#endif
	
	// выясняем есть ли девайсы XBox 360 и если есть добавляем их в маппинг
	m_iniParser.AddJoystickMappingTable(XBOX360_DEVICE);

	// парсим таблицы маппинга джойстиков
	m_iniParser.ParseJoystickMappings();

	// добавляем все XBox360 джойстики в менеджер
	for( dword i = 0 ; i < XBoxGamepadCount ; i++ )
		m_joyMrg->AddJoystick(NEW XBoxDevice(i,m_iniParser));

#ifndef _XBOX

	// добавляем прочие джойстики и другие девайсы в сервис
	m_DI->EnumDevices(DI8DEVCLASS_ALL,DeviceEnumer,this,DIEDFL_ALLDEVICES);

	m_h = (HWND)api->Storage().GetLong("system.hwnd");

#else

	m_devices.Add(NEW XBoxKeyboard(*this));

#endif

	// вытаскиваем все девайсовые контролы в общий набор
	for( int i = 0 ; i < m_devices ; i++ )
		for( dword j = 0; j < m_devices[i]->GetControlsCount() ; j++ )
		{
			long idxFilter = CreateControlFilter( m_devices[i]->GetControlName(j), 1.0f, false );
			GetFilter(idxFilter).SetDataSource(i,j);
		}

	long mouseHfilter;
	long mouseVfilter;

	if((mouseHfilter = FindFilterByName("MouseH")) != INVALID_CODE &&
	   (mouseVfilter = FindFilterByName("MouseV")) != INVALID_CODE)
	{
		long idx = CreateGameControl("MouseH",false);
		GetControl(idx).AddItem(mouseHfilter,false,1.0f,false);

		idx = CreateGameControl("MouseV",false);
		GetControl(idx).AddItem(mouseVfilter,false,1.0f,false);
	}

	// парсим все инишники контролов
	ReadIni();

	IFileService *storage = (IFileService *)api->GetService("FileService");
	Assert(storage)

	IIniFile *ini = storage->SystemIni();

	if( ini )
	{
		////

		m_debug = (ini->GetLong("Controls","DebugInfo",0) != 0);

		////

		m_profile  = "resource\\ini\\controls\\profiles\\";
		m_profile += ini->GetString("controls","profiles","profiles.ini");
	}

	const char *prof;
	
	prof = CurrentProfile(0,true,0); if( prof[0] != '[' ) LoadProfile(0,true,0,prof,false,false);
	prof = CurrentProfile(0,true,1); if( prof[0] != '[' ) LoadProfile(0,true,1,prof,false,false);
	prof = CurrentProfile(0,true,2); if( prof[0] != '[' ) LoadProfile(0,true,2,prof,false,false);
	prof = CurrentProfile(0,true,3); if( prof[0] != '[' ) LoadProfile(0,true,3,prof,false,false);

	api->SetStartFrameLevel(this,Core_DefaultExecuteLevel + 10);
	api->SetEndFrameLevel  (this,Core_DefaultExecuteLevel + 0x1000 - 1);
	
	BuildImages();

	CreateForceData();

	m_mouseH = FindControlByName("MouseH");
	m_mouseV = FindControlByName("MouseV");

	//////////////////////

	m_vibrate = api->Storage().GetItemFloat("Options.controls.vibrate",_FL_);

	//////////////////////

	Patcher::Group *group;

	//// инверсия

	group = &m_patcher.groups[m_patcher.groups.Add(Patcher::Group("Options.controls.inv_mouse",true))];

	group->controls.Add(Patcher::Group::Control("_Mouse_V"));

	group = &m_patcher.groups[m_patcher.groups.Add(Patcher::Group("Options.controls.inv_joy_x",true))];

	group->controls.Add(Patcher::Group::Control("_Joy_H"));

	group = &m_patcher.groups[m_patcher.groups.Add(Patcher::Group("Options.controls.inv_joy_y",true))];

	group->controls.Add(Patcher::Group::Control("_Joy_V"));

	//// сенсивити

	group = &m_patcher.groups[m_patcher.groups.Add(Patcher::Group("Options.controls.sen_mouse_land"))];

	group->controls.Add(Patcher::Group::Control("_MsUp"));
	group->controls.Add(Patcher::Group::Control("_MsDown"));
	group->controls.Add(Patcher::Group::Control("_MsLeft"));
	group->controls.Add(Patcher::Group::Control("_MsRight"));

	group = &m_patcher.groups[m_patcher.groups.Add(Patcher::Group("Options.controls.sen_mouse_shooter"))];

	group->controls.Add(Patcher::Group::Control("_Mouse_H"));
	group->controls.Add(Patcher::Group::Control("_Mouse_V"));

	group = &m_patcher.groups[m_patcher.groups.Add(Patcher::Group("Options.controls.sen_joy"))];

	group->controls.Add(Patcher::Group::Control("_Joy_H"));
	group->controls.Add(Patcher::Group::Control("_Joy_V"));

	m_patcher.Init(this);

	/////////////////

	return true;
}

///////////////////////////////////////////////////////

void Controls::Patcher::Init(Controls *service)
{
	for( int i = 0 ; i < groups ; i++ )
	{
		Group &group = groups[i];

		group.p = api->Storage().GetItemFloat(group.name,_FL_);

		for( int j = 0 ; j < group.controls ; j++ )
		{
			Group::Control &control = group.controls[j];

			control.index = service->FindFilterByName(control.name);

			if( control.index >= 0 )
			{
				control.val = service->GetFilter(control.index).m_scale;
			}
		}
	}
}

void Controls::Patcher::Update(Controls *service)
{
	for( int i = 0 ; i < groups ; i++ )
	{
		const Group &group = groups[i];

		for( int j = 0 ; j < group.controls ; j++ )
		{
			const Group::Control &control = group.controls[j];

			if( control.index >= 0 )
			{
				service->GetFilter(control.index).m_scale = control.val;
			}
		}
	}

	for( int i = 0 ; i < groups ; i++ )
	{
		const Group &group = groups[i];

		if( group.p )
		{
			float k = 1.0;

			if( group.isFlag )
			{
				k = group.p->Get(0.0f) > 0.1f ? -1.0f : 1.0f;
			}
			else
			{
				k = group.p->Get(1.0f);
			}

			for( int j = 0 ; j < group.controls ; j++ )
			{
				const Group::Control &control = group.controls[j];

				if( control.index >= 0 )
				{
					service->GetFilter(control.index).m_scale *= k;
				}
			}
		}
	}
}

void Controls::Patcher::Release()
{
	for( int i = 0 ; i < groups ; i++ )
	{
		Group &group = groups[i];

		RELEASE(group.p)
	}
}

///////////////////////////////////////////////////////

void Controls::Error(long id, const char *errorEnglish)
{
	ILocStrings *locStrings = (ILocStrings *)api->GetService("LocStrings");
	ICoreStorageString *st = api->Storage().GetItemString("system.error",_FL_);

	if( !st )
		return;

	const char *errorString = locStrings->GetString(id);

	st->Set(errorString ? errorString : errorEnglish);
	st->Release();
}

void Controls::CreateForceData()
{
	m_forceHash.SetBadFind(-1);

#ifndef _XBOX
	const char *name = "Resource\\forces.pkx";
#else
	const char *name = "forces.pkx";
#endif
	IPackFile *pack = storage->LoadPack(name,_FL_);

	IFinder* finder = storage->CreateFinder(IForce_force_path,"*.ffe",
		find_all_files_no_mirrors|find_no_recursive,_FL_);

	api->Trace("Create forces:\n");

	dword count = finder->Count();

	bool all_ok = true;

	for( dword i = 0 ; i < count ; i++ )
	{
		IDataFile *file = storage->OpenDataFile(finder->FilePath(i), file_open_any,_FL_);

		if( file )
		{
			Head h;

			file->Read(h,h.Size());

			XSwizzleLong(h.ln);
			XSwizzleLong(h.rn);

			if( h.ln > 0 && h.ln < 100 &&
				h.rn > 0 && h.rn < 100 )
			{
				Data d(h);

				file->Read(d,d.Size());

				bool ok = true;

				for( int j = 0 ; j < h.ln ; j++ )
				{
					Node &node = d.lp[j];

					XSwizzleFloat(node.x);
					XSwizzleFloat(node.y);
					XSwizzleFloat(node.z);

					if( node.x < 0.0f || node.x > 100.0f ||
						node.y < 0.0f || node.y >   1.0f )
					{
						ok = false; break;
					}
				}

				if( ok )
				{
					for( int i = 0 ; i < h.rn ; i++ )
					{
						Node &node = d.rp[i];

						XSwizzleFloat(node.x);
						XSwizzleFloat(node.y);
						XSwizzleFloat(node.z);

						if( node.x < 0.0f || node.x > 100.0f ||
							node.y < 0.0f || node.y >   1.0f )
						{
							ok = false; break;
						}
					}
				}

				if( ok )
				{
					int index;

					ForceData &data = m_forceList[index = m_forceList.Add()];

					data.ls = d.lp; data.ln = h.ln;
					data.rs = d.rp; data.rn = h.rn;

					m_forceHash.Add(finder->Name(i),index);
				}
				else
				{
					d.Release();

					api->Trace("    bad file (skipped) - %s",finder->Name(i));

					all_ok = false;
				}
			}

			file->Release();
		}
	}

	if( all_ok )
		api->Trace("    well done");

	api->Trace("");

	finder->Release();

	if( pack )
		pack->Release();
}

long Controls::GetFilterNode(long index, bool &inverted)
{
	if( index < 0 || index > m_controlFilters )
		return -1; // невалидный алиас

	ControlFilter &filter = m_controlFilters[index];

	if( !filter.m_Items )
	{
		if( filter.m_controlIndex >= 0 && filter.m_deviceIndex >= 0 )
			return index;
		else
			return -1; // невалидный алиас
	}
	else
	{
		if( string::IsEqual(filter.m_fname, GetFilter(filter.m_Items[0].filter).m_fname) )
		{
			api->Trace("CONTROLS: alias '%s' is looped!!!",filter.m_fname);
			return -1;
		}

		if( filter.m_Items[0].isInverted )
			inverted = !inverted;

		return GetFilterNode(filter.m_Items[0].filter,inverted);
	}
}

struct Rec
{
	IBaseTexture *t; bool used;
	IBaseTexture *z;

	long i;

	 Rec() : t(null),z(null),used(false)
	{
		 i = -1;
	}

	~Rec()
	{
		if( t )
			t->Release();
		if( z )
			z->Release();
	}
};

void Controls::BuildImages()
{
	

	if( images )
		return;

	RELEASE(target)

	#ifdef _XBOX

	IIniFile *info = storage->OpenIniFile("resource\\ini\\controls_map.ini",_FL_);

	if( info )
	{
		images = render->CreateTextureFullQuality(_FL_,"controls.txx");

		if( images )
		{
			bool ok = true;

			api->Trace("\nLoading controls atlas:\n");

			ImagePlace p = {0};

			for( int i = 0 ; i < m_controlFilters ; i++ )
			{
				ControlFilter &f = GetFilter(i);

				f.m_place = f.m_place_inv = p;
			}

			dword cx = images->GetWidth();
			dword cy = images->GetHeight();

			if( cx > 0 && cy > 0 )
			{
				int bw = info->GetLong("Image","hsize"); // ширина блока в пикселах
				int bh = info->GetLong("Image","vsize"); // высота блока в пикселах

				if( bw > 0 && bh > 0 )
				{
					float kx = bw/(float)cx;
					float ky = bh/(float)cy;

					char name[32];

					int x; // позиция блока в строке
					int y; // номер строки
					int w; // ширина в блоках
					int h; // высота в блоках

					dword n = info->GetKeysCount("Image","place");

					for( dword i = 0 ; i < n ; i++ )
					{
						const char *s = info->GetString("Image","place","",i);

						int args = sscanf_s(s,"%s %d %d %d %d",name,sizeof(name),&y,&x,&w,&h);

						if( args == 5 )
						{
						//	api->Trace(name);

							const char *control = name; bool inv = control[0] == '-';

							if( inv )
								control++;

							long index = FindFilterByName(control);

							if( index >= 0 )
							{
								p.u = x*kx; p.w = w*kx;
								p.v = y*ky; p.h = h*ky;

							//	api->Trace("%11s %f %f %f %f",control,p.u,p.v,p.w,p.h);

								ControlFilter &f = GetFilter(index);

								if( inv )
									f.m_place_inv = p;
								else
									f.m_place	  = p;

								if( control[0] == 'g' &&
									control[1] == 'p' &&
									control[2] == '_' )
								{
									if( inv )
									{
										GetFilter(index + 26*1).m_place_inv = p;
										GetFilter(index + 26*2).m_place_inv = p;
										GetFilter(index + 26*3).m_place_inv = p;
									}
									else
									{
										GetFilter(index + 26*1).m_place = p;
										GetFilter(index + 26*2).m_place = p;
										GetFilter(index + 26*3).m_place = p;
									}
								}
							}
							else
							{
								api->Trace("    Control [%s] not found.",control);
								ok = false;
							}
						}
						else
						{
							api->Trace("    Invalid place format [%s].",s);
							ok = false;
						}
					}
				}
				else
				{
					api->Trace("    Invalid block size [%dx%d].",bw,bh);
					ok = false;
				}
			}
			else
			{
				api->Trace("    Invalid image size [%dx%d].",cx,cy);
				ok = false;
			}

		//	api->Trace(ok ? "    done.\n" : "");
			api->Trace("");

			info->Release();

			return;
		}
	}

	if( info )
		info->Release();

	#endif // _XBOX

	api->Trace("\nCreate controls atlas:\n");

	bool inScene = render->IsInsideBeginScene();

	array<Rec> map(_FL_);

	map.AddElements(m_controlFilters.Size());

	for( int i = 0 ; i < m_groups ; i++ )
	{
		ControlGroup &group = m_groups[i];

		for( int j = 0 ; j < group.inds ; j++ )
		{
			GameControl &control = GetControl(group.inds[j]);

			if( !control.m_Items )
				continue;

			bool inverted = control.m_Items[0].isInverted;
			long n = GetFilterNode(control.m_Items[0].filter,inverted);

			if( n < 0 )	// невалидный или зацикленный алиас
				continue;

			if( !map[n].used || inverted && !map[n].z || !inverted && !map[n].t )
			{
				const ConstString s(GetFilter(n).GetName());
				//const string &s = GetFilter(n).GetName();

				if( s == MouseDeltaH || s == MouseLDouble ||
					s == MouseDeltaV || s == MouseRDouble )
					continue;

				static char name[128]; const char *str = s.c_str(); const char *p = null;

				// для всех [gp*_...] используем одинаковые текстуры
				if( str[0] == 'g' &&
					str[1] == 'p' && (p = strchr(str,'_')))
				{
					if( str[2] == '_' )
					{
						sprintf_s(name,128,"Controls\\%sgp%s.txx",inverted ? "-" : "",p);

						map[n].i = -1;
					}
					else
					{
						name[0] = 0;

						map[n].i = n - 26*(str[2] - '1');
					}
				}
				else
				{
					sprintf_s(name,128,"Controls\\%s%s.txx",inverted ? "-" : "",str);
				}

				if( map[n].i < 0 )
				{
					if( inverted )
						map[n].z = render->CreateTextureFullQuality(_FL_,name);
					else
						map[n].t = render->CreateTextureFullQuality(_FL_,name);
				}

				if( map[n].z || map[n].t || map[n].i >= 0 )
					map[n].used = true;
			}
		}
	}

	const float max_r = 2048.0f;
	const float max_b = 2048.0f;

	float x = 0.0f;
	float y = 0.0f;

	float b = 0.0f;
	float r = 0.0f;

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		ControlFilter &f = GetFilter(i);

		ImagePlace &p = f.m_place;
		ImagePlace &z = f.m_place_inv;

		p.u = 0.0f;
		p.v = 0.0f;
		p.w = 0.0f;
		p.h = 0.0f;

		z.u = 0.0f;
		z.v = 0.0f;
		z.w = 0.0f;
		z.h = 0.0f;

		if( map[i].used )
		{
			if( map[i].i < 0 )
			{
				if( map[i].t )
				{
					p.w = (float)map[i].t->GetWidth ();
					p.h = (float)map[i].t->GetHeight();

					if( x + p.w > max_r )
					{
						p.u = 0.0f; y = b;
						p.v = y;
					}
					else
					{
						p.u = x;
						p.v = y;
					}

					x = p.u + p.w;

					if( b < y + p.h )
						b = y + p.h;

					if( r < x )
						r = x;

					Assert(b < max_b)
				}

				if( map[i].z )
				{
					z.w = (float)map[i].z->GetWidth ();
					z.h = (float)map[i].z->GetHeight();

					if( x + z.w > max_r )
					{
						z.u = 0.0f; y = b;
						z.v = y;
					}
					else
					{
						z.u = x;
						z.v = y;
					}

					x = z.u + z.w;

					if( b < y + z.h )
						b = y + z.h;

					if( r < x )
						r = x;

					Assert(b < max_b)
				}
			}
			else
			{
				ControlFilter &g = GetFilter(map[i].i);

				p = g.m_place;
				z = g.m_place_inv;
			}
		}
	}

	dword cx = (dword)r;
	dword cy = (dword)b;

	if( cx > 0 && cy > 0 )
	{
		target = render->CreateRenderTarget(cx,cy,_FL_,FMT_A8R8G8B8);
	}

	if( !target )
		return;

	IEditableIniFile *table = null;

	IVariable *Tex = render->GetTechniqueGlobalVariable("NativeTexture",_FL_);

	RENDERVIEWPORT wp;

	wp.X = 0; wp.Width	= cx;
	wp.Y = 0; wp.Height = cy;

	wp.MinZ = 0.0f;
	wp.MaxZ = 1.0f;

	if( inScene )
	{
		render->EndScene();
		render->PushViewport();
	}

	ShaderId simpleQuad_id;
	render->GetShaderId("SimpleQuad_", simpleQuad_id);

	render->PushRenderTarget();

	render->SetRenderTarget(RTO_DONTOCH_CONTEXT,target);
	render->SetViewport(wp);

	render->BeginScene();

	render->Clear(0,null,CLEAR_TARGET,0,0.0f,0);

	char text[256]; long index = 0;

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		if( map[i].used || map[i].i >= 0 )
		{
			ControlFilter &f = GetFilter(i);

			ImagePlace &p = f.m_place;
			ImagePlace &z = f.m_place_inv;

			if( p.h )
			{
				p.u = (p.u + 0.5f)/cx; p.w = (p.w - 1.0f)/cx;
				p.v = (p.v + 0.5f)/cy; p.h = (p.h - 1.0f)/cy;
			}

			if( map[i].t )
			{
				float l = p.u*2.0f - 1.0f; float r = l + p.w*2.0f;
				float t = 1.0f - p.v*2.0f; float b = t - p.h*2.0f;

				float buf[] = {
					r,b,1.0f,1.0f,
					l,b,0.0f,1.0f,
					r,t,1.0f,0.0f,
					l,t,0.0f,0.0f};

				if( Tex )
					Tex->SetTexture(map[i].t);

				render->DrawPrimitiveUP(simpleQuad_id, PT_TRIANGLESTRIP,2,buf,4*sizeof(float));
			}

			if( z.h )
			{
				z.u = (z.u + 0.5f)/cx; z.w = (z.w - 1.0f)/cx;
				z.v = (z.v + 0.5f)/cy; z.h = (z.h - 1.0f)/cy;
			}

			if( map[i].z )
			{
				float l = z.u*2.0f - 1.0f; float r = l + z.w*2.0f;
				float t = 1.0f - z.v*2.0f; float b = t - z.h*2.0f;

				float buf[] = {
					r,b,1.0f,1.0f,
					l,b,0.0f,1.0f,
					r,t,1.0f,0.0f,
					l,t,0.0f,0.0f};

				if( Tex )
					Tex->SetTexture(map[i].z);

				render->DrawPrimitiveUP(simpleQuad_id, PT_TRIANGLESTRIP,2,buf,4*sizeof(float));
			}

			if( table )
			{
				sprintf_s(text,sizeof(text),
					"%11s %f %f %f %f %f %f %f %f",f.GetName(),
					p.u,p.v,p.w,p.h,
					z.u,z.v,z.w,z.h);

				table->SetString("Image","place",text,index++);
			}
		}
	}

	if( table )
		table->Release();

	render->EndScene();

	render->PopRenderTarget(RTO_RESTORE_CONTEXT);

	if( inScene )
	{
		render->PopViewport();
		render->BeginScene();
	}

#ifndef _XBOX

	if( m_debug )
		render->SaveTexture2File(target->AsTexture(),"controls.dds");

#endif

	Tex = NULL;

	m_version++;
}

void Controls::ReadIni()
{
	m_groups.DelAll();

	m_iniParser.Parse(*this);
}

long Controls::FindFilterByName(const char *name)
{
	long * p = m_tableFilters.Find(name);
	if( p )
		return *p;
	return INVALID_CODE;
}

void Controls::BuildFilterChain(ControlFilter &aliasFilter, const array<Token> &tokens)
{
	for( int i = 1 ; i < tokens ; i++ )
	{
		long filter = FindFilterByName(tokens[i].name);

		if( filter != INVALID_CODE )
		{
			aliasFilter.AddItem(filter,tokens[i].isInverted,tokens[i].scale,tokens[i].addNext);
		}
		else
		{
			#ifndef _XBOX
			if( tokens[i].name != "null" )
			{
				api->Trace("Control/alias [%s] not found. Can't create alias",tokens[i].name.GetBuffer());
			}
			#endif

			aliasFilter.SetItemSumFlag(aliasFilter.GetItemsCount() - 1,tokens[i].addNext);
		}
	}
}

void Controls::AddAlias(const array<Token> &tokens)
{
	Assert(tokens.Size() >= 2);

	long n = FindFilterByName(tokens[0].name);

	// альяс с таким именем уже существует, повторно не добавляем, существующий не меняем
	// да, повторно не добавляем, но все-таки меняем
	if( n != INVALID_CODE )
	{
		ControlFilter &f = GetFilter(n);

		f.m_Items.DelAll();

		BuildFilterChain(f,tokens);

		return;
	}

	n = CreateControlFilter( tokens[0].name, 1.f, false );
	BuildFilterChain( GetFilter(n), tokens );
}

void Controls::AddControl(long group, const array<Token> &tokens)
{
	if( group >= m_groups )
		return;

	Assert(tokens.Size() >= 2);
	
	long index = FindControlByName(tokens[0].name);

	if( index != INVALID_CODE )
	{
		api->Trace("Control [%s] already exists. Not changed.",tokens[0].name.GetBuffer());

		return;
	}
	
	long ctrlIndex = CreateGameControl( tokens[0].name.c_str(), tokens[0].isReverse );
	BuildFilterChain(GetControl(ctrlIndex),tokens);
	if( group != INVALID_CODE )
		m_groups[group].inds.Add(ctrlIndex);
}

long Controls::AddGroup(const char *groupName)
{
	long index = FindGroupByName(groupName);

	if( index != INVALID_CODE )
		return index;

	index = CreateControlGroup(groupName);
	return index;
}

//////////////////////////////////////////////////////////////////////////

static bool _mustUpdate = false;

void Controls::StartFrame(float dltTime)
{
	m_curFrame++;

	long active = api->IsActive() ? 1 : 0;

	m_patcher.Update(this);

	// заливаем в "драйверы" данные с устройств
	for( int i = 0 ; i < m_devices ; i++ )
		m_devices[i]->Update(dltTime);

	if( active )
	{
		// говорим, что на текущем кадре состояния контролов непересчитаны
		//for( int i = 0; i < m_gameControls ; i++ )
		//	m_gameControls[i].ResetUpdated();

		//for( int i = 0; i < m_controlFilters ; i++ )
		//	m_controlFilters[i].m_updated = false;
	}

#ifndef _XBOX

	bool show = true;
	bool clip = false;

	if( active )
	{
		if( m_prev_act == false )
		{
			if( GetAsyncKeyState(VK_LBUTTON) < 0 )
			{
				TITLEBARINFO tbi;

				tbi.cbSize = sizeof(tbi); GetTitleBarInfo(m_h,&tbi);

				POINT p; GetCursorPos(&p);

				if( PtInRect(&tbi.rcTitleBar,p))
				{
					m_preserve = true;
				}
			}
		}
		else
		{
			if( m_preserve )
			{
				if( GetAsyncKeyState(VK_LBUTTON) >= 0 )
					m_preserve = false;
			}
		}

		if( m_preserve == false )
		{
			// лочим перемещение виндового курсора если надо
			if( m_lockMouse && !m_freeMouse )
			{
				show = false;
				clip = true;

				RECT r; GetWindowRect(m_h,&r);

				SetCursorPos(
					(r.left + r.right)/2,
					(r.top + r.bottom)/2);
			}
		}

		m_freeMouse = false;
	}
	else
	{
		m_preserve = false;
	}

	m_prev_act = (active != 0);

	if( render->GetScreenInfo3D().bWindowed )
	{
		if( m_clipCursor != clip )
		{
			if( clip )
			{
				RECT r;	GetClientRect(m_h,&r);

				r.right	 -= r.left;
				r.bottom -= r.top;

				r.left = 0;
				r.top  = 0;

				ClientToScreen(m_h,(LPPOINT)&r);

				r.right	 += r.left;
				r.bottom += r.top;

				ClipCursor(&r);
			}
			else
			{
				ClipCursor(null);
			}

			m_clipCursor = clip;
		}

		if( m_showCursor != show )
		{
			ShowCursor(m_showCursor = show);
		}
	}

#endif

	if( _mustUpdate )
	{
		BuildImages();
		_mustUpdate = false;
	}

	if( render->IsRenderReseted())
	{
		_mustUpdate = true;
		RELEASE(target)
	}
}

struct Item
{
	bool on;

	Item() : on(false)
	{
	}
};

bool Controls::HandleFilter(int i, array<Item> &map)
{
	ControlFilter &f = m_controlFilters[i];

	if( f.m_Items < 2 )
	{
		long		   n = f.m_Items[0].filter;
		ControlFilter &g = m_controlFilters[n];

		if( g.m_last < 0 )
		{
			map[n].on = false;
			return true;
		}
		else
		{
			if( HandleFilter(n,map))
			{
				map[n].on = false;
				return true;
			}
		}
	}

	return false;
}

#ifdef ENABLE_STRING_PROFILE

#include "..\..\common_h\corecmds.h"

#endif

//Исполнение в конце кадра
void Controls::EndFrame(float dltTime)
{
	for( int i = 0 ; i < m_devices ; i++ )
		m_devices[i]->EndFrame();

	ApplyForces(dltTime);

#ifdef ENABLE_STRING_PROFILE
	CoreCommand_GetMemStat stats("string.h",-1);
	api->ExecuteCoreCommand(stats);

	render->Print(0,0,0xffffffff,"%d %d",stats.totalAllocSize,stats.numBlocks);
#endif

	for( i = 0; i < m_gameControls.Len() ; i++ )
		if( !m_gameControls[i].IsUpdated() )
		{
#ifdef _XBOX
			if (i + 1 < m_gameControls.Len())
			{
				const GameControl & gc = m_gameControls[i + 1];
				if (gc.m_Items.Len())
				{
					ControlFilter & nextFilter = GetFilter(gc.m_Items[0].filter);
					__dcbt(0, &nextFilter);
				}
			}
#endif
			m_gameControls[i].Update();
		}

	if( m_debug == false )
		return;

	//// определение нажатого контрола ////
	array<Item> map(_FL_);

	map.AddElements(m_controlFilters.Size());

	for( i = 0 ; i < m_controlFilters ; i++ )
	{
		ControlFilter &f = m_controlFilters[i];

		if( !f.m_isMouseVorH && !f.m_isMouseDeltaVorH)
			//f.m_cname != MouseH && f.m_cname != MouseDeltaH &&
			//f.m_cname != MouseV && f.m_cname != MouseDeltaV )
		{
			float v = f.GetValue();

			if( v > 0.3f && f.m_fname[0] != '_' )
			{
				if( f.m_last < 0 )
				{
					map[i].on = true;
				}
				else
				{
					if( HandleFilter(i,map))
						map[i].on = true;
				}
			}
		}
	}

	float x = 515.0f;
	float y =  50.0f;

	string s;

	for( i = 0 ; i < m_controlFilters ; i++ )
	{
		if( map[i].on )
		{
			bool inverted = false;
			long node = GetFilterNode(i,inverted);
			if( node < 0 )
				continue;
			ControlFilter &f = GetFilter(node);

			if( f.GetValue() < 0.0f && !inverted || f.GetValue() >= 0.0f && inverted || f.m_curr < 0 )
				s = "-";
			else
				s = "";

			s += f.m_fname;

			render->Print(x,y,-1,s);

			x += render->GetSystemFont()->GetLength(s);
			x += 10;
		}
	}

	if( !target )
		return;

	IVariable *Tex = render->GetTechniqueGlobalVariable("NativeTexture",_FL_);

	if( Tex )
		Tex->SetTexture(target->AsTexture());

	float off = 0.0f;

	dword cx = render->GetScreenInfo3D().dwWidth;
	dword cy = render->GetScreenInfo3D().dwHeight;

	IBaseTexture *image = target->AsTexture();

	dword imageW = image->GetWidth ();
	dword imageH = image->GetHeight();

	for( int i = 0 ; i < /*256*/m_controlFilters ; i++ )
	{
		if( map[i].on )
		{
			bool inverted = false;
			int n = GetFilterNode(i,inverted);

			if( n < 0 )
				continue;

			ControlFilter &f = GetFilter(n);
			ImagePlace	  &p = inverted ? f.m_place_inv : f.m_place;

			if( p.w )
			{
				float l = p.u; float r = l + p.w;
				float t = p.v; float b = t + p.h;

				float d = imageW*p.w/cx;
				float c = imageH*p.h/cy;

				float buf[] = {
					off + d,0.0f - c,r,b,
					off - d,0.0f - c,l,b,
					off + d,0.0f + c,r,t,
					off - d,0.0f + c,l,t};

				ShaderId simpleQuad_id;
				render->GetShaderId("SimpleQuad", simpleQuad_id);
				render->DrawPrimitiveUP(simpleQuad_id, PT_TRIANGLESTRIP,2,buf,4*sizeof(float));

				off += d + d;
			}
		}
	}

	Tex = NULL;
}

void Controls::ApplyForces(float dltTime)
{
	if( !forces )
		return;

	float _ls[4] = {0.0f,0.0f,0.0f,0.0f};
	float _rs[4] = {0.0f,0.0f,0.0f,0.0f};

	long active  = api->IsActive() ? 1 : 0;

	/////////////////////////////////////////////////

	//bool vibrate = m_vibrate ? m_vibrate->Get(1.0f) > 0.1f : true;
	bool vibrate = api->Storage().GetFloat("Options.controls.vibrate") > 0.1f ? true : false;

	if( active &&( vibrate || m_forceVibrate ))
	{
		for( int i = 0 ; i < forces ; i++ )
		{
			Force &force = forces[i]; const ForceData &data = *force.data;

			float &ls = _ls[force.deviceIndex];
			float &rs = _rs[force.deviceIndex];

			if( force.stopped )
			{
				force.stopped = false;
				continue;
			}

			if( !force.busy || force.time < 0.0f )
				continue;

			if( force.instance &&
				force.instance->IsInactive())
				continue;

			if( force.time == 0.0f )
			{
				ls += coremax(0.0f,data.ls[0].y);
				rs += coremax(0.0f,data.rs[0].y);

				force.time += dltTime;
			}
			else
			{		
				force.time += dltTime;

				if( force.time > data.ls[data.ln - 1].x )
				{
					force.time = FORCE_STOP;

					if( force.autoRelease )
						force.busy = false;
				}
				else
				{
					for( int j = 0 ; j < data.ln ; j++ )
					{
						if( data.ls[j].x >= force.time )
						{
							Vector node = data.ls[j];
							Vector prev = data.ls[j - 1];

							ls += coremax(0.0f,Lerp(prev.y,node.y,(force.time - prev.x)/(node.x - prev.x)));

							break;
						}
					}

					for( int j = 0 ; j < data.rn ; j++ )
					{
						if( data.rs[j].x >= force.time )
						{
							Vector node = data.rs[j];
							Vector prev = data.rs[j - 1];

							rs += coremax(0.0f,Lerp(prev.y,node.y,(force.time - prev.x)/(node.x - prev.x)));

							break;
						}
					}
				}
			}
		}
	}

	for( int i = 0 ; i < 4 ; i++ )
	{
		float sp[] = {_ls[i],_rs[i]};

		m_joyMrg->SetJoystickValues(IJoystick::FFRotorSpeed,sp,2*sizeof(float),i);
	}
}

//////////////////////////////////////////////////////////////////////////

IBaseTexture *Controls::GetControlsImage()
{
	if( images )
		return images;

	return target ? target->AsTexture() : null;
}

int Controls::GetControlsImageVersion()
{
	return m_version;
}

ImagePlace Controls::GetControlImagePlace(const char *control_name)
{
	ImagePlace p;

	p.u = 0.0f;
	p.v = 0.0f;
	p.w = 0.0f;
	p.h = 0.0f;

	if( string::IsEmpty(control_name))
		return p;

	bool inv = control_name[0] == '-';

	if( inv )
		control_name++;

	long n = FindControlByName(control_name);

	if( n != INVALID_CODE )
	{
		GameControl &c = GetControl(n);

		if( !c.m_Items )
		{
			return p;
		}
		else
		{
			if( c.m_Items[0].isInverted )
				inv = !inv;

			bool inverted = inv;
			long node = GetFilterNode(c.m_Items[0].filter,inverted);
			if( node < 0 )
				return p;
			ControlFilter &f = GetFilter(node);

			return inverted ? f.m_place_inv : f.m_place;
		}
	}
	else
	{
		long n = FindFilterByName(control_name);

		if( n == INVALID_CODE )
		{
			return p;
		}
		else
		{
			bool inverted = inv;
			long node = GetFilterNode(n,inverted);
			if( node < 0 )
				return p;
			ControlFilter &f = GetFilter(node);

			return inverted ? f.m_place_inv : f.m_place;
		}
	}
}

// найти контрол по его имени
long Controls::FindControlByName(const char *control_name) const
{
	long* pIdx = m_tableControls.Find(control_name);
	if( pIdx )
		return *pIdx;

	return INVALID_CODE;
}

long Controls::FindGroupByName(const char *group_name)
{
	long* pIdx = m_tableGroups.Find(group_name);
	if( pIdx )
		return *pIdx;

	return INVALID_CODE;
}

// включить/выключить группу контролов
void Controls::EnableControlGroup(long group_num, bool bEnable)
{
	if((dword)group_num >= m_groups.Size())
		return;

	ControlGroup &group = m_groups[group_num]; 

	group.used = bEnable;

	for( int i = 0 ; i < group.inds ; i++ )
	{
		m_gameControls[m_groups[group_num].inds[i]].Enable(bEnable);
	}
}

void Controls::EnableControlGroup(const char *group_name, bool bEnable)
{
	EnableControlGroup(FindGroupByName(group_name),bEnable);
}

float Controls::GetDeviceValue(long deviceIndex, long controlIndex) const
{
	if( deviceIndex >= (long)m_devices.Size())
		return 0.0f;

	return m_devices[deviceIndex]->GetRawValue(controlIndex);
}

// получить состояние активности контрола
ControlStateType Controls::GetControlStateType(long control_code) //const
{
	if((dword)control_code >= m_gameControls.Size())
		return CST_INACTIVE;

//	if( m_locked )
	if( Locked())
		return CST_INACTIVE;

	return m_gameControls[control_code].GetState();
}

ControlStateType Controls::GetControlStateType(const char *control_name) //const
{
	return GetControlStateType(FindControlByName(control_name));
}

// получить значение контрола в формате целого числа
bool Controls::GetControlStateBool(long control_code) //const
{
	if( m_locked )
		return 0;

	if((dword)control_code >= m_gameControls.Size())
		return 0;
	
	return abs(m_gameControls[control_code].GetValue()) > 0.3f;
}

bool Controls::GetControlStateBool(const char *control_name) //const
{
	return GetControlStateBool(FindControlByName(control_name));
}

// получить значение контрола в формате с плавающей точкой
float Controls::GetControlStateFloat(long control_code) //const
{
	if( m_locked && control_code > 1 )
		return 0.0f;

	if((dword)control_code >= m_gameControls.Size())
		return 0.0f;

	return m_gameControls[control_code].GetValue();
}

float Controls::GetControlStateFloat(const char *control_name) //const
{
	return GetControlStateFloat(FindControlByName(control_name));
}

// разрешить/запретить обработку контролов
void Controls::Lock()
{
	m_locked++;
}

void Controls::Unlock()
{
	Assert(m_locked)

	m_locked--;
}

bool Controls::Locked()
{
	return m_locked > 0;
}

long Controls::LockCount()
{
	return m_locked;
}

// получить алиас активного контрола
const char *Controls::GetPressedControl(const char *alias_name)
{
	// параметр не используется!!!

//	Item map[256];
/*	Item map[512];

	Assert(m_controlFilters < 512)*/
	array<Item> map(_FL_);

	map.AddElements(m_controlFilters.Size());

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		ControlFilter &f = m_controlFilters[i];

		if( !f.m_isMouseVorH )//m_cname != MouseH && f.m_cname != MouseV )
		{
			float v = f.GetValue();

			if( v > 0.3f )
			{
				if( f.m_last < 0 )
				{
					map[i].on = true;
				}
				else
				{
					if( HandleFilter(i,map))
						map[i].on = true;
				}
			}
		}
	}

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		if( map[i].on )
		{
			ControlFilter &f = m_controlFilters[i];

			if( f.m_prev <= 0.0f &&
				f.m_curr >  0.0f )
			{
				if( f.m_fname[0] == '_' )
					return GetFilter(f.m_Items[0].filter).GetName();
				else
					return f.m_fname;
			}
		}
	}

	return null;
}

const char *Controls::GetActiveControl(const char *alias_name)
{
	array<Item> map(_FL_);

	map.AddElements(m_controlFilters.Size());

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		ControlFilter &f = m_controlFilters[i];

		if( !f.m_isMouseVorH )//f.m_cname != MouseH && f.m_cname != MouseV )
		{
			float v = f.GetValue();

			if( f.m_last < 0 )
			{
				if( (v) > 0.3f )
					map[i].on = true;
			}
			else
			{
				if( v > 0.3f )
					if( HandleFilter(i,map))
						map[i].on = true;
			}
		}
	}

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		if( map[i].on )
		{
			ControlFilter &f = m_controlFilters[i];
			return f.m_fname;
		}
	}

	return null;
}

const char *Controls::GetReleasedControl(const char *alias_name)
{
	array<Item> map(_FL_);

	map.AddElements(m_controlFilters.Size());

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		ControlFilter &f = m_controlFilters[i];

		if( !f.m_isMouseVorH ) //f.m_cname != MouseH && f.m_cname != MouseV )
		{
			float v = f.GetValue();

			if( f.m_last < 0 )
			{
				if( (v) > 0.3f )
					map[i].on = true;
			}
			else
			{
				if( v > 0.3f )
					if( HandleFilter(i,map))
						map[i].on = true;
			}
		}
	}

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		if( map[i].on )
		{
			ControlFilter &f = m_controlFilters[i];

			if( f.m_prev >  0.0f &&
				f.m_curr <= 0.0f )
				return f.m_fname;
		}
	}

	return null;
}

// обновить значение алиаса
const char *Controls::UpdateAlias(const char *alias_name, const char *name, bool buildImages)
{
	Assert(alias_name)

	for( int i = 0 ; i < m_controlFilters ; i++ )
	{
		ControlFilter &f = m_controlFilters[i];

		if( string::IsEqual(f.m_fname, alias_name) )
		{
			long n = FindFilterByName(name);

			if( f.m_Items && f.m_Items[0].filter == n )
			{
				return name;
			}
			else
			{
				if( n != INVALID_CODE )	// устанавливаем новое значение
				{
					f.m_Items.DelAll();

					ControlFilter::ChainItem &item = f.m_Items[f.m_Items.Add()];

					item.filter = n;

					item.isInverted = false;
					item.isSumNext	= false;

					item.scale = 1.0f;

					if( buildImages )
					{
						BuildImages();	// обновить изображения контролов
					}

					return name;
				}
				else					// возвращаем старое значение
				{
					if( name == null )
					{
						if( f.m_Items )
							return GetFilter(f.m_Items[0].filter).GetName();
						else
							return alias_name;
					}
					else
						api->Trace("CONTROLS: %s = %s: invalid alias name - [%s].",alias_name,name,name);
				}
			}
		}
	}

	return null;
}

void Controls::SetCurrentProfile(long player, bool single, long index, const char *name)
{
	static char buf[128]; const char *group; const char *type = single ? "single" : "second";

	switch( index )
	{
		case 0:
			group = "Land";
			break;
		case 1:
			group = "Sea";
			break;
		case 2:
			group = "Shooter";
			break;
		case 3:
			group = "Menu";
			break;

		default:
			api->Trace("CONTROLS: SetCurrentProfile: invalid group index [%d].",index);
			return;
	}

	crt_snprintf(buf,128,"game.controls.%s.%s",type,group);
	api->Storage().SetString(buf,name);
}

// загрузить профайл
bool Controls::LoadProfile(long player, bool single, long index, const char *name, bool reset, bool buildImages)
{
	if( !name )
		return false;

	static char buf[128]; const char *type = single ? "single" : "second";

	crt_snprintf(buf,128,"game.controls.%s.%s",type,name);

	ICoreStorageFolder *folder = api->Storage().GetItemFolder(buf,_FL_);

	reset = reset || (folder->GetLong("loaded",0) != 1);

	if( reset )
	{
		string s = "resource\\ini\\controls\\profiles\\";

		s += name;
		s += ".ini";

		IIniFile *file = storage->OpenIniFile(s,_FL_);

		if( file )
		{
			array<string> items(_FL_);

			file->GetStrings("aliases","assign",items);

			m_iniParser.ParseAliases(*this,items);

			file->Release();

			SetCurrentProfile(player,single,index,name);

			folder->Release();

			if( buildImages )
			{
				BuildImages(); // обновить изображения контролов
			}

			return true;
		}
		else
		{
			folder->Release();

			return false;
		}
	}
	else
	{
		long count = folder->GetItemsCount();

		for( long i = 1 ; i < count ; i++ )
		{
			const char *alias = folder->GetNameByIndex(i);
			const char *value = folder->GetString(alias);

			const char *tt = UpdateAlias(alias,value,false);
		}

		folder->Release();

		SetCurrentProfile(player,single,index,name);

		if( buildImages )
		{
			BuildImages(); // обновить изображения контролов
		}

		return true;
	}
}

// получить имя текущего профайла
const char *Controls::CurrentProfile(long player, bool single, long index)
{
	static char buf[128]; const char *group; const char *def = "[PROFILE_NAME]";

	switch( index )
	{
		case 0:
			group = "Land";
			break;
		case 1:
			group = "Sea";
			break;
		case 2:
			group = "Shooter";
			break;
		case 3:
			group = "Menu";
			break;

		default:
			api->Trace("CONTROLS: CurrentProfile: invalid group index [%d].",index);
			return def;
	}

	const char *type = single ? "single" : "second";

	crt_snprintf(buf,128,"game.controls.%s.%s",type,group);

	const char *curr = api->Storage().GetString(buf,def);

	if( curr[0] == '[' )
	{
		IIniFile* profiles = storage->OpenIniFile(m_profile,_FL_);

		if( profiles )
		{
			char key[16];

			crt_snprintf(key,16,single ? "active%d" : "second%d",index);

			const char *s = profiles->GetString("profiles",key);

			if( s[0] )
			{
				api->Storage().SetString(buf,s);
			}
			else
				api->Trace("CONTROLS: default profile for group [%s] not specified (%s).",group,m_profile.c_str());

			profiles->Release();
		}
		else
			api->Trace("CONTROLS: default profile for group [%s] not specified (%s).",group,m_profile.c_str());
	}

	return api->Storage().GetString(buf,def);
}
#ifdef ndef
// создать силу на основе данных из файла
IForce *Controls::CreateForce(const char *profile, bool autoDelete, long deviceIndex)
{
	Assert(profile)

	Force *force = GetFreeForce();

	if( force )
	{
		force->autoRelease = autoDelete;
		force->deviceIndex = deviceIndex;

		force->busy	 = true;
		force->time  = FORCE_STOP;

		string name = forcePath + profile;

		if( !strchr(profile,'.'))
			name += ".ffe";

	//	IFile *file = storage->OpenFile(name.c_str(),file_open_existing_for_read,_FL_);
	//	Assert(file)

		IDataFile *file = storage->OpenDataFile(name.c_str(),_FL_);

		if( !file )
		{
			force->Release();
			return NULL;
		}

		force->ls.DelAll();
		force->rs.DelAll();

		Head h;

		file->Read(h,h.Size());

		XSwizzleLong(h.ln);
		XSwizzleLong(h.rn);

		Data d(h);

		file->Read(d,d.Size());

		bool ok = true;

		for( int i = 0 ; i < h.ln ; i++ )
		{
			Node &node = d.lp[i];

			XSwizzleFloat(node.x);
			XSwizzleFloat(node.y);
			XSwizzleFloat(node.z);

			if( node.x < 0.0f || node.x > 100.0f ||
				node.y < 0.0f || node.y >   1.0f )
			{
				ok = false; break;
			}

			force->ls.Add(node);
		}

		if( ok )
		{
			for( int i = 0 ; i < h.rn ; i++ )
			{
				Node &node = d.rp[i];

				XSwizzleFloat(node.x);
				XSwizzleFloat(node.y);
				XSwizzleFloat(node.z);

				if( node.x < 0.0f || node.x > 100.0f ||
					node.y < 0.0f || node.y >   1.0f )
				{
					ok = false; break;
				}

				force->rs.Add(node);
			}
		}

		if( !ok )
		{
			force->ls.DelAll();
			force->rs.DelAll();

			force->Release();
			force = NULL;
		}

	/*	for( int i = 0 ; i < h.ln ; i++ )
			force->ls.Add(d.lp[i]);

		for( int i = 0 ; i < h.rn ; i++ )
			force->rs.Add(d.rp[i]);*/

		file->Release();
	}

	return force;
}
#endif
IForce *Controls::CreateForce(const IControls *instance, const char *profile, bool autoDelete, long deviceIndex)
{
	Assert(profile)

	Force *force = GetFreeForce();

	if( force )
	{
		force->instance = instance;

		force->autoRelease = autoDelete;
		force->deviceIndex = deviceIndex;

		force->busy	 = true;
		force->time  = FORCE_STOP;

		m_tempName = profile;

		if( !strchr(profile,'.'))
			m_tempName += ".ffe";

		int index = m_forceHash[m_tempName];

		if( index >= 0 )
		{
			force->data = &m_forceList[index];
			return force;
		}

		api->Trace("CONTROLS: force \"%s\" not found.",profile);

		force->Release();
		return null;
	}

	return force;
}
// создать силу на основе данных из массивов
IForce *Controls::CreateForce(array<Vector> &ls, array<Vector> &rs, bool autoDelete, long deviceIndex)
{
	m_forceVibrate = true;

	long ln = ls.Size();
	long rn = rs.Size();

	if( ln > 0 && rn > 0 )
	{
		Force *force = GetFreeForce();

		if( force )
		{
			force->instance = null;

			force->autoRelease = autoDelete;
			force->deviceIndex = deviceIndex;

			force->busy	 = true;
			force->time  = FORCE_STOP;

			m_forceData.ls = ls;
			m_forceData.rs = rs;

			m_fakeData.ls = m_forceData.ls.GetBuffer();
			m_fakeData.rs = m_forceData.rs.GetBuffer();

			m_fakeData.ln = ln;
			m_fakeData.rn = rn;

			force->data = &m_fakeData;
		}

		return force;
	}
	else
		return null;
}

// получить количество групп
long Controls::GetControlGroupsQuantity() const
{
	return m_groups.Size();
}

// выполнить системно-зависимую команду
void Controls::ExecuteCommand(InputSrvCommand &cmd)
{
	switch( cmd.id )
	{
		#ifndef _XBOX

		case LockMouseCursorPos:
			if( render->GetScreenInfo3D().bWindowed )
			{
				bool lock = ((InputSrvLockMouse &)cmd).isLock;

				if( m_lockMouse != lock )
				{
					if( lock )
					{
						GetCursorPos(&m_cursorPos);
					}
					else
					{
						SetCursorPos(
							m_cursorPos.x,
							m_cursorPos.y);
					}

					m_lockMouse = lock;
				}
			}
			break;

		#endif

		case QueryNeedPause:
			((InputSrvQueryNeedPause &)cmd).needPause = m_joyMrg->IsPaused();
			break;

		case QueryJoypadConnect:
			((InputSrvQueryJoypadConnect &)cmd).isConnect = m_joyMrg->IsConnected( ((InputSrvQueryJoypadConnect &)cmd).joypadIndex );
			break;

		case QueryJoypadAssignIndex:
			((InputSrvQueryAssignIndex &)cmd).assignIndex = m_joyMrg->GetAssignIndex( ((InputSrvQueryAssignIndex &)cmd).joypadIndex );
			break;
	}
}

//////////////////////////////////////////////////////////////////////////

void Controls::AddKey(const KeyDescr &desc)
{
	m_Keys.Add(desc);
}

unsigned int Controls::GetKeyBufferLength() const
{
	if( m_locked > 0 )
		return 0;

	return m_Keys.Size();
}

const KeyDescr *Controls::GetKeyBuffer() const
{
	if( m_locked > 0 )
		return null;

	if( m_Keys.Size())
		return &m_Keys[0];

	return null;
}

void Controls::ClearKeyBuffer()
{
	m_Keys.DelAll();
}

void Controls::FreeMouse()
{
	#ifndef _XBOX

	m_freeMouse = true;

	#endif
}

////

IControls *Controls::CreateInstance(const char *file, long line)
{
	return NEW ControlsInstance(this,file,line);
}

void Controls::RegInstance(ControlsInstance *p, const char *file, long line)
{
	InstanceInfo *info = null;

	for( int i = 0 ; i < m_inst ; i++ )
	{
		InstanceInfo *q = &m_inst[i];

		if( !q->used )
		{
			info = q; break;
		}
	}

	if( !info )
		 info = &m_inst[m_inst.Add()];

	info->used = true;

	info->p = p;

	info->file = file;
	info->line = line;
}

void Controls::DelInstance(ControlsInstance *p)
{
	for( int i = 0 ; i < m_inst ; i++ )
	{
		InstanceInfo *q = &m_inst[i];

		if( q->p == p )
		{
			q->used = false; break;
		}
	}
}

long Controls::CreateGameControl(const char* name, bool isReverse)
{
	long idx = m_gameControls.Add( GameControl(this,name,isReverse) );
	m_tableControls.AddObj( name, idx );
	return idx;
}

long Controls::CreateControlFilter(const char* name, float scale, bool isReverse)
{
	long idx = m_controlFilters.Add( ControlFilter(this,name,scale,isReverse) );
	m_tableFilters.AddObj( name, idx );
	return idx;
}

long Controls::CreateControlGroup(const char* name)
{
	long idx = m_groups.Add( ControlGroup() );
	m_groups[idx].name = name;
	m_groups[idx].used = true;

	m_tableGroups.AddObj( name, idx );

	return idx;
}
