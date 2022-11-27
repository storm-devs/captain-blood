//============================================================================================
// Spirenkov Maxim, 2005
//============================================================================================
// Mission objects
//============================================================================================
// CollisionBox
//============================================================================================


#include "CollisionBox.h"


MissionCollisionBox::MissionCollisionBox(): colliders(_FL_)
{	
	needAiRegistry = false;
	aiColider = null;
}

MissionCollisionBox::~MissionCollisionBox()
{
	DelAllColliders();
	RELEASE(aiColider);
}


//Инициализировать объект
bool MissionCollisionBox::Create(MOPReader & reader)
{
	DelAllColliders();
	needAiRegistry = false;
	InitParams(reader);
	
	PhysicsCollisionGroup groups[] = { phys_world, phys_character, phys_player, phys_enemy, phys_ally, phys_boss};

	for (int i=0;i<(int)(sizeof(groups)/sizeof(PhysicsCollisionGroup));i++)
	{
		if (reader.Bool())
		{
			AddCollider(groups[i]);

			if (phys_character == groups[i])
			{
				needAiRegistry = true;
			}
		}		
	}		

	Activate(reader.Bool());

	if(reader.Bool())
	{
		SetUpdate(&MissionCollisionBox::Draw, ML_DYNAMIC1);
	}

	return true;
}

//Пересоздать объект
void MissionCollisionBox::Restart()
{
	ReCreate();
}

//Обработчик команд для объекта
void MissionCollisionBox::Command(const char * id, dword numParams, const char ** params)
{
	if(!id || !id[0]) return;

	if(string::IsEqual(id, "teleport"))
	{
		if(numParams < 1) return;

		MOSafePointer obj;
		if(FindObject(ConstString(params[0]), obj))
		{
			Matrix mObj;
			obj.Ptr()->GetMatrix(mObj);

			if(aiColider)
			{
				aiColider->SetMatrix(mObj);
			}	

			for (int i=0;i<(int)colliders.Size();i++)
			{
				colliders[i]->SetTransform(mObj);
			}
			
			LogicDebug("Teleport box \"%s\" to mission object \"%s\"", GetObjectID().c_str(), obj.Ptr()->GetObjectID().c_str());
		}
		else
		{
			LogicDebugError("Can't teleport box \"%s\" to mission object \"%s\", object not found...", GetObjectID().c_str(), params[0]);
		}
	}
}


//Инициализировать объект
bool MissionCollisionBox::EditMode_Create(MOPReader & reader)
{
	InitParams(reader);
	MissionObject::Activate(reader.Bool());
	SetUpdate(&MissionCollisionBox::Draw, ML_DYNAMIC1);

	return true;
}

//Обновить параметры
bool MissionCollisionBox::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

//Получить размеры описывающего ящика
void MissionCollisionBox::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -aiColider->GetBoxSize()*0.5f;
	max = aiColider->GetBoxSize()*0.5f;
}

void MissionCollisionBox::GetBox(Vector & min, Vector & max)
{
	min = -aiColider->GetBoxSize()*0.5f;
	max = aiColider->GetBoxSize()*0.5f;
}

//Получить матрицу объекта
Matrix & MissionCollisionBox::GetMatrix(Matrix & mtx)
{
	return (mtx = aiColider->GetMatrix());
}

//Нарисовать модельку
void _cdecl MissionCollisionBox::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	Render().DrawSolidBox(-aiColider->GetBoxSize()*0.5f, aiColider->GetBoxSize()*0.5f, aiColider->GetMatrix(), IsActive() ? 0xff00ff00 : 0xffff0000);
}

//Активировать/деактивировать объект
void MissionCollisionBox::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	for (int i=0;i<(int)colliders.Size();i++)
	{
		colliders[i]->Activate(isActive);
	}	
	
	if(needAiRegistry)
	{
		aiColider->Activate(isActive);
	}	
}

void MissionCollisionBox::InitParams(MOPReader & reader)
{
	if(!aiColider)
	{
		aiColider = QTCreateObject(MG_AI_COLLISION, _FL_);
	}

	aiColider->Activate(false);
	aiColider->SetBoxCenter(0.0f);
	Vector size;
	size.x = reader.Float();
	size.y = reader.Float();
	size.z = reader.Float();
	aiColider->SetBoxSize(size);
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	aiColider->SetMatrix(Matrix(ang, pos));
}

void MissionCollisionBox::AddCollider(PhysicsCollisionGroup group)
{
	IPhysBox* collider = Physics().CreateBox(_FL_, aiColider->GetBoxSize(), aiColider->GetMatrix(), false);
	collider->SetGroup(group);

	colliders.Add(collider);
}

void MissionCollisionBox::DelAllColliders()
{	
	for (int i=0;i<(int)colliders.Size();i++)
	{
		colliders[i]->Release();
	}

	colliders.DelAll();
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MissionCollisionBox, "Collision box", '1.00', 0, "Designer's defined collision box", "Physics")
	MOP_FLOATEX("Width", 3.0f, 0.01f, 100.0f)
	MOP_FLOATEX("Height", 3.0f, 0.01f, 100.0f)
	MOP_FLOATEX("Length", 0.5f, 0.01f, 100.0f)
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_BOOLC("World", false, "Collision with physic objects")
	MOP_BOOLC("Characters", true, "Collision with any character (include player)")
	MOP_BOOLC("Player", false, "Collision with player")	
	MOP_BOOLC("Enemy", false, "Collision with enemy")
	MOP_BOOLC("Ally", false, "Collision with ally")
	MOP_BOOLC("Boss", false, "Collision with boss")
	MOP_BOOL("Active", true)
	MOP_BOOL("Debug draw", false)
MOP_ENDLIST(MissionCollisionBox)


