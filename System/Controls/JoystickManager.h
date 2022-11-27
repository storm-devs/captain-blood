#pragma once

#include "device.h"

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

class JoystickManager : public IDevice
{
//	enum ControllerType {Primary,Secondary,Last};
	enum ControllerType {Primary,Secondary,Third,Fourth,Last};

	const static unsigned int MaxJoysticks = Last;

	struct Control
	{
		long realDeviceIndex;
		long realControlIndex;

		string name;
	};

	array<IJoystick *>	m_devices;
	array<Control>		m_Controls;

//	long m_primaryIndex;
//	long m_secondaryIndex;

	long m_index_1;
	long m_index_2;
	long m_index_3;
	long m_index_4;

	bool m_isPaused;

private:

	void ScanJoysticks();
	void InitControls();

	// установить номер физического джойстика для первичного или вторичного контроллера
	void AssignIndex(ControllerType type, long deviceIndex);

public:

//	long GetPrimaryIndex() { return m_primaryIndex; }
//	long GetSecondaryIndex() { return m_secondaryIndex; }

	 JoystickManager();
	~JoystickManager();

	void AddJoystick(IJoystick *joy);

	bool IsPaused() const
	{
		return m_isPaused;
	}

	bool SetJoystickValues(IJoystick::ValueType type, const void *values, long nBytes, long joystickIndex = 0);

	bool IsConnected(long index)
	{
		switch( index )
		{
			case 0: return m_index_1 != -1;
			case 1: return m_index_2 != -1;
			case 2: return m_index_3 != -1;
			case 3: return m_index_4 != -1;
		}

		return false;
	}
	long GetAssignIndex(long index)
	{
		switch( index )
		{
			case 0: return m_index_1;
			case 1: return m_index_2;
			case 2: return m_index_3;
			case 3: return m_index_4;
		}

		return -1;
	}

	void ResetTable()
	{
		m_index_1 = -1;
		m_index_2 = -1;
		m_index_3 = -1;
		m_index_4 = -1;

		AssignIndex(Primary	 ,-1);
		AssignIndex(Secondary,-1);
		AssignIndex(Third	 ,-1);
		AssignIndex(Fourth	 ,-1);
	}

	//////////////////////////////////////////////////////////////////////////
	// IDevice
	//////////////////////////////////////////////////////////////////////////

	virtual void Update(float deltaTime);
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

};
