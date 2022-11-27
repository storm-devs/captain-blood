#include "Flamethrowerpattern.h"

FlametrapPattern::FlametrapPattern() :
	m_aComplexity(_FL_)
{
	m_pModel = NULL;
}

FlametrapPattern::~FlametrapPattern()
{
	RELEASE( m_pModel );
}

bool FlametrapPattern::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdateTraps();
	return true;
}

bool FlametrapPattern::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdateTraps();
	return true;
}

bool FlametrapPattern::GetComplexityData(ComplexityData& dat)
{
	const char* pcName = api->Storage().GetString("Profile.Global.Difficulty");
	if( !pcName ) pcName = "Easy";
	for( long n=0; n<m_aComplexity; n++ )
		if( m_aComplexity[n].name && string::IsEqual(m_aComplexity[n].name,pcName) )
		{
			dat = m_aComplexity[n].dat;
			return true;
		}
	//Assert(true);
	if( m_aComplexity.Size() > 0 )
	{
		dat = m_aComplexity[0].dat;
	}
	else
	{
		dat.prepareTime = 1.5f;
		dat.activeTime = 2.f;
		dat.damage = 1.f;
		dat.immuneTime = 0.5f;
	}
	return false;
}

void FlametrapPattern::ReadMOPs(MOPReader & reader)
{
	// модель
	IGMXScene* pOldModel = m_pModel;
	const char* pcModelName = reader.String().c_str();
	m_pModel = Geometry().CreateScene( pcModelName, &Animation(), &Particles(), &Sound(), _FL_ );
	RELEASE( pOldModel );

	// пламя
	m_flame.moveAngle = reader.Angles();
	m_flame.damageAngle = reader.Float() * (PI / 180.f);
	m_flame.distance = reader.Float();
	m_flame.startAng = reader.Float();
	m_flame.rotateSpeed = reader.Float();

	// сложность
	m_aComplexity.DelAll();
	m_aComplexity.AddElements( reader.Array() );
	for( long n=0; n<m_aComplexity; n++ )
	{
		m_aComplexity[n].name = reader.String().c_str();
		m_aComplexity[n].dat.prepareTime = reader.Float();
		m_aComplexity[n].dat.activeTime = reader.Float();
		m_aComplexity[n].dat.damage = reader.Float();
		m_aComplexity[n].dat.immuneTime = reader.Float();
	}

	// партиклы
	m_particles.prepare.sfx = reader.String().c_str();
	m_particles.prepare.scale = reader.Float();
	m_particles.prepare.timescale = reader.Float();
	m_particles.active.sfx = reader.String().c_str();
	m_particles.active.scale = reader.Float();
	m_particles.active.timescale = reader.Float();
	m_particles.track.sfx = reader.String().c_str();
	m_particles.track.scale = reader.Float();
	m_particles.track.timescale = reader.Float();
	m_particles.trackStartTime = reader.Float();
	m_particles.trackUpdateTime = reader.Float();
	m_particles.trackPrepare.sfx = reader.String().c_str();
	m_particles.trackPrepare.scale = reader.Float();
	m_particles.trackPrepare.timescale = reader.Float();

	// звуки
	m_sounds.pcPrepare = reader.String().c_str();
	m_sounds.pcActive = reader.String().c_str();
	m_sounds.pcDeactivate = reader.String().c_str();

	// реакция на дамаг
	ReadCommonData( reader );
}

#define MOP_SFXDESCR(name,descr) \
	MOP_STRINGC(name##" SFX", "", "SFX name of particles for "##descr) \
	MOP_FLOATC(name##" SFX scale", 1.f, "Scale of particles for "##descr) \
	MOP_FLOATC(name##" SFX time scale", 1.f, "Time scale of particles for"##descr)

MOP_BEGINLISTCG(FlametrapPattern, "Flame trap pattern", '1.00', 0, "Pattern for trap flamethrower", "Default")
	MOP_STRINGC("Model", "", "Model name for trap")
	MOP_GROUPBEGC("Flame parameters","Describe for flame sector and movement")
		MOP_ANGLESC("Move angle", Vector(PI*0.25f,0.f,0.f), "Angle of sector for flame rotate")
		MOP_FLOATEXC("Damage angle", 10.f,0.f,90.f, "Angle of sector where flame inflict damage")
		MOP_FLOATC("Distance", 3.f, "Distance where flame inflict damage")
		MOP_FLOATEXC("Start angle", 0.f, -1.f, 1.f, "start position of flamethrower (-1|1 = min|max rotate angle)")
		MOP_FLOATC("Rotate speed", 1.f, "Movement speed of flamethrower (inverse value - back movement)")
	MOP_GROUPEND()
	MOP_ARRAYBEG("Complexity", 1, 5)
		MOP_STRINGC("Name", "Easy", "Name of complexity level")
		MOP_FLOATC("Prepare time", 1.5f, "Time interval while trap be warning mode")
		MOP_FLOATC("Active time", 3.f, "Time interval while trap is inflict damage mode")
		MOP_FLOATC("Damage", 1.f, "Damage inflicted by trap")
		MOP_FLOATC("Immune time", 1.f, "Time interval while trap not inflict repeated damage (negative value is single damage from trap)")
	MOP_ARRAYEND
	MOP_GROUPBEGC("Particles","Describe for particles used by trap")
		MOP_SFXDESCR("Prepare","warning mode")
		MOP_SFXDESCR("Active","active mode")
		MOP_SFXDESCR("Track","flame track on character patch")
		MOP_FLOATC("Track start time", 0.f, "Time since birth track from flame")
		MOP_FLOATC("Track update time", 0.3f, "Time interval for birth track from flame")
		MOP_SFXDESCR("Warning point","warning point on character patch")
	MOP_GROUPEND()
	MOP_GROUPBEGC("Sounds","Describe for sounds used by trap")
		MOP_STRINGC("Sound prepare", "", "Sound while flamethrower in prepare mode")
		MOP_STRINGC("Sound active", "", "Sound while flamethrower in active mode")
		MOP_STRINGC("Sound deactivate", "", "Sound while flamethrower in deactivate mode")
	MOP_GROUPEND()
	MOP_TRAPCOMMONPARAMS
MOP_ENDLIST(FlametrapPattern)
