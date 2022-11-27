#include "..\pch.h"
#include ".\brokensystemmaker.h"
#include "..\..\common_h\physics.h"

#include "Ship.h"
#include "Mast.h"
#include "Yard.h"
#include "Rope.h"
#include "Sail.h"

IPhysRigidBodyConnector* MakeJoint(IPhysicsScene& scene, IPhysRigidBody& a, IPhysRigidBody& b)
{
	if (&a == &b)
		return NULL;

	float aMass = a.GetMass();
	float bMass = b.GetMass();

	IPhysRigidBodyConnector* joint = scene.Connect( &a, &b, Min(aMass*bMass, 1.0f + 1.0f*rand()/(float)RAND_MAX) );
	//a.ApplyImpulse(Vector().Rand()*ExplosionPower*Vector(1.0f, 0.4f, 1.0f), Vector().Rand()*2.0f);
	//b.ApplyImpulse(Vector().Rand()*ExplosionPower*Vector(1.0f, 0.5f, 1.0f), Vector().Rand()*2.0f);
	return joint;
}


void BrokenSystemMaker::Make()
{
//	for (unsigned int i = 0; i < sailesToReattach_.Size(); ++i)
//		sailesToReattach_[i]->Detach();

	for (unsigned int i = 0; i < ropesToReattach_.Size(); ++i)
		ropesToReattach_[i]->Detach();
	
	for (unsigned int i = 0; i < sailesToReattach_.Size(); ++i)
	{
		sailesToReattach_[i]->BeginDestruction( totalBreak_ ? 3.f : 0.8f );
		// закоментил присоединение паруса к отваливающимся частям, так как
		// части из них не назначена корректная кость, и тогда парус останется в воздухе
		//for (unsigned int j = 0; j < system_.Size(); ++j)
		//	sailesToReattach_[i]->Attach(*(system_[j].actor));
	}

	//for (unsigned int i = 0; i < ropesToReattach_.Size(); ++i)
	//	ropesToReattach_[i]->Attach();
}

void BrokenSystemMaker::StandardBreak(ShipPart& part, const Matrix& orient)
{
	if ( !part.GetShapes().Size() )
		return;

	ShipPart::SystemItem	item;
	Matrix mtx;
	Vector boxSize;

	const float massvalue = 0.1f;

	physShip_.GetTransform(mtx);
	IPhysCombined * element = scene_.CreateCombined(_FL_, mtx);

	for (unsigned int i = 0; i < part.GetShapes().Size(); ++i)
	{
		const unsigned int index = part.GetShapes()[i];

		// добавляем шейп к актору разрушаемой системы
		physShip_.GetBox(index, boxSize);
		physShip_.GetLocalTransform(index, mtx);
		element->AddBox(boxSize*2.0f, mtx); // домножаем размеры на 2, т.к. в физике используются полуразмеры

		// отключаем соответствующий шейп на корабле
		physShip_.EnableCollision(index, false);
		physShip_.EnableResponse(index, false);
		physShip_.EnableRaycast(index, false);

		physShip_.SetGlobalTransform(index, Matrix(0.0f, -100.0f, 0.0f));
	}

	element->Build();
	element->SetSolverIterations(1);
	element->SetMotionDamping(damping_);
	element->SetMass(massvalue);
	element->SetGroup(phys_player);

	Matrix mt(true);
	part.GetMOOwner()->GetMatrix(mt);

	if( part.GetShipOwner() )
	{
		Vector vSpeed =  mt.MulNormal( Vector(0.f,0.f,part.GetShipOwner()->GetCurrentVelocity()) );
		element->ApplyImpulse( vSpeed * massvalue, 0.f );
	}

	item.actor = element;
	item.active = true;
	item.boneIndex = -1;//part.GetOwner()->GetAniBlender().SetActor(&part, element);
	item.joint = NULL;
	item.vWakeVelocity = 0.f;
	part.SetPhysActor(element);

	system_.Add(item);
	system_.LastE().joint = MakeJoint(	scene_,
										*system_[Max(0, (int)system_.Last()-1)].actor,
										*system_[system_.Last()].actor );

	Vector vExplodeForce = Vector().RandXZ();
	vExplodeForce.y = part.GetDestroyImpulsePower();
	vExplodeForce *= ExplosionPower;
	// проверим что бы направление отлета разрушаемой части было во внешнюю сторону
	vExplodeForce.x = (mtx.pos.x>0.f) ? fabs(vExplodeForce.x) : -fabs(vExplodeForce.x);
	Vector vExplodePos = Vector().Rand()*2.0f;
	element->ApplyImpulse( mt.MulNormal(vExplodeForce), vExplodePos );
}


void BrokenSystemMaker::StandardMake(ShipPart& part, const Matrix& orient)
{
	if (part.IsBroken())
	{
		for (unsigned int i = 0; i < part.GetBrokenSystem().Size(); ++i)
			system_.Add(part.GetBrokenSystem()[i]);

		part.CleanBrokenSystem();
		return;
	}

	if (&part == beginToMakeFrom_)
	{
		haveBreaks_ = true;
		isMaking_ = true;
	}

	if (isMaking_)
	{
		part.SetBroken(true);
		StandardBreak(part, orient);
	}

	for (unsigned int i = 0; i < part.GetChildren().Size(); ++i)
		part.GetChildren()[i]->AcceptVisitor(*this);

	if (&part == beginToMakeFrom_)
	{
		isMaking_ = false;
	}
}


void BrokenSystemMaker::Visit(ShipPart& part)
{
	StandardMake(part);
}


void BrokenSystemMaker::Visit(Ship& part)
{
	StandardMake(part);
}


void BrokenSystemMaker::Visit(HullPart& part)
{
	//damping_ = 4+2*rand()/(float)RAND_MAX;
	StandardMake(part);
	//damping_ = 0.01f;

	if (totalBreak_)
	{
		part.StopHits();
	}
}


void BrokenSystemMaker::Visit(Mast& part)
{
	haveBreaks_ = false;

	if( part.GetMOOwner() )
	{
		Matrix mtx;
		part.GetMOOwner()->GetMatrix(mtx);
		orient_ = part.GetRotationLocal(mtx);
	}

	StandardMake(part);

	if (haveBreaks_ || totalBreak_)
	{
		SailHelper helper(sailesToReattach_);

		for (unsigned int i = 0; i < part.GetChildren().Size(); ++i)
			part.GetChildren()[i]->AcceptVisitor(helper);
	}
	orient_.SetIdentity();
	haveBreaks_ = false;
}

void BrokenSystemMaker::Visit(MastPart& part)
{
	StandardMake(part);
}


void BrokenSystemMaker::Visit(Yard& part)
{
	//bool bOldHaveBreaks = haveBreaks_;

	StandardMake(part);

	// если ломается только эта рея, то просматриваем всю мачту и оповещаем паруса об отрыве реи
	//if( !bOldHaveBreaks && haveBreaks_ && part.GetParent() && part.GetParent()->GetParent() )
	if( (haveBreaks_ || totalBreak_) && part.GetParent() && part.GetParent()->GetParent() )
	{
		unsigned int i;
		array<Sail*> apSail(_FL_);
		SailHelper helper(apSail);
		if( part.GetShipOwner() )
				part.GetShipOwner()->AcceptVisitor(helper);
		/*ShipPart* pMast = part.GetParent()->GetParent();
		for (i = 0; i < pMast->GetChildren().Size(); ++i)
			pMast->GetChildren()[i]->AcceptVisitor(helper);*/

		for (i = 0; i < apSail.Size(); i++)
		{
			apSail[i]->EventPartBreak(&part);
			/*if( apSail[i]->EventPartBreak(&part) )
			{
				apSail[i]->Attach( *system_.LastE().actor );
			}*/
		}
	}
}


void BrokenSystemMaker::Visit(Rope& part)
{
	ropesToReattach_.Add(&part);
}


void BrokenSystemMaker::Visit(Sail& part)
{
	if (isMaking_)
		part.ActivateBreakParams();
}
