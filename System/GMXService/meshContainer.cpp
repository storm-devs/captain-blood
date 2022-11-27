#include "meshContainer.h"
#include "..\..\common_h\FileService.h"
#include "..\..\common_h\physics.h"
#include "..\xrender\Render.h"


#include "gmxService.h"

#ifndef USE_XBOX_INPLACE_GPU_RESOURCES
static byte tailGPUResources[4096];
#endif


#ifdef _XBOX
#include "..\xrender\GraphicsApi.h"
#endif



MeshContainer::MeshContainer(IRender* renderService)
{
	pRS = renderService;

	const_data_raw = NULL;
	const_data = NULL;

	refCount = 1;
}

MeshContainer::~MeshContainer()
{
	IPhysicsScene::MeshInit * pPhysMeshes = (IPhysicsScene::MeshInit*)msh.pCreatedPhysicMeshes.ptr;
	for (dword i = 0; i < msh.dwCreatedPhysicMeshesCount; i++)
	{
		pPhysMeshes[i].mesh->Release();
	}

	material * __restrict currentMaterial = msh.pMaterials.ptr;
	for (dword i = 0; i < msh.dwMaterialsCount; i++, currentMaterial++)
	{
		for (dword t = 0; t < GMX_MESH_MAX_TEXTURES_COUNT; t++)
		{
			IBaseTexture* pTexture = currentMaterial->tex[t];
			if (pTexture)
			{
				pTexture->Release();
			}
		}
	}

	
	meshFree(const_data_raw, _FL_);
	const_data_raw = NULL;
	const_data = NULL;

	assert(msh.collisionDataPlatform == NULL);


#ifdef USE_XBOX_INPLACE_GPU_RESOURCES
	if (msh.physicalMemory)
	{
		XPhysicalFree(msh.physicalMemory);
		msh.physicalMemory = NULL;
	}

#else
	if (msh.indexBuffer)
	{
		msh.indexBuffer->Release();
		msh.indexBuffer = NULL;
	}

	if (msh.stream0)
	{
		msh.stream0->Release();
		msh.stream0 = NULL;
	}

	if (msh.stream1)
	{
		msh.stream1->Release();
		msh.stream1 = NULL;
	}

#endif

	GMXService::pGS->reportMeshContainerRelease(this);
}

void MeshContainer::LoadAsStatic(IDataFile* file)
{
	AllocConstantData(msh.dwStaticDataSize);
	file->Read(const_data, msh.dwStaticDataSize);


#ifdef USE_XBOX_INPLACE_GPU_RESOURCES

	Assert(sizeof(D3DVertexBuffer) == msh.sizeofD3DVertexBuffer);
	Assert(sizeof(D3DIndexBuffer) == msh.sizeofD3DIndexBuffer);

	dword dwGPUMemorySize = msh.dwIndexBufferDataSize4KAligned + msh.dwVertexBufferDataSize4KAligned0;
	
	if (dwGPUMemorySize > 0)
	{
		msh.inplaceIB.fixupAsOffset(const_data);
		msh.inplaceVBStream0.fixupAsOffset(const_data);
		msh.inplaceVBStream1.reset();

		msh.physicalMemory = (byte*)XPhysicalAlloc( dwGPUMemorySize, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_WRITECOMBINE );

		file->Read(msh.physicalMemory, dwGPUMemorySize);

		XGOffsetResourceAddress( msh.inplaceIB.ptr, msh.physicalMemory ); 
		XGOffsetResourceAddress( msh.inplaceVBStream0.ptr, msh.physicalMemory + msh.dwIndexBufferDataSize4KAligned ); 
	} else
	{
		msh.inplaceIB.reset();
		msh.inplaceVBStream0.reset();
		msh.inplaceVBStream1.reset();
	}


#else

	if (msh.dwIndexBufferDataSize > 0)
	{
		msh.indexBuffer = pRS->CreateIndexBuffer(msh.dwIndexBufferDataSize, _FL_);
		word * indexBuffer_data = (word * )msh.indexBuffer->Lock();
		file->Read(indexBuffer_data, msh.dwIndexBufferDataSize);

		dword dwTailSize = msh.dwIndexBufferDataSize4KAligned - msh.dwIndexBufferDataSize;
		Assert(dwTailSize < 4096);
		file->Read(tailGPUResources, dwTailSize);
#ifndef _XBOX
		for (dword i = 0; i < msh.dwIndexBufferIndexCount; i++)
		{
			MeshSwizzleWord(indexBuffer_data[i]);
		}
#endif

		msh.indexBuffer->Unlock();
	} //(msh.dwIndexBufferDataSize > 0)

	if (msh.dwVertexBufferDataSize0 > 0)
	{
		msh.stream0 = pRS->CreateVertexBuffer(msh.dwVertexBufferDataSize0, sizeof(MeshStaticVertex), _FL_);
		MeshStaticVertex * stream0_data = (MeshStaticVertex *)msh.stream0->Lock();
		file->Read(stream0_data, msh.dwVertexBufferDataSize0);

		dword dwTailSize = msh.dwVertexBufferDataSize4KAligned0 - msh.dwVertexBufferDataSize0;
		Assert(dwTailSize < 4096);
		file->Read(tailGPUResources, dwTailSize);
#ifndef _XBOX
		for (dword i = 0; i < msh.dwVertexBufferVertexCount0; i++)
		{
			MeshSwizzleStaticVertex(stream0_data[i]);
		}
#endif
		msh.stream0->Unlock();
		msh.stream1 = NULL;
	} //(msh.dwVertexBufferDataSize0 > 0)
#endif //USE_XBOX_INPLACE_GPU_RESOURCES


#ifdef _XBOX
	dword dwCollsionSize = msh.dwNovodexCollisionXBOX360_size;
#else
	//на PC загружаем оба коллижена(нет seek в файловой системе), но работаем только с нужным
	dword dwCollsionSize = msh.dwNovodexCollisionXBOX360_size + msh.dwNovodexCollisionPC_size;
#endif
	msh.collisionDataPlatform = (byte *)meshAlloc(dwCollsionSize, _FL_);
	file->Read(msh.collisionDataPlatform, dwCollsionSize);
}

void MeshContainer::LoadAsAnimated(IDataFile* file)
{
	AllocConstantData(msh.dwStaticDataSize);
	file->Read(const_data, msh.dwStaticDataSize);

#ifdef USE_XBOX_INPLACE_GPU_RESOURCES

	Assert(sizeof(D3DVertexBuffer) == msh.sizeofD3DVertexBuffer);
	Assert(sizeof(D3DIndexBuffer) == msh.sizeofD3DIndexBuffer);

	dword dwGPUMemorySize = msh.dwIndexBufferDataSize4KAligned + msh.dwVertexBufferDataSize4KAligned0 + msh.dwVertexBufferDataSize4KAligned1;

	if (dwGPUMemorySize > 0)
	{
		msh.inplaceIB.fixupAsOffset(const_data);
		msh.inplaceVBStream0.fixupAsOffset(const_data);
		msh.inplaceVBStream1.fixupAsOffset(const_data);


		msh.physicalMemory = (byte*)XPhysicalAlloc( dwGPUMemorySize, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_WRITECOMBINE );

		file->Read(msh.physicalMemory, dwGPUMemorySize);

		XGOffsetResourceAddress( msh.inplaceIB.ptr, msh.physicalMemory ); 
		XGOffsetResourceAddress( msh.inplaceVBStream0.ptr, msh.physicalMemory + msh.dwIndexBufferDataSize4KAligned ); 
		XGOffsetResourceAddress( msh.inplaceVBStream1.ptr, msh.physicalMemory + msh.dwIndexBufferDataSize4KAligned + msh.dwVertexBufferDataSize4KAligned0 ); 
	} else
	{
		msh.inplaceIB.reset();
		msh.inplaceVBStream0.reset();
		msh.inplaceVBStream1.reset();
	}

#else

	if (msh.dwIndexBufferDataSize > 0)
	{
		msh.indexBuffer = pRS->CreateIndexBuffer(msh.dwIndexBufferDataSize, _FL_);
		word * indexBuffer_data = (word *)msh.indexBuffer->Lock();
		file->Read(indexBuffer_data, msh.dwIndexBufferDataSize);

		dword dwTailSize = msh.dwIndexBufferDataSize4KAligned - msh.dwIndexBufferDataSize;
		Assert(dwTailSize < 4096);
		file->Read(tailGPUResources, dwTailSize);

#ifndef _XBOX
		for (dword i = 0; i < msh.dwIndexBufferIndexCount; i++)
		{
			MeshSwizzleWord(indexBuffer_data[i]);
		}
#endif
		msh.indexBuffer->Unlock();
	} //(msh.dwIndexBufferDataSize > 0)


	if (msh.dwVertexBufferDataSize0 > 0)
	{
		msh.stream0 = pRS->CreateVertexBuffer(msh.dwVertexBufferDataSize0, sizeof(MeshAnimVertex), _FL_);
		MeshAnimVertex * stream0_data = (MeshAnimVertex *)msh.stream0->Lock();
		file->Read(stream0_data, msh.dwVertexBufferDataSize0);

		dword dwTailSize = msh.dwVertexBufferDataSize4KAligned0 - msh.dwVertexBufferDataSize0;
		Assert(dwTailSize < 4096);
		file->Read(tailGPUResources, dwTailSize);

#ifndef _XBOX
		for (dword i = 0; i < msh.dwVertexBufferVertexCount0; i++)
		{
			MeshSwizzleAnimVertex(stream0_data[i]);
		}
#endif
		msh.stream0->Unlock();
	} //(msh.dwVertexBufferDataSize0 > 0)


	if (msh.dwVertexBufferDataSize1 > 0)
	{
		msh.stream1 = pRS->CreateVertexBuffer(msh.dwVertexBufferDataSize1, sizeof(MeshAnimBoneID), _FL_);
		MeshAnimBoneID * stream1_data = (MeshAnimBoneID *)msh.stream1->Lock();
		file->Read(stream1_data, msh.dwVertexBufferDataSize1);

		dword dwTailSize = msh.dwVertexBufferDataSize4KAligned1 - msh.dwVertexBufferDataSize1;
		Assert(dwTailSize < 4096);
		file->Read(tailGPUResources, dwTailSize);
#ifndef _XBOX
		for (dword i = 0; i < msh.dwVertexBufferVertexCount1; i++)
		{
			MeshSwizzleAnimBoneID(stream1_data[i]);
		}
#endif
		msh.stream1->Unlock();
	} //if (msh.dwVertexBufferDataSize1 > 0)
#endif

}

void MeshContainer::CreateCollision()
{
	if (msh.collisionDataPlatform == NULL)
	{
		return;
	}

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

		dword dwSubsets = geom->dwSubsetCount;
		subsetStatic * __restrict subsetStat = geom->pStaticSubsets.ptr;

		for (dword j = 0; j < dwSubsets; j++, subsetStat++)
		{
#ifdef _XBOX
			byte * collisionPtr = subsetStat->pNovodexCollisionXBOX360.ptr;
			dword collisionSize = subsetStat->dwNovodexCollisionSizeXBOX360;

			byte * pmapPtr = subsetStat->pNovodexPMapXBOX360.ptr;
			dword pmapSize = subsetStat->dwNovodexPMapSizeXBOX360;

#else
			byte * collisionPtr = subsetStat->pNovodexCollisionPC.ptr;
			dword collisionSize = subsetStat->dwNovodexCollisionSizePC;

			byte * pmapPtr = subsetStat->pNovodexPMapPC.ptr;
			dword pmapSize = subsetStat->dwNovodexPMapSizePC;
#endif

			if (collisionSize > 0)
			{
				Assert(GMXService::pPhysic);
				IPhysTriangleMesh * pPhysCollision = GMXService::pPhysic->CreateTriangleMesh(collisionPtr, collisionSize, pmapPtr, pmapSize);

				assert(dwGlobalIndex < msh.dwCreatedPhysicMeshesCount);
				pPhysMeshes[dwGlobalIndex].mesh = pPhysCollision;
				//pPhysMeshes[dwGlobalIndex].mtx.SetIdentity();
				dwGlobalIndex++;
			}
		}
	}


	meshFree(msh.collisionDataPlatform, _FL_);
	msh.collisionDataPlatform = NULL;
}



bool MeshContainer::Load (IDataFile * file, IFileService * pFS, const char * nameForDebugMessages)
{
	if (!file)
	{
		return false;
	}

	dword dwRead = file->Read(&msh, sizeof(header));

	if (dwRead != sizeof(header))
	{
		//invalid file size
		api->Trace("'%s' header too small, incorrect mesh file!", nameForDebugMessages);
		return false;
	}

#ifndef _XBOX
	msh.swizzle(false);
#endif

	if (msh.dwHeader != MESH_HEADER)
	{
		//not mesh
		api->Trace("'%s' is not mesh file !", nameForDebugMessages);
		return false;
	}

	if (msh.dwVersion != MESH_VERSION)
	{
		//invalid version
		api->Trace("'%s' invalid version %d, must be %d", nameForDebugMessages, msh.dwVersion, MESH_VERSION);
		return false;
	}


	if (msh.bAnimated)
	{
		LoadAsAnimated(file);
	} else
	{
		LoadAsStatic(file);
	}

	//файл больше не нужен...
	//----------------------------------------------------------------
	FixupPointers();
	CreateCollision();
	CreateAndLinkMaterials();

	return true;
}


void MeshContainer::CreateAndLinkMaterials()
{
	material * __restrict currentMaterial = msh.pMaterials.ptr;

	for (dword i = 0; i < msh.dwMaterialsCount; i++, currentMaterial++)
	{
		pRS->GetShaderLightingId(currentMaterial->shaderName.c_str(), currentMaterial->tech);


		//Если не diffuse то будет белая текстура
		IBaseTexture* defaultTexture = NGRender::pRS->getWhiteTexture();

		//
		IBaseTexture** __restrict texPtr = &currentMaterial->tex[0];
		for (dword texIdx = 0; texIdx < GMX_MESH_MAX_TEXTURES_COUNT; texIdx++)
		{
			Assert(texIdx < GMX_MESH_MAX_TEXTURES_COUNT);
			const char * texName = currentMaterial->textureNames[texIdx].c_str();

			IBaseTexture* baseTex = NULL;
			if (texName)
			{
				baseTex = pRS->CreateTexture(_FL_, "%s\\%s", GMXService::pGS->GetTexturePath(), texName);

				//Что бы во все незагруженные установился diffuse 
				//(это все, что бы убрать условия в runtime каждый кадр, что если нет текстуры поставить diffuse, а если нет диффуза, поставить белую)
				if (baseTex && texIdx == 0)
				{
					defaultTexture = baseTex;
				}
			}

			if (baseTex == NULL)
			{
				baseTex = defaultTexture;
				baseTex->AddRef();
			}

			Assert(baseTex);
			texPtr[texIdx] = baseTex;
		}

		DWORD dwDiffuseIdx = currentMaterial->dwDiffuseSlotIdx;
		IBaseTexture * diffuseTex = currentMaterial->tex[dwDiffuseIdx];
		if (!diffuseTex) continue;
		if (diffuseTex->GetFormat() != FMT_DXT1) continue;

		currentMaterial->isNoAlpha = 1;
	}

	//Что бы избавиться от индирекции при отрисовке...
	if (msh.bAnimated)
	{
		mesh * __restrict geom = msh.pMeshes.ptr;
		for (dword i = 0; i < msh.dwMeshesCount; i++, geom++)
		{
			dword dwSubsets = geom->dwSubsetCount;

			subsetAnim * __restrict sub = geom->pAnimSubsets.ptr;

			for (dword j = 0; j < dwSubsets; j++, sub++)
			{
				ApplyMaterialToSubsetAnimated(sub->pMaterial.ptr, sub);
			}
		}
	} else
	{
		mesh * __restrict geom = msh.pMeshes.ptr;
		for (dword i = 0; i < msh.dwMeshesCount; i++, geom++)
		{
			dword dwSubsets = geom->dwSubsetCount;
			subsetStatic * __restrict sub = geom->pStaticSubsets.ptr;

			for (dword j = 0; j < dwSubsets; j++, sub++)
			{
				ApplyMaterialToSubsetStatic(sub->pMaterial.ptr, sub);
			}
		}
	}
}



#ifndef _XBOX
void MeshContainer::SwizzleStaticData()
{
	const char * __restrict dataEnd = (const char *)(const_data + msh.dwStringTableSize);
	const char * __restrict dataIterator = (const char *)(const_data + sizeof(stringHead));
	if (dataIterator < dataEnd)
	{
		for (;;)
		{
			stringHead * head = (stringHead*)(dataIterator - sizeof(stringHead));

			MeshSwizzleDWord(head->dwHashNoCase);
			MeshSwizzleDWord(head->dwLen);

			dataIterator += (head->dwLen + sizeof(stringHead)) + sizeof(char);
			if (dataIterator >= dataEnd)
			{
				break;
			}
		}
	}

	material * __restrict currentMaterial = msh.pMaterials.ptr;
	for (dword i = 0; i < msh.dwMaterialsCount; i++, currentMaterial++)
	{
		MeshSwizzleDWord(currentMaterial->dwBonesInSkin);
		MeshSwizzleDWord(currentMaterial->dwDiffuseSlotIdx);
		MeshSwizzleDWord(currentMaterial->dwLevel);
		MeshSwizzleDWord(currentMaterial->dwTexCount);
		MeshSwizzleDWord(currentMaterial->isNoAlpha);
	}

	renderOrder * __restrict order = msh.pRenderOrders.ptr;
	for (dword i = 0; i < msh.dwOrdersCount; i++, order++)
	{
		MeshSwizzleDWord(order->dwMeshOrder);
	}


	locator * __restrict cLocator = msh.pLocators.ptr;
	for (dword i = 0; i < msh.dwLocatorsCount; i++, cLocator++)
	{
		MeshSwizzleDWord(cLocator->desc.dw);
		MeshSwizzleDWord(cLocator->dwDynamicTransformCacheIndex);
		MeshSwizzleDWord(cLocator->dwParticleCacheIndex);
		//Assert(cLocator->dwDynamicTransformCacheIndex != 0xFFFFFFFF);
		MeshSwizzleDWord(cLocator->handle.index);
	}

	mesh * __restrict cMesh = msh.pMeshes.ptr;
	for (dword i = 0; i < msh.dwMeshesCount; i++, cMesh++)
	{
		MeshSwizzleDWord(cMesh->bCollision);
		MeshSwizzleDWord(cMesh->bVisible);
		MeshSwizzleDWord(cMesh->desc.dw);
		MeshSwizzleDWord(cMesh->dwRenderOrder);
		MeshSwizzleDWord(cMesh->dwSubsetCount);
		MeshSwizzleVector3(cMesh->max);
		MeshSwizzleVector3(cMesh->min);
	}

	nodeTransform * __restrict transfrom = msh.pAllTransformations.ptr;
	for (dword i = 0; i < msh.dwTransformationsCount; i++, transfrom++)
	{
		//MeshSwizzleLong(transfrom->animBoneID);
		MeshSwizzleMatrix(transfrom->mtxLocalTransform);
		MeshSwizzleMatrix(transfrom->mtxTransform);
		MeshSwizzleInt(transfrom->parentIdx);
	}


	TTABLEITEM * __restrict ttableItem = msh.pTranslateTables.ptr;
	for (dword i = 0; i < msh.dwTranslateTableCount; i++, ttableItem++)
	{
		MeshSwizzleTableItem(*ttableItem);
	}




	if (msh.bAnimated)
	{
		subsetAnim * __restrict animSubset = msh.pAnimSubsets.ptr;
		for (dword i = 0; i < msh.dwSubsetsCount; i++, animSubset++)
		{
			//MeshSwizzleDWord(animSubset->bonesGlobalOffset);
			//MeshSwizzleDWord(animSubset->dwBonesCount);
			MeshSwizzleDWord(animSubset->dwPassesCount);
			MeshSwizzleDWord(animSubset->numVertexes);
			MeshSwizzleDWord(animSubset->offsetInStream0InBytes);
		}

		subSubsetAnim * __restrict subSubsetAnm = msh.pSubSubsets.ptr;
		for (dword i = 0; i < msh.dwSubSubsetAnimCount; i++, subSubsetAnm++)
		{
			MeshSwizzleDWord(subSubsetAnm->dwTranslateTableSize);
			MeshSwizzleDWord(subSubsetAnm->numVertexes);
			MeshSwizzleDWord(subSubsetAnm->offsetInIndexBufferInInds);
			MeshSwizzleDWord(subSubsetAnm->offsetInStream1InBytes);
			MeshSwizzleDWord(subSubsetAnm->triCount);
		}

		boneDataReadOnly * __restrict bonesData = msh.pBonesInfo.ptr;
		for (dword i = 0; i < msh.dwBonesCount; i++, bonesData++)
		{
			MeshSwizzleVector3(bonesData->bbMax);
			MeshSwizzleVector3(bonesData->bbMin);
			MeshSwizzleVector4(bonesData->bbSphereCenterAndRadius);
			MeshSwizzleVector4(bonesData->bbSphereRadius);
		}

		boneMtxInputReadOnly * __restrict bonesMtx = msh.pBonesMatrices.ptr;
		for (dword i = 0; i < msh.dwBonesCount; i++, bonesMtx++)
		{
			MeshSwizzleMatrix(bonesMtx->mtxBindPose);
		}


		blendShapeBoneTransform * __restrict bsBoneTransform = msh.pBlendShapeBonesTransform.ptr;
		for (dword i = 0; i < msh.dwBlendShapeBonesTransformCount; i++, bsBoneTransform++)
		{
			MeshSwizzleVector3(bsBoneTransform->p);
			MeshSwizzleQuaternion(bsBoneTransform->q);
			MeshSwizzleFloat(bsBoneTransform->weightScale);
		}

		
/*
		blendShapeTarget * __restrict bsMorphTarget = msh.pBlendShapeTargets.ptr;
		for (dword i = 0; i < msh.dwBlendShapeTargetsCount; i++, bsMorphTarget++)
		{
			//bsMorphTarget->frameBones
		}
		
		
		blendShapeBoneDescription * __restrict bsBoneDesc = msh.pBlendShapeBones.ptr;
		for (dword i = 0; i < msh.dwBlendShapeBonesCount; i++, bsBoneDesc++)
		{
			//bsBoneDesc->boneName
		}
*/


	} else
	{
		subsetStatic * __restrict staticSubset = msh.pStaticSubsets.ptr;
		for (dword i = 0; i < msh.dwSubsetsCount; i++, staticSubset++)
		{
			MeshSwizzleVector3(staticSubset->max);
			MeshSwizzleVector3(staticSubset->min);
			MeshSwizzleDWord(staticSubset->numVertexes);
			MeshSwizzleDWord(staticSubset->offsetInIndexBufferInInds);
			MeshSwizzleDWord(staticSubset->offsetInVertexBufferInBytes);
			MeshSwizzleVector3(staticSubset->sphereC);
			MeshSwizzleFloat(staticSubset->sphereR);
			MeshSwizzleDWord(staticSubset->triCount);

			MeshSwizzleDWord(staticSubset->dwNovodexCollisionSizePC);
			MeshSwizzleDWord(staticSubset->dwNovodexPMapSizePC);
			MeshSwizzleDWord(staticSubset->dwNovodexCollisionSizeXBOX360);
			MeshSwizzleDWord(staticSubset->dwNovodexPMapSizeXBOX360);
		}
	}







/*
	msh.pAllTransformations.fixupAsOffset(const_data);
	msh.pDynamicTransformations.fixupAsOffset(const_data);
*/

}
#endif



void MeshContainer::FixupPointers()
{
	if (msh.bAnimated)
	{
		msh.pCreatedPhysicMeshes.reset();
		msh.pMaterials.fixupAsOffset(const_data);
		msh.pRenderOrders.fixupAsOffset(const_data);
		msh.pLocators.fixupAsOffset(const_data);
		msh.pMeshes.fixupAsOffset(const_data);
		msh.pStaticSubsets.reset();
		msh.pAnimSubsets.fixupAsOffset(const_data);
		msh.pTranslateTables.fixupAsOffset(const_data);
		msh.pSubSubsets.fixupAsOffset(const_data);
		msh.pBonesInfo.fixupAsOffset(const_data);
		msh.pBonesMatrices.fixupAsOffset(const_data);
		msh.pBlendShapeBones.fixupAsOffset(const_data);
		msh.pBlendShapeTargets.fixupAsOffset(const_data);
		msh.pBlendShapeBonesTransform.fixupAsOffset(const_data);
		msh.pAllTransformations.fixupAsOffset(const_data);
		msh.pDynamicTransformations.fixupAsOffset(const_data);
	} else
	{
		msh.pCreatedPhysicMeshes.fixupAsOffset(const_data);
		msh.pMaterials.fixupAsOffset(const_data);
		msh.pRenderOrders.fixupAsOffset(const_data);
		msh.pLocators.fixupAsOffset(const_data);
		msh.pMeshes.fixupAsOffset(const_data);
		msh.pStaticSubsets.fixupAsOffset(const_data);
		msh.pAnimSubsets.reset();
		msh.pTranslateTables.reset();
		msh.pSubSubsets.reset();
		msh.pBonesInfo.reset();
		msh.pBonesMatrices.reset();
		msh.pBlendShapeBones.reset();
		msh.pBlendShapeTargets.reset();
		msh.pBlendShapeBonesTransform.reset();
		msh.pAllTransformations.fixupAsOffset(const_data);
		msh.pDynamicTransformations.fixupAsOffset(const_data);
	}

	MESH_ALIGN_16_CHECK(msh.pBlendShapeBonesTransform.ptr);


	for (dword i = 0; i < HASH_TABLE_SIZE; i++)
	{
		msh.entryTable[i].fixupAsIndex(msh.pLocators.ptr);
	}

	


#ifndef _XBOX
	SwizzleStaticData();
#endif


	dword dwMaterialsCount = msh.dwMaterialsCount;
	material * __restrict currentMaterial = msh.pMaterials.ptr;
	for (dword i = 0; i < dwMaterialsCount; i++, currentMaterial++)
	{
		currentMaterial->shaderName.fixup(const_data);
		currentMaterial->textureNames[0].fixup(const_data);
		currentMaterial->textureNames[1].fixup(const_data);
		currentMaterial->textureNames[2].fixup(const_data);
		currentMaterial->textureNames[3].fixup(const_data);
	}


	dword dwMeshesCount = msh.dwMeshesCount;
	mesh * __restrict cMesh = msh.pMeshes.ptr;
	for (dword i = 0; i < dwMeshesCount; i++, cMesh++)
	{
		cMesh->pTransform.fixupAsIndex(msh.pAllTransformations.ptr);
		cMesh->pParentTransform.fixupAsIndex(msh.pAllTransformations.ptr);
		cMesh->name.fixup(const_data);
	}
	cMesh = msh.pMeshes.ptr;


	dword dwLocatorsCount = msh.dwLocatorsCount;
	locator * __restrict cLocator = msh.pLocators.ptr;
	for (dword i = 0; i < dwLocatorsCount; i++, cLocator++)
	{
		cLocator->pTransform.fixupAsIndex(msh.pAllTransformations.ptr);
		cLocator->pParentTransform.fixupAsIndex(msh.pAllTransformations.ptr);

		cLocator->boneName.fixup(const_data);
		cLocator->particleName.fixup(const_data);
		cLocator->name.fixup(const_data);

		cLocator->pHashTableNext.fixupAsIndex(msh.pLocators.ptr);
	}



	if (msh.bAnimated)
	{
		
		nodeTransform * __restrict transform = msh.pAllTransformations.ptr;
		for (dword i = 0; i < msh.dwTransformationsCount; i++, transform++)
		{
			transform->debugNodeName.fixup(const_data);
			transform->boneName.fixup(const_data);
		}
		
		boneDataReadOnly * __restrict bonesData = msh.pBonesInfo.ptr;
		dword dwBonesCount = msh.dwBonesCount;
		for (dword i = 0; i < dwBonesCount; i++, bonesData++)
		{
			bonesData->name.fixup(const_data);
		}

		dword dwSubSubsetCount = msh.dwSubSubsetAnimCount;
		subSubsetAnim * __restrict subSubsetAnm = msh.pSubSubsets.ptr;
		for (dword i = 0; i < dwSubSubsetCount; i++, subSubsetAnm++)
		{
			subSubsetAnm->pTTableLocalToGlobal.fixupAsIndex(msh.pTranslateTables.ptr);
		}

		dword dwSubsetCount = msh.dwSubsetsCount;
		subsetAnim * __restrict animSubset = msh.pAnimSubsets.ptr;
		for (dword i = 0; i < dwSubsetCount; i++, animSubset++)
		{
			animSubset->pMaterial.fixupAsIndex(msh.pMaterials.ptr);
			animSubset->pSubSubsets.fixupAsIndex(msh.pSubSubsets.ptr);
//			animSubset->pBonesData.fixupAsIndex(msh.pBonesInfo.ptr);
//			animSubset->pBonesMatrices.fixupAsIndex(msh.pBonesMatrices.ptr);
		}

		for (dword i = 0; i < dwMeshesCount; i++, cMesh++)
		{
			cMesh->pAnimSubsets.fixupAsIndex(msh.pAnimSubsets.ptr);
		}

		dword dwOrdersCount = msh.dwOrdersCount;
		renderOrder * __restrict order = msh.pRenderOrders.ptr;
		for (dword i = 0; i < dwOrdersCount; i++, order++)
		{
			order->pMesh.fixupAsIndex(msh.pMeshes.ptr);
			
			//т.к. pSubset указатель на void то фиксим руками...
			//order->pSubset.fixupAsIndex(order->pMesh.ptr->pAnimSubsets.ptr);
			dword dwIdx = order->pSubset.index;
#ifndef _XBOX
			MeshSwizzleDWord(dwIdx);
#endif
			order->pSubset.ptr = &order->pMesh.ptr->pAnimSubsets.ptr[dwIdx];
		}

		dword dwBSMorphTargetCount = msh.dwBlendShapeTargetsCount;
		blendShapeTarget * __restrict bsFrame = msh.pBlendShapeTargets.ptr;
		for (dword i = 0; i < dwBSMorphTargetCount; i++, bsFrame++)
		{
			bsFrame->frameBones.fixupAsIndex(msh.pBlendShapeBonesTransform.ptr);
		}

		dword dwBSBonesCount = msh.dwBlendShapeBonesCount;
		blendShapeBoneDescription * __restrict bsBoneName = msh.pBlendShapeBones.ptr;
		for (dword i = 0; i < dwBSBonesCount; i++, bsBoneName++)
		{
			bsBoneName->boneName.fixup(const_data);
		}


	} else
	{


		for (dword i = 0; i < dwMeshesCount; i++, cMesh++)
		{
			cMesh->pStaticSubsets.fixupAsIndex(msh.pStaticSubsets.ptr);
		}

		//---

		nodeTransform * __restrict transform = msh.pAllTransformations.ptr;
		for (dword i = 0; i < msh.dwTransformationsCount; i++, transform++)
		{
			transform->debugNodeName.fixup(const_data);
			transform->boneName.clear();
		}

		dword dwSubsetCount = msh.dwSubsetsCount;
		subsetStatic * __restrict staticSubset = msh.pStaticSubsets.ptr;
		for (dword i = 0; i < dwSubsetCount; i++, staticSubset++)
		{
			staticSubset->pMaterial.fixupAsIndex(msh.pMaterials.ptr);
			//staticSubset->pCollision.fixupAsIndex(NULL);

#ifdef _XBOX
			staticSubset->pNovodexCollisionPC.reset();
			staticSubset->pNovodexPMapPC.reset();
			staticSubset->pNovodexCollisionXBOX360.fixupAsOffset(msh.collisionDataPlatform);
			staticSubset->pNovodexPMapXBOX360.fixupAsOffset(msh.collisionDataPlatform);
#else
			//Учитываем, что в загруженных данных вначале идут Xbox данные...
			byte* collisionDataPC = msh.collisionDataPlatform + msh.dwNovodexCollisionXBOX360_size;
			staticSubset->pNovodexCollisionPC.fixupAsOffset(collisionDataPC);
			staticSubset->pNovodexPMapPC.fixupAsOffset(collisionDataPC);
			staticSubset->pNovodexCollisionXBOX360.reset();
			staticSubset->pNovodexPMapXBOX360.reset();
#endif
		}


		dword dwOrdersCount = msh.dwOrdersCount;
		renderOrder * __restrict order = msh.pRenderOrders.ptr;
		for (dword i = 0; i < dwOrdersCount; i++, order++)
		{
			order->pMesh.fixupAsIndex(msh.pMeshes.ptr);

			//т.к. pSubset указатель на void то фиксим руками...
			//order->pSubset.fixupAsIndex(order->pMesh.ptr->pStaticSubsets.ptr);
			dword dwIdx = order->pSubset.index;
#ifndef _XBOX
			MeshSwizzleDWord(dwIdx);
#endif
			order->pSubset.ptr = &order->pMesh.ptr->pStaticSubsets.ptr[dwIdx];
		}
	}









}


void MeshContainer::AllocConstantData(dword dwBytesCount)
{
	DWORD dwBytesToAllocate = dwBytesCount + 16;
	const_data_raw = (byte *)meshAlloc(dwBytesToAllocate, _FL_);
	const_data = AlignPtr((const_data_raw));
	MESH_ALIGN_16_CHECK(const_data);
	//конструктор не нужен этим данным
}


void MeshContainer::ForceRelease ()
{
	Assert(refCount > 0);

	refCount = 1;
	Release ();
}

bool MeshContainer::Release ()
{
	Assert(refCount > 0);

	refCount--;
	if (refCount <= 0)
	{
		delete this;
		return true;
	}

	return false;
}

void MeshContainer::AddRef ()
{
	refCount++;
}



