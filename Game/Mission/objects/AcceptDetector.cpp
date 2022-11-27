//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// AcceptDetector	
//============================================================================================
			

#include "AcceptDetector.h"

//============================================================================================

AcceptDetector::AcceptDetector()
{
	isTrgInside = false;
	isSelected = false;	
	acceptEnable = true;
}

AcceptDetector::~AcceptDetector()
{
}


//============================================================================================


//Инициализировать объект
bool AcceptDetector::Create(MOPReader & reader)
{
	target.Reset();
	isTrgInside = false;
	isSelected = false;	
	acceptEnable = true;
	connect.Reset();
	if(!AcceptorObject::Create(reader)) return false;
	//Получим идентификатор объекта
	targetID = reader.String();
	//Получить позицию
	iniPos = sphere.pos = reader.Position();
	//Получить радиус
	sphere.radius = reader.Float();
	//Активация
	Activate(reader.Bool());
	acceptEnable = reader.Bool();
	checkDirection = reader.Bool();
	//Триггеры
	eventAccept.Init(reader);
	eventEnter.Init(reader);
	eventExit.Init(reader);	
	connectID = reader.String();
	UpdatePosition();
	//Сбрасывать ли состояние после срабатывания
	return UpdateTarget();
}

//Получить матрицу объекта
Matrix & AcceptDetector::GetMatrix(Matrix & mtx)
{
	return mtx.BuildPosition(sphere.pos);
}

//Активировать
void AcceptDetector::Activate(bool isActive)
{
	AcceptorObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			LogicDebug("Activate");
			SetUpdate(&AcceptDetector::Work, ML_TRIGGERS);
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&AcceptDetector::Work);
			Unregistry(MG_ACTIVEACCEPTOR);
/*			if(isTrgInside)
			{
				eventExit.Activate(Mission(), false);
				isTrgInside = false;
			}*/
			isSelected = false;
		}
	}
}

//Воздействовать на объект
bool AcceptDetector::Accept(MissionObject * obj)
{
	if(!target.Validate()) return false;
	if(obj != target.Ptr()) return false;
	LogicDebug("Triggering");
	eventAccept.Activate(Mission(), false);
	return false;
}

//Инициализировать объект
bool AcceptDetector::EditMode_Create(MOPReader & reader)
{
	if(!AcceptorObject::Create(reader)) return false;
	EditMode_Update(reader);
	return true;
}

//Обновить параметры
bool AcceptDetector::EditMode_Update(MOPReader & reader)
{
	SetUpdate(&AcceptDetector::EditModeWork, ML_ALPHA5);
	Create(reader);
	DelUpdate(&AcceptDetector::Work);
	return true;
}

//Получить размеры описывающего ящика
void AcceptDetector::EditMode_GetSelectBox(Vector & min, Vector & max)
{	
	min = -sphere.radius - 0.1f;
	max = sphere.radius + 0.1f;
}

//============================================================================================

//Работа детектора
void _cdecl AcceptDetector::Work(float dltTime, long level)
{
	if(!target.Validate())
	{
		isSelected = false;
		Unregistry(MG_ACTIVEACCEPTOR);
		target.Reset();
		return;
	}
	//Обновляем позицию
	UpdatePosition();
	//Анализируем
	Matrix targetTransform;
	target.Ptr()->GetMatrix(targetTransform);
	//Проверяем пересечение сферы с отрезком
	Vector dv = sphere.pos - targetTransform.pos;
	bool isInside = (~dv <= sphere.r*sphere.r);
	if(isTrgInside != isInside)
	{
		isTrgInside = isInside;
		if(isTrgInside)
		{			
			eventEnter.Activate(Mission(), false);
		}else{			
			eventExit.Activate(Mission(), false);
		}
	}
	if(acceptEnable)
	{
		if(checkDirection && (targetTransform.vz | dv) < 0.0f)
		{
			isInside = false;
		}
		if(isSelected != isInside)
		{
			isSelected = isInside;
			if(isSelected)
			{
				Registry(MG_ACTIVEACCEPTOR);
			}else{
				Unregistry(MG_ACTIVEACCEPTOR);
			}
		}
	}
}

//Работа детектора в режиме редактирования
void _cdecl AcceptDetector::EditModeWork(float dltTime, long level)
{
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(!EditMode_IsVisible()) return;
	//Обновляем состояние
	dword clr;
	UpdateTarget();
	if(target.Validate())
	{
		if(IsActive())
		{
			clr = 0xff00ffcc;
		}else{
			clr = 0xcccccc80;
		}
	}else{
		clr = 0xffff508c;
	}
	UpdatePosition();
	//Рисуем
	Render().DrawSphere(sphere.pos, sphere.radius,  clr, "ShowDetector");
	if(EditMode_IsSelect())
	{
		Render().Print(sphere.pos, -1.0f, -1.0f, 0xffffffff, "Object id: %s", GetObjectID().c_str());
		Render().Print(sphere.pos, -1.0f, 0.0f, 0xffffffff, "Target: %s", targetID.c_str());
		Render().Print(sphere.pos, -1.0f, 1.0f, 0xffffffff, IsActive() ? "State: on" : "State: off");
	}
}

//Обновить цель
bool AcceptDetector::UpdateTarget()
{
	if(!FindObject(targetID, target)) return false;
	Matrix mtx(true);
	target.Ptr()->GetMatrix(mtx);
	targetPos = mtx.pos;
	return true;
}

//Обновить позицию
void AcceptDetector::UpdatePosition()
{
	if(connectID.NotEmpty())
	{
		if(!connect.Validate())
		{
			FindObject(connectID, connect);
		}
	}
	if(connect.Validate())
	{
		sphere.pos = connect.Ptr()->GetMatrix(Matrix())*iniPos;
	}else{
		sphere.pos = iniPos;
	}
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(AcceptDetector, "Accept detector", '1.00', 0x0fffffff, "Detector accept action from some mission object", "Logic")
	MOP_STRING("Object id", "Player")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_FLOATEX("Radius", 1.0f, 0.001f, 1000000.0f)
	MOP_BOOL("Active", true)
	MOP_BOOLC("Accept", true, "Enable accept perception")
	MOP_BOOLC("Direction", false, "Check direction, when accept")
	MOP_MISSIONTRIGGERG("Accept trigger", "Accept.")
	MOP_MISSIONTRIGGERG("Enter trigger", "Enter.")
	MOP_MISSIONTRIGGERG("Exit trigger", "Exit.")
	MOP_STRING("Connect to object", "")
MOP_ENDLIST(AcceptDetector)


