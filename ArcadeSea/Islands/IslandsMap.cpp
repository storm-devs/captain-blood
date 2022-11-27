#include "IslandsMap.h"


IslandsMap::ColliderDescribe::ColliderDescribe()
{
	collider = null;
}

IslandsMap::ColliderDescribe::~ColliderDescribe()
{
	RELEASE( collider );
}

bool IslandsMap::FindNearIsland(MissionObject* mo, Vector & pos)
{
	if( mo )
	{
		Matrix mtx(true);
		mo->GetMatrix( mtx );

		long objNum = -1;
		long colliderNum = -1;
		float finddistance = 1000.f;

		for( long i=0; i<m_Objects; i++ )
		{
			Vector vpos;
			if( m_bIsTeleported )
				vpos = m_Objects[i].newtransform.MulVertexByInverse( mtx.pos );
			else
				vpos = m_Objects[i].transform.MulVertexByInverse( mtx.pos );
			for( long j=0; j<m_Objects[i].colliders; j++ )
			{
				Vector dir = vpos - m_Objects[i].colliders[j].offset;
				float fdist2pow = ~dir;
				float limit2pow = m_Objects[i].colliders[j].pushoffRadius * m_Objects[i].colliders[j].pushoffRadius;
				if( fdist2pow < limit2pow )
				{
					float dist = sqrtf(fdist2pow) - m_Objects[i].colliders[j].radius;
					if( dist < finddistance )
					{
						objNum = i;
						colliderNum = j;
						finddistance = dist;
					}
				}
			}
		}

		if( objNum >= 0 && colliderNum >=0 )
		{
			Matrix m = m_bIsTeleported ? m_Objects[objNum].newtransform : m_Objects[objNum].transform;
			Vector vc = m.MulVertex( m_Objects[objNum].colliders[colliderNum].offset );
			Vector dir = mtx.pos - vc;
			float fdist = dir.Normalize();
			if( fdist + 10.f < m_Objects[objNum].colliders[colliderNum].radius  )
				fdist -= 10.f;
			else
				fdist = m_Objects[objNum].colliders[colliderNum].radius;
			pos = vc + dir * fdist;
			return true;
		}

	}
	return false;
}

void IslandsMap::ColliderDescribe::ReadMOPs(IslandsMap* islandmap, const Matrix & transform, MOPReader & reader)
{
	Assert(islandmap);

	offset = reader.Position();
	radius = reader.Float();
	pushoffRadius = reader.Float();

	if( radius < 0.1f ) radius = 0.1f;
	pushoffRadius += radius;

	Matrix mtx( transform );
	mtx.pos = mtx.MulVertex( offset );

	collider = islandmap->Physics().CreateSphere( _FL_, radius, mtx, false );
	if( collider )
	{
		collider->SetGroup( phys_ship );
	}
}


IslandsMap::ObjectDescr::ObjectDescr() : colliders(_FL_)
{
	gmxname = "";
	geometry = null;
	refrGeometry = null;
	reflGeometry = null;
}

IslandsMap::ObjectDescr::~ObjectDescr()
{
	RELEASE( geometry );
	RELEASE( refrGeometry );
	RELEASE( reflGeometry );
}

void IslandsMap::ObjectDescr::ReadMOPs(IslandsMap* islandmap, MOPReader & reader)
{
	Assert(islandmap);

	gmxname = reader.String().c_str();
	geometry = islandmap->Geometry().CreateScene( gmxname, &islandmap->Animation(), &islandmap->Particles(), &islandmap->Sound(), _FL_ );

	// refraction
	const char* name = reader.String().c_str();
	refrGeometry = islandmap->Geometry().CreateScene( name, &islandmap->Animation(), &islandmap->Particles(), &islandmap->Sound(), _FL_ );
	if( refrGeometry )
		refrGeometry->SetDynamicLightState(true);
	// reflection
	name = reader.String().c_str();
	reflGeometry = islandmap->Geometry().CreateScene( name, &islandmap->Animation(), &islandmap->Particles(), &islandmap->Sound(), _FL_ );
	if( reflGeometry )
		reflGeometry->SetDynamicLightState(true);

	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	transform.Build( ang, pos );
	if( geometry )
	{
		geometry->SetDynamicLightState(true);
		geometry->SetTransform( transform );
	}

	colliders.DelAll();
	colliders.AddElements( reader.Array() );
	for( long n=0; n<colliders; n++ )
	{
		colliders[n].ReadMOPs(islandmap, transform, reader );
	}
}

void IslandsMap::ObjectDescr::Init(MissionObject* islandmap)
{
	if( !islandmap ) return;
}



IslandsMap::IslandsMap() :
	m_Objects(_FL_)
{
	m_bIsTeleported = false;

	m_bNotUseFogInRefraction = true;

	m_bNoSwing = false;
}

IslandsMap::~IslandsMap()
{
}

bool IslandsMap::Create(MOPReader & reader)
{
	Render().getFogParams(h_density, h_min, h_max, d_density, d_min, d_max, fog_color);
	ReadMOPs(reader);
	return true;
}

void IslandsMap::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;

	if (string::IsEqual(id, "teleport"))
	{
		if( m_bIsTeleported )
		{
			api->Trace( "IslandsMap::Warning - Can`t retry teleport command. Will be backteleport before." );
			return;
		}
		m_bIsTeleported = true;
		if(numParams >= 2)
		{
			MOSafePointer objsrc;
			FindObject(ConstString(params[0]), objsrc);
			MOSafePointer objdst;
			FindObject(ConstString(params[1]), objdst);
			if(objsrc.Ptr() && objdst.Ptr())
			{
				Matrix mtxsrc(true);
				objsrc.Ptr()->GetMatrix( mtxsrc );
				Matrix mtxdst(true);
				objdst.Ptr()->GetMatrix( mtxdst );

				float ang = mtxsrc.vz.GetAngleXZ( mtxdst.vz );
				Vector offset = mtxdst.MulVertex( mtxsrc.MulVertexByInverse(Vector(0.f)) );
				Matrix mtx( Vector(0.f,ang,0.f), offset );

				for( long n=0; n<m_Objects; n++ )
				{
					// новая матрица для геометрии
					m_Objects[n].newtransform = m_Objects[n].transform * mtx;
					if( m_Objects[n].geometry )
						m_Objects[n].geometry->SetTransform( m_Objects[n].newtransform );
					// телепорт коллидеров
					for( long i=0; i<m_Objects[n].colliders; i++ )
					{
						if( m_Objects[n].colliders[i].collider )
						{
							// новая матрица коллидера
							Matrix m( m_Objects[n].newtransform );
							m.pos = m.MulVertex( m_Objects[n].colliders[i].offset );
							m_Objects[n].colliders[i].collider->SetTransform( m );
						}
					}
				}
			}
		}
	}

	if (string::IsEqual(id, "backteleport"))
	{
		if( !m_bIsTeleported )
		{
			api->Trace( "IslandsMap::Warning - Can`t execute backteleport command. Will be teleport before." );
			return;
		}
		m_bIsTeleported = false;
		for( long n=0; n<m_Objects; n++ )
		{
			// новая матрица для геометрии
			if( m_Objects[n].geometry )
				m_Objects[n].geometry->SetTransform( m_Objects[n].transform );
			// телепорт коллидеров
			for( long i=0; i<m_Objects[n].colliders; i++ )
			{
				if( m_Objects[n].colliders[i].collider )
				{
					// новая матрица коллидера
					Matrix m( m_Objects[n].transform );
					m.pos = m.MulVertex( m_Objects[n].colliders[i].offset );
					m_Objects[n].colliders[i].collider->SetTransform( m );
				}
			}
		}
	}
}

void _cdecl IslandsMap::Work(float fDeltaTime, long level)
{
	//Компенсируем свинг, если активна опция
	Matrix oldview(true);
	if(m_bNoSwing)
	{
		oldview = Render().GetView();
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(oldview).Inverse()).Inverse());
	}

	for( long n=0; n<m_Objects; n++ )
	{
		if( m_Objects[n].geometry )
		{
			m_Objects[n].geometry->SetTransform( m_bIsTeleported ? m_Objects[n].newtransform : m_Objects[n].transform );
			m_Objects[n].geometry->Draw();
		}
	}

	if(m_bNoSwing)
	{
		Render().SetView(oldview);
	}

	if( EditMode_IsOn() && EditMode_IsSelect() )
	{
		DebugShow();
	}
}

void _cdecl IslandsMap::DrawRefr(const char*, MissionObject*)
{
	if( m_bNotUseFogInRefraction )
	{
		Safe_Fog();
		Set_Fog(0.f,0.f);
	}
	for( long n=0; n<m_Objects; n++ )
	{
		if( m_Objects[n].refrGeometry )
		{
			m_Objects[n].refrGeometry->SetTransform( m_bIsTeleported ? m_Objects[n].newtransform : m_Objects[n].transform );
			m_Objects[n].refrGeometry->Draw();
		}
	}
	if( m_bNotUseFogInRefraction )
		Restore_Fog();
}

void _cdecl IslandsMap::DrawRefl(const char*, MissionObject*)
{
	if( m_bNotUseFogInRefraction )
	{
		Safe_Fog();
		Set_Fog(0.f,0.f);
	}
	for( long n=0; n<m_Objects; n++ )
	{
		if( m_Objects[n].reflGeometry )
		{
			m_Objects[n].reflGeometry->SetTransform( m_bIsTeleported ? m_Objects[n].newtransform : m_Objects[n].transform );
			m_Objects[n].reflGeometry->Draw();
		}
	}
	if( m_bNotUseFogInRefraction )
		Restore_Fog();
}


void IslandsMap::Show(bool isShow)
{
	if( isShow )
	{
		SetUpdate(&IslandsMap::Work, ML_GEOMETRY5);
	}
	else
	{
		DelUpdate(&IslandsMap::Work);
	}

	if(m_bSeaRefraction && isShow)
	{
		Registry(MG_SEAREFRACTION, &IslandsMap::DrawRefr, ML_GEOMETRY5);
	}else{
		Unregistry(MG_SEAREFRACTION);
	}

	if( m_bSeaReflection && isShow )
	{
		Registry(MG_SEAREFLECTION, &IslandsMap::DrawRefl, ML_GEOMETRY5);
	}
	else
	{
		Unregistry(MG_SEAREFLECTION);
	}

	MissionObject::Show( isShow );
}

void IslandsMap::ReadMOPs(MOPReader & reader)
{
	m_Objects.DelAll();
	m_Objects.AddElements( reader.Array() );
	for( long n=0; n<m_Objects; n++ )
	{
		m_Objects[n].ReadMOPs( this, reader );
	}

	m_bSeaRefraction = reader.Bool();
	m_bSeaReflection = reader.Bool();
	m_bNotUseFogInRefraction = reader.Bool();
	m_bNoSwing = reader.Bool();
	Show( reader.Bool() );
}

void IslandsMap::DebugShow()
{
	for( long n=0; n<m_Objects; n++ )
	{
		for( long i=0; i<m_Objects[n].colliders; i++ )
		{
			if( m_Objects[n].colliders[i].collider )
			{
				Matrix m( m_Objects[n].transform );
				m.pos = m.MulVertex( m_Objects[n].colliders[i].offset );

				Render().DrawSphere( m.pos, m_Objects[n].colliders[i].radius, 0xAA00FF00 );
				Render().DrawSphere( m.pos, m_Objects[n].colliders[i].pushoffRadius, 0x88FF8080 );
			}
		}
	}
}

void IslandsMap::Safe_Fog()
{
	Render().getFogParams(h_density, h_min, h_max, d_density, d_min, d_max, fog_color);
}

void IslandsMap::Set_Fog(float fDensityH, float fDensity)
{
	Render().setFogParams(fDensityH, h_min, h_max, fDensity, d_min, d_max, fog_color);
}

void IslandsMap::Restore_Fog()
{
	Render().setFogParams(h_density, h_min, h_max, d_density, d_min, d_max, fog_color);
}

static char IslandsMapDescription[] =
"-= Islands for sea missions =-\n"
"Supported commands:\n"
"teleport <srctarget> <dsttarget> - move all objects to destination target with saving relative position from src target\n"
"backteleport - move all objects into old positions";

#define MOP_COLLIDERDESCRIBE \
	MOP_POSITIONC ("offset", Vector(0.f), "local position of collider") \
	MOP_FLOATC("radius", 0.f, "Sphere collider radius") \
	MOP_FLOATC("pushoff distance", 100.f, "Distance where ships push out of collider")

#define MOP_STATIC_SEA_OBJECT \
	MOP_STRING ("GMX file","") \
	MOP_STRING ("GMX file for refraction","") \
	MOP_STRING ("GMX file for reflection","") \
	MOP_POSITION ("Position", Vector(0.f)) \
	MOP_ANGLES ("Orientation", Vector(0.f)) \
	MOP_ARRAYBEG ("Colliders", 1, 100) \
		MOP_COLLIDERDESCRIBE \
	MOP_ARRAYEND

MOP_BEGINLISTCG(IslandsMap, "IslandsMap", '1.00', 100, IslandsMapDescription, "Arcade Sea");
	MOP_ARRAYBEG("Objects", 0, 10)
		MOP_STATIC_SEA_OBJECT
	MOP_ARRAYEND
	MOP_BOOL ("Sea refraction",true)
	MOP_BOOL ("Sea reflection",true)
	MOP_BOOL ("Don`t use fog into refraction",true)
	MOP_BOOLC("No swing", false, "No swing islans in swing machine")
	MOP_BOOL("Visible", true)
MOP_ENDLIST(IslandsMap)
