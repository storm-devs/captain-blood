//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////
#include "..\pch.h"
#include ".\weapon.h"
#include "..\..\common_h\IShip.h"
#include "..\ships\PartsClassificator.h"
#include "..\ships\HullPart.h"
#include "..\ships\Mast.h"
#include "..\ships\ShipPart.h"
#include "..\ships\Ship.h"

bool WeaponTargetZone::SphereZone::GetCrossConeXZ(const Vector& vSrc, const Vector& vRay, float fSinAng, Vector* pVOut) const
{
	float fXdir = c.x - vSrc.x;
	float fZdir = c.z - vSrc.z;
	// дистанция от центра цели до луча направления стрельбы
	float fDistSrcToOrtoPlane = vRay.x * fXdir + vRay.z * fZdir;
	if( fDistSrcToOrtoPlane < 0.f )
		return false; // в другую сторону палить не бум

	// оставшееся расстояние с учетом радиуса
	float fEffectRadius = fDistSrcToOrtoPlane - r;

	// дистанция меньше радиуса сферы или большой угол (около 90 град) считаем что уже попали
	if( fEffectRadius <= 0.f || fSinAng > 0.99f )
	{
		if( pVOut )
			*pVOut = c;
		return true;
	}
	// тангенс в квадрате (что бы определить приемлимую дистанцию на указанном удалении)
	float fTanAng2Pow = fSinAng*fSinAng / (1.f - fSinAng*fSinAng);
	// квадрат расстояния до цели по лучу направления стрельбы
	float fDistCenterToRaySqr = fXdir*fXdir + fZdir*fZdir - fDistSrcToOrtoPlane*fDistSrcToOrtoPlane;
	// квадрат приемлимого расстояния до цели (на данной дистанции)
	float fOkDistSqr = fDistCenterToRaySqr * fTanAng2Pow;
	//
	if( fEffectRadius*fEffectRadius > fOkDistSqr )
		return false;

	if( pVOut )
	{
/*		float fDistCenterToRay = sqrtf(fDistCenterToRaySqr);
		if( fDistCenterToRay > 0.1f )
		{
			// минимальная дистанция луча до центра сферы
			float fMinDist = fDistCenterToRay - fRayWidthIntoOrtoPlane;
			if( fMinDist < 0.f )
				fMinDist = 0.f;
			*pVOut = c + (fMinDist / fDistCenterToRay) * (vSrc + vRay*fDistSrcToOrtoPlane - c);
			pVOut->y = c.y;
		}
		else*/
			*pVOut = c;
	}
	return true;
}

WeaponTargetZone::WeaponTargetZone(const Vector& src, MissionObject* pMO, bool upperTarget) :
	z(_FL_)
{
	Matrix m;
	if(pMO)
		pMO->GetMatrix( m );
	target = m.pos;
	bUseZone = false;
	bKnippels = upperTarget;

	if( pMO )
	{
		static const ConstString typeId("Ship");
		if( pMO->Is(typeId) )
		{
			long n,i;
			//PartsClassificator sd;
			//sd.Visit( *(Ship*)pMO );
			PartsClassificator & sd = ((Ship*)pMO)->GetPartClassificator();
			sd.Visit( *(Ship*)pMO );

			// выбираем мачты с парусами - ибо в них целимся
			if( upperTarget )
			{
				// подготовим массив использования мачт (по количеству целых парусов на них)
				struct MastUsage
				{
					ShipPart * pMast;
					float fUsage;
				};
				array<MastUsage> aMasts(_FL_);
				for(n=0; n<sd.GetSails(); n++)
				{
					ShipPart * pMast = sd.GetSails()[n]->GetParent();
					for( i=0; i<aMasts; i++ )
						if( pMast == aMasts[i].pMast )
							break;
					if( i==aMasts.Size() )
					{
						i = aMasts.Add();
						aMasts[i].fUsage = 0.f;
						aMasts[i].pMast = pMast;
					}
					aMasts[i].fUsage += sd.GetSails()[n]->GetHealth() < 1.f ? 1.f : 3.f;
				}

				// бегем по этому массиву и строим из них зоны
				for(n=0; n<aMasts; n++)
				{
					if( aMasts[n].fUsage <= 0.f )
						continue;
					MastPart* pMastPart = ((Mast*)aMasts[n].pMast)->GetPart(0);
					if( !pMastPart || pMastPart->GetInitialPoses().Size()==0 )
						continue;
					// выберем часть мачты по которой палим
					long mp = rand() % pMastPart->GetInitialPoses().Size();
					// добавляем зону
					i = z.Add();
					z[i].c = m.MulNormal( pMastPart->GetInitialPoses()[mp].pos );
					z[i].fPrioritet = aMasts[n].fUsage;
					z[i].r = 1.f; // мачта штука тонкая стараемся попасть точно
				}
			}
			// выбираем корпус - т.к. палим в него
			else
			{
				for(n=0; n<sd.GetHullParts(); n++)
				{
					// уничтоженную часть не расстреливаем снова - ибо нефиг
					if( sd.GetHullParts()[n]->IsBroken() )
						continue;
					// добавляем зону
					i = z.Add();
					Vector &vcpos = sd.GetHullParts()[n]->GetInitialPoses()[0].pos;
					z[i].c = m.MulNormal( Vector(vcpos.x,0.f,vcpos.z) );
					z[i].fPrioritet = sd.GetHullParts()[n]->IsDamaged() ? 1.f : 3.f;
					z[i].r = 0.2f;//0.5f * sqrtf( ~(sd.GetHullParts()[n]->GetGMXEntity()->GetBound().vMax - sd.GetHullParts()[n]->GetGMXEntity()->GetBound().vMin) );
				}
			}
		}
		else
		{
			long i = z.Add();
			z[i].c = m.pos;
			z[i].fPrioritet = 1.f;
			z[i].r = 10.f;
		}
	}

	bUseZone = z.Size()>0;
}

long WeaponTargetZone::GetTargetZoneByCone(const Vector& vSrc, const Vector& vRay, float fAng, Vector* pVOut) const
{
	// проверка на корректность данных
	if( z.Size() == 0 )
		return -1;

	Vector vLocSrc = vSrc - target;

	unsigned int n;

	// подготовить массив вероятностей для выбора зоны
	float fZoneProbability[256] = {0.f};
	float fCommonProbability = 0.f;
	for( n=0; n<z.Size(); n++ )
	{
		fCommonProbability += z[n].fPrioritet;
		fZoneProbability[n] = z[n].fPrioritet;
	}

	float fSinAng = sinf(fAng);
	// выбираем зоны случайным образом и проверяем их на попадание в луч
	while(fCommonProbability > 0.01f)
	{
		float fCheck = fCommonProbability * rand() / RAND_MAX;
		for( n=0; n<z.Size(); n++ )
		{
			fCheck -= fZoneProbability[n];
			if( fCheck < 0.f )
			{
				// выбрали эту зону - смотрим ее
				if( z[n].GetCrossConeXZ( vLocSrc, vRay, fSinAng, pVOut ) )
				{
					if( pVOut )
						*pVOut += target;
					return n;
				}
				// не попала зона в луч - ваыкидываем ее из вероятностей
				fCommonProbability -= fZoneProbability[n];
				break;
			}
		}
		if( n == z.Size() )
			break;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// Weapon
//////////////////////////////////////////////////////////////////////////

Weapon::Weapon(void)
{
}

Weapon::Weapon(const Weapon& weapon)
{
	*this = weapon;
}

const Weapon& Weapon::operator=(const Weapon& weapon)
{
	if (params_.model_)
		params_.model_->Release();

	params_ = weapon.params_;
    
	if (params_.model_)
		params_.model_->AddRef();

	return *this;
}

Weapon::~Weapon(void)
{
	if (params_.model_)
		params_.model_->Release();
}