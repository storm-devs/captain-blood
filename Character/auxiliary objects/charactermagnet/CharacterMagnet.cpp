
#include "CharacterMagnet.h"
#include "..\..\Character\Character.h"
#include "..\..\Character\Components\CharacterLogic.h"
#include "..\..\Character\Components\CharacterPhysics.h"

CharacterMagnet::CharacterMagnet()
{
}

CharacterMagnet::~CharacterMagnet()
{
}

//Инициализировать объект
bool CharacterMagnet::Create(MOPReader & reader)
{
	pos = reader.Position();
	target = reader.String();
	return true;
}

//Инициализировать объект в режиме редактирования
bool CharacterMagnet::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&CharacterMagnet::Draw, ML_ALPHA3);
	return EditMode_Update(reader);
}

//Обновить параметры в режиме редактирования
bool CharacterMagnet::EditMode_Update(MOPReader & reader)
{
	pos = reader.Position();
	return true;
}

//Получить размеры описывающего ящика
void CharacterMagnet::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = Vector(-0.1f, -0.1f, -0.2f);
	max = Vector(0.1f, 0.1f, 0.2f);
}

//Активировать/деактивировать объект
void CharacterMagnet::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		
		//Объект
		MOSafePointerTypeEx<Character> trgt;
		static const ConstString strTypeId("Character");
		trgt.FindObject(&Mission(),target,strTypeId);
		
		if(!trgt.Ptr())
		{
			LogicDebugError("Object \"%s\" not found", target.c_str());
			return;
		}
		
		//Направление
		Vector dir = (pos - trgt.Ptr()->physics->GetPos()).GetXZ();
		dir.NormalizeXZ();
		trgt.Ptr()->logic->Kick(dir);
		LogicDebug("Character is kick to direction (x:%f, y:%f, z:%f)", dir.x, dir.y, dir.z);
	}
}

//Получить матрицу объекта
Matrix & CharacterMagnet::GetMatrix(Matrix & mtx)
{	
	return mtx.BuildPosition(pos);
}

//Рисование точек
void _cdecl CharacterMagnet::Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	Render().DrawSphere(pos + Vector(0.0f, 0.0f, -0.1f), 0.1f, 0xffff0000);
	Render().DrawSphere(pos + Vector(0.0f, 0.0f, 0.1f), 0.1f, 0xff0000ff);
	Render().DrawSphere(pos, 0.05f, 0xcfffffff);
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(CharacterMagnet, "Magnet", '1.00', 100, "Drop target in this position, when activate", "Character objects")	
	MOP_POSITION("Position", Vector(0.0f))
	MOP_STRING("Target", "Player")
MOP_ENDLIST(CharacterMagnet)

