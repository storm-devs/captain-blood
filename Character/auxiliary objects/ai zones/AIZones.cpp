
#include "AIZones.h"


AIZones::AIZones(): zones(_FL_)
{
}

AIZones::~AIZones()
{
}

//Инициализировать объект
bool AIZones::Create(MOPReader & reader)
{
	Registry(AIZones_GROUP);

	InitParams(reader);
	
	Activate(reader.Bool());
	
	if (EditMode_IsOn())	
	{
		SetUpdate(&AIZones::Draw, ML_DYNAMIC1);
	}
	else
	{
		DelUpdate(&AIZones::Draw);
	}

	
	return true;
}

//Обновить параметры
bool AIZones::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

//Получить размеры описывающего ящика
void AIZones::EditMode_GetSelectBox(Vector & min, Vector & max)
{
}

//Получить матрицу объекта
Matrix & AIZones::GetMatrix(Matrix & mtx)
{
	return (mtx = Matrix());
}

//Нарисовать модельку
void _cdecl AIZones::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	for(long i = 0; i < zones; i++)
	{
		Zone & zone = zones[i];

		if (zone.isSelected)
		{
			Render().DrawBox(zone.min, zone.max, zone.mtx, 0xffffff00);
		}
		else
		{
			Render().DrawBox(zone.min, zone.max, zone.mtx, IsActive() ? 0xff00ff00 : 0xffff0000);
		}

		Render().Print(zone.mtx.pos,100,0,0xff00ff00,"%s - %i",GetObjectID().c_str(),i);
	}
}

void AIZones::ShowDebug(bool show)
{
	if (show)
	{
		SetUpdate(&AIZones::Draw, ML_DYNAMIC1);
	}
	else
	{
		DelUpdate(&AIZones::Draw);
	}
}

//Проверить точку на попадание в зону
bool AIZones::IsInside(const Vector & pos,bool check_buffer)
{
	if(!IsActive())
	{
		return false;
	}
	long count = zones;
	Zone * zs = zones.GetBuffer();
	for(long i = 0; i < count; i++)
	{
		Zone & zone = zs[i];
		Vector p = zone.mtx.MulVertexByInverse(pos);

		if (!check_buffer)
		{		
			if(zone.min <= p && p <= zone.max)
			{
				return true;
			}
		}
		else
		{
			float buffer_zone = 1.0f;
			if(zone.min + Vector(buffer_zone)<= p && p <= zone.max + Vector(-buffer_zone))
			{
				return true;
			}

		}
	}

	return false;
}


void AIZones::InitParams(MOPReader & reader)
{
	long zonesCount = reader.Array();	
	zones.Empty();

	for (long i=0;i<zonesCount;i++)
	{
		Zone & zone = zones[zones.Add()];
		zone.max.x = reader.Float()*0.5f;
		zone.max.y = reader.Float()*0.5f;
		zone.max.z = reader.Float()*0.5f;
		zone.min = -zone.max;
		Vector pos = reader.Position();
		Vector ang = reader.Angles();
		zone.mtx.Build(ang, pos);	

		zone.isSelected = reader.Bool();
	}	
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(AIZones, "AI Zone", '1.00', 0, "AI Zones", "Character objects")

	MOP_ARRAYBEG("Zones", 0, 200)

		MOP_FLOATEX("Width", 3.0f, 0.01f, 1024.0f)
		MOP_FLOATEX("Height", 3.0f, 0.01f, 1024.0f)
		MOP_FLOATEX("Length", 0.5f, 0.01f, 1024.0f)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLES("Angles", Vector(0.0f))
		MOP_BOOL("Selected", false)	
	MOP_ARRAYEND
	
	MOP_BOOL("Active", true)	
MOP_ENDLIST(AIZones)


