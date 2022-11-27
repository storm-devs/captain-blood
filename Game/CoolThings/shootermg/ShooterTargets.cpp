#include "ShooterTargets.h"
#include "..\..\..\Common_h\ICharacter.h"

ShooterTargets::ShooterTargets():TargetList(_FL_)
{

}

ShooterTargets::~ShooterTargets()
{
	for (long i = 0; i < TargetList; i++)
	{		
		if (TargetList[i].physCollider)		
		{
			TargetList[i].physCollider->Release();
			TargetList[i].physCollider = null;
		}

		if (TargetList[i].Collider)
		{
			TargetList[i].Collider->Release();
			TargetList[i].Collider = null;
		}
	}
}

bool ShooterTargets::Create(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool ShooterTargets::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void ShooterTargets::InitParams(MOPReader &reader)
{	
	TargetList.DelAll();

	if (EditMode_IsOn()) return;

	int ArrayCount = reader.Array();
	for (long i = 0; i < ArrayCount; i++)
	{
		TargetObjects to;
		to.ObjectName = reader.String();		
		to.objsp.Reset();
		const char* s = reader.String().c_str();

		to.Collider = Geometry().CreateGMX(s, &Animation(), &Mission().Particles(), &Mission().Sound());
		to.physCollider = null;

		if (to.Collider)
		{
			to.physCollider = to.Collider->CreatePhysicsActor(Physics(), true);

			if (to.physCollider)
			{
				to.physCollider->EnableKinematic(true);
				to.physCollider->SetGroup(COLLIDERS_GROUP);
				//to.physCollider->EnableCollision(false);
				//to.physCollider->EnableKinematic(true);
			}			
		}		

		to.bShowCollider = reader.Bool();
		to.HitParticles = reader.String();
		to.HitSound = reader.String();
		to.Life = reader.Long();
		to.MaxLife = to.Life;

		to.bPassiveTarget = reader.Bool();

		to.OnDeadTrigger.Init(reader);
		to.OnHitTigger.Init(reader);

		TargetList.Add(to);
	}
}

void ShooterTargets::HitMoment(int index, const Vector & position, const Vector & normal)
{
	ConstString snd;
	ConstString particle;

	if (TargetList[index].Life > 0)
	{
		LogicDebug("Shooter: On hit trigger activate !");
		TargetList[index].OnHitTigger.Activate(Mission(), false);
		TargetList[index].Life--;
	}

	if (TargetList[index].Life <= 0)
	{
		MissionObject* mo = Mission().Player();

		if (mo)
		{
			if (mo->Is(ICharacter::GetTypeId()))
			{
				ICharacter* chr = (ICharacter*)mo;

				//chr->AchievementReached("40 Gunmaster");
			}
		}		

		LogicDebug("Shooter: On dead trigger activate !");
		TargetList[index].OnDeadTrigger.Activate(Mission(),false);
	}

	snd = TargetList[index].HitSound;
	particle = TargetList[index].HitParticles;			
	
	Sound().Create3D(snd, position, _FL_);

	IParticleSystem* pSys = Particles().CreateParticleSystem(particle.c_str());
	
	if (pSys)
	{
		Matrix mat(true);
		mat.BuildOrient(normal,Vector(0.0f,1.0f,0.0f));
		mat.pos = position;

		pSys->Teleport(mat);
		pSys->AutoDelete(true);
	}
}

void ShooterTargets::BuildPointers()
{
	for (dword i = 0; i < TargetList.Size(); i++)
	{
		FindObject ( TargetList[i].ObjectName, TargetList[i].objsp );
	}
}

void ShooterTargets::UpdateColliders()
{
	for (dword n = 0 ; n < TargetList.Size(); n++)
	{		
		if (!TargetList[n].physCollider) continue;
		if (!TargetList[n].objsp.Validate()) continue;


		bool bActive = TargetList[n].objsp.Ptr()->IsActive();
		bool bVisible = TargetList[n].objsp.Ptr()->IsShow();
		bool bAllive = (TargetList[n].objsp.Ptr()->GetHP()>0);

		//Если неактивный или невидимы переносим в группу, что бы луч в него не попадал
		if (!bActive || !bVisible || !bAllive)		
		{
			TargetList[n].physCollider->SetGroup(COLLIDERS_GROUP_INVISIBLE);
		} else
		{
			//Если неактивный или невидимы переносим в группу, что бы луч в него ПОПАДАЛ
			TargetList[n].physCollider->SetGroup(COLLIDERS_GROUP);
		}
	}	
}

void ShooterTargets::UpdateColliderPositions(bool isUpdatePhysObjects)
{
	dword count = TargetList.Size();
	ShooterTargets::TargetObjects * targetsList = TargetList.GetBuffer();
	for (dword i = 0; i < count; i++)
	{
		ShooterTargets::TargetObjects & target = targetsList[i];
		if( !target.objsp.Validate() ) continue;
		bool havePhysObject = (isUpdatePhysObjects && target.physCollider != null);
		if ( target.Collider != null && !havePhysObject ) continue;
		Matrix mat;
		target.objsp.Ptr()->GetMatrix(mat);
		if(target.Collider)
		{
			target.Collider->SetTransform(mat);
		}
		if(havePhysObject)
		{
			target.physCollider->SetTransform(mat);
		}
	}
}

bool ShooterTargets::FindNearestObject(const Vector & from, const Vector & dir, float maxDist, IPhysBase* & physObj, Vector& pos, float& dist)
{
	dword count = TargetList.Size();
	float maxDist2pow = maxDist * maxDist;
	dword findedIdx = count;
	float curOffset = 1e+6f;
	ShooterTargets::TargetObjects * targetsList = TargetList.GetBuffer();
	for (dword j = 0; j < count; j++)
	{
		ShooterTargets::TargetObjects & target = targetsList[j];
		if ( target.physCollider && target.physCollider->GetGroup()==COLLIDERS_GROUP && !target.bPassiveTarget )
		{
			Matrix mtx(true);
			target.physCollider->GetTransform( mtx );
			Vector dir2targ = mtx.pos - from;
			if( ~dir2targ > maxDist2pow ) continue;

			float offset2pow = ~(dir2targ ^ dir);
			if( offset2pow < curOffset )
			{
				curOffset = offset2pow;
				findedIdx = j;
			}
		}
	}

	if( findedIdx < count )
	{
		physObj = targetsList[findedIdx].physCollider;
		Matrix mtx(true);
		targetsList[findedIdx].physCollider->GetTransform( mtx );
		pos = mtx.pos;
		dist = sqrtf( curOffset );
		return true;
	}
	return false;
}

bool ShooterTargets::IsMyCollider(IPhysBase* physObj, bool &bPassiveTarget)
{
	dword count = TargetList.Size();
	ShooterTargets::TargetObjects * targetsList = TargetList.GetBuffer();
	for (dword j = 0; j < count; j++)
	{	
		ShooterTargets::TargetObjects & target = targetsList[j];
		if (target.physCollider == physObj)
		{
			bPassiveTarget = target.bPassiveTarget;
			return true;
		}
	}
	return false;
}

Vector ShooterTargets::GetRandomPoint(const Vector & pos, Sphere & sph)
{
	// рандомная точка в сфере
	Vector v;
	v.Rand();
	v *= rand()*(sph.r*(1.0f/RAND_MAX));
	v += sph.p;

	// приближение от рандомной точки к стартовой точке,
	// что бы новое попадание не вышло за пределы цели
	Vector vdelta = (pos - v) * 0.2f;
	for( long n=0; n<5; n++ )
	{
		// проверяем попадает ли текущая точка в таргеты
		dword count = TargetList.Size();
		ShooterTargets::TargetObjects * targetsList = TargetList.GetBuffer();
		for (dword j = 0; j < count; j++)
		{	
			ShooterTargets::TargetObjects & target = targetsList[j];
			if( v.InBox( target.Collider->GetBound().vMin, target.Collider->GetBound().vMax ) )
				return v;
		}
		v += vdelta;
	}
	return pos;
//	return v;
}

bool ShooterTargets::CheckHit(const IPhysBase * trg, const Vector & pos, const Vector & nrm)
{
	dword count = TargetList.Size();
	ShooterTargets::TargetObjects * targetsList = TargetList.GetBuffer();
	for (dword j = 0; j < count; j++)
	{	
		ShooterTargets::TargetObjects & target = targetsList[j];
		if (target.physCollider == trg && target.Life>0)
		{
			HitMoment(j, pos, nrm);
			return true;
		}					
	}
	return false;
}

void ShooterTargets::DrawColliders()
{
	dword count = TargetList.Size();
	ShooterTargets::TargetObjects * targetsList = TargetList.GetBuffer();
	for (dword j = 0; j < count; j++)
	{
		ShooterTargets::TargetObjects & target = targetsList[j];
		if (!target.Collider) continue;		
		if (!target.bShowCollider) continue;
		if (!target.physCollider) continue;
		if (!target.objsp.Validate()) continue;

		bool bActive = target.objsp.Ptr()->IsActive();
		bool bVisible = target.objsp.Ptr()->IsShow();
		bool bAllive = (target.objsp.Ptr()->GetHP()>0);

		if ( bActive && bVisible && bAllive)
		{
			Matrix mat;
			target.objsp.Ptr()->GetMatrix(mat);

			target.Collider->SetTransform(mat);
			target.Collider->Draw();

			Render().Print(mat.pos + Vector (0,1.0f,0.0f),15,0,0xffffffff,target.objsp.Ptr()->GetObjectID().c_str());
		}
	}

}

void ShooterTargets::RestoreLife()
{
	for (dword n = 0 ; n < TargetList.Size(); n++)
	{
		TargetList[n].Life = TargetList[n].MaxLife;
	}
}


MOP_BEGINLIST(ShooterTargets, "Shooter Targets", '1.00', 50)

	MOP_ARRAYBEGC("Target objects", 0, 100, "Список шареных между шутеров объектов в которые можно стрелять\nсм. подробный коментарий к каждому элементу массива")
		MOP_STRINGC("Mission object name", "", "Имя объекта миссии в который происходит стрельба")
		MOP_STRINGC("Geometry collider", "", "Имя файла коллидера используемого для проверки попадания снаряда\nGMX файл")
		MOP_BOOLC("Show collider", false, "Показать коллидер, для отладки стрельбы")
		MOP_STRINGC("HitParticles", "", "Партиклы рождаемые при попадании в этот объект\nXPS файл");
		MOP_STRINGC("HitSound", "", "3D Звук проигрываемый при попадании в объект\nИмя ALIASA");

		MOP_LONGC("Life", 100, "Жизнь объекта, при попадании отнимается 1");

		MOP_BOOLC("Passive target", false, "Пассивный таргет не подсвечивает прицел и не нуждается в автодоводке");

		MOP_MISSIONTRIGGER("Dead");
		MOP_MISSIONTRIGGER("OnHit");

	MOP_ARRAYEND

MOP_ENDLIST(ShooterTargets)


