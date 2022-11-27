//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// DamageDetector	
//============================================================================================
			

#include "DamageDetector.h"

#define MissionDamageDetectorRadius		0.2f

//============================================================================================

DamageDetector::DamageDetector()
{
	immuneCount = 0;
	immuneTimerIsActive = false;
}

DamageDetector::~DamageDetector()
{
}


//============================================================================================


//Инициализировать объект
bool DamageDetector::Create(MOPReader & reader)
{
	immuneCount = 0;
	immuneTimerIsActive = false;
	if(!DamageObject::Create(reader)) return false;
	//Получим идентификатор объекта
	SetTarget(reader.String());
	//Получить позицию и размеры
	detector.pos = reader.Position();
	detector.r = reader.Float();
	Matrix mtx;
	mtx.pos = detector.pos;
	Assert(finder);
	finder->SetMatrix(mtx);
	finder->SetBoxSize(Vector(detector.r*2.0f));
	//Количество жизней у детектора
	InitHP(reader.Float());
	//От кого принимать демедж
	multipliers.Init(reader);
	//Активация
	Activate(reader.Bool());
	autoReset = reader.Bool();
	registerEventEverytime = reader.Bool();
	collide = reader.Bool();
	immuneTime = reader.Float();
	//Триггеры
	eventDamage.Init(reader);
	eventActivate.Init(reader);
	return true;
}

//Инициализировать объект
bool DamageDetector::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&DamageDetector::EditModeWork, ML_ALPHA5);
	EditMode_Update(reader);
	return true;
}

//Обновить параметры
bool DamageDetector::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void DamageDetector::EditMode_GetSelectBox(Vector & min, Vector & max)
{	
	min = -detector.r - 0.1f;
	max = detector.r + 0.1f;
}

//============================================================================================

//Воздействовать на объект сферой
bool DamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius)
{
	//Отсекаем воздействие
	if(!IsCanAttack(obj, source, hp))
	{
		return false;
	}
	//Проверяем геометрию
	float d2 = ~(center - detector.pos);
	float r = radius + detector.radius;
	if(d2 > r*r)
	{
		return false;
	}
	//Поподание
	return HitProcess(source, obj, hp);
}

//Воздействовать на объект линией
bool DamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to)
{
	//Отсекаем воздействие
	if(!IsCanAttack(obj, source, hp))
	{
		return false;
	}
	//Проверяем геометрию	
	if(!detector.Intersection(from, to))
	{
		return false;
	}
	//Поподание
	return HitProcess(source, obj, hp);
}

//Воздействовать на объект выпуклым чехырёхугольником
bool DamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4])
{
	//Отсекаем воздействие
	if(!IsCanAttack(obj, source, hp))
	{
		return false;
	}
	//Проверяем геометрию
	Plane plane;
	plane.N = (vrt[1] - vrt[0]) ^ (vrt[2] - vrt[0]);
	if(plane.N.Normalize() > 1e-20f)
	{
		plane.Move(vrt[0]);
		//Проверим дистанцию до плоскости
		float dist = plane.Dist(detector.pos);
		if(fabsf(dist) > detector.r)
		{
			return false;
		}
		//Найдём круг на плоскости
		Vector center = detector.pos - plane.N*dist;
		float r = sqrtf(detector.r*detector.r - dist*dist);
		//Проверим на пересечение круг и четырёхугольник
		Plane pln;
		for(long n = 0; n < 4; n++)
		{
			//Строим плоскость, ортоганальную плоскости четырёхугольника и прохлдящую через грань
			Vector v = vrt[n] - vrt[n - 1 >= 0 ? n - 1 : 3];
			if(v.Normalize() < 1e-20) continue;
			pln.N = v ^ plane.N;
			pln.Move(vrt[n]);
			//Проверяем дистанцию до центра круга
			float d = pln.Dist(center);
			if(d > r) break;
		}
		if(n < 4) return false;
	}else{
		//Это точка или линия
		if(~(vrt[1] - vrt[0]) > 0.0f)
		{
			if(!detector.Intersection(vrt[0], vrt[1]))
			{
				return false;
			}
		}else
		if(~(vrt[2] - vrt[1]) > 0.0f)
		{			
			if(!detector.Intersection(vrt[1], vrt[2]))
			{
				return false;
			}
		}else{
			if(!detector.Intersection(vrt[0]))
			{
				return false;
			}
		}
	}
	//Поподание
	return HitProcess(source, obj, hp);
}

//Логическая проверка на возможность аттаки
inline bool DamageDetector::IsCanAttack(MissionObject * obj, dword source, float & hp)
{
	//Отсекаем воздействие по цели
	if(targetID.NotEmpty())
	{
		if(target.Ptr() != obj)
		{
			return false;
		}
	}
	if(source != ds_check)
	{
		//Отсекаем по источнику демеджа
		hp *= multipliers.Multiplier(source);
		if(fabsf(hp) < 1e-10f)
		{
			return false;
		}
	}else{
		return true;
	}
	//Если режим без коллижена то делаемся имунными на время
	if(!collide)
	{
		//Смотрим список иммуна
		for(dword i = 0; i < immuneCount; i++)
		{
			if(immune[i].obj == obj)
			{
				return false;
			}
		}
	}
	return true;
}

//Отработка попадания
bool DamageDetector::HitProcess(dword source, MissionObject * obj, float hp)
{
	if(source != ds_check)
	{
		SetImmune(obj);
		ApplyDamage(hp);
		return (collide != 0);
	}
	return true;
}

//Сделать объект имунным
inline void DamageDetector::SetImmune(MissionObject * obj)
{
	if(collide)
	{
		return;
	}
	if(immuneCount >= ARRSIZE(immune))
	{
		const dword last =  - 1;
		for(dword i = 1; i < ARRSIZE(immune); i++)
		{
			immune[i - 1] = immune[i];
		}
		immuneCount--;
	}
	immune[immuneCount].obj = obj;
	immune[immuneCount].time = immuneTime;
	immuneCount++;
	immuneTimerWorkTime = 5.0f;
	if(!immuneTimerIsActive)
	{
		immuneTimerIsActive = true;
		SetUpdate(&DamageDetector::ImmuneTimer, ML_ALPHA5);
	}
}

//Таймер невосприимчевости
void _cdecl DamageDetector::ImmuneTimer(float dltTime, long level)
{
	for(dword i = 0; i < immuneCount; i++)
	{
		immune[i].time -= dltTime;
		if(immune[i].time <= 0.0f)
		{
			for(dword j = i + 1; j < immuneCount; j++)
			{
				immune[j - 1] = immune[j];
			}
			immuneCount--;
		}
	}
	if(!immuneCount && immuneTimerWorkTime < 0.0f)
	{
		DelUpdate(&DamageDetector::ImmuneTimer);
		immuneTimerIsActive = false;
	}else{
		immuneTimerWorkTime -= dltTime;
	}
}


//============================================================================================


void DamageDetector::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEqual(id, "reset"))
	{
		Reset();
		LogicDebug("Command <reset> HP now is %f", HP);
	}

}

//Работа детектора в режиме редактирования
void _cdecl DamageDetector::EditModeWork(float dltTime, long level)
{
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(!EditMode_IsVisible()) return;
	//Обновляем состояние
	dword clr;
	UpdateTarget();
	if(!target.Validate())
	{
		FindObject(targetID, target);
	}
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
	//Рисуем
	Render().DrawSphere(detector.pos, detector.r,  clr, "ShowDetector");
	if(EditMode_IsSelect())
	{
		Render().Print(detector.pos, -1.0f, -1.0f, 0xffffffff, "Object id: %s", GetObjectID().c_str());
		Render().Print(detector.pos, -1.0f, 0.0f, 0xffffffff, "Target: %s", targetID.c_str());
		Render().Print(detector.pos, -1.0f, 1.0f, 0xffffffff, IsActive() ? "State: on" : "State: off");
	}
}



//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(DamageDetector, "Damage detector", '1.00', 0x0fffffff, "Damage detector triggering when mission object attack it.", "Logic")
	MOP_STRINGC("Object id", "Player", "Is set object, then apply damage only from this object")
	MOP_POSITIONC("Position", Vector(0.0f), "Detector sphere position")
	MOP_FLOATEXC("Radius", 0.7f, 0.0001f, 1000000.0f, "Detector sphere radius")
	MOP_FLOATEXC("HP", 1.0f, 0.0001f, 1000000.0f, "Hit points of this object")
	MOP_DR_MULTIPLIERSG
	MOP_BOOLC("Active", true, "Active object in start mission time")
	MOP_BOOLC("Autoreset", true, "When trigger is done, reset it")
	MOP_BOOLC("Anytime damage", true, "Activate damage event when detector is done too")
	MOP_BOOLC("Collision", true, "Dont collide with damage source")
	MOP_FLOATEXC("Immune time", 0.2f, 0.001f, 1000000.0f, "Use in non collision mode.\nTime when detector dont take damage from attak object.")
	MOP_MISSIONTRIGGERG("Damage", "Damage.")
	MOP_MISSIONTRIGGERG("Done", "Activate.")
MOP_ENDLIST(DamageDetector)


