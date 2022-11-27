#pragma once

#include "device.h"

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

#include "ControlsIniParser.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class JoystickDevice : public IJoystick
{
	struct Control
	{
		enum Type { Axis, Button, POV_H, POV_V };
		Type			type;
		unsigned int	dataOffset;
		string			name;
		float			value;
		bool			inverted;
	};

	unsigned int m_NumButtons;
	unsigned int m_NumAxes;
	unsigned int m_NumPOVs;

	DIJOYSTATE2	m_Data;

	array<Control> m_Controls;

	long indexStart;

	IDirectInputDevice8A *m_Device;

	void InitControls(const ControlsIniParser::Table &table);
	unsigned int GetAxisDataOffset(unsigned int index);
	Control::Type GetAxisType(unsigned int index);
	float GetButtonData(unsigned int dataOffset);
	float GetAxisData(Control::Type axisType, unsigned int dataOffset);
	float GetPOVData(unsigned int dataOffset, bool isPovH);

	LPDIRECTINPUTEFFECT	lef;
	LPDIRECTINPUTEFFECT	ref;

public:

	 JoystickDevice(IDirectInput8A* di, const DIDEVICEINSTANCEA& deviceInst, const ControlsIniParser& parser);
	~JoystickDevice();

	//////////////////////////////////////////////////////////////////////////
	// IDevice
	//////////////////////////////////////////////////////////////////////////

	virtual void Update(float DeltaTime);
	virtual void EndFrame();

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

	///////////////////////////
	// IJoystick
	///////////////////////////

	virtual bool IsStartPressed() const;
	virtual bool IsDisconnected() const;

	virtual bool SetValues(ValueType type, const void *values, long nBytes);

};
