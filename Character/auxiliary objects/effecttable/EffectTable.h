
#ifndef _EffectTable_h_
#define _EffectTable_h_

#include "..\..\..\Common_h\Mission.h"

#define MAX_PARTICLES_IN_EFFECT_TABLE 10

class Character;
class EffectTable : public MissionObject
{
	struct MatParticle
	{
		float emmit_ps;
		ConstString particle;
		IParticleSystem* pSys;

		MatParticle()
		{
			pSys = NULL;
		}
	};

	struct MaterialEffect
	{
		PhysTriangleMaterialID mtl;
		ConstString sound;
		float minY, maxY;

		DWORD maxParticles;
		MatParticle sfx_particle[MAX_PARTICLES_IN_EFFECT_TABLE];


		MatParticle & getRandParticle()
		{
			Assert(maxParticles > 0);
			dword idx = rand() % maxParticles;
			return sfx_particle[idx];
		}

	};

	array<MaterialEffect> effects;
	array<MatParticle*> particles;

public:

	EffectTable();
	~EffectTable();

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void InitParams		(MOPReader &reader);
	PhysTriangleMaterialID GetEffectPhysMtl(const char* type);

	bool MakeEffect(PhysTriangleMaterialID mtl, Character * victim, Vector pos, Vector normal,IGMXScene * scene = null, GMXHANDLE * loc = null, bool isAttach = false);
	bool MakeChachedEffect(PhysTriangleMaterialID mtl, Character * victim, Vector pos, Vector normal,IGMXScene * scene = null, GMXHANDLE * loc = null, bool isAttach = false);

	virtual void _cdecl Work(float dltTime, long level);

	MO_IS_FUNCTION(EffectTable, MissionObject);

};

#endif
