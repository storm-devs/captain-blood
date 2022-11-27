
#include "ExplosionPatch.h"
#include "..\Common_h\ICharactersArbiter.h"
#include "..\common_h\AnimationStdEvents.h"

//============================================================================================

ExplosionPatchCollider::ExplosionPatchCollider()
{
	m_pPatchGMX = NULL;
	m_pPatchCollider = NULL;
	m_bActivateByCreate = false;
}

ExplosionPatchCollider::~ExplosionPatchCollider()
{
	RELEASE(m_pPatchCollider);
	RELEASE(m_pPatchGMX);
}

bool ExplosionPatchCollider::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	if( !EditMode_IsOn() )
		Activate(m_bActivateByCreate);
	return true;
}

void ExplosionPatchCollider::PostCreate()
{
	if( EditMode_IsOn() )
		SetUpdate(&ExplosionPatchCollider::WorkEdit, ML_ALPHA1);
}

bool ExplosionPatchCollider::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

void ExplosionPatchCollider::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if( m_pPatchCollider )
		m_pPatchCollider->Activate(isActive);

	LogicDebug("ExplosionPatchCollider %s is %s now", GetObjectID().c_str(), isActive?"active":"inactive");
}

void ExplosionPatchCollider::ReadMOPs(MOPReader & reader)
{
	// patch
	ConstString pathName = reader.String();
	Vector ang = reader.Angles();
	Vector pos = reader.Position();
	m_mtxPatchTransform.Build( ang, pos );

	// recreate patch
	if( !m_pPatchGMX || ConstString(m_pPatchGMX->GetFileName())!=pathName )
	{
		RELEASE(m_pPatchCollider);
		RELEASE(m_pPatchGMX);
		m_pPatchGMX = Geometry().CreateScene( pathName.c_str(), &Animation(), &Particles(), &Sound(), _FL_ );
		if( m_pPatchGMX )
			m_pPatchCollider = m_pPatchGMX->CreatePhysicsActor( Physics(), false );
		if( m_pPatchCollider )
		{
			m_pPatchCollider->SetTransform( m_mtxPatchTransform );
			m_pPatchCollider->SetGroup( phys_grp3 );
			m_pPatchCollider->EnableCollision(false);
			m_pPatchCollider->Activate(false);
		}
	}

	// activate state
	m_bActivateByCreate = reader.Bool();
}

void _cdecl ExplosionPatchCollider::WorkEdit(float dltTime, long level)
{
	if( Mission().EditMode_IsAdditionalDraw() && EditMode_IsSelect() && m_pPatchGMX )
	{
		m_pPatchGMX->SetUserColor(Color(1.f, 1.f, 1.f, 1.f));
		m_pPatchGMX->SetTransform( m_mtxPatchTransform );
		m_pPatchGMX->Draw();
	}
}


//============================================================================================

ExplosionPatch::ExplosionPatch() :
	m_hitParams(_FL_),
	m_models(_FL_),
	m_aReadyExplosions(_FL_),
	m_aGmxPreload(_FL_)
{
	m_arbiter.Reset();
	m_bIsShadowCast = false;
	m_nExplosionLimit = 0;
	m_nExplosionDeleteLimit = 0;
	m_fModelFadeTime = 0.f;
	m_fModelFadePower = 0.5f;
	m_fTraceBottom = -0.3f;
	m_fTraceTop = 20.f;
	m_bTurnOnDebugTrace = false;
	m_nLevelDraw = ML_PARTICLES5+10;
	m_bRefraction = false;
}

ExplosionPatch::~ExplosionPatch()
{
	// отгружаем модели
	ReleaseModels();
	ReleasePreloadModels();
}


//============================================================================================

//Инициализировать объект
bool ExplosionPatch::Create(MOPReader & reader)
{
	// read mission params
	ReadMOPs(reader);

	// for edit mode:
	if( EditMode_IsOn() )
	{
		SetUpdate(&ExplosionPatch::WorkEdit, m_nLevelDraw);
	}
	else
	{
		long n,q;
		// create models buffer for render
		ReleaseModels();
		m_models.AddElements( m_nExplosionLimit );
		for( n=0; n<m_nExplosionLimit; n++ )
		{
			m_models[n].gmxModel = NULL;
			m_models[n].timer = 0.f;
			m_models[n].state = est_free;
		}

		// preload models
		ReleasePreloadModels();
		for( n=0; n<m_hitParams; n++ )
		{
			for( q=0; q<m_hitParams[n].hits; q++ )
				PreloadModel( m_hitParams[n].hits[q].modelName, m_hitParams[n].hits[q].modelAni );
		}
	}

	return true;
}

void ExplosionPatch::PostCreate()
{
	static const ConstString arbiterId("CharactersArbiter");
	MissionObject::FindObject(arbiterId, m_arbiter);

	Activate( true );
	SetUpdate(&ExplosionPatch::WorkGame, m_nLevelDraw);
	if( m_bIsShadowCast )
		Registry(MG_SHADOWCAST, (MOF_EVENT)&ExplosionPatch::ShadowDraw, ML_GEOMETRY1);

	if( m_bRefraction )
		Registry(MG_SEAREFRACTION, (MOF_EVENT)&ExplosionPatch::SeaRefraction, ML_GEOMETRY1);
}

// редактировать объект
bool ExplosionPatch::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

void ExplosionPatch::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;
	if (string::IsEqual(id, "Explosion"))
	{
		// проверка на параметр: тип попадания
		if( numParams < 1 ) {
			api->Trace("Error: Command 'Explosion' will be have parameter: <explosion type>");
			return;
		}
		// тип попадания
		long nExplosionType = GetHitGroup(params[0]);
		if( nExplosionType < 0 ) {
			api->Trace("Error: Command 'Explosion' can`t find explosion type: %s", params[0]);
			return;
		}

		// проверка на параметр: объект от которого берется позиция попадания
		if( numParams < 2 ) {
			api->Trace("Error: Command 'Explosion' will be have parameter: <object id>");
			return;
		}
		// ищем объект для рождения взрыва
		ConstString fid(params[1]);
		MOSafePointer sptr;
		if( !FindObject(fid,sptr) ) {
			api->Trace("Error: Command 'Explosion' can`t find object: %s", params[1]);
			return;
		}

		// берем смещение позиции попадания
		Vector pos;
		if( numParams < 5 )
			pos = 0.f;
		else
		{
			pos.x = (float)atof(params[2]);
			pos.y = (float)atof(params[3]);
			pos.z = (float)atof(params[4]);
		}
		Matrix mtx(true);
		sptr.Ptr()->GetMatrix(mtx);

		// собственно рождаем эффект взрыва
		MakeExplosion( mtx.MulVertex(pos), nExplosionType );
	}
	else if (string::IsEqual(id, "ondebug"))
	{
		m_bTurnOnDebugTrace = !m_bTurnOnDebugTrace;
		Console().Trace(COL_CMD_OUTPUT,"Explosion debug mode is turn %s", m_bTurnOnDebugTrace?"on":"off");
	}
}

long ExplosionPatch::FindHitParams(const HitGroup& hitGroup, dword dwExplosionMaterial)
{
	for( long n=0; n<hitGroup.hits; n++ )
		if( hitGroup.hits[n].explosionMaterial == dwExplosionMaterial )
			return n;

	return -1;
}

dword ExplosionPatch::GetExplodeMaterial( ConstString& matID )
{
	if( matID == ConstString("air") )
		return pmtlid_air;
	if( matID == ConstString("ground") )
		return pmtlid_ground;
	if( matID == ConstString("stone") )
		return pmtlid_stone;
	if( matID == ConstString("sand") )
		return pmtlid_sand;
	if( matID == ConstString("wood") )
		return pmtlid_wood;
	if( matID == ConstString("grass") )
		return pmtlid_grass;
	if( matID == ConstString("water") )
		return pmtlid_water;
	if( matID == ConstString("iron") )
		return pmtlid_iron;
	if( matID == ConstString("fabrics") )
		return pmtlid_fabrics;
	if( matID == ConstString("other1") )
		return pmtlid_other1;
	if( matID == ConstString("other2") )
		return pmtlid_other2;
	if( matID == ConstString("other3") )
		return pmtlid_other3;
	if( matID == ConstString("other4") )
		return pmtlid_other4;
	return 0;
}

long ExplosionPatch::GetHitGroup(const char* groupName)
{
	ConstString group( groupName );
	for( long n=0; n<m_hitParams; n++ )
		if( m_hitParams[n].type == group )
			return n;
	return -1;
}

void ExplosionPatch::MakeExplosion(const Vector& pos, long nHitGroupIndex)
{
	dword dwMaterial = 0xFFFFFFFF;

	Vector vPosExplosion = pos;
	// проверим попадание в коллидер
	IPhysicsScene::RaycastResult HitDetail;
	Vector from = pos; from.y += m_fTraceTop;
	Vector to = pos; to.y += m_fTraceBottom;
	IPhysBase* result = Physics().Raycast(from, to, phys_mask(phys_grp3), &HitDetail);
	if( result )
	{
		dwMaterial = HitDetail.mtl;
		vPosExplosion.y = HitDetail.position.y;
	}
	// промахнулись?
	if (m_bTurnOnDebugTrace && !result)
		api->Trace("ExplosionPatch: patch raycast failed (%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f)",
			from.x, from.y, from.z, to.x, to.y, to.z );

	MakeExplosion(vPosExplosion,nHitGroupIndex,dwMaterial);
}

void ExplosionPatch::MakeExplosion(const Vector& pos, long nHitGroupIndex, dword dwExplosionMaterial)
{
	if( nHitGroupIndex<0 ) return;
	if( dwExplosionMaterial == 0xFFFFFFFF ) return;

	Matrix mPos;
	mPos.BuildPosition(pos);

	long idxHitParams = FindHitParams( m_hitParams[nHitGroupIndex], dwExplosionMaterial );

	// model explosion
	if( idxHitParams >= 0 )
	{
		m_critSection.Enter();
		long n = m_aReadyExplosions.Add();
		m_aReadyExplosions[n].pos = pos;
		m_aReadyExplosions[n].group = nHitGroupIndex;
		m_aReadyExplosions[n].material = idxHitParams;
		m_critSection.Leave();
	}
	else
		api->Trace("ExplosionPatch: cant find hit parameters for material %d", dwExplosionMaterial);
}

void ExplosionPatch::MakeExplosion(const Vector& pos, const char* groupName)
{
	long ngrp = GetHitGroup(groupName);
	if( ngrp < 0 )
		api->Trace("ExplosionPatch: can`t find explosion type %s", groupName);
	else
		MakeExplosion(pos,ngrp);
}

void ExplosionPatch::MakeExplosion(const Vector& pos, const ConstString & groupName)
{
	for( long n=0; n<m_hitParams; n++ )
		if( m_hitParams[n].type == groupName )
			MakeExplosion(pos,n);
	if( n == m_hitParams )
		api->Trace("ExplosionPatch: can`t find explosion type %s", groupName.c_str());
}

void ExplosionPatch::ClearExplosions()
{
	for( long n=0; n<m_models; n++ )
		if( m_models[n].state != est_free )
			FreeModel(n);
}

//============================================================================================

void _cdecl ExplosionPatch::WorkGame(float dltTime, long level)
{
	// запросы на рождение взрывов. Могут быть из других потоков/анимация/, поэтому в крит секции
	m_critSection.Enter();
	for( long n=0; n<m_aReadyExplosions; n++ )
		BorrowModel( m_aReadyExplosions[n].pos, m_hitParams[m_aReadyExplosions[n].group].hits[m_aReadyExplosions[n].material] );
	m_aReadyExplosions.DelAll();
	m_critSection.Leave();

	// render geometry
	RenderModels(false);

	// проверка на удаление
	UpdateModels(dltTime);
}

void _cdecl ExplosionPatch::ShadowDraw ()
{
	RenderModels(true);
}

void _cdecl ExplosionPatch::SeaRefraction ()
{
	RenderModels(false);
}

void _cdecl ExplosionPatch::WorkEdit(float dltTime, long level)
{
}

void ExplosionPatch::PreloadModel( ConstString& gmxName, ConstString& aniName )
{
	long n;

	// добавляем новую
	n = m_aGmxPreload.Add();
	m_aGmxPreload[n] = Geometry().CreateScene( gmxName.c_str(), &Animation(), &Particles(), &Sound(), _FL_ );
	if( m_aGmxPreload[n] )
	{
		// загружаем анимацию
		m_aGmxPreload[n]->SetAnimationFile( aniName.c_str() );
		// ставим анимацию на паузу
		IAnimation* pAni = m_aGmxPreload[n]->GetAnimation();
		if( pAni )
		{
			pAni->Pause(true);
			pAni->Release();
		}
	}
}

void ExplosionPatch::ReadMOPs(MOPReader & reader)
{
	long n,q;

	m_bIsShadowCast = false;

	// hit groups
	m_hitParams.DelAll();
	q = reader.Array();
	if( q>0 )
		m_hitParams.AddElements(q);
	for( n=0; n<q; n++ )
	{
		// hit type
		m_hitParams[n].type = reader.String();
		// hit params into group
		long q2 = reader.Array();
		if( q2>0 )
			m_hitParams[n].hits.AddElements(q2);
		for( long n2=0; n2<q2; n2++ )
		{
			m_hitParams[n].hits[n2].explosionMaterial = GetExplodeMaterial( reader.Enum() );

			m_hitParams[n].hits[n2].modelName = reader.String();
			m_hitParams[n].hits[n2].modelAni = reader.String();
			m_hitParams[n].hits[n2].modelTimeout = reader.Float();
			m_hitParams[n].hits[n2].modelScale = reader.Float();
			m_hitParams[n].hits[n2].isDeletedByTimeout = reader.Bool();
			m_hitParams[n].hits[n2].isDynamicLight = reader.Bool();
			m_hitParams[n].hits[n2].isShadowReceive = reader.Bool();
			m_hitParams[n].hits[n2].isShadowCast = reader.Bool();

			if( m_hitParams[n].hits[n2].isShadowCast )
				m_bIsShadowCast = true;
		}
	}

	// limit for explosion at one time
	m_nExplosionLimit = reader.Long();
	Assert(m_nExplosionLimit>0);
	m_nExplosionDeleteLimit = reader.Long();
	m_fModelFadeTime = reader.Float();
	m_fModelFadePower = reader.Float();
	m_fTraceBottom = reader.Float();
	m_fTraceTop = reader.Float();
	m_nLevelDraw = reader.Long();
	m_bRefraction = reader.Bool();
}

long ExplosionPatch::BorrowModel(const Vector& pos, const HitParams& hitParams)
{
	for( long n=0; n<m_models; n++ )
	{
		if( m_models[n].state == est_free )
		{
			// занимаем
			m_models[n].state = est_active;
			m_models[n].isDeletedByQuantityLimit = !hitParams.isDeletedByTimeout;

			// пересоздаем модель
			RELEASE( m_models[n].gmxModel );
			m_models[n].gmxModel = Geometry().CreateScene( hitParams.modelName.c_str(), &Animation(), &Particles(), &Sound(), _FL_ );
			if( !m_models[n].gmxModel )
				return -1;

			m_models[n].gmxModel->SetAnimationFile (hitParams.modelAni.c_str());
			m_models[n].gmxModel->SetDynamicLightState (hitParams.isDynamicLight);
			m_models[n].gmxModel->SetShadowReceiveState (hitParams.isShadowReceive);
			m_models[n].gmxModel->SetFloatAlphaReference (0.003921f);
			m_models[n].gmxModel->SetUserColor(Color(0.f,1.f));

			// ставим позицию
			m_models[n].mtxPos.BuildRotateY( FRAND(PIm2) );
			m_models[n].mtxPos.Mul(hitParams.modelScale);
			m_models[n].mtxPos.w = 1.f;
			m_models[n].mtxPos.pos = pos;
			m_models[n].gmxModel->SetTransform( m_models[n].mtxPos );
			// ставим задержку на удаление
			m_models[n].timer = hitParams.modelTimeout;
			// кастить тень?
			m_models[n].isShadowCast = hitParams.isShadowCast;
			// стартуем анимацию
			IAnimation* pAni = m_models[n].gmxModel->GetAnimation();
			if( pAni )
			{
				pAni->Pause(false);
				pAni->Start();
				pAni->Release();
			}
			return n;
		}
	}
	if( m_bTurnOnDebugTrace )
		api->Trace("ExplosionPatch: can`t create new explosion because conclude limit = %d", m_nExplosionLimit);
	return -1;
}

void ExplosionPatch::FreeModel( long idxModel )
{
	if( idxModel>=0 && idxModel<m_models )
	{
		// освобождаем модель для использования
		m_models[idxModel].state = est_free;

		// паузим анимацию
		if( m_models[idxModel].gmxModel )
		{
			IAnimation* pAni = m_models[idxModel].gmxModel->GetAnimation();
			if( pAni )
			{
				pAni->Pause(true);
				pAni->Release();
			}
		}
	}
}

void ExplosionPatch::GoToNextState( long idxModel )
{
	if( idxModel>=0 && idxModel<m_models )
	{
		switch( m_models[idxModel].state )
		{
			// из свободного сотсояния никуда не переходим (ибо для перехода надо заводить правильные параметры)
			case est_free:
				api->Trace("ExplosionPatch::GoToNextState(%d) : Can`t change state 'free', use BorrowModel() for this", idxModel);
				break;
			// из активного состояния переходим либо в фейд, либо в ожидании удаления по лимиту количества
			case est_active:
				if( m_models[idxModel].isDeletedByQuantityLimit )
					m_models[idxModel].state = est_ready_to_delete;
				else
				{
					m_models[idxModel].state = est_fadeout;
					m_models[idxModel].timer = m_fModelFadeTime;
				}
				break;
			// из ожидания удаления переходим к фейду (удалению)
			case est_ready_to_delete:
				m_models[idxModel].state = est_fadeout;
				m_models[idxModel].timer = m_fModelFadeTime;
				break;
			// по окончанию фейда, освобождаем полностью место для следующего взрыва
			case est_fadeout:
				FreeModel(idxModel);
				break;
		}
		if( m_bTurnOnDebugTrace )
		{
			api->Trace("ExplosionPatch: next state for model %d(%s) is %s",
				idxModel, m_models[idxModel].gmxModel ? m_models[idxModel].gmxModel->GetFileName() : "null",
				m_models[idxModel].state==est_free ? "free" :
				m_models[idxModel].state==est_active ? "active" :
				m_models[idxModel].state==est_ready_to_delete ? "ready_to_delete" :
				m_models[idxModel].state==est_fadeout ? "fade" : "unknown");
		}
	}
}

void ExplosionPatch::ReleaseModels()
{
	for( long n=0; n<m_models; n++ )
	{
		if( m_models[n].gmxModel )
			m_models[n].gmxModel->Release();
	}
	m_models.DelAll();
}

void ExplosionPatch::ReleasePreloadModels()
{
	for( long n=0; n<m_aGmxPreload; n++ )
	{
		if( m_aGmxPreload[n] )
			m_aGmxPreload[n]->Release();
	}
	m_aGmxPreload.DelAll();
}

void ExplosionPatch::RenderModels(bool shadowCast)
{
	for( long n=0; n<m_models; n++ )
	{
		if( m_models[n].state!=est_free && (!shadowCast || m_models[n].isShadowCast))
		{
			// расчет альфы при фейде
			if( m_models[n].gmxModel && m_models[n].state==est_fadeout )
			{
				float alpha = 0.f;
				if( m_fModelFadeTime > 0.f )
					alpha = powf( Clampf( m_models[n].timer / m_fModelFadeTime ), m_fModelFadePower );

				if( m_models[n].gmxModel )
					m_models[n].gmxModel->SetUserColor(Color(0.f,alpha));
			}
			// отрисовка
			if( m_models[n].gmxModel )
				m_models[n].gmxModel->Draw();
		}
	}
}

void ExplosionPatch::UpdateModels( float dltTime )
{
	// идет время
	long usedQuantity = 0;
	for( long n=0; n<m_models; n++ )
	{
		if( m_models[n].state==est_free ) continue;

		if( m_models[n].state != est_fadeout )
			usedQuantity++;

		// если модель ждет удаления через лимит количества, то время не учитываем
		if( m_models[n].state != est_ready_to_delete )
		{
			m_models[n].timer -= dltTime;
			// доиграли до конца - переходим к следующему состоянию
			if( m_models[n].timer <= 0.f )
				GoToNextState(n);
		}
	}

	// удалим лишние модели
	if( usedQuantity >= m_nExplosionDeleteLimit )
	{
		long nLimitOverflow = 1 + usedQuantity - m_nExplosionDeleteLimit;
		for( n=0; n<m_models && nLimitOverflow>0; n++ )
		{
			// тех кто ждет удаления удаляем
			if( m_models[n].state == est_ready_to_delete )
			{
				GoToNextState(n);
				nLimitOverflow--;
			}
		}
	}
}

//============================================================================================
//Параметры инициализации
//============================================================================================
#define HIT_PARAMS \
			MOP_STRING("Explode model","") \
			MOP_STRING("Explode animation","") \
			MOP_FLOAT("Explode timeout",2.f) \
			MOP_FLOAT("Explode model scale",1.f) \
			MOP_BOOLC("FreeByTimeout",false,"After timeout do free of explosion model\n(if true then left model till limit of quantity)") \
			MOP_BOOL("Dynamic light",true) \
			MOP_BOOL("Shadow receive",true) \
			MOP_BOOL("Shadow cast",false)

MOP_BEGINLISTCG(ExplosionPatch, "Explosion patch", '1.00', 100, "Object for create explosion by patch", "default")
	MOP_ENUMBEG ( "ExplosionMaterial" )
	  MOP_ENUMELEMENT ( "air" )
	  MOP_ENUMELEMENT ( "ground" )
	  MOP_ENUMELEMENT ( "stone" )
	  MOP_ENUMELEMENT ( "sand" )
	  MOP_ENUMELEMENT ( "wood" )
	  MOP_ENUMELEMENT ( "grass" )
	  MOP_ENUMELEMENT ( "water" )
	  MOP_ENUMELEMENT ( "iron" )
	  MOP_ENUMELEMENT ( "fabrics" )
	  MOP_ENUMELEMENT ( "other1" )
	  MOP_ENUMELEMENT ( "other2" )
	  MOP_ENUMELEMENT ( "other3" )
	  MOP_ENUMELEMENT ( "other4" )
	MOP_ENUMEND

	MOP_ARRAYBEG( "Hit params", 1, 10 )
		MOP_STRINGC("type","bomb","type of damage")
		MOP_ARRAYBEG( "Hit params", 1, 15 )
			MOP_ENUMC("ExplosionMaterial", "Material", "Material of explosion");
			HIT_PARAMS
		MOP_ARRAYEND
	MOP_ARRAYEND

	MOP_LONGEXC("Explosion limit", 10, 1,100, "Limit of explosion at one time")
	MOP_LONGEXC("Explosion delete limit", 10, 1,100, "Limit of undeletable explosions")
	MOP_FLOATC("Explode fade time",0.8f, "Time for fadeout of explosion")
	MOP_FLOATEXC("Explode fade power",0.5f, 0.05f, 5.f, "Power for fadeout of explosion")
	MOP_FLOAT("Trace bottom distance",-0.3f)
	MOP_FLOAT("Trace top distance",20.f)
	MOP_LONGC("DrawLevel", ML_ALPHA5+10, "Draw level for render of explosions")
	MOP_BOOLC("Refraction", false, "Use refraction by water")
MOP_ENDLIST(ExplosionPatch)

MOP_BEGINLISTCG(ExplosionPatchCollider, "Explosion collider", '1.00', 100, "Patch for explosion", "default")
	MOP_STRINGC("Patch","","Patch with material for explosions")
	MOP_ANGLES("Patch angle",Vector(0.f))
	MOP_POSITION("Patch pos",Vector(0.f))
	MOP_BOOL("Active",true)
MOP_ENDLIST(ExplosionPatchCollider)
