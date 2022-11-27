#include "WeaponPlatform.h"
#include "..\SeaMissionParams.h"
#include "cannon.h"
#include "..\TargetPoints.h"

WeaponPlatform::WeaponPlatform() :
	m_aWeaponSide(_FL_)
{
	m_pShipIterator = null;
	m_bAutoTarget = true;
	m_bUseTargetPoint = false;
}

WeaponPlatform::~WeaponPlatform()
{
	Release();
}

bool WeaponPlatform::Create(MOPReader & reader)
{
	ReadParams(reader);
	return true;
}

void WeaponPlatform::PostCreate()
{
	MOSafePointer sp;
	static const ConstString id_SeaMissionParams("SeaMissionParams");
	if( FindObject(id_SeaMissionParams,sp) )
	{
		MO_IS_IF(tid, "SeaMissionParams", sp.Ptr())
		{
			m_pShipIterator = ((SeaMissionParams*)sp.Ptr())->GetShipIterator();
		}
	}
}

void WeaponPlatform::Restart()
{
	//Release
}

bool WeaponPlatform::EditMode_Create(MOPReader & reader)
{
	ReadParams(reader);
	SetUpdate(&WeaponPlatform::EditDraw, ML_DEBUG);
	return true;
}

bool WeaponPlatform::EditMode_Update(MOPReader & reader)
{
	ReadParams(reader);
	return true;
}

void WeaponPlatform::Activate(bool isActive)
{
	if( isActive )
		SetUpdate(&WeaponPlatform::Draw, ML_GEOMETRY2);
	else
		DelUpdate(&WeaponPlatform::Draw);
}

//Обработчик команд для объекта
void WeaponPlatform::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;

	// команда огонь?
	if( id[0]=='f' || id[0]=='F' )
	{
		if( string::IsEqual(id,"fire") )
		{
			// должен передаваться ИД объекта по которому мы будем стрелять
			if( numParams < 1 )
				api->Trace("Warning! Invalid parameters for command: fire");
			else
			{
				MOSafePointer sptr;
				if( FindObject( ConstString(params[0]), sptr ) )
					ShootByTarget( sptr.Ptr() );
			}
		}
		else if( string::IsEqual(id,"fireToPos") )
		{
			// должна передаваться позиция по которой мы будем стрелять
			if( numParams < 3 )
				api->Trace("Warning! Invalid parameters for command: fireToPos");
			else
			{
				Vector targpos;
				targpos.x = (float)atof(params[0]);
				targpos.y = (float)atof(params[1]);
				targpos.z = (float)atof(params[2]);
				ShootByTarget( targpos );
			}
		}

	}
}


Matrix & WeaponPlatform::GetMatrix(Matrix & mtx)
{
	return m_mTransform;
}

void _cdecl WeaponPlatform::Draw( float deltaTime, long level)
{
	// автоматическая стрельба по кораблям
	if( m_bAutoTarget )
		if( m_bUseTargetPoint )
		{
			TargetPoints* pTargPoints = TargetPoints::GetNextTargetPoint(&Mission());
			// стрельба только по тагрет поинтам
			if( pTargPoints )
			{
				// со всех платформ
				for( long i=0; i<m_aWeaponSide; i++ )
				{
					if( !m_aWeaponSide[i].side ) continue;

					// сколькими надо стрелять
					unsigned int q = m_aWeaponSide[i].side->GetWeaponCount();
					// все ли пушки готовы?
					for( unsigned int k=0; k<q; k++ )
					{
						Weapon * pW = m_aWeaponSide[i].side->GetWeapon(k).GetWeapon();
						if( pW && !pW->CanFire() )
							break;
					}
					// если есть неготовые пушки, то пропускаем их
					if( k<q ) continue;
					// сколько есть точек для стрельбы
					array<long> aPoints(_FL_);
					long nTargQnt = pTargPoints->GetPointsArray(q, aPoints);
					long n = 0;

					// стреляем
					for( k=0; k<q && n<nTargQnt; k++ )
					{
						Weapon * pW = m_aWeaponSide[i].side->GetWeapon(k).GetWeapon();
						// не пушка - пропускаем
						if( !pW || pW->GetType() != Weapon::WeaponType_cannon )
							continue;

						// стрельба
						pW->FireByPoint( pTargPoints, aPoints[n] );

						n++;
					}
					// занимаем таргет поинты
					TargetPoints::BorrowTargetPoints(pTargPoints,this);
					// другие платформы не стреляют
					break;
				}
			}
		}
		else if( m_pShipIterator )
		{
			// проверим все корабли на попадание в зону обстрела
			for( m_pShipIterator->Reset(); !m_pShipIterator->IsDone(); m_pShipIterator->Next() )
			{
				MissionObject* mo = m_pShipIterator->Get();
				if( !mo->IsShow() || mo->IsDead() )
					continue;

				Matrix m(true);
				mo->GetMatrix(m);

				Vector vtarg;
				ZoneIntersection( m.pos, vtarg );
			}
		}

	for( dword n=0; n<m_aWeaponSide.Size(); n++ )
	{
		m_aWeaponSide[n].side->FrameUpdate( deltaTime, m_mTransform );
	}
}

bool WeaponPlatform::ZoneIntersection( const Vector & pos, Vector & targ )
{
	targ = pos;

	// target direction
	Vector targDir = pos - m_mTransform.pos;
	float targDist = ~targDir;

	// check target distance
	if( targDist < m_shootZone.fMinDist * m_shootZone.fMinDist )
		return false;
	if( targDist > m_shootZone.fMaxDist * m_shootZone.fMaxDist )
		return false;

	// normalize target direction
	targDir *= 1.f / sqrtf(targDist);

	for( dword n=0; n<m_aWeaponSide.Size(); n++ )
	{
		float targCos = targDir | m_aWeaponSide[n].vdir;
		if( targCos < m_shootZone.fCos_MaxAng )
			continue;
		FireCannon(n,targ);
	}

	return true;
}

void WeaponPlatform::FireCannon(dword nSide, const Vector & vtarg)
{
	if( nSide >= m_aWeaponSide.Size() )
		return;

	WeaponTargetZone wtz_(vtarg);

	unsigned int q = m_aWeaponSide[nSide].side->GetWeaponCount();
	for( unsigned int i=0; i<q; i++ )
	{
		SideWeapon & sw = m_aWeaponSide[nSide].side->GetWeapon(i);
		if( sw.GetWeapon()->GetType() == Weapon::WeaponType_cannon )
			((Cannon*)sw.GetWeapon())->Fire( wtz_, false );
	}
}

// стрельба по заданной цели
void WeaponPlatform::ShootByTarget( MissionObject* pObj )
{
	Assert(pObj);

	// матрица цели
	Matrix mtx(true);
	pObj->GetMatrix(mtx);

	// строим зону обстрела
	WeaponTargetZone wtz( m_mTransform.pos, pObj, false );
	//WeaponTargetZone wtz(mtx.pos);

	FireToZone(wtz);
}

void WeaponPlatform::ShootByTarget( Vector& pos )
{
	// строим зону обстрела
	WeaponTargetZone wtz( pos );
	// стреляем
	FireToZone(wtz);
}

void WeaponPlatform::FireToZone( WeaponTargetZone& wzone )
{
	// идем по списку бортов
	for( dword n=0; n<m_aWeaponSide.Size(); n++ )
	{
		// идем по списку орудий на борту
		dword q = m_aWeaponSide[n].side->GetWeaponCount();
		for( dword i=0; i<q; i++ )
		{
			SideWeapon & sw = m_aWeaponSide[n].side->GetWeapon(i);
			// из пушки стреляем
			if( sw.GetWeapon()->GetType() == Weapon::WeaponType_cannon )
				((Cannon*)sw.GetWeapon())->Fire( wzone, false );
		}
	}
}

void _cdecl WeaponPlatform::EditDraw( float deltaTime, long level)
{
	for( dword n=0; n<m_aWeaponSide.Size(); n++ )
	{
		unsigned int q = m_aWeaponSide[n].side->GetWeaponCount();
		for( unsigned int i=0; i<q; i++ )
		{
			SideWeapon& sw = m_aWeaponSide[n].side->GetWeapon(i);
			sw.GetPosition();
			sw.GetDirAngle();
		}
	}

	for (unsigned int i = 0; i < m_aWeaponSide.Size(); i++)
	{
		unsigned int emCount = m_aWeaponSide[i].side->GetSmokePointsCount();
		
		for (unsigned int k = 0; k < emCount; ++k)
		{
			Vector pos = m_mTransform.MulVertex(m_aWeaponSide[i].side->GetSmokePoint(k));
			Matrix m;
			m.BuildPosition( pos );

			Render().DrawSphere(pos, 0.5f, 0xFFFFFF00);
		}

		Matrix m;
		Vector arrow[] = { Vector(0,0,2), Vector(0.5f,0,0), Vector(-0.5f,0,0) };
		Vector poly[3];

		for (unsigned int k = 0; k < m_aWeaponSide[i].side->GetWeaponCount(); ++k)
		{
			m = m_mTransform;
			m.RotateY( m_aWeaponSide[i].side->GetWeapon(k).GetDirAngle() );
			m.pos = m_mTransform.MulVertex(m_aWeaponSide[i].side->GetWeapon(k).GetPosition());
			poly[0] = m.MulVertex(arrow[0]);
			poly[1] = m.MulVertex(arrow[1]);
			poly[2] = m.MulVertex(arrow[2]);

			Render().DrawPolygon(poly, sizeof(poly)/sizeof(poly[0]), 0xFF0000FF);
		}
	}

	if( m_shootZone.bShowDebug )
	{
		Render().DrawXZCircle( m_mTransform.pos, m_shootZone.fMinDist, 0xFF00FF00 );
		Render().DrawXZCircle( m_mTransform.pos, m_shootZone.fMaxDist, 0xFF00FF00 );
		Vector poly[4];
		for( dword n=0; n<m_aWeaponSide.Size(); n++ )
		{
			float fs = sinf( m_shootZone.fMaxAng );
			float fc = cosf( m_shootZone.fMaxAng );
			Vector & dir = m_aWeaponSide[n].vdir;
			poly[0] = m_mTransform.pos + Vector((dir.x * fc - dir.z * fs) * m_shootZone.fMinDist,
				0.f,	(dir.z * fc + dir.x * fs) * m_shootZone.fMinDist );
			poly[1] = m_mTransform.pos + Vector((dir.x * fc - dir.z * fs) * m_shootZone.fMaxDist,
				0.f,	(dir.z * fc + dir.x * fs) * m_shootZone.fMaxDist );
			poly[2] = m_mTransform.pos + Vector((dir.x * fc + dir.z * fs) * m_shootZone.fMaxDist,
				0.f,	(dir.z * fc - dir.x * fs) * m_shootZone.fMaxDist );
			poly[3] = m_mTransform.pos + Vector((dir.x * fc + dir.z * fs) * m_shootZone.fMinDist,
				0.f,	(dir.z * fc - dir.x * fs) * m_shootZone.fMinDist );
			Render().DrawPolygon(poly, 4, 0xFF0000FF);
		}
	}
}

void WeaponPlatform::ReadParams(MOPReader& reader)
{
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	m_mTransform.Build( ang, pos );

	m_shootZone.fMinDist = reader.Float();
	m_shootZone.fMaxDist = reader.Float();
	m_shootZone.fMaxAng = Deg2Rad(reader.Float());
	m_shootZone.bShowDebug = reader.Bool();

	m_shootZone.fCos_MaxAng = cosf(m_shootZone.fMaxAng);

	dword q = reader.Array();
	m_aWeaponSide.DelAll();
	m_aWeaponSide.AddElements(q);
	for( dword n=0; n<q; n++ )
	{
		m_aWeaponSide[n].side = NEW ShipSide(this);
		Assert(m_aWeaponSide[n].side);

		m_aWeaponSide[n].side->ClearWeapons();
		m_aWeaponSide[n].side->SetOwner( this );

		m_aWeaponSide[n].side->SetAniModel( reader.String().c_str() );

		// чтение бортовых орудий
		dword count = reader.Array();
		for (unsigned int k = 0; k < count; ++k)
		{
			SideWeapon weapon(this);
			weapon.SetPatternName(reader.String());
			weapon.SetPosition(reader.Position());
			weapon.SetDirAngle(Deg2Rad(reader.Float()));
			weapon.SetAnimBoneName(reader.String().c_str());
			
			m_aWeaponSide[n].side->AddWeapon(weapon);
		}

		m_aWeaponSide[n].side->SetGrouping(reader.Float());
		m_aWeaponSide[n].side->SetSmokeSFX(reader.String().c_str());
		
		// чтение эмиттеров дыма после стрельбы
		count = reader.Array();
		m_aWeaponSide[n].side->ClearSmokePoints();
		for (unsigned int k = 0; k < count; ++k)
			m_aWeaponSide[n].side->AddSmokePoint(reader.Position());

		// направление для зоны обстрела
		m_aWeaponSide[n].vdir = m_mTransform.vz;
		m_aWeaponSide[n].vdir.Rotate( reader.Float() );
	}

	m_bAutoTarget = reader.Bool();
	m_bUseTargetPoint = reader.Bool();

	bool bActivate = reader.Bool();
	if( !EditMode_IsOn() )
		Activate( bActivate );
}

void WeaponPlatform::Release()
{
	m_aWeaponSide.DelAll();

	m_pShipIterator = null;
}

static const char* g_WeaponPlatformDescr = "Weapon platform the mission object.\n"
"Commands:\n"
"  fire <targetID> - Shoot from all weapons to target identified by <targetID>";
//////////////////////////////////////////////////////////////////////////
// Миссионные параметры для орудийной платформы
//////////////////////////////////////////////////////////////////////////
MOP_BEGINLISTCG(WeaponPlatform, "Weapon Platform", '1.00', 100, g_WeaponPlatformDescr, "Arcade Sea");
	MOP_POSITION("Position", Vector(0,0,0))
	MOP_ANGLES("Angles", Vector(0,0,0))

	MOP_GROUPBEG("Shoot zone")
		MOP_FLOATC("Min distance", 10.f, "Minimal distance for shooting")
		MOP_FLOATC("Max distance", 500.f, "Maximal distance for shooting")
		MOP_FLOATC("Max shoot angle", 10.f, "Maximal angle for shooting")
		MOP_BOOLC("Show debug", true, "In EditMode do draw shooter zone")
	MOP_GROUPEND()

	MOP_ARRAYBEG("Sides", 0, 4)
		MOP_STRING("Weapon animated model", "")

		// бортовые орудия
		MOP_ARRAYBEG("Weapons", 0, 100)
			MOP_STRING("Weapon pattern", "")
			MOP_POSITION("Position", Vector(0.0f))
			MOP_FLOATEX("Direction angle", 0.0f, 0.0f, 360.0f)
			MOP_STRING("Animation bone name", "")
		MOP_ARRAYEND

		MOP_FLOAT("Shoot grouping", 15.0f)
		MOP_STRING("Shoot smoke SFX", "")

		MOP_ARRAYBEG("Shoot smoke points", 0, 10)
			MOP_POSITION("Position", Vector(0.0f))
		MOP_ARRAYEND

		MOP_FLOATEXC("Zone direction", 0.f, 0.f,360.f, "Base angle of zone for enemy detecting")
	MOP_ARRAYEND

	MOP_BOOLC("AutoTarget",false,"Automatical find and shoot to target, else need special command to set target and start firing")

	MOP_BOOLC("UseTargetPoint",false,"Fire by target points array")

	MOP_BOOL("Activate", false);
MOP_ENDLIST(WeaponPlatform)
