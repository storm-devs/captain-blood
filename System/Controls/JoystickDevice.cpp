#ifndef _XBOX

#include "JoystickDevice.h"
#include "..\..\common_h\Render.h"
#include "..\..\common_h\controls.h"

const long AxisMaxValue = 10000;

JoystickDevice::JoystickDevice(IDirectInput8A* di, const DIDEVICEINSTANCEA& deviceInst, const ControlsIniParser& parser) :
m_Device(NULL),
m_Controls(__FILE__, __LINE__)
{
	Assert(di);
	di->CreateDevice(deviceInst.guidInstance, &m_Device, NULL);
	Assert(m_Device);

	HWND hWnd = (HWND)api->Storage().GetLong("system.hWnd");
//	m_Device->SetCooperativeLevel(hWnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	m_Device->SetCooperativeLevel(hWnd,DISCL_FOREGROUND|DISCL_EXCLUSIVE);

	DIDEVCAPS deviceCaps = {0};
	deviceCaps.dwSize = sizeof(deviceCaps);
	m_Device->GetCapabilities(&deviceCaps);
	// ставим предельное число, т.к. трактуем оси иначе чем DirectInput
	// (24 обычных оси + 8 осей для слайдеров)
	m_NumAxes = 24+8;
	m_NumButtons = deviceCaps.dwButtons;
	m_NumPOVs = deviceCaps.dwPOVs;

	// Установить мин/макс значения для осей
	DIPROPRANGE diprg;
	diprg.diph.dwSize       = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow        = DIPH_BYID;
	for (unsigned int i = 0; i < m_NumAxes; ++i)
	{
		diprg.diph.dwObj	= DIDFT_ABSAXIS | (i << 8);

		diprg.lMin          = -AxisMaxValue;
		diprg.lMax          = AxisMaxValue;

		m_Device->SetProperty( DIPROP_RANGE, &diprg.diph );
	}

	HRESULT hr = m_Device->SetDataFormat(&c_dfDIJoystick2);
	hr=m_Device->Acquire();

	if (parser.GetMappingTable(deviceInst.tszProductName).items.Size() > 0)
		InitControls(parser.GetMappingTable(deviceInst.tszProductName));
	else
		InitControls(parser.GetMappingTable("Default"));

	DIPROPDWORD dip;

	dip.diph.dwSize		  = sizeof(DIPROPDWORD);
	dip.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dip.diph.dwObj		  = 0;
	dip.diph.dwHow		  = DIPH_DEVICE;
	dip.dwData			  = FALSE;

	m_Device->SetProperty(DIPROP_AUTOCENTER,&dip.diph);

	lef = NULL;
	ref = NULL;

	DWORD       rgdwAxes	[2]	= {DIJOFS_X,DIJOFS_Y};
	LONG        rglDirection[2]	= {0,0};
	DIRAMPFORCE cf				= {0,0};

	DIEFFECT eff; ZeroMemory(&eff,sizeof(eff));

	eff.dwSize                  = sizeof(DIEFFECT);
	eff.dwFlags                 = DIEFF_CARTESIAN|DIEFF_OBJECTOFFSETS;
	eff.dwDuration              = INFINITE;
	eff.dwSamplePeriod          = 0;
	eff.dwGain                  = DI_FFNOMINALMAX;
	eff.dwTriggerButton         = DIEB_NOTRIGGER;
	eff.dwTriggerRepeatInterval = 0;
//	eff.cAxes                   = axesN;
	eff.cAxes                   = 2;
	eff.rgdwAxes                = rgdwAxes;
	eff.rglDirection            = rglDirection;
	eff.lpEnvelope              = 0;
	eff.cbTypeSpecificParams    = sizeof(DIRAMPFORCE);
	eff.lpvTypeSpecificParams   = &cf;
	eff.dwStartDelay            = 0;
	eff.dwDuration				= INFINITE;

	m_Device->CreateEffect(GUID_RampForce,&eff,&lef,NULL);
	m_Device->CreateEffect(GUID_RampForce,&eff,&ref,NULL);

	if( lef ) lef->Start(1,0);
	if( ref ) ref->Start(1,0);
}

JoystickDevice::~JoystickDevice()
{
	if (m_Device)
	{
		m_Device->Unacquire();
		m_Device->Release();
		m_Device = NULL;
	}

//	if( lef ) lef->Release();
//	if( ref ) ref->Release();
}

unsigned int JoystickDevice::GetAxisDataOffset(unsigned int index)
{
	DIJOYSTATE2 state;
	
	unsigned int Offsets[] =
	{	
		(char*)&(state.lX) - (char*)&state,
		(char*)&(state.lY) - (char*)&state,
		(char*)&(state.lZ) - (char*)&state,
		(char*)&(state.lRx) - (char*)&state,
		(char*)&(state.lRy) - (char*)&state,
		(char*)&(state.lRz) - (char*)&state,

		(char*)&(state.rglSlider[0]) - (char*)&state,
		(char*)&(state.rglSlider[1]) - (char*)&state,
				
		(char*)&(state.lVX) - (char*)&state,
		(char*)&(state.lVY) - (char*)&state,
		(char*)&(state.lVZ) - (char*)&state,
		(char*)&(state.lVRx) - (char*)&state,
		(char*)&(state.lVRy) - (char*)&state,
		(char*)&(state.lVRz) - (char*)&state,

		(char*)&(state.rglVSlider[0]) - (char*)&state,
		(char*)&(state.rglVSlider[1]) - (char*)&state,
	
		(char*)&(state.lAX) - (char*)&state,
		(char*)&(state.lAY) - (char*)&state,
		(char*)&(state.lAZ) - (char*)&state,
		(char*)&(state.lARx) - (char*)&state,
		(char*)&(state.lARy) - (char*)&state,
		(char*)&(state.lARz) - (char*)&state,

		(char*)&(state.rglASlider[0]) - (char*)&state,
		(char*)&(state.rglASlider[1]) - (char*)&state,

		(char*)&(state.lFX) - (char*)&state,
		(char*)&(state.lFY) - (char*)&state,
		(char*)&(state.lFZ) - (char*)&state,
		(char*)&(state.lFRx) - (char*)&state,
		(char*)&(state.lFRy) - (char*)&state,
		(char*)&(state.lFRz) - (char*)&state,

		(char*)&(state.rglFSlider[0]) - (char*)&state,
		(char*)&(state.rglFSlider[1]) - (char*)&state,
	};

	return Offsets[index];
}

float JoystickDevice::GetButtonData(unsigned int dataOffset)
{
	unsigned char value = *(unsigned char*)(((char*)&m_Data) + dataOffset);

	return (float)((value & 0x80) >> 7);
}

float JoystickDevice::GetAxisData(JoystickDevice::Control::Type axisType, unsigned int dataOffset)
{
	long value = *(long*)(((char*)&m_Data) + dataOffset);

	if (abs(value) < AxisMaxValue/10)
		return 0.0f;
	return value / (float)AxisMaxValue;
}

float JoystickDevice::GetPOVData(unsigned int dataOffset, bool isPovH)
{
	long value = *(long*)(((char*)&m_Data) + dataOffset);

	if ( (LOWORD(value) == 0xFFFF) ) return 0.0f;

	if (isPovH)
	{
		float ret[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f };
		return ret[value / 4500];
	}
	else
	{
		float ret[] = { 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, 0.0f, 1.0f };
		return ret[value / 4500];
	}
}

static const char* GetMappedName(const char* hwName, const ControlsIniParser::Table& table, bool& inverted)
{
	string s1, s2;
	for (unsigned int i = 0; i < table.items.Size(); ++i)
	{
		s1 = table.items[i].hwName;
		s2 = hwName;
		s1.Upper();
		s2.Upper();
		if (s1.FindSubStr(s2) != -1)
		{
			inverted = s1[0] == '-';
			return table.items[i].logName.c_str();
		}
	}

	inverted = false;
	return hwName;
}

void JoystickDevice::InitControls( const ControlsIniParser::Table& table )
{
	Control ctrl;
	ctrl.value = 0.0f;

	ctrl.type = Control::Button;
	DIJOYSTATE2 state;
	for (unsigned int i = 0; i < m_NumButtons; ++i)
	{
		ctrl.name.Format("hw_b%d", i+1);
		ctrl.name = GetMappedName(ctrl.name, table, ctrl.inverted);
		ctrl.dataOffset = (char*)&(state.rgbButtons[i]) - (char*)&state;
		m_Controls.Add(ctrl);
	}

	for (unsigned int i = 0; i < m_NumAxes; ++i)
	{
		ctrl.name.Format("hw_Axis%d", i+1);
		ctrl.name = GetMappedName(ctrl.name, table, ctrl.inverted);
		ctrl.dataOffset = GetAxisDataOffset(i);
		ctrl.type = Control::Axis;
		m_Controls.Add(ctrl);
	}

	for (unsigned int i = 0; i < m_NumPOVs; ++i)
	{
		ctrl.type = Control::POV_H;
		ctrl.name.Format("hw_PovH%d", i+1);
		ctrl.name = GetMappedName(ctrl.name, table, ctrl.inverted);
		ctrl.dataOffset = (char*)&(state.rgdwPOV[i]) - (char*)&state;
		m_Controls.Add(ctrl);

		ctrl.type = Control::POV_V;
		ctrl.name.Format("hw_PovV%d", i+1);
		ctrl.name = GetMappedName(ctrl.name, table, ctrl.inverted);
		ctrl.dataOffset = (char*)&(state.rgdwPOV[i]) - (char*)&state;
		m_Controls.Add(ctrl);
	}

	///////////////////////////////

	indexStart = GetIndex("gp_B7");
}

void JoystickDevice::Update(float DeltaTime)
{
	if (!m_Device) return;

	HRESULT hr = m_Device->Poll();

	hr=m_Device->GetDeviceState( sizeof(m_Data), &m_Data);
	if (SUCCEEDED(hr))
	{
		for (unsigned int i = 0; i < m_Controls.Size(); ++i)
		{
			if (m_Controls[i].type == Control::Button)
			{
				m_Controls[i].value = GetButtonData(m_Controls[i].dataOffset);
			}
			else
			if (m_Controls[i].type == Control::Axis)
			{
				m_Controls[i].value = GetAxisData(m_Controls[i].type, m_Controls[i].dataOffset);
			}
			else
			if (m_Controls[i].type == Control::POV_H)
			{
				m_Controls[i].value = GetPOVData(m_Controls[i].dataOffset, true);
			}
			else
			if (m_Controls[i].type == Control::POV_V)
			{
				m_Controls[i].value = GetPOVData(m_Controls[i].dataOffset, false);
			}
		}
	}
	else
		for (unsigned int i = 0; i < m_Controls.Size(); ++i)
			m_Controls[i].value = 0.0f;

	if (FAILED(hr))
		m_Device->Acquire();
}

void JoystickDevice::EndFrame()
{
//	if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
	if( api->DebugKeyState(VK_NUMPAD0))
	{
		IRender* render = (IRender*)api->GetService("DX9Render");
		for (unsigned int i = 0; i < m_Controls.Size(); ++i)
			render->GetSystemFont()->Print(0.0f, i*15.0f, "%s : %0.2f", m_Controls[i].name.GetBuffer(), m_Controls[i].value);
	}
}

long JoystickDevice::GetIndex(const char* deviceControl)
{
	for (unsigned int i = 0; i < m_Controls.Size(); ++i)
		if ( m_Controls[i].name == deviceControl )
			return i;
	return INVALID_CODE;
}

float JoystickDevice::GetRawValue(long controlIndex) const
{
	if ( (unsigned long)controlIndex >= m_Controls.Size() )
		return 0.0f;

	return m_Controls[controlIndex].inverted ? -m_Controls[controlIndex].value : m_Controls[controlIndex].value;
}

bool JoystickDevice::IsStartPressed() const
{
	return fabsf(GetRawValue(indexStart)) > 0.001f;
}

bool JoystickDevice::IsDisconnected() const
{
	HRESULT hr = m_Device->Poll();
	if (SUCCEEDED(hr) || hr == DIERR_NOTACQUIRED)
		return false;

	return true;
}

bool JoystickDevice::SetValues(ValueType type, const void *values, long nBytes)
{
	if( type == FFRotorSpeed )
	{
		long n = nBytes/sizeof(float);

		Assert(n >= 2)

		float ls = ((float *)values)[0];
		float rs = ((float *)values)[1];

		LONG rglDirection[2] = {0,0};

		ls *= -10000.0f; if( ls < -10000.0f ) ls = -10000.0f;
		rs *=  10000.0f; if( rs >  10000.0f ) rs =  10000.0f;

		DIRAMPFORCE cf;
		DIEFFECT   eff; ZeroMemory(&eff,sizeof(eff));

		eff.dwSize				  = sizeof(DIEFFECT);
		eff.dwFlags				  = DIEFF_CARTESIAN|DIEFF_OBJECTOFFSETS;
	//	eff.cAxes				  = axesN;
		eff.cAxes				  = 2;
		eff.rglDirection          = rglDirection;
		eff.lpEnvelope			  = 0;
		eff.cbTypeSpecificParams  = sizeof(DIRAMPFORCE);
		eff.lpvTypeSpecificParams = &cf;
		eff.dwStartDelay		  = 0;
		eff.dwDuration			  = INFINITE;

		cf.lStart = (LONG)ls;
		cf.lEnd   = (LONG)ls;

		if( lef )
			lef->SetParameters(&eff,DIEP_TYPESPECIFICPARAMS|DIEP_START);

		cf.lStart = (LONG)rs;
		cf.lEnd   = (LONG)rs;

		if( ref )
			ref->SetParameters(&eff,DIEP_TYPESPECIFICPARAMS|DIEP_START);

		return true;
	}
	else
		return false;
}

#endif