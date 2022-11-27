
#include "BombBoom.h"
#include "..\Arbiter\CharactersArbiter.h"

BombBoom::BombBoom()
{
	connectObjectPtr.Reset();
}

BombBoom::~BombBoom()
{
}

//Инициализировать объект
bool BombBoom::Create(MOPReader & reader)
{
	pos = reader.Position();
	radius = reader.Float();
	damage = reader.Float();
	power = reader.Float();
	bDamagePlayer = reader.Bool();

	connectToObject = reader.String();
	connectObjectPtr.Reset();

	UpdateConnectPointer();

	return true;
}

//Инициализировать объект в режиме редактирования
bool BombBoom::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&BombBoom::Draw, ML_ALPHA3);
	return EditMode_Update(reader);
}

//Обновить параметры в режиме редактирования
bool BombBoom::EditMode_Update(MOPReader & reader)
{
	pos = reader.Position();
	radius = reader.Float();
	damage = reader.Float();
	power = reader.Float();

	return true;
}

//Получить размеры описывающего ящика
void BombBoom::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = Vector(-0.6f, -0.3f, -0.6f);
	max = Vector(0.6f, 1.0f, 0.6f);
}

//Активировать/деактивировать объект
void BombBoom::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
		Boom(pos * transform);
	}
}

//Получить матрицу объекта
Matrix & BombBoom::GetMatrix(Matrix & mtx)
{	
	mtx.BuildPosition(pos);
	mtx = mtx * transform;

	return mtx;
}

//Рисование точек
void _cdecl BombBoom::Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;

 	for(long i = 0; i < 8; i++)
	{
		dword color = 0x2fff2000 + ((8 - i) << 28);
		if(EditMode_IsSelect())
		{
			if(rand() & 1) color += 0x20000000;
		}
		Render().DrawSphere(pos + Vector(0.0f, i*0.05f, 0.0f), 0.1f + i*0.08f + (i ? 0.0f : 0.03f), color);
	}	
	if(EditMode_IsSelect())
	{
		Render().DrawSphere(pos, radius, 0xcfffffff);
	}
}

void BombBoom::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) return;

	if(string::IsEqual(id, "teleport"))
	{
		if(numParams < 1)
		{
			LogicDebugError("Command <teleport> error. Invalidate parameters...");
			return;
		}

		MOSafePointer obj;
		FindObject(ConstString(params[0]),obj);

		if(obj.Ptr())
		{
			Matrix mtx;
			obj.Ptr()->GetMatrix(mtx);
			
			pos = mtx.pos;			

			LogicDebug("Command <teleport>. Bomb explosion is teleported to \"%s\" (%4.3f,%4.3f,%4.3f)", params[0], mtx.pos.x, mtx.pos.y, mtx.pos.z);
		}
		else
		{
			LogicDebugError("Command <teleport> error. Can't find object <%s>.",params[0]);
		}
	}else
	if(string::IsEqual(id, "boom"))
	{
		if(numParams < 3)
		{
			LogicDebugError("Command <boom> error. Invalidate parameters...");
			return;
		}
		Vector p(0.0f);
		char * spos = null;
		p.x = (float)strtod(params[0], &spos);
		p.y = (float)strtod(params[1], &spos);
		p.z = (float)strtod(params[2], &spos);
		Boom(p);
		LogicDebug("Command <teleport>. Bomb explosion is boom at position (%4.3f,%4.3f,%4.3f)", p.x, p.y, p.z);
	}
}


void BombBoom::Boom(const Vector & p)
{
	MOSafePointerType<CharactersArbiter> arbiter;

	static const ConstString objectId("CharactersArbiter");
	Mission().CreateObject(arbiter.GetSPObject(),"CharactersArbiter", objectId);	
	Assert(arbiter.Ptr());

	MissionObject* source = this;

	if (!bDamagePlayer)
	{
		source = Mission().Player();
	}

	arbiter.Ptr()->Boom(source, DamageReceiver::ds_bomb, p, radius, damage, power);
}

void BombBoom::UpdateConnectPointer()
{
	if(connectToObject.IsEmpty())
	{
		return;
	}

	if(!connectToObject.c_str()[0])
	{
		connectToObject.Empty();		
		DelUpdate(&BombBoom::UpdateConnection);
		return;
	}

	LogicDebug("Flare connected to %s", connectToObject.c_str());		
	SetUpdate(&BombBoom::UpdateConnection);	
}

//Обновить позицию источника
void _cdecl BombBoom::UpdateConnection(float dltTime, long level)
{
	if(connectObjectPtr.Validate())
	{
		transform = connectObjectPtr.Ptr()->GetMatrix(Matrix());
	}
	else
	{
		transform.SetIdentity();
		FindObject(connectToObject,connectObjectPtr);
	}	
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(BombBoom, "Bomb explosion", '1.00', 100, "Provide bomb explosion, when activate this object - BOOM!", "Character objects")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_FLOATEX("Radius", 3.5f, 0.1f, 1000000.0f)
	MOP_FLOAT("Damage", 100.0f)
	MOP_FLOATEX("Power", 0.75f,0.1f,100.0f)
	MOP_BOOL("Damage Player", true)
	MOP_STRING("Connect to object", "")
MOP_ENDLIST(BombBoom)

