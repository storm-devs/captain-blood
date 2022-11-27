
#include "ShipDamageDetector.h"
#include "Ships\ship.h"
#include "..\Common_h\ICharactersArbiter.h"

//============================================================================================

ShipDamageDetector::ShipDamageDetector() :
	m_events(_FL_)
{
	m_fMaxHP = m_fHP = 0.f;

	m_minBound = -Vector(0.1f,0.1f,0.1f);
	m_maxBound = Vector(0.1f,0.1f,0.1f);
	m_pFinder = NULL;

	m_pcColliderGeoName = "";
	m_pColliderGeo = null;
	m_pCollider = null;
	m_bShowCollider = false;

	m_pcHitSFX = "";
	m_fHitSFXScale = 1.f;
	m_pcHitSFXlod = "";
	m_fHitSFXlodScale = 1.f;
	m_fHitSFXlodDistance = 10.f;
	m_pcHitSound = "";

	m_arbiter.Reset();
	m_bExplodeHit = false;
	m_fExplodeRadius = 1.f;
	m_fExplodeDamage = 1.f;
	m_fExplodePower = 1.f;
}

ShipDamageDetector::~ShipDamageDetector()
{
	m_events.DelAll();
	RELEASE( m_pCollider );
	RELEASE( m_pColliderGeo );
	RELEASE( m_pFinder );
}

//Инициализировать объект
bool ShipDamageDetector::Create(MOPReader & reader)
{
	EditMode_Update(reader);
	Show(true);

	if( !m_pFinder )
		m_pFinder = QTCreateObject(MG_DAMAGEACCEPTOR, _FL_);
	if( m_pFinder )
	{
		m_pFinder->SetBox( m_transform.MulVertexByInverse(m_minBound),
						m_transform.MulVertexByInverse(m_maxBound) );
		m_pFinder->SetMatrix( m_transform );
		m_pFinder->Activate(true);
	}

	return true;
}

void ShipDamageDetector::PostCreate()
{
	static const ConstString arbiterId("CharactersArbiter");
	MissionObject::FindObject(arbiterId, m_arbiter);
}

bool ShipDamageDetector::EditMode_Update(MOPReader & reader)
{
	long n,q;

	m_events.DelAll();
	q = reader.Array();
	if( q>0 )
	{
		m_events.AddElements( q );
		for(n=0; n<q; n++)
		{
			m_events[n].hp = reader.Float();
			m_events[n].trigger.Init( reader );
			m_events[n].bIsDoing = false;
		}
	}

	Vector pos = reader.Position();
	Vector angle = reader.Angles();
	m_transform.Build( angle, pos );

	m_fMaxHP = m_fHP = reader.Float();
	m_pcColliderGeoName = reader.String().c_str();
	IGMXScene* pOldGeo = m_pColliderGeo;
	m_pColliderGeo = Geometry().CreateScene( m_pcColliderGeoName, &Animation(), &Particles(), &Sound(), _FL_ );
	RELEASE( pOldGeo );

	RELEASE( m_pCollider );
	if( m_pColliderGeo )
	{
		m_minBound = m_pColliderGeo->GetBound().vMin;
		m_maxBound = m_pColliderGeo->GetBound().vMax;
		m_pColliderGeo->SetTransform( m_transform );
		m_pCollider = m_pColliderGeo->CreatePhysicsActor( Physics(), true );
	}
	if( m_pCollider )
	{
		m_pCollider->SetTransform( m_transform );
		EnableCollision(false);
	}
	else
		LogicDebug("ShipDamageDetector: Can`t create physics collider from geometry '%s'. To cause is no attack reaction.",m_pcColliderGeoName);

	m_bShowCollider = reader.Bool();

	// effects
	m_pcHitSFX = reader.String().c_str();
	m_fHitSFXScale = reader.Float();
	m_pcHitSFXlod = reader.String().c_str();
	m_fHitSFXlodScale = reader.Float();
	m_fHitSFXlodDistance = reader.Float();
	m_pcHitSound = reader.String().c_str();

	m_bExplodeHit = reader.Bool();
	m_fExplodeRadius = reader.Float();
	m_fExplodeDamage = reader.Float();
	m_fExplodePower = reader.Float();

	Activate(reader.Bool());
	return true;
}

void ShipDamageDetector::EnableCollision(bool bEnable)
{
	if( m_pCollider )
		m_pCollider->SetGroup( bEnable ? phys_grp3 : phys_nocollision );
}

//Активировать
void ShipDamageDetector::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if(!EditMode_IsOn())
	{
		if(IsActive() )
		{
			LogicDebug("Activate");
			SetUpdate(&ShipDamageDetector::Work, ML_TRIGGERS);
			EnableCollision(true);
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&ShipDamageDetector::Work);
			EnableCollision(false);
		}
	}else{
		DelUpdate(&ShipDamageDetector::Work);
		EnableCollision(false);
	}

	if( m_bShowCollider )
		SetUpdate(&ShipDamageDetector::DebugDraw, ML_DEBUG);
	else
		DelUpdate(&ShipDamageDetector::DebugDraw);
}

bool ShipDamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius)
{
	if( !(m_pColliderGeo && m_pCollider) ) return false;

	// по уже убитому не попадаем
	if( m_fHP <= 0.f ) return false;

	if( source == DamageReceiver::ds_sword ||
		source == DamageReceiver::ds_bullet ||
		source == DamageReceiver::ds_shooter ||
		source == DamageReceiver::ds_check)
		return false;

	// бомбу принимаем только от корабля
	if( source == DamageReceiver::ds_bomb &&
		(!obj || *obj->GetObjectType() != 'S') )
		return false;

	// проверим на попадание в бокс
	Vector vmax = m_pColliderGeo->GetBound().vMax;
	Vector voffset = (m_pColliderGeo->GetBound().vMax + m_pColliderGeo->GetBound().vMin) * .5f;
	vmax -= voffset;
	Matrix m = m_pColliderGeo->GetTransform();
	m.pos += voffset;
	Vector vc = center;
	if( !Box::OverlapsBoxSphere( m, vmax, vc, radius ) )
		return false;

	array<Vector> aTrngl(_FL_);
	if( Physics().OverlapSphere(vc,radius,phys_mask(phys_grp3), true,true, aTrngl) )
	{
		// если это только проверка, то возвращаем попадание и не паримся
		if( source == DamageReceiver::ds_check )
			return true;
		// наносим повреждения
		//DoDamage(obj, source, hp);
		// рожаем эффект попадания
		CreateEffect( center, Vector(0.f,1.f,0.f), obj, source );
		return true;
	}

	return false;
}

bool ShipDamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to)
{
	if( !(m_pColliderGeo && m_pCollider) ) return false;

	// по уже убитому не попадаем
	if( m_fHP <= 0.f ) return false;

	if( source == DamageReceiver::ds_sword ||
		source == DamageReceiver::ds_bullet ||
		source == DamageReceiver::ds_shooter ||
		source == DamageReceiver::ds_check)
		return false;

	// проверим на попадание в бокс
	Vector vmax = m_pColliderGeo->GetBound().vMax;
	Vector voffset = (m_pColliderGeo->GetBound().vMax + m_pColliderGeo->GetBound().vMin) * .5f;
	vmax -= voffset;
	Matrix m = m_pColliderGeo->GetTransform();
	m.pos += voffset;
	if( !Box::OverlapsBoxLine( m, vmax, from, to ) )
		return false;

	// проверим точное попадание в коллидер
	IPhysicsScene::RaycastResult HitDetail;
	IPhysBase* result = Physics().Raycast(from, to, phys_mask(phys_grp3), &HitDetail);
	// коллидер в воздухе не считаем за пересечение
	if (HitDetail.mtl == pmtlid_air) result = NULL;
	if( m_pCollider == result )
	{
		// если это только проверка, то возвращаем попадание и не паримся
		if( source == DamageReceiver::ds_check )
			return true;
		// наносим повреждения
		//DoDamage(obj, source, hp);
		// рожаем эффект попадания
		CreateEffect( HitDetail.position, HitDetail.normal, obj, source);
		return true;
	}

	return false;
}

bool ShipDamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4])
{
	return false;
}

void ShipDamageDetector::Restart()
{
	m_fHP = m_fMaxHP;
	for( dword n=0; n<m_events.Size(); n++ )
	{
		m_events[n].bIsDoing = false;
	}
}

//============================================================================================

//Работа детектора
void _cdecl ShipDamageDetector::Work(float dltTime, long level)
{
	for( dword i=m_events.Size(); true; i=m_events.Size() )
	{
		// ищем еще несработавший тригер, с наибольшим ХП, который удовлетворяет текущему порогу ХП
		for( dword n=0; n<m_events.Size(); n++ )
		{
			// пропускаем сработавшие тригеры
			if( m_events[n].bIsDoing ) continue;
			// порог ХП пройден?
			if( m_fHP <= m_events[n].hp )
			{
				// тригер еще не выбран или его ХП больше выбранного ранее
				if( i == m_events.Size() || m_events[n].hp > m_events[i].hp )
					i = n;
			}
		}
		// есть выбранный тригер?
		if( i < m_events.Size() )
		{
			// срабатываем его
			m_events[i].bIsDoing = true;
			m_events[i].trigger.Activate( Mission(), false );
			// запускаем цикл снова
			continue;
		}
		// если дошли до сюда, то выходим из цикла (не осталось тригеров удовлетворяющих запросу)
		break;
	}
}

void _cdecl ShipDamageDetector::DebugDraw(float dltTime, long level)
{
	if( m_pColliderGeo )
	{
		m_pColliderGeo->SetUserColor(Color(1.f, 1.f, 1.f, 1.f));
		m_pColliderGeo->SetTransform( m_transform );
		m_pColliderGeo->Draw();
	}
}

// наносим повреждения
void ShipDamageDetector::DoDamage(MissionObject* srcobj, dword source, float hp)
{
	m_fHP -= hp;
}

// рожаем эффект попадания
void ShipDamageDetector::CreateEffect(const Vector& pos, const Vector& normal, MissionObject* srcobj, dword source)
{
	Matrix mtx(true);
	mtx.BuildPosition( pos );

	// particle
	const char* pcSFX = m_pcHitSFX;
	float fScale = m_fHitSFXScale;
	float dist2cam = ~(Render().GetView().GetCamPos() - pos);
	if( dist2cam < m_fHitSFXlodDistance * m_fHitSFXlodDistance )
	{
		pcSFX = m_pcHitSFXlod;
		fScale = m_fHitSFXlodScale;
	}
	//if( m_pcHitSFX )
	//	Particles().CreateParticleSystemEx2( m_pcHitSFX, mtx, true, _FL_ );
	if( pcSFX )
	{
		IParticleSystem* pParticle = Particles().CreateParticleSystemEx2( pcSFX, mtx, false, _FL_ );
		if( pParticle )
		{
			pParticle->SetScale(fScale);
			pParticle->AutoDelete(true);
		}
	}

	// sound
	if( m_pcHitSound )
		Sound().Create3D( m_pcHitSound, pos, _FL_ );

	if (m_bExplodeHit)
	{
		if(m_arbiter.Validate())
		{
			((ICharactersArbiter *)m_arbiter.Ptr())->Boom(this, DamageReceiver::ds_cannon, pos, m_fExplodeRadius, m_fExplodeDamage, m_fExplodePower);
		}
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(ShipDamageDetector, "Ship damage detector", '1.00', 0x0fffffff, "Damage detector from ships attack, triggering by receiving damage portion", "Logic")
	MOP_ARRAYBEG("Receive damage events", 1, 20)
		MOP_FLOAT("Last hp value",0.f)
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
	MOP_POSITION( "Pos", Vector(0.f) );
	MOP_ANGLES( "Angle", Vector(0.f) );
	MOP_FLOAT("HP",0.f)
	MOP_STRING("Collider geometry","")
	MOP_BOOL("Show collider", false)
	MOP_GROUPBEG("Effects")
		MOP_STRING("HitSFX","")
		MOP_FLOAT("HitSFXScale",1.f)
		MOP_STRING("HitSFXlod","")
		MOP_FLOAT("HitSFXlodScale",1.f)
		MOP_FLOAT("HitLodDistance",10.f)
		MOP_STRING("HitSound","")
		MOP_BOOLC("Explode On", true, "Make explode into hit point")
		MOP_FLOATEX("Explode Radius", 2.0f, 0.1f, 1000000.0f)
		MOP_FLOAT("Explode Damage", 100.0f)
		MOP_FLOATEX("Explode Power", 1.0f,0.1f,100.0f)
	MOP_GROUPEND()
	MOP_BOOL("Active", true)
MOP_ENDLIST(ShipDamageDetector)
