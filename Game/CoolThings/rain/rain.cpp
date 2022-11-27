#include "..\..\..\Common_h/mission/Mission.h"
#include "Rain.h"


#define MG_EXPORTED_RAIN GroupId('e','x','r','n')

#define MAX_BB_COUNT 4096


IVBuffer* RainSFX::pVBufferStatic = NULL;
IIBuffer* RainSFX::pIBufferStatic = NULL;



__forceinline bool IsSphereVisible (const Plane *frustum, const Vector& SphereCenter, float SphereRadius)
{
	for(long p = 0; p < 5; p++)
		if(((SphereCenter | frustum[p].n) - frustum[p].d) < -SphereRadius) return false;

	return true;
}



__forceinline long VisCode(const Vector & vP, const Plane* pFrustumPlanes)
{
	long vc = 0;
	if(pFrustumPlanes[0].N.x*vP.x + pFrustumPlanes[0].N.y*vP.y + pFrustumPlanes[0].N.z*vP.z < pFrustumPlanes[0].D) vc |= 0x01;
	if(pFrustumPlanes[1].N.x*vP.x + pFrustumPlanes[1].N.y*vP.y + pFrustumPlanes[1].N.z*vP.z < pFrustumPlanes[1].D) vc |= 0x02;
	if(pFrustumPlanes[2].N.x*vP.x + pFrustumPlanes[2].N.y*vP.y + pFrustumPlanes[2].N.z*vP.z < pFrustumPlanes[2].D) vc |= 0x04;
	if(pFrustumPlanes[3].N.x*vP.x + pFrustumPlanes[3].N.y*vP.y + pFrustumPlanes[3].N.z*vP.z < pFrustumPlanes[3].D) vc |= 0x08;
	return vc;
}

__forceinline bool IsOBBVisible(const Vector & min, const Vector & max, const Matrix& mtx, const Plane* pFrustumPlanes)
{
	// check box visible
	long vc = 0xFF;
	vc &= VisCode(Vector(min.x, min.y, min.z)*mtx, pFrustumPlanes); if (vc == 0) return true;
	vc &= VisCode(Vector(min.x, max.y, min.z)*mtx, pFrustumPlanes); 
	vc &= VisCode(Vector(max.x, min.y, min.z)*mtx, pFrustumPlanes); 
	vc &= VisCode(Vector(max.x, max.y, min.z)*mtx, pFrustumPlanes); if (vc == 0) return true;
	vc &= VisCode(Vector(min.x, min.y, max.z)*mtx, pFrustumPlanes); 
	vc &= VisCode(Vector(min.x, max.y, max.z)*mtx, pFrustumPlanes); 
	vc &= VisCode(Vector(max.x, min.y, max.z)*mtx, pFrustumPlanes); 
	vc &= VisCode(Vector(max.x, max.y, max.z)*mtx, pFrustumPlanes); 

	return vc == 0;
}



//Конструктор
RainSFX::RainSFX() : Billboards(_FL_, 128)
														
{
	loadedData = NULL;
	needUpdateCache = 0;

	fMinSplashInterval = 0;
	fMaxSplashInterval = 0;

	bAllRainVisible = false;
	fSpeed = 10.0f;
	pRS = (IRender*)api->GetService("DX9Render");
	Assert(pRS);

	RainColor = pRS->GetTechniqueGlobalVariable("RainColor", _FL_);
	RainTexture = pRS->GetTechniqueGlobalVariable("RainTexture", _FL_);
	RainCameraPos = pRS->GetTechniqueGlobalVariable("RainCameraPos", _FL_);


	pTextureFrame1 = NULL;

	inplaceDropsBuffer = NULL;
	inplaceDropsDataCount = NULL;
	inplaceDropsData = NULL;

	splashesCount = 0;

}

//Деструктор
RainSFX::~RainSFX()
{	
	if (loadedData == NULL && inplaceDropsBuffer)
	{
		delete [] inplaceDropsBuffer;
		inplaceDropsBuffer = NULL;
		inplaceDropsDataCount = NULL;
		inplaceDropsData = NULL;
	}

	if (loadedData)
	{
		loadedData->Release();
		loadedData = NULL;
	}


	RainCameraPos = NULL;
	RainTexture = NULL;
	RainColor = NULL;

	bool bReleased = pVBufferStatic->Release();
	if (bReleased)
	{
		pVBufferStatic = NULL;
	}

	bReleased = pIBufferStatic->Release();
	if (bReleased)
	{
		pIBufferStatic = NULL;
	}


	if (pTextureFrame1) pTextureFrame1->Release();
	pTextureFrame1 = NULL;
}



void RainSFX::CreateBuffers ()
{

	int RectVertexSize =  sizeof(RECT_VERTEX);
	
	if (pVBufferStatic == NULL)
	{
		pVBufferStatic = pRS->CreateVertexBuffer(MAX_BB_COUNT * 4*RectVertexSize, RectVertexSize, _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
		Assert (pVBufferStatic != NULL);
	} else
	{
		pVBufferStatic->AddRef();
	}

	if (pIBufferStatic == NULL)
	{
		pIBufferStatic = pRS->CreateIndexBuffer(MAX_BB_COUNT * 6 * sizeof(WORD), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC); 
		Assert (pIBufferStatic != NULL);
		WORD * pTrgs = (WORD*)pIBufferStatic->Lock(); 
		Assert(pTrgs != NULL);
		for (long i = 0; i < MAX_BB_COUNT; i++)
		{
			pTrgs[i * 6 + 0] = WORD(i * 4 + 0);
			pTrgs[i * 6 + 1] = WORD(i * 4 + 1);
			pTrgs[i * 6 + 2] = WORD(i * 4 + 2);
			pTrgs[i * 6 + 3] = WORD(i * 4 + 0);
			pTrgs[i * 6 + 4] = WORD(i * 4 + 2);
			pTrgs[i * 6 + 5] = WORD(i * 4 + 3);
		}
		pIBufferStatic->Unlock();
	} else
	{
		pIBufferStatic->AddRef();
	}




}


//Создание объекта
bool RainSFX::Create(MOPReader & reader)
{
	MOSafePointer sp;
	static const ConstString objectId("RainSFXManager");
	Mission().CreateObject(sp,"RainSFXManager",objectId);
	RainSFXManager *rainSaver = (RainSFXManager *)sp.Ptr();

	Render().GetShaderId("RainWeather", RainWeather_id);

	CreateBuffers();

	EditMode_Update (reader);

#ifndef _XBOX
	if(EditMode_IsOn())
	{
		needUpdateCache = 2;
		Registry(MG_EXPORTED_RAIN);
	} else
	{
		LoadFromFile();
	}
#else
	LoadFromFile();
#endif


	


	return true;
}

void RainSFX::LoadTexture(const char* szTex1)
{
	if (sTextureName1 != szTex1)
	{
		if (pTextureFrame1) pTextureFrame1->Release();
		pTextureFrame1 = pRS->CreateTexture(_FL_, "%s", szTex1);

		sTextureName1 = szTex1;
	}
}


void RainSFX::GenerateRandom()
{
	Billboards.DelAll();

	for (dword n = 0; n < dropsCount; n++)
	{
		BillBoard* bb = &Billboards[Billboards.Add()];
		bb->vPos.Rand(-vSize, vSize);
		bb->fSizeX = RRnd (0.01f, 0.02f) * dropX_mul;
		bb->fSizeY = RRnd (0.1f, 0.2f) * dropY_mul;
	}

}

void RainSFX::Update(float fDeltaTime)
{
	if (fDeltaTime > 0.05f)
	{
		fDeltaTime = 0.05f;
	}

	for (dword n = 0; n < Billboards.Size(); n++)
	{
		BillBoard* bb = &Billboards[n];

		//if (GetAsyncKeyState(VK_CONTROL) < 0)
		{
			bb->vPos += (Vector(0.0f, -fSpeed, 0.0f) * fDeltaTime);
		}

		float d = -vSize.y - bb->vPos.y;
		if (d >= 0.0f)
		{
			bb->vPos.Rand(Vector(-vSize.x, vSize.y + d, -vSize.z), Vector(vSize.x, vSize.y + d, vSize.z));
			bb->fSizeX = RRnd (0.01f, 0.02f) * dropX_mul;
			bb->fSizeY = RRnd (0.1f, 0.2f) * dropY_mul;
		}
	}
}

//Обновление параметров
bool RainSFX::EditMode_Update(MOPReader & reader)
{
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	mWorld.Build(ang, pos);

	vSize.x = reader.Float();
	vSize.y = reader.Float();
	vSize.z = reader.Float();



	LoadTexture (reader.String().c_str());

	dropsCount = reader.Long();

	dropX_mul = reader.Float();
	dropY_mul = reader.Float();

	fSpeed = reader.Float();

	vColor = reader.Colors().v4;
	bShow = reader.Bool();
	bActive = reader.Bool();
	bDebugInfo = reader.Bool();

	splashesCount = reader.Long();

	fMinSplashInterval = reader.Float();
	fMaxSplashInterval = reader.Float();

	waterParticlesSystem = reader.String();
	groundParticlesSystem = reader.String();

	bIgnoreMaterials =reader.Bool();

	ChangeStatus();


	GenerateRandom();


	needUpdateCache = 2;

	return true;
}

void RainSFX::ChangeStatus ()
{
	if (bShow && bActive)
	{
		SetUpdate((MOF_UPDATE)&RainSFX::Realize, ML_PARTICLES5);
		return;
	}

	DelUpdate((MOF_UPDATE)&RainSFX::Realize);


}




void _cdecl RainSFX::Realize(float fDeltaTime, long level)
{

	if (loadedData == NULL && needUpdateCache > 0)
	{
		CreateInplaceBuffer(splashesCount);
		GenerateInplaceBuffer();
		needUpdateCache = needUpdateCache - 1;
	}

	
	if (!EditMode_IsVisible()) return;

/*
	if(Mission().EditMode_IsAdditionalDraw() && EditMode_IsOn())
	{
		Render().DrawBox(-vSize, vSize, mWorld, Color(vColor).GetDword());
	}
*/

	const Plane* frustum = Render().GetFrustum();



	bAllRainVisible = IsOBBVisible(-vSize, vSize, mWorld, frustum);



	if (bDebugInfo)
	{
		Render().Print(0.0f, 0.0f, 0xFFFFFFFF, "Rain drops: %d, Is visible [%d]", Billboards.Size(), bAllRainVisible);
	}

	if (!bAllRainVisible)
	{
		return;
	}



	Update(fDeltaTime);




	RECT_VERTEX * pVerts = (RECT_VERTEX*)pVBufferStatic->Lock(0, 0, LOCK_DISCARD);

	if (!pVerts) return;

	long Index = 0;
	DWORD RenderBillboardsCount = 0;
	for (DWORD j = 0; j <  Billboards.Size(); j++)
	{
		BillBoard & b = Billboards[j];
		const Vector& vPos = b.vPos;

		RECT_VERTEX	* pV = &pVerts[Index * 4];
		Index++;

		float fSizeX = b.fSizeX;
		float fSizeY = b.fSizeY;

		if (j & 1)
		{
			pV[0].vGlobalPos = Vector(vPos.x - fSizeX, vPos.y - fSizeY, vPos.z);
			pV[1].vGlobalPos = Vector(vPos.x - fSizeX, vPos.y + fSizeY, vPos.z);
			pV[2].vGlobalPos = Vector(vPos.x + fSizeX, vPos.y + fSizeY, vPos.z);
			pV[3].vGlobalPos = Vector(vPos.x + fSizeX, vPos.y - fSizeY, vPos.z);
		} else
		{
			pV[0].vGlobalPos = Vector(vPos.x, vPos.y - fSizeY, vPos.z - fSizeX);
			pV[1].vGlobalPos = Vector(vPos.x, vPos.y + fSizeY, vPos.z - fSizeX);
			pV[2].vGlobalPos = Vector(vPos.x, vPos.y + fSizeY, vPos.z + fSizeX);
			pV[3].vGlobalPos = Vector(vPos.x, vPos.y - fSizeY, vPos.z + fSizeX);
		}

		pV[0].tu1 = 0;		pV[0].tv1 = 0;
		pV[1].tu1 = 0;		pV[1].tv1 = 32767;
		pV[2].tu1 = 32767;	pV[2].tv1 = 32767;
		pV[3].tu1 = 32767;	pV[3].tv1 = 0;
		/*pV[0].tu1 = 0.0f; pV[0].tv1 = 0.0f;
		pV[1].tu1 = 0.0f; pV[1].tv1 = 1.0f;
		pV[2].tu1 = 1.0f; pV[2].tv1 = 1.0f;
		pV[3].tu1 = 1.0f; pV[3].tv1 = 0.0f;*/

		RenderBillboardsCount++;
	}

	pVBufferStatic->Unlock();


	if (RainColor)
	{
		RainColor->SetVector4(vColor);
	}

	if (RainCameraPos)
	{
		Vector vCamPos = Render().GetView().GetCamPos();
		RainCameraPos->SetVector(vCamPos);
	}

	if (pTextureFrame1)
	{
		RainTexture->SetTexture(pTextureFrame1);
	} else
	{
		RainTexture->ResetTexture();
	}

	pRS->SetWorld(mWorld);
	pRS->SetStreamSource(0, pVBufferStatic);
	pRS->SetIndices(pIBufferStatic, 0);

	pRS->DrawIndexedPrimitive(RainWeather_id, PT_TRIANGLELIST, 0, RenderBillboardsCount * 4, 0, RenderBillboardsCount * 2);


	RenderInplaceBuffer(fDeltaTime);
}

void RainSFX::Show(bool isShow)
{
	bShow = isShow;
	ChangeStatus();
}

void RainSFX::Activate(bool isActive)
{
	bActive = isActive;
	ChangeStatus();
}


//Получить размеры описывающего ящика
void RainSFX::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	GetBox(min, max);
}

//Получить бокс, описывающий объект в локальных координатах
void RainSFX::GetBox(Vector & min, Vector & max)
{
	min = -vSize;
	max = vSize;
}

//Получить матрицу объекта
Matrix & RainSFX::GetMatrix(Matrix & mtx)
{
	mtx = mWorld;

	return mtx;
};

void RainSFX::RenderInplaceBuffer(float fDeltaTime)
{
	if (!bAllRainVisible)
	{
		return;
	}

	const Plane * frustum = Render().GetFrustum();

	dword dwSlotsCount = *inplaceDropsDataCount;

	for (dword i = 0; i < dwSlotsCount; i++)
	{
		rainDropSFX & r = inplaceDropsData[i];

		if (r.dwEnabled == 0)
		{
			continue;
		}

		//Render().DrawLine(r.tmp1, 0xFFFFFFFF, r.tmp2, 0xFFFFFFFF);

		//Пропускаем невидимые точки рождения
		if (!IsSphereVisible(frustum, r.vPos, 0.8f))
		{
			continue;
		}

		r.fTime -= fDeltaTime;

		if (r.fTime <= 0.0f)
		{
			ConstString * fileName = NULL;

			switch (r.dwType)
			{
			case 0:
				fileName = &groundParticlesSystem;
				break;
			case 1:
				fileName = &waterParticlesSystem;
				break;
			}

			if (fileName)
			{
				Particles().CreateParticleSystemEx2(fileName->c_str(), Matrix(Vector(0.0f), r.vPos), true, _FL_);
			}

			r.fTime = RRnd(fMinSplashInterval, fMaxSplashInterval);
		}

	}

	

}


void RainSFX::GenerateInplaceBuffer ()
{
	//Данные не должны быть загружены из файла, что то не так...
	Assert (loadedData == NULL);

	if (!inplaceDropsBuffer)
	{
		return;
	}

	dword dwSlotsCount = *inplaceDropsDataCount;


	Vector vRandUp;
	Vector vRandDown;
	IPhysicsScene::RaycastResult res;
	for (dword i = 0; i < dwSlotsCount; i++)
	{
		vRandUp.Rand(-vSize, vSize);
		vRandUp.y = vSize.y;

		vRandDown = vRandUp;
		vRandDown.y = -vSize.y;

		vRandUp = vRandUp * mWorld;
		vRandDown = vRandDown * mWorld;

		IPhysBase * physobject = Physics().Raycast(vRandUp, vRandDown, phys_mask(phys_bloodpatch), &res);

		if (physobject)
		{
			if (!bIgnoreMaterials)
			{
				if (res.mtl == pmtlid_air)
				{
					inplaceDropsData[i].vPos = vRandUp;
					inplaceDropsData[i].dwEnabled = 0;
				} else
				{
					inplaceDropsData[i].vPos = res.position;
					inplaceDropsData[i].dwEnabled = 1;

					if (res.mtl == pmtlid_water)
					{
						inplaceDropsData[i].dwType = 1;
					} else
					{
						inplaceDropsData[i].dwType = 0;
					}
				}
			} else
			{
				inplaceDropsData[i].vPos = res.position;
				inplaceDropsData[i].dwEnabled = 1;
				inplaceDropsData[i].dwType = 0;
			}



		} else
		{
			inplaceDropsData[i].vPos = vRandUp;
			inplaceDropsData[i].dwEnabled = 0;
		}

		//inplaceDropsData[i].tmp1 = vRandUp;
		//inplaceDropsData[i].tmp2 = vRandDown;
		inplaceDropsData[i].fTime = RRnd(fMinSplashInterval, fMaxSplashInterval);
		inplaceDropsData[i].dwType = 0;
	}

}

dword RainSFX::GetInplaceBufferSize (dword dwNumDrops)
{
	dword inplaceDropsBufferSize = dwNumDrops * sizeof(rainDropSFX) + sizeof(dword);
	return inplaceDropsBufferSize;
}


void RainSFX::CreateInplaceBuffer (dword dwNumDrops)
{
	dword inplaceDropsBufferSize = GetInplaceBufferSize(dwNumDrops);

	if (inplaceDropsBuffer)
	{
		delete [] inplaceDropsBuffer;
		inplaceDropsBuffer = NULL;
		inplaceDropsDataCount = NULL;
		inplaceDropsData = NULL;
	}

	inplaceDropsBuffer = NEW byte [inplaceDropsBufferSize];

	//Тут заголовок
	inplaceDropsDataCount = (dword *)inplaceDropsBuffer;

	*inplaceDropsDataCount = dwNumDrops;

	//Тут данные
	inplaceDropsData = (rainDropSFX *)(inplaceDropsDataCount + 1);
}

bool RainSFX::LoadFromFile()
{


	if (inplaceDropsBuffer)
	{
		delete [] inplaceDropsBuffer;
		inplaceDropsBuffer = NULL;
		inplaceDropsDataCount = NULL;
		inplaceDropsData = NULL;
	}

	string loadPath;
	loadPath = "resource\\missions\\";
	loadPath += Mission().GetMissionName();
	loadPath += "\\data\\";
	
	loadPath += Mission().GetMissionName();
	loadPath += "_";

	loadPath += GetObjectID().c_str();

#ifdef _XBOX
	loadPath += ".x360";
#else
	loadPath += ".pc";
#endif

	loadedData = Files().LoadData(loadPath.c_str(), _FL_);


	//Max нету, потомучто капли не падают на патч, падает движёк. Вот и не генериться
	if(!loadedData)
	{
		return false;
	}


	//Как это блин нет файла !!! он должен был сгенерится !!!
	Assert (loadedData);


	inplaceDropsBuffer = loadedData->Buffer();

	//Тут заголовок
	inplaceDropsDataCount = (dword *)inplaceDropsBuffer;

	//Тут данные
	inplaceDropsData = (rainDropSFX *)(inplaceDropsDataCount + 1);

	return true;
}

bool RainSFX::SaveToFile()
{
#ifndef _XBOX
	string exportPathPC;
	exportPathPC = "resource\\missions\\";
	exportPathPC += Mission().GetMissionName();
	exportPathPC += "\\rainCooked\\";
	exportPathPC += Mission().GetMissionName();
	exportPathPC += "_";
	exportPathPC += GetObjectID().c_str();
	exportPathPC += ".pc";

	string exportPathX360;
	exportPathX360 = "resource\\missions\\";
	exportPathX360 += Mission().GetMissionName();
	exportPathX360 += "\\rainCooked\\";
	exportPathX360 += Mission().GetMissionName();
	exportPathX360 += "_";
	exportPathX360 += GetObjectID().c_str();
	exportPathX360 += ".x360";

	//Max
	if(!inplaceDropsDataCount)
	{
		return true;
	}

	dword fileBufferSize = GetInplaceBufferSize(*inplaceDropsDataCount);
	byte * fileBuferPC = NEW byte [fileBufferSize];
	byte * fileBuferX360 = NEW byte [fileBufferSize];


	dword dwSavedBufLen = *inplaceDropsDataCount;
	

	dword * dwBufPC = (dword*)fileBuferPC;
	dword * dwBufX360 = (dword*)fileBuferX360;
	*dwBufPC = dwSavedBufLen;
	__RefDataSwizzler(dwSavedBufLen);
	*dwBufX360 = dwSavedBufLen;

	dwBufPC += 1;
	rainDropSFX * rdBufPC = (rainDropSFX *)dwBufPC;

	dwBufX360 += 1;
	rainDropSFX * rdBufX360 = (rainDropSFX *)dwBufX360;


	for (dword i = 0; i < *inplaceDropsDataCount; i++)
	{
		rdBufPC[i] = inplaceDropsData[i];
		rdBufX360[i] = inplaceDropsData[i];
	
		__RefDataSwizzler(rdBufX360[i].dwType);

		__RefDataSwizzler(rdBufX360[i].fTime);
		__RefDataSwizzler(rdBufX360[i].vPos.x);
		__RefDataSwizzler(rdBufX360[i].vPos.y);
		__RefDataSwizzler(rdBufX360[i].vPos.z);
	}


	
	Files().SaveData(exportPathPC.c_str(), fileBuferPC, fileBufferSize);
	Files().SaveData(exportPathX360.c_str(), fileBuferX360, fileBufferSize);


	delete [] fileBuferPC;
	delete [] fileBuferX360;

#endif
	return true;

}




RainSFXManager::RainSFXManager()
{

}

RainSFXManager::~RainSFXManager()
{

}

bool RainSFXManager::Create(MOPReader & reader)
{
	string folder;
	folder = "resource\\missions\\";
	folder += Mission().GetMissionName();
	
	
	folder += "\\";
	folder += Mission().GetMissionName();
	folder += "_";


#ifdef _XBOX
	folder += "rainCooked.x";
#else
	folder += "rainCooked.p";
#endif
	
	Files().LoadPack(folder.c_str(), _FL_);
	return true;

}

bool RainSFXManager::EditMode_Create(MOPReader & reader)
{
	if(EditMode_IsOn())
	{
		Registry(MG_EXPORT);
	}

	return true;
}

#ifndef _XBOX
void RainSFXManager::RecursiveDeleteFolder(const char* szPath)
{
	IFinder * finder = Files().CreateFinder(szPath, "*", find_all_files_no_mirrors, _FL_);

	for(dword j = 0; j < finder->Count(); j++)
	{
		if (finder->IsMirror(j))
		{
			continue;
		}

		if (finder->IsFolder(j))
		{
			continue;
		}

		const char * fileName = finder->FilePath(j);
		BOOL bRes = DeleteFile(fileName);
	
	}


	finder->Release();

	RemoveDirectory(szPath);
	
}
void RainSFXManager::BuildPack()
{
	MGIterator & groupIter = Mission().GroupIterator(MG_EXPORTED_RAIN, _FL_);

	while( !groupIter.IsDone() )
	{
		RainSFX* pRain = (RainSFX*)groupIter.Get();
		pRain->SaveToFile();
		groupIter.Next();
	}
	groupIter.Release();


	string folder;
	folder = "resource\\missions\\";
	folder += Mission().GetMissionName();
	folder += "\\";



	string name_pc;
	name_pc = Mission().GetMissionName();
	name_pc += "_rainCooked.p";
	

	string name_x360;
	name_x360 = Mission().GetMissionName();
	name_x360 += "_rainCooked.x";


	Files().BuildPack(name_pc.c_str(), folder.c_str(), "*.pc", pack_cmpr_method_archive);
	Files().BuildPack(name_x360.c_str(), folder.c_str(), "*.x360", pack_cmpr_method_archive);

	string temp;
	temp = folder + "rainCooked\\";

	RecursiveDeleteFolder(temp.c_str());

}
#endif



MOP_BEGINLIST(RainSFXManager, "", '1.00', 0)
MOP_ENDLIST(RainSFXManager)




MOP_BEGINLIST(RainSFX, "Rain weather", '1.00', 900)
	MOP_POSITIONC("Position", Vector(0.0f), "Позиция дождевого эмитера (только для отрисовки)")
	MOP_ANGLESC("Angles", Vector(0.0f), "Углы дождевого эмитера (только для отрисовки)")
	MOP_FLOATC("Length", 10.0f, "Размеры дождевого эмитера по X")
	MOP_FLOATC("Height", 8.0f, "Высота, пролетев которую капли умирают")
	MOP_FLOATC("Width",  5.0f, "Размеры дождевого эмитера по X")

	MOP_STRING("Texture", "")
	MOP_LONGEXC("DropsCount", 1024, 128, 4096, "Кол-во капель дождя")
	MOP_FLOATC("Drop Width Scale", 1.0f, "Масштаб капель по ширине")
	MOP_FLOATC("Drop Length Scale", 1.0f, "Масштаб капель по длинне")
	MOP_FLOATC("Speed", 20.0f, "Скорость падения капель")
	MOP_COLOR("Color", Color(1.0f, 1.0f, 1.0f))
	MOP_BOOL("Show", true)
	MOP_BOOL("Active", true)
	MOP_BOOL("DebugInfo", false)

	MOP_LONGEXC("SplashesCount", 32, 0, 512, "Кол-во партиклов брызг")
	MOP_FLOATC("Min splah interval", 0.2f, "Минимальное появление капель в секундах")
	MOP_FLOATC("Max splah interval", 2.0f, "Максимальное появление капель в секундах")
	MOP_STRING("WaterSplashParticle", "")
	MOP_STRING("GroundSplashParticle", "")

	MOP_BOOL("Ignore Patch Materials", false)

MOP_ENDLIST(RainSFX)



