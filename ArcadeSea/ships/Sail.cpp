#include "..\pch.h"
#include ".\sail.h"
#include "Ship.h"
#include "..\..\Common_h\Mission.h"
#include "PartsClassificator.h"
#include "..\asspecial\ClothBurns.h"
#include "..\SeaMissionParams.h"

// максимальное количество хитов (прострелов в парусе) после которого его парусность считается нулевой
const unsigned int MaxHits = 2;
// время в течении которого влияние ветра на падающий парус считается в специальном режиме
static const float SailFallTime = 2.f;
// максимальный коеффициент крепкости паруса (работает пока парус мало поврежден - что бы не порвался случайно при пробуждении)
static const float fHighTearFactor = 100.f;
// время в секундах которое держится высокий коеффициент крепкости паруса
static const float fMaxHighTearTime = 1.f;
// время через которое на парус прикладываются выталкивающие силы - что бы не зацеплялся вечно в корпусе корабля
static const float fClothPushTimeMax = 3.f;

Sail::Sail(const Sail::Params& other, const char* name, SailOwner* owner) :
	m_aDeferredActions(_FL_),
	params_(other)
{
	m_pSailOwner = owner;
	name_ = name;

	builder_ = NULL;
	simData_ = NULL;
	cloth_ = NULL;
	doNotRenderPhysicalCounter_ = 0;
	m_bUseHighTearFactor = true;
	m_fHighTearTime = fMaxHighTearTime;
	m_fClothPushTime = 0.f;
	m_bSleep = false;

	m_eWindMode = Sail::WindMode_Enabled;
	hits_ = 0;
	isFake_ = true;
	m_bNeverPhysics = false;
	m_bIsDestroyed = false;
	m_bNoShow = false;

	m_fFadeInTime = 0.f;
	m_fFadeInTimeMax = 1.f;

	m_bUpdateBuffers = true;
}

Sail::Sail(const Sail& other) :
	m_aDeferredActions(_FL_),
	params_(other.params_)
{
	m_pSailOwner = other.m_pSailOwner;
	name_ = other.name_;

	builder_ = other.builder_;
	simData_ = NULL;
	cloth_ = NULL;
	doNotRenderPhysicalCounter_ = other.doNotRenderPhysicalCounter_;
	m_bUseHighTearFactor = other.m_bUseHighTearFactor;
	m_fHighTearTime = other.m_fHighTearTime;
	m_fClothPushTime = other.m_fClothPushTime;
	m_bSleep = false;

	m_eWindMode = other.m_eWindMode;
	hits_ = other.hits_;
	isFake_ = other.isFake_;
	m_bNeverPhysics = other.m_bNeverPhysics;
	m_bIsDestroyed = other.m_bIsDestroyed;
	m_bNoShow = other.m_bNoShow;

	m_fFadeInTime = 0.f;
	m_fFadeInTimeMax = 1.f;
}

Sail::~Sail(void)
{
	ExcludeSailFromBurns();
	ExcludeSailFromLimits();

	RELEASE( simData_ );

	//RELEASE(cloth_);
	// удаляем паруса через глобальный объект (для того что бы выполнить последовательное удаление)
	if(cloth_)
	{
		SeaMissionParams* pSMParams = GetSeaMParams();
		if( pSMParams )
			pSMParams->ClothToDelete(cloth_);
		else
		{
			cloth_->Release();
			api->Trace("Sail perfomance warning: cloth do forced release, because not finded SeaMissionParams.");
		}
		cloth_ = NULL;
	}
}

// Привязать парус к рее
void Sail::TieSailToYards()
{
	if( !cloth_ ) return;
	if( !GetSailOwner()->GetPhysView() ) return;

	// tie top line to yard
	if( m_TieData.pTopYard && !m_TieData.pTopYard->IsBroken() && m_TieData.pTopYard->GetShapes().Size()>0 )
	{
		TieSailToPhysShape(true, *GetSailOwner()->GetPhysView(), m_TieData.pTopYard->GetShapes()[0], m_TieData.points[0], m_TieData.points[1]);
		m_TieData.bTied = true;
	}

	// tie bottom corners to yard
	if( m_TieData.pBottomYard && !m_TieData.pBottomYard->IsBroken() && m_TieData.pBottomYard->GetShapes()>0 )
	{
		TieSailToPhysShape(false, *GetSailOwner()->GetPhysView(), m_TieData.pBottomYard->GetShapes()[0], m_TieData.points[2], m_TieData.points[3]);
		m_TieData.bBottomTied = true;
	}
}

void Sail::Rebuild()
{
	ExcludeSailFromBurns();
	ExcludeSailFromLimits();

	// парус не спит, почти не рвется сам и время до ослабления прочности паруса максимальное
	m_bSleep = false;
	m_bUseHighTearFactor = true;
	m_fHighTearTime = fMaxHighTearTime;

	// Парус не физический, режим ветра включен (на раздувание паруса), парус не уничтожен и рисуется
	isFake_ = true;
	m_eWindMode = WindMode_Enabled;
	m_bIsDestroyed = false;
	m_bNoShow = false;

	// если физ парус еще есть, то он пересоздается
	if (cloth_)
		cloth_->Recreate();
	// если физ паруса нет, то делаем его если позволено
	else if( !m_bNeverPhysics )
		cloth_ = GetSailOwner()->CreateCloth(_FL_, sailRender_.GetRenderInfo(), *builder_, *simData_);

	// ставим параметры физ паруса:
	if (cloth_)
	{
		// очень прочный
		cloth_->SetTearCoef(fHighTearFactor);
		// держится за реи
		cloth_->SetAttachmentTearCoef(params_.tearFactor);
		// спит (пока в режиме фейкового паруса)
		SleepCloth(Matrix());
	}
}

void Sail::FadeIn(float fFadeTime)
{
	if( fFadeTime > 0.f )
	{
		m_fFadeInTime = fFadeTime;
		m_fFadeInTimeMax = fFadeTime;
	}
	else
		m_bNoShow = true;
}


void Sail::InitSimData()
{
	Assert(builder_);

	// инициализируем текстурные координаты
	const IClothMeshBuilder::Vertex* vb = NULL;
	const unsigned short* ib = NULL;
	unsigned int vCount = 0;
	unsigned int iCount = 0;
	builder_->GetCombinedBuffers(vb, ib, vCount, iCount);

	RELEASE( simData_ );
	simData_ = NEW IPhysCloth::SimulationData;
	Assert( simData_ );

	// буферы удвоенные, чтобы во время разрыва связей количество как вертексов, так и индексов могло
	// увеличиваться безболезненно (если разрывать все связи, то их все равно не хватит, и очередной разрыв не состоится.
	// хотя будет варнинг в физическом движке, но ситуация штатная)
	simData_->maxIndexCount = iCount*2;
	simData_->maxVertexCount = vCount*2;
	simData_->indexStride = sailRender_.GetRenderInfo().GetSingleIndexSize();
	simData_->posStride = sailRender_.GetRenderInfo().GetSingleVertexSize();

	simData_->vertexCount = 0;
	simData_->indexCount = 0;
	simData_->parentIndexCount = 0;

	simData_->posBuffer = NEW char[sailRender_.GetRenderInfo().GetSingleVertexSize()*simData_->maxVertexCount];
	simData_->ib = NEW char[sailRender_.GetRenderInfo().GetSingleIndexSize()*simData_->maxIndexCount];
	simData_->parentIB = NEW unsigned int[simData_->maxVertexCount];

#ifdef _DEBUG
	memset (simData_->posBuffer, 0, sailRender_.GetRenderInfo().GetSingleVertexSize()*simData_->maxVertexCount);
	memset (simData_->ib, 0, sailRender_.GetRenderInfo().GetSingleIndexSize()*simData_->maxIndexCount);
	memset (simData_->parentIB, 0, simData_->maxVertexCount * sizeof(unsigned int));
#endif

	char* texCoords = (char*)simData_->posBuffer + sailRender_.GetRenderInfo().GetTexCoordOffset();
	for (unsigned int i = 0; i < vCount; ++i)
	{
		((float*)texCoords)[0] = vb[i].tu;
		((float*)texCoords)[1] = vb[i].tv;

		texCoords += sailRender_.GetRenderInfo().GetSingleVertexSize();
	}
}

void Sail::Make(bool isFake, IClothMeshBuilder* builder, bool neverPhysics)
{
	// можем поменять флаг фейка (в редакторе или ограничение по количеству)
	if( GetSailOwner()->EditMode_IsOn() )
		isFake = true;
	if( neverPhysics )
		isFake = true;
	if( !isFake && !MissionSailsSetFake(isFake) )
		isFake = true;
	isFake_ = isFake;

	m_bNeverPhysics = neverPhysics;

	//Assert(builder);
	if( !builder ) return;
	builder_ = builder;

	Matrix parent(true);
	GetSailOwner()->GetMatrix(parent);

	Vector minPoint(1e38f, 1e38f, 1e38f);
	Vector maxPoint(-minPoint);
	fakeABB_.center = 0.f;

	Matrix mtx(params_.orient, params_.pos);

	for (unsigned int i = 0; i < params_.pointsCount; ++i)
	{
		Vector vcurp = params_.points[i] * mtx;
		minPoint.Min(vcurp);
		maxPoint.Max(vcurp);
		fakeABB_.center += vcurp;
	}
	fakeABB_.center /= (float)params_.pointsCount;
	fakeABB_.size = maxPoint-minPoint;
	const float minSize = params_.fakeAnimAmplitude;
	if (fakeABB_.size.x < minSize) fakeABB_.size.x = minSize;
	if (fakeABB_.size.y < minSize) fakeABB_.size.y = minSize;
	if (fakeABB_.size.z < minSize) fakeABB_.size.z = minSize;

	InitSimData();

	if (!neverPhysics && !GetSailOwner()->EditMode_IsOn())
	{
		cloth_ = GetSailOwner()->CreateCloth(_FL_, sailRender_.GetRenderInfo(), *builder_, *simData_);
		Assert(cloth_);
		m_bSleep = false;

		m_bUseHighTearFactor = true;
		m_fHighTearTime = fMaxHighTearTime;
		cloth_->SetTearCoef(fHighTearFactor);
		cloth_->SetAttachmentTearCoef(params_.tearFactor);

		cloth_->Transform(parent);

		cloth_->SetSolverIterations( 2 );

		FindTiedYard();

		if ( isFake )
			SleepCloth( parent );
		else
			TieSailToYards();
	}

	sailRender_.SetTexture(params_.texture);
	sailRender_.SetGlimpseTexture(params_.glimpseTexture);
	sailRender_.SetGlimpseColor( params_.glimpseColor.v4.v );
	sailRender_.SetColor( params_.color.v4.v, params_.colorScale );
	sailRender_.CreateBuffers(simData_->maxVertexCount, simData_->maxIndexCount);

	fakeRender_.SetTexture(params_.texture);
	fakeRender_.SetGlimpseTexture(params_.glimpseTexture);
	fakeRender_.SetGlimpseColor( params_.glimpseColor.v4.v );
	fakeRender_.SetColor( params_.color.v4.v, params_.colorScale );
	fakeRender_.SetFreq(params_.xFreq, params_.yFreq);
	fakeRender_.SetFakeAnimAmplitude(params_.fakeAnimAmplitude);
	fakeRender_.SetSailParams(params_.widthPointsCount, params_.heightPointsCount );
	if( params_.pointsCount==3 )
		fakeRender_.SetTriangleSailParams(params_.points);
	else
		fakeRender_.SetSquareSailParams(params_.points,params_.windCurvature);
	fakeRender_.InitBuffers(*builder_);
}


// переключить режим паруса
bool Sail::SetFake(bool isFake)
{
	if ( isFake_ == isFake )
		return true;

	if( !isFake && !cloth_ )
		return true;

	if( !MissionSailsSetFake(isFake) )
		return true;

	Matrix mtxCurTransform(true);
	if( m_TieData.pTopYard && !m_TieData.pTopYard->IsBroken() )
	{
		m_TieData.pTopYard->GetWorldTransform(mtxCurTransform);
		mtxCurTransform = m_TieData.mtxToSailTransf * mtxCurTransform;
	}
	else
	{
		GetSailOwner()->GetMatrix(mtxCurTransform);
	}

	isFake_ = isFake;
	if (!isFake)
	{
		doNotRenderPhysicalCounter_ = 4; // still render fake during next 4 frames
		if( cloth_ )
		{
			WakeupCloth(mtxCurTransform);
		}
	}
	else
	{
		if( cloth_ )
			SleepCloth(mtxCurTransform);
	}

	return false;
}


//! проверка видимости сферы в frustum'e камеры
bool SphereIsVisible(const Plane *frustum, const Vector& SphereCenter, float SphereRadius)
{
	for(long p = 0; p < 5; p++)
		if(((SphereCenter | frustum[p].n) - frustum[p].d) < -SphereRadius) return false;

	return true;
}

// проверка нахождения бокса под водой
bool IsUnderLevel(const Box& box, float level)
{
	if ( box.center.y + box.size.GetLength()*0.5f < level ) 
		return true;
	return false;
}

// парус в области видимости камеры?
bool Sail::InFrustum(IRender& render) const
{
	Plane	*p = NULL;
	Box		bb;

	if (isFake_ || !cloth_)
		bb = fakeABB_;
	else
		bb = cloth_->GetABB();

	for (unsigned int i = 0; i < render.GetNumFrustumPlanes(); ++i)
		if ( SphereIsVisible(render.GetFrustum()+i, bb.c, bb.size.GetLength()*0.5f) )
			return true;

	return false;
}

// отрисовать парус
void Sail::Draw(const Matrix& parent)
{
	if( m_bNoShow )
		return;

	// показать дебажную инфу
	if ( api->DebugKeyState(VK_SHIFT, VK_CONTROL, 'Z') )
	{
		Vector vpos;
		if (isFake_ || !cloth_) {
			vpos = parent.MulVertex( fakeABB_.center );
		} else {
			vpos = cloth_->GetABB().center;
		}
		if( GetMOOwner() )
		{
			GetMOOwner()->Render().Print( vpos, 0.f, 0.f, 0xFFFFFFFF, "sail: %s", name_.c_str() );
			GetMOOwner()->Render().Print( vpos, 0.f, 1.f, 0xFFFFFFFF, "top %s %s%s",
				(m_TieData.bTied ? "tied" : "no tie"),
				(m_TieData.pTopYard ? m_TieData.pTopYard->GetDebugName() : "no_yard"),
				((m_TieData.pTopYard && m_TieData.pTopYard->IsBroken()) ? " (broken)":" ") );
			GetMOOwner()->Render().Print( vpos, 0.f, 2.f, 0xFFFFFFFF, "bottom %s %s%s",
				(m_TieData.bBottomTied ? "tied" : "no tie"),
				(m_TieData.pBottomYard ? m_TieData.pBottomYard->GetDebugName() : "no_yard"),
				((m_TieData.pBottomYard && m_TieData.pBottomYard->IsBroken()) ? " (broken)":" ") );
		}

		if( cloth_ )
		{
			Box bb = cloth_->GetABB();
			GetMOOwner()->Render().DrawBox( bb.c-bb.s*0.5f, bb.c+bb.s*0.5f, Matrix(), 0xFF00FFFF );
		}
		GetMOOwner()->Render().DrawBox( fakeABB_.pos-fakeABB_.size*0.5f, fakeABB_.pos+fakeABB_.size*0.5f, shipMtx_, 0xFF0000FF );
	}

	if (!isFake_ && doNotRenderPhysicalCounter_ == 0)
	{
		sailRender_.SetAlpha(GetSailOwner()->GetAlpha());
		if( m_bUpdateBuffers )
		{
			sailRender_.UpdateBuffers(*simData_);
			m_bUpdateBuffers = false;
		}
		sailRender_.Render(*simData_);
	}
	else
	{
		float fCurAlpha = GetSailOwner()->GetAlpha();
		if( m_fFadeInTime > 0.f )
		{
			m_fFadeInTime -= api->GetDeltaTime();
			fCurAlpha *= m_fFadeInTimeMax>0.f ? m_fFadeInTime / m_fFadeInTimeMax : 0.f;
			if( m_fFadeInTime <= 0.f )
				m_bNoShow = true;
		}
		fakeRender_.SetAlpha(fCurAlpha);
		if( m_TieData.pTopYard )
		{
			fakeRender_.Render( m_TieData.mtxYardTransf );
		}
		else
			fakeRender_.Render( shipMtx_ );
	}
}

void Sail::DrawPartRefl(const Matrix& parent)
{
	if( m_bNoShow )
		return;

	if( hits_ < MaxHits )
		fakeRender_.Render( parent );
}

void Sail::DoWork(const Matrix& parent)
{
	long n;
	shipMtx_ = parent;
	float fDeltaTime = GetSailOwner()->GetLastDeltaTime();

	// проверяем отложенные действия
	for( n=0; n<m_aDeferredActions; n++ )
	{
		m_aDeferredActions[n].fTime -= fDeltaTime;
		// время наступило - надо делать действие
		if( m_aDeferredActions[n].fTime <= 0.f )
		{
			// выполняем
			switch( m_aDeferredActions[n].act )
			{
			case DeferredAction::da_hDetachSail :
				Detach();
				break;
			}
			// удаляем
			m_aDeferredActions.DelIndex(n);
			n--;
		}
	}

	// не надо показывать, значит нечего и считать
	if( m_bNoShow )
		return;

	// корректируем матрицу паруса... если часть отвалившаяся, то матрица в входном параметре неверная
	// (ибо она для целой части).
	// В основном это надо для фейковых парусов, но для физики тоже понадобиться, когда мы делаем sleep
	// для паруса, ибо потом при восстановлении его надо будет переместить на новую позицию, так как сама физика
	// его чаще всего просто убъет... верлет порвет все связи из за больших дистанций и скоростей
	if( m_TieData.pTopYard )
	{
		Matrix mYard;
		m_TieData.pTopYard->GetWorldTransform(mYard);
		m_TieData.mtxYardTransf = m_TieData.mtxToSailTransf * mYard;
		shipMtx_ = m_TieData.mtxYardTransf;
	}

	// физический парус развалившийся более чем на половину отделяем от низа.
	if( !isFake_ && GetTearedState() > 0.5f && m_TieData.bBottomTied )
	{
		DetachBottom();
		// и запускаем отложенно разрезание паруса по горизонтальной оси возле вершины:
		// что бы не болтался некрасивый длинный хомут лохмотьев
		n = m_aDeferredActions.Add();
		m_aDeferredActions[n].act = DeferredAction::da_hCutSail;	// отрезание
		m_aDeferredActions[n].fTime = 4.f;							// через 4 секунды
		m_aDeferredActions[n].param.dwVal = 3;						// вдоль линии №3 (четвертая линия партиклов паруса)
	}

	// счетчик кадров задержки на переключение рендера физических парусов
	// (физика подготовит правильные значения для буферов, только через некоторое время после включения)
	if ( doNotRenderPhysicalCounter_ > 0 )
		doNotRenderPhysicalCounter_--;

	// действия только для физических парусов
	if (!isFake_)
	{
		// проверим возможность снижения "крепкости" паруса
		// (сильно порванный парус делаем менее крепким, что бы он сам расслоился со временем)
		if( m_bUseHighTearFactor )
		{
			m_fHighTearTime -= fDeltaTime;
			if( m_fHighTearTime <= 0.f || GetTearedState() > 0.3f )
			{
				cloth_->SetTearCoef(params_.tearFactor);
				m_bUseHighTearFactor = false;
			}
		}

		Box bb = cloth_->GetABB();
		// если все части паруса под водой ниже 5 метров, то гасим парус
		if( bb.c.y + bb.s.y*0.5f < -5.f )
		{
			m_bNoShow = true;
			m_bIsDestroyed = true;
			m_bSleep = true;
			cloth_->Sleep();
			ExcludeSailFromBurns();
			ExcludeSailFromLimits();
			return;
		}

		if( m_bIsDestroyed )
			DestroyFrame();

		// поставить бокс за пределами которого парус уничтожается
		// не ищем реальный бокс вокруг корабля. Ибо уничтожать парус сразу за его пределами не есть гуд -
		// пусть немного полетает рядом и уйдет под воду (берем 100 на 100 и глубину 50)
		Vector vMin, vMax;
		Box::FindABBforOBB(shipMtx_, Vector(-200.f,-30.f,-200.f), Vector(200.f,200.f,200.f), vMin, vMax);
		cloth_->SetValidBox(vMin, vMax);
		cloth_->EnableValidBox(true);

		// проверка паруса на горение
		bool bIsBurned = false;
		IClothBurns * pCBurns = GetSailOwner()->GetClothBurns();
		if( pCBurns )
			bIsBurned = pCBurns->ProcessCloth(GetMOOwner(), cloth_, simData_);

		// если парус загорелся, то считаем его максимально поврежденным
		if( bIsBurned )
		{
			GetSailOwner()->PartDestroyFlagSet( Ship::pdf_fire_sail );
			hits_ = MaxHits;
		}

		// колбасня от ветра
		if ( fDeltaTime>0.f && (m_eWindMode==WindMode_Enabled || m_eWindMode==WindMode_FallBegin) )
		{ // обычная колбасня (парус висит на рее)
			// Стандартная сила - рандомная, направлена вперед и в сторону
			Vector v(10.0f*(rand()/(float)RAND_MAX-0.5f),0,10.f*(rand()/(float)RAND_MAX+1.f));
			// Расчет силы для выталкивания частей паруса (раз в некий промежуток времени)
			// нужна для того чтоб части паруса не застревали надолго в корпусе
			m_fClothPushTime += fDeltaTime;
			if( m_fClothPushTime > fClothPushTimeMax )
			{
				m_fClothPushTime = 0.f;
				v.y += 100.f;
				v.x = (rand()&0x1) ? -100.f : 100.f;
			}
			v = parent.MulNormal(v);
			// для нестандартного режима сила ветра уменьшена
			if( m_eWindMode!=WindMode_Enabled )
				v *= 0.1f;
			cloth_->SetAcceleration(v);

			/*
			--------------------------------------------------------
			 Вариант, когда сила прикладывается не одинаково на весь парус,
			 а сферой на область паруса. что позволяет парусу не лететь при падении "фанерой",
			 и кроме того, горящие части паруса не улетают почти мгновенно, что намного улучшает картинку
			--------------------------------------------------------*/
			float fSailHeight = (params_.points[2] - params_.points[0]).GetLength();

			// собственно воздействие на парус
			cloth_->AddForceAtPos( shipMtx_.MulVertex((params_.points[0]+params_.points[1]) * 0.5f + Vector(0.f,-fSailHeight*0.3f,-fSailHeight*0.5f)),
				500.f+FRAND(200.f),
				fSailHeight,
				pfm_acceleration );
			// ограничивающее воздействие - чтоб болтающийся парус не перекидывался через верхнюю часть назад.
			cloth_->AddForceAtPos( shipMtx_.MulVertex((params_.points[0]+params_.points[1]) * 0.5f + Vector(0.f,fSailHeight*0.25f,-fSailHeight*0.1f)),
				1000.f,
				fSailHeight*0.5f,
				pfm_acceleration );

			// расчет специального режима - отделение и падение паруса при принудительном уничтожении
			if ( m_eWindMode==WindMode_FallBegin )
			{
				m_fFallTime -= fDeltaTime;
				if( m_fFallTime <= 0.f )
				{
					m_eWindMode = WindMode_FallEnding;
				}
			}
		}
		else
		{
			cloth_->SetAcceleration(Vector(0.f));
		}
	}
	// фейковый парус расчетов не требует... пусть колбасится себе на здоровье.
	else
	{
		fakeRender_.AddAnimTime( fDeltaTime );

		// если парус хочет гореть, то делаем его не фейковым - на следущем кадре загорит
		if( !GetSailOwner()->EditMode_IsOn() ) // в режиме редактора не горим
		{
			IClothBurns * pCBurns = GetSailOwner()->GetClothBurns();
			if( pCBurns && pCBurns->CheckBurnIntoBox(cloth_, Matrix().BuildPosition(fakeABB_.pos)*shipMtx_, fakeABB_.size*0.5f) )
				SetFake(false);
		}
	}
}

// начать уничтожение паруса (вызывается при разваливании корабля)
void Sail::BeginDestruction(float fDetachTime)
{
	SetFake(false);
	if (cloth_)
	{
		if( m_TieData.pTopYard && m_TieData.pTopYard->GetPhysActor() )
		{
			TieSailToPhysShape(true, *m_TieData.pTopYard->GetPhysActor(), 0, m_TieData.points[0], m_TieData.points[1]);

			// через время отвязать парус от всего
			long n = m_aDeferredActions.Add();
			m_aDeferredActions[n].act = DeferredAction::da_hDetachSail;	// отвязывание
			m_aDeferredActions[n].fTime = fDetachTime;					// через 1 секунду
		}
	}
}

// отсоединить парус от рей
void Sail::Detach()
{
	if( cloth_ )
	{
		cloth_->Detach();
		m_TieData.bTied = false;
		m_TieData.bBottomTied = false;
	}
	//InitSailFall();
}

// отсоединить парус от нижней реи
void Sail::DetachBottom()
{
	if( !cloth_ )
		return;

	long idx = params_.pointsCount == 3 ?
		((params_.widthPointsCount+1) * params_.heightPointsCount / 2 - params_.widthPointsCount) :
		(params_.widthPointsCount * params_.heightPointsCount - params_.widthPointsCount);
	// отвязать правый нижний угол паруса
	cloth_->Detach( idx );
	// отвязать левый нижний угол паруса
	if( params_.widthPointsCount > 1 )
		cloth_->Detach( idx + params_.widthPointsCount - 1 );

	m_TieData.bBottomTied = false;
}

// присоединить парус к физ телу (вызывается при разрушении корабля)
void Sail::Attach(IPhysRigidBody& actor)
{
	if( cloth_ )
		cloth_->Attach(actor, false, true);
}

// здоровье паруса (влияет на парусность - скорость корабля)
float Sail::GetHealth()
{
	if ( isFake_ )
		return 1.0f;

	return Max(0.0f, 1.0f - hits_/(float)MaxHits);
}

// атакуем парус
bool Sail::Attack(const Vector& v1, const Vector& v2, bool isTrace)
{
	bool bHit = false;
	unsigned int vID = 0;

	// физический парус рейкастит физика
	if (!isFake_)
	{
		bHit = cloth_->Raycast(v1, v2, NULL, &vID);
	}
	// фейковый парус трейсим сами
	else
	{
		// если линия атаки пересекает бокс паруса
		Matrix mtxBox = shipMtx_;
		mtxBox.pos = fakeABB_.pos * shipMtx_;
		if ( Box::OverlapsBoxLine( mtxBox, fakeABB_.size*0.5f, v1, v2) )
		{
			// то выполняем фейковый (неточный) трейс
			if( fakeRender_.FakeRaycast( shipMtx_, v1, v2, &vID ) )
			{
				// засчитываем попадание если мы можем сделать парус физическим.
				// но в режиме трейса, мы не пытаемся делать парус физическим
				bHit = isTrace ? true : !SetFake(false);
			}
		}
	}

	// есть попадание?
	if ( bHit && !isTrace && vID<simData_->vertexCount )
	{
		if( cloth_ )
		{
			// проверим парус на максимальное количество разрывов - что бы не рвать больше
			if( simData_->vertexCount < simData_->maxVertexCount - 1 &&
				simData_->indexCount < simData_->maxIndexCount - 3 )
			{
				// рвем в точке попадания
				cloth_->Tear(vID, true);
				// и в соседней точке, что бы получилась дырка, а не невидимый разрыв
				if (vID > 0)
					cloth_->Tear(vID-1, true);
			}
		}

		// звук рвущегося паруса
		if( GetSailOwner() && GetMOOwner() )
		{
			const char* pcSoundName = GetSailOwner()->GetSoundNameForEvent( Ship::pdf_destroy_sail );
			if( pcSoundName )
				GetMOOwner()->Sound().Create3D( pcSoundName, v2, _FL_ );
		}

		hits_++;
	}
	return bHit;
}

// атакуем парус сферой
bool Sail::Attack(const Vector& vCenter, float fRadius)
{
	bool bHit = false;

	Matrix mtxBox = shipMtx_;
	mtxBox.pos = fakeABB_.pos * shipMtx_;
	if( Box::OverlapsBoxSphere( mtxBox, fakeABB_.size*0.5f, vCenter,fRadius ) )
		bHit = isFake_ ? (!SetFake(false)) : true;

	if( bHit )
	{
		int q = 1 + (rand()%3);
		for( int n=0; n<q; n++ )
		{
			unsigned int vID = rand() % simData_->vertexCount;
			// рвем в точке попадания
			cloth_->Tear(vID, true);
			// и в соседней точке, что бы получилась дырка, а не невидимый разрыв
			if (vID > 0)
				cloth_->Tear(vID-1, true);
		}
	}

	return bHit;
}

// при разрушении корабля, меняем параметры паруса для улучшения визуализации
void Sail::ActivateBreakParams()
{
	if (cloth_)
	{
		m_bUseHighTearFactor = false;
		cloth_->SetAttachmentTearCoef(2.0f);
		cloth_->SetTearCoef(2.5f);
		cloth_->SetSolverIterations(1);
		m_eWindMode = WindMode_Disabled;
	}
}

// режим включения/выключения паруса (активация/деактивация корабля)
void Sail::ShipPart_EnablePhysicActor(bool bEnable)
{
	if( cloth_ && GetSailOwner() )
	{
		// физический парус переводим в сон или пробуждаем, чтобы за время выключения актера(корабля) парус никуда
		// не потерялся (т.к. привязки к рее в этот момент не происходит и парус находится в свободном падении)
		if( !isFake_ )
		{
			// матрица засыпания/просыпания
			Matrix m(true);
			if( m_TieData.pTopYard )
			{
				m_TieData.pTopYard->GetWorldTransform(m);
				m = m_TieData.mtxToSailTransf * m;
			}
			else
			{
				GetSailOwner()->GetMatrix(m);
			}

			// собстно включение/выключение паруса
			SeaMissionParams* pSMParams = GetSeaMParams();
			if( bEnable )
			{
				//WakeupCloth( m ); не могем юзать, чтобы не привязать частично отсоединившийся парус к рее вновь
				// разбудим парус
				if( m_bSleep && !m_bNoShow )
				{
					m_bSleep = false;
					cloth_->WakeUp( m_mtxSleepTransform.Inverse() * m );
					// восстановим парус в списке ограничений
					if( pSMParams ) pSMParams->SailLimit_AddSail(this);
				}
			}
			else
			{
				//SleepCloth( m ); не могем юзать, что бы не отвязать парус от реи (мы не знаем что потом привязывать обратно)
				// уберем парус из списка ограничений
				if( pSMParams ) pSMParams->SailLimit_DelSail(this);
				// усыпим парус
				if( !m_bSleep )
				{
					m_bSleep = true;
					m_mtxSleepTransform = m;
					cloth_->Sleep();
				}
			}
		}
	}

	ShipPart::ShipPart_EnablePhysicActor(bEnable);
}

// попытка включения физ. паруса с миссионными ограничениями
bool Sail::MissionSailsSetFake(bool bFake)
{
	if( bFake == isFake_ )
		return true;

	SeaMissionParams* pSMParams = GetSeaMParams();
	if( pSMParams )
		return pSMParams->SailLimit_SetFake(bFake, this);
	return true;
}

// запускаем уничтожение паруса.
// Вызывается из миссионного манагера парусов, когда превышен лимит физических парусов.
// Это необходимо что бы освободить место для новых физ.парусов.
// Вызов данного метода происходит у самого рваного паруса.
void Sail::LaunchDestroy()
{
	BeginDestruction(0.1f);
	Detach();
	InitSailFall();
	m_bIsDestroyed = true;
	hits_ = MaxHits;
}

// кадр уничтожения паруса
void Sail::DestroyFrame()
{
	// все убираем нафиг...
	// должно было быть приложение просчитанных сил к нужным точкам, что бы красивее разлетался парус...
	// и постоянное разрывание паруса, что бы он даже если зацепился бы за что нить, то быстро освободился

	// Важно было быстро убить парус, так как только вышедший за пределы валидного бокса, парус исключается из
	// списка физ.парусов и дает возможность включать новые физ.паруса (если работет ограничение на количество)

	// Но все это слишком сильно тормозило, да и постоянное разрывание паруса быстро приводило к лимиту
	// (запас на буферы вертексов установлен в 2 раза, а полное раздублирование требует ~6 кратного буфера)
}

// состояние порванности паруса
float Sail::GetTearedState()
{
	// фейковый всегда целый
	if( isFake_ )
		return 0.f;

	// отношение числа вертексов к базовому числу вертексов
	float fK = params_.pointsCount == 3 ?
		((float)simData_->vertexCount / (float)((params_.widthPointsCount+1) * params_.heightPointsCount / 2)) :
		((float)simData_->vertexCount / (float)(params_.widthPointsCount * params_.heightPointsCount));
	// оставляем только относительное количество дубликатов вертексов (разрывов)
	fK = Clampf(fK-1.f);
	return fK;
}

void Sail::ExcludeSailFromLimits()
{
	SeaMissionParams* pSMParams = GetSeaMParams();
	if( pSMParams )
		pSMParams->SailLimit_DelSail( this );
}

void Sail::ExcludeSailFromBurns()
{
	if( !cloth_ ) return;
	IClothBurns * pCBurns = GetSailOwner()->GetClothBurns();
	if( pCBurns )
		pCBurns->ExcludeObjects( cloth_ );
}

void Sail::InitSailFall()
{
	m_eWindMode = WindMode_FallBegin;
	m_fFallTime = SailFallTime;
}

void Sail::FindTiedYard()
{
	if( GetSailOwner() && GetSailOwner()->GetPhysView() )
	{
		ShipPart* pTop = 0;
		ShipPart* pBottom = 0;

		if( !GetShipOwner() ) return;

		PartsClassificator & cls = GetShipOwner()->GetPartClassificator();
		GetSailOwner()->AcceptVisitor(cls);

		// Ищем привязанные реи
		for(int i=0; i<cls.GetYards(); i++)
		{
			if( cls.GetYards()[i]->IsBroken() )
				continue;
			Matrix mtx = cls.GetYards()[i]->GetInitialPoses()[0];
			Vector boxSize = cls.GetYards()[i]->GetPhysForm().boxes_[0].s;

			if( !pTop )
			{
				Vector vLoc = mtx.MulVertexByInverse( params_.points[0] );
				//float fdelta = vLoc.z*vLoc.z + vLoc.y*vLoc.y;
				bool inBox = vLoc.InBox(-boxSize,boxSize);
				if( params_.pointsCount == 4 ) {
					vLoc = mtx.MulVertexByInverse( params_.points[1] );
					//fdelta += vLoc.z*vLoc.z + vLoc.y*vLoc.y;
					if( !vLoc.InBox(-boxSize,boxSize) )
						inBox = false;
				}
				//if( fdelta < 0.2f )
				if( inBox )
				{
					// нашли привязынную рею - ее запомним
					pTop = cls.GetYards()[i];
				}
			}

			if( !pBottom )
			{
				Vector vLoc1,vLoc2;
				if( params_.pointsCount == 4 ) {
					vLoc1 = mtx.MulVertexByInverse( params_.points[2] );
					vLoc2 = mtx.MulVertexByInverse( params_.points[3] );
				} else {
					vLoc1 = mtx.MulVertexByInverse( params_.points[1] );
					vLoc2 = mtx.MulVertexByInverse( params_.points[2] );
				}
				if( vLoc1.InBox(-boxSize,boxSize) && vLoc2.InBox(-boxSize,boxSize) )
				{
					// нашли привязынную рею - ее запомним
					pBottom = cls.GetYards()[i];
				}
			}
		}

		if( pTop )
		{
			m_TieData.bTied = false;
			m_TieData.bBottomTied = false;
			m_TieData.pTopYard = pTop;
			m_TieData.pBottomYard = pBottom ? pBottom : pTop;

			// позиции привязки для точек верхнего ряда
			Matrix mtx = m_TieData.pTopYard->GetInitialPoses()[0];
			m_TieData.points[0] = mtx.MulVertexByInverse( params_.points[0] );
			// для теугольного паруса две верхние точки привязки - это одна точка
			if( params_.pointsCount == 3 )
				m_TieData.points[1] = m_TieData.points[1];
			// для прямоугольного паруса - привязка идет по обоим точкам
			else
				m_TieData.points[1] = mtx.MulVertexByInverse( params_.points[1] );

			// позиции привязки для точек нижнего ряда
			mtx = m_TieData.pBottomYard->GetInitialPoses()[0];
			if( params_.pointsCount == 3 ) {
			// для треугольного паруса, нижняя привязка по второй и третей точке
				m_TieData.points[2] = mtx.MulVertexByInverse( params_.points[1] );
				m_TieData.points[3] = mtx.MulVertexByInverse( params_.points[2] );
			} else {
			// для прямоугольного паруса, нижняя привязка по третей и четвертой точке
				m_TieData.points[2] = mtx.MulVertexByInverse( params_.points[2] );
				m_TieData.points[3] = mtx.MulVertexByInverse( params_.points[3] );
			}

			// матрицы трансформации для фейкового паруса
			Matrix mOwner(true);
			GetSailOwner()->GetMatrix( mOwner );
			m_TieData.mtxYardTransf = mOwner;
			m_TieData.mtxToSailTransf = m_TieData.pTopYard->GetInitialPoses()[0];
			m_TieData.mtxToSailTransf.Inverse();
		}
		else
		{
			api->Trace( "Warning! Sail %s not tied to yard", name_.GetBuffer() );
		}
	}
}

void Sail::SleepCloth(const Matrix& mtxSleepTransform)
{
	if( m_bSleep ) return;
	m_bSleep = true;
	if( cloth_ )
	{
		m_bUseHighTearFactor = true;
		m_fHighTearTime = fMaxHighTearTime;
		cloth_->SetTearCoef(fHighTearFactor);
		if( m_TieData.bTied )
			Detach();
		m_mtxSleepTransform = mtxSleepTransform;
		cloth_->Sleep();
	}
}

void Sail::WakeupCloth(const Matrix& mtxWakeupTransform)
{
	if( !m_bSleep ) return;
	m_bSleep = false;
	if( cloth_ )
	{
		cloth_->WakeUp( m_mtxSleepTransform.Inverse() * mtxWakeupTransform );
		TieSailToYards();
	}
}

bool Sail::EventPartBreak(ShipPart* pPart)
{
	if( m_TieData.bTied && m_TieData.pTopYard == pPart )
	{
		Detach();
		return true;
	}
	else if ( m_TieData.bBottomTied && m_TieData.pBottomYard==pPart )
	{
		DetachBottom();
		return true;
	}
	return false;
}

void Sail::ShowSailDebugInfo(float x,float y)
{
	if( !GetMOOwner() ) return;
	GetMOOwner()->Render().Print( x, y, 0xFF00FFFF, "Debug info for sail %s : (%s) (%s)", name_.GetBuffer(), isFake_?"Fake":"Physics", m_bSleep?"Sleep":"Wake" );
	if( simData_ )
		GetMOOwner()->Render().Print( x, y+18, 0xFF00FFFF, "    vertex quatity = %d, index quantity = %d, teared = %3.1f", simData_->vertexCount, simData_->indexCount, GetTearedState()*100.f );

	Box bb = cloth_->GetABB();
	GetMOOwner()->Render().DrawBox( bb.c-bb.s*0.5f, bb.c+bb.s*0.5f, Matrix(), 0xFF00FFFF );

	GetMOOwner()->Render().DrawBox( fakeABB_.pos-fakeABB_.size*0.5f, fakeABB_.pos+fakeABB_.size*0.5f, shipMtx_, 0xFF0000FF );
}

void Sail::GetDebugData(SailDebugData& sdd)
{
	if( simData_ )
	{
		sdd.nVertexQ = simData_->vertexCount;
		sdd.nIndexQ = simData_->indexCount;
	}
}

void Sail::TieSailToPhysShape(bool bTop, IPhysRigidBody & body, long nShape, const Vector & v1, const Vector & v2)
{
	if( !cloth_ || nShape<0 )
		return;

	if( bTop )
	{
		// треугольный парус
		if( params_.pointsCount == 3 )
		{
			cloth_->Attach( body, nShape, v1, 0, false, false );
		}
		// прямоугольный парус
		else
		{
			// идем по всей ширине паруса и каждую точку подсоединяем отдельно
			for( unsigned int n=0; n<params_.widthPointsCount; n++ )
			{
				Vector vp = v1 + (v2-v1) * ((float)n/(float)(params_.widthPointsCount-1));
				cloth_->Attach( body, nShape, vp, n, false, false );
			}
		}
	}
	else
	{
		// индексы нижних точек паруса
		long nV2ID,nV3ID;
		if( params_.pointsCount == 3 )
		{
			nV2ID = (params_.widthPointsCount+1)*params_.heightPointsCount/2 - 1;
			nV3ID = nV2ID - (params_.widthPointsCount-1);
		}
		else
		{
			nV2ID = params_.widthPointsCount*params_.heightPointsCount - 1;
			nV3ID = params_.widthPointsCount*(params_.heightPointsCount-1);
		}
		cloth_->Attach( body, nShape, v1, nV2ID, false, false );
		cloth_->Attach( body, nShape, v2, nV3ID, false, false );
	}
}
