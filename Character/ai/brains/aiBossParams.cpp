

#include "aiBossParams.h"
#include "..\CharacterAI.h"


aiBossParams::aiBossParams()
{
}

aiBossParams::~aiBossParams()
{
}

bool aiBossParams::IsControllerSupport(const ConstString & controllerClassName)
{
	CHECK_CONTROLLERSUPPORT("aiBossParams")
}

bool aiBossParams::Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiBossParams::EditMode_Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiBossParams::EditMode_Update(MOPReader & reader)
{
	Init(reader);
	return true;
}

void aiBossParams::Init(MOPReader & reader)
{
	WayPointsName=reader.String();
	fDistofView=reader.Float();	
}

MOP_BEGINLISTCG(aiBossParams, "AI Boss params", '1.00', 90, "AI Boss Params", "Character")	

	MOP_STRING("WayPoints Name", "WayPoints")
	MOP_FLOATEX("Dist of View", 12.0f, 3.0f, 1000.0f)

MOP_ENDLIST(aiBossParams)