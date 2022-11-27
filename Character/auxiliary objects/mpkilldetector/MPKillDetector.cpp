#include "MPKillDetector.h"
#include "..\..\..\Common_h\ICharacter.h"

MPKillDetector::MPKillDetector() :
	killers(_FL_, 8),
	unknownKiller(_FL_, 4)
{
}

MPKillDetector::~MPKillDetector()
{
}

bool MPKillDetector::Create(MOPReader & reader)
{
	FindObject(reader.String(), object);
	isLive = false;
	isDead = false;

	int killersNum = reader.Array();
	killers.AddElements(killersNum);
	for (int i=0; i<killersNum; i++)
		CreateKiller(reader, killers[i]);

	int activatesNum = reader.Array();
	unknownKiller.AddElements(activatesNum);
	for (int i=0; i<activatesNum; i++)
	{
		unknownKiller[i].object = reader.String();
		unknownKiller[i].isActivate = reader.Bool();
	}

	Activate(reader.Bool());
	return true;
}

void MPKillDetector::CreateKiller(MOPReader & reader, Killer & killer)
{
	killer.offender = reader.String();
	
	int activatesNum = reader.Array();
	killer.objects.AddElements(activatesNum);
	for (int i=0; i<activatesNum; i++)
	{
		killer.objects[i].object = reader.String();
		killer.objects[i].isActivate = reader.Bool();
	}
}

void MPKillDetector::Restart()
{
	isLive = false;
	isDead = false;
}

void MPKillDetector::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (!EditMode_IsOn())
	{
		if (IsActive())
		{
			LogicDebug("Activate");
			SetUpdate(&MPKillDetector::Work, ML_TRIGGERS);

			if (object.Validate())
			{
				isLive = IsAlive();
				isDead = !isLive;
			}
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&MPKillDetector::Work);
		}
	}else{
		DelUpdate(&MPKillDetector::Work);
	}
}

//Инициализировать объект
bool MPKillDetector::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool MPKillDetector::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Работа детектора
void _cdecl MPKillDetector::Work(float dltTime, long level)
{
	if (object.Validate())
	{
		if (isLive)
		{
			isDead = !IsAlive();
			if (isDead)
			{
				bool activateUnknown = true;
				isLive = false;	

				if (object.Ptr()->Is(ICharacter::GetTypeId()))
				{
					const ConstString & offender = ((ICharacter*)object.Ptr())->GetKillerId();
					if (offender.NotEmpty())
					{
						for (int i=0; i<killers.Len(); i++) 
							if (offender == killers[i].offender)
							{
								ActivateObjects(killers[i].objects);
								activateUnknown = false;
								break;
							}
					}
				}

				if (activateUnknown)
				{
					// выполняем дефолтные триггера 
					ActivateObjects(unknownKiller);
				}
			}
		}
		else
		{
			isLive = IsAlive();
			isDead = !isLive;
		}
	}
}

void MPKillDetector::ActivateObjects(const array<Activator> & activator)
{
	for (int i=0; i<activator.Len(); i++)
	{
		MOSafePointer active;
		if (FindObject(activator[i].object, active))
			active.Ptr()->Activate(true);
	}
}

//Проверить жив или нет
bool MPKillDetector::IsAlive()
{
	bool dead = object.Ptr()->IsDie() | object.Ptr()->IsDead();
	return !dead;
}

MOP_BEGINLISTCG(MPKillDetector, "Multiplayer kill detector", '1.00', 0x0fffffff, "Multiplayer kill detector", "Logic")
	MOP_STRING("Object id", "")
	MOP_ARRAYBEG("Killers", 0, 100)
		MOP_STRING("Killer id", "")
		MOP_ARRAYBEG("Activate objects", 0, 100)
			MOP_STRING("Object name", "")
			MOP_BOOL("Is active object", false)
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_GROUPBEG("Unknown killer")
		MOP_ARRAYBEG("Activate objects", 0, 100)
			MOP_STRING("Object name", "")
			MOP_BOOL("Is active object", false)
		MOP_ARRAYEND
	MOP_GROUPEND();
	MOP_BOOL("Active", true)
MOP_ENDLIST(MPKillDetector)
