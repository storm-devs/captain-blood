#pragma once

#include "..\..\common_h\Mission.h"

class ControlsManager : public MissionObject
{
	struct Item
	{
		ConstString alias;
		ConstString value; const char *name;
	};

	struct Profile
	{
		Profile() : ctls(_FL_)
		{
		}

		int player;	bool single;

		int index;

		const char *name;
		const char *title;

		bool stat;

		array<long> ctls;

		MissionTrigger onSelect;
	};

public:

	 ControlsManager();
	~ControlsManager();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Command(const char *id, dword numParams, const char **params);

private:

	void Update();

	void _cdecl Draw(float dltTime, long level);

	void __declspec(dllexport) InitParams(MOPReader &reader);

	void SetProfile(const char *name, int index, bool reset = false);

//	void Save();
//	void Load();

private:

	array<Item> items;
	int curItem;

	float curTime;

	array<Profile> profiles;
	int curProfile;

	const char *title;

	const char *profile;
	int number;

	MissionTrigger defTrigger;
	MissionTrigger newTrigger;

	bool updated;

	bool m_locked;

};
