#pragma once

#include "..\..\..\Common_h\ISlowmo.h"

class Slowmo : public ISlowmo
{
public:
	Slowmo();
	virtual ~Slowmo();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual const Slowmo::SlowmoParams * GetSlowmo(const ConstString & name) const;

private:
	array<ISlowmo::SlowmoParams> slowmos;
};

