#include "meshInstance.h"
#include "meshContainer.h"
#include "..\..\common_h\animation.h"
#include "..\..\common_h\physics.h"
#include "..\..\common_h\particles.h"


#include "..\..\common_h\corecmds.h"


#include "gmxService.h"
#include "blendShapeMixer.h"

//#ifdef _XBOX
#include "..\xrender\Render.h"
//#endif


#ifdef _XBOX
//Использовать VMX набор инструкций для умножения матриц анимации на матрицы мастер скелета
//и для расчета анимированных OBB модельки
#define USE_XBOX_VMX_MATRIXCODE
#endif


#ifndef STOP_DEBUG
const char * visemesName[] =
{ "<1> AA", "<2> AH, h", "<3> AO", "<4> AW OW", "<5> OY UH UW", "<6> EH, AE", "<7> IH,AY", "<8> EY", "<9> y, IY", "<Q> r, ER", "<W> l", "<E> w", "<R> m,p,b", "<T> n,NG,DH,d,g,t,z,ZH,s", "<Y> CH, j, SH", "<U> f,v", "<I> x"};

long debugKeys[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O,','P'};
#endif




IVariable * MeshInstance::texStage0 = NULL;
IVariable * MeshInstance::texStage1 = NULL;
IVariable * MeshInstance::texStage2 = NULL;
IVariable * MeshInstance::texStage3 = NULL;
IVariable* MeshInstance::pShadowColorVar = NULL;
IVariable* MeshInstance::pShadowVar = NULL;
IShadowsService* MeshInstance::shadowSRV = NULL;


#ifndef _XBOX
IVariable * MeshInstance::skinBonesVar = NULL;
#endif


DRAW_FUNCTION MeshInstance::drawFunctions[8];
dword MeshInstance::bDrawFunctionsInited = FALSE;


#ifdef _XBOX


//x360 version
__forceinline dword FrustumSphereTest (const Plane * __restrict frustum, XMVECTOR SphereCenter, XMVECTOR SphereRadius)
{
	MESH_ALIGN_16_CHECK(frustum);
	MESH_ALIGN_16_CHECK(sizeof(Plane));

	XMVECTOR frustumP0;
	frustumP0 = __lvx(frustum, 0);
	XMVECTOR frustumD0 = XMVectorReplicate(frustum->d);
	frustum++;
	
	XMVECTOR frustumP1;
	frustumP1 = __lvx(frustum, 0);
	XMVECTOR frustumD1 = XMVectorReplicate(frustum->d);
	frustum++;

	XMVECTOR frustumP2;
	frustumP2 = __lvx(frustum, 0);
	XMVECTOR frustumD2 = XMVectorReplicate(frustum->d);
	frustum++;

	XMVECTOR frustumP3;
	frustumP3 = __lvx(frustum, 0);
	XMVECTOR frustumD3 = XMVectorReplicate(frustum->d);
	frustum++;

	XMVECTOR frustumP4;
	frustumP4 = __lvx(frustum, 0);
	XMVECTOR frustumD4 = XMVectorReplicate(frustum->d);



	XMVECTOR dist0 = XMVector3Dot(frustumP0, SphereCenter) - frustumD0;
	XMVECTOR dist1 = XMVector3Dot(frustumP1, SphereCenter) - frustumD1;
	XMVECTOR dist2 = XMVector3Dot(frustumP2, SphereCenter) - frustumD2;
	XMVECTOR dist3 = XMVector3Dot(frustumP3, SphereCenter) - frustumD3;
	XMVECTOR dist4 = XMVector3Dot(frustumP4, SphereCenter) - frustumD4;

	//if (dist0 < -SphereRadius) return FALSE;

	XMVECTOR greater0 = XMVectorLess(dist0, -SphereRadius);
	XMVECTOR greater1 = XMVectorLess(dist1, -SphereRadius);
	XMVECTOR greater2 = XMVectorLess(dist2, -SphereRadius);
	XMVECTOR greater3 = XMVectorLess(dist3, -SphereRadius);
	XMVECTOR greater4 = XMVectorLess(dist4, -SphereRadius);

	XMVECTOR Outside = XMVectorFalseInt();
	// Outside the plane?
	Outside = XMVectorOrInt( Outside, greater0);
	Outside = XMVectorOrInt( Outside, greater1);
	Outside = XMVectorOrInt( Outside, greater2);
	Outside = XMVectorOrInt( Outside, greater3);
	Outside = XMVectorOrInt( Outside, greater4);

	// If the sphere is outside any of the planes it is outside. 
	if ( XMVector4EqualInt( Outside, XMVectorTrueInt()))
	{
		return FALSE;
	}


/*

	for(long p = 0; p < 5; p++)
	{
		float fDist = ((SphereCenter | frustum[p].n) - frustum[p].d);
		if(fDist < -SphereRadius)
		{
			return FALSE;
		}
	}
*/

	return TRUE;
}


#else

//pc version
__forceinline dword FrustumSphereTest (const Plane * __restrict frustum, const Vector& SphereCenter, float SphereRadius)
{
	MESH_ALIGN_16_CHECK(frustum);

	for(long p = 0; p < 5; p++)
	{
		float fDist = ((SphereCenter | frustum[p].n) - frustum[p].d);
		if(fDist < -SphereRadius)
		{
			return FALSE;
		}
	}

	return TRUE;
}


#endif




MESH_INLINE void SetupShadow(DWORD bShadowReceive)
{
	//обрабатываем deffer тень...
	if (bShadowReceive)
	{
		NGRender::pRS->SetupShadowLimiters();
		IRenderTarget* pRT = MeshInstance::shadowSRV->GetPostProcessTexture();

		//надо поставить реальную тень....
		if (MeshInstance::shadowSRV && pRT)
		{
			MeshInstance::pShadowColorVar->SetVector4(MeshInstance::shadowSRV->GetShadowColor().v4);                                                                                    
			MeshInstance::pShadowVar->SetTexture(pRT->AsTexture());
		} else
		{
			MeshInstance::pShadowVar->SetTexture(GMXService::pGS->GetNoShadowTexture());
		}
	} else
	{
		//Ставим текстуру - тени нет...
		MeshInstance::pShadowVar->SetTexture(GMXService::pGS->GetNoShadowTexture());
	}
}



MESH_INLINE void setIndices(const header & msh)
{
#ifdef USE_XBOX_INPLACE_GPU_RESOURCES
	NGRender::pRS->SetIndicesD3D(msh.inplaceIB.ptr, 0, 0, NULL);
#else
	NGRender::pRS->SetIndices(msh.indexBuffer);
#endif
}

MESH_INLINE void setAnimStream0(const header & msh, dword offsetInStream0InBytes)
{
#ifdef USE_XBOX_INPLACE_GPU_RESOURCES
	NGRender::pRS->SetStreamSourceD3D(0, msh.inplaceVBStream0.ptr, sizeof(MeshAnimVertex), offsetInStream0InBytes, 0, NULL);
#else
	NGRender::pRS->SetStreamSource(0, msh.stream0, sizeof(MeshAnimVertex), offsetInStream0InBytes);
#endif
}

MESH_INLINE void setAnimStream1(const header & msh, dword offsetInStream1InBytes)
{
#ifdef USE_XBOX_INPLACE_GPU_RESOURCES
	NGRender::pRS->SetStreamSourceD3D(1, msh.inplaceVBStream1.ptr, sizeof(MeshAnimBoneID), offsetInStream1InBytes, 0, NULL);
#else
	NGRender::pRS->SetStreamSource(1, msh.stream1, sizeof(MeshAnimBoneID), offsetInStream1InBytes);
#endif
}

MESH_INLINE void setStaticStream(const header & msh, dword offsetInVertexBufferInBytes)
{
#ifdef USE_XBOX_INPLACE_GPU_RESOURCES
	NGRender::pRS->SetStreamSourceD3D(0, msh.inplaceVBStream0.ptr, sizeof(MeshStaticVertex), offsetInVertexBufferInBytes, 0, NULL);
#else
	NGRender::pRS->SetStreamSource(0, msh.stream0, sizeof(MeshStaticVertex), offsetInVertexBufferInBytes);
#endif
}




MESH_INLINE void SetTextures(IBaseTexture** __restrict texPtr)
{
#ifndef STOP_DEBUG
	GMX_ShowMode showMode = GMXService::pGS->GetShowMode();
#endif

	Assert(GMX_MESH_MAX_TEXTURES_COUNT >= 4);


	IBaseTexture* tex = *texPtr;
	Assert(tex);
	MeshInstance::texStage0->SetTexture(tex);
	texPtr++;

	tex = *texPtr;
#ifndef STOP_DEBUG
	//Если  НЕ показать все или показать без нормалей, сбрасывае
	if (!(showMode == GMXSM_ALL || showMode == GMXSM_NONORMAL))
	{
		MeshInstance::texStage1->ResetTexture();
	}  else
	{
		Assert(tex);
		MeshInstance::texStage1->SetTexture(tex);
	}
#else
	Assert(tex);
	MeshInstance::texStage1->SetTexture(tex);
#endif
	texPtr++;

	tex = *texPtr;
#ifndef STOP_DEBUG
	if (!(showMode == GMXSM_ALL || showMode == GMXSM_NODIFFUSE))
	{
		MeshInstance::texStage2->SetTexture(GMXService::pGS->getEmptyNormalMap());
	}  else
	{
		Assert(tex);
		MeshInstance::texStage2->SetTexture(tex);
	}
#else
	Assert(tex);
	MeshInstance::texStage2->SetTexture(tex);
#endif
	texPtr++;

	tex = *texPtr;
	Assert(tex);
	MeshInstance::texStage3->SetTexture(tex);
	texPtr++;
}





MeshInstance::MeshInstance(IRender* renderService, MeshContainer * _container, const char * debugMeshName, const char* pSourceFile, int Line,
						   boneGpuData * _pBonesGPUdata, long * _pGeomToAnim_TranslateTable, long * _pAnimToGeom_TranslateTable,
						   long * _pDynamicTransofrmsBonesID, Matrix * _pDynamicTransofrmsBonesMatrixCache,
						   MeshParticleOnLocator* _pParticlesPtrCache,
						   float * _pMorphTargetsWeights, long * _pAnimToGeomBlendShapes_TranslateTable) : subscribedToDeletionEvent(_FL_),
						                                                                                   msh(_container->getHeader())
{

#ifndef STOP_DEBUG
	debugName = debugMeshName;
#endif


	dwCurrentDrawFunc = 0;
	if (bDrawFunctionsInited == FALSE)
	{
		drawFunctions[0] = &MeshInstance::DrawStatic<false, false>;
		drawFunctions[1] = &MeshInstance::DrawStatic<true, false>;
		drawFunctions[2] = &MeshInstance::DrawStatic<false, true>;
		drawFunctions[3] = &MeshInstance::DrawStatic<true, true>;

		drawFunctions[4] = &MeshInstance::DrawAnimated<false, false>;
		drawFunctions[5] = &MeshInstance::DrawAnimated<true, false>;
		drawFunctions[6] = &MeshInstance::DrawAnimated<false, true>;
		drawFunctions[7] = &MeshInstance::DrawAnimated<true, true>;

		bDrawFunctionsInited = TRUE;
	}

	pBonesGPUdata = _pBonesGPUdata;
	pGeomToAnim_TranslateTable = _pGeomToAnim_TranslateTable;
	pAnimToGeom_TranslateTable = _pAnimToGeom_TranslateTable;
	pDynamicTransofrmsBonesID = _pDynamicTransofrmsBonesID;
	pDynamicTransofrmsBonesMatrixCache = _pDynamicTransofrmsBonesMatrixCache;
	pParticlesPtrCache = _pParticlesPtrCache;
	
	pMorphTargetsWeights = _pMorphTargetsWeights;
	pAnimToGeomBlendShapes_TranslateTable = _pAnimToGeomBlendShapes_TranslateTable;

	if (_container->getHeader().bHaveBlendShapes && _container->getHeader().dwBlendShapeTargetsCount > 0)
	{
		//api->Trace("phonemes count : %d", _container->getHeader().dwBlendShapeTargetsCount);

		Assert(pMorphTargetsWeights);
		Assert(pAnimToGeomBlendShapes_TranslateTable);
	}

	blendShapeMixer = NULL;

	szSourceFile = pSourceFile;
	iSourceLine = Line;



	//Связываем список созданных мешей...
	MeshInstance * pRoot = GMXService::pHead;
	GMXService::pHead = this;
	if (pRoot == NULL)
	{
		pPrev = NULL;
		pNext = NULL;
	} else
	{
		pNext = pRoot;
		pPrev = NULL;
		pRoot->pPrev = this;
	}

	pRS = renderService;

	if (MeshInstance::texStage0 == NULL)
	{
		MeshInstance::texStage0 = pRS->GetTechniqueGlobalVariable("GMX_TextureStage0", _FL_);
		MeshInstance::texStage1 = pRS->GetTechniqueGlobalVariable("GMX_TextureStage1", _FL_);
		MeshInstance::texStage2 = pRS->GetTechniqueGlobalVariable("GMX_TextureStage2", _FL_);
		MeshInstance::texStage3 = pRS->GetTechniqueGlobalVariable("GMX_TextureStage3", _FL_);

		MeshInstance::pShadowColorVar = GMXService::pGS->GetShadowColor_Variable();
		MeshInstance::pShadowVar = GMXService::pGS->GetShadowTex_Variable();
		MeshInstance::shadowSRV = GMXService::pGS->GetShadowSrv();
	}

#ifndef _XBOX
	if (MeshInstance::skinBonesVar == NULL)
	{
		MeshInstance::skinBonesVar = GMXService::pGS->GetSkinBones_Variable();
	}
#endif

	container = _container;


	localBoundBoxAligned = AlignPtr((GMXBoundBox *)localBoundBoxStorage);
	MESH_ALIGN_16_CHECK(localBoundBoxAligned);


	globalBoundBoxAligned = AlignPtr((GMXBoundBox *)globalBoundBoxStorage);
	MESH_ALIGN_16_CHECK(globalBoundBoxAligned);


	globalBoundSphereAligned = AlignPtr((GMXBoundSphere *)globalBoundSphereStorage);
	MESH_ALIGN_16_CHECK(globalBoundSphereAligned);

	localBoundSphereAligned = AlignPtr((GMXBoundSphere *)localBoundSphereStorage);
	MESH_ALIGN_16_CHECK(localBoundSphereAligned);




	pAnimation = NULL;
	

	sceneAlphaRef = 0.5f;
	refCount = 1;
	clrUser = Color (0.0f, 0.0f, 0.0f, 1.0f);
	bShadowReceive = TRUE;
	bFrustumCull = TRUE;
	bDynamicLight = FALSE;

	pAnimManager = NULL;
	pParticleManager = NULL;
	pSoundManager = NULL;


	ResetReverseTranslateTables();

	



	ResetAllMorphTargets();

	SpoilAnimationCache ();
	SpoilTransformationsCache();

	SelectDrawFunction();
}

MeshInstance::~MeshInstance()
{
	SendDeleteNotifiaction();
	DeleteParticles();

	//Пересвязываем список
	if (pPrev == NULL)
	{
		Assert(GMXService::pHead = this);
		GMXService::pHead = pNext;
	} else
	{
		pPrev->pNext = pNext;
	}

	if (pNext)
	{
		pNext->pPrev = pPrev;
	}
	
	if (blendShapeMixer)
	{
		delete blendShapeMixer;
		blendShapeMixer = NULL;
	}


	if (container)
	{
		container->Release();
		container = NULL;
	}

	if (pAnimation)
	{
		pAnimation->Release();
		pAnimation = NULL;
	}

}


void MeshInstance::ResetAllMorphTargets()
{
	for (dword i = 0; i < msh.dwBlendShapeTargetsCount; i++)
	{
		pMorphTargetsWeights[i] = 0.0f;
	}
}

void MeshInstance::CreateAnimation(IAnimationScene * pAnimScene, const char * fileName)
{
	if (!msh.bAnimated)
	{
		return;
	}

	IAnimation * pNewAnim = pAnimScene->Create(fileName, _FL_);
	if (!pNewAnim)
	{
		return;
	}

	SetAnimation(pNewAnim);

	//т.к. вызвали Clone внутри SetAnimation тут релизим
	pNewAnim->Release();
	pNewAnim = NULL;
}


void MeshInstance::ResetReverseTranslateTables()
{
	if (pAnimToGeom_TranslateTable)
	{
		for (dword i = 0; i < MAX_ANIMATION_BONES; i++)
		{
			pAnimToGeom_TranslateTable[i] = -1;
		}
	}

	if (pAnimToGeomBlendShapes_TranslateTable)
	{
		for (dword i = 0; i < MAX_ANIMATION_BONES; i++)
		{
			pAnimToGeomBlendShapes_TranslateTable[i] = -1;
		}
	}

}

void MeshInstance::SetAnimation(IAnimationTransform * pAnim)
{
	if (!msh.bAnimated)
	{
		return;
	}

	if (pAnimation)
	{
		pAnimation->Release();
		pAnimation = NULL;
		ResetReverseTranslateTables();
	}

	if (pAnim)
	{
		pAnimation = pAnim->Clone();
	} else
	{
		return;
	}

	dword dwNotFoundBonesCount = 0;
	boneDataReadOnly * __restrict bonesData = msh.pBonesInfo.ptr;
	dword dwBonesCount = msh.dwBonesCount;
	Assert(dwBonesCount < 255);

	for (dword i = 0; i < dwBonesCount; i++, bonesData++)
	{
		//искать по короткому имени
		long indexInAnim = pAnimation->FindBoneUseHash (bonesData->name.c_str(), bonesData->name.hashNoCase());
		
		if (indexInAnim < 0)
		{
			dwNotFoundBonesCount++;
		} else
		{
			Assert(indexInAnim < MAX_ANIMATION_BONES);
			pAnimToGeom_TranslateTable[indexInAnim] = i;
		}

		pGeomToAnim_TranslateTable[i] = indexInAnim;

	} 

	if (dwNotFoundBonesCount > 0)
	{
		api->Trace("Warning: MeshInstance bind to Animation - %d bones not binded !! Check master skeleton !\n", dwNotFoundBonesCount);
	}


	nodeTransform * __restrict transform = msh.pDynamicTransformations.ptr;
	for (dword i = 0; i < msh.dwDynamicTransformationsCount; i++, transform++)
	{
		if (transform->boneName.c_str() != NULL)
		{
			pDynamicTransofrmsBonesID[i] = pAnimation->FindBoneUseHash (transform->boneName.c_str(), transform->boneName.hashNoCase());
			continue;
		}

		pDynamicTransofrmsBonesID[i] = -1;
	}



	dwNotFoundBonesCount = 0;
	blendShapeBoneDescription * __restrict bsBone = msh.pBlendShapeBones.ptr;
	for (dword i = 0; i < msh.dwBlendShapeBonesCount; i++, bsBone++)
	{
		if (bsBone->boneName.c_str() != NULL)
		{
			long indexInAnim = pAnimation->FindBoneUseHash (bsBone->boneName.c_str(), bsBone->boneName.hashNoCase());
			if (indexInAnim < 0)
			{
				dwNotFoundBonesCount++;
			} else
			{
				Assert(indexInAnim < MAX_ANIMATION_BONES);
				pAnimToGeomBlendShapes_TranslateTable[indexInAnim] = i;
			}
		}
	}


	if (dwNotFoundBonesCount > 0)
	{
		api->Trace("Warning: MeshInstance bind to Animation - %d blend shapes bones not binded !! Check master skeleton !\n", dwNotFoundBonesCount);
	}


	if (blendShapeMixer)
	{
		blendShapeMixer->RegisterAnimation(pAnimation, DEFAULT_BLENDSHAPE_LEVEL);
	}


}

void MeshInstance::UpdateTransformations(IAnimationTransform * pAnim) const
{
	dword dwServiceCacheIdx = GMXService::pGS->getCacheFrame();
	if (dwServiceCacheIdx == dwTransformationsUpdateCacheIdx)
	{
		return;
	}
	dwTransformationsUpdateCacheIdx = dwServiceCacheIdx;

	MESH_ALIGN_16_CHECK(msh.pAllTransformations.ptr);
	MESH_ALIGN_16_CHECK(sizeof(nodeTransform));

	Matrix * __restrict mtxCache = pDynamicTransofrmsBonesMatrixCache;
	long * __restrict boneId = pDynamicTransofrmsBonesID;
	nodeTransform * __restrict transform = msh.pDynamicTransformations.ptr;


	if (!pAnim || msh.bAnimated == false)
	{
		//обновляем без анимации...
		//--------------------------------------

		for (dword i = 0; i < msh.dwDynamicTransformationsCount; i++, transform++, mtxCache++)
		{
			//по идее рутовый нод не может быть динамическим, а только у него невалидный parentIndex
			int parentIdx = transform->parentIdx;
			Assert(parentIdx >= 0);

			nodeTransform * parentTransform = &msh.pAllTransformations.ptr[parentIdx];

			Matrix mtxLocalTransform = transform->mtxTransform * parentTransform->mtxLocalTransform;

			//сюда пишем, что бы не делать условие для parentTransform->mtxLocalTransform (парентом же может быть как дин. так и стат. трансформация)
			//просто как кеш используем
			transform->mtxLocalTransform = mtxLocalTransform;

			//Сюда пишем, что бы для каждого инстанса сохранить результат
			*mtxCache = mtxLocalTransform;
		}

		return;
	}

	Matrix mCurrentMtx(false);
	for (dword i = 0; i < msh.dwDynamicTransformationsCount; i++, transform++, boneId++, mtxCache++)
	{
		//Не привязанная трансформация к кости, просто наследует трансформацию
		long boneID = *boneId;
		if(boneID >= 0)
		{
			mCurrentMtx = pAnim->GetBoneMatrix(boneID);
		} else
		{
			mCurrentMtx = transform->mtxTransform;
		}

		//по идее рутовый нод не может быть динамическим, а только у него невалидный parentIndex
		int parentIdx = transform->parentIdx;
		Assert(parentIdx >= 0);

		nodeTransform * parentTransform = &msh.pAllTransformations.ptr[parentIdx];

		Matrix mtxLocalTransform = mCurrentMtx * parentTransform->mtxLocalTransform;

		//сюда пишем, что бы не делать условие для parentTransform->mtxLocalTransform (парентом же может быть как дин. так и стат. трансформация)
		//просто как кеш используем
		transform->mtxLocalTransform = mtxLocalTransform;

		//Сюда пишем, что бы для каждого инстанса сохранить результат
		*mtxCache = mtxLocalTransform;
	}
}

const Matrix& MeshInstance::GetNodeLocalTransform (GMXHANDLE handle)
{
	/*
	if (handle == INVALID_GMXHANDLE)
	{
	return NULL;
	}
	*/
	Assert(handle.index >= 1 && handle.index <= msh.dwLocatorsCount);

	dword dwLocIndex = (handle.index-1);

	dword dwDynamicTrasformIdx = msh.pLocators.ptr[dwLocIndex].dwDynamicTransformCacheIndex;
	if (dwDynamicTrasformIdx == INVALID_OFFSET)
	{
		//Статическая уже посчитаная есть...
		return msh.pLocators.ptr[dwLocIndex].pTransform.ptr->mtxLocalTransform;
	}

	UpdateTransformations(pAnimation);

	return pDynamicTransofrmsBonesMatrixCache[dwDynamicTrasformIdx];
}


void MeshInstance::UpdateAnimation(IAnimationTransform * pAnim) const
{
	dword dwServiceCacheIdx = GMXService::pGS->getCacheFrame();
	if (dwServiceCacheIdx == dwAnimationUpdateCacheIdx)
	{
		return;
	}
	dwAnimationUpdateCacheIdx = dwServiceCacheIdx;


	dword dwBonesCount = msh.dwBonesCount;
	Assert(dwBonesCount < 255);


	Vector boundVectorTempMax = Vector(0.0f, 0.0f, 0.0f);
	Vector boundVectorTempMin = Vector(0.0f, 0.0f, 0.0f);

	localBoundBoxAligned->vMax.x = -FLT_MAX;
	localBoundBoxAligned->vMax.y = -FLT_MAX;
	localBoundBoxAligned->vMax.z = -FLT_MAX;
	localBoundBoxAligned->padding0 = 0;

	localBoundBoxAligned->vMin.x = FLT_MAX;
	localBoundBoxAligned->vMin.y = FLT_MAX;
	localBoundBoxAligned->vMin.z = FLT_MAX;
	localBoundBoxAligned->padding1 = 0;



	if (pAnim == NULL)
	{
		const boneDataReadOnly * __restrict inputBone = msh.pBonesInfo.ptr;

		boneGpuData * __restrict gpuData = pBonesGPUdata;
		for (dword i = 0; i < dwBonesCount; i++, gpuData++, inputBone++)
		{
			gpuData->data[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			gpuData->data[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			gpuData->data[2].w = 0.0f;

			const Vector4 & bbSphereCenterAndRadius = inputBone->bbSphereCenterAndRadius;

			boundVectorTempMax = bbSphereCenterAndRadius.v + bbSphereCenterAndRadius.w;
			boundVectorTempMin = bbSphereCenterAndRadius.v - bbSphereCenterAndRadius.w;
			localBoundBoxAligned->vMax.Max(boundVectorTempMax);
			localBoundBoxAligned->vMax.Min(boundVectorTempMin);
		}

		return;
	}


	
	const long * __restrict translateTable = pGeomToAnim_TranslateTable;
	const boneMtxInputReadOnly * __restrict bindPoseMtx = msh.pBonesMatrices.ptr;
	const boneDataReadOnly * __restrict inputBone = msh.pBonesInfo.ptr;
	const Matrix * __restrict animBones = pAnim->GetBoneMatrices ();

	boneGpuData * __restrict gpuData = pBonesGPUdata;

	if (animBones == NULL)
	{
		return;
	}


	MESH_ALIGN_16_CHECK(sizeof(Matrix));
	MESH_ALIGN_16_CHECK(sizeof(boneGpuData));
	MESH_ALIGN_16_CHECK(sizeof(boneDataReadOnly));
	//MESH_ALIGN_16_CHECK(sizeof(boneCachedData));
	MESH_ALIGN_16_CHECK(sizeof(boneMtxInputReadOnly));

	MESH_ALIGN_16_CHECK(animBones);
	MESH_ALIGN_16_CHECK(gpuData);
	MESH_ALIGN_16_CHECK(bindPoseMtx);
	MESH_ALIGN_16_CHECK(inputBone);

#ifdef USE_XBOX_VMX_MATRIXCODE
	
	byte * pLocalBoundBoxMin = (byte *)&localBoundBoxAligned->vMin.x;
	byte * pLocalBoundBoxMax = (byte *)&localBoundBoxAligned->vMax.x;

	MESH_ALIGN_16_CHECK(pLocalBoundBoxMin);
	MESH_ALIGN_16_CHECK(pLocalBoundBoxMax);

	XMVECTOR localBoundBoxMin_VMX = __lvx(pLocalBoundBoxMin, 0);
	XMVECTOR localBoundBoxMax_VMX = __lvx(pLocalBoundBoxMax, 0);

#endif


	for (dword i = 0; i < dwBonesCount; i++, bindPoseMtx++, gpuData++, translateTable++, inputBone++)
	{
		long idx = *translateTable;

		if (idx >= 0)
		{
			const Matrix& mAnimFPU = animBones[idx];

			//pRS->DrawMatrix(mAnimFPU * mtxWorld, 0.3f);

#ifdef USE_XBOX_VMX_MATRIXCODE

			byte * __restrict pMtxBindPose = (byte*)&bindPoseMtx->mtxBindPose.matrix[0];
			XMVECTOR mInitVMX_r0 = __lvx(pMtxBindPose, 0);
			pMtxBindPose += sizeof(XMVECTOR);
			XMVECTOR mInitVMX_r1 = __lvx(pMtxBindPose, 0);
			pMtxBindPose += sizeof(XMVECTOR);
			XMVECTOR mInitVMX_r2 = __lvx(pMtxBindPose, 0);
			pMtxBindPose += sizeof(XMVECTOR);
			XMVECTOR mInitVMX_r3 = __lvx(pMtxBindPose, 0);

			byte * __restrict pMtxAnimFPU = (byte*)&mAnimFPU.matrix[0];
			XMVECTOR mAnimVMX_r0 = __lvx(pMtxAnimFPU, 0);
			pMtxAnimFPU += sizeof(XMVECTOR);
			XMVECTOR mAnimVMX_r1 = __lvx(pMtxAnimFPU, 0);
			pMtxAnimFPU += sizeof(XMVECTOR);
			XMVECTOR mAnimVMX_r2 = __lvx(pMtxAnimFPU, 0);
			pMtxAnimFPU += sizeof(XMVECTOR);
			XMVECTOR mAnimVMX_r3 = __lvx(pMtxAnimFPU, 0);

			//XMMATRIX * mInitVMX = (XMMATRIX *)&bindPoseMtx->mtxBindPose;
			//XMMATRIX * mAnimVMX = (XMMATRIX *)&mAnimFPU;

			//Готовим кешлинейку куда положить результат...
			__dcbt(0, (const void *)&gpuData->data[0]);


			//Транспонированная матрица mAnim

			XMVECTOR t0 = __vmrghw(mAnimVMX_r0, mAnimVMX_r2);
			XMVECTOR t1 = __vmrghw(mAnimVMX_r1, mAnimVMX_r3);
			XMVECTOR t2 = __vmrglw(mAnimVMX_r0, mAnimVMX_r2);
			XMVECTOR t3 = __vmrglw(mAnimVMX_r1, mAnimVMX_r3);

			XMMATRIX mAnimTransposed;
			mAnimTransposed.r[0] = __vmrghw(t0, t1);
			mAnimTransposed.r[2] = __vmrghw(t2, t3);
			mAnimTransposed.r[1] = __vmrglw(t0, t1);
			mAnimTransposed.r[3] = __vmrglw(t2, t3);
			//Получили mAnimTransposed


			//Считаем умножение, двух матриц - но результаты кладуться в вектора а не во float 
			XMVECTOR v00 = __vmsum4fp(mInitVMX_r0, mAnimTransposed.r[0]);//gpu
			XMVECTOR v02 = __vmsum4fp(mInitVMX_r0, mAnimTransposed.r[2]);//only for transform
			XMVECTOR v01 = __vmsum4fp(mInitVMX_r0, mAnimTransposed.r[1]);//gpu

			XMVECTOR v10 = __vmsum4fp(mInitVMX_r1, mAnimTransposed.r[0]);//gpu
			XMVECTOR v12 = __vmsum4fp(mInitVMX_r1, mAnimTransposed.r[2]);//only for transform
			XMVECTOR v11 = __vmsum4fp(mInitVMX_r1, mAnimTransposed.r[1]);//gpu

			XMVECTOR v20 = __vmsum4fp(mInitVMX_r2, mAnimTransposed.r[0]);//gpu
			XMVECTOR v22 = __vmsum4fp(mInitVMX_r2, mAnimTransposed.r[2]);//only for transform
			XMVECTOR v21 = __vmsum4fp(mInitVMX_r2, mAnimTransposed.r[1]);//gpu

			XMVECTOR v30 = __vmsum4fp(mInitVMX_r3, mAnimTransposed.r[0]);//gpu
			XMVECTOR v32 = __vmsum4fp(mInitVMX_r3, mAnimTransposed.r[2]);//gpu
			XMVECTOR v31 = __vmsum4fp(mInitVMX_r3, mAnimTransposed.r[1]);//gpu


			XMVECTOR _zero = {0.0f, 0.0f, 0.0f, 0.0f};
			XMVECTOR _one = {1.0f, 1.0f, 1.0f, 1.0f};

			XMVECTOR v00v02 = __vmrghw(v00, v02);
			XMVECTOR v01v03 = __vmrghw(v01, _zero);
			XMVECTOR v10v12 = __vmrghw(v10, v12);
			XMVECTOR v11v13 = __vmrghw(v11, _zero);
			XMVECTOR v20v22 = __vmrghw(v20, v22);
			XMVECTOR v21v23 = __vmrghw(v21, _zero);
			XMVECTOR v30v32 = __vmrghw(v30, v32);
			XMVECTOR v31v33 = __vmrghw(v31, _one);

			//результирующая матрица mInitVMX*mAnimVMX
			XMMATRIX resMatrix;
			resMatrix.r[0] = __vmrghw(v00v02, v01v03);
			resMatrix.r[1] = __vmrghw(v10v12, v11v13);
			resMatrix.r[2] = __vmrghw(v20v22, v21v23);
			resMatrix.r[3] = __vmrghw(v30v32, v31v33);


			//gpu data...
			//-------------------------
			//00, 10, 20, 30
			//01, 11, 21, 31
			//            32

			XMVECTOR v00v20 = __vmrghw(v00, v20);
			XMVECTOR v10v30 = __vmrghw(v10, v30);
			XMVECTOR v01v21 = __vmrghw(v01, v21);
			XMVECTOR v11v31 = __vmrghw(v11, v31);

			//---------------------------------------------------

			
			
			//XMVECTOR * _centerLocalVMXptr = (XMVECTOR *)&inputBone->bbSphereCenterAndRadius.x;
			//XMVECTOR * _radiusLocalVMXptr = (XMVECTOR *)&inputBone->bbSphereRadius.x;

			XMVECTOR centerLocalVMX = __lvx(&inputBone->bbSphereCenterAndRadius.x, 0);
			XMVECTOR radiusLocalVMX = __lvx(&inputBone->bbSphereRadius.x, 0);
			//XMVECTOR * minLocalVMXptr = (XMVECTOR *)&inputBone->bbMin.x;
			//XMVECTOR * maxLocalVMXptr = (XMVECTOR *)&inputBone->bbMax.x;

			//XMVECTOR * _centerWorldVMXptr = (XMVECTOR *)&cachedBoneData->bbSphereCenterGlobalAndRadius.x;

			//центр сферы посчитали...
			XMVECTOR centerWorldVMX = XMVector3Transform(centerLocalVMX, resMatrix);

			//XMVECTOR minWorldVMX = XMVector3Transform(*minLocalVMXptr, resMatrix);
			//XMVECTOR maxWorldVMX = XMVector3Transform(*maxLocalVMXptr, resMatrix);

			//XMVECTOR * maxWorldVMXptr = (XMVECTOR *)&cachedBoneData->bbMaxGlobal.x;
			//XMVECTOR * minWorldVMXptr = (XMVECTOR *)&cachedBoneData->bbMinGlobal.x;

			//min, max посчитали...
			//*maxWorldVMXptr = __vmaxfp(minWorldVMX, maxWorldVMX);
			//*minWorldVMXptr = __vminfp(minWorldVMX, maxWorldVMX);

			XMVECTOR boundVectorTempVMX_Max = __vaddfp(centerWorldVMX, radiusLocalVMX);
			XMVECTOR boundVectorTempVMX_Min = __vsubfp(centerWorldVMX, radiusLocalVMX);

			localBoundBoxMax_VMX = __vmaxfp(localBoundBoxMax_VMX, boundVectorTempVMX_Max);
			localBoundBoxMin_VMX = __vminfp(localBoundBoxMin_VMX, boundVectorTempVMX_Min);

			XMVECTOR * gpuDataVMX = (XMVECTOR *)&gpuData->data[0];

			gpuDataVMX[0] = __vmrghw(v00v20, v10v30);
			gpuDataVMX[1] = __vmrghw(v01v21, v11v31);
			gpuDataVMX[2].w = v32.x;

#else

			//FPU код
			const Matrix & mtxBindPoseFPU = bindPoseMtx->mtxBindPose;

			

			//Эти значения нужно будет сохранить в gpuData
			float v00FPU = mAnimFPU.m[0][0] * mtxBindPoseFPU.m[0][0] + mAnimFPU.m[1][0] * mtxBindPoseFPU.m[0][1] + mAnimFPU.m[2][0] * mtxBindPoseFPU.m[0][2];
			float v10FPU = mAnimFPU.m[0][0] * mtxBindPoseFPU.m[1][0] + mAnimFPU.m[1][0] * mtxBindPoseFPU.m[1][1] + mAnimFPU.m[2][0] * mtxBindPoseFPU.m[1][2];
			float v20FPU = mAnimFPU.m[0][0] * mtxBindPoseFPU.m[2][0] + mAnimFPU.m[1][0] * mtxBindPoseFPU.m[2][1] + mAnimFPU.m[2][0] * mtxBindPoseFPU.m[2][2];
			float v30FPU = mAnimFPU.m[0][0] * mtxBindPoseFPU.m[3][0] + mAnimFPU.m[1][0] * mtxBindPoseFPU.m[3][1] + mAnimFPU.m[2][0] * mtxBindPoseFPU.m[3][2] + mAnimFPU.m[3][0];
			float v01FPU = mAnimFPU.m[0][1] * mtxBindPoseFPU.m[0][0] + mAnimFPU.m[1][1] * mtxBindPoseFPU.m[0][1] + mAnimFPU.m[2][1] * mtxBindPoseFPU.m[0][2];
			float v11FPU = mAnimFPU.m[0][1] * mtxBindPoseFPU.m[1][0] + mAnimFPU.m[1][1] * mtxBindPoseFPU.m[1][1] + mAnimFPU.m[2][1] * mtxBindPoseFPU.m[1][2];
			float v21FPU = mAnimFPU.m[0][1] * mtxBindPoseFPU.m[2][0] + mAnimFPU.m[1][1] * mtxBindPoseFPU.m[2][1] + mAnimFPU.m[2][1] * mtxBindPoseFPU.m[2][2];
			float v31FPU = mAnimFPU.m[0][1] * mtxBindPoseFPU.m[3][0] + mAnimFPU.m[1][1] * mtxBindPoseFPU.m[3][1] + mAnimFPU.m[2][1] * mtxBindPoseFPU.m[3][2] + mAnimFPU.m[3][1];
			float v32FPU = mAnimFPU.m[0][2] * mtxBindPoseFPU.m[3][0] + mAnimFPU.m[1][2] * mtxBindPoseFPU.m[3][1] + mAnimFPU.m[2][2] * mtxBindPoseFPU.m[3][2] + mAnimFPU.m[3][2];

			//Эти значения нужны только для умножения на вектор...
			float v02FPU = mAnimFPU.m[0][2] * mtxBindPoseFPU.m[0][0] + mAnimFPU.m[1][2] * mtxBindPoseFPU.m[0][1] + mAnimFPU.m[2][2] * mtxBindPoseFPU.m[0][2];
			float v12FPU = mAnimFPU.m[0][2] * mtxBindPoseFPU.m[1][0] + mAnimFPU.m[1][2] * mtxBindPoseFPU.m[1][1] + mAnimFPU.m[2][2] * mtxBindPoseFPU.m[1][2];
			float v22FPU = mAnimFPU.m[0][2] * mtxBindPoseFPU.m[2][0] + mAnimFPU.m[1][2] * mtxBindPoseFPU.m[2][1] + mAnimFPU.m[2][2] * mtxBindPoseFPU.m[2][2];

			//Vector4 & centerWorldFPU = cachedBoneData->bbSphereCenterGlobalAndRadius;
			//Vector4 & minGlobalFPU = cachedBoneData->bbMinGlobal;
			//Vector4 & maxGlobalFPU = cachedBoneData->bbMaxGlobal;

			Vector4 minGlobalFPU_temp;
			Vector4 maxGlobalFPU_temp;

			const Vector4 & centerLocalFPU = inputBone->bbSphereCenterAndRadius;
			//const Vector & minLocalFPU = inputBone->bbMin;
			//const Vector & maxLocalFPU = inputBone->bbMax;

			//Вектор на матрицу
			Vector4 tmp;
			tmp.x = v00FPU * centerLocalFPU.x + v10FPU * centerLocalFPU.y + v20FPU * centerLocalFPU.z + v30FPU;
			tmp.y = v01FPU * centerLocalFPU.x + v11FPU * centerLocalFPU.y + v21FPU * centerLocalFPU.z + v31FPU;
			tmp.z = v02FPU * centerLocalFPU.x + v12FPU * centerLocalFPU.y + v22FPU * centerLocalFPU.z + v32FPU;	

			//centerWorldFPU.w = centerLocalFPU.w;

			boundVectorTempMax = tmp.v + centerLocalFPU.w;
			boundVectorTempMin = tmp.v - centerLocalFPU.w;
			localBoundBoxAligned->vMax.Max(boundVectorTempMax);
			localBoundBoxAligned->vMin.Min(boundVectorTempMin);



			//Вектор на матрицу
			//minGlobalFPU_temp.x = v00FPU * minLocalFPU.x + v10FPU * minLocalFPU.y + v20FPU * minLocalFPU.z + v30FPU;
			//minGlobalFPU_temp.y = v01FPU * minLocalFPU.x + v11FPU * minLocalFPU.y + v21FPU * minLocalFPU.z + v31FPU;
			//minGlobalFPU_temp.z = v02FPU * minLocalFPU.x + v12FPU * minLocalFPU.y + v22FPU * minLocalFPU.z + v32FPU;	

			//Вектор на матрицу
			//maxGlobalFPU_temp.x = v00FPU * maxLocalFPU.x + v10FPU * maxLocalFPU.y + v20FPU * maxLocalFPU.z + v30FPU;
			//maxGlobalFPU_temp.y = v01FPU * maxLocalFPU.x + v11FPU * maxLocalFPU.y + v21FPU * maxLocalFPU.z + v31FPU;
			//maxGlobalFPU_temp.z = v02FPU * maxLocalFPU.x + v12FPU * maxLocalFPU.y + v22FPU * maxLocalFPU.z + v32FPU;	


			//minGlobalFPU.v.Min(minGlobalFPU_temp.v, maxGlobalFPU_temp.v);
			//maxGlobalFPU.v.Max(minGlobalFPU_temp.v, maxGlobalFPU_temp.v);

			//Сохраняем для GPU
			gpuData->data[0].x = v00FPU;
			gpuData->data[0].y = v10FPU;
			gpuData->data[0].z = v20FPU;
			gpuData->data[0].w = v30FPU;

			gpuData->data[1].x = v01FPU;
			gpuData->data[1].y = v11FPU;
			gpuData->data[1].z = v21FPU;
			gpuData->data[1].w = v31FPU;

			gpuData->data[2].w = v32FPU;

/*
			if (GetAsyncKeyState('U') < 0)
			{
				pRS->DrawSphere(tmp.v * mtxWorld, centerLocalFPU.w);
			}

			
			

			if (GetAsyncKeyState('I') < 0)
			{
				pRS->DrawMatrix(Matrix(Vector(0.0f), tmp.v * mtxWorld), 0.3f);
				pRS->Print(tmp.v * mtxWorld, 100000.0f, 0.0f, 0xFFFFFFFF, "%s", inputBone->name.c_str());
			}

			if (stricmp (inputBone->name.c_str(), "t21") == 0 || stricmp (inputBone->name.c_str(), "q17") == 0)
			{
				//centerLocalFPU.xyz не должен быть нулем !!!! какой то баг....
				//коробка тоже пустая !!!! надо заново считать !!!

				Matrix mBindPos = mtxBindPoseFPU;
				mBindPos.Inverse();

				Vector ttt = tmp.v * mtxWorld;
				Vector ttt2 = mAnimFPU.pos * mtxWorld;

				Vector ttt3 = mBindPos.pos * mtxWorld;
				
				pRS->DrawSphere(ttt, centerLocalFPU.w);

				pRS->DrawMatrix(Matrix(Vector(0.0f), ttt2), 0.1f);
				pRS->DrawMatrix(Matrix(Vector(0.0f), ttt), 0.3f);
				pRS->DrawMatrix(Matrix(Vector(0.0f), ttt3), 0.2f);

				pRS->DrawLine(ttt2, 0xFFFF0000, ttt, 0xFFFFFFFF);
				pRS->DrawLine(ttt3, 0xFF00FF00, ttt, 0xFFFFFFFF);

				pRS->Print(ttt, 100000.0f, 0.0f, 0xFFFFFFFF, "%s", inputBone->name.c_str());
				pRS->Print(ttt2, 100000.0f, 0.0f, 0xFFFF0000, "%s", inputBone->name.c_str());
				pRS->Print(ttt3, 100000.0f, 0.0f, 0xFF00FF00, "%s", inputBone->name.c_str());
			}
*/

			

#endif
		} else
		{
			gpuData->data[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			gpuData->data[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			gpuData->data[2].w = 0.0f;

			//cachedBoneData->bbSphereCenterGlobalAndRadius = inputBone->bbSphereCenterAndRadius;
			//cachedBoneData->bbMaxGlobal = inputBone->bbMax;
			//cachedBoneData->bbMinGlobal = inputBone->bbMin;
		}

		
	}

#ifdef USE_XBOX_VMX_MATRIXCODE

	__stvx(localBoundBoxMin_VMX, pLocalBoundBoxMin, 0);
	__stvx(localBoundBoxMax_VMX, pLocalBoundBoxMax, 0);

#endif

	//pRS->DrawBox(localBoundBoxAligned->vMin, localBoundBoxAligned->vMax, mtxWorld, 0xFFFF0000);

	
}



void MeshInstance::SendSkeletonToGPU(subsetAnim * subset, const TTABLEITEM * pTTableLocalToGlobal, dword dwTTableSize) const
{
#ifdef _XBOX
	IVBuffer* bonesVB = GMXService::pGS->GetSkinBones_Stream();
	Vector4* GMX_skinBones = (Vector4*)bonesVB->Lock();
#else
	//Нужен все равно массив, т.к. мы мэпинг делаем в пространство HLSL массива...
	static Vector4 GMX_skinBones[225];
#endif

	//dword dwSubsetBonesCount = subset->dwBonesCount;
	//dword dwGlobalOffset = subset->bonesGlobalOffset;
	const TTABLEITEM * __restrict ttable = pTTableLocalToGlobal;
	boneGpuData * __restrict bonesGpuData = pBonesGPUdata;

	for (dword localBoneIndex = 0; localBoneIndex < dwTTableSize; localBoneIndex++, ttable++)
	{
		TTABLEITEM globalBoneIdx = *ttable;
		const boneGpuData & gpuData = bonesGpuData[globalBoneIdx];

		GMX_skinBones[localBoneIndex * 3 + 0] = gpuData.data[0];
		GMX_skinBones[localBoneIndex * 3 + 1] = gpuData.data[1];
		GMX_skinBones[localBoneIndex * 3 + 2] = gpuData.data[2];
	}

#ifdef _XBOX
	bonesVB->Unlock();
	pRS->SetStreamSource(2, bonesVB);
#else
	Vector4 * gpuBonesArray = &GMX_skinBones[0];
	MeshInstance::skinBonesVar->SetVector4Array(gpuBonesArray, 225);
#endif
}



	
void MeshInstance::Draw()
{

#ifndef STOP_DEBUG
	if (GMXService::bBlendShapeDebug)
	{
		dword index = INVALID_OFFSET;
		dword dwKeysCount = ARRSIZE(debugKeys);
		for (dword i = 0; i < dwKeysCount; i++)
		{
			if (api->DebugKeyState(debugKeys[i]))
			{
				index = i;
				break;
			}
		}
		
	
		for (dword j = 0; j < msh.dwBlendShapeTargetsCount; j++)
		{
			float val = 0.0f;
			if (j == index) val = 1.0f;
			SetBlendShapeMorphTargetWeight(j , val);
		}

		debugDrawBlendShapes();
	}
#endif



	if (bFrustumCull && GMXService::pGS->GetGlobalFrustumCullStateFast())
	{
		const GMXBoundSphere & bsphere = GetLocalBoundSphere();
		//const GMXBoundBox & bbox = GetLocalBound();
		const Plane* frustumPlanes = NGRender::pRS->GetFrustumFast();

#ifdef _XBOX
		Vector sphWorldCenter = bsphere.vCenter * mtxWorld;

		XMVECTOR sphereWorldCenter = XMLoadFloat3A((CONST XMFLOAT3A*)&sphWorldCenter.x);
		XMVECTOR sphereRadis = XMVectorReplicate(bsphere.fRadius);
		if (FrustumSphereTest(frustumPlanes, sphereWorldCenter, sphereRadis) == FALSE)
		{
			return;
		}


#else
		Vector sphereWorldCenter = bsphere.vCenter * mtxWorld;
		if (FrustumSphereTest(frustumPlanes, sphereWorldCenter, bsphere.fRadius) == FALSE)
		{
			return;
		}
#endif
	}






	if (!GMXService::pGS->IsSuppressSceneSetUserColor())
	{
		GMXService::pGS->SetUserColor(clrUser);
	}
	GMXService::pGS->SetCustomAlphaRef(sceneAlphaRef);

	pRS->SetWorld(mtxWorld);
	SetupShadow(bShadowReceive);

	dword dwOffset = 0;
	if (GMXService::pGS->GetRenderMode_Fast())
	{
		dwOffset = 2;
	}

	DRAW_FUNCTION drawFunc = drawFunctions[dwCurrentDrawFunc + dwOffset];
	(this->*drawFunc)();


#ifndef STOP_DEBUG
	if (GMXService::bGeometryDebug)
	{
		const GMXBoundBox & box = GetLocalBound();
		NGRender::pRS->DrawBox(box.vMin, box.vMax, mtxWorld);

		Matrix mtx;
		GMXHANDLE h;
		for (dword i = 0; i < msh.dwLocatorsCount; i++)
		{
			locator & loc = msh.pLocators.ptr[i];

			h.index = (i + 1);
			GetNodeWorldTransform(h, mtx);
			NGRender::pRS->DrawMatrix(mtx, 0.1f);
			NGRender::pRS->Print(mtx.pos, 100000.0f, 0.0f, 0xFFFFFFFF, "%s", loc.name.c_str());
		}
	}

#endif
}



void MeshInstance::ForceRelease ()
{
	Assert(refCount > 0);

	refCount = 1;
	Release ();
}

bool MeshInstance::Release ()
{
	Assert(refCount > 0);

	refCount--;
	if (refCount <= 0)
	{
		GMXService::pGS->freeMeshInstance(this);
		return true;
	}

	return false;
}

void MeshInstance::AddRef ()
{
	refCount++;
}



bool MeshInstance::IsAnimated ()
{
	return (msh.bAnimated != 0);
}

const Color& MeshInstance::GetUserColor()
{
	return clrUser;
}

const Matrix& MeshInstance::GetTransform ()
{
	return mtxWorld;
}

IAnimationTransform* MeshInstance::GetAnimationTransform ()
{
	if (pAnimation == NULL)
	{
		return NULL;
	}

	return pAnimation->Clone();
}

bool MeshInstance::GetDynamicLightState ()
{
	return (bDynamicLight != FALSE);
}

bool MeshInstance::GetShadowReceiveState ()
{
	IShadowsService* shadwSRV = GMXService::pGS->GetShadowSrv();

	if (!shadwSRV)
	{
		return false;
	}

	if (!shadwSRV->IsActiveShadowMap())
	{
		return false;
	}
	return (bShadowReceive != FALSE);
}


void MeshInstance::SetTransform (const Matrix& mWorld)
{
	mtxWorld = mWorld;
}

void MeshInstance::SetAnimationFile (const char* szAnimationFile)
{
	if (!msh.bAnimated || pAnimManager == NULL)
	{
		return;
	}

	CreateAnimation(pAnimManager, szAnimationFile);
}

void MeshInstance::SetBoneAlpha (dword dwBoneAnimationIdx, float fAlpha)
{
	if (!msh.bAnimated)
	{
		return;
	}

	if (dwBoneAnimationIdx >= 255)
	{
		return;
	}	

	long idxGeom = pAnimToGeom_TranslateTable[dwBoneAnimationIdx];
	if (idxGeom >= 0)
	{
		pBonesGPUdata[idxGeom].data[2].x = fAlpha;
	}
}

void MeshInstance::SetBoneScale (dword dwBoneAnimationIdx, float fScale)
{
	if (!msh.bAnimated)
	{
		return;
	}

	if (dwBoneAnimationIdx >= 255)
	{
		return;
	}

	long idxGeom = pAnimToGeom_TranslateTable[dwBoneAnimationIdx];
	if (idxGeom >= 0)
	{
		pBonesGPUdata[idxGeom].data[2].y = fScale;
	}
}

void MeshInstance::SetManagers (IAnimationScene * animationScene, IParticleManager* pManager, ISoundScene* pSound)
{
	pAnimManager = animationScene;
	pParticleManager = pManager;
	pSoundManager = pSound;

	CreateParticles();
}

void MeshInstance::SetUserColor(const Color& clr)
{
	clrUser = clr;
}

void MeshInstance::SetFloatAlphaReference(float alphaRef)
{
	sceneAlphaRef = alphaRef;
}

void MeshInstance::SetDynamicLightState (bool bState)
{
	bDynamicLight = bState;
	SelectDrawFunction();
}

void MeshInstance::SelectDrawFunction()
{
	if (msh.bAnimated == 0)
	{
		if (bDynamicLight == 0)
		{
			dwCurrentDrawFunc = 0;
		} else
		{
			dwCurrentDrawFunc = 1;
		}
	} else
	{
		if (bDynamicLight == 0)
		{
			dwCurrentDrawFunc = 4;
		} else
		{
			dwCurrentDrawFunc = 5;
		}
	}
}

void MeshInstance::SetShadowReceiveState (bool bState)
{
	bShadowReceive = bState;
}

IAnimationScene * MeshInstance::AnimationScene()
{
	return pAnimManager;
}

IParticleManager* MeshInstance::Particles()
{
	return pParticleManager;
}

ISoundScene* MeshInstance::Sound()
{
	return pSoundManager;
}

const GMXBoundSphere & MeshInstance::GetLocalBoundSphere()
{
	if (msh.bAnimated && pAnimation)
	{
		dword dwServiceCacheIdx = GMXService::pGS->getCacheFrame();
		if (dwServiceCacheIdx == dwLocalBSUpdateCacheIdx)
		{
			return *localBoundSphereAligned;
		}

		UpdateAnimation(pAnimation);

		Vector vExtents = (localBoundBoxAligned->vMax - localBoundBoxAligned->vMin) * 0.5f;

		localBoundSphereAligned->vCenter = localBoundBoxAligned->vMin + vExtents;
		localBoundSphereAligned->fRadius = vExtents.GetLength();

		dwLocalBSUpdateCacheIdx = dwServiceCacheIdx;

		return *localBoundSphereAligned;
	}

	return msh.staticBoundSphere;
}

const GMXBoundSphere & MeshInstance::GetBoundSphere()
{
	const GMXBoundSphere & localBound = GetLocalBoundSphere();

	globalBoundSphereAligned->vCenter = msh.staticBoundSphere.vCenter * mtxWorld;
	globalBoundSphereAligned->fRadius = msh.staticBoundSphere.fRadius;
	return *globalBoundSphereAligned;
}

const GMXBoundBox & MeshInstance::GetLocalBound()
{
	if (msh.bAnimated && pAnimation)
	{
		UpdateAnimation(pAnimation);
		return *localBoundBoxAligned;
	}

	return msh.staticBoundBox;
}

const GMXBoundBox & MeshInstance::GetBound()
{
	const GMXBoundBox & localBound = GetLocalBound();

	Vector vExtents = (localBound.vMax - localBound.vMin) * 0.5f;
	static Matrix mTempTranspose;

	mTempTranspose.pos = localBound.vMin + vExtents;

	//fixme speed: тут не полное умножение можно по идее...
	//и вообще посмотреть нормальный быстрый алгоритм OBB->AABB
	Matrix mTransform = mTempTranspose * mtxWorld;

	Vector vAABBExtents;

	float v1 = mTransform.m[0][0] * vExtents.x;
	float v2 = mTransform.m[0][1] * vExtents.y;
	float v3 = mTransform.m[0][2] * vExtents.z;

	float v4 = mTransform.m[1][0] * vExtents.x;
	float v5 = mTransform.m[1][1] * vExtents.y;
	float v6 = mTransform.m[1][2] * vExtents.z;

	float v7 = mTransform.m[2][0] * vExtents.x;
	float v8 = mTransform.m[2][1] * vExtents.y;
	float v9 = mTransform.m[2][2] * vExtents.z;

	//fixme speed: это на боксе смерть !! работа через память !!
	ffabs(v1); ffabs(v2); ffabs(v3);
	ffabs(v4); ffabs(v5); ffabs(v6);
	ffabs(v7); ffabs(v8); ffabs(v9);

	vAABBExtents.x = v1+v2+v3;
	vAABBExtents.y = v4+v5+v6;
	vAABBExtents.z = v7+v8+v9;

	Vector vAABBCenter = mTransform.pos;

	globalBoundBoxAligned->vMin = vAABBCenter - vAABBExtents;
	globalBoundBoxAligned->vMax = vAABBCenter + vAABBExtents;

	return *globalBoundBoxAligned;
}


GMXHANDLE MeshInstance::FindEntityByHashedString (GMXENTITYTYPE type, const char* szName, dword dwStringHash, dword dwLen)
{
	locator * entry = msh.entryTable[dwStringHash & (HASH_TABLE_SIZE - 1)].ptr;
	while(entry)
	{
		if (type == GMXET_LOCATOR && entry->desc.type != NODE_TYPE_LOCATOR)
		{
			entry = entry->pHashTableNext.ptr;
			continue;
		}

		if (type == GMXET_LOCATORPARTICLES && entry->desc.type != NODE_TYPE_LOCATORPARTICLES)
		{
			entry = entry->pHashTableNext.ptr;
			continue;
		}


		dword dwEntryHash = entry->name.hashNoCase();
		dword dwEntryLen = entry->name.length();

		if (dwEntryHash == dwStringHash && dwEntryLen == dwLen)
		{
			const char * szEntryName = entry->name.c_str();
			if (_stricmp(szEntryName, szName) == 0)
			{
				return entry->handle;
			}
		}

		entry = entry->pHashTableNext.ptr;
	}

	GMXHANDLE invalidHandle;
	invalidHandle.reset();
	return invalidHandle;
}


const Matrix & MeshInstance::GetNodeWorldTransform(GMXHANDLE handle, Matrix & mtx)
{
	Assert(handle.index >= 1 && handle.index <= msh.dwLocatorsCount);
	const Matrix & mtxNode = GetNodeLocalTransform(handle);
	mtx = mtxNode * mtxWorld;
	return mtx;
}

const char* MeshInstance::GetNodeName (GMXHANDLE handle)
{
	if (handle.isValid())
	{
		Assert(handle.index >= 1 && handle.index <= msh.dwLocatorsCount);
		dword dwLocIndex = (handle.index-1);
		return msh.pLocators.ptr[dwLocIndex].name.c_str();
	}

	return NULL;
}



IPhysRigidBody* MeshInstance::CreatePhysicsActor (IPhysicsScene & scene, bool bDynamic)
{
	if (msh.bAnimated || msh.dwCreatedPhysicMeshesCount == 0)
	{
		return NULL;
	}

	//---------------------------------------------
	Matrix mtxLocalTransform(false);
	IPhysicsScene::MeshInit * pPhysMeshes = (IPhysicsScene::MeshInit*)msh.pCreatedPhysicMeshes.ptr;
	mesh * __restrict geom = msh.pMeshes.ptr;
	dword dwMeshesCount = msh.dwMeshesCount;
	dword dwGlobalIndex = 0;
	for (dword i = 0; i < dwMeshesCount; i++, geom++)
	{
		if (geom->bCollision == 0)
		{
			continue;
		}

		mtxLocalTransform = geom->pTransform.ptr->mtxLocalTransform;
		dword dwSubsets = geom->dwSubsetCount;
		subsetStatic * __restrict subsetStat = geom->pStaticSubsets.ptr;
		for (dword j = 0; j < dwSubsets; j++, subsetStat++)
		{
			if (pPhysMeshes[dwGlobalIndex].mesh)
			{
				assert(dwGlobalIndex < msh.dwCreatedPhysicMeshesCount);
				pPhysMeshes[dwGlobalIndex].mtx = mtxLocalTransform;
				dwGlobalIndex++;
			}
		}
	}
	//---------------------------------------------


	IPhysRigidBody * pActorMesh = scene.CreateMesh(getSourceFile(), getSourceLine(), pPhysMeshes, msh.dwCreatedPhysicMeshesCount, bDynamic);
	return pActorMesh;
}



dword MeshInstance::GetBlendShapesCount()
{
	return msh.dwBlendShapeTargetsCount;
}


void MeshInstance::SetBlendShapeMorphTargetWeight (dword dwMorphtargetIndex, float fWeight)
{
	if (dwMorphtargetIndex >= msh.dwBlendShapeTargetsCount)
	{
		api->Trace("Morph Index = %d, Max Morph Index is = %d. CRITICAL ERROR !!!", dwMorphtargetIndex, msh.dwBlendShapeTargetsCount);
		return;
	}

	Assert(dwMorphtargetIndex < msh.dwBlendShapeTargetsCount);
	Assert(pMorphTargetsWeights);

	CreateBlendShapesIfNeed();

	pMorphTargetsWeights[dwMorphtargetIndex] = fWeight;
}


void MeshInstance::SpoilAnimationCache ()
{
	dword dwSpoiledCacheIdx = (GMXService::pGS->getCacheFrame() - 1);
	dwAnimationUpdateCacheIdx = dwSpoiledCacheIdx;
	dwLocalBSUpdateCacheIdx = dwSpoiledCacheIdx;
}

void MeshInstance::SpoilTransformationsCache ()
{
	dword dwSpoiledCacheIdx = (GMXService::pGS->getCacheFrame() - 1);
	dwTransformationsUpdateCacheIdx = dwSpoiledCacheIdx;
}



//Возвращает количество костей, и заполняет указатели информационная часть кости и трансформация кости...
dword MeshInstance::GetBonesArray(boneDataReadOnly ** boneData, boneMtxInputReadOnly ** boneTransformations)
{
	if(boneData)
	{
		*boneData = msh.pBonesInfo.ptr;
	}

	if (boneTransformations)
	{
		*boneTransformations = msh.pBonesMatrices.ptr;
	}

	return msh.dwBonesCount;
}

void MeshInstance::GetAllLocators(array<GMXHANDLE> & locators)
{
	locators.Reserve(msh.dwLocatorsCount);

	for (dword i = 0; i < msh.dwLocatorsCount; i++)
	{
		const locator & loc = msh.pLocators.ptr[i];
		locators.Add(loc.handle);
	}
}


int MeshInstance::findSubscribedClass (Object* _class)
{
	for (dword n = 0; n < subscribedToDeletionEvent.Size(); n++)
	{
		if (subscribedToDeletionEvent[n].object == _class) return n;
	}

	return -1;
}


void MeshInstance::SubscribeDeletionEvent (Object* _class, GMX_EVENT _method)
{
	// already subscribed...
	if (findSubscribedClass(_class) != -1) return;

	SubscribeStruct temp;
	temp.event = _method;
	temp.object = _class;
	subscribedToDeletionEvent.Add(temp);

}

void MeshInstance::UnSubscribeDeletionEvent (Object* _class)
{
	int founded = findSubscribedClass(_class);
	if (founded == -1) return;

	subscribedToDeletionEvent.ExtractNoShift(founded);
}


void MeshInstance::CreateParticles()
{
	if (!pParticleManager)
	{
		for (dword i = 0; i < msh.dwLocatorsWithParticlesCount; i++)
		{
			pParticlesPtrCache[i].pSystem = NULL;
			pParticlesPtrCache[i].pMasterLocator.reset();
		}

		return;
	}

	Matrix mWorldTransform;
	locator * __restrict cLocator = msh.pLocators.ptr;
	for (dword i = 0; i < msh.dwLocatorsCount; i++, cLocator++)
	{
		dword dwCachIdx = cLocator->dwParticleCacheIndex;
		if (dwCachIdx == INVALID_OFFSET)
		{
			continue;
		}

		Assert(dwCachIdx < msh.dwLocatorsWithParticlesCount);


		const char * particleSystemName = cLocator->particleName.c_str();


		IParticleSystem* pSystem = pParticleManager->CreateParticleSystemEx(particleSystemName, _FL_);
		if (pSystem)
		{
			GetNodeWorldTransform(cLocator->handle, mWorldTransform);
			pSystem->Teleport(mWorldTransform);
			pSystem->AutoHide(true);
		}

		pParticlesPtrCache[dwCachIdx].pSystem = pSystem;
		pParticlesPtrCache[dwCachIdx].pMasterLocator = cLocator->handle;
	}

}

void MeshInstance::DeleteParticles()
{
	for (dword i = 0; i < msh.dwLocatorsWithParticlesCount; i++)
	{
		MeshParticleOnLocator * particle = &pParticlesPtrCache[i];
		if (particle->pSystem == NULL)
		{
			continue;
		}

		particle->pSystem->Release();

		particle->pMasterLocator.reset();
		particle->pSystem = NULL;
	}


}

void MeshInstance::SendDeleteNotifiaction()
{
	for (int n = 0; n < subscribedToDeletionEvent; n++)
	{
		(subscribedToDeletionEvent[n].object->*subscribedToDeletionEvent[n].event) ();
	}
	subscribedToDeletionEvent.DelAll();
}


void MeshInstance::UpdateParticles()
{
	Matrix mWorldTransform;
	for (dword i = 0; i < msh.dwLocatorsWithParticlesCount; i++)
	{
		MeshParticleOnLocator * particle = &pParticlesPtrCache[i];
		if (particle->pSystem == NULL)
		{
			continue;
		}

		GetNodeWorldTransform(particle->pMasterLocator, mWorldTransform);
		particle->pSystem->SetTransform(mWorldTransform);
		particle->pSystem->CancelHide();
	}

}

#ifndef STOP_DEBUG
void MeshInstance::debugDrawBlendShapes()
{
	static char tempBar[16];

	Vector vDebugPos = mtxWorld.pos + Vector (0.0f, 2.0f, 0.0f);



	pRS->Print(vDebugPos, 10000.0f, 0, 0xFFFFFFFF, "BlendShapes : %d", msh.dwBlendShapeTargetsCount);
	for (dword i = 0; i < msh.dwBlendShapeTargetsCount; i++)
	{
		const char * visName = "notFound";
		float w = pMorphTargetsWeights[i];

		float _w = w;

		if (_w < 0.0f)
		{
			_w = 0.0f;
		}

		if (_w > 1.0f)
		{
			_w = 1.0f;
		}


		int barCount = (int)(_w * (ARRSIZE(tempBar)-1));
		memset(tempBar, '*', barCount);
		tempBar[barCount] = 0;

		if (i < ARRSIZE(visemesName))
		{
			visName = visemesName[i];
		}

		pRS->Print(vDebugPos, 10000.0f, (float)i + 1.0f, 0xFFFFFFFF, "(%s) %f %s", visName, w, tempBar);
	}


}
#endif

const char* MeshInstance::GetFileName()
{
#ifndef STOP_DEBUG
	return debugName.c_str();
#else
	return "";
#endif
}

void MeshInstance::CreateBlendShapesIfNeed()
{
	//Уже создали
	if (blendShapeMixer != NULL)
	{
		return;
	}

	//Не анимированный меш
	if (msh.bAnimated == false)
	{
		return;
	}

	//Нет блендшейпных костей
	if (msh.dwBlendShapeBonesCount <= 0)
	{
		return;
	}


	//Нет блендшейп визем
	if (msh.dwBlendShapeTargetsCount <= 0)
	{
		return;
	}

	//Нет установленной анимации
	if (pAnimation == NULL)
	{
		return;
	}


	//Нужна анимация, а не animation transform
	if (!pAnimation->Is(anitype_animation))
	{
		return;
	}
	
	
	
	blendShapeMixer = NEW GMXBlendShapeMixer(msh.dwBlendShapeTargetsCount, pMorphTargetsWeights, msh.pBlendShapeTargets.ptr,
                                             msh.dwBlendShapeBonesTransformCount, msh.pBlendShapeBonesTransform.ptr, pAnimToGeomBlendShapes_TranslateTable);

	blendShapeMixer->RegisterAnimation(pAnimation, DEFAULT_BLENDSHAPE_LEVEL);
}