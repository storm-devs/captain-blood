#pragma once
#include "partvisitor.h"
#include "..\..\Common_h\math3D.h"

class NxShape;
class IPhysicsScene;
class AttackHandler : public IPartVisitor
{
	enum AttackType { LineAttack, SphereAttack, ConvexQuadrangleAttack };

	float		damage_;			// величина дэмэджа
	float		transferedDamage_;	// кол-во дэмэджа перенесенного на корабль при повреждении частей
	AttackType	type_;				// тип атаки
	bool		continue_;			// флаг продолжения распростарнения повреждений
	bool		isFlamethrower_;
	bool		isTrace_;
	long		nDamageChildRecursiveLevel;

	Vector		vLineStart_;		//
	Vector		vLineEnd_;			//

	Vector		vSphCenter_;		//
	float		sphRadius_;			//

	Vector		quadVx_[4];			//

	bool		hitDetected_;		// флаг наличия попадания
	Vector		hitPos_;			// точка последнего попадания

	bool		bSkipSails;			// попадание в парус не засчитывается (но парус рвется)
	bool		bNoTearSails;		// не рвать парус

	bool	LineHit(ShipPart&, Vector&);
	bool	SphereHit(ShipPart&, Vector&);
	void	VisitChildren(ShipPart&);

public:
	AttackHandler() {}
	AttackHandler(const Vector& src, const Vector& dst, IPhysicsScene& phys) {Init(src,dst,phys);}
	AttackHandler(const Vector& src, const Vector& dst, float dmg, IPhysicsScene& phys, bool isFlamethrower = false) {Init(src,dst,dmg,phys,isFlamethrower);}
	AttackHandler(const Vector& c, float r, float dmg, IPhysicsScene& phys) {Init(c,r,dmg,phys);}
	AttackHandler(const Vector vrt[4], float dmg, IPhysicsScene& phys) {Init(vrt,dmg,phys);}

	void Init(const Vector&, const Vector&, IPhysicsScene&);
	void Init(const Vector&, const Vector&, float, IPhysicsScene&, bool isFlamethrower);
	void Init(const Vector&, float, float, IPhysicsScene&);
	void Init(const Vector vrt[4], float, IPhysicsScene&);

	virtual ~AttackHandler(void);

	float	TransferedDamage() const { return transferedDamage_; }
	bool	HitDetected() const { return hitDetected_; }
	const Vector& GetLastHitPos() { return hitPos_; }
	void SetChildRecursionLevel(long nLvl) {nDamageChildRecursiveLevel=nLvl;}
	void SetSailsSkip(bool bSkip) {bSkipSails=bSkip;}
	void SetNoTear(bool bNoTear) {bNoTearSails=bNoTear;}

	virtual void Visit(ShipPart&);
	virtual void Visit(Ship&);
	virtual void Visit(HullPart&);
	virtual void Visit(MastPart&);
	virtual void Visit(Mast&);
	virtual void Visit(Yard&);
	virtual void Visit(Rope&);
	virtual void Visit(Sail&);

	virtual void Reset() {}
};
