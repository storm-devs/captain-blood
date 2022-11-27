#pragma once

#include "device.h"

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

struct IDirectInputDevice8A;
struct IDirectInput8A;

struct DIDEVICEINSTANCEA;

class MouseDevice : public IDevice
{
	enum DataItem { DeltaX, DeltaY, Wheel, X, Y, LButton, RButton, MButton,
					DblLButton,	DblRButton,	DblMButton, Last };

	struct Control
	{
		DataItem dataItem;

		string name;
		float value;
	};

	long m_Data[Last];

	HWND m_hWnd;

	IDirectInput8A *di;

	IDirectInputDevice8A *m_Device;

	array<Control> m_Controls;

	void __declspec(dllexport) InitControls();

public:

	MouseDevice(IDirectInput8A* di, const DIDEVICEINSTANCEA& deviceInst);
	virtual ~MouseDevice();

	//////////////////////////////////////////////////////////////////////////
	// IDevice
	//////////////////////////////////////////////////////////////////////////

	virtual void Update(float DeltaTime);
	virtual void EndFrame() {}

	virtual long GetIndex(const char *deviceControl);
	virtual float GetRawValue(long controlIndex) const;

	virtual unsigned int GetControlsCount() const
	{
		return m_Controls.Size();
	}
	virtual const char *GetControlName(unsigned int index) const
	{
		Assert(index < m_Controls.Size());
		return m_Controls[index].name.c_str();
	}

};
