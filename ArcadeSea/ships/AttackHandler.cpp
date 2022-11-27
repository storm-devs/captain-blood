#include "..\pch.h"
#include ".\attackhandler.h"
#include "..\..\Common_h\Physics.h"

#include "Ship.h"
#include "Mast.h"
#include "Yard.h"
#include "Rope.h"
#include "Sail.h"

void AttackHandler::Init(const Vector& vStart, const Vector& vEnd, IPhysicsScene& phys)
{
	damage_ = 0.f;
	transferedDamage_ = 0.0f;
	type_ = LineAttack;
	continue_ = true;
	vLineStart_ = vStart;
	vLineEnd_ = vEnd;
	hitDetected_ = false;
	isFlamethrower_ = false;
	isTrace_ = true;
	hitPos_ = vEnd;
	nDamageChildRecursiveLevel = 0;
	bSkipSails = false;
	bNoTearSails = false;
}

void AttackHandler::Init(const Vector& vStart, const Vector& vEnd, float dmg, IPhysicsScene& phys, bool isFlamethrower)
{
	damage_ = dmg;
	transferedDamage_ = 0.0f;
	type_ = LineAttack;
	continue_ = true;
	vLineStart_ = vStart;
	vLineEnd_ = vEnd;
	hitDetected_ = false;
	isFlamethrower_ = isFlamethrower;
	isTrace_ = false;
	hitPos_ = vEnd;
	nDamageChildRecursiveLevel = 0;
	bSkipSails = false;
	bNoTearSails = false;
}

void AttackHandler::Init(const Vector& center, float radius, float dmg, IPhysicsScene& phys)
{
	damage_ = dmg;
	transferedDamage_ = 0.0f;
	type_ = SphereAttack;
	continue_ = true;
	hitDetected_ = false;
	vSphCenter_ = center;
	sphRadius_ = radius;
	isFlamethrower_ = false;
	isTrace_ = false;
	hitPos_ = center;
	nDamageChildRecursiveLevel = 0;
	bSkipSails = false;
	bNoTearSails = false;
}

void AttackHandler::Init(const Vector vrt[4], float dmg, IPhysicsScene& phys)
{
	damage_ = dmg;
	transferedDamage_ = 0.0f;
	type_ = ConvexQuadrangleAttack;
	continue_ = true;
	hitDetected_ = false;
	isFlamethrower_ = false;
	isTrace_ = false;
	hitPos_ = 0.f;
	nDamageChildRecursiveLevel = 0;
	bSkipSails = false;
	bNoTearSails = false;
}

AttackHandler::~AttackHandler(void)
{
}

void AttackHandler::VisitChildren(ShipPart& part)
{
	for (unsigned int i = 0; i < part.GetChildren().Size(); ++i)
		part.GetChildren()[i]->AcceptVisitor(*this);
}

bool AttackHandler::LineHit(ShipPart& part, Vector& pos)
{
	if (part.IsBroken())
		return false;

	IPhysCombined::RaycastResult details;
	for (unsigned int i = 0; i < part.GetShapes().Size(); ++i)
	{
		if ( part.GetShipOwner() && part.GetShipOwner()->GetPhysView()->Raycast(part.GetShapes()[i],
			vLineStart_, vLineEnd_, &details) )
		{
			pos = details.position;
			return true;
		}
	}

	return false;
}

bool AttackHandler::SphereHit(ShipPart& part, Vector& pos)
{
	if (part.IsBroken())
		return false;

	IPhysCombined::RaycastResult details;
	Matrix mtx;

	for (unsigned int i = 0; i < part.GetShapes().Size(); ++i)
	{
		const unsigned int index = part.GetShapes()[i];

		if ( part.GetShipOwner() && part.GetShipOwner()->GetPhysView()->OverlapSphere(index, vSphCenter_, sphRadius_) )
		{
			part.GetShipOwner()->GetPhysView()->GetGlobalTransform(index, mtx);
			part.GetShipOwner()->GetPhysView()->Raycast(index, vSphCenter_, mtx.pos, &details);

			pos = details.position;
			return true;
		}
	}

	return false;
}

void AttackHandler::Visit(ShipPart& part)
{
	VisitChildren(part);
}

void AttackHandler::Visit(Ship& part)
{
	VisitChildren(part);
}

void AttackHandler::Visit(HullPart& part)
{
	Vector v;
 	if (LineAttack == type_ && LineHit(part, v))
	{
		hitDetected_ = true;
		hitPos_ = v;
		if( !isTrace_ )
			transferedDamage_ = part.HandleDamage(v, damage_, isFlamethrower_, nDamageChildRecursiveLevel);
		return;
	}
	
	if (SphereAttack == type_ && SphereHit(part, v))
	{
		hitDetected_ = true;
		if( !isTrace_ )
			transferedDamage_ += part.HandleDamage(v, damage_, false, nDamageChildRecursiveLevel);
		return;
	}
	
	VisitChildren(part);
}

void AttackHandler::Visit(MastPart& part)
{
	Vector v;
	if (LineAttack == type_ && LineHit(part, v))
	{
		hitDetected_ = true;
		hitPos_ = v;
		if( !isTrace_ )
			transferedDamage_ = part.HandleDamage(v, damage_);
		return;
	}

	if (SphereAttack == type_ && SphereHit(part, v))
	{
		hitDetected_ = true;
		if( !isTrace_ )
			transferedDamage_ += part.HandleDamage(v, damage_);
		return;
	}

	VisitChildren(part);
}

void AttackHandler::Visit(Mast& part)
{
	VisitChildren(part);
}

void AttackHandler::Visit(Yard& part)
{
	Vector v;
	if (LineAttack == type_ && LineHit(part, v))
	{
		hitDetected_ = true;
		hitPos_ = v;
		if( !isTrace_ )
			transferedDamage_ = part.HandleDamage(v, damage_);
		return;
	}
	if (SphereAttack == type_ && SphereHit(part, v))
	{
		hitDetected_ = true;
		if( !isTrace_ )
			transferedDamage_ += part.HandleDamage(v, damage_);
		return;
	}
    
	VisitChildren(part);
}

void AttackHandler::Visit(Rope& part)
{

}

void AttackHandler::Visit(Sail& part)
{
	if (LineAttack == type_ && !bNoTearSails)
	{
		bool bIsHit = part.Attack(vLineStart_, vLineEnd_, isTrace_ );
		if( bIsHit && !bSkipSails )
			hitDetected_ = true;
	}
}
