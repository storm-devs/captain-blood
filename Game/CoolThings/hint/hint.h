#ifndef COOL_THINGS_PLAYER_HINT
#define COOL_THINGS_PLAYER_HINT

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\Common_h\QSort.h"



class csHint : public MissionObject
{
	IFont* pFnt;

	string HintText;

	float fShowTime;
	float fBaseShowTime;


public:

	csHint();
	~csHint();

	void _cdecl Realize(float fDeltaTime, long level);

	void Activate(bool isActive);
	void Show(bool isShow);

	bool Create(MOPReader & reader);
	bool EditMode_Update(MOPReader & reader);
};


#endif