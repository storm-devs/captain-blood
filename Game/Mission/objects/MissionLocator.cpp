//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Mission objects
//============================================================================================
// MissionLocator
//============================================================================================

#include "MissionLocator.h"

//============================================================================================

MissionLocator::MissionLocator()
{
	sound = null;
}

MissionLocator::~MissionLocator()
{
	ReleaseSound();
}

//============================================================================================

//Инициализировать объект
bool MissionLocator::Create(MOPReader & reader)
{
	objectPtr.Reset();
	if(!MissionObject::Create(reader)) return false;
	//Позиция
	Vector pos = reader.Position();
	Vector ang = reader.Angles();	
	matrix.Build(ang, pos);
	connectToObject = reader.String();
	if(reader.Bool())
	{
		SetUpdate(&MissionLocator::Draw, ML_ALPHA1);
	}
	return true;
}

//Инициализировать объект
bool MissionLocator::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&MissionLocator::EditMode_Draw, ML_ALPHA1);
	Create(reader);
	return true;
}

//Обновить параметры
bool MissionLocator::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void MissionLocator::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -0.5f;
	max = 0.5f;
}

//Получить матрицу объекта
Matrix & MissionLocator::GetMatrix(Matrix & mtx)
{
	if(connectToObject.NotEmpty())
	{
		if(objectPtr.Validate())
		{
			mtx = matrix*objectPtr.Ptr()->GetMatrix(Matrix());
			return mtx;
		}else{
			if(FindObject(connectToObject, objectPtr))
			{
				mtx = matrix*objectPtr.Ptr()->GetMatrix(Matrix());
				return mtx;
			}
		}
	}
	return (mtx = matrix);
};

//Нарисовать модельку
void _cdecl MissionLocator::Draw(float dltTime, long level)
{
	Matrix mtx;
	GetMatrix(mtx);
	Render().DrawMatrix(mtx);
	Render().DrawSphere(mtx.pos, 0.1f, 0x90ffff80);
	Render().Print(mtx.pos, 5.0f, 1.0f, 0xffffff80, "Mission locator");
	Render().Print(mtx.pos, 5.0f, 2.0f, 0xffffff80, "\"%s\"", GetObjectID().c_str());
};

//Нарисовать модельку
void _cdecl MissionLocator::EditMode_Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	Draw(dltTime, level);
};

//Пересоздать объект
void MissionLocator::Restart()
{
	ReleaseSound();
	DelUpdate(&MissionLocator::UpdateSound);
}

//Обработчик команд для объекта
void MissionLocator::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEqual(id, "RedirectSoundEvent"))
	{
		LogicDebug("RedirectSoundEvent");
		ReleaseSound();
		if(numParams > 0 && string::NotEmpty(params[0]))
		{
			Matrix mtx;
			GetMatrix(mtx);
			sound = Sound().Create3D(params[0], mtx.pos, _FL_, true, false);
			SetUpdate(&MissionLocator::UpdateSound, ML_ALPHA5);
		}else{
			LogicDebugError("Error sound name: %s", id);
		}
	}else{
		LogicDebugError("Unknown command: \"%s\"", numParams > 0 ? params[0] : "<no command params>");
	}
}

//Обновить позицию и состояние звука
void _cdecl MissionLocator::UpdateSound(float dltTime, long level)
{
	if(sound)
	{
		if(sound->IsPlay())
		{
			if(objectPtr.Validate())
			{
				Matrix mtx;
				GetMatrix(mtx);
				sound->SetPosition(mtx.pos);
			}
			return;
		}
	}
	ReleaseSound();
	DelUpdate(&MissionLocator::UpdateSound);
}

//Удалить звук
void MissionLocator::ReleaseSound()
{
	if(sound)
	{
		sound->Release();
		sound = null;
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MissionLocator, "Mission locator", '1.00', 0x0fffffff, "Editable locator", "Geometry")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_STRING("Connect to object", "")
	MOP_BOOL("Debug draw", false)
MOP_ENDLIST(MissionLocator)

