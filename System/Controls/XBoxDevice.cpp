#include "XBoxDevice.h"

#include "..\..\common_h\Render.h"
#include "..\..\common_h\controls.h"

#ifdef _XBOX
#include <xtl.h>
#else
#include <xinput.h>
#endif

XBoxDevice:: XBoxDevice(unsigned int ctrlIndex, const ControlsIniParser &parser) :
	m_Controller(ctrlIndex),
	m_Controls(_FL_)
{
	XINPUT_VIBRATION v = {0,0};

	XInputSetState(m_Controller,&v);

	InitControls(parser.GetMappingTable(XBOX360_DEVICE));

	dwPacketNumber = -1;
}

XBoxDevice::~XBoxDevice()
{
	XINPUT_VIBRATION v = {0,0};

	XInputSetState(m_Controller,&v);
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

void XBoxDevice::InitControls(const ControlsIniParser::Table& table)
{
	Control ctrl;

	ctrl.type = Control::Axis;
	ctrl.name = GetMappedName("hw_Axis1", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_Axis2", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_Axis3", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_Axis4", table, ctrl.inverted); m_Controls.Add(ctrl);

	ctrl.type = Control::Slider;
	ctrl.name = GetMappedName("hw_Axis5", table, ctrl.inverted); m_Controls.Add(ctrl);

	ctrl.type = Control::Button;
	ctrl.name = GetMappedName("hw_b1", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b2", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b3", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b4", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b5", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b6", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b7", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b8", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b9", table, ctrl.inverted); m_Controls.Add(ctrl);
	ctrl.name = GetMappedName("hw_b10", table, ctrl.inverted); m_Controls.Add(ctrl);

	ctrl.type = Control::POV_H;
	ctrl.name = GetMappedName("hw_PovH1", table, ctrl.inverted); m_Controls.Add(ctrl);

	ctrl.type = Control::POV_V;
	ctrl.name = GetMappedName("hw_PovV1", table, ctrl.inverted); m_Controls.Add(ctrl);

	///////////////////////////////

	indexStart = GetIndex("gp_B7");
}

void XBoxDevice::Update(float DeltaTime)
{
	XINPUT_STATE state;

	if( XInputGetState(m_Controller,&state) != ERROR_SUCCESS)
	{
		for( int i = 0 ; i < m_Controls; i++ )
		{
			m_Controls[i].value = 0.0f;
		}

		return;
	}

	if( state.dwPacketNumber == dwPacketNumber )
		return;

	dwPacketNumber = state.dwPacketNumber;

/*	if (state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		state.Gamepad.sThumbLX = 0;
	if (state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		state.Gamepad.sThumbLY = 0;
	if (state.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		state.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		state.Gamepad.sThumbRX = 0;
	if (state.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		state.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		state.Gamepad.sThumbRY = 0;*/

	const short l_x_l = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  + 3000;
	const short l_x_h = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  + 1000;

	const short l_y_l = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  + 2000;
	const short l_y_h = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  + 2000;

	const short r_x_l = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 2000;
	const short r_x_h = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 2000;

	const short r_y_l = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 2000;
	const short r_y_h = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 2000;

	if( state.Gamepad.sThumbLX <  l_x_h &&
		state.Gamepad.sThumbLX > -l_x_l )
		state.Gamepad.sThumbLX = 0;
	if( state.Gamepad.sThumbLY <  l_y_h &&
		state.Gamepad.sThumbLY > -l_y_l )
		state.Gamepad.sThumbLY = 0;
	if (state.Gamepad.sThumbRX <  r_x_h &&
		state.Gamepad.sThumbRX > -r_x_l )
		state.Gamepad.sThumbRX = 0;
	if (state.Gamepad.sThumbRY <  r_y_h &&
		state.Gamepad.sThumbRY > -r_y_l )
		state.Gamepad.sThumbRY = 0;

	m_Controls[0].value = state.Gamepad.sThumbLX/(float)0x7FFF;
	m_Controls[1].value = state.Gamepad.sThumbLY/(float)0x7FFF;
	m_Controls[2].value = state.Gamepad.sThumbRX/(float)0x7FFF;
	m_Controls[3].value = state.Gamepad.sThumbRY/(float)0x7FFF;

	if( state.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
		state.Gamepad.bLeftTrigger = 0;
	if( state.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
		state.Gamepad.bRightTrigger = 0;

	// оба шифта как одна ось
	m_Controls[4].value = (state.Gamepad.bLeftTrigger-state.Gamepad.bRightTrigger) / 255.0f;

	m_Controls[5].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0 ? 1.0f : 0.0f;
	m_Controls[6].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0 ? 1.0f : 0.0f;
	m_Controls[7].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0 ? 1.0f : 0.0f;
	m_Controls[8].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0 ? 1.0f : 0.0f;
	m_Controls[9].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0 ? 1.0f : 0.0f;
	m_Controls[10].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0 ? 1.0f : 0.0f;
	m_Controls[11].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0 ? 1.0f : 0.0f;
	m_Controls[12].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0 ? 1.0f : 0.0f;
	m_Controls[13].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0 ? 1.0f : 0.0f;
	m_Controls[14].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0 ? 1.0f : 0.0f;
		
	m_Controls[15].value = 0.0f;
	(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0 ? m_Controls[15].value = 1.0f : m_Controls[15].value;
	(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0 ? m_Controls[15].value = -1.0f : m_Controls[15].value;

	m_Controls[16].value = 0.0f;
	(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0 ? m_Controls[16].value = 1.0f : m_Controls[16].value;
	(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0 ? m_Controls[16].value = -1.0f : m_Controls[16].value;
}

void XBoxDevice::EndFrame()
{
#ifndef _XBOX
//	if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
	if( api->DebugKeyState(VK_NUMPAD0))
	{
		IRender* render = (IRender*)api->GetService("DX9Render");
		for (unsigned int i = 0; i < m_Controls.Size(); ++i)
			render->GetSystemFont()->Print((m_Controller+1)*200.0f, i*15.0f, "%s : %0.2f", m_Controls[i].name.GetBuffer(), m_Controls[i].value);
	}
#endif
}

long XBoxDevice::GetIndex(const char *deviceControl)
{
	for( int i = 0; i < m_Controls ; i++ )
	{
		if( m_Controls[i].name == deviceControl )
			return i;
	}

	return INVALID_CODE;
}

float XBoxDevice::GetRawValue(long controlIndex) const
{
	if((dword)controlIndex >= m_Controls.Size())
		return 0;

	return m_Controls[controlIndex].inverted ? -m_Controls[controlIndex].value : m_Controls[controlIndex].value;
}

bool XBoxDevice::IsStartPressed() const
{
	return fabsf(GetRawValue(indexStart)) > 0.001f;
}

bool XBoxDevice::IsDisconnected() const
{
	XINPUT_STATE state;

	return XInputGetState(m_Controller, &state) != ERROR_SUCCESS;
}

bool XBoxDevice::SetValues(ValueType type, const void *values, long nBytes)
{
	if( type == FFRotorSpeed )
	{
		long n = nBytes/sizeof(float);

		Assert(n >= 2)

		float ls = ((float *)values)[0];
		float rs = ((float *)values)[1];

		ls *= 65535.0f; if( ls > 65535.0f ) ls = 65535.0f;
		rs *= 65535.0f; if( rs > 65535.0f ) rs = 65535.0f;

		XINPUT_VIBRATION v = {(WORD)ls,(WORD)rs};

		XInputSetState(m_Controller,&v);

		return true;
	}
	else
		return false;
}
