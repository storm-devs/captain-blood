//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// ObjectsGroup	
//============================================================================================
			

#include "ObjectsGroup.h"

//============================================================================================

ObjectsGroup::ObjectsGroup() : objects(_FL_)
{
	processActivate = false;
	processShow = false;
	processCommand = false;
	processRestart = false;
}

ObjectsGroup::~ObjectsGroup()
{
}


//============================================================================================


//Инициализировать объект
bool ObjectsGroup::Create(MOPReader & reader)
{
	objects.DelAll();
	long counter = reader.Array();
	for(long i = 0; i < counter; i++)
	{
		ObjectInfo & info = objects[objects.Add()];
		info.object.Reset();
		info.id = reader.String();
		info.changeShow = reader.Bool();
		info.changeActive = reader.Bool();
		info.recivingCommand = reader.Bool();
	}
	processActivate = false;
	processShow = false;
	processCommand = false;
	processRestart = false;
	return true;
}

//Показать/скрыть объект
void ObjectsGroup::Show(bool isShow)
{
	if(processShow)
	{
		LogicDebugError("Group already in show process! Critical error!");
		return;
	}
	processShow = true;
	UpdateObjectsList();
	for(long i = 0; i < objects; i++)
	{
		ObjectInfo & info = objects[i];
		if(info.object.Validate())
		{
			if(info.changeShow) info.object.Ptr()->Show(isShow);
		}		
	}
	processShow = false;
}

//Активировать/деактивировать объект
void ObjectsGroup::Activate(bool isActive)
{
	if(processActivate)
	{
		LogicDebugError("Group already in activate process! Critical error!");
		return;
	}
	if(isActive)
	{
		LogicDebug("Activate");
	}else{
		LogicDebug("Deactivate");
	}
	processActivate = true;
	LogicDebugLevel(true);
	UpdateObjectsList();
	for(long i = 0; i < objects; i++)
	{
		ObjectInfo & info = objects[i];
		if(info.object.Validate())
		{
			if(info.changeActive) info.object.Ptr()->Activate(isActive);
		}		
	}
	LogicDebugLevel(false);
	processActivate = false;
}

//Распространить команду
void ObjectsGroup::Command(const char * id, dword numParams, const char ** params)
{
	if(processCommand)
	{
		LogicDebugError("Group already in command process! Critical error!");
		return;
	}
	processCommand = true;
	UpdateObjectsList();
	for(long i = 0; i < objects; i++)
	{
		ObjectInfo & info = objects[i];
		if(info.object.Validate())
		{
			if(info.recivingCommand) info.object.Ptr()->Command(id, numParams, params);
		}		
	}
	processCommand = false;
}

//Пересоздать объект
void ObjectsGroup::Restart()
{
	if(processRestart)
	{
		LogicDebugError("Group already in restart process! Critical error!");
		return;
	}
	processRestart = true;
	UpdateObjectsList();
	for(long i = 0; i < objects; i++)
	{
		ObjectInfo & info = objects[i];
		if(info.object.Validate())
		{
			info.object.Ptr()->Restart();
		}		
	}
	processRestart = false;
}

//Инициализировать объект
bool ObjectsGroup::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обновить параметры
bool ObjectsGroup::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обновить список объектов
void ObjectsGroup::UpdateObjectsList()
{
	for(long i = 0; i < objects; i++)
	{
		ObjectInfo & info = objects[i];
		if(!info.object.Validate())
		{
			if(info.id.IsEmpty()) continue;			
			FindObject(info.id, info.object);
		}else{
			if(!info.object.Validate())
			{
				info.object.Reset();
			}
		}
	}
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(ObjectsGroup, "Objects group", '1.00', 0, "Retranslate events to group mission objects", "Logic")
	MOP_ARRAYBEG("Objects", 0, 1000)
		MOP_STRING("Object id", "")
		MOP_BOOL("Change visible", true)
		MOP_BOOL("Change active", true)
		MOP_BOOL("Reciving command", true)
	MOP_ARRAYEND
MOP_ENDLIST(ObjectsGroup)





