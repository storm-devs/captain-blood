
#include "NoFatality.h"


NoFatality::NoFatality(): zones(_FL_)
{
}

NoFatality::~NoFatality()
{
}


//Инициализировать объект
bool NoFatality::Create(MOPReader & reader)
{
	Registry(NoFatality_GROUP);

	InitParams(reader);
	
	Activate(reader.Bool());
	
	if (EditMode_IsOn())	
	{
		SetUpdate(&NoFatality::Draw, ML_DYNAMIC1);
	}
	else
	{
		DelUpdate(&NoFatality::Draw);
	}

	
	return true;
}

//Обновить параметры
bool NoFatality::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

//Получить размеры описывающего ящика
void NoFatality::EditMode_GetSelectBox(Vector & min, Vector & max)
{
}

//Получить матрицу объекта
Matrix & NoFatality::GetMatrix(Matrix & mtx)
{
	return (mtx = Matrix());
}

//Нарисовать модельку
void _cdecl NoFatality::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	for(long i = 0; i < zones; i++)
	{
		Zone & zone = zones[i];
		Render().DrawBox(zone.min, zone.max, zone.mtx, IsActive() ? 0xffff0000 : 0xffff0000);
	}
}

//Активировать/деактивировать объект
void NoFatality::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	//if(worldBox) worldBox->Activate(isActive);
	//if(charsBox) charsBox->Activate(isActive);
}

//Проверить точку на попадание в зону
bool NoFatality::IsInside(const Vector & pos)
{
	long count = zones;
	Zone * zs = zones.GetBuffer();
	for(long i = 0; i < count; i++)
	{
		Zone & zone = zs[i];
		Vector p = zone.mtx.MulVertexByInverse(pos);
		if(zone.min <= p && p <= zone.max)
		{
			return true;
		}
	}
	return false;
}


void NoFatality::InitParams(MOPReader & reader)
{
	long zonesCount = reader.Array();

	zones.DelAll();

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
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(NoFatality, "NoFatality", '1.00', 0, "NoFatality", "Character objects")

	MOP_ARRAYBEG("Zones", 0, 200)

		MOP_FLOATEX("Width", 3.0f, 0.01f, 1024.0f)
		MOP_FLOATEX("Height", 3.0f, 0.01f, 1024.0f)
		MOP_FLOATEX("Length", 0.5f, 0.01f, 1024.0f)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLES("Angles", Vector(0.0f))
	MOP_ARRAYEND
	
	MOP_BOOL("Active", true)	
MOP_ENDLIST(NoFatality)