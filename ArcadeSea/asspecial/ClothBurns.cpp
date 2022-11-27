#include "ClothBurns.h"
#include "..\..\common_h\IShip.h"


static const float g_fFreezeTimeMax = 1.f;


class BurnZone : public IBurnZone
{
public:
	BurnZone(ClothBurns* pOwner);
	virtual ~BurnZone();

	void AddSphere(ClothBurns::BurnSphere* pSphere) {m_aBurnSphere.Add(pSphere);}
	void DelSphere(ClothBurns::BurnSphere* pSphere) {m_aBurnSphere.Del(pSphere);}

	ClothBurns* m_pOwner;
	array<ClothBurns::BurnSphere*> m_aBurnSphere;
};

BurnZone::BurnZone(ClothBurns* pOwner) :
	m_aBurnSphere(_FL_)
{
	m_pOwner = pOwner;
}

BurnZone::~BurnZone()
{
	if( m_pOwner )
	{
		for( long n=0; n<m_aBurnSphere; n++ )
			m_pOwner->DeleteBurn( m_aBurnSphere[n] );
		m_pOwner->DeleteZone( this );
	}
	// физическое удаление "сферы огня" происходит в манагере "ClothBurns" так что тут только уничтожим динамический массив
	m_aBurnSphere.DelAll();
}





ClothBurns::ClothBurns() :
	m_burnzone(_FL_),
	m_burns(_FL_),
	m_burnedpoints(_FL_)
{
}

ClothBurns::~ClothBurns()
{
	// физическое убиение объекта "зона" лежит на том кто ее получал
	for( long n=0; n<m_burnzone; n++ )
		((BurnZone*)m_burnzone[n])->m_pOwner = 0;
	m_burnzone.DelAll();

	m_burns.DelAllWithPointers();
	for( long n=0; n<m_burnedpoints; n++ )
	{
		RELEASE( m_burnedpoints[n].pParticle );
	}
	m_burnedpoints.DelAll();
}

void _cdecl ClothBurns::Update(float fDeltaTime, long level)
{
	long n;

	Matrix diff(true);
	if( m_burnedpoints.Size()>0 )
	{
		//Видовая матрица с включёной свинг матрицей
		Matrix view = Render().GetView();
		//Чистая видовая матрица
		Matrix realView = Mission().GetInverseSwingMatrix()*Matrix(view).Inverse();
		realView.Inverse();
		//Разносная матрица
		diff.EqMultiply( realView, Matrix(view).Inverse() );
	}
	else
		diff.SetIdentity();

	// обработка точек паруса которые горят
	for( n=0; n<m_burnedpoints; n++ )
	{
		// если объект которому принадлежит огонь невидим, то и нечего обновлять
		if( m_burnedpoints[n].pMO && !m_burnedpoints[n].pMO->IsShow() )
			continue;

		m_burnedpoints[n].fBurningTime += fDeltaTime;
		if( m_burnedpoints[n].fBurningTime >= m_params.m_fMaxBurningTime )
		{
			// удалим партикл горения
			RELEASE( m_burnedpoints[n].pParticle );

			// забабахаем патиклы догорания в ентом месте
			if( m_params.m_pcBurnDestroySFX )
				Particles().CreateParticleSystemEx2( m_params.m_pcBurnDestroySFX, Matrix().EqMultiply(Matrix().BuildPosition(m_burnedpoints[n].GetPosition()), diff), true, _FL_ );

			// оторвем этот кусок паруса
			DestroyClothPoint( m_burnedpoints[n].pMO, m_burnedpoints[n].pCloth, m_burnedpoints[n].pSimData, m_burnedpoints[n].nVertexID );

			// и удалим все... пока никто не заметил :P
			m_burnedpoints.DelIndex(n);
			n--;
		}
		else
		{
			// подвинем партикл в нужную точку
			if( m_burnedpoints[n].pParticle )
			{
				Vector v = m_burnedpoints[n].GetPosition();
				// признак необходимости удаления партикла
				bool bDeleteParticle = false;
				// если погружены в воду, то тушим огонь
				if( v.y < 0.f )
					bDeleteParticle = true;
				// если совпадаем со старой позицией, то
				else if( v.x==m_burnedpoints[n].vOldPos.x && v.y==m_burnedpoints[n].vOldPos.y && v.z==m_burnedpoints[n].vOldPos.z )
				{
					// проверяем: это может быть убитый кусок паруса
					if( (m_burnedpoints[n].fFreezeTime-=fDeltaTime) <= 0.f )
						bDeleteParticle = true;
				}
				else
				{
					m_burnedpoints[n].fFreezeTime = g_fFreezeTimeMax;
					m_burnedpoints[n].vOldPos = v;
				}

				// удалим партикл горения
				if( bDeleteParticle )
				{
					RELEASE( m_burnedpoints[n].pParticle );
				}
				// либо двигаем его за парусом
				else
				{
					m_burnedpoints[n].pParticle->CancelHide();
					m_burnedpoints[n].pParticle->SetTransform( Matrix().EqMultiply(Matrix().BuildPosition(v), diff) );
					static const ConstString typeId("Ship");
					if( m_burnedpoints[n].pMO && m_burnedpoints[n].pMO->Is(typeId) )
					{
						Matrix m(true);
						m_burnedpoints[n].pParticle->AdditionalStartVelocity(
							((IShip*)m_burnedpoints[n].pMO)->GetCurrentVelocity() *
							m_burnedpoints[n].pMO->GetMatrix(m).MulNormal(Vector(0.f,0.f,1.f)) );
					}
				}
			}
		}
	}

	// обработка сфер горения
	for( n=0; n<m_burns; n++ )
	{
		// обновим время
		m_burns[n]->fLiveTime += fDeltaTime;
		if( m_burns[n]->fLiveTime >= m_burns[n]->fLiveTimeMax )
		{
			DELETE( m_burns[n] );
			m_burns.DelIndex(n--);
			continue;
		}
		float k = m_burns[n]->fLiveTime / m_burns[n]->fLiveTimeMax;
		// обновим позицию для привязанного к ткани огня
		if( m_burns[n]->pCloth )
		{
			m_burns[n]->c = *(Vector*)((BYTE*)m_burns[n]->pSimData->posBuffer + m_burns[n]->pSimData->posStride*m_burns[n]->nVertexID);
		}
		// обновим позицию для привязанного к объекту огня
		else if( m_burns[n]->pTieObj )
		{
			Matrix m(true);
			m_burns[n]->c = m_burns[n]->pTieObj->GetMatrix(m).MulVertex(m_burns[n]->vLoc);
		}
		m_burns[n]->r = m_burns[n]->fRad + k * m_burns[n]->fRadAdd;

		// под водой гореть не можем - удаляемся
		if( m_burns[n]->c.y < 0.f )
		{
			DELETE( m_burns[n] );
			m_burns.DelIndex(n--);
			continue;
		}

		// дебажная отрисовка
		if( m_params.m_bShowDebug )
		{
			IRender* render = (IRender*)api->GetService("DX9Render");
			Assert(render);
			render->DrawSphere( m_burns[n]->c, m_burns[n]->r, m_burns[n]->pCloth ? 0xFF802080 : 0xFF802020 );
		}
	}
}

bool ClothBurns::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	//Registry( "ClothBurns" );

	SetUpdate(&ClothBurns::Update, ML_GEOMETRY5+1);

	return true;
}

bool ClothBurns::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

bool ClothBurns::Init()
{
	return true;
}

void ClothBurns::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;
	if( string::IsEqual(id,"addpos") )
	{
		if( numParams>=3 )
		{
			Vector vPos;
			vPos.x = (float)atof(params[0]);
			vPos.y = (float)atof(params[1]);
			vPos.z = (float)atof(params[2]);
			AddOneBurn(group_shipfire, vPos,0,300.f,1.0f);
		}
	}
}

void ClothBurns::AddBurnNotHosted(BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime, float fRad)
{
	AddOneBurn(grpID, vPos, pMO, fLiveTime, fRad);
}

IBurnZone* ClothBurns::AddBurn(IClothBurns::BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime, float fRad)
{
	BurnSphere* pSphere = AddOneBurn(grpID, vPos, pMO, fLiveTime, fRad);
	if( pSphere )
	{
		BurnZone* pZone = NEW BurnZone(this);
		Assert(pZone);
		m_burnzone.Add( pZone );
		pZone->AddSphere( pSphere );
		return pZone;
	}
	return NULL;
}

IBurnZone* ClothBurns::AddBurnLine(IClothBurns::BurnGroupID grpID, const Vector& vPosSrc, const Vector& vPosDst, MissionObject* pMO, float fLiveTime, float fRad)
{
	// вычисляем радиус который нам нужен
	BurnParams::BurnGroup & params = grpID==group_flamethrower ? m_params.m_FlameThrowerGroup :
									grpID==group_shipfire ? m_params.m_ShipFireGroup :
									grpID==group_explose ? m_params.m_ExploseGroup :
									m_params.m_ClothGroup;
	if( fRad <= 0.f )
		fRad = params.fMinRadius + (params.fMaxRadius-params.fMinRadius)* rand()/RAND_MAX;
	if( fRad < 0.05f )
		return NULL;
	// и время жизни
	if( fLiveTime <= 0.f )
		fLiveTime = params.fTimeMin + (params.fTimeMax-params.fTimeMin)* rand()/RAND_MAX;

	// направление ряда пламени и его дальность
	Vector vDir = vPosDst - vPosSrc;
	float fLength = vDir.Normalize();
	// количство пламени в этом ряду
	long nQ = (long)(0.5f * fLength / fRad);
	if( nQ <= 1 )
		return AddBurn(grpID, 0.5f*(vPosSrc+vPosDst), pMO, fLiveTime, fRad);

	// родим зону
	BurnZone* pZone = NEW BurnZone(this);
	Assert(pZone);
	m_burnzone.Add( pZone );

	// родим сферы пламени и добавим их в зону
	float fStep = fLength / nQ;
	float fK = 0.5f * fStep;
	for( long n=0; n<nQ; n++ )
	{
		BurnSphere* pSphere = AddOneBurn(grpID, vPosSrc + vDir*fK, pMO, fLiveTime, fRad);
		if( pSphere )
			pZone->AddSphere( pSphere );
		fK += fStep;
	}

	return pZone;
}

bool ClothBurns::ProcessCloth(MissionObject* pTiedMO, IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData)
{
	if( !pCloth || !pSimData )
		return false;

	// пробегемся по огонькам и проверим их на попадание в бокс вокруг ткани
	Box clothbox = pCloth->GetABB();
	clothbox.size *= 0.5f;
	Matrix mtxBox(true);
	mtxBox.BuildPosition( clothbox.pos );
	long q = m_burns.Size();
	for( long i=0; i<q; i++ )
	{
		// уже прогорел огонек - не используем его
		if( m_burns[i]->fLiveTime < m_burns[i]->fInactiveTime )
			continue;

		// огни другой ткани не могут поджечь эту ткань
		// т.к. иначе одна искра на парусе вероятно спалит все паруса на корабле (отлетая, куски паруса частенько жгут другие)
		if( m_burns[i]->pCloth )
		{
			if( m_burns[i]->pCloth != pCloth )
				continue;
		}

		// попадаем - значит обрабатываем конкретно
		if( clothbox.OverlapsBoxSphere( mtxBox, clothbox.size, m_burns[i]->c, m_burns[i]->r ) )
		{
			Vector vBurnC = m_burns[i]->c;
			float fBurnRQuad = m_burns[i]->r * m_burns[i]->r;
			// бегем по вертексам и проверяем их
			for( long n=0; n<(long)pSimData->vertexCount; n++ )
			{
				Vector v = *(Vector*)((BYTE*)pSimData->posBuffer + pSimData->posStride * n);
				if( (~(v - vBurnC)) < fBurnRQuad )
				{
					if( AddBurnedPoint(pCloth, pSimData, n, pTiedMO) )
					{
						// рождаем новый огонь
						Matrix m(true);
						AddOneBurn( group_cloth, pTiedMO->GetMatrix(m).MulVertexByInverse(v), pTiedMO, 0, 0, pCloth, pSimData, n );
					}
				}
			}
		}
	}

	// если этот парус горит, то возвращаем истину
	for(i=0; i<m_burnedpoints; i++)
		if( m_burnedpoints[i].pCloth == pCloth )
			return true;

	// если парус не горит, то возвращаем ложь
	return false;
}

bool ClothBurns::CheckBurnIntoBox(IPhysCloth* pCloth, const Matrix& mtx, const Vector& size)
{
	// бегем по всем огням
	for( long i=0; i<m_burns; i++ )
		// огонь на другом парусе не поджигает этот
		if( !m_burns[i]->pCloth )
			// проверим попадание в бокс
			if( Box::OverlapsBoxSphere( mtx, size, m_burns[i]->c, m_burns[i]->fRad ) )
				return true;
	return false;
}

void ClothBurns::ExcludeObjects(MissionObject* pMO)
{
	if( !pMO ) return;

	// ищем источники пожара ссылаемые на этот объект
	for( long n=0; n<m_burns; n++ )
		if( m_burns[n] && m_burns[n]->pTieObj == pMO )
		{
			DELETE( m_burns[n] );
			m_burns.DelIndex( n );
			n--;
		}
}

void ClothBurns::ExcludeObjects(IPhysCloth* pCloth)
{
	if( !pCloth ) return;

	// ищем огни на парусе связанные с этим объектом
	for( long n=0; n<m_burnedpoints; n++ )
		if( m_burnedpoints[n].pCloth == pCloth )
		{
			if( m_burnedpoints[n].pParticle )
			{
				RELEASE( m_burnedpoints[n].pParticle );
			}
			m_burnedpoints.DelIndex( n );
			n--;
		}

	// ищем очаги огня привязанные к этой тряпке
	for( n=0; n<m_burns; n++ )
		if( m_burns[n]->pCloth == pCloth )
		{
			DELETE( m_burns[n] );
			m_burns.DelIndex( n );
			n--;
		}
}

void ClothBurns::ReadMOPs(MOPReader & reader)
{
	m_params.m_fGroupingRadius = reader.Float();
	m_params.m_pcBurnSFX = reader.String().c_str();
	m_params.m_fMaxBurningTime = reader.Float();
	m_params.m_pcBurnDestroySFX = reader.String().c_str();
	m_params.m_fReburnProbability = reader.Float();

	m_params.m_ClothGroup.fTimeMin = reader.Float();
	m_params.m_ClothGroup.fTimeMax = reader.Float();
	m_params.m_ClothGroup.fTimeInactive = reader.Float();
	m_params.m_ClothGroup.fMinRadius = reader.Float();
	m_params.m_ClothGroup.fMaxRadius = reader.Float();
	m_params.m_ClothGroup.fGrowingFactor = reader.Float() * 0.01f;

	m_params.m_ShipFireGroup.fTimeMin = reader.Float();
	m_params.m_ShipFireGroup.fTimeMax = reader.Float();
	m_params.m_ShipFireGroup.fTimeInactive = reader.Float();
	m_params.m_ShipFireGroup.fMinRadius = reader.Float();
	m_params.m_ShipFireGroup.fMaxRadius = reader.Float();
	m_params.m_ShipFireGroup.fGrowingFactor = reader.Float() * 0.01f;

	m_params.m_FlameThrowerGroup.fTimeMin = reader.Float();
	m_params.m_FlameThrowerGroup.fTimeMax = reader.Float();
	m_params.m_FlameThrowerGroup.fTimeInactive = reader.Float();
	m_params.m_FlameThrowerGroup.fMinRadius = reader.Float();
	m_params.m_FlameThrowerGroup.fMaxRadius = reader.Float();
	m_params.m_FlameThrowerGroup.fGrowingFactor = reader.Float() * 0.01f;

	m_params.m_ExploseGroup.fTimeMin = reader.Float();
	m_params.m_ExploseGroup.fTimeMax = reader.Float();
	m_params.m_ExploseGroup.fTimeInactive = reader.Float();
	m_params.m_ExploseGroup.fMinRadius = reader.Float();
	m_params.m_ExploseGroup.fMaxRadius = reader.Float();
	m_params.m_ExploseGroup.fGrowingFactor = reader.Float() * 0.01f;

	m_params.m_fExplosionBurnProbability = reader.Float();

	m_params.m_bShowDebug = reader.Bool();
}

ClothBurns::BurnSphere* ClothBurns::AddOneBurn(IClothBurns::BurnGroupID grpID, const Vector& vPos, MissionObject* pMO, float fLiveTime, float fRad, IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData, long nVertexID)
{
	BurnSphere bs;

	BurnParams::BurnGroup & params = grpID==group_flamethrower ? m_params.m_FlameThrowerGroup :
									grpID==group_shipfire ? m_params.m_ShipFireGroup :
									grpID==group_explose ? m_params.m_ExploseGroup :
									m_params.m_ClothGroup;

	// постоянные параметры
	bs.grpID = grpID;
	bs.fInactiveTime = params.fTimeInactive;
	bs.vLoc = vPos;
	bs.pTieObj = pMO;
	bs.fLiveTimeMax = fLiveTime>0.f ?
							fLiveTime :
							(params.fTimeMin + (params.fTimeMax-params.fTimeMin)* rand()/RAND_MAX );
	bs.fRad = fRad>0.f ?
						fRad :
						(params.fMinRadius + (params.fMaxRadius-params.fMinRadius)* rand()/RAND_MAX );
	bs.fRadAdd = bs.fRad * params.fGrowingFactor;

	// переменные параметры
	Matrix m(true);
	bs.c = pMO ? pMO->GetMatrix(m).MulVertex(vPos) : vPos;
	bs.r = bs.fRad;
	bs.fLiveTime = 0.f;

	// привязка к ткани если есть
	bs.pCloth = pCloth;
	bs.pSimData = pSimData;
	bs.nVertexID = nVertexID;

	// поищем с кем бы сгруппироваться (много огоньков в одном месте это большой один огонь)
	// или добавляем новый огенек в спиcок если не удалось присоединиться к другому огоньку
	BurnSphere* pBurnSphere = 0;
	if( grpID==group_cloth )
		pBurnSphere = GroupBurns(bs);

	if( !pBurnSphere )
	{
		pBurnSphere = NEW BurnSphere(bs);
		Assert( pBurnSphere );
		m_burns.Add( pBurnSphere );
	}
	return pBurnSphere;
}

ClothBurns::BurnSphere* ClothBurns::GroupBurns(BurnSphere& bs)
{
	// смысл группироваться есть только тогда, когда мы еще не достигли максимального радиуса группировки
	if( bs.fRad < m_params.m_fGroupingRadius )
	{
		int nGroupingIdx = -1;
		float fGroupingDist = 4.f * m_params.m_fGroupingRadius * m_params.m_fGroupingRadius;
		for(int i=0; i<m_burns; i++)
		{
			// группировать можно только огни рожденные от ткани
			if( m_burns[i]->grpID != group_cloth )
				continue;

			// если огонь не с этой ткани то не группируемся с ним
			if( m_burns[i]->pCloth != bs.pCloth )
				continue;

			// максимальная дистанция для группирования этой пары очагов горения
			float fAccessDistance = 2.f * m_params.m_fGroupingRadius - bs.r - m_burns[i]->r;
			if( fAccessDistance <= 0.f )
				continue;
			// реальная дистанция (в квадрате)
			float fQuadDist = ~(bs.c - m_burns[i]->c);
			if( fQuadDist < fAccessDistance*fAccessDistance && fQuadDist < fGroupingDist )
			{
				fGroupingDist = fQuadDist;
				nGroupingIdx = i;
			}
		}
		if( nGroupingIdx >= 0 )
		{
			float fDist = sqrtf(fGroupingDist);
			float fNewRad = 0.5f * ( fDist + bs.r + m_burns[nGroupingIdx]->r );
			float k = fNewRad / m_burns[nGroupingIdx]->r;
			// глобальная позиция центра огня
			m_burns[nGroupingIdx]->c += ((fNewRad-m_burns[nGroupingIdx]->r) / fDist) * (bs.c - m_burns[nGroupingIdx]->c);
			// локальная позиция центра огня
			Matrix m(true);
			m_burns[nGroupingIdx]->vLoc = m_burns[nGroupingIdx]->pTieObj ?
				m_burns[nGroupingIdx]->pTieObj->GetMatrix(m).MulVertexByInverse( m_burns[nGroupingIdx]->c ) :
				m_burns[nGroupingIdx]->c;
			// базовый радиус огня
			m_burns[nGroupingIdx]->fRad *= k;
			// прирост радиуса огня от времени
			m_burns[nGroupingIdx]->fRadAdd *= k;
			// текущий радиус огня
			m_burns[nGroupingIdx]->r = m_burns[nGroupingIdx]->fRad + m_burns[nGroupingIdx]->fRadAdd * m_burns[nGroupingIdx]->fLiveTime / m_burns[nGroupingIdx]->fLiveTimeMax;
			return m_burns[nGroupingIdx];
		}
	}
	return NULL;
}

bool ClothBurns::DestroyClothPoint(MissionObject* pMO, IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData, long nVertexID)
{
	// проверим ткань на максимально допустимое количество разрывов
	if( pSimData->vertexCount >= pSimData->maxVertexCount - 1 )
		return false;
	if( pSimData->indexCount >= pSimData->maxIndexCount - 3 )
		return false;

	// ищем вертексы присоединенные к удаляемому - что бы порвать ткань у них
	WORD* pI = (WORD*)pSimData->ib;
	array<unsigned int> aTearedVertex(_FL_);
	for( unsigned int i=0; i<pSimData->indexCount; i++ )
	{
		if( pI[i]==nVertexID )
		{
			unsigned int ni = i/3 * 3;
			if( ni != i ) aTearedVertex.Add(pI[ni]);
			if( ni+1 != i ) aTearedVertex.Add(pI[ni+1]);
			if( ni+2 != i ) aTearedVertex.Add(pI[ni+2]);
		}
	}

	// отсоединим точку паруса от всех привязанностей
	pCloth->Detach( nVertexID );
	// получим ее позицию
	Vector vc = *(Vector*)((BYTE*)pSimData->posBuffer + pSimData->posStride * nVertexID);
	// пробежимся по всем точкам которые составляют треугольник с искомой точкой
	for( i=0; i<aTearedVertex.Size(); i++ )
	{
		// отсоединяем эту точку от всех привязанностей
		pCloth->Detach( aTearedVertex[i] );
		// получаем направление от этой точки до искомой
		Vector vn = vc - *(Vector*)((BYTE*)pSimData->posBuffer + pSimData->posStride * aTearedVertex[i]);
		vn.Normalize();
		// рвем связи в этой точке в направлении до искомой точки - т.е. отраваем кусок так, что бы
		// остальной парус остался целым, а вырывался кусок который содержит центральную исключаемую точку
		pCloth->TearVertex(aTearedVertex[i], vn);

		// добавим огонь в точку которая осталась после отрыва
		if( 100.f/RAND_MAX * rand() < m_params.m_fReburnProbability )
			AddBurnedPoint( pCloth, pSimData, aTearedVertex[i], pMO );
	}
	//pCloth->Attach( nVertexID, Vector(vc.x,-100.f,vc.z) );

	return true;
}

bool ClothBurns::AddBurnedPoint(IPhysCloth* pCloth, IPhysCloth::SimulationData* pSimData, long nVertexID, MissionObject* pMO)
{
	static const int maxBurnsInOneTime = 2;
	int curBurnsInOneTime = 0;
	// если уже горит такая точка то уйдем отсюда
	for( long n=0; n<m_burnedpoints; n++ )
		if( m_burnedpoints[n].pCloth == pCloth )
		{
			// горит именно эта точка - выходим сразу
			if( m_burnedpoints[n].nVertexID == nVertexID )
				return false;
			// горит другая точка, но зажглась недавно, также выходим, ибо
			// не хотим чтоб парус сгорел сразу весь в один миг
			if( m_burnedpoints[n].fBurningTime < 0.25f*m_params.m_fMaxBurningTime )
				if( ++curBurnsInOneTime >= maxBurnsInOneTime )
					return false;
		}

	// а вот если не горит, то жгем еее!
	n = m_burnedpoints.Add();
	m_burnedpoints[n].pCloth = pCloth;
	m_burnedpoints[n].pMO = pMO;
	m_burnedpoints[n].pSimData = pSimData;
	m_burnedpoints[n].nVertexID = nVertexID;
	m_burnedpoints[n].fBurningTime = 0.f;
	m_burnedpoints[n].fFreezeTime = g_fFreezeTimeMax;
	m_burnedpoints[n].pParticle = m_params.m_pcBurnSFX ? Particles().CreateParticleSystemEx2( m_params.m_pcBurnSFX, Matrix().BuildPosition(Mission().GetInverseSwingMatrix().MulVertex(m_burnedpoints[n].GetPosition())), false, _FL_ ) : NULL;
	if( m_burnedpoints[n].pParticle )
		m_burnedpoints[n].pParticle->AutoHide(true);
	return true;
}

static const char description[] = "Cloth burning object. Common settings and parameters.";
MOP_BEGINLISTCG(ClothBurns, "ClothBurns", '1.00', 100, description, "Arcade Sea");
	MOP_FLOATC("Grouping radius", 2.25f, "Радиус до которого соседние огни объединяются в один");
	MOP_STRING("Cloth burn SFX", "ClothBurning.xps");
	MOP_FLOATC("Cloth burning time", 4.f, "Время которое точка ткани живет прежде чем уничтожается");
	MOP_STRING("Cloth burn destroy SFX", "");
	MOP_FLOATEXC("Cloth reburn probability", 50, 0, 100, "Вероятность распространения огня от прогоревшей точки на соседние (в процентах)");

	MOP_GROUPBEGC("BurnByCloth", "Параметры огня рождаемого от сгораемой ткани")
	MOP_FLOAT("(GC) Min live time", 5);
	MOP_FLOAT("(GC) Max live time", 7);
	MOP_FLOATC("(GC) Inactive time", 2, "Время в течении которого огонь не поджигает ткань");
	MOP_FLOAT("(GC) Min burn radius", 1.1f);
	MOP_FLOAT("(GC) Max burn radius", 1.3f);
	MOP_FLOAT("(GC) Burn growing procents", 80.f);
	MOP_GROUPEND()

	MOP_GROUPBEGC("BurnByShipfire", "Параметры огня рождаемого от горения корабля")
	MOP_FLOAT("(GS) Min live time", 180);
	MOP_FLOAT("(GS) Max live time", 180);
	MOP_FLOATC("(GS) Inactive time", 0.0f, "Время в течении которого огонь не поджигает ткань");
	MOP_FLOAT("(GS) Min burn radius", 1.4f);
	MOP_FLOAT("(GS) Max burn radius", 1.7f);
	MOP_FLOAT("(GS) Burn growing procents", 30.f);
	MOP_GROUPEND()

	MOP_GROUPBEGC("BurnByFlamethrower", "Параметры огня рождаемого огнеметом")
	MOP_FLOAT("(GF) Min live time", 60);
	MOP_FLOAT("(GF) Max live time", 60);
	MOP_FLOATC("(GF) Inactive time", 0.0f, "Время в течении которого огонь не поджигает ткань");
	MOP_FLOAT("(GF) Min burn radius", 3.0f);
	MOP_FLOAT("(GF) Max burn radius", 3.0f);
	MOP_FLOAT("(GF) Burn growing procents", 0.f);
	MOP_GROUPEND()

	MOP_GROUPBEGC("BurnByExplose", "Параметры огня рождаемого при взрыве ядра о корпус/мачту")
	MOP_FLOAT("(GE) Min live time", 2);
	MOP_FLOAT("(GE) Max live time", 2);
	MOP_FLOATC("(GE) Inactive time", 0.0f, "Время в течении которого огонь не поджигает ткань");
	MOP_FLOAT("(GE) Min burn radius", 4.0f);
	MOP_FLOAT("(GE) Max burn radius", 4.0f);
	MOP_FLOAT("(GE) Burn growing procents", 30.f);
	MOP_GROUPEND()

	MOP_FLOATEXC("Explosion burn probability", 70, 0, 100, "Вероятность (0-100 процентов) рождения огня в месте взрыва ядра");

	MOP_BOOL("Draw debug",false);
MOP_ENDLIST(ClothBurns)
