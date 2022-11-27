
#ifndef _BasePattern_h_
#define _BasePattern_h_


#include "..\..\..\Common_h\mission.h"

class BasePattern : public MissionCharacter
{	
public:
	
	bool pattern_was_updated;

	BasePattern();

	virtual bool Create(MOPReader & reader);

	virtual bool EditMode_Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);
	
	inline  bool IsNeedUpdate() { return pattern_was_updated; };

	void _cdecl  EditMode_Work(float dltTime, long level);

	MO_IS_FUNCTION(BasePattern, MissionObject);

};

#endif


