#pragma once

#include "device.h"

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

class XBoxKeyboard : public IDevice
{
	struct Control
	{
		string name;
		float value;
	};

	void InitControls();

	array<Control>	m_Controls;
	class Controls &m_ControlsService;

public:

	XBoxKeyboard(Controls &ctrlSrv);

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
