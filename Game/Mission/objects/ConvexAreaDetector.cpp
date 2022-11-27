//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// ConvexAreaDetector	
//============================================================================================
			

#include "ConvexAreaDetector.h"

//============================================================================================

ConvexAreaDetector::ConvexAreaDetector() : targets(_FL_)
{
	previewPoints = null;
}

ConvexAreaDetector::~ConvexAreaDetector()
{
	if(previewPoints)
	{
		delete previewPoints;
	}
}


//============================================================================================


//Инициализировать объект
bool ConvexAreaDetector::Create(MOPReader & reader)
{
	targets.DelAll();
	eventEnter.Init(reader);
	eventExit.Init(reader);
	long objectsCount = reader.Array();
	targets.AddElements(objectsCount);
	for(long i = 0; i < objectsCount; i++)
	{
		targets[i].target.Reset();
		targets[i].targetID = reader.String();
		targets[i].isInside = false;
		targets[i].eventEnter.Init(reader);
		targets[i].eventExit.Init(reader);
	}
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	matrix.Build(ang, pos);
	connectID = reader.String();
	connect.Reset();
	pCount = reader.Array();
	planesCount = 0;
	Assert(pCount <= ARRSIZE(plane));
	Vector p0, prev;
	float s = 0.0f;
	for(long i = 0; i <= pCount; i++)
	{
		Vector pnt = (i < pCount) ? reader.Position() : p0;
		if(i)
		{
			Vector edge = (pnt - prev).GetXZ();
			if(edge.NormalizeXZ() > 1e-5f)
			{
				plane[planesCount].n.x = -edge.z;
				plane[planesCount].n.y = 0.0f;
				plane[planesCount].n.z = edge.x;
				plane[planesCount].d = plane[planesCount].n | pnt;
				planesCount++;
			}
		}else{
			p0 = pnt;
		}
		if(previewPoints)
		{
			previewPoints[i*2].pos = pnt;
		}
		if(i == 2)
		{
			s = (p0.x - prev.x)*(pnt.z - prev.z) - (p0.z - prev.z)*(pnt.x - prev.x);
		}
		prev = pnt;
	}
	if(planesCount < 3)
	{
		planesCount = 0;
	}else{
		if(s < 0.0f)
		{
			for(long i = 0; i < planesCount; i++)
			{
				plane[i].n = -plane[i].n;
				plane[i].d = -plane[i].d;
			}
		}
	}
	hmin = reader.Float();
	hmax = reader.Float();
	if(hmax <= hmin)
	{
		float t = hmax;
		hmax = hmin + 1e-10f;
		hmin = t - 1e-10f;
	}
	//eventEnter.Init(reader);
	//eventExit.Init(reader);
	isCheckVisible = reader.Bool();
	isCheckActive = reader.Bool();
	thisTriggers = reader.Bool();
	//Активация
	Activate(reader.Bool());
	isDebug = reader.Bool();
	return planesCount != 0;
}

//Получить матрицу объекта
Matrix & ConvexAreaDetector::GetMatrix(Matrix & mtx)
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
		mtx = matrix*connect.Ptr()->GetMatrix(Matrix());		
	}else{
		mtx = matrix;
	}
	return mtx;
}

//Активировать
void ConvexAreaDetector::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			LogicDebug("Activate");
			SetUpdate(&ConvexAreaDetector::Work, ML_TRIGGERS);
			for(long i = 0; i < targets; i++)
			{
				targets[i].isInside = false;
			}
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&ConvexAreaDetector::Work);
			Unregistry(MG_ACTIVEACCEPTOR);
			eventExit.Activate(Mission(), false);
		}
	}
}

//Инициализировать объект
bool ConvexAreaDetector::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&ConvexAreaDetector::EditModeWork, ML_ALPHA5);
	previewPoints = NEW Vertex[(ARRSIZE(plane) + 1)*4];
	EditMode_Update(reader);
	return true;
}

//Обновить параметры
bool ConvexAreaDetector::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	DelUpdate(&ConvexAreaDetector::Work);
	//Формируем буфер для рисования тригера
	center = 0.0f;
	Vertex * backPoints = previewPoints + (pCount + 1)*2;
	float sgn = 0.0f;
	if(pCount >= 3)
	{
		sgn = (previewPoints[0].pos.x - previewPoints[2].pos.x)*(previewPoints[4].pos.z - previewPoints[2].pos.z);
		sgn -= (previewPoints[0].pos.z - previewPoints[2].pos.z)*(previewPoints[4].pos.x - previewPoints[2].pos.x);
	}
	for(long i = 0; i <= pCount; i++)
	{	
		//Считаем среднюю точку
		if(i)
		{
			center += previewPoints[i*2].pos;
		}
		//Определяем выпуклость
		long prev = (i > 0) ? (i - 1)*2 : (pCount - 1)*2;
		long next = (i < pCount) ? (i + 1)*2 : 2;		
		Vector v1 = previewPoints[prev].pos - previewPoints[i*2].pos;
		Vector v2 = previewPoints[next].pos - previewPoints[i*2].pos;
		float s = v1.x*v2.z - v1.z*v2.x;
		//Заполняем вершины
		previewPoints[i*2].pos.y = hmin;
		previewPoints[i*2].color = 0L;
		previewPoints[i*2].color.r = (s*sgn >= 0.0f) ? 0x00 : 0xff;
		previewPoints[i*2].color.a = (sgn >= 0.0f) ? 0x40 : 0xc0;
		previewPoints[i*2 + 1] = previewPoints[i*2];
		previewPoints[i*2 + 1].pos.y = hmax;
		backPoints[i*2] = previewPoints[i*2 + 1];
		backPoints[i*2].color.a = (sgn >= 0.0f) ? 0xc0 : 0x40;
		backPoints[i*2 + 1] = previewPoints[i*2];
		backPoints[i*2 + 1].color.a = backPoints[i*2].color.a;
	}
	//Центральная точка
	if(pCount)
	{
		center /= (float)pCount;
	}
	pCount = pCount*4 + 2;
	center.y = (hmin + hmax)*0.5f;
	return true;
}

//Получить размеры описывающего ящика
void ConvexAreaDetector::EditMode_GetSelectBox(Vector & min, Vector & max)
{	
	if(previewPoints)
	{
		min = max = previewPoints[0].pos;
		for(long i = 0; i < pCount + 2; i++)
		{
			min.Min(previewPoints[i].pos);
			max.Max(previewPoints[i].pos);
		}
	}else{
		min = 0.0f;
		max = 0.0f;
	}

}

//============================================================================================

//Работа детектора
void _cdecl ConvexAreaDetector::Work(float dltTime, long level)
{
	Matrix mtx, trgMtx;
	GetMatrix(mtx);
	//Анализируем
	bool currentInside = false;
	bool isAnyInside = false;
	for(long i = 0; i < targets; i++)
	{
		Target & trg = targets[i];
		currentInside |= trg.isInside;
		if(!trg.target.Validate())
		{
			if(trg.targetID.NotEmpty())
			{
				trg.isInside = false;
				FindObject(trg.targetID, trg.target);
				trg.targetID.Empty();
			}else{
				trg.target.Reset();
			}
		}
		if(!trg.target.Validate())
		{
			continue;
		}
		if(isCheckVisible)
		{
			if(!trg.target.Ptr()->IsShow())
			{
				trg.actEnter = false;
				trg.actExit = false;
				continue;
			}
		}
		if(isCheckActive)
		{
			if(!trg.target.Ptr()->IsActive())
			{
				trg.actEnter = false;
				trg.actExit = false;
				continue;
			}
		}
		Vector targetPos = mtx.MulVertexByInverse(trg.target.Ptr()->GetMatrix(trgMtx).pos);
		bool isInside = false;
		if(targetPos.y >= hmin && targetPos.y <= hmax)
		{
			for(long j = 0; j < planesCount; j++)
			{
				if((plane[j].n | targetPos) > plane[j].d)
				{
					break;
				}
			}
			isInside = (j >= planesCount);
		}
		if(trg.isInside != isInside)
		{			
			trg.actEnter = isInside;
			trg.actExit = !isInside;
			trg.isInside = isInside;
		}else{
			trg.actEnter = false;
			trg.actExit = false;
		}
		isAnyInside |= isInside;
	}
	for(long i = 0; i < targets; i++)
	{
		Target & trg = targets[i];
		if(!trg.target.Validate()) continue;
		if(trg.actEnter)
		{
			LogicDebug("Enter object \"%s\"", trg.target.Ptr()->GetObjectID().c_str());
			trg.eventEnter.Activate(Mission(), false, trg.target.Ptr());
			if(thisTriggers)
			{
				LogicDebug("Triggering enter event for *this* = \"%s\"", trg.target.Ptr()->GetObjectID().c_str());
				eventEnter.Activate(Mission(), false, trg.target.Ptr());
			}
		}
		if(trg.actExit)
		{
			LogicDebug("Exit object \"%s\"", trg.target.Ptr()->GetObjectID().c_str());
			trg.eventExit.Activate(Mission(), false, trg.target.Ptr());
			if(thisTriggers)
			{
				LogicDebug("Triggering exit event for *this* = \"%s\"", trg.target.Ptr()->GetObjectID().c_str());
				eventExit.Activate(Mission(), false, trg.target.Ptr());
			}
		}
	}
	if(!thisTriggers && currentInside != isAnyInside)
	{
		if(isAnyInside)
		{
			LogicDebug("Triggering enter event");
			eventEnter.Activate(Mission(), false);
		}else{
			LogicDebug("Triggering exit event");
			eventExit.Activate(Mission(), false);
		}
	}
}

//Работа детектора в режиме редактирования
void _cdecl ConvexAreaDetector::EditModeWork(float dltTime, long level)
{
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(!EditMode_IsVisible()) return;
	//Обновляем состояние
	bool res = true;
	for(long i = 0; i < targets; i++)
	{
		Target & trg = targets[i];
		if(!trg.target.Validate())
		{
			trg.isInside = false;
			if(!FindObject(trg.targetID, trg.target))
			{
				res = false;
			}
		}
	}
	byte g = 0;
	byte b = 0;
	if(res)
	{
		if(IsActive())
		{
			g = 0xff;
			b = 0x00;
		}else{
			g = 0x00;
			b = 0x80;
		}
	}
	Matrix mtx;
	GetMatrix(mtx);
	if(isDebug)
	{
		Vector targetPos = mtx.MulVertexByInverse(Render().GetView().GetCamPos());
		bool isInside = false;
		if(targetPos.y >= hmin && targetPos.y <= hmax)
		{
			for(long j = 0; j < planesCount; j++)
			{
				if((plane[j].n | targetPos) > plane[j].d)
				{
					break;
				}
			}
			isInside = (j >= planesCount);
		}
		if(isInside)
		{
			g = 0xff;
			b = 0xff;
		}
	}
	for(long i = 0; i < pCount + 2; i++)
	{
		previewPoints[i].color.g = g;
		previewPoints[i].color.b = b;
	}
	//Рисуем
	Render().SetWorld(mtx);

	ShaderId id;
	Render().GetShaderId("ShowDetector", id);
	Render().DrawPrimitiveUP(id, PT_TRIANGLESTRIP, pCount, previewPoints, sizeof(Vertex));
	if(EditMode_IsSelect())
	{
		Render().Print(mtx*center, -1.0f, -1.0f, 0xffffffff, "Object id: %s", GetObjectID().c_str());
		Render().Print(mtx*center, -1.0f, 1.0f, 0xffffffff, IsActive() ? "State: on" : "State: off");
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(ConvexAreaDetector, "Convex area detector", '1.00', 0x0fffffff, "Detector check area for some mission objects", "Logic")
	MOP_MISSIONTRIGGERG("Enter trigger", "Enter.")
	MOP_MISSIONTRIGGERG("Exit trigger", "Exit.")
	MOP_ARRAYBEG("Objects", 1, 1000)
		MOP_STRING("Object id", "Player")
		MOP_MISSIONTRIGGER("Enter.")
		MOP_MISSIONTRIGGER("Exit.")
	MOP_ARRAYEND
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angle", Vector(0.0f))
	MOP_STRING("Connect to object", "")
	MOP_ARRAYBEG("Points", 3, 16)
		MOP_POSITION("Point", Vector(0.0f))
	MOP_ARRAYEND
	MOP_FLOAT("Height min", -2.0f)
	MOP_FLOAT("Height max", 5.0f)
	MOP_BOOL("Check visible flag", false)
	MOP_BOOL("Check active flag", false)
	MOP_BOOLC("This triggers", false, "Activate enter/exit trigger for any object with *this* param,\nor once for any enter and once when all exit")
	MOP_BOOL("Active", true)
	MOP_BOOLC("Debug", false, "Check detector in editor with camera position")
MOP_ENDLIST(ConvexAreaDetector)


