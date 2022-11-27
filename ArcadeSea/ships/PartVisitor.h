#pragma once

class ShipPart;
class Ship;
class HullPart;
class MastPart;
class Mast;
class Yard;
class Rope;
class Sail;

// интерфейс Visitor'а для обхода иерархии частей корабля
class IPartVisitor
{
public:
	virtual ~IPartVisitor(void) {};

	virtual void Visit(ShipPart&) = 0; // уловитель

	virtual void Visit(Ship&) = 0;
	virtual void Visit(HullPart&) = 0;
	virtual void Visit(MastPart&) = 0;
	virtual void Visit(Mast&) = 0;
	virtual void Visit(Yard&) = 0;
	virtual void Visit(Rope&) = 0;
	virtual void Visit(Sail&) = 0;

	virtual void Reset() = 0;
};

class SailHelper : public IPartVisitor
{
	array<Sail*>&	sailArray_;

	virtual void Visit(ShipPart&) {}
	virtual void Visit(Ship& part);
	virtual void Visit(HullPart&) {}
	virtual void Visit(MastPart&) {}
	virtual void Visit(Mast& part);
	virtual void Visit(Yard&) {}
	virtual void Visit(Rope&) {}
	virtual void Visit(Sail& sail)
	{
		sailArray_.Add(&sail);
	}

	virtual void Reset() {sailArray_.Empty();}

	void VisitChildren(ShipPart& part);
public:
	SailHelper(array<Sail*>& _array) : sailArray_(_array) {}
};
