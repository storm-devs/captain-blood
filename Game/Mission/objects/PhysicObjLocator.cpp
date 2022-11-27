//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Mission objects
//============================================================================================
// PhysicObjLocator
//============================================================================================

#include "PhysicObjLocator.h"

//============================================================================================

PhysicObjLocator::PhysicObjLocator()
{
	locatorIndex = -1;
	error = false;
}

PhysicObjLocator::~PhysicObjLocator()
{
}

//============================================================================================

//Инициализировать объект
bool PhysicObjLocator::Create(MOPReader & reader)
{
	if(!MissionObject::Create(reader)) return false;
	//Освобождаем старые ресурсы
	po.Reset();
	locatorIndex = -1;
	//Параметры
	objectName = reader.String();
	locatorName = reader.String();
	error = false;
	return true;
}

//Инициализировать объект
bool PhysicObjLocator::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&PhysicObjLocator::EditMode_Draw, ML_ALPHA1);
	Create(reader);
	return true;
}

//Обновить параметры
bool PhysicObjLocator::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void PhysicObjLocator::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -0.2f;
	max = 0.2f;
}

//Получить матрицу объекта
Matrix & PhysicObjLocator::GetMatrix(Matrix & mtx)
{
	//Проверяем на валидность геометрию
	if(!po.Validate() && !error)
	{
		bool isPtr = FindObject(objectName, po);
		if(isPtr)
		{
			MO_IS_IF_NOT(id_IMissionPhysObject, "IMissionPhysObject", po.Ptr())
			{
				isPtr = false;
			}
		}
		if(!isPtr)
		{
			po.Reset();
			if(!EditMode_IsOn() && !error)
			{
				error = true;
				LogicDebugError("Physic object \"%s\" not found", objectName.c_str());
				return mtx;
			}
		}
	}
	if(locatorIndex < 0 || EditMode_IsOn())
	{
		if(locatorName.NotEmpty() && po.Validate())
		{
			locatorIndex = ((IMissionPhysObject *)po.Ptr())->GetLocatorIndexByName(locatorName);
			if(locatorIndex < 0)
			{
				if(EditMode_IsOn())
				{
					locatorName.Empty();
					error = true;
					LogicDebugError("Physic object \"%s\" not content locator \"%s\" at pattern", objectName.c_str(), locatorName.c_str());				
				}
				return mtx;				
			}
		}else{
			return mtx;
		}
	}
	bool res = ((IMissionPhysObject *)po.Ptr())->GetLocator(locatorIndex, mtx);
	Assert(res);
	return mtx;
};

//Нарисовать модельку
void _cdecl PhysicObjLocator::EditMode_Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if(!EditMode_IsSelect()) return;
	Matrix matrix;
	matrix.SetZero();
	GetMatrix(matrix);
	Render().DrawMatrix(matrix);
	Render().DrawSphere(matrix.pos, 0.01f, 0x90ffff80);
	Render().Print(matrix.pos, 5.0f, 1.0f, 0xffffff80, "Physic locator");
	Render().Print(matrix.pos, 5.0f, 2.0f, 0xffffff80, "\"%s\"", GetObjectID().c_str());
};

//============================================================================================
//Параметры инициализации
//============================================================================================


MOP_BEGINLISTCG(PhysicObjLocator, "Physic locator", '1.00', 100, "Object make access to Physic object locator", "Physics")
	MOP_STRINGC("Physic object", "", "Mission object's name type of \"Physic object\"")
	MOP_STRINGC("Locator", "", "Locator name defined in \"Physic object pattern\"")
MOP_ENDLIST(PhysicObjLocator)

