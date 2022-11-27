#ifndef _VERLET_H_
#define _VERLET_H_


#include "..\..\..\..\common_h\mission\mission.h"


class VerletSolver
{
private:

	

	struct Particle
	{
		Vector vPos;
		Vector vOldPos;
		Vector vForce;
		bool bIsStatic;
	};

	struct Constraint
	{
		dword particleA;
		dword particleB;
		float restlength;
	};


	array<Particle> particles;
	array<Constraint> constraints;



	void Solve();
	void SatisfyConstraints();
	

public:

	VerletSolver();
	~VerletSolver();

	void ApplyForces();
	void Work();

	dword AddParticle (const Vector& pos, bool bIsStatic);
	void AddConstraint (dword from, dword to);


	dword GetParticlesCount ();
	const Vector& GetParticlePos (dword index);
	bool IsParticlesStatic (dword index);

	dword GetConstraintsCount ();
	void GetConstraintsData (dword index, dword& i1, dword& i2);

};

class Verlet : public MissionObject
{
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);

	VerletSolver vs;
};

#endif