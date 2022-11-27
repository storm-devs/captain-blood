//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharacterPhysics
//===========================================================================================================================

#include "..\Character.h"
#include "CharacterPhysics.h"
#include "CharacterLogic.h"
#include "CharacterItems.h"
#include "CharacterAnimation.h"

//===========================================================================================================================

CharacterPhysics::CharacterPhysics()
{	
	//Позиция персонажа
	pos = 0.0f;
	move = 0.0f;
	ay = 0.0f;
	az = 0.0f;
	newAy = 0.0f;
	rotDirect = 0;
	turnSpeed = 10.0f;

	curFrame = -1;

	//Размеры персонажа
	radius = 0.8f;
	height = 1.8f;

	velY = 0.0f;
	frcY = 0.0f;
	kickState = false;
	kickDir = 0.0f;	

	vFlyDir = 0.0f;

	vAttackerDir = 0.0f;

	bAllowMove = true;	

	bNeedCalcModelMatrix = true;

	dist_to_move = 0.0f;
	move_dir = 0.0f;

	bGravityOn = true;
	bUseFakeModelPosition = false;

	isFromRagdoll = false;

	dltPos = 0.0f;								
	isActive = false;
		
	fake_pos = 0.0f;

	dltPosSpeed = 0.0f;
	dltPos = 0.0f;	
	vFlyDir = 0.0f;
	vAttackerDir = 0.0f;

	phchr = null;
}

CharacterPhysics::~CharacterPhysics()
{
	Release();
}

void CharacterPhysics::Release()
{
	if(phchr) phchr->Release();
	phchr = null;
}

void CharacterPhysics::SetOwner(Character* owner)
{
	chr = owner;

	if(!chr->EditMode_IsOn())
	{
		phchr = chr->Mission().Physics().CreateCharacter(_FL_, radius, height);
		Assert(phchr);
		phchr->ApplyForceToObjects(20.0f);
	}
	else
	{
		phchr = null;
	}
}

//Активировать/деактивировать коллижен
void CharacterPhysics::Activate(bool active, const Vector * ragdollPos, bool isRagdoll)
{	
	isActive = active;

	if (!active && isRagdoll)
	{
		isFromRagdoll = true;
		lastRagdollPos = pos;
	}

	if (phchr)
		phchr->Activate(isActive);

	if (active && isFromRagdoll && ragdollPos)
	{
		if (CheckRagdollPosition(*ragdollPos))
			SetPos(*ragdollPos);
		else
			Move(*ragdollPos - lastRagdollPos, false);
	}
	else
	{
		if (phchr)
		{
			//phchr->Activate(isActive);
			SetPos((ragdollPos) ? *ragdollPos : pos);
		}
	}

	if (!isActive)
	{
		newAy = ay;
	}
}

bool CharacterPhysics::CheckRagdollPosition(const Vector & checkPos)
{
	Vector newPos = Vector(checkPos.x, checkPos.y + height * 0.5f + 0.01f, checkPos.z);

	// если есть патч под этой точкой, и он не слишком низко от позиции чара
	Vector to = newPos + Vector(0.0f, -10.0f, 0.0f);
	IPhysicsScene::RaycastResult detail;
	IPhysBase * phys_obj = chr->Physics().Raycast(newPos, to, phys_mask(phys_character), &detail);
	Vector charPos;
	if (phys_obj)
	{
		// если слишком большая высота, то скорее всего там высокий регдолл патч, потому пытаемся сделать Move к этой точке
		if (checkPos.y - detail.position.y > height * 0.5f)
			return false;
	}
	else
		return false;

	// если есть куда встать капсуле
	bool result = chr->Physics().CheckOverlapCapsule(newPos, height - radius * 2.0f, radius, phys_mask(phys_character), true, false);

	return !result;
}

void CharacterPhysics::InnerReset()
{
	velY = 0.0f;
	move = 0.0f;

	turnSpeed = 10.0f;

	curFrame = -1;

	bNeedCalcModelMatrix = true;
}

//Обнулить состояние физической части
void CharacterPhysics::Reset()
{
	InnerReset();
	isFromRagdoll = false;
}

//Установить радиус персонажа
void CharacterPhysics::SetRadius(float radius)
{
	this->radius = radius;
	if (phchr) phchr->SetRadius(radius);
}

//Установить высоту персонажа
void CharacterPhysics::SetHeight(float height)
{
	this->height = height;
	if (phchr) phchr->SetHeight(height);
}

//Установить позицию
void CharacterPhysics::SetPos(const Vector & p)
{
	pos = p;
	
	if (phchr)
	{
		if (chr && !chr->logic->IsActor())
		{
			Vector from = pos + Vector(0.0f, height*0.9f, 0.0f);
			Vector to = pos + Vector(0.0f, -height*0.5f, 0.0f);
			IPhysicsScene::RaycastResult detail;
			IPhysBase * phys_obj = chr->Physics().Raycast(from, to, phys_mask(phys_character, phys_player), &detail);
			Vector charPos;
			if (phys_obj)
				charPos = detail.position + Vector(0.0f, height*0.5f + 0.01f, 0.0f);
			else
				charPos = pos + Vector(0.0f, height*0.5f, 0.0f);
	
			phchr->SetPosition(charPos);
		}
		else
			phchr->SetPosition(pos + Vector(0.0f, height*0.5f, 0.0f));		
	}

	InnerReset();
}

//Получить позицию
const Vector CharacterPhysics::GetPos(bool count_bone_offset)
{	
	if (count_bone_offset)
	{
		return mModelBoneMatrix.pos;
	}
	
	if (curFrame == chr->arbiter->GetCurFrame())
	{
		return last_frame_pos; 
	}
	else if (phchr && IsActive())
	{
		pos = phchr->GetPosition();
		pos.y -= height*0.5f;
		last_frame_pos = pos;
		curFrame = chr->arbiter->GetCurFrame();
	}

	return pos;
}


//Повернуться в заданное направление
void CharacterPhysics::Turn(float angle, bool isInstantly, bool ignoreState)
{
	if(!ignoreState)
	{
		CharacterLogic::State state = chr->logic->GetState();
		switch(state)
		{
		case CharacterLogic::state_unknown:
		case CharacterLogic::state_topair:
		case CharacterLogic::state_pair:
		case CharacterLogic::state_knockdown:
		case CharacterLogic::state_kick:
		case CharacterLogic::state_die:
		case CharacterLogic::state_dead:
		case CharacterLogic::state_dropbomb:
			return;
		}
	}
	if(isInstantly)
	{
		ay = newAy = angle;
		rotDirect = 0;

		bNeedCalcModelMatrix = true;

		return;
	}
	//Задаём скорость изменения угла
	Vector newa, cura;
	newa.MakeXZ(angle);
	cura.MakeXZ(ay);
	float delta = -newa.GetAngleXZ(cura);
	if(delta > 0.0f)
	{
		rotDirect = 1;
	}else{
		if(delta < 0.0f)
		{
			rotDirect = -1;
		}else{
			rotDirect = 0;
		}
	}
	newAy = ay + delta;	
}

//Сориентироваться на заданную точку
void CharacterPhysics::Orient(const Vector & by, bool isInstantly, bool ignoreState)
{
	Vector dir;
	
	if (IsUseFakeModelPosition())
	{	
		dir = by - fake_pos;
	}
	else
	{
		dir = by - pos;
	}
	
	Turn(dir.GetAY(ay), isInstantly, ignoreState);
}

//Обновить угол
void CharacterPhysics::TurnUpdate(float dltTime)
{
	//Угол наклона модельки персонажа
	if(chr->animation)
	{
		float instAz = Clampf((ay - newAy)*0.4f*chr->animation->GetConstBlend("slope"), -0.2f, 0.2f);
		az += (instAz - az)*Clampf(dltTime*12.0f);		
	}
		
	//Угол ориентации прсонажа
	ay += (newAy - ay)*Clampf(dltTime*turnSpeed);

	bNeedCalcModelMatrix = true;
}

//Обновить положение объектов
void CharacterPhysics::Move(float dltTime)
{	
	MoveInPosition(dltTime);

	if (!bAllowMove && !chr->logic->IsActor() && !chr->logic->IsPairMode())
	{
		return;
	}

	if (!phchr) return;

	//Ограничим временной интервал
	if(dltTime > 0.1f) dltTime = 0.1f;

	if (!chr->logic->IsActor())
	{	
		if(chr->animation)
		{			
			//Считываем перемещения с анимации
			dltPos = Vector(0.0f);
			float dltAng = 0.0f;
			chr->animation->GetMovement(dltPos, dltAng);

			//dltPos *= 2.0f;

			if(!kickState)
			{
				move += (dltPos.GetXZ()).Rotate(ay);
				ay += dltAng;
			}
			else
			{
				move += kickDir*dltPos.GetLengthXZ();
			}

			dltPosSpeed = dltPos / dltTime;
		}

		//Падаем
		if (!chr->logic->IsPairMode() && bGravityOn && IsActive())
		{		
			velY -= velY*Clampf(frcY*18.5f*dltTime);
			move.y += velY*dltTime;
			velY += -12.5f*dltTime;			

			if(fabsf(velY) > 100.0f)
			{
				chr->logic->SetHP(0.0f);
			}
		}
		else
		{
			velY = 0.0f;
		}
	}
	else
	if (chr->logic->IsActor())
	{
		if(chr->actorData.ani)
		{			
			dltPos = Vector(0.0f);
			
			float dltAng = 0.0f;
			chr->actorData.ani->GetMovement(dltPos);
			if(!kickState)
			{
				move += (dltPos.GetXZ()).Rotate(ay);
				ay += dltAng;
			}
			else
			{
				move += kickDir*dltPos.GetLengthXZ();
			}
		}
	}
	else
	{
		move = 0.0f;
	}
		
	dword flags = 0;	

	if (IsActive())
	{
		if(!chr->isPlayer && (!chr->logic->IsPairMode() || (chr->logic->IsPairMode() && chr->logic->pair != NULL)))
		{
			PhysicsCollisionGroup gr = phys_enemy;
			
			if (chr->logic->GetSide() == CharacterLogic::s_ally)
			{
				gr = phys_ally;
			}
			else
			if (chr->logic->GetSide() == CharacterLogic::s_boss)
			{
				gr = phys_boss;
			}

			if (!chr->logic->IsActor())
			{		
				flags = phchr->Move(move, phys_mask(phys_physobjects,phys_character, phys_playerctrl, gr));
			}
			else
			{
				flags = phchr->Move(move, phys_mask(phys_character, phys_playerctrl));
			}
		}
		else
		if(chr->isPlayer)
		{
			if (chr->logic->IsPairMode())
			{
				flags = phchr->Move(move, phys_mask(phys_physobjects,phys_character, phys_player));
			}
			else
			{
				flags = phchr->Move(move, phys_mask(phys_physobjects, phys_character, phys_player, phys_playerctrl));
			}
		}
	}

	if (!IsActive() && chr->IsShow() /*&& !chr->logic->IsPairMode()*/ && !chr->ragdoll)	
	{
		pos += move;
		bNeedCalcModelMatrix = true;

		move = 0.0f;

		return;	
	}

	if(flags & IPhysCharacter::cf_down)
	{
		velY = 0.0f;
	}

	move = 0.0f;
	
	Vector dltpos = pos;
	pos = phchr->GetPosition();
	pos.y -= height*0.5f;

	bNeedCalcModelMatrix = true;
}

void CharacterPhysics::Move(const Vector & vDir,bool collide_with_characters)
{
	if (!bAllowMove && !chr->logic->IsActor() && !chr->logic->IsPairMode())
	{
		return;
	}

	if (phchr)
	{		
		if (collide_with_characters)
		{
			phchr->Move(vDir, phys_mask(phys_physobjects, phys_character, phys_playerctrl));
		}
		else
		{
			phchr->Move(vDir, phys_mask(phys_physobjects,phys_character));
			//phchr->Move(vDir, 0);
		}
		
		bNeedCalcModelMatrix = true;
	}
}


//Дать пинок персонажу с заданном направлении (2D)
void CharacterPhysics::Kick(const Vector & dir)
{
	kickDir = dir.GetXZ();
	kickState = true;
}

//Состояния полёта в пинке
bool CharacterPhysics::IsKickState()
{
	return kickState;
}

//Остановить состояние полёта в пинке
void CharacterPhysics::StopKick()
{
	kickState = false;
}

void CharacterPhysics::SetMovePosition(Vector mv_pos)
{
	move_pos = mv_pos;
	move_dir = GetPos() - mv_pos;
	dist_to_move = move_dir.Normalize();	
}

void CharacterPhysics::MoveInPosition(float dltTime)
{
	float k = 2.5f;
	if (dist_to_move>0)
	{		
		float dst_mv = dltTime * k;

		dist_to_move -= dltTime * k;

		if (dist_to_move<0)
		{			
			dst_mv += dist_to_move;
			dist_to_move = 0.0f;
		}		

		SetPos(pos + move_dir * dst_mv);
	}
}

void CharacterPhysics::ReCalcMatrices()
{
	if (!phchr) return;

	bNeedCalcModelMatrix = true;
	pos = phchr->GetPosition();
	pos.y -= height*0.5f;
}

void CharacterPhysics::CalcMatrices()
{
	if (bNeedCalcModelMatrix)
	{
		bNeedCalcModelMatrix = false;

		if (IsUseFakeModelPosition())
		{
			mModelMatrix.Build(Vector(0.0f, ay, az), fake_pos);
		}
		else
		{
			mModelMatrix.Build(Vector(0.0f, ay, az), GetPos());
		}

		mModelBoneMatrix = chr->GetBoneMatrix(chr->hips_boneIdx) * mModelMatrix;
	}
}

/*class CapsuleTestor : public MissionObject
{
public:
	CapsuleTestor();
	virtual ~CapsuleTestor() {}

	void _cdecl Work(float dltTime, long level);
	
	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

private:
	IPhysCharacter * phchr;	//Физическое представление персонажа
	Vector pos;
	float height;
	float radius;
};

CapsuleTestor::CapsuleTestor()
{
	phchr = null;
}

bool CapsuleTestor::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool CapsuleTestor::EditMode_Update(MOPReader & reader)
{
	pos = reader.Position();
	height = reader.Float();
	radius = reader.Float();

	RELEASE(phchr);

	phchr = Physics().CreateCharacter(_FL_, radius, height);
	phchr->Activate(true);
	phchr->SetPosition(pos);

	SetUpdate(&CapsuleTestor::Work, ML_EXECUTE1);

	return true;
}

void _cdecl CapsuleTestor::Work(float dltTime, long level)
{
	Vector mv = 0.0f;
	if (api->DebugKeyState('W'))
		mv += Vector(0.0f, 0.0f, 1.0f);
	if (api->DebugKeyState('S'))
		mv += Vector(0.0f, 0.0f, -1.0f);
	if (api->DebugKeyState('A'))
		mv += Vector(-1.0f, 0.0f, 0.0f);
	if (api->DebugKeyState('D'))
		mv += Vector(1.0f, 0.0f, 0.0f);

	mv.Normalize();
	mv *= 2.0f;
	mv.y -= 0.3f;

	phchr->Move(mv * dltTime, phys_mask(phys_character));
}

MOP_BEGINLIST(CapsuleTestor, "CapsuleTestor", '1.00', 100)
	MOP_POSITION("Pos", 0.0f)
	MOP_FLOAT("Height", 2.3f)
	MOP_FLOAT("Radius", 0.8f)
MOP_ENDLIST(CapsuleTestor)*/