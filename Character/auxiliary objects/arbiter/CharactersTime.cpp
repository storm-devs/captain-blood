#include "CharactersTime.h"

CharactersTime::CharactersTime()
{
	db_time = 0.0;
	dw_time = 0;
}

CharactersTime::~CharactersTime()
{
}

void CharactersTime::Execute(float deltaTime)
{
	db_time += deltaTime;
	dw_time = dword(db_time * 128.0);
}

dword CharactersTime::Convert2Dword(float time) const
{
	return dword(time * 128.0f);
}
