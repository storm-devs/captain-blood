#pragma once

#include "device.h"

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

struct IDirectInputDevice8A;
struct IDirectInput8A;

struct DIDEVICEINSTANCEA;

class KeyboardDevice : public IDevice
{
	struct Control
	{
		string name;
		float value;
		byte diCode;
	};

	IDirectInputDevice8A *m_Device;

	array<Control>	m_Controls;
	class Controls &m_ControlsService;

	void __declspec(dllexport) InitControls();

public:

	 KeyboardDevice(Controls &ctrlSrv, IDirectInput8A *di, const DIDEVICEINSTANCEA &deviceInst);
	~KeyboardDevice();

	//////////////////////////////////////////////////////////////////////////
	//  IDevice
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
