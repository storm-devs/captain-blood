//============================================================================================
// Mission objects
//============================================================================================
// AmbientLight
//============================================================================================

#include "AmbientLight.h"

//============================================================================================

AmbientLight::AmbientLight()
{
	fGamma = 0.0f;
}

AmbientLight::~AmbientLight()
{
}

//============================================================================================

//Инициализировать объект
bool AmbientLight::Create(MOPReader & reader)
{
	if(!MissionObject::Create(reader)) return false;
	//Позиция
	Vector pos = reader.Position();
	Vector ang = reader.Angles();	
	matrix.Build(ang, pos);

	vAmbientColor = reader.Colors();

	vAmbientColorSpecular = reader.Colors();
	vAmbientColorSpecular.v4.v = vAmbientColorSpecular.v4.v * (vAmbientColorSpecular.v4.w * 16.0f);
	vAmbientColorSpecular.a = reader.Float();

	fGamma = reader.Float();

	if (fGamma < 0.0f)
	{
		fGamma = 1.0f - (0.08f * fGamma);
	} else
	{
		fGamma = 1.0f + fGamma;
	}


	Activate (reader.Bool());
	return true;
}

//Инициализировать объект
bool AmbientLight::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&AmbientLight::EditMode_Draw, ML_ALPHA1);
	Create(reader);
	return true;
}

//Обновить параметры
bool AmbientLight::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void AmbientLight::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -0.5f;
	max = 0.5f;
}

//Получить матрицу объекта
Matrix & AmbientLight::GetMatrix(Matrix & mtx)
{
	return (mtx = matrix);
};

//Нарисовать модельку
void _cdecl AmbientLight::EditMode_Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	Render().DrawMatrix(matrix);
	Render().DrawSphere(matrix.pos, 0.1f, 0x90ffff80);
	Render().Print(matrix.pos, 5.0f, 1.0f, 0xffffff80, "Mission locator");
	Render().Print(matrix.pos, 5.0f, 2.0f, 0xffffff80, "\"%s\"", GetObjectID().c_str());
};

void AmbientLight::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (IsActive())
	{
		LogicDebug("Activate");
		Console().Trace(COL_ALL, "Ambient Light '%s' enabled", GetObjectID().c_str());		
		Registry(ACTIVATE_EVENT_GROUP, &AmbientLight::ActivateEvent, 0);
		SetUpdate(&AmbientLight::SetLight, ML_LIGHTS_ON);
	} else {
		LogicDebug("Deactivate");
		Console().Trace(COL_ALL, "Ambient Light '%s' disabled", GetObjectID().c_str());
		Unregistry(ACTIVATE_EVENT_GROUP);
		DelUpdate(&AmbientLight::SetLight);
	}	

}

//Установка источника в основном цикле
void _cdecl AmbientLight::SetLight(float dltTime, long level)
{
	ActivateEvent(null, null);
}

//Выполнить событие активации
void _cdecl AmbientLight::ActivateEvent(const char * group, MissionObject * sender)
{
	Render().SetAmbient(vAmbientColor);
	Render().SetAmbientSpecular(vAmbientColorSpecular);
	Render().SetAmbientGamma(fGamma);
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(AmbientLight, "Ambinet Light", '1.00', 0, "Ambinet Light", "Lighting")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_COLOR("Color", Color(0.1f, 0.1f, 0.1f));
	MOP_COLOR("Specular", Color(1.0f, 1.0f, 1.0f, 1.0f / 16.0f));
	MOP_FLOATEX("Specular Shininess", 32.0f, 1.01f, 256.0f);

	MOP_FLOATEX("Gamma", 0.0f, -10.0f, 10.0f);
	MOP_BOOL("Active", true);
MOP_ENDLIST(AmbientLight)

