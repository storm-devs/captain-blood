#pragma once

class IDevice
{
public:

	virtual ~IDevice()
	{
	}

	virtual void Update(float DeltaTime) = 0;
	virtual void EndFrame() = 0;

	virtual long GetIndex(const char *deviceControl) = 0;
	virtual float GetRawValue(long controlIndex) const = 0;

	virtual unsigned int GetControlsCount() const = 0;
	virtual const char *GetControlName(unsigned int index) const = 0;

};

class IJoystick : public IDevice
{
public:

	enum ValueType {FFRotorSpeed};

public:

	virtual bool IsStartPressed() const = 0;
	virtual bool IsDisconnected() const = 0;

	virtual bool SetValues(ValueType type, const void *values, long nBytes) = 0;

};
