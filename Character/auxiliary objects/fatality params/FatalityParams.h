#ifndef FATALITY_PARAMS_H
#define FATALITY_PARAMS_H

#include "..\..\..\Common_h\Mission.h"

#define FATALITY_PARAMS_GROUP GroupId('F','T','P','M')

class FatalityParams : public MissionObject
{
public:

	enum TFatalityType
	{
		ingame = 0,
		ai_initiate = 1,
		ingame_instante = 2
	};

	struct hpParams
	{
		ConstString diff;
		float fMinHP;
		float fMaxHP;
	};

	struct FatalityLink
	{

		ConstString Name;
		
		int num_hp_params;
		int cur_hp_params;
		hpParams hp_params[5];

		TFatalityType fatal_type;		
	};

	array<FatalityLink> Links;
	
	//Конструктор - деструктор
	FatalityParams();
	virtual ~FatalityParams();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(FatalityParams, MissionObject);

	void FindCurHPBorder(const ConstString & diff);
	bool CheckHPBorder(float hp, int index);

	static const char * comment;

};

#endif