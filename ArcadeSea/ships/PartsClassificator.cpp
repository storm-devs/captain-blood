#include "..\pch.h"
#include ".\PartsClassificator.h"

#include "Ship.h"
#include "Mast.h"
#include "Yard.h"
#include "Rope.h"
#include "Sail.h"

void SailHelper::VisitChildren(ShipPart& part)
{
	for (unsigned int i = 0; i < part.GetChildren().Size(); i++ )
		part.GetChildren()[i]->AcceptVisitor(*this);
}
void SailHelper::Visit(Ship& part) {VisitChildren(part);}
void SailHelper::Visit(Mast& part) {VisitChildren(part);}


PartsClassificator::PartsClassificator(void) :
	hullParts_(__FILE__, __LINE__),
	mastParts_(__FILE__, __LINE__, 16),
	masts_(__FILE__, __LINE__),
	yards_(__FILE__, __LINE__, 32),
	sails_(__FILE__, __LINE__, 16),
	ropes_(__FILE__, __LINE__),
	totalCount_(0),
	brokenCount_(0)
{
}

PartsClassificator::~PartsClassificator(void)
{
}

void PartsClassificator::VisitChildren(ShipPart& part)
{
	++totalCount_;
	if (part.IsBroken())
		++brokenCount_;

	for (unsigned int i = 0; i < part.GetChildren().Size(); ++i )
		part.GetChildren()[i]->AcceptVisitor(*this);
}

void PartsClassificator::Visit(Ship& part)
{
	VisitChildren(part);
}

void PartsClassificator::Visit(HullPart& part)
{
	hullParts_.Add(&part);
	VisitChildren(part);
}

void PartsClassificator::Visit(MastPart& part)
{
	mastParts_.Add(&part);
	VisitChildren(part);
}

void PartsClassificator::Visit(Mast& part)
{
	masts_.Add(&part);
	VisitChildren(part);
}

void PartsClassificator::Visit(Yard& part)
{
	yards_.Add(&part);
	VisitChildren(part);
}

void PartsClassificator::Visit(Sail& part)
{
	sails_.Add(&part);
	VisitChildren(part);
}

void PartsClassificator::Visit(Rope& part)
{
	ropes_.Add(&part);
	VisitChildren(part);
}

void PartsClassificator::Reset()
{
	hullParts_.Empty();
	mastParts_.Empty();
	masts_.Empty();
	yards_.Empty();
	sails_.Empty();
	ropes_.Empty();

	totalCount_ = 0;
	brokenCount_ = 0;
}