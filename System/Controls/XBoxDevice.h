#pragma once

#include "device.h"

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

#include "ControlsIniParser.h"

#define XBOX360_DEVICE "XBox 360 Device"
class XBoxDevice : public IJoystick
{
	struct Control
	{
		enum Type { Slider, Axis, Button, POV_H, POV_V };
		Type type;
		float value;
		string name;
		bool inverted;
	};

	DWORD dwPacketNumber;

	unsigned int	m_Controller;

	array<Control>	m_Controls;

	long indexStart;

	void InitControls(const ControlsIniParser::Table &table);

public:

	 XBoxDevice(unsigned int ctrlIndex, const ControlsIniParser &parser);
	~XBoxDevice();

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