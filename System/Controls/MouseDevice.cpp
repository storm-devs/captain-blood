#ifndef _XBOX

#include "MouseDevice.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "..\..\common_h\systemmsg.h"
#include "..\..\common_h\controls.h"

MouseDevice::MouseDevice(IDirectInput8A* di, const DIDEVICEINSTANCEA& deviceInst)
	: m_Controls(_FL_),m_Device(null)
{
	Assert(di)

	di->CreateDevice(deviceInst.guidInstance,&m_Device,null);
	Assert(m_Device)

	m_hWnd = (HWND)api->Storage().GetLong("system.hwnd");
	m_Device->SetCooperativeLevel(m_hWnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);

	m_Device->SetDataFormat(&c_dfDIMouse);
	m_Device->Acquire();

	InitControls();

	this->di = di;
}

MouseDevice::~MouseDevice()
{
	if( m_Device )
	{
		m_Device->Unacquire();

		RELEASE(m_Device)
	}
}

void __declspec(dllexport) MouseDevice::InitControls()
{
	Control ctrl;

	ctrl.value = 0.0f;

	ctrl.dataItem = DeltaX; ctrl.name = "MouseDeltaH"; m_Controls.Add(ctrl);
	ctrl.dataItem = DeltaY; ctrl.name = "MouseDeltaV"; m_Controls.Add(ctrl);

	ctrl.dataItem = Wheel; ctrl.name = "MouseWheel"; m_Controls.Add(ctrl);

	ctrl.dataItem = X; ctrl.name = "MouseH"; m_Controls.Add(ctrl);
	ctrl.dataItem = Y; ctrl.name = "MouseV"; m_Controls.Add(ctrl);

	ctrl.dataItem = LButton; ctrl.name = "MouseLClick"; m_Controls.Add(ctrl);
	ctrl.dataItem = RButton; ctrl.name = "MouseRClick"; m_Controls.Add(ctrl);
	ctrl.dataItem = MButton; ctrl.name = "MouseMClick"; m_Controls.Add(ctrl);

	ctrl.dataItem = DblLButton; ctrl.name = "MouseLDouble"; m_Controls.Add(ctrl);
	ctrl.dataItem = DblRButton; ctrl.name = "MouseRDouble"; m_Controls.Add(ctrl);
	ctrl.dataItem = DblMButton; ctrl.name = "MouseMDouble"; m_Controls.Add(ctrl);
}

void MouseDevice::Update(float DeltaTime)
{
	if( !m_Device )
		return;

	m_Data[DblLButton] =
	m_Data[DblRButton] =
	m_Data[DblMButton] = 0;

	CoreSystemMessage msg;
	ControlsMessage *data = null;

	for( dword i = 0; i < api->GetSystemMessagesCount() ; i++ )
	{
		api->GetSystemMessage(i,msg);

		if( string::IsEqual(msg.id,CONTROLS_MESSAGE))
		{
			if( data = (ControlsMessage *)msg.data )
			{
				switch( data->dwMsgID )
				{
					case WM_LBUTTONDBLCLK:
						m_Data[DblLButton] = 1; break;
					case WM_RBUTTONDBLCLK:
						m_Data[DblRButton] = 1; break;
					case WM_MBUTTONDBLCLK:
						m_Data[DblMButton] = 1; break;
				}
			}
		}
	}
		
	POINT absMousePos = {0};

	DIMOUSESTATE mouseState;
	HRESULT hr = m_Device->GetDeviceState(sizeof(mouseState),&mouseState);

	if( SUCCEEDED(hr))
	{
		GetCursorPos(&absMousePos); ScreenToClient(m_hWnd,&absMousePos);

		m_Data[DeltaX] = mouseState.lX;
		m_Data[DeltaY] = mouseState.lY;

		m_Data[Wheel] = mouseState.lZ/120;

		m_Data[X] = absMousePos.x;
		m_Data[Y] = absMousePos.y;

		m_Data[LButton] = (mouseState.rgbButtons[0] & 0x80) >> 7;
		m_Data[RButton] = (mouseState.rgbButtons[1] & 0x80) >> 7;
		m_Data[MButton] = (mouseState.rgbButtons[2] & 0x80) >> 7;
	}
	else
	{
		m_Data[DeltaX] = m_Data[DeltaY] = m_Data[Wheel] = 0;

		m_Data[X] = m_Data[Y] = 0;

		m_Data[LButton] = m_Data[RButton] = m_Data[MButton] = 0;
	}

	for( dword i = 0; i < m_Controls.Size() ; i++ )
	{
		m_Controls[i].value = (float)m_Data[m_Controls[i].dataItem];
	}

	if( FAILED(hr))
	{
		m_Device->SetCooperativeLevel(m_hWnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
		m_Device->Acquire();
	}
}

long MouseDevice::GetIndex(const char* deviceControl)
{
	for( dword i = 0; i < m_Controls.Size() ; i++ )
	{
		if( deviceControl == m_Controls[i].name )
			return i;
	}

	return INVALID_CODE;
}

float MouseDevice::GetRawValue(long controlIndex) const
{
	if((dword)controlIndex >= m_Controls.Size())
		return 0;

	return m_Controls[controlIndex].value;
}

#endif
