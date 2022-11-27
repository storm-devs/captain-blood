
#include "BombSafe.h"


BombSafe::BombSafe()
{
}

BombSafe::~BombSafe()
{
}


//Инициализировать объект
bool BombSafe::Create(MOPReader & reader)
{
	Registry(BombSafe_GROUP);

	InitParams(reader);
	
	Activate(reader.Bool());
	
	if (EditMode_IsOn())	
	{
		SetUpdate(&BombSafe::Draw, ML_DYNAMIC1);
	}
	else
	{
		DelUpdate(&BombSafe::Draw);
	}
	
	return true;
}

//Обновить параметры
bool BombSafe::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

//Получить размеры описывающего ящика
void BombSafe::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min =-0.5f*size;
	max = 0.5f*size;
}

//Получить матрицу объекта
Matrix & BombSafe::GetMatrix(Matrix & mtx)
{
	return (mtx = transform);
}

//Нарисовать модельку
void _cdecl BombSafe::Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	//Render().DrawSolidBox(-0.5f*size, 0.5f*size, transform, IsActive() ? 0xff00ff00 : 0xffff0000);
	Render().DrawBox(-0.5f*size, 0.5f*size, transform, IsActive() ? 0xffff00ff : 0xffff0000);
}

//Активировать/деактивировать объект
void BombSafe::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	//if(worldBox) worldBox->Activate(isActive);
	//if(charsBox) charsBox->Activate(isActive);
}

void BombSafe::InitParams(MOPReader & reader)
{
	size.x = reader.Float();
	size.y = reader.Float();
	size.z = reader.Float();
	Vector pos = reader.Position();	
	transform.Build(Vector(0,0,0), pos);
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(BombSafe, "BombSafe", '1.00', 0, "Area where character can't be target for bomb", "Character objects")
	MOP_FLOATEX("Width", 3.0f, 0.01f, 1024.0f)
	MOP_FLOATEX("Height", 3.0f, 0.01f, 1024.0f)
	MOP_FLOATEX("Length", 0.5f, 0.01f, 1024.0f)
	MOP_POSITION("Position", Vector(0.0f))		
	MOP_BOOL("Active", true)	
MOP_ENDLIST(BombSafe)


