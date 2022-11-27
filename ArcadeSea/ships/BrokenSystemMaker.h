#pragma once
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\templates\array.h"
#include "PartVisitor.h"
#include "ShipPart.h"

class IPhysCombined;
class IPhysicsScene;

class BrokenSystemMaker : public IPartVisitor
{
	IPhysCombined&	physShip_;
	IPhysicsScene&	scene_;

	array<ShipPart::SystemItem>		system_;

	ShipPart						*beginToMakeFrom_;	// часть с которой начинать построение

	array<Sail*>					sailesToReattach_;	// паруса, которые надо реаттачить
	array<Rope*>					ropesToReattach_;	// веревки, которые надо реаттачить

	bool							isMaking_;			// добавлять/не добавлять часть в систему
	bool							haveBreaks_;		// добавлять/не добавлять часть в систему
	bool							totalBreak_;
	Matrix							orient_;

	float							ExplosionPower;

	float							damping_;

	void StandardBreak(ShipPart& part, const Matrix& orient = Matrix());
	void StandardMake(ShipPart& part, const Matrix& orient = Matrix());

public:
	BrokenSystemMaker(IPhysCombined& shipPhysView, IPhysicsScene& scene, bool isTotal=false, float explosePower=15.f) :
		physShip_(shipPhysView),
		scene_(scene),
		system_(__FILE__, __LINE__),
		sailesToReattach_(__FILE__, __LINE__),
		ropesToReattach_(__FILE__, __LINE__),
		isMaking_(false),
		damping_(0.01f),
		totalBreak_(isTotal),
		haveBreaks_(false),
		ExplosionPower(explosePower) {}
	virtual ~BrokenSystemMaker(void) {}

	//////////////////////////////////////////////////////////////////////////
	// Реализация IPartVisitor
	//////////////////////////////////////////////////////////////////////////
	virtual void Visit(ShipPart&); // уловитель

	virtual void Visit(Ship&);
	virtual void Visit(HullPart&);
	virtual void Visit(MastPart&);
	virtual void Visit(Mast&);
	virtual void Visit(Yard&);
	virtual void Visit(Rope&);
	virtual void Visit(Sail&);

	virtual void Reset() {}

	void SetStartNode(ShipPart* makeFrom) { beginToMakeFrom_ = makeFrom; }
	void Make();
	//////////////////////////////////////////////////////////////////////////

	array<ShipPart::SystemItem>& GetSystem() { return system_; }
};
