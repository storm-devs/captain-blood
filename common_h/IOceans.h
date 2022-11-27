#pragma once

#include "core.h"

class IOcean3Service : public Service
{
public:
	dword __inline ProtectValue() { return protectValue; }
	virtual IRender * GetVRender() { return vRender; }

protected:
	dword protectValue;
	IRender * vRender;
};
