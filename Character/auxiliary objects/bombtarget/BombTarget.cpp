
#include "BombTarget.h"


BombTarget::BombTarget()
{
	fAimDistance = 15.0f;
}

BombTarget::~BombTarget()
{
}

//Инициализировать объект
bool BombTarget::Create(MOPReader & reader)
{
	pos = reader.Position();
	fAimDistance = reader.Float();
	Activate(reader.Bool());
	return true;
}

//Инициализировать объект в режиме редактирования
bool BombTarget::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&BombTarget::Draw, ML_ALPHA3);
	return EditMode_Update(reader);
}

//Обновить параметры в режиме редактирования
bool BombTarget::EditMode_Update(MOPReader & reader)
{
	pos = reader.Position();
	return true;
}

//Получить размеры описывающего ящика
void BombTarget::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = Vector(-0.35f, -0.25f, -0.35f);
	max = Vector(0.35f, 0.75f, 0.35f);
}

//Активировать/деактивировать объект
void BombTarget::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		Registry(GroupId('b','m','b','t'));
	}else{
		LogicDebug("Deactivate");
		Unregistry(GroupId('b','m','b','t'));
	}
}

//Получить матрицу объекта
Matrix & BombTarget::GetMatrix(Matrix & mtx)
{	
	return mtx.BuildPosition(pos);
}

//Рисование точек
void _cdecl BombTarget::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	for(long i = 0; i < 5; i++)
	{
		Render().DrawSphere(pos + Vector(0.0f, i*0.1f, 0.0f), 0.1f + i*0.05f + (i ? 0.0f : 0.1f), i ? 0xff000000 : 0xffff0000);
	}	
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(BombTarget, "Bomb target", '1.00', 100, "Target for player bomb", "Character objects")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_FLOAT("Aim Distance", 15.0f);
	MOP_BOOL("Active", true)
MOP_ENDLIST(BombTarget)

