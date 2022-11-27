#ifndef NO_TOOLS

#include "SoundTools.h"

////////////////////////////////////////////

bool SoundTrigger::Create(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool SoundTrigger::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void SoundTrigger::InitParams(MOPReader &reader)
{
	trigger.Init(reader);

	Activate(reader.Bool());
}

void SoundTrigger::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	Unregistry(GroupId('S','d','T','r'));

	if( isActive )
	{
		Registry(GroupId('S','d','T','r'),&SoundTrigger::Hand,ML_FIRST);
	}

	LogicDebug(isActive ? "Activate" : "Deactivate");
}

void _cdecl SoundTrigger::Hand(const char *group, MissionObject *sender)
{
	Assert(IsActive())

	trigger.Activate(Mission(),false);
}

MOP_BEGINLISTCG(SoundTrigger, "SoundTrigger", '1.00', 100, "SoundTrigger", "Character")

	MOP_MISSIONTRIGGER("")

	MOP_BOOL("Active", true)

MOP_ENDLIST(SoundTrigger)

//////////////////////////////////////////////

bool CharacterTable::Create(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool CharacterTable::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void CharacterTable::InitParams(MOPReader &reader)
{
	blend = reader.Float();

	rot = reader.Bool();

	table.DelAll();

	int n = reader.Array();

	for( int i = 0 ; i < n ; i++ )
	{
		Item &item = table[table.Add()];

		item.name = reader.String();
		item.play = reader.Bool();
	}

	locators.DelAll();

	int m = reader.Array();

	for( int i = 0 ; i < m ; i++ )
	{
		Item &item = locators[locators.Add()];

		item.name = reader.String();
		item.play = reader.Bool();
	}

	Activate(reader.Bool());
}

void CharacterTable::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	Unregistry(GroupId('C','r','T','b'));

	if( isActive )
	{
		Registry(GroupId('C','r','T','b'),&CharacterTable::Hand,ML_FIRST);
	}

	LogicDebug(isActive ? "Activate" : "Deactivate");
}

void _cdecl CharacterTable::Hand(const char *group, MissionObject *sender)
{
	Assert(IsActive())

	Assert(0)
}

MOP_BEGINLISTCG(CharacterTable, "CharacterTable", '1.00', 100, "CharacterTable", "Character")

	MOP_FLOATC("Blend time", 0.2f, "Время блендинга для перехода между нодами")

	MOP_BOOLC("Model rotation", false, "Позволить вращать модельку чарактера")

	MOP_ARRAYBEG("Characters", 0, 100)

		MOP_STRINGC("Name",   "", "Имя миссионного объекта")
		MOP_BOOLC  ("Play", true, "Управлять объектом")

	MOP_ARRAYEND

	MOP_ARRAYBEG("Locators", 0, 100)

		MOP_STRINGC("Name",   "", "Имя локатора")
		MOP_BOOLC  ("Play", true, "Использовать")

	MOP_ARRAYEND

	MOP_BOOLC("Active", true, "Использовать таблицу")

MOP_ENDLIST(CharacterTable)

#endif // #ifndef NO_TOOLS
