#ifndef AI_BossParams
#define AI_BossParams

#include "aiBrain.h"


class aiBossParams : public CharacterControllerParams
{

public:	

	ConstString WayPointsName;
	float fDistofView;	

	aiBossParams();
	~aiBossParams();
	
	virtual bool IsControllerSupport(const ConstString & controllerClassName);
	
	bool Create(MOPReader & reader);
	
	bool EditMode_Create(MOPReader & reader);
	
	bool EditMode_Update(MOPReader & reader);
	
	void Init (MOPReader & reader);	
};

#endif


