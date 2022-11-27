#include "..\pch.h"
#include "ShipPart.h"
#include "..\..\Common_h\particles.h"
#include "..\..\Common_h\mission.h"
#include "Ship.h"
#include "..\asspecial\ClothBurns.h"
#include "..\SeaMissionParams.h"

static const float SeaHeight = 0.f;

ShipPart::~ShipPart(void)
{
	CleanBrokenSystem(true);

	children_.DelAllWithPointers();

	ReleaseHitParticles();
}

void ShipPart::Rebuild()
{
	// больше часть не считается разбитой
	SetBroken(false);
	// очищаем систему овалившихся частей (т.к. теперь мы снова часть корабля)
	CleanBrokenSystem(true);
	// убираем партиклы повреждения
	ReleaseHitParticles();
	// восстановим альфу, она могла быть убрана при плавном исчезновении части
	SetAlpha(1.f);

	// если мы являемся частью разрушенной системы, то ищем ее и исключаемся оттуда
	if( physdata_.actor )
	{
		for ( ShipPart* pPar=GetParent(); pPar; pPar=pPar->GetParent() )
		{
			if( pPar->GetBrokenSystem().Size()>0 )
			{
				for( long n=0; n<pPar->GetBrokenSystem(); n++ )
				{
					if( pPar->GetBrokenSystem()[n].actor == physdata_.actor )
					{
						ReleasePhysData( physdata_.actor );
						pPar->DelFromBrokenSystem(n);
						break;
					}
				}
				break;
			}
		}
		if( !pPar )
		{
			ReleasePhysData( physdata_.actor );
		}
	}

	// включаем шейпы частей (т.к. теперь это снова часть корабля)
	if( GetShipOwner() && GetShipOwner()->GetPhysView() )
	{
		for (unsigned int i = 0; i < GetShapes().Size(); ++i)
		{
			GetShipOwner()->GetPhysView()->SetLocalTransform(GetShapes()[i], GetInitialPoses()[i]);
			GetShipOwner()->GetPhysView()->EnableCollision(GetShapes()[i], true);
			GetShipOwner()->GetPhysView()->EnableResponse(GetShapes()[i], true);
			GetShipOwner()->GetPhysView()->EnableRaycast(GetShapes()[i], true);
		}
	}
}

void ShipPart::SetBroken(bool value)
{
	if( isBroken_ == value )
		return;
	isBroken_ = value;
	if( value && GetShipOwner() )
	{
		// сбросить человечков - если есть привязанные к этой части
		GetShipOwner()->DropSailors( this );
	}
}

void ShipPart::CleanBrokenSystem(bool doRelease)
{
	if (brokenSystem_.Size() == 0)
		return;

	if (doRelease)
		for (unsigned int i = 0; i < brokenSystem_.Size(); ++i)
		{
			if (brokenSystem_[i].actor)
			{
				ReleasePhysData( brokenSystem_[i].actor );
				DelFromBrokenSystem(i);
				i--;
			}
		}
	brokenSystem_.DelAll();
}

void ShipPart::DelFromBrokenSystem(long nItemIdx)
{
	if( nItemIdx >= 0 && nItemIdx < brokenSystem_ )
	{
		SeaMissionParams* pSMParams = GetSeaMParams();
		if( pSMParams )
		{
			pSMParams->JointToDelete(brokenSystem_[nItemIdx].joint);
			pSMParams->ActorToDelete(brokenSystem_[nItemIdx].actor);
		}
		else
		{
			RELEASE( brokenSystem_[nItemIdx].joint );
			RELEASE( brokenSystem_[nItemIdx].actor );
			api->Trace("ShipPart perfomance warning: broken system do forced release, because not finded SeaMissionParams.");
		}
		brokenSystem_.DelIndex( nItemIdx );
	}
}

bool ShipPart::ReleasePhysData(IPhysRigidBody* actor)
{
	if( physdata_.actor == actor )
	{
		physdata_.active = false;
		physdata_.actor = NULL;
		physdata_.bWaitFallToSea = false;
		return true;
	}

	for( unsigned int n=0; n<children_.Size(); n++ )
		if( children_[n]->ReleasePhysData(actor) )
			return true;

	return false;
}

void ShipPart::SetPhysActor(IPhysRigidBody* actor)
{
	Assert(!physdata_.actor);
	physdata_.actor = actor;
	physdata_.active = true;
	physdata_.bWaitFallToSea = true;
}

bool ShipPart::AddHitAnimation(const char* sfxName, float fOwnerVelocityFactor, const Vector& worldPosition, unsigned short grpID, long nID, float fLiveTime)
{
	// если это специальный дамаг, то проверим есть ли он уже в списке (чтоб не создавать заново)
	if( grpID != hitgrp_common && nID != -1 )
	{
		for (unsigned int i=0; i<hits_.Size(); i++)
			if( hits_[i].nID == nID && hits_[i].dwGroupID == grpID )
			{
				// неживой партикл воскрешаем
				if ( !hits_[i].ps || !hits_[i].ps->IsAlive() )
				{
					hits_[i].Release();
					hits_[i].Create( GetMOOwner()->Particles(), sfxName );
				}
				// отсчет времени запускаем снова
				hits_[i].fLiveTime = 0.f;
				hits_[i].fMaxLiveTime = fLiveTime;
				// если на паузе, то распаузим
				if( hits_[i].ps && !hits_[i].ps->IsActive() )
					hits_[i].ps->PauseEmission(false);

				return false;
			}
	}

	// найдем место под новый дамаг
	unsigned int iHit;
	for (iHit = 0; iHit < hits_.Size(); iHit++)
		if ( !hits_[iHit].ps || !hits_[iHit].ps->IsAlive() )
		{
			hits_[iHit].Release();
			break;
		}
	if( iHit == hits_.Size() )
		iHit = hits_.Add();

	// ставим этому повреждению все параметры
	Hit & h = hits_[iHit];
	// позиция
	Matrix m;
	GetMOOwner()->GetMatrix(m);
	h.pos = m.MulVertexByInverse(worldPosition);
	// партикловая система
	h.Create( GetMOOwner()->Particles(), sfxName );
	//h.ps = GetMOOwner()->Particles().CreateParticleSystemEx(sfxName, __FILE__, __LINE__);
	//if( h.ps )
	//	h.ps->AutoHide( true );
	// идентификатор
	h.dwGroupID = grpID;
	h.nID = nID;
	// время жизни
	h.fMaxLiveTime = fLiveTime;
	h.fLiveTime = 0.f;
	// привязка к хозяину по скорости
	h.fOwnerVelocityFactor = fOwnerVelocityFactor;

	return true;
}

bool ShipPart::AddHitAnimation(OwnerDependedParticles & particles, const Vector& worldPosition, unsigned short grpID, long nID, float fLiveTime)
{
	bool bAddNew = false;
	for( long n=0; n<particles.e; n++ )
		if( particles.e[n].sfx != "" )
			if( AddHitAnimation( particles.e[n].sfx, particles.e[n].fOwnerVelocityFactor, worldPosition, grpID, nID | (n<<8), fLiveTime ) )
				bAddNew = true;
	return bAddNew;
}

void ShipPart::AddClothBurnSphere(const Vector& worldPosition)
{
	IClothBurns * pCBurns = GetShipOwner() ? GetShipOwner()->GetClothBurns() : null;
	if( pCBurns )
	{
		// только % взрывов рождают горение паруса
		if( 100.f * rand() / RAND_MAX < 100.f - pCBurns->GetExplosionBurnProbability() )
			return;
		Matrix m;
		GetMOOwner()->GetMatrix( m );
		pCBurns->AddBurnNotHosted( IClothBurns::group_explose, m.MulVertexByInverse(worldPosition), GetMOOwner() );
	}
}

void ShipPart::RestoreLiveTimeForHitGroup(unsigned short grpID)
{
	// для заданной группы, отсчет времени запускаем снова
	for (unsigned int i=0; i<hits_.Size(); i++)
		if( hits_[i].dwGroupID == grpID && hits_[i].nID != -1 && hits_[i].ps )
			hits_[i].fLiveTime = 0.f;
}

void ShipPart::DrawBroken()
{
	if( !physdata_.actor ) return;
	Matrix m(true);

	// определяем не упал ли кусок
	// уже довольно-таки глубоко под воду
	if( physdata_.active )
	{
		const float DeepLevel = -30.0f;
		bool deepUnderWater = false;
		((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,m);
		if (m.pos.y < DeepLevel)
			deepUnderWater = true;
		float alpha = m.pos.y;

		// если не слишком глубоко - рисуем
		if (!deepUnderWater)
		{
			if (alpha >= 0.0f)
				alpha = 1.0f;
			else
				alpha = 1.0f - alpha / DeepLevel;

			SetAlpha(alpha);
		}
		else // иначе мочим все это
		{
			physdata_.actor->Activate(false);
			physdata_.active = false;
			KillJoint( physdata_.actor );
		}
	}
	UpdateDamagedModelStatus();
}

void ShipPart::KillJoint(IPhysRigidBody* actor)
{
	if( brokenSystem_.Size() > 0 )
	{
		for( long n=0; n<brokenSystem_; n++ )
			if( brokenSystem_[n].actor == actor )
			{
				RELEASE( brokenSystem_[n].joint );
				if( n+1 < brokenSystem_ )
				{
					RELEASE( brokenSystem_[n+1].joint );
				}
				break;
			}
	}
	else if( parent_ )
		parent_->KillJoint( actor );
}

void ShipPart::DrawHits(const Matrix& parent)
{
	Matrix m;
	float fDeltaTime = GetShipOwner() ? GetShipOwner()->GetLastDeltaTime() : api->GetDeltaTime();

	// посчитаем матрицу для компенсации качания камеры
	Matrix diff(true);
	if( GetShipOwner() && GetShipOwner()->IsNoSwing() )
	{
		//Видовая матрица с включёной свинг матрицей
		Matrix view = GetMOOwner()->Render().GetView();
		//Чистая видовая матрица
		Matrix realView = GetMOOwner()->Mission().GetInverseSwingMatrix()*Matrix(view).Inverse();
		realView.Inverse();
		diff.EqMultiply(Matrix(realView), Matrix(view).Inverse());
	}
	else
		diff.SetIdentity();

	for (unsigned int i = 0; i < hits_.Size(); ++i)
	{
		if (hits_[i].ps && hits_[i].ps->IsAlive())
		{
			hits_[i].ps->CancelHide();
			// установим новую позицию для эмитера
			m.BuildPosition(hits_[i].pos);
			m *= parent;

			//hits_[i].ps->SetTransform(m);
			//Разносная матрица
			hits_[i].ps->SetTransform(Matrix().EqMultiply(m, diff));

			// ведем отсчет времени до уничтожения
			if( hits_[i].fMaxLiveTime > 0.f )
			{
				hits_[i].fLiveTime += fDeltaTime;
				if( hits_[i].fLiveTime >= hits_[i].fMaxLiveTime )
				{
					// если достигли до уничтожения, то сначала только запаузим
					// и добавим еще время жизни - чтоб партиклы могли доработать
					if( hits_[i].ps->IsActive() )
					{
						hits_[i].ps->PauseEmission(true);
						hits_[i].fLiveTime = hits_[i].fMaxLiveTime - 2.5f;
						// исключим их из специальных партиклов - пусть не мешают рожднию новых
						hits_[i].dwGroupID = hitgrp_common;
						hits_[i].nID = -1;
					}
					// а потом покилим все
					else
					{
						hits_[i].Release();
					}
				}
			}

			if( hits_[i].ps && hits_[i].dwGroupID == hitgrp_burn && GetShipOwner() )
			{
				GetShipOwner()->PartDestroyFlagSet( Ship::pdf_fire_hull );
			}
		}
	}
}

void ShipPart::DrawPart(const Matrix& parent)
{
	for (unsigned int i = 0;  i < GetChildren().Size(); ++i)
		GetChildren()[i]->DrawPart(parent);

	if (IsBroken())
		DrawBroken();
	else
		Draw(parent);

	DrawHits(parent);

	// debug draw
	//====================================================================
	/*Matrix m;
	Vector vShapeSize;
	if( physdata_.actor )
	{
		((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,m);
		((IPhysCombined*)physdata_.actor)->GetBox(0,vShapeSize);
		GetMOOwner()->Render().DrawBox( -vShapeSize, vShapeSize, m );
	}
	else if( GetShipOwner() && GetShipOwner()->GetPhysView() )
	{
		for( long n=0; n<shapes_; n++ )
		{
			GetShipOwner()->GetPhysView()->GetGlobalTransform( shapes_[n], m );
			GetShipOwner()->GetPhysView()->GetBox(shapes_[n],vShapeSize);
			GetMOOwner()->Render().DrawBox( -vShapeSize, vShapeSize, m, n==0 ? 0xFFFF0000 : 0xFF00FF00 );
		}
	}*/
	/*if( GetGMXEntity() )
	{
		GetMOOwner()->Render().DrawBox( GetGMXEntity()->GetBound().vMin, GetGMXEntity()->GetBound().vMax, GetGMXEntity()->GetWorldTransform(), 0xFFFF00FF );
	}*/
	//====================================================================
}

void ShipPart::DrawPartRefl(const Matrix& parent)
{
	for (unsigned int i = 0;  i < GetChildren().Size(); ++i)
		GetChildren()[i]->DrawPartRefl(parent);
}

void ShipPart::DoWaitFallToSea()
{
	if( !physdata_.active )
	{
		physdata_.bWaitFallToSea = false;
		return;
	}

	Matrix m(true);
	((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,m);

	if( m.pos.y < SeaHeight )
	{
		Vector vSize = 0.f;
		((IPhysCombined*)physdata_.actor)->GetBox(0,vSize);
		if( vSize.x > vSize.y && vSize.x > vSize.z )
			vSize.y = vSize.z = 0.f;
		else if( vSize.y > vSize.x && vSize.y > vSize.z )
			vSize.x = vSize.z = 0.f;
		else
			vSize.x = vSize.y = 0.f;

		Vector vBeg = m.MulVertex(vSize);
		Vector vEnd = m.MulVertex(-vSize);

		if( vBeg.y > SeaHeight )
			vBeg = m.pos + (SeaHeight - m.pos.y) / (vBeg.y - m.pos.y) * (vBeg-m.pos);

		if( vEnd.y > SeaHeight )
			vEnd = m.pos + (SeaHeight - m.pos.y) / (vEnd.y - m.pos.y) * (vEnd-m.pos);

		if( GetShipOwner() )
		{
			long nq = 1 + (long)((vEnd-vBeg).GetLength() / GetShipOwner()->GetShipPartWaterSplashStep());
			if( nq > 10 ) nq = 10;
			float fStep = 1.f / nq;
			for( long n=0; n<=nq; n++ )
			{
				Vector v = vBeg + n*fStep * (vEnd-vBeg);
				GetShipOwner()->CreateAutoParticle( GetShipOwner()->GetShipPartWaterSplashSFX(), Matrix().BuildPosition(v) );
			}
		}

		physdata_.bWaitFallToSea = false;
	}
}

void ShipPart::DoWork(const Matrix&)
{
	UpdateBoneTransform();

	if( physdata_.bWaitFallToSea )
		DoWaitFallToSea();

	if( physdata_.actor && physdata_.curDamping < physdata_.maxDamping )
	{
		Matrix mtx;
		((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,mtx);
		//physdata_.actor->GetTransform(mtx);
		if( mtx.pos.y < -5.f )
		{
			//physdata_.curDamping += physdata_.maxDamping * 0.5f * GetShipOwner()->GetLastDeltaTime();
			physdata_.curDamping = physdata_.maxDamping;
			physdata_.actor->SetMotionDamping( physdata_.curDamping );
		}
	}
}

void ShipPart::DoWorkChildren(const Matrix& parent)
{
	for (unsigned int i = 0;  i < GetChildren().Size(); ++i)
		GetChildren()[i]->Work(parent);
}

void ShipPart::Work(const Matrix& parent)
{
	if ( GetShipOwner() && GetShipOwner()->GetPhysView() )
		for (unsigned int i = 0; i < hits_.Size(); ++i)
			if ( hits_[i].ps && hits_[i].fOwnerVelocityFactor > 0.f )
				hits_[i].ps->AdditionalStartVelocity(hits_[i].fOwnerVelocityFactor*GetShipOwner()->GetPhysView()->GetLinearVelocity());

	DoWork(parent);
	DoWorkChildren(parent);
}

void ShipPart::SetBrokenSystem(const array<ShipPart::SystemItem> & system)
{
	// если была старая система разбитых частей, то удалим ее
	if( brokenSystem_.Size() > 0 )
		CleanBrokenSystem(true);
	// ставим новую систему
	brokenSystem_ = system;
}

void ShipPart::ReleaseHitParticles()
{
	for (unsigned int i = 0; i < hits_.Size(); ++i)
		hits_[i].Release();
	hits_.DelAll();
}

Matrix& ShipPart::GetInitMatrix(Matrix& mtx)
{
	if( poses_.Size()>0 )
	{
		mtx = poses_[0];
		return mtx;
	}
	mtx.SetIdentity();
	return mtx;
}

void ShipPart::UpdateBoneTransform()
{
	// неактивный корабль не рулит своими частями
	if( !GetMOOwner()->IsActive() )
		return;
	if( !GetShipOwner() )
		return;

	// незачем обновлять матрицу для убитой части
	if( physdata_.actor && !physdata_.active )
		return;

	Matrix mRoot(true);
	GetMOOwner()->GetMatrix( mRoot );
	Matrix m(true);

	// если играется анимация, то получаем положение кости из анимации
	// и размещаем физактера части в соответствии с этим положением
	if( m_nAnimationBone>=0 && GetShipOwner()->GetDestructAnimation() )
	{
		const Matrix & mbone = GetShipOwner()->GetDestructAnimation()->GetBoneMatrix(m_nAnimationBone);
		// !!! Не совсем правильно
		// Если позиция кости в скелете будет с поворотами по осям или отличаться по позиции от физического бокса, то
		// матрица будет неправильной, но искать сейчас Init матрицу скелета не буду (при тех скелетах что были до этого, это должно работать)
		GetInitMatrix(m);
		m.pos = 0.f;
		m *= mbone;
		m *= mRoot;
		if( physdata_.active && physdata_.actor )
		{
			((IPhysCombined*)physdata_.actor)->SetGlobalTransform(0,m);
		}
		else if( shapes_.Size()>0 && GetShipOwner()->GetPhysView() )
			GetShipOwner()->GetPhysView()->SetGlobalTransform(shapes_[0],m);

		return;
	}

	if( physdata_.active && physdata_.actor )
		((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,m);
	else if( shapes_.Size()>0 && GetShipOwner()->GetPhysView() )
		GetShipOwner()->GetPhysView()->GetGlobalTransform(shapes_[0],m);
	else
		m.SetIdentity();

	m = m * mRoot.Inverse();
}

void ShipPart::SetAlpha(float fAlpha)
{
}

Matrix& ShipPart::GetWorldTransform(Matrix& mtx)
{
	if( physdata_.actor )
	{
		((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,mtx);
	} else
	if( GetShipOwner() && GetShipOwner()->GetPhysView() && shapes_.Size()>0 )
	{
		//GetOwner()->GetPhysView()->GetGlobalTransform(shapes_[0],mtx);
		Matrix mLoc(true);
		Matrix mShip(true);
		GetShipOwner()->GetPhysView()->GetLocalTransform(shapes_[0],mLoc);
		mtx.EqMultiply( mLoc, GetMOOwner()->GetMatrix(mShip) );
	} else
	{
		mtx.SetIdentity();
	}
	return mtx;
}

void ShipPart::ShipPart_EnablePhysicActor(bool bEnable)
{
	// если есть разваливающиеся части, то временно отключим их симуляцию... ибо например
	// за время проирывания ролика фаталити эти части под действием силы тяжести улетят вниз
	// а хотелось бы вернуться к игре в то же состояние на котором мы перешли в ролик
	if( brokenSystem_.Size() > 0 )
	{
		for( long n=0; n<brokenSystem_; n++ )
		{
			if( brokenSystem_[n].actor && brokenSystem_[n].active )
			{
				if( bEnable )
				{
					//phis.actor->wakeUp();
					brokenSystem_[n].actor->Activate( true );
					brokenSystem_[n].actor->ApplyImpulse( brokenSystem_[n].vWakeVelocity * brokenSystem_[n].actor->GetMass(), Vector(0.f) );
				}
				else
				{
					//phis.actor->putToSleep();
					brokenSystem_[n].actor->Activate( false );
					brokenSystem_[n].vWakeVelocity = brokenSystem_[n].actor->GetLinearVelocity();
				}
			}
		}
	}

	// обработаем чилдренов
	for( int i=0; i<GetChildren(); i++ )
		GetChildren()[i]->ShipPart_EnablePhysicActor(bEnable);
}

void ShipPart::SetPhysTransform( const Matrix& mtx )
{
	if( physdata_.actor )
	{
		Matrix mShip(true);
		Matrix mLoc(true);
		((IPhysCombined*)physdata_.actor)->GetLocalTransform(0,mLoc);
		((IPhysCombined*)physdata_.actor)->SetTransform( mLoc.Inverse() * mtx * GetMOOwner()->GetMatrix(mShip) );
	} else
	if( GetShipOwner() && GetShipOwner()->GetPhysView() && shapes_.Size()>0 )
	{
		GetShipOwner()->GetPhysView()->SetLocalTransform( shapes_[0], mtx );
	}
}

void ShipPart::GetPhysTransform( Matrix& mtx )
{
	if( physdata_.actor )
	{
		Matrix mShip(true);
		Matrix mGlob(true);
		GetMOOwner()->GetMatrix(mShip);
		mShip.Inverse();
		((IPhysCombined*)physdata_.actor)->GetGlobalTransform(0,mGlob);
		mtx.EqMultiplyFast( mGlob, mShip );
	} else
	if( GetShipOwner() && GetShipOwner()->GetPhysView() && shapes_.Size()>0 )
	{
		GetShipOwner()->GetPhysView()->GetLocalTransform(shapes_[0],mtx);
	} else
	{
		mtx.SetIdentity();
	}
}

SeaMissionParams* ShipPart::GetSeaMParams()
{
	MOSafePointer safeptr;
	static const ConstString misParamsId("SeaMissionParams");
	if( GetMOOwner() )
		GetMOOwner()->FindObject(misParamsId,safeptr);
	else
		safeptr.Reset();
	if( safeptr.Ptr() && !safeptr.Ptr()->Is(misParamsId) )
		safeptr.Reset();
	return (SeaMissionParams*)safeptr.Ptr();
}

void ShipPart::DebugShowPhysForm()
{
	// показать свою физику
	if( physdata_.active && physdata_.actor )
	{
		IPhysCombined* pActor = (IPhysCombined*)physdata_.actor;
		Matrix mtx(true);
		Vector size;
		pActor->GetGlobalTransform(0,mtx);
		pActor->GetBox(0,size);
		dword col = 0xFF00FF00;
		if( GetMOOwner() )
			GetMOOwner()->Render().DrawBox(-size, size, mtx, col);
	}

	// показать инфу на детей
	for( long n=0; n<children_; n++ )
		children_[n]->DebugShowPhysForm();
}
