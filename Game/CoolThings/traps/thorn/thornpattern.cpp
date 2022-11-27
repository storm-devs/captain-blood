#include "ThornPattern.h"

ThornPattern::ThornPattern() :
	m_aComplexity(_FL_)
{
	m_pModel = NULL;

}

ThornPattern::~ThornPattern()
{
	RELEASE( m_pModel );
}

bool ThornPattern::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdateTraps();
	return true;
}

bool ThornPattern::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdateTraps();
	return true;
}

bool ThornPattern::GetComplexityData(ComlexityData& dat)
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
	dat.warningTime = 1.5f;
	dat.damage = 1.0f;
	dat.m_fImmuneTime = 0.5f;

	return false;
}

void ThornPattern::ReadMOPs(MOPReader & reader)
{
	// модель
	IGMXScene* pOldModel = m_pModel;
	m_pModel = Geometry().CreateScene(reader.String().c_str(), &Animation(), &Particles(), &Sound(), __FILE__, __LINE__ );
	RELEASE( pOldModel );
	if( m_pModel )
		m_pModel->SetDynamicLightState( true );

	// позиции
	m_heights.fDisable = reader.Float();
	m_heights.fWarning = reader.Float();
	m_heights.fActive = reader.Float();

	m_heights.fDisableAccelerate = reader.Float();
	m_heights.fWarningAccelerate = reader.Float();
	m_heights.fActiveAccelerate = reader.Float();

	// логика
	m_aComplexity.DelAll();
	long q = reader.Array();
	m_aComplexity.AddElements(q);
	for( long n=0; n<q; n++ )
	{
		m_aComplexity[n].name = reader.String().c_str();
		m_aComplexity[n].dat.warningTime = reader.Float();
		m_aComplexity[n].dat.damage = reader.Float();
		m_aComplexity[n].dat.m_fImmuneTime = reader.Float();
	}
	m_fActiveTime = reader.Float();
	m_vDamageBoxSize = reader.Position();
	m_vDamageBoxCenter = reader.Position();

	// звуки
	m_sounds.pcStartMove = reader.String();
	m_sounds.pcWarning = reader.String();
	m_sounds.pcActivate = reader.String();
	m_sounds.pcDeactivate = reader.String();
	// reactions
	ReadCommonData(reader);
}

MOP_BEGINLISTCG(ThornPattern, "Thorn pattern", '1.00', 0, "Pattern for trap thorn", "Default")
	MOP_STRINGC("Model", "", "Model name for trap")
	MOP_FLOATC("Height base", 1.9f, "Local Y-position for trap while disable mode")
	MOP_FLOATC("Height warning", 2.2f, "Local Y-position for trap while warning mode")
	MOP_FLOATC("Height activing", 3.5f, "Local Y-position for trap while active mode")
	MOP_FLOATC("Accelerate Disable", 9.8f, "Accelerate for moving while disable mode")
	MOP_FLOATC("Accelerate Warning", 9.8f, "Accelerate for moving while warning mode")
	MOP_FLOATC("Accelerate Active", 9.8f, "Accelerate for moving while active mode")
	MOP_ARRAYBEG("Complexity", 1, 5)
		MOP_STRINGC("Name", "Easy", "Name of complexity level")
		MOP_FLOATC("Warning time", 1.5f, "Time while trap be warning mode")
		MOP_FLOATC("Damage", 1.f, "Damage inflicted by trap")
		MOP_FLOATC("Immune time", 0.5f, "Time while target is immuned after damage receive (negative value is single damage from trap)")
	MOP_ARRAYEND
	MOP_FLOATC("Active time", 2.f, "Time while thorn state is active")	
	MOP_POSITIONC("Damage box size", Vector(1.f,1.f,1.f), "Size of box which inflict damage")
	MOP_POSITIONC("Damage box center", Vector(0.f,0.f,0.f), "Local position of box which inflict damage")
	MOP_STRINGC("Sound start move", "", "Sound for start thorn move")
	MOP_STRINGC("Sound warning", "", "Sound while thorn in warning mode")
	MOP_STRINGC("Sound activate", "", "Sound for thorn turn to active mode")
	MOP_STRINGC("Sound deactivate", "", "Sound for thorn move back to inactive mode")
	MOP_TRAPCOMMONPARAMS
MOP_ENDLIST(ThornPattern)
