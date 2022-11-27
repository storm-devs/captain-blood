#pragma once
#include "partvisitor.h"
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\templates\array.h"

class PartsClassificator : public IPartVisitor
{
	array<HullPart*>	hullParts_;
	array<MastPart*>	mastParts_;
	array<Mast*>		masts_;
	array<Yard*>		yards_;
	array<Sail*>		sails_;
	array<Rope*>		ropes_;

	unsigned int		totalCount_;
	unsigned int		brokenCount_;


	void VisitChildren(ShipPart&);
public:
	PartsClassificator(void);
	virtual ~PartsClassificator(void);

	virtual void Visit(ShipPart& part) { VisitChildren(part); }

	virtual void Visit(Ship&);
	virtual void Visit(HullPart&);
	virtual void Visit(MastPart&);
	virtual void Visit(Mast&);
	virtual void Visit(Yard&);
	virtual void Visit(Sail&);
	virtual void Visit(Rope&);

	virtual void Reset();


	array<HullPart*>&	GetHullParts() { return hullParts_; }
	array<MastPart*>&	GetMastParts() { return mastParts_; }
	array<Mast*>&		GetMasts() { return masts_; }
	array<Yard*>&		GetYards() { return yards_; }
	array<Sail*>&		GetSails() { return sails_; }
	array<Rope*>&		GetRopes() { return ropes_; }

	unsigned int GetTotalPartsCount() const { return totalCount_; }
	unsigned int GetBrokenPartsCount() const { return brokenCount_; }
};
