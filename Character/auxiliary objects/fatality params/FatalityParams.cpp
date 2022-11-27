
#include "FatalityParams.h"

FatalityParams::FatalityParams(): Links(_FL_)
{
	
}

FatalityParams::~FatalityParams()
{
}

bool FatalityParams::Create(MOPReader & reader)
{
	Activate(true);	
	EditMode_Update (reader);

	Registry(FATALITY_PARAMS_GROUP);

	return true;
}

bool FatalityParams::EditMode_Update(MOPReader & reader)
{
	Links.DelAll();

	long PointsCount = reader.Array();

	for (long i=0;i<PointsCount;i++)
	{
		FatalityLink & FLink = Links[Links.Add()];

		FLink.Name = reader.String();
		
		FLink.num_hp_params = reader.Array();
		FLink.cur_hp_params = 0;

		for (long j=0;j<FLink.num_hp_params;j++)
		{
			FLink.hp_params[j].diff = reader.String();
			FLink.hp_params[j].fMinHP = reader.Float();
			FLink.hp_params[j].fMaxHP = reader.Float();
		}		

		ConstString fat_type = reader.Enum();
				
		if (fat_type.NotEmpty() && fat_type.c_str()[0] == 'a')
		{
			FLink.fatal_type = ai_initiate;
		}
		else		
		{
			static const ConstString ingameId("ingame");
			if (fat_type == ingameId)
			{
				FLink.fatal_type = ingame;
			}
			else
			{
				FLink.fatal_type = ingame_instante;
			}						
		}		
	}

	return true;
}

void FatalityParams::FindCurHPBorder(const ConstString & diff)
{
	for (int i=0;i<(int)Links.Size();i++)
	{
		FatalityLink & FLink = Links[i];

		FLink.cur_hp_params = 0;

		for (int j=0;j<FLink.num_hp_params;j++)
		{
			if (FLink.hp_params[j].diff == diff)
			{
				FLink.cur_hp_params = j;
				break;
			}
		}
	}
}

bool FatalityParams::CheckHPBorder(float hp, int index)
{
	hpParams & hp_param = Links[index].hp_params[Links[index].cur_hp_params];

	if ( hp_param.fMinHP <= hp && hp <= hp_param.fMaxHP)
	{
		return true;
	}

	return false;
}

static const char * comment;
const char * FatalityParams::comment = 
"Fatality Params";

MOP_BEGINLISTG(FatalityParams, "Fatality Params", '1.00', 50, "Character")

	MOP_ENUMBEG("FatalityType")
		MOP_ENUMELEMENT("ingame")
		MOP_ENUMELEMENT("ai_initiate")
		MOP_ENUMELEMENT("ingame_instante")
	MOP_ENUMEND

	MOP_ARRAYBEG("Fatality Links", 0, 200)
		
	    MOP_STRING("Link Name", "")

		MOP_ARRAYBEGC("HP Border", 1, 5, "HP Border")
			MOP_STRING("Difficulty", "")
			MOP_FLOATEX("MinHP", 0.0f, 0, 1.0f)
			MOP_FLOATEX("MaxHP", 1.0f, 0, 1.0f)			
		MOP_ARRAYEND
		
		MOP_ENUM("FatalityType", "FatalityType")

	MOP_ARRAYEND		
MOP_ENDLIST(FatalityParams)