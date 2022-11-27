//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// SphereDetector	
//============================================================================================
			

#include "SphereDetector.h"

//============================================================================================

SphereDetector::SphereDetector()
{
}

SphereDetector::~SphereDetector()
{
}


//============================================================================================


//Инициализировать объект
bool SphereDetector::Create(MOPReader & reader)
{
	target.Reset();
	isDown = false;
	if(!DetectorObject::Create(reader)) return false;
	//Получим идентификатор объекта
	targetID = reader.String();
	//Получить позицию
	iniPos = sphere.pos = reader.Position();
	//Получить радиус
	sphere.radius = reader.Float();
	//Имя отсылаемого события
	event.Init(reader);
	//Активация
	Activate(reader.Bool());
	//Сбрасывать ли состояние после срабатывания
	autoReset = reader.Bool();
	//Объект перемещения цели
	connectID = reader.String();
	FindObject(connectID, connect);
	if(connect.Validate())
	{
		sphere.pos = connect.Ptr()->GetMatrix(Matrix()).pos;
	}
	return UpdateTarget();
}

//Получить матрицу объекта
Matrix & SphereDetector::GetMatrix(Matrix & mtx)
{
	return mtx.BuildPosition(sphere.pos);
}

//Инициализировать объект
bool SphereDetector::EditMode_Create(MOPReader & reader)
{
	isDown = false;
	if(!DetectorObject::Create(reader)) return false;
	SetUpdate(&SphereDetector::EditModeWork, ML_ALPHA5);
	Create(reader);
	highlight = 0.0f;
	resetTime = 0.0f;
	return true;
}

//Обновить параметры
bool SphereDetector::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void SphereDetector::EditMode_GetSelectBox(Vector & min, Vector & max)
{	
	min = -sphere.radius - 0.1f;
	max = sphere.radius + 0.1f;
}

//Активация детектора
void SphereDetector::ActivateDetector(const char * initiatorID)
{
	if(!EditMode_IsOn())
	{
		LogicDebug("Triggering");
		event.Activate(Mission(), false);
	}
}

//Активировать
void SphereDetector::Activate(bool isActive)
{
	DetectorObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		SetUpdate(&SphereDetector::Work, ML_TRIGGERS);
	}else{
		LogicDebug("Deactivate");
		DelUpdate(&SphereDetector::Work);
	}
}

//============================================================================================

//Работа детектора
void _cdecl SphereDetector::Work(float dltTime, long level)
{
	if(!target.Validate())
	{
		target.Reset();
		return;
	}
	//Отработали и не сами себя востанавливаем - неработаем
	if(isDown && !autoReset) return;
	//Не активны - не работаем
	if(!IsActive()) return;	
	//Обновляем позицию
	if(connect.Validate())
	{
		sphere.pos = connect.Ptr()->GetMatrix(Matrix()).pos;
	}else{
		sphere.pos = iniPos;
		connect.Reset();
	}
	//Анализируем
	Matrix mtx;
	target.Ptr()->GetMatrix(mtx);
	//Проверяем пересечение сферы с отрезком
	//isDown = sphere.Intersection(targetPos, mtx.pos);
	isDown = sphere.Intersection(mtx.pos);
	targetPos = mtx.pos;
	if(isDown)
	{
		if(!EditMode_IsOn())
		{
			ActivateDetector(target.Ptr()->GetObjectID().c_str());
		}else{
			highlight = 1.0f;
			resetTime = 4.0f;
		}
	}
}

//Работа детектора в режиме редактирования
void _cdecl SphereDetector::EditModeWork(float dltTime, long level)
{
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(!EditMode_IsVisible()) return;
	//Обновляем позицию
	if(!connect.Validate())
	{
		if(connectID.NotEmpty())
		{
			FindObject(connectID, connect);
		}
		if(connect.Validate())
		{
			sphere.pos = connect.Ptr()->GetMatrix(Matrix()).pos;
		}else{
			sphere.pos = iniPos;
			connect.Reset();
		}
	}
	//Работа детектора
	Work(dltTime, level);
	//Обновляем состояние
	highlight -= dltTime;
	if(highlight < 0.0f) highlight = 0.0f;
	if(isDown)
	{
		resetTime -= dltTime;
		if(resetTime <= 0.0f)
		{
			isDown = false;
			resetTime = 0.0f;
		}
	}
	//Проверяем наличие объекта
	if(!target.Validate())
	{
		UpdateTarget();
	}
	//Выбираем цвет
	Color clr;
	if(target.Validate())
	{
		if(IsActive())
		{
			if(!isDown)
			{
				clr = Color(0.0f, 0.8f, 0.0f, 0.8f);
			}else{
				clr = Color(0.8f, 0.0f, 0.0f, 0.8f);
			}
		}else{
			clr = Color(0.8f, 0.8f, 0.8f, 0.5f);
		}
	}else{
		clr = Color(1.0f, 0.0f, 1.0f, 0.8f);
	}
	clr += highlight;	
	clr.Clamp();
	dword dclr = clr.GetDword();
	//Рисуем
	Render().DrawSphere(sphere.pos, sphere.radius,  dclr, "ShowDetector");
	if(EditMode_IsSelect())
	{
		Render().Print(sphere.pos, -1.0f, -1.0f, 0xffffffff, "Object id: %s", GetObjectID().c_str());
		Render().Print(sphere.pos, -1.0f, 0.0f, 0xffffffff, "Target: %s", targetID.c_str());
		Render().Print(sphere.pos, -1.0f, 1.0f, 0xffffffff, IsActive() ? "State: on" : "State: off");
	}
}

//Обновить цель
bool SphereDetector::UpdateTarget()
{
	if(!FindObject(targetID, target)) return false;
	Matrix mtx(true);
	target.Ptr()->GetMatrix(mtx);
	targetPos = mtx.pos;
	return true;
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(SphereDetector, "Sphere detector", '1.00', 0x0fffffff, "Detector triggering if mission object enter or leave in sphere.", "Logic")
	MOP_STRING("Object id", "Player")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_FLOATEX("Radius", 1.0f, 0.001f, 1000000.0f)
	MOP_MISSIONTRIGGERG("Events", "")
	MOP_BOOL("Active", true)
	MOP_BOOL("Auto reset", false)
	MOP_STRING("Connect to object", "")
MOP_ENDLIST(SphereDetector)


