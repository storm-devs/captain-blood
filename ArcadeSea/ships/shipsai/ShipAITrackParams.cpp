
#include "ShipAITrackParams.h"

#define DefaultDetectorRadius	50.f

MOPShipWayPoint::MOPShipWayPoint()
{
	vPos = 0.f;
	fDetectorRadius = DefaultDetectorRadius;
	pcEnemyName = "";

	eBehaviourType = Behaviour_follow;
	fBehaviourTime = 0.f;

	speedK = 1.f;
	strafeZoneWidth = 0.f;
	strafeZoneHeight = 0.f;
	fStrafeRotSpeed = 10.f;
	fStrafeMaxAngle = 0.f;
	fStrafeMaxSpeed = 0.f;
	fStrafeBaseAngle = 0.f;
}

void MOPShipWayPoint::ReadMOPs(MOPReader& reader)
{
	vPos = reader.Position();
	fDetectorRadius = reader.Float();

	// read and set behaviour
	eBehaviourType = Behaviour_follow;
	ConstString pcBehaviourName = reader.Enum();
	if( string::IsEqual(pcBehaviourName.c_str(),"Strafe mode") )
		eBehaviourType = Behaviour_strafe;
	fBehaviourTime = reader.Float();

	// read behaviour parameters
	speedK = reader.Float();
	if( speedK < 0.001f )
		speedK = 0.001f;
	strafeZoneWidth = reader.Float();
	strafeZoneHeight = reader.Float();
	fStrafeRotSpeed = reader.Float() * (PI / 180.f);
	fStrafeMaxAngle = reader.Float() * (PI / 180.f);
	fStrafeMaxSpeed = reader.Float();
	fStrafeBaseAngle = reader.Float() * (PI / 180.f);

	vStrafePosOffset = reader.Position();
	vStrafePosOffset.y = 0.f;
	fStrafeAngOffset = reader.Float() * (PI / 180.f);
	while( fStrafeAngOffset > PI )
		fStrafeAngOffset -= PIm2;

	// trigger init
	triggerCome.Init(reader);

	// следующая точка
	nNextPoint = reader.Long();
}

void MOPShipWayPoint::Empty()
{
	vPos = 0.f;
	pcEnemyName = "";
	fDetectorRadius = DefaultDetectorRadius;
}

//============================================================================================
//реализация класса объекта параметров для АИ боя
//============================================================================================
ShipTrackAIParams::ShipTrackAIParams () :
	m_aWayTrack(_FL_)
{
	m_sptrTransformObj.Reset();

	m_bShowDebugInfo = false;
	m_params.fStrafeRotSpeed = 10.f;
	m_params.fStrafeMaxAngle = 0.f;
	m_params.fStrafeMaxSpeed = 0.f;
	m_params.fStrafeBaseAngle = 0.f;

	m_bLoopTrack = false;
}

ShipTrackAIParams::~ShipTrackAIParams ()
{
}


//-----------------------------------------------------------------------------
// интерфейс, наследуемый от MissionObject
//-----------------------------------------------------------------------------
//Инициализировать объект
bool ShipTrackAIParams::Create ( MOPReader & reader )
{
	return Update ( reader );
}

void ShipTrackAIParams::PostCreate ()
{
	if( m_pcTransformObjName.NotEmpty() )
	{
		Mission().FindObject(m_pcTransformObjName, m_sptrTransformObj);
	}
}

//Инициализировать объект в режиме редактирования
bool ShipTrackAIParams::EditMode_Create ( MOPReader & reader )
{
	SetUpdateForDebugShow();
	return Update ( reader );
}

//Обновить параметры в режиме редактирования
bool ShipTrackAIParams::EditMode_Update ( MOPReader & reader )
{
	return Update ( reader );
}

//прочитать параметры  
bool ShipTrackAIParams::Update ( MOPReader & reader )
{
	m_params.pcEnemyName = reader.String();
	long q = reader.Array();
	m_aWayTrack.DelAll();
	if( q>0 )
	{
		m_aWayTrack.AddElements(q);
		for( long n=0; n<q; n++ )
		{
			m_aWayTrack[n].ReadMOPs( reader );
			m_aWayTrack[n].vPos.y = 10.f;
		}
	}
	m_bLoopTrack = reader.Bool();

	m_pcTransformObjName = reader.String();
	m_sptrTransformObj.Reset();

	m_bShowDebugInfo = reader.Bool();
	if( m_bShowDebugInfo && EditMode_IsOn() )
	{
		SetUpdate(&ShipTrackAIParams::DrawDebug, ML_DEBUG);
	}
	else
	{
		DelUpdate(&ShipTrackAIParams::DrawDebug);
	}

	return ShipControllerParams::ReadMOPs(reader);
}

Matrix & ShipTrackAIParams::GetMatrix(Matrix& mtx)
{
	// находим объект для задания внешней трансформации, если мы в режиме редактора
	if( !m_sptrTransformObj.Ptr() && EditMode_IsOn() && m_pcTransformObjName.NotEmpty() )
		Mission().FindObject(m_pcTransformObjName, m_sptrTransformObj);

	if( m_sptrTransformObj.Validate() )
		return m_sptrTransformObj.Ptr()->GetMatrix(mtx);
	return mtx.SetIdentity();
}

void ShipTrackAIParams::DrawDebug(float fDeltaTime, long level)
{
	Matrix mtxTransf(true);
	GetMatrix(mtxTransf);

	for( dword n=0; n<m_aWayTrack.Size(); n++ )
	{
		if( m_aWayTrack[n].eBehaviourType == MOPShipWayPoint::Behaviour_strafe )
		{
			// рисуем зону
			Vector vdir = Vector( cosf(m_aWayTrack[n].fStrafeBaseAngle), 0.f, sinf(m_aWayTrack[n].fStrafeBaseAngle) );
			Vector voff = vdir ^ Vector(0.f,1.f,0.f);
			Vector vc = m_aWayTrack[n].vPos;
			vdir *= m_aWayTrack[n].strafeZoneWidth * 0.5f;
			voff *= m_aWayTrack[n].strafeZoneHeight * 0.5f;
			Vector v[4];
			v[0] = vc - vdir + voff;
			v[1] = vc + vdir + voff;
			v[2] = vc + vdir - voff;
			v[3] = vc - vdir - voff;
			Render().DrawPolygon(v,4,Color(0.f,1.f,0.f,1.f),mtxTransf);
			vc.y += 0.2f;

			// рисуем угол в зоне
			float fs = sinf(m_aWayTrack[n].fStrafeMaxAngle);
			float fc = cosf(m_aWayTrack[n].fStrafeMaxAngle);
			v[0] = vc;
			v[1] = Vector(vc.x + vdir.x*fc - vdir.z*fs, vc.y, vc.z + vdir.z*fc + vdir.x*fs);
			v[2] = Vector(vc.x + vdir.x*fc + vdir.z*fs, vc.y, vc.z + vdir.z*fc - vdir.x*fs);
			Render().DrawPolygon(v,3,Color(0.f,0.f,1.f,1.f),mtxTransf);

			Render().DrawVector(mtxTransf.MulVertex(vc), mtxTransf.MulVertex(m_aWayTrack[n].vPos+vdir), 0xFFFF0000 );

			// рисуем конечную зону в случае перемещения зоны
			if( m_aWayTrack[n].vStrafePosOffset.x != 0.f ||
				m_aWayTrack[n].vStrafePosOffset.z != 0.f ||
				m_aWayTrack[n].fStrafeAngOffset != 0.f )
			{
				// зона
				vdir = Vector(	cosf(m_aWayTrack[n].fStrafeBaseAngle + m_aWayTrack[n].fStrafeAngOffset),
								0.f,
								sinf(m_aWayTrack[n].fStrafeBaseAngle + m_aWayTrack[n].fStrafeAngOffset) );
				voff = vdir ^ Vector(0.f,1.f,0.f);
				vc = m_aWayTrack[n].vPos + m_aWayTrack[n].vStrafePosOffset;
				vdir *= m_aWayTrack[n].strafeZoneWidth * 0.5f;
				voff *= m_aWayTrack[n].strafeZoneHeight * 0.5f;
				v[0] = vc - vdir + voff;
				v[1] = vc + vdir + voff;
				v[2] = vc + vdir - voff;
				v[3] = vc - vdir - voff;
				Render().DrawPolygon(v,4,Color(0.f,1.f,0.f,1.f),mtxTransf);
				vc.y += 0.2f;

				// угол в зоне
				fs = sinf(m_aWayTrack[n].fStrafeMaxAngle);
				fc = cosf(m_aWayTrack[n].fStrafeMaxAngle);
				v[0] = vc;
				v[1] = Vector(vc.x + vdir.x*fc - vdir.z*fs, vc.y, vc.z + vdir.z*fc + vdir.x*fs);
				v[2] = Vector(vc.x + vdir.x*fc + vdir.z*fs, vc.y, vc.z + vdir.z*fc - vdir.x*fs);
				Render().DrawPolygon(v,3,Color(0.f,0.f,1.f,1.f),mtxTransf);
			}
		}

		Render().DrawSphere( mtxTransf.MulVertex(m_aWayTrack[n].vPos), m_aWayTrack[n].fDetectorRadius, 0xFFFFFFFF );
		if( n>0 )
		{
			Render().DrawVector(mtxTransf.MulVertex(m_aWayTrack[n-1].vPos), mtxTransf.MulVertex(m_aWayTrack[n].vPos), 0xFF00FF00 );
		}
	}
}

#define MOP_WAYPOINT \
	MOP_POSITION("pos", Vector(0.f)) \
	MOP_FLOAT("DetectorRadius",DefaultDetectorRadius) \
	MOP_ENUMC("Behaviour", "none", "Behaviour type into this track point") \
	MOP_FLOATC("Behaviour time",0.f,"Time while ship have changed behaviour. If zerro then no change behaviour") \
		MOP_FLOATEX("speed scale",1.f,0.1f,50.f) \
		MOP_FLOATC("strafe width",100.f,"Width of strafe mode zone") \
		MOP_FLOATC("strafe height",50.f, "Height of strafe mode zone") \
		MOP_FLOATEXC("Strafe rotate speed", 10.f, 0.f, 300.f, "Rotate speed for strafe mode") \
		MOP_FLOATEXC("Strafe max rotate", 10.f, 0.f, 90.f, "Maximal change angle for strafe mode") \
		MOP_FLOATC("Strafe max speed", 10.f, "Maximal ship speed for strafe mode") \
		MOP_FLOATEXC("Base strafe angle", 0.f, 0.f, 360.f, "Angle of Z-axis direction (strafe zone width direction and base strafe angle") \
		MOP_POSITIONC("Strafe offset", Vector(0.f), "Offset of strafe zone position (moving strafe zone)") \
		MOP_FLOATC("Strafe angle offset", 0.f, "Offset of strafe zone base angle (rotating strafe zone)") \
	MOP_MISSIONTRIGGER("Incomming trigger") \
	MOP_LONGC("Next point index", -1, "Индекс следущей точки трека (если = -1, то берется следующая точка)")

MOP_BEGINLISTG  ( ShipTrackAIParams, "ShipTrackAIParams", '1.00', 0, "Arcade Sea" )
	MOP_ENUMBEG("Behaviour")
		MOP_ENUMELEMENT("none")
		MOP_ENUMELEMENT("Strafe mode")
	MOP_ENUMEND

	MOP_STRING("Enemy object", "")
	MOP_ARRAYBEG("Track", 1, 100)
		MOP_WAYPOINT
	MOP_ARRAYEND
	MOP_BOOL("Loop track",false);
	MOP_STRING("Transform object", "")
	MOP_BOOL("ShowDebug",false)
	MOP_ShipControllerParams
MOP_ENDLIST ( ShipTrackAIParams )
