//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// RectDetector	
//============================================================================================
			

#include "RectDetector.h"

//============================================================================================

RectDetector::RectDetector()
{
	Show(false);
	isDown = false;	
	ignoreDistance = 5.0f;
}

RectDetector::~RectDetector()
{
}


//============================================================================================


//Инициализировать объект
bool RectDetector::Create(MOPReader & reader)
{
	isDown = false;
	target.Reset();
	ignoreDistance = 5.0f;
	if(!DetectorObject::Create(reader)) return false;	
	//Получим идентификатор объекта
	targetID = reader.String();
	//Получим позицию
	Vector pos = reader.Position();
	//Получим направление
	Vector ang = reader.Angles();
	mtx.Build(ang, pos);
	//Плоскость
	plane.normal = mtx.vz;
	plane.Move(pos);
	//Матрица преобразования
	imtx = mtx;
	imtx.Inverse();
	//Получим размеры
	width05 = reader.Float()*0.5f;
	height05 = reader.Float()*0.5f;
	//Имя отсылаемого события
	event.Init(reader);
	//Состояние
	Activate(reader.Bool());
	//Сбрасывать ли состояние после срабатывания
	autoReset = reader.Bool();
	//Одно или двух сторонний
	isDoubleSide = reader.Bool();
	//
	ignoreDistance = reader.Float();
	//Ищим объект среди существующих
	return UpdateTarget();
}

//Получить матрицу объекта
Matrix & RectDetector::GetMatrix(Matrix & mtx)
{
	return (mtx = this->mtx);
}

//Инициализировать объект
bool RectDetector::EditMode_Create(MOPReader & reader)
{
	isDown = false;
	if(!DetectorObject::Create(reader)) return false;
	SetUpdate(&RectDetector::EditModeWork, ML_ALPHA5);
	Create(reader);
	highlight = 0.0f;
	resetTime = 0.0f;
	return true;
}

//Обновить параметры
bool RectDetector::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void RectDetector::EditMode_GetSelectBox(Vector & min, Vector & max)
{	
	min = Vector(-width05 - 0.1f, -height05 - 0.1f, -0.1f);
	max = Vector(width05 + 0.1f, height05 + 0.1f, 0.1f);
}

//Активация детектора
void RectDetector::ActivateDetector(const char * initiatorID)
{
	if(!EditMode_IsOn())
	{
		LogicDebug("Triggering");		
		event.Activate(Mission(), false);
	}
}

//Активировать
void RectDetector::Activate(bool isActive)
{
	DetectorObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		SetUpdate(&RectDetector::Work, ML_TRIGGERS);
	}else{
		LogicDebug("Deactivate");
		DelUpdate(&RectDetector::Work);
	}
}

//============================================================================================

//Работа детектора
void _cdecl RectDetector::Work(float dltTime, long level)
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
	//Анализируем
	Matrix mtx;
	target.Ptr()->GetMatrix(mtx);
	//Проверяем пересечение плоскости
	Vector res;
	isDown = false;
	if(plane.Intersection(targetPos, mtx.pos, res))
	{
		if(double(~(targetPos - mtx.pos)) < double(ignoreDistance)*ignoreDistance)
		{
			//Произошло пересечение плоскости
			res = imtx*res;
			if(-width05 <= res.x && res.x <= width05)
			{
				if(-height05 <= res.y && res.y <= height05)
				{
					isDown = true;
					if(!isDoubleSide)
					{
						if(((mtx.pos - targetPos) | plane.n) > 0.0f) isDown = false;
					}
				}
			}
		}
	}
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

//Нарисовать детектор
void _cdecl RectDetector::EditModeWork(float dltTime, long level)
{
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(!EditMode_IsVisible()) return;

	//Структура вершины
#pragma pack(push, 1)
	struct Vertex{ Vector p; dword c; };
#pragma pack(pop)
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
	//Выбираем цвета
	Color front, back;
	if(target.Validate())
	{
		if(IsActive())
		{
			if(!isDown)
			{
				front = Color(0.0f, 0.8f, 0.0f, 0.8f);
				back = Color(0.0f, 0.4f, 0.0f, 0.8f);
			}else{
				front = Color(0.8f, 0.0f, 0.0f, 0.8f);
				back = Color(0.4f, 0.0f, 0.0f, 0.8f);
			}
		}else{
			front = Color(0.8f, 0.8f, 0.8f, 0.5f);
			back = Color(0.4f, 0.4f, 0.4f, 0.5f);
		}
	}else{
		front = Color(1.0f, 0.0f, 1.0f, 0.8f);
		back = Color(0.6f, 0.0f, 0.6f, 0.8f);
	}
	front += highlight;
	back += highlight;
	front.Clamp();
	back.Clamp();
	dword dfront = front.GetDword();
	dword dback = back.GetDword();
	//Рисуем
	Vertex v[12];
	v[0].p.x = -width05; v[0].p.y = -height05; v[0].p.z = 0.0f; v[0].c = dfront;
	v[1].p.x = -width05; v[1].p.y = height05; v[1].p.z = 0.0f; v[1].c = dfront;
	v[2].p.x = width05; v[2].p.y = height05; v[2].p.z = 0.0f; v[2].c = dfront;
	v[3].p.x = -width05; v[3].p.y = -height05; v[3].p.z = 0.0f; v[3].c = dfront;
	v[4].p.x = width05; v[4].p.y = height05; v[4].p.z = 0.0f; v[4].c = dfront;
	v[5].p.x = width05; v[5].p.y = -height05; v[5].p.z = 0.0f; v[5].c = dfront;
	v[6].p.x = -width05; v[6].p.y = -height05; v[6].p.z = 0.0f; v[6].c = dback;
	v[7].p.x = width05; v[7].p.y = height05; v[7].p.z = 0.0f; v[7].c = dback;
	v[8].p.x = -width05; v[8].p.y = height05; v[8].p.z = 0.0f; v[8].c = dback;	
	v[9].p.x = -width05; v[9].p.y = -height05; v[9].p.z = 0.0f; v[9].c = dback;
	v[10].p.x = width05; v[10].p.y = -height05; v[10].p.z = 0.0f; v[10].c = dback;
	v[11].p.x = width05; v[11].p.y = height05; v[11].p.z = 0.0f; v[11].c = dback;	
	Render().SetWorld(mtx);

	ShaderId id;
	Render().GetShaderId("ShowDetector", id);
	Render().DrawPrimitiveUP(id, PT_TRIANGLELIST, 4, v, sizeof(Vertex));
	if(EditMode_IsSelect())
	{
		Render().Print(mtx.pos, -1.0f, -1.0f, 0xffffffff, "Object id: %s", GetObjectID().c_str());
		Render().Print(mtx.pos, -1.0f, 0.0f, 0xffffffff, "Target: %s", targetID.c_str());
		Render().Print(mtx.pos, -1.0f, 1.0f, 0xffffffff, IsActive() ? "State: on" : "State: off");
	}
}

//Обновить цель
bool RectDetector::UpdateTarget()
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

MOP_BEGINLISTCG(RectDetector, "Rect detector", '1.00', 0x0fffffff, "Detector triggering if mission object intersection it.", "Logic")
	MOP_STRING("Object id", "Player")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_FLOATEX("Width", 1.0f, 0.01f, 1000000.0f)
	MOP_FLOATEX("Height", 1.0f, 0.01f, 1000000.0f)
	MOP_MISSIONTRIGGERG("Events", "")
	MOP_BOOL("Active", true)
	MOP_BOOL("Auto reset", false)
	MOP_BOOL("Double side", true)
	MOP_FLOATEXC("Ignore distance", 5.0f, 0.1f, 1000000.0f, "If length of target path per frame more then this distance, detector ignore that moving")
MOP_ENDLIST(RectDetector)





