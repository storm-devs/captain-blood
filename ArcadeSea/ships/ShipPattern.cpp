#include "..\pch.h"
#include "ShipPattern.h"
#include "Ship.h"

ShipPattern::ShipPattern(void) :
registry_(__FILE__, __LINE__)
{
	m_nEditLittleManIndex = -1;
	mainModel_ = NULL;
}

ShipPattern::~ShipPattern(void)
{
	NotifyShipObjects(PatternRemoved);

	dword n;

	for( n=0; n<4; n++ )
		pattern_.sides_[n].ClearWeapons();

	pattern_.m_Lods.DelAll();
	RELEASE(mainModel_);

	for (unsigned int i = 0; i < pattern_.cookedSails_.Size(); ++i)
	{
		RELEASE( pattern_.cookedSails_[i] );
	}
	pattern_.cookedSails_.DelAll();
}

// регистрирует корабль в шаблоне
void ShipPattern::RegisterShipObject(Ship* ship)
{
	if (registry_.Find(ship) == INVALID_ARRAY_INDEX)
		registry_.Add(ship);
}

// отменяет регистрацию корабля в шаблоне
void ShipPattern::UnregisterShipObject(Ship* ship)
{
	registry_.Del(ship);
}

// оповещает все зарегистрированные корабли об изменениях шаблона
void ShipPattern::NotifyShipObjects(Notification type) const
{
	unsigned int count = registry_.Size();
	for (unsigned int i = 0; i < count; ++i)
		registry_[i]->OnNotify(type);
}

void ShipPattern::BuildSail(const Sail::Params& params, unsigned int index)
{
	// создаем главный парус
	IPhysics * physService = (IPhysics*)api->GetService("PhysicsService");
	Assert(physService);
	IClothMeshBuilder * builder = physService->CreateClothMeshBuilder();

	Matrix pose = Matrix().Build(params.orient, params.pos);
	Vector points[4];
	for (unsigned int i = 0; i < params.pointsCount; ++i)
		points[i] = pose.MulVertex(params.points[i]);

	unsigned int sailIndex = 0;

	if (params.pointsCount == 4)
		sailIndex = builder->AddSail( points, params.widthPointsCount, params.heightPointsCount, params.windCurvature, params.bottomCurvature);
	else
		sailIndex = builder->AddTriangle( points, params.widthPointsCount);

	// прицепляем к парусу веревки
	Matrix ropePose;
	for (unsigned int i = 0; i < params.ropes.Size(); ++i)
	{
		unsigned int ropeIndex = builder->AddRope( params.ropes[i].begin, params.ropes[i].end, params.ropes[i].thickness);

		sailIndex = builder->Connect(sailIndex, ropeIndex, params.ropes[i].begin);
	}

	string sailBinName;
	sailBinName.Format("%s_%s_%d.pxccm", Mission().GetMissionName(), GetObjectID().c_str(), index);
	bool bBuildSail = false;
	if( !EditMode_IsOn() )
	{
		bBuildSail = builder->Load(Physics(), Mission(), sailBinName);
		if( !bBuildSail )
			api->Trace("Error! Can`t load cooked mesh for sail %s (check file: shippattern\\%s.pxm)",sailBinName.GetBuffer(),Mission().GetMissionName());
	}
	else
		bBuildSail = builder->Build(Physics(), true, &Mission(), sailBinName);

	// если не был построен парус, то мы удаляем билдер за ненадобностью
	if( !bBuildSail )
	{
		builder->Release();
		pattern_.cookedSails_.Add(0);
	}
	else
		pattern_.cookedSails_.Add(builder);
}

bool ShipPattern::ReadMOPString(MOPReader & reader,string & sStr)
{
	const char* pcNewStr = reader.String().c_str();
	bool bNotEqual = (sStr != pcNewStr);
	sStr = pcNewStr;
	return bNotEqual;
}

// чтение параметров 
void ShipPattern::ReadMOPs(MOPReader & reader)
{
	array<Mast::Params> aOldMasts(_FL_);
	aOldMasts = pattern_.masts_;

	pattern_.hull_.DelAll();
	pattern_.masts_.DelAll();
	pattern_.ropes_.DelAll();

	vPos_ = reader.Position();

	bDrawHull_ = reader.Bool();
	bDrawMasts_ = reader.Bool();
	bDrawRopes_ = reader.Bool();
	bDrawSides_ = reader.Bool();
	bDrawShootSmokeSFX_ = reader.Bool();
	m_bDrawSailors = reader.Bool();
	m_bDrawBurnPoints = reader.Bool();

	pattern_.pcBreakAnimation = reader.String().c_str();

	pattern_.m_fSFXLimitationDistance = reader.Float();

	pattern_.bUsedByStatistics = reader.Bool();
	pattern_.bIsBoat = reader.Bool();

	// читаем параметры лодов
	ReadLodsParams(reader);

	// если имя основной модели не изменилось, то не меняем анимацию и т.д. (для оптимизации времени на изменения паттерна, ибо тормозит и так все очень сильно)
	bool bReloadModel = pattern_.m_Lods.Size()==0 || pattern_.modelName_ != pattern_.m_Lods[0].mainModelName;
	pattern_.modelName_ = pattern_.m_Lods.Size()==0 ? "" : pattern_.m_Lods[0].mainModelName;

	// чтение физико-геометрических параметров
	pattern_.m_cModelColor = reader.Colors();
	pattern_.massBoxCenter_ = reader.Position();
	pattern_.massBoxBounds_ = reader.Position();

	pattern_.boundsCenter_ = reader.Position();
	pattern_.boundsSize_ = reader.Position();

	pattern_.vCenterMass_ = reader.Position();
	pattern_.fMass_ = reader.Float();
	pattern_.fWaterLineHeight_ = reader.Float();
	pattern_.fForwardSlopeForce_ = reader.Float();
	pattern_.fBackwardSlopeForce_ = reader.Float();
	pattern_.fBackAlongSlopeScale_ = reader.Float();
	pattern_.fLinearFrictionForce_ = reader.Float();
	pattern_.fAngularFrictionForce_ = reader.Float();
	pattern_.fFloatForce_ = reader.Float();
	pattern_.fWaveness_ = reader.Float();

	IAnimationProcedural* pAni = NULL;
	if( bReloadModel )
	{
		IGMXScene* pOldModel = mainModel_;
		mainModel_ = (pattern_.m_Lods.Size()>0 && pattern_.m_Lods[0].mainModelName!="") ?
			Geometry().CreateScene(pattern_.m_Lods[0].mainModelName, &Animation(), &Particles(), &Sound(), _FL_) :
			NULL;
		// set transform animation
		if (mainModel_)
		{
			mainModel_->SetDynamicLightState(true);
			IShipAnimationMaker::CreateProcedureAnimation(mainModel_,pAni,0);
			if( pAni )
			{
				mainModel_->SetAnimation( pAni );
				// декремент рефкаунтера - теперь только моделька держит эту анимацию
				pAni->Release();
			}
		}
		RELEASE(pOldModel);
	}

	boneDataReadOnly* pBoneData = NULL;
	boneMtxInputReadOnly* pBoneTransformations = NULL;
	dword dwBoneQ = 0;
	if( mainModel_ )
		dwBoneQ = mainModel_->GetBonesArray(&pBoneData,&pBoneTransformations);

	float fExplodeImpulsePower = reader.Float();
	pattern_.nDamageHullRecurseLevel = reader.Long();

	unsigned int arraySize = 0;
	// чтение HullParts
	pattern_.hull_.DelAll();
	arraySize = reader.Array();
	for (unsigned int i = 0; i < arraySize; ++i)
	{
		HullPart::Params part;

		part.modelName_ = reader.String().c_str();
		part.modelDamagedName_ = reader.String().c_str();

		if( bReloadModel && pAni )
		{
			if( bDrawHull_ )
			{
				long idx = pAni->FindBone(part.modelDamagedName_,true);
				if (idx >= 0) pAni->CollapseBone( idx );

				dword nb = pAni->FindBone(part.modelName_,true);
				if( nb>=0 && nb<dwBoneQ )
				{
					Matrix mtmp(pBoneTransformations[nb].mtxBindPose);
					pAni->SetBoneMatrix( nb, mtmp.Inverse() );
				}
			}
			else
			{
				long idx = pAni->FindBone(part.modelName_,true);
				if (idx >= 0) pAni->CollapseBone( idx );

				idx = pAni->FindBone(part.modelDamagedName_,true);
				if (idx >= 0) pAni->CollapseBone( idx );
			}
		}

		part.ballHitSfx_.ReadMOPs( reader );
		part.flamethrowerHitSfx_ = reader.String().c_str();
		part.burnParam_.ReadMOPs(reader);
		part.breakSfx_.ReadMOPs(reader);

		part.breakSfxPos_ = reader.Position();
		part.hp_ = reader.Float();
		part.shipDamageCoef_ = reader.Float();

		part.afterBreakHP_ = reader.Float();
		unsigned int damageReceiverCount = reader.Array();
		for( unsigned int k = 0; k<damageReceiverCount; k++ )
		{
			unsigned nReceiver = part.damageReceive_.Add();
			part.damageReceive_[nReceiver].nPart = reader.Long();
			part.damageReceive_[nReceiver].fDamage = reader.Float();
		}
		part.touchDamageFactor_ = reader.Float() * 0.01f;
		part.touchSfxPos_ = reader.Position(); // позиция эффекта столкновения
		part.touchSfx_ = reader.String().c_str(); // эффект столкновения

		unsigned int burnPointsCount = reader.Array();
		for (unsigned int k = 0; k < burnPointsCount; ++k)
		{
			part.burnPoints_.Add(reader.Position());
		}
		part.m_fBurnLiveTime = reader.Float();
		part.m_fBurnLiveTimeRandomAdd = reader.Float();

		burnPointsCount = reader.Array();
		for (k = 0; k < burnPointsCount; k++)
		{
			part.burnFromFlamethrowerPoints_.Add(reader.Position());
		}

		unsigned int primCount = reader.Array();
		for (unsigned int k = 0; k < primCount; ++k)
		{
			ShipBox b;
			b.ReadMOPs(reader);
			part.form_.boxes_.Add(b);
		}

		part.fExplodeImpulsePower = fExplodeImpulsePower;

		pattern_.hull_.Add(part);
	}
	pattern_.shipExplosionParticles_.ReadMOPs(reader);
	pattern_.shipExplosionSound_ = reader.String().c_str();

	// чтение ShipSides
	reader.Array(); // всегда 4
	for (unsigned int i = 0; i < 4; ++i)
	{
		pattern_.sides_[i].SetAniModel(reader.String().c_str());

		// чтение бортовых орудий
		unsigned int count = reader.Array();
		pattern_.sides_[i].ClearWeapons();
		pattern_.sides_[i].SetOwner( this );
		for (unsigned int k = 0; k < count; ++k)
		{
			SideWeapon weapon(this);
			weapon.SetPatternName(reader.String());
			weapon.SetPosition(reader.Position());
			weapon.SetDirAngle(Deg2Rad(reader.Float()));
			weapon.SetAnimBoneName(reader.String().c_str());

			pattern_.sides_[i].AddWeapon(weapon);
		}

		pattern_.sides_[i].SetGrouping(reader.Float());
		pattern_.sides_[i].SetSmokeSFX(reader.String().c_str());

		// чтение эмиттеров дыма после стрельбы
		count = reader.Array();
		pattern_.sides_[i].ClearSmokePoints();
		for (unsigned int k = 0; k < count; ++k)
			pattern_.sides_[i].AddSmokePoint(reader.Position());
	}

	// чтение Masts
	arraySize = reader.Array();
	pattern_.masts_.DelAll();
	for (unsigned int i = 0; i < arraySize; ++i)
	{
		Mast::Params mast;

		// чтение парусов
		unsigned int sailCount = reader.Array();
		for (unsigned int j = 0; j < sailCount; ++j)
		{
			Sail::Params sail;
			sail.pointsCount = reader.Array();
			for (unsigned int k = 0; k < sail.pointsCount; ++k)
				sail.points[k] = reader.Position();

			sail.widthPointsCount = reader.Long();
			sail.heightPointsCount = reader.Long();

			unsigned int sailRopesCount = reader.Array();
			for (unsigned int k = 0; k < sailRopesCount; ++k)
			{
				Sail::Params::Rope rope;
				rope.begin		= reader.Position();
				rope.end		= reader.Position();
				rope.thickness	= reader.Float();
				sail.ropes.Add(rope);
			}

			sail.orient = reader.Angles();
			sail.pos = reader.Position();

			sail.tearFactor = reader.Float();

			sail.texture = reader.String().c_str();
			sail.glimpseTexture = reader.String().c_str();
			sail.color = reader.Colors();
			sail.glimpseColor = reader.Colors();
			sail.colorScale = reader.Float();
			
			sail.fakeAnimAmplitude= reader.Float();
			sail.xFreq = reader.Float();
			sail.yFreq = reader.Float();
			sail.windCurvature = reader.Float();
			sail.bottomCurvature = reader.Float();
			mast.sails_.Add(sail);
		}
		
		mast.hitSFX_.ReadMOPs( reader );
		mast.shipDamageCoef_ = reader.Float();
		mast.isRotatable_ = reader.Bool();
		mast.rotationAxisBegin_ = reader.Position();
		mast.rotationAxisEnd_ = reader.Position();

		// кол-во частей мачты
		unsigned int partsCount = reader.Array();
		for (unsigned int n = 0; n < partsCount; ++n)
		{
			MastPart::Params part;

			// чтение рей
			unsigned int count = reader.Array();
			for (unsigned int k = 0; k < count; ++k)
			{
				Yard::Params yard;

				yard.modelName_ = reader.String().c_str();

				if( bReloadModel && pAni )
					if( bDrawMasts_ )
					{
						dword nb = pAni->FindBone(yard.modelName_,true);
						if( nb>=0 && nb<dwBoneQ )
						{
							Matrix mtmp(pBoneTransformations[nb].mtxBindPose);
							pAni->SetBoneMatrix( nb, mtmp.Inverse() );
						}
					}
					else
					{
						long idx = pAni->FindBone(yard.modelName_,true);
						if (idx >= 0) pAni->CollapseBone( idx );
					}

				yard.hitSFX_.ReadMOPs( reader );

				yard.hp_ = reader.Float();
				yard.shipDamageCoef_ = reader.Float();

				unsigned int primCount = reader.Array();
				for (unsigned int l = 0; l < primCount; ++l)
				{
					ShipBox b;
					b.ReadMOPs(reader);
					yard.form_.boxes_.Add(b);
				}

				yard.bNotRotable = reader.Bool();

				part.yards_.Add(yard);
			}

			part.modelName_ = reader.String().c_str();


			if( bReloadModel && pAni )
				if( bDrawMasts_ )
				{
					dword nb = pAni->FindBone(part.modelName_,true);
					if( nb>=0 && nb<dwBoneQ )
					{
						Matrix mtmp(pBoneTransformations[nb].mtxBindPose);
						pAni->SetBoneMatrix( nb, mtmp.Inverse() );
					}
				}
				else
				{
					long idx = pAni->FindBone(part.modelName_,true);
					if (idx >= 0) pAni->CollapseBone( idx );
				}

			part.hp_ = reader.Float();
			
			unsigned int primCount = reader.Array();
			for (unsigned int k = 0; k < primCount; ++k)
			{
				ShipBox b;
				b.ReadMOPs(reader);
				part.form_.boxes_.Add(b);
			}
			mast.partsDesc_.Add(part);
		}

		pattern_.masts_.Add(mast);
	}
	pattern_.m_fDamageFromMastBroke = reader.Float();

	// чтение веревок
	pattern_.ropes_.DelAll();
	
	// чтение человечков
	MOSafePointer safepointer;
	FindObject( GetObjectID(), safepointer );
	arraySize = reader.Array();
	bool bFindEditLittleMan = true;
	if( pattern_.littleMen_.Size() != arraySize )
	{
		bFindEditLittleMan = false;
		m_nEditLittleManIndex = -1;
		pattern_.littleMen_.DelAll();
		if( arraySize > 0 )
			pattern_.littleMen_.AddElements(arraySize);
	}
	for (unsigned int i = 0; i < arraySize; ++i)
	{
		ShipContent::LittleMan & man = pattern_.littleMen_[i];

		bool bChangeParam = false;
		man.pos = reader.Position();
		man.angles = reader.Angles();
		ChangeFlag_Float( Deg2Rad(reader.Float()), man.explFlyDir, bChangeParam );
		ChangeFlag_Float( reader.Float(), man.explFlyLength, bChangeParam );
		ChangeFlag_Float( reader.Float(), man.explFlySpeed, bChangeParam );
		ChangeFlag_Float( reader.Float(), man.ballHitDist, bChangeParam );
		man.patternName = reader.String();

		MOSafePointer mo;
		FindObject(man.patternName, mo);
		static const ConstString typeId("LittleManPattern");
		if (mo.Ptr() && mo.Ptr()->Is(typeId))
		{
			((LittleManPattern*)mo.Ptr())->SetOwningPattern(safepointer);
		}

		unsigned int groupsCount = reader.Array();
		if( man.groups.Size() != groupsCount )
		{
			man.groups.DelAll();
			if( groupsCount > 0 )
				man.groups.AddElements(groupsCount);
		}
		for (unsigned int nGroup = 0; nGroup < groupsCount; nGroup++)
			man.groups[nGroup] = reader.String().c_str();

		man.defaultBehaviour = reader.String().c_str();
		man.show = reader.Bool();

		if( bFindEditLittleMan && bChangeParam )
		{
			bFindEditLittleMan = false;
			m_nEditLittleManIndex = i;
		}
	}

	// падение части корабля в воду
	pattern_.m_pcShipPartWaterSplash = reader.String().c_str();
	pattern_.m_fShipPartWaterSplashStep = reader.Float();
	pattern_.m_pcShipBoneWaterSplash = reader.String().c_str();

	// звуки
	pattern_.pSound_[ShipContent::se_hull_destroy] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_ship_not_moved] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_ship_moved] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_ship_fire] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_ship_touched] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_ship_knock] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_sail_damage] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_sail_fire] = reader.String().c_str();
	pattern_.pSound_[ShipContent::se_ship_hit] = reader.String().c_str();

	pattern_.fShipMoveFadeTime_ = reader.Float();
	if( pattern_.fShipMoveFadeTime_ <= 0.1f )
		pattern_.fShipMoveFadeTime_ = 0.1f;

	// евенты на выстрел (обычный и мощный)
	pattern_.cannonFireBase.Init( reader );
	pattern_.cannonFireMight.Init( reader );
	// набор источников освещения для выстрелов
	pattern_.fExcludeLightRadius = reader.Float();
	pattern_.fLightOffset = reader.Float();
	int nLightsQ = reader.Array();
	pattern_.aWeaponLights.DelAll();
	if( nLightsQ > 0 )
	{
		pattern_.aWeaponLights.AddElements( nLightsQ );
		for( long nl=0; nl<pattern_.aWeaponLights; nl++ )
		{
			pattern_.aWeaponLights[nl].fLifeTime = reader.Float();
			pattern_.aWeaponLights[nl].pcLightObjID = reader.String();
		}
	}

	// переделываем паруса если они поменялись
	if( CheckSailChange( aOldMasts, pattern_.masts_ ) )
	{
		aOldMasts.DelAll();
		for (unsigned int i = 0; i < pattern_.cookedSails_.Size(); ++i)
		{
			RELEASE( pattern_.cookedSails_[i] );
		}
		pattern_.cookedSails_.DelAll();

		for( long i=0; i<pattern_.masts_; i++ )
			for (long j=0; j < pattern_.masts_[i].sails_; j++)
			{
				BuildSail(pattern_.masts_[i].sails_[j], i*1000 + j);
			}
	}

	NotifyShipObjects(ParamChanged);
}

bool ShipPattern::CheckSailChange(array<Mast::Params> & oldMasts, array<Mast::Params> & newMasts)
{
	if( oldMasts.Size() != newMasts.Size() ) return true;
	for( long n=0; n<oldMasts; n++ )
	{
		if( oldMasts[n].sails_.Size() != newMasts[n].sails_.Size() )
			return true;
		for( long i=0; i<oldMasts[n].sails_; i++ )
		{
			Sail::Params & s1 = oldMasts[n].sails_[i];
			Sail::Params & s2 = newMasts[n].sails_[i];

			if( s1.ropes.Size() != s2.ropes.Size() ) return true;
			for( long r=0; r<s1.ropes; r++ )
			{
				if( s1.ropes[r].begin != s2.ropes[r].begin ) return true;
				if( s1.ropes[r].end != s2.ropes[r].end ) return true;
				if( s1.ropes[r].thickness != s2.ropes[r].thickness ) return true;
			}

			if( s1.heightPointsCount != s2.heightPointsCount ) return true;
			if( s1.widthPointsCount != s2.widthPointsCount ) return true;
			if( s1.orient != s2.orient ) return true;
			if( s1.pos != s2.pos ) return true;
			if( s1.pointsCount != s2.pointsCount ) return true;
			for( unsigned int j=0; j<s1.pointsCount; j++ )
				if( s1.points[j] != s2.points[j] )
					return true;
			if( s1.windCurvature != s2.windCurvature ) return true;
			if( s1.bottomCurvature != s2.bottomCurvature ) return true;
		}
	}
	return false;
}

void ShipPattern::ReadLodsParams(MOPReader & reader)
{
	pattern_.m_Lods.DelAll();

	pattern_.fShipLodFadeDist = reader.Float();
	pattern_.fShipLodFadeLength = reader.Float();

	dword q = reader.Array();
	if( q>0 )
		pattern_.m_Lods.AddElements(q);
	for(dword n=0; n<q; n++)
	{
		// читаем имена моделек для лодов
		pattern_.m_Lods[n].mainModelName = reader.String().c_str();
		pattern_.m_Lods[n].reflModelName = reader.String().c_str();
		pattern_.m_Lods[n].refrModelName = reader.String().c_str();
		// читаем дистанцию до лода
		float fdist = reader.Float();
		pattern_.m_Lods[n].fDist2Pow = fdist * fdist;
	}

	// первый лод включается всегда (расстояние = 0.f)
	Assert( pattern_.m_Lods.Size()!=0 );
	pattern_.m_Lods[0].fDist2Pow = 0.f;
}

// Создание объекта
bool ShipPattern::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	return true;
}

bool ShipPattern::EditMode_Create(MOPReader & reader)
{
	EditMode_Update(reader);
	SetUpdate(&ShipPattern::Draw, ML_GEOMETRY1-10);
	return true;
}

// Обновление параметров
bool ShipPattern::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

// Инициализация 
bool ShipPattern::Init()
{
	return true;
}

void ShipPattern::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = pattern_.boundsCenter_ - 0.5f*pattern_.boundsSize_;
	max = pattern_.boundsCenter_ + 0.5f*pattern_.boundsSize_;
}

Matrix & ShipPattern::GetMatrix(Matrix & mtx)
{
	mtx.BuildPosition(vPos_);
	return mtx;
}

void ShipPattern::VisualizePhysForm(const PhysForm& form)
{
	Matrix m, t;
	
	for (unsigned int i = 0; i < form.boxes_.Size(); ++i)
	{
		Vector a = form.boxes_[i].angles;
		m.Build(form.boxes_[i].angles, form.boxes_[i].center);
		t.BuildPosition(vPos_);
		m = m * t;

		Render().DrawBox(	-form.boxes_[i].size/2.0,
							 form.boxes_[i].size/2.0, m,
							0xFFFF0000);
	}
}


// отрисовка в редакторе
void _cdecl ShipPattern::Draw(float fDeltaTime, long level)
{
	const float lineEndMarkerSize = 0.1f;
	Matrix m;
	m.BuildPosition(vPos_);
	if ( mainModel_ )
	{
		mainModel_->SetTransform(m);
		mainModel_->Draw();
	}

	if (bDrawHull_)
	{
		unsigned int count = pattern_.hull_.Size();
		for (unsigned int i = 0; i < count; ++i)
		{
			VisualizePhysForm(pattern_.hull_[i].form_);

			// показать места рождения огня
			if( m_bDrawBurnPoints )
			{
				for( dword n=0; n<pattern_.hull_[i].burnPoints_.Size(); n++ )
				{
					Vector v = vPos_ + pattern_.hull_[i].burnPoints_[n];
					Render().DrawSphere( v, 0.5f, 0xFFFF0000 );
					Render().DrawLine( v, 0xFF00FF00, v + Vector(0.f, 2.f, 0.f), 0xFFFF0000 );
				}

				for( dword n=0; n<pattern_.hull_[i].burnFromFlamethrowerPoints_.Size(); n++ )
				{
					Vector v = vPos_ + pattern_.hull_[i].burnFromFlamethrowerPoints_[n];
					Render().DrawSphere( v, 0.5f, 0xFF0000FF );
					Render().DrawLine( v, 0xFF00FF00, v + Vector(0.f, 2.f, 0.f), 0xFFFF0000 );
				}
			}
		}
	}

	if (bDrawMasts_)
	{
		unsigned int count = pattern_.masts_.Size();
		for (unsigned int i = 0; i < count; ++i)
		{
			for (unsigned int k = 0; k < pattern_.masts_[i].partsDesc_.Size(); ++k)
			{
				VisualizePhysForm(pattern_.masts_[i].partsDesc_[k].form_);
			
				unsigned int yardsCount = pattern_.masts_[i].partsDesc_[k].yards_.Size();
				for (unsigned int l = 0; l < yardsCount; ++l)
				{
					VisualizePhysForm(pattern_.masts_[i].partsDesc_[k].yards_[l].form_);
				}
			}

			// рисуем ось вращения мачты
			if (pattern_.masts_[i].isRotatable_)
			{
				Render().DrawLine(	pattern_.masts_[i].rotationAxisBegin_ + vPos_, 0xFF0FF0FF,
									pattern_.masts_[i].rotationAxisEnd_ + vPos_, 0xFF0FF0FF);

				const unsigned int tessFactor = 10;
				const Vector v = vPos_ + (pattern_.masts_[i].rotationAxisBegin_ + pattern_.masts_[i].rotationAxisEnd_)/2.0f;
				const float r = 2.0f;
				
				for (unsigned int i = 0; i < tessFactor; ++i)
				{
					Render().DrawLine(	v+Vector(r*sinf(2*PI*i/tessFactor), 0, r*cosf(2*PI*i/tessFactor)), 0xFF0FF0FF,
										v+Vector(r*sinf(2*PI*(i+1)/tessFactor), 0, r*cosf(2*PI*(i+1)/tessFactor)), 0xFF0FF0FF);
				}
			}
			

			for (unsigned int k = 0; k < pattern_.masts_[i].sails_.Size(); ++k)
			{
				Sail::Params & p = pattern_.masts_[i].sails_[k];
				
				for (unsigned int i = 0; i < p.pointsCount; ++i)
				{
					Render().DrawSphere(vPos_+p.points[i], lineEndMarkerSize, 0xFF00FF00);
				}

				for (unsigned int i = 0; i < p.ropes.Size(); ++i)
				{
					Vector v1, v2;

					v1 = p.ropes[i].begin;
					v2 = p.ropes[i].end;

					Render().DrawSphere(vPos_+v1, lineEndMarkerSize, 0xFF0000FF);
					Render().DrawSphere(vPos_+v2, lineEndMarkerSize, 0xFF0000FF);
					Render().DrawLine(vPos_+v1, 0xFF0000FF, vPos_+v2, 0xFF0000FF);
				}
			}
		}
	}

	if (bDrawRopes_)
	{
		unsigned int count = pattern_.ropes_.Size();
		for (unsigned int i = 0; i < count; ++i)
		{
			Vector v1, v2;

			v1 = pattern_.ropes_[i].pos_[0];
			v2 = pattern_.ropes_[i].pos_[1];

			Render().DrawSphere(vPos_+v1, lineEndMarkerSize, 0xFF0000FF);
			Render().DrawSphere(vPos_+v2, lineEndMarkerSize, 0xFF0000FF);
			Render().DrawLine(vPos_+v1, 0xFF0000FF, vPos_+v2, 0xFF0000FF);
		}
	}

	if (bDrawSides_)
	{
		for (unsigned int i = 0; i < sizeof(pattern_.sides_)/sizeof(pattern_.sides_[0]); ++i)
		{
			
			unsigned int emCount = pattern_.sides_[i].GetSmokePointsCount();
			
			for (unsigned int k = 0; k < emCount; ++k)
			{
				Matrix m;
                m.BuildPosition(vPos_+pattern_.sides_[i].GetSmokePoint(k));

				Render().DrawSphere(vPos_+pattern_.sides_[i].GetSmokePoint(k), 0.5f, 0xFFFFFF00);

				if (bDrawShootSmokeSFX_)
				{
					IParticleSystem* ps = Particles().CreateParticleSystemEx(pattern_.sides_[i].GetSmokeSFX(), __FILE__, __LINE__);
					ps->AutoDelete(true);
					ps->SetTransform(m);
				}
			}

			Matrix old = Render().GetWorld();
			Matrix m;
			Vector arrow[] = { Vector(0,0,2), Vector(0.5f,0,0), Vector(-0.5f,0,0) };
			Vector poly[3];

			for (unsigned int k = 0; k < pattern_.sides_[i].GetWeaponCount(); ++k)
			{
				m.Build(Vector(0,pattern_.sides_[i].GetWeapon(k).GetDirAngle(),0), vPos_ + pattern_.sides_[i].GetWeapon(k).GetPosition());
				poly[0] = m.MulVertex(arrow[0]);
				poly[1] = m.MulVertex(arrow[1]);
				poly[2] = m.MulVertex(arrow[2]);

				Render().DrawPolygon(poly, sizeof(poly)/sizeof(poly[0]), 0xFF0000FF);
			}

			Render().SetWorld(old);
		}

		bDrawShootSmokeSFX_ = false;
	}

	Render().DrawBox(	pattern_.massBoxCenter_-pattern_.massBoxBounds_*0.5f,
						pattern_.massBoxCenter_+pattern_.massBoxBounds_*0.5f,
						Matrix().BuildPosition(vPos_), 0xFF00FF00);

	// показать человечков
	if( m_bDrawSailors && pattern_.littleMen_.Size()>0 )
	{
		Matrix mw;
		IGMXScene* pSailorModel = 0;
		LittleManPattern* pSailorPattern = null;
		MOSafePointer sp;
		FindObject( pattern_.littleMen_[0].patternName, sp );
		static const ConstString typeId("LittleManPattern");
		if( sp.Ptr() && sp.Ptr()->Is(typeId) )
			pSailorPattern = (LittleManPattern*)sp.Ptr();
		if( pSailorPattern )
			pSailorModel = Geometry().CreateScene( pSailorPattern->GetParams().model, &Animation(), &Particles(), &Sound(), _FL_ );

		for( unsigned int n=0; n<pattern_.littleMen_.Size(); n++ )
		{
			mw.Build( pattern_.littleMen_[n].angles, vPos_ + pattern_.littleMen_[n].pos );
			if( pSailorModel )
			{
				pSailorModel->SetTransform( mw );
				pSailorModel->Draw();
			}
			else
				Render().DrawCapsule( 0.5f, 2.f, 0x8000FFFF, mw );

			if( m_nEditLittleManIndex == n )
			{
				Ballistics dieTrack;
				Vector endPoint = mw.pos + Vector(0,0,pattern_.littleMen_[n].explFlyLength).Rotate(pattern_.littleMen_[n].explFlyDir);
				endPoint.y = 0.0f;
				dieTrack.SetTrajectoryParams(mw.pos, endPoint, pattern_.littleMen_[n].explFlySpeed);
				dieTrack.DebugDraw(Render(),0xFFFF00FF);
				dieTrack.SetTrajectoryParams(mw.pos, endPoint, pattern_.littleMen_[n].explFlySpeed * 1.25f);
				dieTrack.DebugDraw(Render(),0xFFFF0000);
				Render().DrawLine(mw.pos, 0xFF00FF00, endPoint, 0xFF00FF00);
			}
		}

		RELEASE( pSailorModel );
	}
}

#define MOP_SHIPBOX \
	MOP_POSITION("Center", Vector(0.0f)); \
	MOP_POSITION("Size", Vector(0.5f)); \
	MOP_ANGLES("Orientation", Vector(0,0,0));
	//MOP_BOOL("Have Geometry", true);

#define MOP_SFXDESCR(name,def) \
	MOP_STRING(name,def) \
	MOP_STRING(name##" lod",def);

MOP_BEGINLISTG(ShipPattern, "Ship pattern", '1.00', 90, "Arcade Sea")
	// параметры визуализации в редакторе
	MOP_POSITION("Position", Vector(0,0,0))
	MOP_BOOL("Draw hull", true)
	MOP_BOOL("Draw masts", true)
	MOP_BOOL("Draw ropes", true)
	MOP_BOOL("Draw sides", true)
	MOP_BOOL("Draw shoot smoke SFX", false)
	MOP_BOOL("Draw sailors", false)
	MOP_BOOLC("Draw burn points", false, "vertical line + red sphere = cannon burn points; vertical line + blue sphere = flamethrower burn points")

	MOP_STRING("Destroy animation","");

	MOP_FLOATC("SFX lod distance", 100.f, "Distance where used small lod of SFX");

	MOP_BOOLC("Used by statistics", true, "Increase ship counter when ship destroyed")
	MOP_BOOLC("Is boat", false, "Use as achievement for boat (not ship)")

	// чтение лодов
	MOP_FLOATC("Ship visible distance",550.f,"Дистанция на которой корабль начинает становиться прозрачным")
	MOP_FLOATC("Ship visible range",150.f,"Дистанция в течении которой прозрачность корабля меняется от непрозрачного до полностью прозрачного")
	MOP_ARRAYBEGC("Ship lods",1,5,"Параметры для лодов модели корабля. Первый лод, это основная модель - остальные по дальности, в порядке возрастания дальности")
		MOP_STRING("Main model", "");
		MOP_STRING("Reflection model", "");
		MOP_STRING("Refraction model", "");
		MOP_FLOATC("Distance",0.f,"Дистанция на которой включается данный лод (для первого лода всегда=0.f)");
	MOP_ARRAYEND

	// физико-геометрические параметры
	MOP_GROUPBEG("Physics/Geometry params")
		MOP_COLOR("Model Color", Color(0.0f, 0.0f, 0.0f, 1.0f))
		MOP_POSITION("Mass shape center", Vector(0.0f, -0.5f, 0.0f))
		MOP_POSITION("Mass shape size", Vector(2.0f, 1.0f, 13.0f))

		MOP_POSITION("Bounding box center", Vector(0.0f, 25.0f, 0.0f))
		MOP_POSITION("Bounding box size", Vector(10.0f, 50.0f, 50.0f))

		MOP_POSITION("Mass center", Vector(0.0f, -1.5f, 0.0f))
		MOP_FLOAT("Mass", 10.0f)

		MOP_FLOAT("Waterline height", 0.3f)
		MOP_FLOAT("Forward slope force", 2000.0f)
		MOP_FLOAT("Backward slope force", 300.0f)
		MOP_FLOAT("Backward slope along ship scale", 4.0f)
		MOP_FLOAT("Angular friction force", 10.0f)
		MOP_FLOAT("Linear friction force", 10.0f)
		MOP_FLOAT("Return to waterline force", 300.0f)
		MOP_FLOAT("Waveness factor", 1.0f);
	MOP_GROUPEND()

	// корпус
	MOP_FLOATEX("ExplodeHullPower", 15.f, 0.1f, 100.f)
	MOP_LONGEXC("DamageHullRecurceDeep", 2, 0, 4, "Количество частей корпуса корабля на которое распространяется разрушение от одной части\nРаботает только для шутера")
	MOP_ARRAYBEG("Parts", 1, 100)
		MOP_STRING ("Model", "")
		MOP_STRING ("Damaged model", "")

		MOP_SFXDESCR("Ball hit SFX","hitany.xps")
		MOP_STRING ("Flamethrower hit SFX", "")
		MOP_PARTICLE_DESCR("Burn particles")
		MOP_SFXDESCR("Break SFX","")

		MOP_POSITION("Break SFX position", Vector(0.0f))
		MOP_FLOAT("HP", 100.0f)
		MOP_FLOATEX("Damage transfer coef", 1.0f, 0.0f, 20.0f)
		MOP_FLOATC("After break HP", 10.0f, "Восстановление ХП после уничтожения (для возможности повторного уничтожения)")
		MOP_ARRAYBEGC("Damaged Parts", 0, 100,"Части которым наносится повреждение от взрыва после уничтожения этой части")
			MOP_LONG("Part index",0)
			MOP_FLOAT("Receive damage",1.0f)
		MOP_ARRAYEND
		MOP_FLOATEXC("Touch damage factor", 100.f, 0.f, 100.f, "Процент получаемого повреждения при столкновении")
		MOP_POSITION("Touch SFX position", Vector(0.0f))
		MOP_STRING ("Touch SFX", "")

		MOP_ARRAYBEG("Burn points", 1, 10)
			MOP_POSITION("Position", Vector(0.0f, 10.0f, 0.0f))
		MOP_ARRAYEND
		MOP_FLOATC("Burn live time", 10.f, "Время жизни огня после взрыва")
		MOP_FLOATC("Burn live time add", 5.f, "Добавочное время жизни - для случайного разброса параметров")

		MOP_ARRAYBEG("Flamethrowers burn points", 0, 10)
			MOP_POSITION("Position", Vector(0.0f, 10.0f, 0.0f))
		MOP_ARRAYEND

		MOP_ARRAYBEG("PhysForm Boxes", 0, 10)
			MOP_SHIPBOX;
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_SFXDESCR("Ship explosion particles", "")
	MOP_STRING("Ship explosion sound", "")

	// борта
	MOP_ARRAYBEG("Sides", 4, 4)
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
	MOP_ARRAYEND

	// мачты
	MOP_ARRAYBEG("Masts", 1, 10)
		
		MOP_ARRAYBEG("Sails", 0, 10)
			MOP_ARRAYBEG("Sail corners", 3, 4)
				MOP_POSITIONC("Corner", Vector(0.0f, 10.0f, 0.0f), "It is relative position")
			MOP_ARRAYEND

			MOP_LONGEX("Width points count", 10, 5, 100)
			MOP_LONGEX("Height points count", 10, 5, 100)

			MOP_ARRAYBEG("Sails ropes", 0, 4)
				MOP_POSITIONC("Begin", Vector(0.0f, 10.0f, 5.0f), "Should be nearest to sail")
				MOP_POSITION("End", Vector(0.0f, 0.0f, 0.0f))
				MOP_FLOAT("Thickness", 0.1f)
			MOP_ARRAYEND

			MOP_ANGLESEX("Orientation", Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 0.0f), Vector(360.0f, 360.0f, 360.0f))
			MOP_POSITION("Position", Vector(0.0f, 0.0f, 0.0f))

			MOP_FLOATEXC( "Tear Factor", 35.0f, 1, 100, "Cloth Tear Factor" )
			MOP_STRINGC( "Diffuse texture", "Parus", "Texture File Name  ( *.txx, *.tga )" )
			MOP_STRINGC( "Glimpse texture", "", "Texture File Name  ( *.txx, *.tga )" )
			MOP_COLOR( "Sail color", Color (1.0f, 1.0f, 1.0f) )
			MOP_COLOR( "Glimpse color", Color (1.0f, 1.0f, 0.8f) )
			MOP_FLOAT("Color scale", 1.0f);

			MOP_FLOAT("Anim sail wave amplitude", 0.25f)
			MOP_FLOAT("X Wave frequency", 1.0f)
			MOP_FLOAT("Y Wave frequency", 1.0f)
			MOP_FLOAT("Wind curvature", 1.5f)
			MOP_FLOATEX("Bottom curvature", 0.9f, 0.0f, 2.0f)

		MOP_ARRAYEND

		MOP_SFXDESCR ("Hit SFX", "")
		MOP_FLOATEX("Damage transfer coefficient", 1.0f, 0.0f, 20.0f)
		MOP_BOOL ("Is rotatable", true)
		MOP_POSITION("Rotation axis start", Vector(0.0f, 0.0f, 0.0f))
		MOP_POSITION("Rotation axis end", Vector(0.0f, 10.0f, 0.0f))

		MOP_ARRAYBEG("Mast parts", 1, 10)
			// реи
			MOP_ARRAYBEG("Yards", 0, 10)
				MOP_STRING("Model", "")
				MOP_SFXDESCR("Hit SFX", "")
				
				MOP_FLOAT("HP", 5.0f)
				MOP_FLOATEX("Damage transfer coef", 1.0f, 0.0f, 20.0f)

				MOP_ARRAYBEG("PhysForm Boxes", 0, 10)
					MOP_SHIPBOX
				MOP_ARRAYEND

				MOP_BOOL("Not rotatable", false)
									
			MOP_ARRAYEND

			MOP_STRING("Model", "")
			MOP_FLOAT("HP", 10.0f)
			

			MOP_ARRAYBEG("PhysForm Boxes", 0, 10)
				MOP_SHIPBOX
			MOP_ARRAYEND
			
		MOP_ARRAYEND

	MOP_ARRAYEND
	MOP_FLOATC("Mast break damage",0.f,"Damage received by ship from any mast part breaking")

	MOP_GROUPBEG("Sailors")
		MOP_ARRAYBEG("Human", 0, 50)
			MOP_POSITION("Position at ship", Vector(0.0f))
			MOP_ANGLES("Angles", Vector(0.0f))

			MOP_FLOATC("Explosion fly direction", 0.0f, "Угол вокруг оси Y направления полета человечка при взрыве (0-соответсвует направлению по оси Z)")
			MOP_FLOATEXC("Explosion fly distance", 15.0f, 5.0f, 100.0f, "Дистанция полета человечка после взрыва (только в плоскости XZ)" )
			MOP_FLOATEXC("Explosion fly speed", 20.0f, 10.0f, 100.0f, "Скорость полета человечка после взрыва" )

			MOP_FLOATEX("Ball hit distance", 2.0f, 0.0f, 100.0f)

			MOP_STRING("Pattern name", "")
			MOP_ARRAYBEG("", 1, 10)
				MOP_STRING("Group name", "")
			MOP_ARRAYEND

			MOP_STRING("Default behaviour", "")
			MOP_BOOL("Show", true)
		MOP_ARRAYEND
	MOP_GROUPEND()

	// падение части корабля в море
	MOP_STRING("Ship part water splash SFX", "");
	MOP_FLOATC("Ship part water splash step",1.f,"шаг партиклов если на объекте их нужно много (например длинная мачта)");
	MOP_STRING("Ship bone water splash SFX", "");

	// звуки
	MOP_GROUPBEG("Sounds")
		MOP_STRING("Hull destroy", "");
		MOP_STRING("Ship not moved", "");
		MOP_STRING("Ship moved", "");
		MOP_STRING("Ship fire", "");
		MOP_STRING("Ship touched", "");
		MOP_STRING("Ship knock", "");
		MOP_STRING("Sail damage", "");
		MOP_STRING("Sail fire", "");
		MOP_STRING("Ship hit", "");

		MOP_FLOATC("Ship move sound fade time",1.5f,"Время включения/выключения звука движущегося/стоящего корабля");
	MOP_GROUPEND()

	MOP_MISSIONTRIGGER("Event CannonFire base")
	MOP_MISSIONTRIGGER("Event CannonFire mighty")
	MOP_FLOATC("ExcludeLightRadius",5.f,"Minimal distance for create new point light")
	MOP_FLOATC("LightOffset",5.f,"Offset point light to cannon direction")
	MOP_ARRAYBEG("CannonLights", 0, 10)
		MOP_FLOAT("Life time", 0.5f)
		MOP_STRING("Light object ID", "")
	MOP_ARRAYEND

MOP_ENDLIST(ShipPattern)
