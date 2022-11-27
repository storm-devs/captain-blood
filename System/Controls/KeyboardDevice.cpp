#ifndef _XBOX

#include "KeyboardDevice.h"
#include "pc.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "..\..\common_h\systemmsg.h"

KeyboardDevice::KeyboardDevice(Controls& ctrlSrv, IDirectInput8A* di, const DIDEVICEINSTANCEA& deviceInst) :
m_Device(NULL),
m_ControlsService(ctrlSrv),
m_Controls(__FILE__, __LINE__)
{
	Assert(di);
	di->CreateDevice(deviceInst.guidInstance, &m_Device, NULL);
	Assert(m_Device);

	HWND hWnd = (HWND)api->Storage().GetLong("system.hWnd");
	m_Device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ); 

	m_Device->SetDataFormat(&c_dfDIKeyboard);
	m_Device->Acquire();

	InitControls();
}

KeyboardDevice::~KeyboardDevice()
{
	if (m_Device)
	{
		m_Device->Unacquire();
		m_Device->Release(), m_Device = NULL;
	}
}

void __declspec(dllexport) KeyboardDevice::InitControls()
{
	Control ctrl;
	ctrl.value = 0.0f;

	ctrl.diCode = DIK_A; ctrl.name = "A"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_B; ctrl.name = "B"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_C; ctrl.name = "C"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_D; ctrl.name = "D"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_E; ctrl.name = "E"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F; ctrl.name = "F"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_G; ctrl.name = "G"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_H; ctrl.name = "H"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_I; ctrl.name = "I"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_J; ctrl.name = "J"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_K; ctrl.name = "K"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_L; ctrl.name = "L"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_M; ctrl.name = "M"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_N; ctrl.name = "N"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_O; ctrl.name = "O"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_P; ctrl.name = "P"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_Q; ctrl.name = "Q"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_R; ctrl.name = "R"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_S; ctrl.name = "S"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_T; ctrl.name = "T"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_U; ctrl.name = "U"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_V; ctrl.name = "V"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_W; ctrl.name = "W"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_X; ctrl.name = "X"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_Y; ctrl.name = "Y"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_Z; ctrl.name = "Z"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_0; ctrl.name = "0"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_1; ctrl.name = "1"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_2; ctrl.name = "2"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_3; ctrl.name = "3"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_4; ctrl.name = "4"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_5; ctrl.name = "5"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_6; ctrl.name = "6"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_7; ctrl.name = "7"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_8; ctrl.name = "8"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_9; ctrl.name = "9"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_NUMPAD0; ctrl.name = "kb_numpad0"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD1; ctrl.name = "kb_numpad1"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD2; ctrl.name = "kb_numpad2"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD3; ctrl.name = "kb_numpad3"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD4; ctrl.name = "kb_numpad4"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD5; ctrl.name = "kb_numpad5"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD6; ctrl.name = "kb_numpad6"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD7; ctrl.name = "kb_numpad7"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD8; ctrl.name = "kb_numpad8"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPAD9; ctrl.name = "kb_numpad9"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_F1; ctrl.name = "kb_f1"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F2; ctrl.name = "kb_f2"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F3; ctrl.name = "kb_f3"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F4; ctrl.name = "kb_f4"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F5; ctrl.name = "kb_f5"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F6; ctrl.name = "kb_f6"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F7; ctrl.name = "kb_f7"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F8; ctrl.name = "kb_f8"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F9; ctrl.name = "kb_f9"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F10; ctrl.name = "kb_f10"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F11; ctrl.name = "kb_f11"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F12; ctrl.name = "kb_f12"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F13; ctrl.name = "kb_f13"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F14; ctrl.name = "kb_f14"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_F15; ctrl.name = "kb_f15"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_UP; ctrl.name = "kb_up"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_DOWN; ctrl.name = "kb_down"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_LEFT; ctrl.name = "kb_left"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RIGHT; ctrl.name = "kb_right"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_HOME; ctrl.name = "kb_home"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_END; ctrl.name = "kb_end"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_PRIOR; ctrl.name = "kb_prior"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NEXT; ctrl.name = "kb_next"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_INSERT; ctrl.name = "kb_insert"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_DELETE; ctrl.name = "kb_delete"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_SPACE; ctrl.name = "kb_space"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RETURN; ctrl.name = "kb_return"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_TAB; ctrl.name = "kb_tab"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_BACKSPACE; ctrl.name = "kb_back"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_ESCAPE; ctrl.name = "kb_escape"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_LCONTROL; ctrl.name = "kb_lcontrol"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RCONTROL; ctrl.name = "kb_rcontrol"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_LSHIFT; ctrl.name = "kb_lshift"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RSHIFT; ctrl.name = "kb_rshift"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_LALT; ctrl.name = "kb_lalt"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RALT; ctrl.name = "kb_ralt"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_LWIN; ctrl.name = "kb_lwin"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RWIN; ctrl.name = "kb_rwin"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_NUMLOCK; ctrl.name = "kb_numlock"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_CAPSLOCK; ctrl.name = "kb_capital"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_SCROLL; ctrl.name = "kb_scroll"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_PAUSE; ctrl.name = "kb_pause"; m_Controls.Add(ctrl);
//	ctrl.diCode = DIK_CAPITAL; ctrl.name = "kb_capital"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_SYSRQ; ctrl.name = "kb_snapshot"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_MULTIPLY; ctrl.name = "kb_multiply"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_DIVIDE; ctrl.name = "kb_divide"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_ADD; ctrl.name = "kb_add"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_SUBTRACT; ctrl.name = "kb_subtract"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_APPS; ctrl.name = "kb_apps"; m_Controls.Add(ctrl);

//	ctrl.diCode = DIK_GRAVE; ctrl.name = "kb_grave"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_MINUS;  ctrl.name = "kb_minus";  m_Controls.Add(ctrl);
	ctrl.diCode = DIK_EQUALS; ctrl.name = "kb_equals"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_BACKSLASH; ctrl.name = "kb_backslash"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_LBRACKET; ctrl.name = "kb_lbracket"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_RBRACKET; ctrl.name = "kb_rbracket"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_SEMICOLON;  ctrl.name = "kb_semicolon";  m_Controls.Add(ctrl);
	ctrl.diCode = DIK_APOSTROPHE; ctrl.name = "kb_apostrophe"; m_Controls.Add(ctrl);

	ctrl.diCode = DIK_COMMA;  ctrl.name = "kb_comma";  m_Controls.Add(ctrl);
	ctrl.diCode = DIK_PERIOD; ctrl.name = "kb_period"; m_Controls.Add(ctrl);
	ctrl.diCode = DIK_SLASH;  ctrl.name = "kb_slash";  m_Controls.Add(ctrl);

	ctrl.diCode = DIK_DECIMAL;	   ctrl.name = "kb_decimal";	 m_Controls.Add(ctrl);
	ctrl.diCode = DIK_NUMPADENTER; ctrl.name = "kb_numpadenter"; m_Controls.Add(ctrl);
}

void KeyboardDevice::Update(float DeltaTime)
{
	if( !m_Device )
		return;

	m_ControlsService.ClearKeyBuffer();

	CoreSystemMessage msg;
	ControlsMessage* data = null;

	for( dword i = 0; i < api->GetSystemMessagesCount() ; i++ )
	{
		api->GetSystemMessage(i,msg);

		if( string::IsEqual(msg.id,CONTROLS_MESSAGE))
		{
			data = (ControlsMessage*)msg.data;
			if (data)
			{
				if (data->dwMsgID == WM_CHAR)
				{
					KeyDescr desc;
					desc.bSystem = false;
					desc.ucVKey = data->wParam;
					m_ControlsService.AddKey(desc);
				}

				if (data->dwMsgID == WM_KEYDOWN)
				{
					KeyDescr desc;
					desc.bSystem = true;
					desc.ucVKey = data->wParam;
					m_ControlsService.AddKey(desc);
				}
			}
		}
	}


	unsigned char buffer[256]; 
	HRESULT hr = m_Device->GetDeviceState(sizeof(buffer), buffer);

	if ( SUCCEEDED(hr) )
	{
		for (unsigned int i = 0; i < m_Controls.Size(); ++i)
			m_Controls[i].value = (float)(buffer[m_Controls[i].diCode] >> 7);
	}
	else
	{
		for (unsigned int i = 0; i < m_Controls.Size(); ++i)
			m_Controls[i].value = 0.0f;
	}

	if ( FAILED(hr) )
		m_Device->Acquire();
}

long KeyboardDevice::GetIndex(const char* deviceControl)
{
	for (unsigned int i = 0; i < m_Controls.Size(); ++i)
		if (deviceControl == m_Controls[i].name)
			return i;

	return INVALID_CODE;
}

float KeyboardDevice::GetRawValue(long controlIndex) const
{
	if ((unsigned long)controlIndex >= m_Controls.Size())
		return 0.0f;

	return m_Controls[controlIndex].value;
}

#endif