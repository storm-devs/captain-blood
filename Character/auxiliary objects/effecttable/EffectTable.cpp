
#include "EffectTable.h"
#include "..\..\character\Character.h"

//JOKER: Я знаю что я плохой
//----------------------------------------
static bool globalDebugET = false;
static bool EtHandlerRegistred = false;
void _cdecl EtLogSwitch(const ConsoleStack& stack)
{
	globalDebugET = !globalDebugET;
}
//----------------------------------------

EffectTable::EffectTable(): effects(_FL_, 1), particles(_FL_, 4)
{

}

EffectTable::~EffectTable()
{
	for (int i=0;i<(int)effects.Size();i++)
	{
		for (int j = 0; j < MAX_PARTICLES_IN_EFFECT_TABLE; j++)
		{
			RELEASE(effects[i].sfx_particle[j].pSys);		
		}
	}	

	effects.Empty();
}

PhysTriangleMaterialID EffectTable::GetEffectPhysMtl(const char* type)
{
	if (!type) return pmtlid_other1;

	if (type[0]=='G' || type[0]=='g')
	{
		if (type[2]=='O' || type[2]=='o') return pmtlid_ground;

		return pmtlid_grass;
	}
	else
	if (type[0]=='S' || type[0]=='s')
	{
		if (type[1]=='T' || type[1]=='t') return pmtlid_stone;

		return pmtlid_sand;
	}
	else
	if (type[0]=='W' || type[0]=='w')
	{
		if (type[1]=='O' || type[1]=='o') return pmtlid_wood;

		return pmtlid_water;
	}
	else
	if (type[0]=='I' || type[0]=='i')
	{	
		return pmtlid_iron;
	}
	else
	if (type[0]=='F' || type[0]=='f')
	{	
		return pmtlid_fabrics;
	}
	else
	if (type[0]=='A' || type[0]=='a')
	{	
		return pmtlid_other2;
	}
	else
	if (type[0]=='B' || type[0]=='b')
	{	
		return pmtlid_other3;
	}
	
	return pmtlid_other1;
}

bool EffectTable::Create(MOPReader &reader)
{
	if (EtHandlerRegistred == false)
	{
		Console().Register_PureC_Command("ET_Log", "Log all effect table calls", (PURE_C_CONSOLE_COMMAND)&EtLogSwitch);
		EtHandlerRegistred = true;
	}

	InitParams(reader);

	if (!EditMode_IsOn())	
	{
		SetUpdate(&EffectTable::Work, ML_DYNAMIC1);	
	}
	else
	{		
		DelUpdate(&EffectTable::Work);
	}


	return true;
}

bool EffectTable::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void EffectTable::InitParams(MOPReader &reader)
{	
	effects.Empty();

	int count = reader.Array();

	effects.AddElements(count);

	for (int i=0;i<count;i++)
	{
		MaterialEffect& effect = effects[i];

		effect.mtl = GetEffectPhysMtl(reader.Enum().c_str());		
		//effect.mtl = pmtlid_air;
		effect.sound = reader.String();


		int particlesCount = reader.Array();

		effect.maxParticles = particlesCount;
		Assert(effect.maxParticles < MAX_PARTICLES_IN_EFFECT_TABLE);

		for (int j=0;j < particlesCount; j++)
		{
			effect.sfx_particle[j].particle = reader.String();

			effect.sfx_particle[j].emmit_ps = -1.0f;

			if (effect.sfx_particle[j].particle.NotEmpty())
			{
				effect.sfx_particle[j].pSys = Particles().CreateParticleSystemEx2(effects[i].sfx_particle[j].particle.c_str(), Matrix(), false, _FL_);

				if (effect.sfx_particle[j].pSys)
				{
					effect.sfx_particle[j].pSys->PauseEmission(true);
				}
			}

		}


		
		effect.minY = reader.Float();
		effect.maxY = reader.Float();

	}	
}

bool EffectTable::MakeEffect(PhysTriangleMaterialID mtl, Character * victim, Vector pos, Vector normal, IGMXScene * scene, GMXHANDLE * loc, bool isAttach)
{
	for (int i=0;i<(int)effects.Size();i++)
	{
		if (mtl != effects[i].mtl)
		{
			continue;
		}

		Sound().Create3D(effects[i].sound, pos, _FL_);


		if (effects[i].maxParticles > 0)
		{
			MatParticle & sfx = effects[i].getRandParticle();
			IParticleSystem* pSys = Particles().CreateParticleSystem(sfx.particle.c_str());

			if (pSys)
			{
				Matrix mat(true);
				mat.BuildOrient(normal,Vector(0.0f,1.0f,0.0f));
				mat.pos = pos;

				if (victim)
				{
					Matrix victimMtx(true);
					Vector victimPos = victim->GetMatrix(victimMtx).pos;
					mat.pos.y = MinMax(victimPos.y + effects[i].minY, victimPos.y + effects[i].maxY, mat.pos.y);
				}

				pSys->Teleport(mat);

				if (globalDebugET)
				{
					LogicDebug("EffectTable - CreateParticleSystem '%s', done. Position %f, %f, %f", sfx.particle.c_str(), mat.pos.x, mat.pos.y, mat.pos.z);
					const char * locName = "Not attached";
					if (scene && loc) locName = scene->GetNodeName(*loc);
					LogicDebug("EffectTable attached = %d, attached locator = '%s'", isAttach, locName);
				}


				if (isAttach && scene && loc)
				{
					pSys->AttachTo(scene, *loc, false);
				}

				pSys->AutoDelete(true);
				pSys->Restart(0);
			} else
			{
				LogicDebug("EffectTable(error) - can't create particle '%s'", sfx.particle.c_str());
			}
		}
		


		return true;
	}


	LogicDebug("EffectTable(error) - material (%d) not found in effect table, skip work", mtl);
	return false;
}

bool EffectTable::MakeChachedEffect(PhysTriangleMaterialID mtl, Character * victim, Vector pos, Vector normal, IGMXScene * scene, GMXHANDLE * loc, bool isAttach)
{
	for (int i=0;i<(int)effects.Size();i++)
	{
		if (mtl != effects[i].mtl)
		{
			continue;
		}
		Sound().Create3D(effects[i].sound, pos, _FL_);


		if (effects[i].maxParticles > 0)
		{
			MatParticle & sfx = effects[i].getRandParticle();

			if (sfx.pSys)
			{
				Matrix mat(true);
				mat.BuildOrient(normal,Vector(0.0f,1.0f,0.0f));
				mat.pos = pos;

				if (victim)
				{
					Matrix victimMtx(true);
					Vector victimPos = victim->GetMatrix(victimMtx).pos;
					mat.pos.y = MinMax(victimPos.y + effects[i].minY, victimPos.y + effects[i].maxY, mat.pos.y);
				}

				if (sfx.emmit_ps>0.0f)
				{
					sfx.pSys->Teleport(mat);
					sfx.pSys->Restart(rand());
				}
				else
				{
					sfx.pSys->SetTransform(mat);
				}

				if (globalDebugET)
				{
					LogicDebug("EffectTable - RestartParticleSystem '%s', done. Position %f, %f, %f", sfx.particle.c_str(), mat.pos.x, mat.pos.y, mat.pos.z);
					const char * locName = "Not attached";
					if (scene && loc) locName = scene->GetNodeName(*loc);
					LogicDebug("EffectTable attached = %d, attached locator = '%s'", isAttach, locName);
				}


				if (isAttach && scene && loc)
				{
					sfx.pSys->AttachTo(scene, *loc, false);
				}

				sfx.emmit_ps = 0.1f;
				sfx.pSys->PauseEmission(false);			

				if (particles.Find(&sfx) == INVALID_ARRAY_INDEX)
					particles.Add(&sfx);
			} else
			{
				LogicDebug("EffectTable(error) - can't restart particle '%s'", sfx.particle.c_str());
			}
		}


		return true;
	}

	LogicDebug("EffectTable(error) - material (%d) not found in effect table, skip work", mtl);
	return false;
}

void _cdecl EffectTable::Work(float dltTime, long level)
{
	for (int i=particles.Last(); i>=0; i--)
	{
		MatParticle * sfx = particles[i];

		sfx->emmit_ps -= dltTime;
		if (sfx->emmit_ps < 0.0f)
		{
			sfx->emmit_ps = -1.0f;
			sfx->pSys->PauseEmission(true);
			particles.ExtractNoShift(i);
		}
	}

	/*for (int i=0;i<(int)effects.Size();i++)
	{
		for (int j = 0; j < MAX_PARTICLES_IN_EFFECT_TABLE; j++)
		{
			MatParticle & sfx = effects[i].sfx_particle[j];

			if (sfx.emmit_ps > 0.0f && sfx.pSys)
			{
				sfx.emmit_ps -= dltTime;

				if (sfx.emmit_ps < 0.0f)
				{
					sfx.emmit_ps = -1.0f;
					sfx.pSys->PauseEmission(true);
				}
			}
		}
	}*/

}

MOP_BEGINLISTCG(EffectTable, "Effect Table", '1.00', 50, "EffectTable", "Character")
	
	MOP_ENUMBEG("Mtrl")
		MOP_ENUMELEMENT("ground")
		MOP_ENUMELEMENT("stone")
		MOP_ENUMELEMENT("sand")
		MOP_ENUMELEMENT("wood")		
		MOP_ENUMELEMENT("grass")
		MOP_ENUMELEMENT("water")		
		MOP_ENUMELEMENT("iron")
		MOP_ENUMELEMENT("fabrics")		
		MOP_ENUMELEMENT("armor")
		MOP_ENUMELEMENT("body")
	MOP_ENUMEND

	MOP_ARRAYBEG("Effects", 0, 100)
		
		MOP_ENUM("Mtrl", "Material")
		MOP_STRING("Sound", "")

		MOP_ARRAYBEG("Particles", 0, MAX_PARTICLES_IN_EFFECT_TABLE)
			MOP_STRING("Particle", "")
		MOP_ARRAYEND
		
		MOP_FLOATC("MinY", -25000.0f, "Минимальная высота партикла(относительно центра персонажа)");
		MOP_FLOATC("MaxY", 25000.0f, "Максимальная высота партикла(относительно центра персонажа)");

	MOP_ARRAYEND
	
MOP_ENDLIST(EffectTable)
