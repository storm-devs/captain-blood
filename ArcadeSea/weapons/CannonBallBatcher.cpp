#include "CannonBallBatcher.h"
#include "Weapon.h"
#include "..\SeaMissionParams.h"
#include "..\TargetPoints.h"

inline float lerp(float a, float b, float t) { return a + (b-a)*Clamp(t); }

CannonBallBatcher::CannonBallBatcher() :
m_Positions(__FILE__, __LINE__, m_nMaxInstance),
m_shadowTexture(NULL),
m_shadowTextureVariable(NULL),
m_vbShadow(NULL),
m_ibShadow(NULL),
m_pAniModel(NULL),
m_pAni(NULL)
{
	IRender * render = (IRender *)api->GetService("DX9Render");
	Assert(render);


	render->GetShaderId("BallShadow", BallShadow_id);

	// shadow initialization section
	m_shadowTextureVariable = render->GetTechniqueGlobalVariable("ballShadowTex", __FILE__, __LINE__);
	Assert(m_shadowTextureVariable);
	InitShadowBuffers();

	m_nBoneQuantity = m_nMaxInstance;
}

void CannonBallBatcher::SetShadowTexture(const char* pcTextureName)
{
	IRender * render = (IRender *)api->GetService("DX9Render");
	Assert(render);

	IBaseTexture* pOldTex = m_shadowTexture;
	m_shadowTexture = render->CreateTexture(__FILE__, __LINE__, pcTextureName ? pcTextureName : "ballShadow");
	if( m_shadowTexture )
		m_shadowTextureVariable->SetTexture(m_shadowTexture);
	RELEASE( pOldTex );
}

CannonBallBatcher::~CannonBallBatcher(void)
{
	// shadow release
	RELEASE(m_vbShadow);
	RELEASE(m_ibShadow);
	RELEASE(m_shadowTexture);
	m_shadowTextureVariable = NULL;

	ReleaseModel();
}

void CannonBallBatcher::AddInstance(const Vector& pos)
{
	m_Positions.Add(pos);
	if( m_Positions >= m_nBoneQuantity || m_Positions >= m_nMaxInstance )
	{
		Draw();
		Flush();
	}
}

void CannonBallBatcher::Draw()
{
	unsigned int instanceCount = m_Positions.Size();
	if( instanceCount==0 ) return;

	IRender * render = (IRender *)api->GetService("DX9Render");
	Assert(render);

	// shadow render
	Update();
	render->SetStreamSource(0, m_vbShadow);
	render->SetIndices(m_ibShadow);
	render->SetWorld(Matrix());
	if( m_shadowTextureVariable )
	{
		if( m_shadowTexture )
			m_shadowTextureVariable->SetTexture(m_shadowTexture);
		else
			m_shadowTextureVariable->ResetTexture();
	}
	Assert(instanceCount <= m_nMaxInstance);
	render->DrawIndexedPrimitive(BallShadow_id, PT_TRIANGLELIST, 0, instanceCount*4, 0, instanceCount*2);

	// cannon ball render
	if( m_pAniModel )
	{
		if( m_pAni )
		{
			Matrix mtx;
			// ставим позиции для используемых ядер
			for( long n=0; n<m_nBoneQuantity && n<m_Positions && n<m_nMaxInstance; n++ )
			{
				mtx.pos = m_Positions[n].v;
				m_pAni->SetBoneMatrix(n, mtx);
			}
			// неиспользуемые ядра уводим глубоко вниз - чтоб не маячили
			for( ; n<m_nBoneQuantity; n++ )
			{
				m_pAni->CollapseBone( n );
			}
			m_pAniModel->SpoilAnimationCache();
			m_pAniModel->Draw();
		}
	}
}

void CannonBallBatcher::InitShadowBuffers()
{
	IRender * render = (IRender *)api->GetService("DX9Render");
	Assert(render);

	m_vbShadow = render->CreateVertexBuffer(m_nMaxInstance*sizeof(ShadowVertex)*4, sizeof(ShadowVertex), __FILE__, __LINE__, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
	m_ibShadow = render->CreateIndexBuffer(m_nMaxInstance*sizeof(short)*6, __FILE__, __LINE__, USAGE_WRITEONLY | USAGE_DYNAMIC);

	unsigned short * ibData = (unsigned short *)m_ibShadow->Lock();
	if (ibData)
	{
		for (unsigned int i = 0; i < m_nMaxInstance; ++i)
		{
			ibData[i*6 + 0] = i*4+0;
			ibData[i*6 + 1] = i*4+1;
			ibData[i*6 + 2] = i*4+2;

			ibData[i*6 + 3] = i*4+0;
			ibData[i*6 + 4] = i*4+2;
			ibData[i*6 + 5] = i*4+3;
		}
		m_ibShadow->Unlock();
	}
}

void CannonBallBatcher::Update()
{
	const float ShadowSize = 1.0f;
	const float Level = 0.1f;
	const float HeightFactor = 15.0f;

	if ( m_Positions.Size() == 0 ) return;

	ShadowVertex * vbData = (ShadowVertex *)m_vbShadow->Lock(0,0,LOCK_DISCARD);
	if (vbData)
	{
		for (unsigned int i = 0; i < m_Positions.Size(); ++i)
		{
			float alpha = lerp( 1.0f, 0.2f, m_Positions[i].y/HeightFactor);
			
			vbData[i*4 + 0].pos = m_Positions[i].v + Vector(-1,0,-1)*ShadowSize;
			vbData[i*4 + 0].pos.y = Level;
			vbData[i*4 + 0].u = 0;
			vbData[i*4 + 0].v = 0;
			vbData[i*4 + 0].alpha = alpha;

			vbData[i*4 + 1].pos = m_Positions[i].v + Vector(-1,0,1)*ShadowSize;
			vbData[i*4 + 1].pos.y = Level;
			vbData[i*4 + 1].u = 0;
			vbData[i*4 + 1].v = 1;
			vbData[i*4 + 1].alpha = alpha;

			vbData[i*4 + 2].pos = m_Positions[i].v + Vector(1,0,1)*ShadowSize;
			vbData[i*4 + 2].pos.y = Level;
			vbData[i*4 + 2].u = 1;
			vbData[i*4 + 2].v = 1;
			vbData[i*4 + 2].alpha = alpha;

			vbData[i*4 + 3].pos = m_Positions[i].v + Vector(1,0,-1)*ShadowSize;
			vbData[i*4 + 3].pos.y = Level;
			vbData[i*4 + 3].u = 1;
			vbData[i*4 + 3].v = 0;
			vbData[i*4 + 3].alpha = alpha;
		}
		m_vbShadow->Unlock();
	}
}

void CannonBallBatcher::SetBallModel(IGMXScene* pModel)
{
	if( m_pAniModel == pModel ) return;
	ReleaseModel();
	m_pAniModel = pModel;
	if(!m_pAniModel) return;
	m_pAniModel->AddRef();

	// выскребаем скелет из модели

	boneDataReadOnly* boneData = NULL;
	boneMtxInputReadOnly* boneTransformations = NULL;
	dword dwBonesQ = pModel->GetBonesArray(&boneData, &boneTransformations);

	// создаем анимацию для ядра и запускаем ее
	if( dwBonesQ>0 )
	{
		// создаем скелет в формате необходимый для анимации:
		array<IAnimationScene::Bone> aBones(_FL_);
		aBones.AddElements( dwBonesQ );
		for( dword nb=0; nb<dwBonesQ; nb++ )
		{
			Matrix mtx = boneTransformations[nb].mtxBindPose;
			mtx.Inverse();
			aBones[nb].name = boneData[nb].name.c_str();
			aBones[nb].parentIndex = -1;
			aBones[nb].mtx = mtx;
		}
		m_nBoneQuantity = aBones.Size();
		m_pAni = m_pAniModel->AnimationScene()->CreateProcedural(aBones.GetBuffer(),m_nBoneQuantity, _FL_);
		m_pAniModel->SetAnimation( m_pAni );

		if( m_nBoneQuantity != m_nMaxInstance )
		{
			api->Trace("Warning! Cannon ball model (%s) have wrong bones quantity = %d. Max bones = %d", m_pAniModel->GetFileName(), m_nBoneQuantity, m_nMaxInstance );
		}
	}
	else
	{
		api->Trace("Error! Model hav`t master sceleton (%s)", m_pAniModel->GetFileName() );
	}
}

void CannonBallBatcher::ReleaseModel()
{
	if( m_pAniModel )
	{
		m_pAniModel->SetAnimation( NULL );
		m_pAniModel->Release();
		m_pAniModel = NULL;
	}
	if( m_pAni )
	{
		m_pAni->Release();
		m_pAni = NULL;
	}
}




CannonBallContainer::CannonBallContainer() :
	m_aBalls(_FL_)
{
}

CannonBallContainer::~CannonBallContainer()
{
	// если манагера трейлов уже нет, то сбросим у ядер все трейлы на ноль - чтоб не удалять невалидные указатели
	if( !m_trailmanager.Ptr() || !m_trailmanager.Validate() )
	{
		for( long n=0; n<m_aBalls; n++ )
			m_aBalls[n].SetTrailNull();
	}
	m_trailmanager.Reset();
}

void CannonBallContainer::Simulate(float deltaTime)
{
	for( long n=0; n<m_aBalls; n++ )
		if( m_aBalls[n].IsActive() )
			m_aBalls[n].Step( deltaTime );
}

bool CannonBallContainer::Create(MOPReader &reader)
{
	static const ConstString id_TrailManager("TrailManager");
	Mission().CreateObject( m_trailmanager, "TrailManager", id_TrailManager );

	Show(true);
	return true;
}

void CannonBallContainer::SetShadowTexture(const char* pcShadowTexture)
{
	for( long n=0; n<CannonBallBatcherMaxQuantity; n++ )
		m_Batch[n].batcher.SetShadowTexture(pcShadowTexture);
}

void CannonBallContainer::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&CannonBallContainer::Draw,ML_ALPHA4);
	else
		DelUpdate(&CannonBallContainer::Draw);
}

void _cdecl CannonBallContainer::Draw(float dltTime, long level)
{
	// расчет полета
	Simulate(dltTime);

	// отрисовка
	for( long n=0; n<CannonBallBatcherMaxQuantity; n++ )
	{
		// больше нет используемых батчеров
		if( m_Batch[n].modelname.IsEmpty() )
			break;

		// рисуем все ядра этого батчера
		for( long i=0; i<m_aBalls; i++ )
		{
			if( m_aBalls[i].IsActive() && m_aBalls[i].GetBatcherID()==n )
			{
				m_Batch[n].batcher.AddInstance( m_aBalls[i].GetPosition() );
			}
		}
		// доотрисуем все что осталось в этом батчере
		m_Batch[n].batcher.Draw();
		m_Batch[n].batcher.Flush();
	}
}

bool CannonBallContainer::AddBall(Weapon * parent, const char* waterHitSFX, long nModelBatcher, IGMXScene* pModel, const Vector& vFrom, const Vector& vTo, float fSpeed, float fMinAngle, float fMaxAngle, const WeaponTrailParams& trail, float fDamageMultiply, const char* pcFlySound)
{
	CannonBall & ball = GetFreeBall();

	// можем стрелять по данной траектории?
	if ( ball.ShootBall(vFrom, vTo, fSpeed, fMinAngle, fMaxAngle) )
	{
		SetBallParams(ball, parent, NULL, -1, nModelBatcher, pModel, trail, waterHitSFX, pcFlySound);
		ball.EnableCollision(true);
		return true;
	}

	return false;
}

void CannonBallContainer::AddBallWithoutCheck(Weapon * parent, const Vector& src, const Vector& dst, float fSpeed, TargetPoints* pTargPoints, long pointIdx, long nModelBatcher, IGMXScene* pModel, const WeaponTrailParams& trail, const char* waterHitSFX, const char* pcFlySound)
{
	CannonBall & ball = GetFreeBall();

	// можем стрелять по данной траектории?
	float fMinTime = 2.5f;
	float fMinHeight = 20.f;
	if( pTargPoints )
		pTargPoints->GetTimeHeightLimit(fMinTime,fMinHeight);

	ball.SetTrajectory(src, dst, fMinTime, fMinHeight);
	if( ball.IsActive() )
	{
		SetBallParams(ball, parent, pTargPoints, pointIdx, nModelBatcher, pModel, trail, waterHitSFX, pcFlySound);
		ball.EnableCollision(false);
	}
}

void CannonBallContainer::DeleteByWeapon(Weapon * parent)
{
	for( int i = 0; i < m_aBalls; i++ )
		if( m_aBalls[i].IsActive() && m_aBalls[i].GetParent()==parent )
		{
			m_aBalls[i].InActivate();
		}
}

void CannonBallContainer::SetBallParams(CannonBall& ball, Weapon * parent, TargetPoints* pTargPoints, long pointIdx, long nModelBatcher, IGMXScene* pModel, const WeaponTrailParams& trail, const char* waterHitSFX, const char* pcFlySound)
{
	// установим параметры для выстрела
	ball.SetParent( parent );
	ball.SetWaterHitSFX( waterHitSFX );
	ball.SetModel( pModel );
	ball.SetDamage( parent ? parent->GetDamage() : 1.f );
	ball.SetTrailParams( (ITrailManager*)m_trailmanager.Ptr(),
							trail.fTraceInitSize,
							trail.fTraceFinalSize,
							trail.fTraceMinFadeoutTime,
							trail.fTraceMaxFadeoutTime,
							trail.cTraceColor,
							trail.fTraceOffsetStrength );
	ball.SetFlySound( pcFlySound );
	ball.SetTargetPoint(pTargPoints, pointIdx);
	if( pTargPoints )
		pTargPoints->BorrowPoint( pointIdx, ball.GetPrecalculateFlyTime() );

	ball.SetBatcherID( (pModel && nModelBatcher<CannonBallBatcherMaxQuantity) ? nModelBatcher : -1 );
}

long CannonBallContainer::RegistryCannonballBatcher(const char* pcModelName, IGMXScene* pModel)
{
	for( long n=0; n<CannonBallBatcherMaxQuantity; n++ )
	{
		// уже есть такой - используем его
		if( m_Batch[n].modelname == pcModelName )
			return n;
		// нет такого - создаем новый с этой моделью
		if( m_Batch[n].modelname.IsEmpty() )
		{
			m_Batch[n].modelname = pcModelName;
			m_Batch[n].batcher.SetBallModel( pModel );
			return n;
		}
	}
	api->Trace("Warning! CannonBall : so mach ball models (more then %d). Can`t create new ball batcher!", CannonBallBatcherMaxQuantity);
	return -1;
}

CannonBall & CannonBallContainer::GetFreeBall()
{
	//найдем свободное ядро для полета
	long i;
	for( i = 0; i < m_aBalls; i++ )
		if( !m_aBalls[i].IsActive() )
			break;

	// нет свободных ядер - заведем новое
	if (i == m_aBalls.Size())
		i = m_aBalls.Add();

	m_aBalls[i].SetActive(false);

	return m_aBalls[i];
}

MOP_BEGINLIST(CannonBallContainer, "", '1.00', 100)
MOP_ENDLIST(CannonBallContainer)
