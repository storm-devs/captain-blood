//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Light	
//============================================================================================
			

#include "Light.h"

//============================================================================================

Light::Light()
{
	model = null;
}

Light::~Light()
{
	if(model)
	{
		model->Release();
		model = null;
	}	
}

//============================================================================================
	
//Инициализировать объект
bool Light::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&Light::EditModeDraw, ML_GEOMETRY1);
	return true;
}

//Обновить параметры
bool Light::EditMode_Update(MOPReader & reader)
{
	return EditMode_Create(reader);
}

//Получить размеры описывающего ящика
void Light::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(model)
	{
		min = model->GetLocalBound().vMin;
		max = model->GetLocalBound().vMax;
		return;
	}
	min = -0.3f;
	max = 0.3f;
}

//Создать модельку
void Light::CreateModel(const char * name)
{
	IGMXScene * m = Geometry().CreateGMX(name, &Animation(), &Particles(), &Sound());
	if(model)
	{
		model->Release();
		model = null;
	}
	model = m;
}

//Рисование модельки в режиме редактирования
void _cdecl Light::EditModeDraw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	Matrix mtx;
	/*
	if(model)
	{
		Color c = color;
		c.Normalize();
		parameter->SetDword(0, c.GetDword());
		Render().SetEffect("LightColorPreveiwEffect", parameter);
		model->SetTransform(GetMatrix(mtx));
		model->Draw();
		Render().SetEffect();
	}else*/
	Render().DrawSphere(GetMatrix(mtx).pos, 0.1f, color.GetDword());
}

