#include "ShipAni.h"
#include "ShipPart.h"
#include "Ship.h"

//----------------------------------------------------------
// блендер
//----------------------------------------------------------
ShipAniBlendStage::ShipAniBlendStage() :
	m_aBoneDescr(_FL_),
	m_aParts(_FL_),
	m_aWaterSplash(_FL_),
	m_aCreateParticlesPos(_FL_)
{
	m_pAni = 0;
	m_bFinish = true;

#ifndef _XBOX
	bTraceDebug = false;
#endif
}

ShipAniBlendStage::~ShipAniBlendStage()
{
	UnregistryBlendStage();
	RELEASE(m_pAni);
}

void ShipAniBlendStage::Init(ShipPart* pRootPart, IAnimationTransform* ani)
{
	// удаление всех рабочих массивов
	m_aBoneDescr.DelAll();
	m_aWaterSplash.DelAll();
	m_aParts.DelAll();

	// анимация которую мы ставим вместо себя
	m_pAni = !ani ? null : (IAnimation*)ani->Clone();
	if( !ani ) {
		m_bFinish = true;
		return;
	}

	long q = m_pAni->GetNumBones();

	// по умолчанию все кости скелета управляются анимацией
	m_aBoneDescr.AddElements( q );
	for( long n=0; n<q; n++ )
	{
		m_aBoneDescr[n].partIndex = -1;
		m_aBoneDescr[n].isChild = false;
		m_aBoneDescr[n].wasWaterSplash = false;
	}

	// анимация к которой мы прикручиваем блендер (процедурное управление костями)
	SetAnimation(m_pAni);

	RegistryBlendStage(100);

	m_bFinish = false;
}

void ShipAniBlendStage::CreateWaterSplashParticles(const char* pcSFX, MissionObject* pMO)
{
	Matrix mtx(true);
	pMO->GetMatrix(mtx);
	Matrix m(mtx);

	for( dword n=0; n<m_aCreateParticlesPos.Size(); n++ )
	{
		m.pos = mtx.MulVertex( m_aCreateParticlesPos[n] );
		pMO->Particles().CreateParticleSystemEx2( pcSFX, m, true, _FL_ );
		//pMO->Render().DrawMatrix( m, 10.f );
	}

	m_aCreateParticlesPos.DelAll();
}

void ShipAniBlendStage::AttachPart(ShipPart* pPart,const char* pcBoneName)
{
	if( !m_pAni )
		return;

	// если часть не содержит физ боксов, то нечего ее присобачивать, все равно она ничем не управляет
	// анимация сама справится
	if( pPart->GetInitialPoses().Size() == 0 )
		return;

	// нет указания кости для привязки, тоже ничего не делаем, часть сама по себе
	// (например, фейковые реи для привязки парусов)
	if( !pcBoneName || pcBoneName[0]==0 )
		return;

	// ищем косточку
	long bone = m_pAni->FindBone(pcBoneName,true);

	// если кость не найдена (вообще это ошибка в паттерне!), то не учитываем часть в блендере
	// нечего путать анимацию
	if( bone<0 )
	{
		api->Trace("Ship Animation: Bone name %s is not exist into skeleton",pcBoneName);
		return;
	}

	// проверяем на повторное использование этой же кости
	long nParentPart = -1;
	for(long n=0; n<m_aParts; n++)
	{
		if( m_aParts[n].bone == bone )
		{
			api->Trace("Ship animation: Warning! Bone with name %s: already used.",pcBoneName);
			nParentPart = n;
			break;
		}
	}

	// добавляем часть в послужной список
	n = m_aParts.Add();

	m_aParts[n].part = pPart;
	m_aParts[n].bone = bone;

	// при повторном использовании, новая часть будет управлятся через старую часть
	// т.е. она будет как дите для части которая заняла кость раньше
	if( nParentPart != -1 )
	{
		m_aParts[n].ptm = ptm_child;
		// матрица преобразования трансформации родителя в трансформацию для этой части
		m_aParts[n].mtxTransform.EqMultiplyFast(
			pPart->GetInitialPoses()[0],
			Matrix(m_pAni->GetBoneMatrix(bone)).Inverse() );
	}
	else
	{
		m_aParts[n].ptm = ptm_physics;

		m_aBoneDescr[bone].partIndex = n;

		// матрица преобразования трансформации физбокса в трансформацию кости
		m_aBoneDescr[bone].mtxPhys2Bone.EqMultiplyFast(
			m_pAni->GetBoneMatrix(bone),
			Matrix(pPart->GetInitialPoses()[0]).Inverse() );
	}
}

void ShipAniBlendStage::TurnOnAnimation()
{
	for(long n=0; n<m_aParts; n++)
	{
		if( !m_aParts[n].part->IsBroken() && m_aParts[n].ptm == ptm_physics )
			m_aParts[n].ptm = ptm_animation;
	}
}

void ShipAniBlendStage::SetupChilds()
{
	if( !m_pAni )
		return;

	for( long n=0; n<m_aBoneDescr; n++ )
	{
		if( m_aBoneDescr[n].partIndex == -1 )
		{
			// Ищем родительскую кость завязанную на физический бокс в паттерне корабля
			for( long i=m_pAni->GetBoneParent(n); i>=0 && m_aBoneDescr[i].partIndex<0; )
				i = m_pAni->GetBoneParent(i);
			// Нет такого родителя, то пропускаем эту кость (она управляется только анимацией)
			if( i<0 || m_aBoneDescr[i].partIndex<0 )
				continue;

			// ставим флаг того, что эта кость зависит от части из паттерна корабля
			m_aBoneDescr[n].isChild = true;
			// запоминаем индекс кости которая напрямую зависит от части корабля
			m_aBoneDescr[n].partIndex = m_aBoneDescr[i].partIndex;
			// запоминаем относительную матрицу для формирования трансформации на основе управляющей части.
			//m_aBoneDescr[n].mtxPhys2Bone.EqMultiplyFast(
			//	m_pAni->GetBoneMatrix(n),
				//Matrix( m_aParts[m_aBoneDescr[i].partIndex].part->GetInitialPoses()[0]).Inverse() );
			//	Matrix( m_pAni->GetBoneMatrix(i) ).Inverse() );
			m_aBoneDescr[n].mtxPhys2Bone.EqMultiplyFast(
				m_pAni->GetBoneMatrix(n),
				Matrix(m_pAni->GetBoneMatrix(m_pAni->GetBoneParent(n))).Inverse() );
		}
	}
}

#ifndef _XBOX
void ShipAniBlendStage::TraceDebugInfo()
{
	// вывод дебажной инфы:
	const char* shipname = "unknown";
	for( long n=0; n<m_aParts; n++ )
		if( m_aParts[n].part )
		{
			shipname = m_aParts[n].part->GetMOOwner()->GetObjectID().c_str();
			break;
		}
	api->Trace("Ship (%s) animation blender info");
	api->Trace("============================================================");
	// кости
	for( n=0; n<m_aBoneDescr; n++ )
	{
		const char* partname = "noname";
		long bone = -1;
		const char* state = "undefined";
		if( m_aBoneDescr[n].partIndex >=0 )
		{
			bone = m_aParts[m_aBoneDescr[n].partIndex].bone;
			if( m_aParts[m_aBoneDescr[n].partIndex].part )
				partname = m_aParts[m_aBoneDescr[n].partIndex].part->GetDebugName();
			switch(m_aParts[m_aBoneDescr[n].partIndex].ptm)
			{
			case ptm_physics: state = "physics"; break;
			case ptm_animation: state = "animation"; break;
			case ptm_child: state = "child"; break;
			case ptm_collapse: state = "collapse"; break;
			}
		}
		api->Trace("Bone %d(%s): %s, part %d(%s) tied bone %d state is %s",
			n, m_pAni->GetBoneName(n),
			m_aBoneDescr[n].isChild ? "child" : "parent",
			m_aBoneDescr[n].partIndex, partname, bone, state );
	}
	// части
	for( n=0; n<m_aParts; n++ )
	{
		const char* partname = "noname";
		if( m_aParts[n].part )
			partname = m_aParts[n].part->GetDebugName();
		const char* state = "undefined";
		switch(m_aParts[n].ptm)
		{
		case ptm_physics: state = "physics"; break;
		case ptm_animation: state = "animation"; break;
		case ptm_child: state = "child"; break;
		case ptm_collapse: state = "collapse"; break;
		}
		api->Trace("Part %d(%s): state is %s, tied bone %d(%s)",
			n, partname, state, m_aParts[n].bone, m_pAni->GetBoneName(m_aParts[n].bone) );
	}
	api->Trace("============================================================");
}
#endif

void ShipAniBlendStage::GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
{
	// кость управляется анимацией и у нее нет управляющей части корабля
	if( m_aBoneDescr[boneIndex].partIndex < 0 )
	{
		rotation = prevRotation;
		position = prevPosition;
		scale = prevScale;
	}
	// кость имеет управляющую часть корабля и управляется ее физ обектом, а не анимацией из файла
	else if( m_aParts[m_aBoneDescr[boneIndex].partIndex].ptm==ptm_physics )
	{
		if( m_aBoneDescr[boneIndex].isChild )
		{
			//rotation = prevRotation;
			//position = prevPosition;
			//scale = prevScale;

			Matrix mtx = m_aBoneDescr[boneIndex].mtxPhys2Bone;// * m_aParts[m_aBoneDescr[boneIndex].partIndex].mtxTransform;
			rotation.Set( mtx );
			position = mtx.pos;
			scale = 1.f;
		}
		else
		{
			const Matrix& mtx = m_aParts[m_aBoneDescr[boneIndex].partIndex].mtxTransform;

			rotation.Set( mtx );
			position = mtx.pos;
			scale = 1.f;
		}
	}
	// кость имеет управляющую часть корабля, но управляется анимацией (требуется подгонять физ объекты под эту анимацию)
	else if( m_aParts[m_aBoneDescr[boneIndex].partIndex].ptm==ptm_animation )
	{
		// только единственный родитель управляет физикой части (остальные /дети/ ничего не трогают)
		if( !m_aBoneDescr[boneIndex].isChild )
		{
			Matrix& mtx = m_aParts[m_aBoneDescr[boneIndex].partIndex].mtxTransform;
			prevRotation.GetMatrix(mtx);
			mtx.pos = prevPosition;
		}

		rotation = prevRotation;
		position = prevPosition;
		scale = prevScale;
	}
	// кость имеет управляющую часть корабля и она уже сколлапшена (уничтожена)
	else if( m_aParts[m_aBoneDescr[boneIndex].partIndex].ptm==ptm_collapse )
	{
		rotation = prevRotation;
		position = prevPosition;
		scale = 0.f;
	}
	else
	// это ошибка!
		Assert(false);

	// проверка падения в воду
	if( position.y<0.f && !m_aBoneDescr[boneIndex].wasWaterSplash )
	{
		m_aBoneDescr[boneIndex].wasWaterSplash = true;
		m_aCreateParticlesPos.Add(position);
	}
}

void ShipAniBlendStage::Update(float dltTime)
{
	// флаг окончания проигрывания анимации
	if( !m_pAni || m_pAni->CurrentNodeIsStop() )
		m_bFinish = true;

	// обновляем положения физ объектов или же берем из них текущую трансформацию
	Matrix mtx(true);
	for(long n=0; n<m_aParts; n++)
	{
		switch( m_aParts[n].ptm )
		{
		case ptm_physics:
			m_aParts[n].part->GetPhysTransform( mtx );
			m_aParts[n].mtxTransform.EqMultiplyFast( m_aBoneDescr[m_aParts[n].bone].mtxPhys2Bone, mtx );
			break;

		case ptm_animation:
			mtx.EqMultiplyFast( Matrix( m_aBoneDescr[m_aParts[n].bone].mtxPhys2Bone ).Inverse(), m_aParts[n].mtxTransform );
			m_aParts[n].part->SetPhysTransform( mtx );
			break;

		case ptm_child:
			mtx.EqMultiplyFast( m_aParts[n].mtxTransform, m_aParts[m_aBoneDescr[m_aParts[n].bone].partIndex].mtxTransform );
			m_aParts[n].part->SetPhysTransform( mtx );
			break;
		}
	}

#ifndef _XBOX
	if( api->DebugKeyState( VK_SHIFT, VK_CONTROL, 'B' ) )
		if( !bTraceDebug ) {
			TraceDebugInfo();
			bTraceDebug = true;
		}
	else
		bTraceDebug = false;
#endif
}




//Отрыв парусов
void _cdecl ShipAniEvents::DetachSail(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if( numParams < 1) return;
	//m_pShip->
}


	
	
	
	
//----------------------------------------------------------
// утилиты
//----------------------------------------------------------
/*void IShipAnimationMaker::GetMasterSceleton(IGMXScene* pModel, array<const GMXBone*>& aSkeleton)
{
	if( !pModel ) return;

	//pModel->getBonesArrayFIXME(aSkeleton);
}*/

dword IShipAnimationMaker::CreateProcedureAnimation(IGMXScene* pModel, IAnimationProcedural* & pAni, IAnimationProcedural** ppAniRefl)
{
	pAni = NULL;
	if( ppAniRefl )
		*ppAniRefl = NULL;

	// нет модели - нет анимации
	if( !pModel ) return 0;

	boneDataReadOnly* boneData = NULL;
	boneMtxInputReadOnly* boneTransformations = NULL;
	dword dwBonesQ = pModel->GetBonesArray(&boneData, &boneTransformations);

	if( dwBonesQ > 0 )
	{
		// создаем скелет в формате необходимый для анимации:
		array<IAnimationScene::Bone> aBones(_FL_);
		aBones.AddElements( dwBonesQ );
		for( dword nb=0; nb<dwBonesQ; nb++ )
		{
			Matrix mtx = boneTransformations[nb].mtxBindPose;
			mtx.Inverse();
			aBones[nb].name = boneData[nb].name.c_str();
			aBones[nb].parentIndex = -1;//pModel->GetBoneParentIndexFIXME(aSkeleton[nb]);
			aBones[nb].mtx = mtx;
		}

		pAni = pModel->AnimationScene()->CreateProcedural( aBones.GetBuffer(), aBones.Size() , _FL_);
		if( ppAniRefl != NULL )
			*ppAniRefl = pModel->AnimationScene()->CreateProcedural( aBones.GetBuffer(), aBones.Size() , _FL_);
		pModel->SetAnimation( pAni );
		return dwBonesQ;
	}

	api->Trace("Warning! Model hav`t master sceleton (%s)", pModel->GetFileName() );
	return 0;
}
