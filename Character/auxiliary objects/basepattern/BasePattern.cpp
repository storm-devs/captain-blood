
#include "BasePattern.h"

BasePattern::BasePattern()
{
	pattern_was_updated = false;
}

bool BasePattern::Create(MOPReader & reader)
{
	Activate(true);	
	EditMode_Update(reader);

	return true;
}

bool BasePattern::EditMode_Create(MOPReader & reader)
{
	EditMode_Update(reader);
	SetUpdate(&BasePattern::EditMode_Work,ML_EXECUTE4);

	return true;
}

void _cdecl BasePattern::EditMode_Work(float dltTime, long level)
{	
	if (pattern_was_updated) pattern_was_updated = false;
	//DelUpdate(&BasePattern::EditMode_Work);
}

bool BasePattern::EditMode_Update(MOPReader & reader)
{
	pattern_was_updated = true;

	return true;	
}