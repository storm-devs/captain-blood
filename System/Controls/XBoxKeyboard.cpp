#ifdef _XBOX

#include "XBoxKeyboard.h"
#include "pc.h"
#include <xtl.h>

XBoxKeyboard::XBoxKeyboard(Controls& ctrlSrv):
m_Controls(__FILE__, __LINE__),
m_ControlsService(ctrlSrv)
{

}

void XBoxKeyboard::Update(float DeltaTime)
{
	m_ControlsService.ClearKeyBuffer();

	for (;;)
	{
		XINPUT_KEYSTROKE xKey;
		DWORD result = XInputGetKeystroke(0, XINPUT_FLAG_KEYBOARD, &xKey);

		if (result != ERROR_SUCCESS)
		{
			return;
		}

		KeyDescr key;
		if (xKey.Unicode)
		{
			key.bSystem = false;
			key.ucVKey = xKey.Unicode;
		}
		else
		{
			key.bSystem = true;
			key.ucVKey = xKey.VirtualKey;
		}
		m_ControlsService.AddKey(key);
	}
}

long XBoxKeyboard::GetIndex(const char* deviceControl)
{
	return INVALID_CODE;
}

float XBoxKeyboard::GetRawValue(long controlIndex) const
{
	return 0;
}

#endif