#ifndef NO_TOOLS

#pragma once

#include "..\..\..\Common_h\Mission.h"

class SoundTrigger : public MissionObject
{
public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

private:

	void InitParams		(MOPReader &reader);

public:

	void Activate(bool isActive);

	MO_IS_FUNCTION(SoundTrigger, MissionObject);

private:

	void _cdecl Hand(const char *group, MissionObject *sender);

private:

	MissionTrigger trigger;

};

struct ICharacterTable : public MissionObject
{
	struct Item
	{
		ConstString name;	// имя миссионного объекта
		bool		play;	// синхронно играть граф
	};

	virtual const array<Item> &GetTable() = 0;
	virtual const array<Item> &GetLocators() = 0;

	virtual float GetBlend() = 0;

	virtual bool GetRot() = 0;
};

class CharacterTable : public ICharacterTable
{
public:

	CharacterTable() :
		table	(_FL_),
		locators(_FL_)
	{
	}

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

private:

	void InitParams		(MOPReader &reader);

public:

	void Activate(bool isActive);

	MO_IS_FUNCTION(CharacterTable, MissionObject);

public:

	virtual const array<Item> &GetTable()
	{
		return table;
	}

	virtual const array<Item> &GetLocators()
	{
		return locators;
	}

	virtual float GetBlend()
	{
		return blend;
	}

	virtual bool GetRot()
	{
		return rot;
	}

private:

	void _cdecl Hand(const char *group, MissionObject *sender);

private:

	array<Item> table;
	array<Item> locators;

	float blend;

	bool rot;

};

#endif // #ifndef NO_TOOLS
