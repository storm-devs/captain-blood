
#ifndef ShooterTargets_h
#define ShooterTargets_h

#include "..\..\..\Common_h\Mission.h"

#define COLLIDERS_GROUP phys_grp2
#define COLLIDERS_GROUP_INVISIBLE phys_grp4

class ShooterTargets : public MissionObject
{
public:

	struct TargetObjects
	{
		ConstString HitParticles;
		ConstString HitSound;
		MOSafePointer objsp;
		ConstString ObjectName;
		bool bShowCollider;
		IGMXScene* Collider;
		IPhysRigidBody* physCollider;		
		int Life;
		int MaxLife;
		bool bPassiveTarget;

		MissionTrigger OnDeadTrigger;
		MissionTrigger OnHitTigger;
	};
	

	
	array<TargetObjects> TargetList;

	ShooterTargets();
	~ShooterTargets();

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);	

	void HitMoment(int index, const Vector & position, const Vector & normal);
	void BuildPointers();
	void UpdateColliders();

	void UpdateColliderPositions(bool isUpdatePhysObjects);

	bool FindNearestObject(const Vector & from, const Vector & dir, float maxDist, IPhysBase* & physObj, Vector& pos, float& dist);
	bool IsMyCollider(IPhysBase* physObj, bool &bPassiveTarget);

	Vector GetRandomPoint(const Vector & pos, Sphere & sph);

	bool CheckHit(const IPhysBase * trg, const Vector & pos, const Vector & nrm);

	void DrawColliders();

	void RestoreLife();

	MO_IS_FUNCTION(ShooterTargets, MissionObject);

private:

	void InitParams		(MOPReader &reader);
};

#endif
