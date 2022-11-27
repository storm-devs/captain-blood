
#include "TargetPoints.h"
#include "ships\ship.h"
#include "..\Common_h\ICharactersArbiter.h"
#include "..\Common_h\IExplosionPatch.h"
#include "..\Common_h\IShooter.h"
#include "SeaMissionParams.h"

//============================================================================================

array<TargetPoints*> TargetPoints::g_targObjectsList(_FL_);
long TargetPoints::g_nTargObjIndex = 0;
float TargetPoints::g_fCooldawn = 0.f;
TargetPoints* TargetPoints::g_pLastTargetPoints = NULL;

TargetPoints::TargetPoints() :
	m_points(_FL_)
{
	m_renderParams.pointradius = 1.f;
	m_renderParams.scalePeriod = 1.f;
	m_renderParams.scaleAmplitude = 1.2f;
	m_renderParams.colPrepare = 0xC060FF60;
	m_renderParams.startSize = 0.5f;
	m_renderParams.startColor = 0x55FFFFFF;
	m_renderParams.finishColor = 0xCCFFFFFF;


	m_renderParams.pTexture = NULL;
	m_renderParams.pVertex = NULL;
	m_renderParams.pIndex = NULL;
	m_renderParams.pShVarTexture = NULL;
	m_renderParams.pShVarCol = NULL;

	m_logicParams.minShowDelay = 0.f;
	m_logicParams.maxShowDelay = 1.f;
	m_logicParams.activeRadius = 5.f;
	m_logicParams.minFlyTime = 1.f;
	m_logicParams.maxFlyTime = 3.f;
	m_logicParams.minFlyHeight = 10.f;
	m_logicParams.maxFlyHeight = 40.f;

	m_hitParams.explodeRadius = 2.f;
	m_hitParams.explodeDamage = 100.f;
	m_hitParams.explodePower = 1.f;
	m_hitParams.hitSFXScale = 1.f;

	g_nTargObjIndex = 0;
	g_targObjectsList.Add(this);
	g_fCooldawn = 0.f;
	g_pLastTargetPoints = NULL;

	m_arbiter.Reset();

	m_ExplosionPatch = NULL;
	m_HitGroupIndex = -1;

	m_bWaitKickOut = false;
	m_bWaitKickOutPrepair = false;
}

TargetPoints::~TargetPoints()
{
	RELEASE(m_renderParams.pTexture);
	RELEASE(m_renderParams.pVertex);
	RELEASE(m_renderParams.pIndex);

	g_nTargObjIndex = 0;
	g_targObjectsList.Del(this);
	g_fCooldawn = 0.f;
	g_pLastTargetPoints = NULL;

	m_ExplosionPatch = NULL;
}


//============================================================================================

//Инициализировать объект
bool TargetPoints::Create(MOPReader & reader)
{
	// shader
	Render().GetShaderId("TargetPoints",m_renderParams.idShader);

	// shader variables
	m_renderParams.pShVarTexture = Render().GetTechniqueGlobalVariable("TargPntTexture",_FL_);
	Assert(m_renderParams.pShVarTexture);
	m_renderParams.pShVarCol = Render().GetTechniqueGlobalVariable("TargPntColor",_FL_);
	Assert(m_renderParams.pShVarCol);

	// read mission params
	ReadMOPs(reader);

	// for edit mode:
	if( EditMode_IsOn() )
	{
		SetUpdate(&TargetPoints::WorkEdit, ML_PARTICLES1);

		// set all points into prepare state
		for( dword n=0; n<m_points.Size(); n++ )
		{
			m_points[n].state = tps_prepare;
			m_points[n].maxFlyTime = 0.f;
			m_points[n].curFlyTime = 0.f;
		}
		UpdatePointsVertex(0.f);
	}

	return true;
}

void TargetPoints::PostCreate()
{
	static const ConstString arbiterId("CharactersArbiter");
	MissionObject::FindObject(arbiterId, m_arbiter);

	static const ConstString seaMisParamsId("SeaMissionParams");
	MissionObject::FindObject(seaMisParamsId, m_seaMisParams);

	if( !FindObject( m_logicParams.watchObjectID, m_logicParams.watchObject ) )
	{
		LogicDebug("Target points array: can`t find watched object");
	}

	m_ExplosionPatch = IExplosionPatch::GetExplosionPatch(Mission());
	if( m_ExplosionPatch )
	{
		m_HitGroupIndex = m_ExplosionPatch->GetHitGroup("ShipCannon");
		if( m_HitGroupIndex < 0 )
		{
			LogicDebug("Target point create: warning - Can`t find explosion patch geometry group 'ShipCannon'");
			m_HitGroupIndex = m_ExplosionPatch->GetHitGroup("bomb");
		}
	}

	static const ConstString strShooterKickOut = ConstString("ShooterKickOut");
	if (!FindObject(strShooterKickOut, m_ShooterKickOut))
		api->Error("ERROR: TargetPoints: Can't find KickOut object ShooterKickOut");
}

// редактировать объект
bool TargetPoints::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);

	// set all points into prepare state
	for( dword n=0; n<m_points.Size(); n++ )
	{
		m_points[n].state = tps_prepare;
		m_points[n].maxFlyTime = 0.f;
		m_points[n].curFlyTime = 0.f;
	}
	UpdatePointsVertex(0.f);

	return true;
}

//Активировать
void TargetPoints::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if(!EditMode_IsOn())
	{
		if(IsActive() )
		{
			LogicDebug("Activate");
			SetUpdate(&TargetPoints::WorkGame, ML_PARTICLES1);
		}else{
			LogicDebug("Deactivate");
			DelUpdate(&TargetPoints::WorkGame);
		}
	}
}

long TargetPoints::GetPointsArray(long shootQuantity, array<long>& points)
{
	long leftQnt = shootQuantity;
	long chooseQnt = 0;

	// дистанция в квадрате
	float dist2pow = m_logicParams.activeRadius * m_logicParams.activeRadius;

	// позиция источника (игрока)
	Vector cpos = 0.f;
	if( m_logicParams.watchObject.Validate() )
	{
		Matrix mtx(true);
		m_logicParams.watchObject.Ptr()->GetMatrix(mtx);
		cpos = mtx.pos;
	}

	// массив дистанций
	array<float> aPowDist(_FL_);
	points.DelAll();

	for(dword n=0; n<m_points.Size() && leftQnt>0; n++)
	{
		// пропускаем уже выбранные точки
		if( m_points[n].state == tps_prepare ) continue;
		// дистанция до точки
		float curDist = ~(m_points[n].pos - cpos);
		// пропускаем точки которые далеко от нас
		if( curDist > dist2pow ) continue;
		points.Add( n );
		aPowDist.Add( curDist );
		chooseQnt++;
		leftQnt--;
	}

	// лимит на количество
	while(chooseQnt > m_logicParams.pointsLimit)
	{
		dword imax = 0;
		for(n=1; n<aPowDist.Size(); n++)
			if( aPowDist[n] > aPowDist[imax] )
				imax = n;
		aPowDist.DelIndex(imax);
		points.DelIndex(imax);
		chooseQnt--;
	}

	return chooseQnt;
}

const Vector & TargetPoints::GetPointPos(long n)
{
	return m_points[n].pos;
}

void TargetPoints::ResetPoint(long n)
{
	if( n < m_points )
	{
		m_points[n].state = tps_disable;
	}
}

void TargetPoints::BorrowPoint(long n, float flyTime)
{
	m_points[n].state = tps_showdelay;
	m_points[n].maxFlyTime = flyTime;
	m_points[n].curFlyTime = 0.f;

	// подготавливаем выбиватель из шутера
	if( !m_bWaitKickOutPrepair && !m_bWaitKickOut )
	{
		m_bWaitKickOutPrepair = true;
		m_bWaitKickOut = false;
	}
}

void TargetPoints::BorrowTargetPoints(TargetPoints* pTargPoints, MissionObject* pObj)
{
	if( g_pLastTargetPoints != NULL ) return;
	if( g_fCooldawn > 0.f ) return;
	if( pTargPoints == NULL ) return;

	g_pLastTargetPoints = pTargPoints;
	g_fCooldawn = pTargPoints->GetCooldawn();
	// в следующий раз берем другой набор точек
	g_nTargObjIndex++;
	// поставим позицию для стреляющего
	if( pObj )
	{
		Matrix mtx(true);
		pObj->GetMatrix(mtx);
		pTargPoints->SetSrcPosition(mtx.pos);
	}
}

void TargetPoints::GetTimeHeightLimit(float & fTimeLimit, float & fHeightLimit)
{
	fTimeLimit = m_logicParams.minFlyTime + FRAND( m_logicParams.maxFlyTime - m_logicParams.minFlyTime );
	fHeightLimit = m_logicParams.minFlyHeight + FRAND( m_logicParams.maxFlyHeight - m_logicParams.minFlyHeight );
}

void TargetPoints::MakePointDamage(long n)
{
	Vector vPos = m_points[n].pos;
	Matrix mPos;
	mPos.BuildPosition(vPos);

	// particle
	if( !m_hitParams.hitSFX.IsEmpty() )
	{
		IParticleSystem* pParticle = Particles().CreateParticleSystemEx2( m_hitParams.hitSFX.c_str(), mPos, false, _FL_ );
		if( pParticle )
		{
			pParticle->SetScale( m_hitParams.hitSFXScale );
			pParticle->AutoDelete(true);
		}
	}

	// sound
	if( !m_hitParams.hitSound.IsEmpty() )
		Sound().Create3D( m_hitParams.hitSound.c_str(), vPos, _FL_ );

	// boom
	if(m_arbiter.Validate())
	{
		((ICharactersArbiter *)m_arbiter.Ptr())->Boom(this, DamageReceiver::ds_cannon, vPos, m_hitParams.explodeRadius, m_hitParams.explodeDamage, m_hitParams.explodePower);
	}

	// попытка выкинуть из шутера
	if( m_bWaitKickOut )
	{
		if( m_ShooterKickOut.Validate() )
			m_bWaitKickOut = !((IShooterKickOut*)m_ShooterKickOut.Ptr())->KickOut(m_vSrcPosition, vPos);
	}

	// рождаем модель для взрыва
//	if( m_ExplosionPatch && m_HitGroupIndex >= 0 )
//		m_ExplosionPatch->MakeExplosion( vPos, m_HitGroupIndex );
}

TargetPoints* TargetPoints::GetNextTargetPoint(IMission* pMission)
{
	if( g_pLastTargetPoints != NULL ) return NULL;
	if( g_fCooldawn > 0.f ) return NULL;

	TargetPoints* pFirstFinded = NULL;
	TargetPoints* pLastFinded = NULL;

	long nFindCount = 0;
	for( long n=0; n<g_targObjectsList; n++ )
	{
		// текущий обхект принадлежит данной миссии?
		if( pMission == &g_targObjectsList[n]->Mission() )
		{
			// нашли искомое
			if( nFindCount == g_nTargObjIndex )
			{
				pLastFinded = g_targObjectsList[n];
				break;
			}
			nFindCount++;
			// это будет первая находка
			if( !pFirstFinded )
				pFirstFinded = g_targObjectsList[n];
		}
	}

	// не нашли?
	if( pLastFinded == NULL )
	{
		// никого нет?
		if( pFirstFinded == NULL )
			return NULL;
		// берем первого попавшегося и правим индекс на 0
		pLastFinded = pFirstFinded;
		g_nTargObjIndex = 0;
	}

	if( !pLastFinded )
		return NULL;

	// в следубщий раз берем другого
	//g_nTargObjIndex++;
	return pLastFinded;
}

//============================================================================================

void _cdecl TargetPoints::WorkGame(float dltTime, long level)
{
	// draw points
	UpdatePointsVertex(dltTime);
	DrawPoints();

	// включаем предупреждение кикера из шутера
	if( m_renderParams.pointQPrepare > 0 && m_bWaitKickOutPrepair )
	{
		m_bWaitKickOutPrepair = false;
		m_bWaitKickOut = true;
		if( m_ShooterKickOut.Validate() )
			((IShooterKickOut*)m_ShooterKickOut.Ptr())->KickOutPrepair();
	}

	// обновление кулдауна на выстрелы
	if( this == g_pLastTargetPoints )
	{
		g_fCooldawn -= dltTime;
		if( g_fCooldawn <= 0.f )
			g_pLastTargetPoints = NULL;
	}
}

void _cdecl TargetPoints::WorkEdit(float dltTime, long level)
{
	// draw points
	if( m_editorParams.isShowPoints )
	{
		DrawPoints();
	}
}

void TargetPoints::ReadMOPs(MOPReader & reader)
{
	dword n;

	// render params
	m_renderParams.texturename = reader.String();
	m_renderParams.pointradius = reader.Float();
	m_renderParams.scalePeriod = reader.Float();
	m_renderParams.scaleAmplitude = 1.f + reader.Float();
	m_renderParams.colPrepare = reader.Colors();
	m_renderParams.startSize = reader.Float();
	m_renderParams.startColor = reader.Colors();
	m_renderParams.finishColor = reader.Colors();

	// logic params
	m_logicParams.minShowDelay = reader.Float();
	m_logicParams.maxShowDelay = reader.Float();
	m_logicParams.activeRadius = reader.Float();
	m_logicParams.watchObjectID = reader.String();
	m_logicParams.cooldawn = reader.Float();
	m_logicParams.pointsLimit = reader.Long();
	m_logicParams.minFlyTime = reader.Float();
	m_logicParams.maxFlyTime = reader.Float();
	m_logicParams.minFlyHeight = reader.Float();
	m_logicParams.maxFlyHeight = reader.Float();

	// hit params
	m_hitParams.explodeRadius = reader.Float();
	m_hitParams.explodeDamage = reader.Float();
	m_hitParams.explodePower = reader.Float();
	m_hitParams.hitSFX = reader.String();
	m_hitParams.hitSFXScale = reader.Float();
	m_hitParams.hitSound = reader.String();

	// target points
	m_points.DelAll();
	dword q = reader.Array();
	m_points.AddElements(q);
	for( n=0; n<q; n++ )
	{
		m_points[n].pos = reader.Position();
		m_points[n].state = tps_disable;
		m_points[n].maxFlyTime = 0.f;
		m_points[n].curFlyTime = 0.f;
	}

	// editor params
	m_editorParams.isShowPoints = reader.Bool();

	// activate state
	Activate( reader.Bool() );

	// create depended params:
	// texture
	RELEASE( m_renderParams.pTexture );
	m_renderParams.pTexture = Render().CreateTexture( _FL_, "%s", m_renderParams.texturename.c_str() );
	// vertex buffer
	RELEASE( m_renderParams.pVertex );
	m_renderParams.pointQMax = m_points.Size();
	m_renderParams.pointQPrepare = m_renderParams.pointQMax;
	m_renderParams.pVertex = Render().CreateVertexBuffer( m_renderParams.pointQMax * 4 * sizeof(TargetPointVertex), sizeof(TargetPointVertex), _FL_ );
	// index buffer
	RELEASE( m_renderParams.pIndex );
	m_renderParams.pIndex = Render().CreateIndexBuffer( m_renderParams.pointQMax * 6 * sizeof(unsigned short), _FL_ );
	// watch object
	FindObject( m_logicParams.watchObjectID, m_logicParams.watchObject );

	// fill index buffer
	Assert( m_renderParams.pIndex );
	unsigned short* pI = (unsigned short*)m_renderParams.pIndex->Lock();
	Assert( pI );
	for( n=0; n<m_renderParams.pointQMax; n++ )
	{
		pI[n*6] = n*4;
		pI[n*6+1] = n*4 + 1;
		pI[n*6+2] = n*4 + 2;
		pI[n*6+3] = n*4 + 2;
		pI[n*6+4] = n*4 + 1;
		pI[n*6+5] = n*4 + 3;
	}
	m_renderParams.pIndex->Unlock();
}

void TargetPoints::InitShowDelay()
{
	// поиск минимального и максимального времени полета ядра
	float minFlyTime = 10000.f;
	float maxFlyTime = 0.f;
	for( dword n=0; n<m_points.Size(); n++ )
	{
		if( m_points[n].state == tps_showdelay )
		{
			minFlyTime = Min(minFlyTime, m_points[n].maxFlyTime);
			maxFlyTime = Max(maxFlyTime, m_points[n].maxFlyTime);
		}
	}

	float delay = m_logicParams.maxShowDelay - m_logicParams.minShowDelay;
	float fMinDelay = m_logicParams.minShowDelay;
	if( fMinDelay > maxFlyTime * 0.9f )
		fMinDelay = maxFlyTime * 0.9f;
	// если задержка больше половины времени полета ядра, то уменьшаем задержку
	if( fMinDelay + delay > maxFlyTime * 0.98f )
		delay = maxFlyTime * 0.98f - fMinDelay;
	// коеффициент для расчета времени задержки
	float kTime = (maxFlyTime - minFlyTime > 0.01f) ? (delay / (maxFlyTime - minFlyTime)) : 0.f;

	for( n=0; n<m_points.Size(); n++ )
	{
		if( m_points[n].state == tps_showdelay )
		{
			float time = fMinDelay + kTime * (m_points[n].maxFlyTime - minFlyTime);
			m_points[n].curFlyTime = -time;
			m_points[n].maxFlyTime -= time;
			m_points[n].state = tps_prepare;
		}
	}
}

void TargetPoints::UpdatePointsVertex(float dltTime)
{
	Assert(m_renderParams.pVertex);

	m_renderParams.pointQPrepare = 0;

	// массив вертексов
	TargetPointVertex* pV = (TargetPointVertex*)m_renderParams.pVertex->Lock();

	float sz = m_renderParams.pointradius;

	// текущий номер вертекса
	dword q = 0;
	for( dword n=0; n<m_points.Size(); n++ )
	{
		// если попалась новая точка, то было обновление: ставим время задержки для них
		if( m_points[n].state == tps_showdelay )
			InitShowDelay();

		if( m_points[n].state == tps_prepare )
		{
			// до начала времени ничего не показываем
			if( m_points[n].curFlyTime < 0.f )
			{
				m_points[n].curFlyTime += dltTime;
				if( m_points[n].curFlyTime < 0.f )
					continue;
			}

			float x = m_points[n].pos.x;
			float y = m_points[n].pos.y;
			float z = m_points[n].pos.z;

			float fScaleK = Clamp( m_points[n].curFlyTime / m_points[n].maxFlyTime );
			dword dwCol = Color().LerpA( Color(m_renderParams.startColor), Color(m_renderParams.finishColor), fScaleK ).GetDword();

			float uvScale = 1.f / (m_renderParams.startSize + (1.f - m_renderParams.startSize) * fScaleK);

			if( m_points[n].curFlyTime < m_points[n].maxFlyTime )
				m_points[n].curFlyTime += dltTime;

			pV[q].pos = Vector(x-sz,y,z+sz);
			pV[q].col = dwCol;
			pV[q].u = 0.f;
			pV[q].v = 0.f;
			pV[q].u2 = 0.75f - uvScale*.25f;
			pV[q].v2 = 0.5f - uvScale*.5f;

			pV[q+1].pos = Vector(x+sz,y,z+sz);
			pV[q+1].col = dwCol;
			pV[q+1].u = 0.5f;
			pV[q+1].v = 0.f;
			pV[q+1].u2 = 0.75f + uvScale*.25f;
			pV[q+1].v2 = 0.5f - uvScale*.5f;

			pV[q+2].pos = Vector(x-sz,y,z-sz);
			pV[q+2].col = dwCol;
			pV[q+2].u = 0.f;
			pV[q+2].v = 1.f;
			pV[q+2].u2 = 0.75f - uvScale*.25f;
			pV[q+2].v2 = 0.5f + uvScale*.5f;

			pV[q+3].pos = Vector(x+sz,y,z-sz);
			pV[q+3].col = dwCol;
			pV[q+3].u = 0.5f;
			pV[q+3].v = 1.f;
			pV[q+3].u2 = 0.75f + uvScale*.25f;
			pV[q+3].v2 = 0.5f + uvScale*.5f;

			q += 4;
			m_renderParams.pointQPrepare++;
		}
	}

	m_renderParams.pVertex->Unlock();
}

void TargetPoints::DrawPoints()
{
	// get quantity of squares
	dword squareQantity = m_renderParams.pointQPrepare;
	if( squareQantity == 0 ) return;

	// set texture
	if( m_renderParams.pTexture )
		m_renderParams.pShVarTexture->SetTexture(m_renderParams.pTexture);
	else
		m_renderParams.pShVarTexture->ResetTexture();

	// set color
	m_renderParams.pShVarCol->SetVector4( Color(m_renderParams.colPrepare).v4 );

	// stream sources
	Render().SetStreamSource(0, m_renderParams.pVertex);
	Render().SetIndices(m_renderParams.pIndex);

	Render().SetWorld(Matrix());

	// draw squares
	Render().DrawIndexedPrimitive( m_renderParams.idShader, PT_TRIANGLELIST, 0, squareQantity*4, 0, squareQantity*2 );
}

//============================================================================================
//Параметры инициализации
//============================================================================================
MOP_BEGINLISTCG(TargetPoints, "Target points array", '1.00', 100, "Target point set for enemy sea ship", "Arcade Sea")
	MOP_GROUPBEG("Render params")
		MOP_STRING("Texture name", "")
		MOP_FLOATC("Point radius", 1.0f, "Size of target point")
		MOP_FLOATC("Blinding period", 1.0f, "Time period for blinding of the target points (sec)")
		MOP_FLOATC("Blinding amplutude", 0.2f, "Amplutude change while blinding (relative value: 0.5 = half of radius")
		MOP_COLORC("Prepare color",Color(0xC060FF60),"Color of target point wait for ship cannon shoot")
		MOP_FLOATEXC("Start size", 0.2f, 0.1f, 1.f, "Start point size (grow to 1.0 until cannon ball fly)")
		MOP_COLORC("Start color",Color((dword)0x55FFFFFF),"Start color of target point")
		MOP_COLORC("Finish color",Color(0xCCFFFFFF),"Final color of target point")
	MOP_GROUPEND()

	MOP_GROUPBEG("Logic params")
		MOP_FLOATC("Min show delay", 1.0f, "Time delay for show target point min(sec)")
		MOP_FLOATC("Max show delay", 1.0f, "Time delay for show target point max(sec)")
		MOP_FLOATC("Active radius", 5.0f, "Maximum distance from player to choosed points")
		MOP_STRINGC("Watch object", "Player", "Mission object which activated target points group by enter into area box")
		MOP_FLOATC("Cooldown", 5.0f, "Time until ship cant shoot by target points")
		MOP_LONGC("Points limit", 15, "Limit of simultaneous target points for shoot")
		MOP_FLOATC("Min fly time", 1.0f, "Minimal time of cannonball flying")
		MOP_FLOATC("Max fly time", 3.0f, "Maximal time of cannonball flying")
		MOP_FLOATC("Min fly height", 10.0f, "Minimal height of cannonball flying")
		MOP_FLOATC("Max fly height", 40.0f, "Maximal height of cannonball flying")
	MOP_GROUPEND()

	MOP_GROUPBEG("Hit params")
		MOP_FLOATEX("Explode Radius", 2.0f, 0.1f, 1000000.0f)
		MOP_FLOAT("Explode Damage", 100.0f)
		MOP_FLOATEX("Explode Power", 1.0f,0.1f,100.0f)
		MOP_STRING("HitSFX","")
		MOP_FLOAT("HitSFXScale",1.f)
		MOP_STRING("HitSound","")
	MOP_GROUPEND()

	MOP_ARRAYBEG( "Points", 1, 100 )
		MOP_POSITION( "pos", Vector(0.f) )
	MOP_ARRAYEND

	MOP_GROUPBEG("Editor params")
		MOP_BOOL("Show points", false)
	MOP_GROUPEND()

	MOP_BOOL("Active",true)
MOP_ENDLIST(TargetPoints)
