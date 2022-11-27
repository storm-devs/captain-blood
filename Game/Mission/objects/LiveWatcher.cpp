

#include "LiveWatcher.h"


LiveWatcher::LiveWatcher()
{
}

LiveWatcher::~LiveWatcher()
{
}

//Инициализировать объект
bool LiveWatcher::Create(MOPReader & reader)
{
	ConstString objId = reader.String();
	if(!FindObject(objId, object))
	{
		LogicDebugError("Mission object \"%s\" not found, skip LiveWatcher creation", objId.c_str());
		return false;
	}
	triggerHP = reader.Float()*0.01f;
	trigger.Init(reader);
	Activate(reader.Bool());
	return true;
}

//Активировать
void LiveWatcher::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			LogicDebug("Activate");
			SetUpdate(&LiveWatcher::Work, ML_TRIGGERS);
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&LiveWatcher::Work);
		}
	}else{
		DelUpdate(&LiveWatcher::Work);
	}
}

//Инициализировать объект
bool LiveWatcher::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool LiveWatcher::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Работа детектора
void _cdecl LiveWatcher::Work(float dltTime, long level)
{
	if(!object.Validate())
	{
		LogicDebugError("Lost object pointer, detector is off");
		object.Reset();
		Activate(false);
		return;
	}
	float hp = object.Ptr()->GetHP();
	float maxhp = object.Ptr()->GetMaxHP();
	if(maxhp > 0.0f)
	{
		float level = hp/maxhp;
		if(level < triggerHP)
		{
			LogicDebug("Triggering with object HP = %f", level*100.0f);
			trigger.Activate(Mission(), false);
			Activate(false);
			return;
		}
	}else{
		LogicDebugError("Invalidate object (\"%s\") MaxHP (%f<=0), detector is off", object.Ptr()->GetObjectID().c_str(), maxhp);
		object.Reset();
		Activate(false);
		return;
	}
}

MOP_BEGINLISTCG(LiveWatcher, "Live watcher", '1.00', 0x0fffffff, "This detector watching object HP and triggering when HP down more then setting", "Logic")
	MOP_STRING("Object id", "")
	MOP_FLOATEXC("HP", 50.0f, 0.0f, 100.0f, "Relative HP value in percents when decector triggering")
	MOP_MISSIONTRIGGERG("Trigger", "")
	MOP_BOOL("Active", true)
MOP_ENDLIST(LiveWatcher)

