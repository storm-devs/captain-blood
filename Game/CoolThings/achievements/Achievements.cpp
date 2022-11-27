#include "Achievements.h"
#include "..\..\..\common_h\ILiveService.h"

Achievement::Achievement()
{
	storageValue = null;
	isEarned = false;
}

Achievement::~Achievement()
{
	RELEASE(storageValue);
}

bool Achievement::Create(MOPReader & reader)
{
	liveService = (ILiveService *)api->GetService("LiveService");
	return EditMode_Update(reader);
}

bool Achievement::EditMode_Update(MOPReader & reader)
{
	name = reader.String();
	liveID = reader.Long();
	maxCount = reader.Long();
	isMultiplayer = reader.Bool();
	mpIndex = reader.Long();
	bool active = reader.Bool();
	
	// 
	isEarned = false;

	RELEASE(storageValue);

	char temp[256];
	if (isMultiplayer)
		crt_snprintf(temp, sizeof(temp), "Profile.Achievements.%s%d", name.c_str(), mpIndex);
	else
		crt_snprintf(temp, sizeof(temp), "Profile.Achievements.%s", name.c_str());

	storageValue = api->Storage().GetItemFloat(temp, __FILE__, __LINE__);

	// обнуляем значение если мультиплеерная ачивка
	if (isMultiplayer)
		storageValue->Set(0.0f);

	Activate(active);

	return true;
}

void Achievement::Activate(bool active)
{
	MissionObject::Activate(active);

	if (EditMode_IsOn())
		return;

	if (active)
		SetUpdate((MOF_UPDATE)&Achievement::Execute, ML_GUI1);
	else
		DelUpdate();
}

void Achievement::EarnAchievement()
{
	if (!liveService)
		return;

	if (isEarned)
		return;

	if (isMultiplayer)
		liveService->Achievement_EarnMP(mpIndex, liveID);
	else
		liveService->Achievement_Earn(liveID);

	isEarned = true;
}

void Achievement::CheckAchievement()
{
	if (EditMode_IsOn()) return;

	if (isEarned || !storageValue)
		return;

	if (storageValue->Get() >= maxCount)
		EarnAchievement();
}

void Achievement::Increment(float count)
{
	if (storageValue)
		storageValue->Set(storageValue->Get() + count);

	CheckAchievement();
}

void Achievement::Set(float count)
{
	if (storageValue)
		storageValue->Set(count);

	CheckAchievement();
}

void Achievement::SetZero()
{
	if (storageValue)
		storageValue->Set(0.0f);
}

void Achievement::Command(const char * id, dword numParams, const char * * params)
{
	if (!id || !id[0])
	{
		LogicDebugError("Command id empty, achievement: %s", name.c_str());
		return;
	}

	if (id[0] == 'e' || id[0] == 'E')
	{
		LogicDebug("Achievement earn : %s", name.c_str());
		EarnAchievement();
		return;
	}
	else if (id[0] == 'i' || id[0] == 'i')
	{
		if (numParams >= 1)
		{
			float value = float(atof(params[0]));
			LogicDebug("Achievement increase : %s, %.0f", name.c_str(), value);
			Increment(value);
		}
		else
			LogicDebugError("Achievement increase : %s, need number in parameters", name.c_str());
		return;
	}
	else if (id[0] == 'z' || id[0] == 'Z')
	{
		LogicDebug("Achievement set zero : %s", name.c_str());
		SetZero();
		return;
	}
	else if (id[0] == 's' || id[0] == 'S')
	{
		if (numParams >= 1)
		{
			float value = float(atof(params[0]));
			LogicDebug("Achievement set : %s, %.0f", name.c_str(), value);
			Set(value);
		}
		else
			LogicDebugError("Achievement set : %s, need number in parameters", name.c_str());
		return;
	}
	else if (id[0] == 'c' || id[0] == 'C')
	{
		if (id[1] == 'o' || id[1] == 'O')
		{
			if (numParams >= 1)
			{
				bool isOk = false;
				float value = api->Storage().GetFloat(params[0], 0.0f, &isOk);
				if (isOk)
				{
					LogicDebug("Achievement copy from: %s, value: %.0f, achievement: %s", params[0], value, name.c_str());
					Set(value);
				}
				else
					LogicDebugError("Achievement copy, can't find profile attribute: %s, achievement: %s", params[0], name.c_str());
			}
			else
				LogicDebugError("Achievement set : %s, need profile attribute name in parameters", name.c_str());
			return;
		}
		else
		{
			LogicDebug("Achievement check : %s", name.c_str());
			CheckAchievement();
			return;
		}
	}

	LogicDebugError("Unknown achivement id command: %s, achievement: %s", id, name.c_str());
}

void _cdecl Achievement::Execute(float deltaTime, long level)
{
	CheckAchievement();
}

const char * Achievement::comment = 
"This object representation achievement in game\n"
"  Aviable commands list:\n"
"    check - проверить ачивмент на достижение\n"
"    earn - получить ачивмент\n"
"    increase - добавить число\n"    
"    zero - обнулить ачивмент\n"
"    set - установить число\n"
"    copy - устанавливает число из переменной в профайле\n"
" ";
