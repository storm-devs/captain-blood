#include "LittleManImpl.h"
#include "ShipPattern.h"

LittleManImpl::LittleManImpl(MissionObject* mo, const ShipContent::LittleMan& params, IMission& mission):
model_(NULL),
deathParts_(NULL),
alive_(true),
groups_(__FILE__, __LINE__),
mission_(mission),
animIndex_(0),
behaviourIndex_(0),
nodeIndex_(0),
animTime_(10.0f),
executeAnimation_(true),
hideShowFadeTime_(0.1f),
fade_(0.f),
alpha_(1.0f)
{
	localPos_ = params.pos;
	angles_ = params.angles;
	groups_ = params.groups;
	dieFlyAngle_ = params.explFlyDir;
	dieFlyDist_ = params.explFlyLength;
	dieFlySpeed_ = params.explFlySpeed;
	ballReactRadius_ = params.ballHitDist;
	hide_ = !params.show;

	Assert(mo);
	mo->FindObject( params.patternName, pattern_ );
	static const ConstString id_LittleManPattern("LittleManPattern");
	if ( pattern_.Ptr() && pattern_.Ptr()->Is(id_LittleManPattern) )
	{
		LittleManPattern* patptr = (LittleManPattern*)pattern_.Ptr();
		model_ = mission_.Geometry().CreateScene(	patptr->GetParams().model,
													&mission_.Animation(),
													&mission_.Particles(),
													&mission_.Sound(), __FILE__, __LINE__);

		m_rotateSpeed = Vector().RandXZ() * patptr->GetParams().dieRotateSpeed;

		if (model_)
		{
			model_->SetDynamicLightState(true);
			bool found = false;
			for (unsigned int i = 0; i < patptr->GetParams().anims.Size() && !found; ++i)
				for (unsigned int j = 0; j < patptr->GetParams().anims[i].behaviours.Size(); ++j)
					if( string::IsEqual(params.defaultBehaviour,patptr->GetParams().anims[i].behaviours[j].name) )
					//if (params.defaultBehaviour == patptr->GetParams().anims[i].behaviours[j].name)
					{
						animIndex_ = i;
						behaviourIndex_ = j;
						found = true;
						break;
					}

			if (patptr->GetParams().anims.Size())
			{
				model_->SetAnimationFile(patptr->GetParams().anims[animIndex_].animFileName);
				IAnimation* pAni = model_->GetAnimation();
				if ( pAni )
				{
					if (	patptr->GetParams().anims[animIndex_].behaviours.Size() &&
							patptr->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes.Size())
						pAni->Goto(	patptr->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes[0].name,
														patptr->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes[0].blendTime);
					pAni->Release();
				}
			}
		}
		else
		{
			alive_ = false;
		}

		const unsigned int partsCount = patptr->GetParams().dieParticles.Size();
		if ( partsCount )
		{
			dword nChoosedOne = rand() % partsCount;// (dword)(Rnd(partsCount-1.0f)+0.5f);
			if( patptr->GetParams().dieParticles[nChoosedOne] && patptr->GetParams().dieParticles[nChoosedOne][0] )
				deathParts_ = mission_.Particles().CreateParticleSystemEx( patptr->GetParams().dieParticles[nChoosedOne], __FILE__, __LINE__);
		}

		if (deathParts_)
		{
			deathParts_->PauseEmission(true);
			deathParts_->AutoHide(true);
		}
	}
	else
	{
		api->Trace("Warning! Can`t find sailor pattern: %s", params.patternName.c_str());
		pattern_.Reset();
	}
}

LittleManImpl::~LittleManImpl(void)
{
	if (model_)
		model_->Release(), model_ = NULL;
	if (deathParts_)
		deathParts_->Release(), deathParts_ = NULL;
}

void LittleManImpl::ChangeBehaviour(const char* behaviour)
{
	if (!pattern_.Ptr() || !model_)
	{
		api->Trace("Can`t change behaviour for sailor becouse pattern or model is null");
		return;
	}

	LittleManPattern* patptr = (LittleManPattern*)pattern_.Ptr();

	bool found = false;
	unsigned int oldAnimIndex = animIndex_;
	for (unsigned int i = 0; i < patptr->GetParams().anims.Size() && !found; ++i)
		for (unsigned int j = 0; j < patptr->GetParams().anims[i].behaviours.Size() && !found; ++j)
			if (string::IsEqual(behaviour,patptr->GetParams().anims[i].behaviours[j].name))
			{
				animIndex_ = i;
				behaviourIndex_ = j;
				found = true;
			}

	if (animIndex_ != oldAnimIndex)
		model_->SetAnimationFile(patptr->GetParams().anims[animIndex_].animFileName);

	if (patptr->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes.Size())
	{
		IAnimation* pAni = model_->GetAnimation();
		if ( pAni )
		{
			pAni->Goto(	patptr->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes[0].name,
						patptr->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes[0].blendTime );
			pAni->Release();

			executeAnimation_ = true;
			pAni->Pause( false );
		}
	}

	nodeIndex_ = 0;
}

void LittleManImpl::SetBehaviour(const char* group, const char* behaviour)
{
	if (!pattern_.Ptr() || !model_)
		return;

	for (unsigned int i = 0; i < groups_.Size(); ++i)
		if ( string::IsEqual(groups_[i],group) )
		{
			ChangeBehaviour(behaviour);
			break;
		}
}

void LittleManImpl::Hide(const char* group, bool hide, bool withFade)
{
	const float ShowHideFadeTime = 5.0f;
	for (unsigned int i = 0; i < groups_.Size(); ++i)
		if ( string::IsEqual(groups_[i],group) )
		{
			if (withFade)
				hideShowFadeTime_ = ShowHideFadeTime;
			else
				hideShowFadeTime_ = 0.1f;

			if (deathParts_)
				deathParts_->PauseEmission(true);

			hide_ = hide;
			break;
		}
}

void LittleManImpl::Work(const Matrix& parent, const Vector& vCamPos, float fDeltaTime)
{
	if (!pattern_.Validate() || !model_ || (hide_ && fade_ <= 0.0f) )
		return;

	float aniOffDistance = ((LittleManPattern*)pattern_.Ptr())->GetParams().animOffDistance;
	float AnimMinTime = ((LittleManPattern*)pattern_.Ptr())->GetParams().animMaxTime;
	float AnimMaxTime = AnimMinTime * 1.1f;
	AnimMinTime = AnimMinTime * 0.9f;

	worldPos_ = parent.MulVertex(localPos_);

	// дистация от камеры и флаги работы в зависимости от этой дальности
	float fCamDist2Pow = ~(worldPos_ - vCamPos);
	bool bExecuteAni = fCamDist2Pow < aniOffDistance*aniOffDistance; // выполнять/паузить анимацию
	bool bEnableShow = fCamDist2Pow < 160.f*160.f; // показывать/скрыть геометрию

	// паузим / распаузиваем анимацию человечка
	if( alive_ && executeAnimation_ != bExecuteAni )
	{
		IAnimation* pAni = model_->GetAnimation();
		if ( pAni )
		{
			pAni->Pause( executeAnimation_ );
			pAni->Release();
		}
		executeAnimation_ = bExecuteAni;
	}

	if (hide_)
	{
		model_->SetUserColor(Color(0.0f, Min(fade_/hideShowFadeTime_, 1.0f)*alpha_));
		fade_ -= fDeltaTime;
	}
	else
	{
		model_->SetUserColor(Color(0.0f, Min(fade_/hideShowFadeTime_, 1.0f)*alpha_));

		fade_ += fDeltaTime;
		if (fade_ > hideShowFadeTime_ ) fade_ = hideShowFadeTime_;
	}


	if (alive_)
	{
		if( bExecuteAni )
		{
			animTime_ -= fDeltaTime;
			if (animTime_ <= 0.0f)
			{
				++nodeIndex_;
				if (nodeIndex_ >= ((LittleManPattern*)pattern_.Ptr())->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes.Size())
					nodeIndex_ = 0;

				IAnimation* pAni = model_->GetAnimation();
				if ( pAni )
				{
					pAni->Goto(	((LittleManPattern*)pattern_.Ptr())->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes[nodeIndex_].name,
												((LittleManPattern*)pattern_.Ptr())->GetParams().anims[animIndex_].behaviours[behaviourIndex_].nodes[nodeIndex_].blendTime);
					pAni->Release();
				}

				animTime_ = AnimMinTime + ((AnimMaxTime-AnimMinTime)*rand())/RAND_MAX;
			}
		}
		if( bEnableShow )
		{
			model_->SetTransform(Matrix().Build(angles_, localPos_)*parent);
			model_->Draw();
		}
	}
	else if ( dieTrack_.GetPosition().y >= 0.0f)
	{
		dieTrack_.Step(fDeltaTime);
		if( deathParts_ )
			deathParts_->CancelHide();

		if (dieTrack_.GetPosition().y < 0.0f) // плюхнулись в воду
		{
			// партикл плюха
			mission_.Particles().CreateParticleSystemEx2( ((LittleManPattern*)pattern_.Ptr())->GetParams().waterHitParticles, Matrix().BuildPosition(dieTrack_.GetPosition()), true, __FILE__, __LINE__);
			if (deathParts_)
				deathParts_->PauseEmission(true);

			// звук плюха
			if( ((LittleManPattern*)pattern_.Ptr()) && ((LittleManPattern*)pattern_.Ptr())->GetParams().waterHitSound )
			{
				mission_.Sound().Create3D(((LittleManPattern*)pattern_.Ptr())->GetParams().waterHitSound, dieTrack_.GetPosition(), __FILE__, __LINE__);
			}
		}

		angles_ += m_rotateSpeed * fDeltaTime;
		model_->SetTransform(Matrix().Build(angles_, dieTrack_.GetPosition()));
		model_->Draw();
	}
}

void LittleManImpl::AttackBall(const Vector& hitPoint)
{
	if (!pattern_.Ptr() || !alive_)
		return;

	if ( (worldPos_-hitPoint).GetLength() < ballReactRadius_ )
		Kill();
}

void LittleManImpl::AttackMine(const Vector& explCenter, float radius)
{
	if (!pattern_.Ptr() || !alive_)
		return;

	if ( (worldPos_-explCenter).GetLength() < radius )
		Kill();
}

void LittleManImpl::Kill()
{
	if (deathParts_ && model_)
	{
		//FIXME: GetRoot() больше нет, надо искать локатор ручками через FindEntity
		//deathParts_->AttachTo(model_->GetRoot(), false);
		//deathParts_->PauseEmission(false);
	}
	LittleManPattern* ptrn = (LittleManPattern*)pattern_.Ptr();
	if( !ptrn ) return;
	if( ptrn && ptrn->GetParams().throwOutSound )
	{
		mission_.Sound().Create3D(ptrn->GetParams().throwOutSound, worldPos_, __FILE__, __LINE__);
	}
	const float flyDist = dieFlyDist_;
	const float flySpeed = dieFlySpeed_ * (1.f + 0.25f*rand()/(RAND_MAX-1.0f));
	Vector endPoint = worldPos_+Vector(0,0,flyDist).Rotate(dieFlyAngle_);
	endPoint.y = 0.0f;
	dieTrack_.SetTrajectoryParams(worldPos_, endPoint, flySpeed);

	alive_ = false;
	ChangeBehaviour(ptrn->GetParams().explosionBehaviour);
}

//////////////////////////////////////////////////////////////////////////

bool LittleManPattern::Create(MOPReader & reader)
{
	bool bOk = EditMode_Update(reader);

	return bOk;
}

bool LittleManPattern::EditMode_Update(MOPReader & reader)
{
	Params params;
	params.model = reader.String().c_str();
	unsigned int animsCount = reader.Array();
	for (unsigned int i = 0; i < animsCount; ++i)
	{
		Params::Animation anim;

		anim.animFileName = reader.String().c_str();

		unsigned int behavioursCount = reader.Array();
		for (unsigned int j = 0; j < behavioursCount; ++j)
		{
			Params::Animation::Behaviour behaviour;

			behaviour.name = reader.String().c_str();

			unsigned int nodesCount = reader.Array();
			for (unsigned int k = 0; k < nodesCount; ++k)
			{
				Params::Animation::Behaviour::Node node;
				node.name = reader.String().c_str();
				node.blendTime = reader.Float();	

				behaviour.nodes.Add(node);
			}
			anim.behaviours.Add(behaviour);
		}

		params.anims.Add(anim);
	}
	params.explosionBehaviour = reader.String().c_str();

	unsigned int count = reader.Array();
	for (unsigned int i = 0; i < count; ++i)
		params.dieParticles.Add(reader.String().c_str());
	params.dieRotateSpeed = reader.Float() * (PIm2 / 360.f);

	params.waterHitParticles = reader.String().c_str();

	params.waterHitSound = reader.String().c_str();
	params.throwOutSound = reader.String().c_str();

	params.animOffDistance = reader.Float();
	params.animMaxTime = reader.Float();

	params_ = params;

	if ( owningPattern_.Validate() )
		((ShipPattern*)owningPattern_.Ptr())->NotifyShipObjects(ShipPattern::ParamChanged);
	return true;
}

MOP_BEGINLISTCG(LittleManPattern, "Sailor Pattern", '1.00', 99, "-= Sailor pattern =-", "Arcade Sea");
	MOP_STRING("Model", "")

	MOP_ARRAYBEG("Animation", 1, 10)
		MOP_STRING("Animation file", "")
		MOP_ARRAYBEG("Behaviours", 1, 10)
			MOP_STRING("Behaviour name", "")
			MOP_ARRAYBEG("Behaviour", 1, 10)
				MOP_STRING("Node name", "")
				MOP_FLOAT("Blend time", 2)
			MOP_ARRAYEND
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_STRING("Explosion behaviour", "")
	MOP_ARRAYBEG("Die particles", 0, 10)
		MOP_STRING("Name", "")
	MOP_ARRAYEND
	MOP_FLOATEXC("Die rotate speed", 40.f, 0.f, 360.f, "Скорость вращения при полете после взрыва")
	MOP_STRING("Water hit particles", "")
	MOP_STRING("Water hit sound", "")
	MOP_STRING("Throw out sound", "")
	MOP_FLOATEX("Animation off distance", 50.f, 50.f, 500.f)
	MOP_FLOATEXC("Animation change time", 10.f, 5.f, 60.f, "Время после которого происходит смена анимации (нужно для рандомизации движения матросиков)")

MOP_ENDLIST(LittleManPattern)