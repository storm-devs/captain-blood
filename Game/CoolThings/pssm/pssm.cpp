#include "pssm.h"

#define SHADOWMAP_GROUP GroupId('S','D','M','P')

// clamp variables of any type
template<class Type> inline Type Clamp3(Type A, Type Min, Type Max)
{
  if (A<Min) return Min;
  if (A>Max) return Max;
  return A;
}

bool OverlapFrustumOBB(const Frustum & frustum, const Matrix & mtx, const Vector4 & min, const Vector4 & max);

PSSMHolder PSShadowMap::PH;

PSSMHolder::PSSMHolder()
{
	iRef = 0;

	//pIViewProj = null;
	pIZBias = null;

	//pIShadowDistance = null;
	//pIShadowDistanceStart = null;
	pIShading = null;
	
	pIShadowColor = null;

	pISplits = null;
	
	pIShadowMapMatrix = null;
	pIShadowMapTexture = null;

	pILightDir = null;

	pIShadowPower = null;
	pIShdK = null;	

	//pILightView = null;
	pILightViewOnlyZ = null;
	//pITestUShift = null;

	pITexOffsets = null;

	m_projNum = 0;
	m_projBuffer = null;
	m_projMinMax = null;

	aSCObjects = null;
}

PSSMHolder::~PSSMHolder()
{
}

void PSSMHolder::Init()
{
	//pIViewProj = Render().GetTechniqueGlobalVariable("mPSSM_ViewProj", _FL_);
	pIZBias = Render().GetTechniqueGlobalVariable("vPSSMZBias", _FL_);

	varLinearZTex = Render().GetTechniqueGlobalVariable("tPSSM_linearZTexture", _FL_);

	pIShading = Render().GetTechniqueGlobalVariable("vPSSMShading", _FL_);
	//pIShadowDistance = Render().GetTechniqueGlobalVariable("fPSSMShadowDistance", _FL_);
	//pIShadowDistanceStart = Render().GetTechniqueGlobalVariable("fPSSMShadowDistanceStart", _FL_);
	
	//pILightView = Render().GetTechniqueGlobalVariable("mPSSM_LightView", _FL_);
	pILightViewOnlyZ = Render().GetTechniqueGlobalVariable("v4PSSM_LightViewOnlyZ", _FL_);
	//pITestUShift = Render().GetTechniqueGlobalVariable("fPSSMTestUShift", _FL_);
	pIShadowColor = Render().GetTechniqueGlobalVariable("vPSSMShadowColor", _FL_);

	pISplits = Render().GetTechniqueGlobalVariable("v4PSSMSplits", _FL_);

	m_varCamPos = Render().GetTechniqueGlobalVariable("vPSSMCamPos", _FL_);

	pIShadowMapMatrix = Render().GetTechniqueGlobalVariable("mPSSM_ShadowMap", _FL_);
	pIShadowMapTexture = Render().GetTechniqueGlobalVariable("tPSSM_ShadowMap", _FL_);

	pVarTexParams = Render().GetTechniqueGlobalVariable("pssm_TexParams", _FL_);
	
	pILightDir = Render().GetTechniqueGlobalVariable("vPSSMLightDir", _FL_);

	pIShadowPower = Render().GetTechniqueGlobalVariable("fPSSMShadowPow", _FL_);	

	pIShdK = Render().GetTechniqueGlobalVariable("vPSSMshdK", _FL_);	

	pITexOffsets = Render().GetTechniqueGlobalVariable("vPSSM_TexOffsets", _FL_);

	pShdServ = (IShadowsService *)api->GetService("ShadowsService");

	pGeo->GetRenderModeShaderId("PSSM_PROJ_CAST", PSSM_PROJ_CAST_id);
	pGeo->GetRenderModeShaderId("PSSM_MT_CAST", PSSM_MT_CAST_id);
	pGeo->GetRenderModeShaderId("PSSM_RECV_SOLID", PSSM_RECV_SOLID_id);
	pGeo->GetRenderModeShaderId("PSSM_ST_RECV", PSSM_ST_RECV_id);

	pGeo->GetRenderModeShaderId("PSSM_ST_CAST", PSSM_ST_CAST_id);
	pGeo->GetRenderModeShaderId("PSSM_ST_CASTNPS", PSSM_ST_CASTNPS_id);

	Render().GetShaderId("PSSM_ZPass", zpass_id);
	Render().GetShaderId("PSSM_ZPass_WireFrame", zpasswire_id);

	Render().GetShaderId("DefShade", defShade_id);

	//pGeo->GetRenderModeShaderId("PSSM_ZPass_Recv", PSSM_ZPass_Recv_id);
	//pGeo->GetRenderModeShaderId("PSSM_ZPass_Recv_Alpha", PSSM_ZPass_Recv_Alpha_id);

	aSCObjects = new array<SCOData>(_FL_, 64);
	aSCObjects->AddElements(128);
}

bool PSSMHolder::IsInited() const
{
	return (iRef > 0);
}

void PSSMHolder::AddRef()
{
	// Инициализируем если еще не было инициализации
	if (iRef == 0) Init();

	iRef++;
}

void PSSMHolder::Release()
{
	Assert(iRef);

	iRef--;
	if (iRef == 0) 
	{
		//RELEASE(pIViewProj);
		pIZBias = NULL;

		//RELEASE(pIShadowDistance);
		//RELEASE(pIShadowDistanceStart);
		pIShading = NULL;
		
		//RELEASE(pILightView);
		pILightViewOnlyZ = NULL;
		//RELEASE(pITestUShift);
		
		pIShadowColor = NULL;
		
		pISplits = NULL;
		m_varCamPos = null;

		pIShadowMapMatrix = NULL;
		pIShadowMapTexture = NULL;

		pILightDir = NULL;

		pIShadowPower = NULL;
		pIShdK = NULL;

		pITexOffsets = NULL;
	
		m_projNum = 0;
		DELETE(m_projBuffer);
		DELETE(aSCObjects);
	}
}

PSShadowMap::PSShadowMap() : 
	aPntLights(_FL_),
	m_clipPlanes(_FL_, 1)
{
	bPointLight = false;
	bErrorCreation = true;
	iNumRecv = 0;
	pIterSRecv = null;

	m_zPassVB = null;
	m_zPassIB = null;

	// резервируем кучу элементов
	//aSCObjects.AddElements(80);
	//aSCObjects.Empty();
}

PSShadowMap::~PSShadowMap()
{
	PH.Release();

	RELEASE(m_zPassVB);
	RELEASE(m_zPassIB);

	RELEASE(pIterSRecv);
}

//Создание объекта
bool PSShadowMap::Create(MOPReader & reader)
{
	Registry(SHADOWMAP_GROUP);

	PH.pRS = &Render();
	PH.pGeo = &Geometry();
	PH.AddRef();

	pIterSRecv = &Mission().GroupIterator(MG_SHADOWRECEIVE, _FL_);

	return EditMode_Update(reader);
}

void PSShadowMap::RegisterUnregisterPixEvents ()
{
	if (EditMode_IsOn()) return;

	if (IsActive())
	{
		SetUpdate((MOF_UPDATE)&PSShadowMap::Begin0, ML_GEOMETRY1-1);
		SetUpdate((MOF_UPDATE)&PSShadowMap::End0, ML_GEOMETRY5+1);

		SetUpdate((MOF_UPDATE)&PSShadowMap::Begin1, ML_DYNAMIC1-1);
		SetUpdate((MOF_UPDATE)&PSShadowMap::End1, ML_DYNAMIC5+1);

		SetUpdate((MOF_UPDATE)&PSShadowMap::Begin2, ML_ALPHA1-1);
		SetUpdate((MOF_UPDATE)&PSShadowMap::End2, ML_ALPHA5+1);

		SetUpdate((MOF_UPDATE)&PSShadowMap::Begin3, ML_WEATHER1-1);
		SetUpdate((MOF_UPDATE)&PSShadowMap::End3, ML_WEATHER5+1);

		SetUpdate((MOF_UPDATE)&PSShadowMap::Begin4, ML_GUI1-1);
		SetUpdate((MOF_UPDATE)&PSShadowMap::End4, ML_GUI5+1);

	} else
	{
		DelUpdate(ML_GEOMETRY1-1, (MOF_UPDATE)&PSShadowMap::Begin0);
		DelUpdate(ML_GEOMETRY5+1, (MOF_UPDATE)&PSShadowMap::End0);

		DelUpdate(ML_DYNAMIC1-1, (MOF_UPDATE)&PSShadowMap::Begin1);
		DelUpdate(ML_DYNAMIC5+1, (MOF_UPDATE)&PSShadowMap::End1);

		DelUpdate(ML_ALPHA1-1, (MOF_UPDATE)&PSShadowMap::Begin2);
		DelUpdate(ML_ALPHA5+1, (MOF_UPDATE)&PSShadowMap::End2);

		DelUpdate(ML_WEATHER1-1, (MOF_UPDATE)&PSShadowMap::Begin3);
		DelUpdate(ML_WEATHER5+1, (MOF_UPDATE)&PSShadowMap::End3);

		DelUpdate(ML_GUI1-1, (MOF_UPDATE)&PSShadowMap::Begin4);
		DelUpdate(ML_GUI5+1, (MOF_UPDATE)&PSShadowMap::End4);
	}
}

void _cdecl PSShadowMap::Begin0(float fDeltaTime, long level)
{
	dwId0 = Render().pixBeginEvent(_FL_, "Mission::ML_GEOMETRY" );
}

void _cdecl PSShadowMap::End0(float fDeltaTime, long level)
{
	Render().pixEndEvent(_FL_, dwId0);
}

void _cdecl PSShadowMap::Begin1(float fDeltaTime, long level)
{
	dwId1 = Render().pixBeginEvent(_FL_, "Mission::ML_DYNAMIC" );
}

void _cdecl PSShadowMap::End1(float fDeltaTime, long level)
{
	Render().pixEndEvent(_FL_, dwId1);
}

void _cdecl PSShadowMap::Begin2(float fDeltaTime, long level)
{
	dwId2 = Render().pixBeginEvent(_FL_, "Mission::ML_ALPHA" );
}

void _cdecl PSShadowMap::End2(float fDeltaTime, long level)
{
	Render().pixEndEvent(_FL_, dwId2);
}

void _cdecl PSShadowMap::Begin3(float fDeltaTime, long level)
{
	dwId3 = Render().pixBeginEvent(_FL_, "Mission::ML_WEATHER" );
}

void _cdecl PSShadowMap::End3(float fDeltaTime, long level)
{
	Render().pixEndEvent(_FL_, dwId3);
}

void _cdecl PSShadowMap::Begin4(float fDeltaTime, long level)
{
	dwId4 = Render().pixBeginEvent(_FL_, "Mission::ML_GUI" );
}

void _cdecl PSShadowMap::End4(float fDeltaTime, long level)
{
	Render().pixEndEvent(_FL_, dwId4);
}



//Обновление параметров
bool PSShadowMap::EditMode_Update(MOPReader & reader)
{
	fLightNear = 0.4f;
	fLightFar = 400.0f; 
	fLightFarMax = 400.0f;
	fLightFOV = 150.0f;

	fCameraAspect = 1024.0f / 768.0f;
	fCameraFOV = 71.0f;

	fCameraNear = 0.4f;
	fCameraFar = 400.0f; 
	fCameraFarMax = 400.0f;

	Vector vAngles = reader.Angles();
	cShadowColor = reader.Colors();
	fShdPower = reader.Float();
	fCharacterShdPower = reader.Float();
	fShdK1 = reader.Float();
	fShdK2 = reader.Float();
	fBlur = reader.Float();
	fBlurAngle = reader.Float() * PIm2 / 360.0f;
	iNumSplits = reader.Long();
	iNumSplits = 3;
	fZBias = reader.Float() * 0.1f * 0.25f;
	fCharacterZBias = reader.Float() * 0.1f * 0.25f;
	fShadowDistance = reader.Float();
	fSplitSchemeLambda = reader.Float();
	fNearZ = reader.Float();
	fMinVolumeY = reader.Float();
	fMaxVolumeY = reader.Float();
	bPointLight = reader.Bool();

	aPntLights.Empty();
	long iNumLights = reader.Array();
	for (long i=0; i<iNumLights; i++)
	{
		PntLight & pl = aPntLights[aPntLights.Add()];
		
		pl.cName = reader.String();
		pl.fSize = reader.Float();
		pl.light.Reset();
	}

	lightMtx = Matrix(vAngles);
	lightDir = lightMtx.vz;

	m_numZPassPlanes = reader.Array();
	if (m_numZPassPlanes)
	{
		// релизим старые IB/VB
		RELEASE(m_zPassVB);
		RELEASE(m_zPassIB);

		m_zPassVB = Render().CreateVertexBuffer(m_numZPassPlanes * 4 * sizeof(Vector), sizeof(Vector), _FL_);
		m_zPassIB = Render().CreateIndexBuffer(m_numZPassPlanes * 2 * 3 * sizeof(word), _FL_);

		Vector * verts = (Vector *)m_zPassVB->Lock();
		for (long i=0; i<m_numZPassPlanes; i++)
		{
			Vector pos = reader.Position();
			Vector angles = reader.Angles();
			float w = reader.Float() * 0.5f;
			float l = reader.Float() * 0.5f;

			Matrix mat(angles, pos);

			*verts++ = Vector(-l, 0.0f, -w) * mat;
			*verts++ = Vector(l, 0.0f, -w) * mat;
			*verts++ = Vector(l, 0.0f, w) * mat;
			*verts++ = Vector(-l, 0.0f, w) * mat;
		}
		m_zPassVB->Unlock();

		word * pI = (word *)m_zPassIB->Lock();
		for (long i=0; i<m_numZPassPlanes; i++)
		{
			*pI++ = word((i * 4) + 0);	*pI++ = word((i * 4) + 1);	*pI++ = word((i * 4) + 2);
			*pI++ = word((i * 4) + 0);	*pI++ = word((i * 4) + 2);	*pI++ = word((i * 4) + 3);
		}
		m_zPassIB->Unlock();
	}

	int numClipPlanes = reader.Array();
	m_clipPlanes.Empty().AddElements(numClipPlanes);
	for (int i=0; i<numClipPlanes; i++)
	{
		Vector pos = reader.Position();
		Vector angles = reader.Angles() + Vector(PI / 2.0f, 0.0f, 0.0f);
		Matrix mtx(angles, 0.0f);
		m_clipPlanes[i].pos = pos;
		m_clipPlanes[i].angles = angles;
		m_clipPlanes[i].plane = Plane(mtx.vz, pos);
	}

	bShowDebug = reader.Bool();
	Activate(reader.Bool());

	// рассчитываем новый ZBias исходя из разрешения и формата пиксела
	float multiplier = 0.1f;
	dword dwSize = PH.pShdServ->GetShadowMapRes();
	if (dwSize >= 2048) multiplier = 1.0f;
	else if (dwSize >= 1024) multiplier = 1.0f;
	else if (dwSize >= 512) multiplier = 3.0f;
	else multiplier = 4.0f;

	// делаем подальше, потому что стало все гораздо круче
	fShadowDistance = Max(30.0f, fShadowDistance);
	
	// УБЕЙМЕНЯНАУ!
	//fShadowDistance = 100.0f;

	fZBias *= multiplier;
	fCharacterZBias *= multiplier;

	if (bErrorCreation)
	{
		fOldMinVolumeY = fMinVolumeY;
		fOldMaxVolumeY = fMaxVolumeY;
	}
	bErrorCreation = false;

	ZERO(m_radius);

	//float fTest = reader.Float();
	//pITestUShift->SetFloat(fTest);
	//PH.pITestUShift->SetFloat(0.0f);

	return true;
}

void PSShadowMap::UpdateVariables()
{
	shadowMapResolution = PH.pShdServ->GetShadowMapRes();

	float fTexSize = 1.0f / float(shadowMapResolution) * fBlur;
	Vector4 vTexOffsets[4];
	for (long i=1; i<4; i++)
	{
		float fAng = fBlurAngle + float(i) / 3.0f * PIm2;
		float fCos = cosf(fAng);
		float fSin = sinf(fAng);

		Vector v(0.0f, fTexSize, 0.0f);
		v.RotateXY(fCos, fSin);
		vTexOffsets[i] = Vector4(v.x, v.y, 0.0f, 0.0f);
	}
	vTexOffsets[0] = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	PH.pITexOffsets->SetVector4Array(&vTexOffsets[0], 4);

	PH.pIShdK->SetVector4(Vector4(fShdK1, fShdK2, 0.0f, 0.0f));
	PH.pIShadowColor->SetVector(cShadowColor.color);
	//PH.pIShadowDistance->SetFloat(fShadowDistance);
	PH.pIShading->SetVector(Vector(fShadowDistance, 1.0f / (fShadowDistance * 0.05f), fShadowDistance * (1.0f - 0.05f)));
	PH.pILightDir->SetVector(lightDir);

}

#ifndef NO_CONSOLE
void _cdecl PSShadowMap::Console_Parameters(const ConsoleStack & params)
{
	dword num = params.GetSize();

	if (!num)
	{
		Console().Trace(COL_CMD_OUTPUT, "Current shadow map \"%s\"", GetObjectID().c_str());
		Console().Trace(COL_CMD_OUTPUT, "box = %.2f", fShadowDistance);
		Console().Trace(COL_CMD_OUTPUT, "zbias = %.3f", fZBias * 10.0f);
		Console().Trace(COL_CMD_OUTPUT, "charzbias = %.3f", fCharacterZBias * 10.0f);
		Console().Trace(COL_CMD_OUTPUT, "nearz = %.2f", fNearZ);
		Console().Trace(COL_CMD_OUTPUT, "kf = %.2f", fSplitSchemeLambda);
		Console().Trace(COL_CMD_OUTPUT, "shadowpower = %.2f", fShdPower);
		Console().Trace(COL_CMD_OUTPUT, "charshadowpower = %.2f", fCharacterShdPower);
		Console().Trace(COL_CMD_OUTPUT, "blur = %.2f", fBlur);
		return;
	}

	if (num == 2)
	{
		string command = params.GetParam(0);

		if (command == "box")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev box = %.2f", fShadowDistance);
			fShadowDistance = float(atof(params.GetParam(1)));
		}

		if (command == "zbias")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev zbias = %.3f", fZBias * 10.0f);
			fZBias = 0.1f * float(atof(params.GetParam(1)));
		}

		if (command == "charzbias")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev charzbias = %.3f", fCharacterZBias * 10.0f);
			fCharacterZBias = 0.1f * float(atof(params.GetParam(1)));
		}

		if (command == "nearz")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev nearz = %.2f", fNearZ);
			fNearZ = float(atof(params.GetParam(1)));
		}

		if (command == "kf")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev kf = %.2f", fSplitSchemeLambda);
			fSplitSchemeLambda = float(atof(params.GetParam(1)));
		}

		if (command == "shadowpower")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev shadowpower = %.2f", fShdPower);
			fShdPower = float(atof(params.GetParam(1)));
		}

		if (command == "charshadowpower")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev charshadowpower = %.2f", fCharacterShdPower);
			fCharacterShdPower = float(atof(params.GetParam(1)));
		}

		if (command == "blur")
		{
			Console().Trace(COL_CMD_OUTPUT, "prev blur = %.2f", fBlur);
			fBlur = float(atof(params.GetParam(1)));
		}

		UpdateVariables();
	}

}
#endif

void PSShadowMap::Activate(bool isActive)
{
	if (isActive)
	{
		if (!EditMode_IsOn())
		{
			Console().Trace(COL_ALL, "PSShadowMap '%s' enabled", GetObjectID().c_str());
			LogicDebug("PSShadowMap '%s' enabled", GetObjectID().c_str());

			// Смотрим если есть другие включенные шадоумапы, то деактивируем их и ругаемся на это
			MGIterator * iter = &Mission().GroupIterator(SHADOWMAP_GROUP, _FL_);
			for (;!iter->IsDone();iter->Next())
			{
				if ((iter->Get())->IsActive() && (iter->Get()) != this)
				{
					iter->Get()->Activate(false);
					LogicDebugError("PSSM: Shadowmap '%s' was deactivated, potential double/tripple/quad shadowmaps active at the same time.", iter->Get()->GetObjectID().c_str());
					break;
				}
			}
			iter->Release();		
		
#ifndef NO_CONSOLE
			Console().RegisterCommand("pssm", "sets shadow map parameters", this, (CONSOLE_COMMAND)&PSShadowMap::Console_Parameters);
#endif
		}
	}
	else	
	{
#ifndef NO_CONSOLE
		Console().UnregisterCommand("pssm");
#endif

		if (!EditMode_IsOn())
		{
			Console().Trace(COL_ALL, "PSShadowMap '%s' disabled", GetObjectID().c_str());
			LogicDebug("PSShadowMap '%s' disabled", GetObjectID().c_str());
		}
	}

	if (isActive)
	{	
		SetUpdate((MOF_UPDATE)&PSShadowMap::Realize, ML_SHADOWCAST);
		SetUpdate((MOF_UPDATE)&PSShadowMap::ClearScreen, ML_FILL - 1);
		
#ifndef STOP_DEBUG
		SetUpdate((MOF_UPDATE)&PSShadowMap::DebugRealize, ML_LAST);
#endif

		UpdateVariables();
	}
	else
	{
		DelUpdate((MOF_UPDATE)&PSShadowMap::Realize);
		DelUpdate((MOF_UPDATE)&PSShadowMap::ClearScreen);
#ifndef STOP_DEBUG
		DelUpdate((MOF_UPDATE)&PSShadowMap::DebugRealize);
#endif
	}

	MissionObject::Activate(isActive);

	RegisterUnregisterPixEvents();
}

void PSShadowMap::AddShadowCastObject(MissionObject * obj, MOF_EVENT func, const Vector & vMin, const Vector & vMax)
{
	PSSMHolder::SCOData & sco = (*PH.aSCObjects)[PH.aSCObjects->Add()];

	const char * type = obj->GetObjectType();

	sco.obj = obj;
	sco.isVisible = true;
	sco.isCharacter = (type[0]=='C' && type[1]=='h' && type[2]=='a');
	sco.func = func;
	obj->GetBox(sco.vMin, sco.vMax);
	obj->GetMatrix(sco.mtx);

	// FIX-ME, квадратов будет достаточно
	//sco.distance2Light = (m_lightFarPosition - sco.mtx.pos).GetLength();
	//sco.radius = (sco.vMax - sco.vMin).GetLength() * 0.5f;

	//PH.aSCObjects->Add(sco);
}

//! Проверка видимости сферы в frustum'e камеры
/*__forceinline bool PSShadowMap::SphereIsVisibleSquare(const Plane * frustum, const Vector & SphereCenter, float SphereRadius)
{
	for(long p = 0; p < 5; p++)
	{
		float dist = frustum[p].Dist(SphereCenter);
		if (dist * fabsf(dist) < -Sqr(SphereRadius)) return false;
	}

	return true;
}*/

__forceinline bool PSShadowMap::SphereIsVisibleSquare(const Plane * frustum, const Vector4 & SphereCenter, float SphereRadius)
{
#ifndef _XBOX
	for (int p = 1; p < 5; p++)
	{
		float dist = frustum[p].Dist(SphereCenter.v);
		if (dist < -SphereRadius) 
			return false;
	}
#else
	__vector4 vradius = XMVectorReplicate(-SphereRadius);
	__vector4 vcenter = __lvlx(&SphereCenter, 0);

	__vector4 v1 = __vmsum4fp(__VMXGetReg(vfrustum1), vcenter);
	__vector4 v2 = __vmsum4fp(__VMXGetReg(vfrustum2), vcenter);
	__vector4 v3 = __vmsum4fp(__VMXGetReg(vfrustum3), vcenter);
	__vector4 v4 = __vmsum4fp(__VMXGetReg(vfrustum4), vcenter);
	
	__vector4 v12 = __vmrglw(v1, v2);
	__vector4 v34 = __vmrghw(v3, v4);

	__vector4 v1234 = __vperm(v12, v34, __VMXGetReg(vperm0167));

	unsigned int compareResult = 0;
	__vcmpgtfpR(vradius, v1234, &compareResult);

	 if (!(compareResult & (1 << 5)))
		return false;
#endif

	return true;
}

bool PSShadowMap::BoxIsVisible(const Plane * frustum, const Vector & vMin, const Vector & vMax)
{
	for(long i=0; i<5; i++)
	{
		const Plane & p = frustum[i];

		if (p.Dist(Vector(vMin.x, vMin.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMin.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMin.x, vMin.y, vMax.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMin.y, vMax.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMin.x, vMax.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMax.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMin.x, vMax.y, vMax.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMax.y, vMax.z)) > 0.0f) continue;

		return false;
	}

	return true;
}

IS_XBOX(IRenderTargetDepth *, IRenderTarget *) PSShadowMap::GetZRenderTarget()
{
	return PH.pShdServ->GetShadowMap();
}

void PSShadowMap::CalcProjection(const Matrix & mShadowView, Matrix & mShadowProjection, const Vector & min, const Vector & max)
{
	Vector box_vertices[8];
	box_vertices[0] = Vector ( min.x,  min.y,  min.z) * mShadowView;
	box_vertices[1] = Vector ( max.x,  min.y,  min.z) * mShadowView;
	box_vertices[2] = Vector ( min.x,  min.y,  max.z) * mShadowView;
	box_vertices[3] = Vector ( max.x,  min.y,  max.z) * mShadowView;
	box_vertices[4] = Vector ( min.x,  max.y,  min.z) * mShadowView;
	box_vertices[5] = Vector ( max.x,  max.y,  min.z) * mShadowView;
	box_vertices[6] = Vector ( min.x,  max.y,  max.z) * mShadowView;
	box_vertices[7] = Vector ( max.x,  max.y,  max.z) * mShadowView;

	float min_x = 1e32f;
	float max_x =-1e32f;

	float min_y = 1e32f;
	float max_y =-1e32f;

	float min_z = 1e32f;
	float max_z =-1e32f;

	for (int i=0;i < 8; i++) 
	{
		min_x = Min(min_x, box_vertices[i].x);
		max_x = Max(max_x, box_vertices[i].x);

		min_y = Min(min_y, box_vertices[i].y);
		max_y = Max(max_y, box_vertices[i].y);

		min_z = Min(min_z, box_vertices[i].z);
		max_z = Max(max_z, box_vertices[i].z);
	}

	if (min_z < 0.05f) min_z = 0.05f;

	float dist = Max(fabsf(max_x),fabsf(max_y));
	
	dist = Max(dist,fabsf(min_x));
	dist = Max(dist,fabsf(min_y));

	float fov = atanf(max_x/min_z) * 2.0f;

	//if (fov > PI / 180.0f * 120.0f)	fov = PI / 180.0f * 120.0f;

	mShadowProjection.BuildProjection(fov, 1.0f, 1.0f, 0.01f, fShadowDistance);
}

void PSShadowMap::CalculateMinMaxReceiversY()
{
	iNumRecvObjects = 0;
	fMinReceiversY = 10000.0f;
	fMaxReceiversY = -10000.0f;

	SetupXBOXFrustumRegisters();

	__declspec(align(16)) Vector4 vCenter, vProjCenter;
	vCenter.w = -1.0f;
	vProjCenter.w = -1.0f;

	// Находим верхнюю и нижнюю границы объектов на которые будет падать тень
	for (pIterSRecv->Reset(); !pIterSRecv->IsDone(); pIterSRecv->Next())
	{
		Vector vBox[8];
		Vector vMin, vMax;
		Matrix mObj;
		pIterSRecv->Get()->GetBox(vMin, vMax);
		pIterSRecv->Get()->GetMatrix(mObj);

		Vector min, max;
		Box::FindABBforOBB(mObj, vMin, vMax, min, max, false);

		vCenter.v = (min + max) / 2.0f;
		float fRadiusSquare = ~((vMax - vMin) / 2.0f);//.GetLength();

		if (!SphereIsVisibleSquare(frustum, vCenter, Sqr(fRadiusSquare))) continue;

		float fRadius = sqrtf(fRadiusSquare);

		vCenter.v = vCenter.v * m_View;
		if (vCenter.z + fRadius <= Splits[0]) continue;
		if (vCenter.z - fRadius >= Splits[3]) continue;

		fMinReceiversY = Min(fMinReceiversY, min.y);
		fMaxReceiversY = Max(fMaxReceiversY, max.y);

		iNumRecvObjects++;
	}

	fMinReceiversY = Max(fMinVolumeY, fMinReceiversY);
	fMaxReceiversY = Min(fMaxVolumeY, fMaxReceiversY);
}

void PSShadowMap::SetupXBOXFrustumRegisters()
{
#ifdef _XBOX
	__declspec(align(16)) Plane x_frustum[4];
	for (int i=0; i<4; i++)
		x_frustum[i] = frustum[i + 1];

	__declspec(align(16)) byte byteControl0167[16] = {0,1,2,3, 4,5,6,7, 24,25,26,27,  28,29,30,31};
	__VMXSetReg(vfrustum1, __lvlx(x_frustum, 0));
	__VMXSetReg(vfrustum2, __lvlx(x_frustum, 16));
	__VMXSetReg(vfrustum3, __lvlx(x_frustum, 32));
	__VMXSetReg(vfrustum4, __lvlx(x_frustum, 48));
	__VMXSetReg(vperm0167, __lvlx(&byteControl0167, 0));
	__VMXSetReg(vabs, XMVectorReplicateInt(0x7FFFFFFF));
#endif
}

void PSShadowMap::CalculateCastObjectParameters()
{
	__declspec(align(16)) Vector4 vProjCenter;
	vProjCenter.w = -1.0f;

	array<PSSMHolder::SCOData> & scoObjs = *PH.aSCObjects;

	SetupXBOXFrustumRegisters();

	// Рассчитываем бокс объекта вместе с тенью
	for (long j=0; j<scoObjs.Len(); j++)
	{
		Vector vBox[8];
		
		const Vector & vMin = scoObjs[j].vMin;
		const Vector & vMax = scoObjs[j].vMax;
		const Matrix & mObj = scoObjs[j].mtx;

		bool bCharacter = scoObjs[j].isCharacter;

		Vector min, max;
		Box::FindABBforOBB(mObj, vMin, vMax, min, max, false);

		Vector vCenter = (min + max) / 2.0f;
		float fRadius = ((vMax - vMin) / 2.0f).GetLength();

		float fTestY = (lightDir.y < 0.0f) ? fMinReceiversY : fMaxReceiversY;
		if (bCharacter) fTestY = vCenter.y + fRadius * ((lightDir.y < 0.0f) ? -2.5f : 2.5f);
		float len = (fabsf(lightDir.y) > 1e-5f) ? 
			(vCenter.y - fTestY) / fabsf(lightDir.y) : 1.0f;

		Vector vProj = vCenter + lightDir * len;
		
		Vector v1 = vCenter - Vector(fRadius, fRadius, fRadius);
		Vector v2 = vCenter + Vector(fRadius, fRadius, fRadius);
		Vector v3 = vProj - Vector(fRadius, 0.0f, fRadius);
		Vector v4 = vProj + Vector(fRadius, 0.0f, fRadius);

		Vector vProjMin, vProjMax;
		vProjMin.Min(v1, v3);
		vProjMax.Max(v2, v4);
		
		vProjCenter.v = (vProjMax + vProjMin) / 2.0f;
		float fProjRadius = ~((vProjMax - vProjMin) / 2.0f);//.GetLength();
		// пробуем грубо отсечь, если не попадают в общий фрустум
		scoObjs[j].isVisible = SphereIsVisibleSquare(frustum, vProjCenter, Sqr(fProjRadius));
		if (scoObjs[j].isVisible)
			scoObjs[j].isVisible = BoxIsVisible(frustum, vProjMin, vProjMax);

		if (!m_clipPlanes.IsEmpty() && scoObjs[j].isVisible)
		{
			for (int i=0; i<m_clipPlanes.Len(); i++)
				if (m_clipPlanes[i].plane.Dist(vCenter) < -fRadius)
				{
					scoObjs[j].isVisible = false;
					break;
				}
		}

		// приготавливаем данные для точного отсечения бокса
		if (scoObjs[j].isVisible)
		{
			scoObjs[j].projectedMtx = lightMtx;
			scoObjs[j].projectedMtx.pos = vCenter + lightDir * len * 0.5f;
			PH.m_projMinMax[j * 2 + 0].v = -Vector(fRadius, fRadius, len * 0.5f + fRadius);
			PH.m_projMinMax[j * 2 + 1].v = Vector(fRadius, fRadius, len * 0.5f + fRadius);
		}

		float fZ[4];
		fZ[0] = (vCenter * m_View).z - fRadius;
		fZ[1] = (vCenter * m_View).z + fRadius;
		fZ[2] = (vProj * m_View).z - fRadius;
		fZ[3] = (vProj * m_View).z + fRadius;

		float fMinZ = 10000.0f, fMaxZ = -10000.0f;
		for (long k=0; k<4; k++)
		{
			fMinZ = Min(fMinZ, fZ[k]);
			fMaxZ = Max(fMaxZ, fZ[k]);
		}

		scoObjs[j].fProjZMin = fMinZ;
		scoObjs[j].fProjZMax = fMaxZ;
	}
}

void PSShadowMap::ShadowMap_Cast(int split)
{
	float fTexOffset = 0.5f + (0.5f / float(shadowMapResolution));
	Matrix mTexScale = Matrix().BuildScale(0.5f, -0.5f, 1.0f) * Matrix().BuildPosition(fTexOffset, fTexOffset, 0.0f);

	RENDERVIEWPORT castVP;
	castVP.X = 0;
	castVP.Y = 0;
	castVP.Width = PH.pShdServ->GetShadowMapRes();
	castVP.Height = PH.pShdServ->GetShadowMapRes();
	castVP.MinZ = 0.0f;
	castVP.MaxZ = 1.0f;
	Render().SetViewport(castVP);

	bool bGeoCull = Geometry().GetGlobalFrustumCullState();
	Geometry().SetGlobalFrustumCullState(false);

	array<PSSMHolder::SCOData> & scoObjs = *PH.aSCObjects;

	if (iNumRecvObjects)
	{
		dword dwId = 0;

		if (!EditMode_IsOn())
			dwId = Render().pixBeginEvent(_FL_, "SHADOWCAST" );

		Frustum fr;
		fr.SetView(m_View);
		fr.SetProjection(m_Proj);
		fr.SetAspect(fCameraAspect);

		//for (long i=0; i<iNumSplits; i++)
		{
#ifndef _XBOX
			Geometry().SetRenderMode(&PH.PSSM_MT_CAST_id);
#else
			Geometry().SetRenderMode(&PH.PSSM_ST_CAST_id, &PH.PSSM_ST_CASTNPS_id);
#endif

			float fZ1 = Splits[split];
			float fZ2 = Splits[split + 1];

			fr.SetNearFarZ(Splits[split], Splits[split + 1]);

			// Вычисляем View и Projection матрицы для сплитов (и сразу записываем в шейдерную константу)
			CalcViewProjForSplit(split, Splits[split], Splits[split + 1]);

			Render().SetView(mLightView);
			Render().SetProjection(mLightProj);

#ifndef _XBOX
			Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, GetZRenderTarget(), PH.pShdServ->GetZStencil());
			Render().BeginScene();
			Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0x0);
#else
			Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, 0, PH.pShdServ->GetShadowMap());
			Render().BeginScene();
			Render().Clear(0, null, CLEAR_ZBUFFER, 0, 1.0f, 0x0);
#endif

			// Рендерим все объекты в shadowmap
			castSplitTimer[split].Start();
			for (long j=0; j<scoObjs.Len(); j++) 
			{
				PSSMHolder::SCOData & sco = scoObjs[j];

				if (!sco.isVisible) continue;
				if (sco.fProjZMax < fZ1) continue;
				if (sco.fProjZMin > fZ2) continue;

				__declspec(align(16)) Matrix mObj = sco.projectedMtx;
				bool visible = OverlapFrustumOBB(fr, mObj, PH.m_projMinMax[j*2+0], PH.m_projMinMax[j*2+1]);
				if (!visible) continue;

				statDrawSplit[split]++;
				if (sco.isCharacter && api->DebugKeyState('4')) continue;
				ExecuteEvent(scoObjs[j].obj, scoObjs[j].func);
			}
			castSplitTimer[split].Stop();

			Matrix mShadowMap = (mLightView * mLightProj) * mTexScale;
			PH.pIShadowMapMatrix->SetMatrix(mShadowMap);

			Render().EndScene();
		}

		if (!EditMode_IsOn())
			Render().pixEndEvent(_FL_, dwId);
	}

	//Восстанавливаем рендерные и геометрические параметры
	Geometry().SetRenderMode(NULL);	
	Geometry().SetGlobalFrustumCullState(bGeoCull);
}

// Чистим принимающий шадоумап, если ни одного чарактера небыло отрисованно
void PSShadowMap::ShadowMap_Recv_Clear()
{
	PH.pShdServ->SetPostProcessAsRT(false);
	Render().BeginScene();
	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0x0);
	Render().EndScene();
}

void PSShadowMap::Recv_ZPrePass()
{
	dword dwId = Render().pixBeginEvent(_FL_, "ShadowRecv_ZPrePass");

	RENDERVIEWPORT D3DViewPort = normalViewport;

	PH.pShdServ->SetLinearZAsRT();
	PH.pShdServ->SetPostProcessTexViewPort();

	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0f, 0x0);

	Render().BeginScene();

	D3DViewPort.X = (DWORD)((float)D3DViewPort.X / (float)renderInfo.dwWidth * (float)PH.pShdServ->GetPostProcessTexWidth());
	D3DViewPort.Y = (DWORD)((float)D3DViewPort.Y / (float)renderInfo.dwHeight * (float)PH.pShdServ->GetPostProcessTexHeight());
	D3DViewPort.Width = (DWORD)((float)D3DViewPort.Width / (float)renderInfo.dwWidth * (float)PH.pShdServ->GetPostProcessTexWidth());
	D3DViewPort.Height = (DWORD)((float)D3DViewPort.Height / (float)renderInfo.dwHeight * (float)PH.pShdServ->GetPostProcessTexHeight());
	
	D3DViewPort.X = 0;
	D3DViewPort.Y = 0;
	D3DViewPort.Width = PH.pShdServ->GetPostProcessTexWidth();
	D3DViewPort.Height = PH.pShdServ->GetPostProcessTexHeight();
	D3DViewPort.MinZ = 0.0f;
	D3DViewPort.MaxZ = 1.0f;
	Render().SetViewport(D3DViewPort);

	// рисуем z-pass
	if (m_numZPassPlanes)
	{
		Render().SetWorld(Matrix());
		Render().SetStreamSource(0, m_zPassVB, 0);
		Render().SetIndices(m_zPassIB);
		Render().DrawIndexedPrimitive(PH.zpass_id, PT_TRIANGLELIST, 0, m_numZPassPlanes * 4, 0, m_numZPassPlanes * 2);
	}

	PH.pIShadowMapTexture->SetTexture(GetZRenderTarget()->AsTexture());
 
	float fZBiasMultiply = (bPointLight) ? 0.01f : 1.0f;//IS_XBOX(4.0f, 1.0f);

	Frustum fr;
	fr.SetView(m_View);
	fr.SetProjection(m_Proj);
	fr.SetAspect(fCameraAspect);
	fr.SetNearFarZ(Splits[0], Splits[3]);

	__declspec(align(16)) Vector4 vCenter;
	vCenter.w = -1.0f;

	Geometry().SetRenderMode(&PH.PSSM_ST_RECV_id);

	// Рендерим все объекты в shadowmap
	iNumRecv = 0;
	short index = 0;
	for (pIterSRecv->Reset(); !pIterSRecv->IsDone(); pIterSRecv->Next(), index++)
	{
		MissionObject * mo = pIterSRecv->Get();
		const char * type = mo->GetObjectType();
		bool bCharacter = (type[0]=='C' && type[1]=='h' && type[2]=='a');
		
		__declspec(align(16)) Vector4 vMin, vMax;
		Vector min, max;
		__declspec(align(16)) Matrix mObj;
		mo->GetBox(vMin.v, vMax.v);
		mo->GetMatrix(mObj);

		Box::FindABBforOBB(mObj, vMin.v, vMax.v, min, max, false);

		vCenter.v = (min + max) / 2.0f;
		float fRadiusSquare = ~((vMax.v - vMin.v) / 2.0f);//.GetLength();

		if (!SphereIsVisibleSquare(frustum, vCenter, Sqr(fRadiusSquare))) continue;

		float fRadius = sqrtf(fRadiusSquare);

		vCenter.v = vCenter.v * m_View;
		if (vCenter.z + fRadius <= Splits[0]) continue;
		if (vCenter.z - fRadius >= Splits[3]) continue;

		bool visible = OverlapFrustumOBB(fr, mObj, vMin, vMax);
		if (!visible) continue;

		if (bCharacter && api->DebugKeyState('5')) continue;

		float fB = (bCharacter) ? fCharacterZBias * fZBiasMultiply * 0.5f: fZBias * fZBiasMultiply;

		// FIX-ME - Проверить эту переменную, а то она тут как-то непонятно для чего прикручена
		PH.pIZBias->SetVector4(Vector4(fB, fB * 1.2f, fB * 2.0f, 0.0f));

		PH.pIShadowPower->SetFloat((bCharacter) ? fCharacterShdPower : fShdPower);

		pIterSRecv->ExecuteEvent();
		iNumRecv++;
	}

	//Восстанавливаем рендерные и геометрические параметры
	Geometry().SetRenderMode(NULL);

	//Render().EndScene(null, false, true);
	Render().EndScene(null);

	Render().pixEndEvent(_FL_, dwId);
}

// Рисуем саму теневую текстуру
void PSShadowMap::Recv_ShadowPass(int split)
{
	if (api->DebugKeyState('B'))
		return;

	dword dwId;
	if (!EditMode_IsOn())
		dwId = Render().pixBeginEvent(_FL_, "SHADOWRECEIVE" );

	RENDERVIEWPORT D3DViewPort = normalViewport;

	RecvVertex v[4];

	m_invView = m_View; m_invView.Inverse();

	float HalfPixelSizeX = 1.0f / 512.0f;
	float HalfPixelSizeY = 1.0f / 512.0f;

	float left = -1.0f - HalfPixelSizeX;
	float right = 1.0f - HalfPixelSizeX;

	float up = -1.0f + HalfPixelSizeY;
	float down = 1.0f + HalfPixelSizeY;

	float curZ = (split == 0) ? 0.100001f : Splits[split];
	Vector4 zz = m_Proj.MulVertex(Vector4(0.0f, 0.0f, curZ, 1.0f));
	zz.z /= zz.w;

	PH.pISplits->SetVector4(Vector4(Splits[split + 1], 0.0f, 0.0f, 0.0f));

	Vector vv[4], dirs[4];
	vv[0] = Vector(1.0f / m_Proj.m[0][0],	1.0f / m_Proj.m[1][1],	1.0f);
	vv[1] = Vector(1.0f / -m_Proj.m[0][0],	1.0f / m_Proj.m[1][1],	1.0f);
	vv[2] = Vector(1.0f / m_Proj.m[0][0],	1.0f / -m_Proj.m[1][1],	1.0f);
	vv[3] = Vector(1.0f / -m_Proj.m[0][0],	1.0f / -m_Proj.m[1][1],	1.0f);

	for (int i=0; i<4; i++)
		dirs[i] = m_invView.MulNormal(vv[i]);

	v[0].pos = Vector4(left, down, zz.z, 1.0f);
	v[0].dir = dirs[1];
	v[0].u = 0.0f;	v[0].v = 0.0f;
    
	v[1].pos = Vector4(left, up, zz.z, 1.0f);
	v[1].dir = dirs[3];
	v[1].u = 0.0f;	v[1].v = 1.0f;
    
	v[2].pos = Vector4(right, down, zz.z, 1.0f);
	v[2].dir = dirs[0]; 
	v[2].u = 1.0f;	v[2].v = 0.0f;
    
	v[3].pos = Vector4(right, up, zz.z, 1.0f);
	v[3].dir = dirs[2]; 
	v[3].u = 1.0f;	v[3].v = 1.0f;

	PH.varLinearZTex->SetTexture(PH.pShdServ->GetLinearZRT()->AsTexture());

	PH.pShdServ->SetPostProcessAsRT(true);//!bClear);
	
	D3DViewPort.X = (DWORD)((float)normalViewport.X / (float)renderInfo.dwWidth * (float)PH.pShdServ->GetPostProcessTexWidth());
	D3DViewPort.Y = (DWORD)((float)normalViewport.Y / (float)renderInfo.dwHeight * (float)PH.pShdServ->GetPostProcessTexHeight());
	D3DViewPort.Width = (DWORD)((float)normalViewport.Width / (float)renderInfo.dwWidth * (float)PH.pShdServ->GetPostProcessTexWidth());
	D3DViewPort.Height = (DWORD)((float)normalViewport.Height / (float)renderInfo.dwHeight * (float)PH.pShdServ->GetPostProcessTexHeight());
	D3DViewPort.MinZ = 0.0f;
	D3DViewPort.MaxZ = 1.0f;
	Render().SetViewport(D3DViewPort);

	Render().BeginScene();
	Render().SetWorld(Matrix());

	if (split == 0)
		Render().Clear(0, null, CLEAR_TARGET, 0xFFFFFFFF, 1.0f, 0x0);

	Render().DrawPrimitiveUP(PH.defShade_id, PT_TRIANGLESTRIP, 2, v, sizeof(RecvVertex));

	Render().EndScene(null, false, true);
//#endif

	if (!EditMode_IsOn())
		Render().pixEndEvent(_FL_, dwId);
}

void PSShadowMap::CalculateSplitDistances()
{
	fSplitSchemeLambda = Clamp3(fSplitSchemeLambda, 0.0f, 1.0f);
  
	for(int i=0; i<3; i++)
	{
		float fIDM = float(i) / 3.0f;
		float fLog = fCameraNear * powf((fCameraFar / fCameraNear), fIDM);
		float fUniform = fCameraNear + (fCameraFar - fCameraNear) * fIDM;
	
		Splits[i] = fLog * fSplitSchemeLambda + fUniform * (1.0f - fSplitSchemeLambda);
	}

	Splits[0] = fCameraNear;
	Splits[3] = fCameraFar;

	float fD = fCameraFar - fCameraNear;

	/*Splits[0] = fCameraNear;
	Splits[1] = fCameraNear + fD * 0.1f;
	Splits[2] = fCameraNear + fD * 0.2f;
	Splits[3] = fCameraFar;*/

	PH.pISplits->SetVector4(Vector4(Splits[0], Splits[1], Splits[2], Splits[3]));
}

void PSShadowMap::CalcViewProjForSplit(long iIndex, float fNear, float fFar)
{
	// Calculate corner points of frustum split
    // To avoid edge problems, scale the frustum so
    // that it's at least a few pixels larger
	Vector vCorners[8];
	CalculateFrustumCorners(vCorners, vCamPos, vCamTarget, vCamUp,
                            fNear, fFar, fCameraFOV, fCameraAspect, 1.0f);	// Here is HACK = 1.2 - FIX ME!!!!

	// Calculate view and projection matrices
	CalculateLightForFrustum(iIndex, vCorners);

	// Ставим результирующую матрицу в шейдерную константу
	//PH.pIViewProj->SetMatrix(mLightView * mLightProj);
}

//ProfileTimer t1, t2, t3;

#ifndef STOP_DEBUG
void _cdecl PSShadowMap::DebugRealize(float fDeltaTime, long level)
{
	if (EditMode_IsOn() && (EditMode_IsSelect()))
	{
		for (int i=0; i<m_clipPlanes.Len(); i++)
		{
			Vector v[10];
			Matrix mtx = Matrix(m_clipPlanes[i].angles - Vector(PI / 2.0f, 0.0f, 0.0f), 0.0f);
			for (int j=0; j<10; j++)
			{
				float r = 100.0f;
				Matrix rot(0.0f, float(j) / 10.0f * PIm2, 0.0f, 0.0f, 0.0f, 0.0f);
				v[j] = (mtx) * (rot * Vector(0.0f, 0.0f, 100.0f));
				v[j] += m_clipPlanes[i].pos;
			}
			Render().DrawPolygon(v, 10, Color(1.0f, 0.0f, 0.0f, 0.5f));
		}

		if (m_numZPassPlanes)
		{
			Render().SetWorld(Matrix());
			Render().SetStreamSource(0, m_zPassVB, 0);
			Render().SetIndices(m_zPassIB);
			Render().DrawIndexedPrimitive(PH.zpasswire_id, PT_TRIANGLELIST, 0, m_numZPassPlanes * 4, 0, m_numZPassPlanes * 2);
		}
	}

	const Matrix & view = Render().GetView();
	const Matrix & proj = Render().GetProjection();
	Matrix invView = Matrix(view).Inverse();
	Vector vz = invView.vz;

	static float fShow_MinVolumeY = 0.0f;
	static float fShow_MaxVolumeY = 0.0f;

	if (fabsf(fOldMinVolumeY - fMinVolumeY) > 0.01f)
		fShow_MinVolumeY = 5.0f;

	if (fabsf(fOldMaxVolumeY - fMaxVolumeY) > 0.01f)
		fShow_MaxVolumeY = 5.0f;

	fOldMinVolumeY = fMinVolumeY;
	fOldMaxVolumeY = fMaxVolumeY;

	float d = 3000.0f;
	if (fShow_MinVolumeY > 0.0f)
	{
		fShow_MinVolumeY -= fDeltaTime;

		Vector circle[4];
		circle[0] = Vector(-d, fMinVolumeY, d);
		circle[1] = Vector(d, fMinVolumeY, d);
		circle[2] = Vector(d, fMinVolumeY, -d);
		circle[3] = Vector(-d, fMinVolumeY, -d);
		Color color(0.7f, 1.0f, 0.7f, Min(0.8f, fShow_MinVolumeY * 0.5f));
		Render().DrawPolygon(circle, 4, color);
	}

	if (fShow_MaxVolumeY > 0.0f)
	{
		fShow_MaxVolumeY -= fDeltaTime;

		Vector circle[4];
		circle[0] = Vector(-d, fMaxVolumeY, -d);
		circle[1] = Vector(d, fMaxVolumeY, -d);
		circle[2] = Vector(d, fMaxVolumeY, d);
		circle[3] = Vector(-d, fMaxVolumeY, d);
		Color color(1.0f, 0.5f, 1.0f, Min(0.8f, fShow_MaxVolumeY * 0.5f));
		Render().DrawPolygon(circle, 4, color);
	}

	static bool bDebugDraw = false;

	if (api->DebugKeyState(VK_CONTROL, VK_SHIFT, 'N'))
	{
		bDebugDraw ^= 1;
		Sleep(100);
	}

	if (bDebugDraw)
	{
		dword total = castTimer.GetTime32() + recvTimer.GetTime32() + enumTimer.GetTime32();

		Render().Print(20, 20, 0xFFFFFFFF, "objects: %d, shadowmap: %s", PH.aSCObjects->Size(), GetObjectID().c_str());
		Render().Print(20, 40, 0xFFFFFFFF, "split 0: %d, %.3f,   %.0f", statDrawSplit[0], castSplitTimer[0].GetTime32() / 1000000.0f, m_radius[0]);
		Render().Print(20, 60, 0xFFFFFFFF, "split 1: %d, %.3f,   %.0f", statDrawSplit[1], castSplitTimer[1].GetTime32() / 1000000.0f, m_radius[1]);
		Render().Print(20, 80, 0xFFFFFFFF, "split 2: %d, %.3f,   %.0f", statDrawSplit[2], castSplitTimer[2].GetTime32() / 1000000.0f, m_radius[2]);
		Render().Print(20, 100, 0xFFFFFFFF, "cast rdtsc: %.2fM", castTimer.GetTime32() / 1000000.0f);
		Render().Print(20, 140, 0xFFFFFFFF, "recv rdtsc: %d, %.2fM", iNumRecv, recvTimer.GetTime32() / 1000000.0f);
		Render().Print(20, 160, 0xFFFFFFFF, "total rdtsc: %.2fM", total / 1000000.0f);

		Render().Print(20, 220, 0xFFFFFFFF, "max receiver: %.3f, %s", fMaxReceiversY, sMaxReceiver.c_str());
		Render().Print(20, 200, 0xFFFFFFFF, "min receiver: %.3f, %s", fMinReceiversY, sMinReceiver.c_str());
	}

	if (api->DebugKeyState('N'))
	{
		for (long j=0; j<PH.aSCObjects->Len(); j++)
		{
			Matrix mObj;
			(*PH.aSCObjects)[j].obj->GetMatrix(mObj);

			//Render().DrawBox(aSCObjects[j].vProjMin, aSCObjects[j].vProjMax, Matrix(), 0x1F001F5F);
			Render().DrawBox(PH.m_projMinMax[j*2+0].v, PH.m_projMinMax[j*2+1].v, (*PH.aSCObjects)[j].projectedMtx, 0xFF00FF00);
		}
	}

	if (bShowDebug)
	{
		//Render().Print(300.0f, 200.0f, 0xFF00FF00, "Casted objects: %d", dwObjectsCasted1);
		
		Render().DrawSphere(lightSource, 3.0f);
		//Render().DrawBox(vBoxMin, vBoxMax, Matrix(), 0xFF00FF00);

		RS_SPRITE spr[4];

		spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
		spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
		spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
		spr[3].tu = 0.0f;  spr[3].tv = 1.0f;

		spr[0].vPos = Vector (0.0f, 1.0f, 0.0f);
		spr[1].vPos = Vector (1.0f, 1.0f, 0.0f);
		spr[2].vPos = Vector (1.0f, 0.0f, 0.0f);
		spr[3].vPos = Vector (0.0f, 0.0f, 0.0f);

		spr[0].dwColor = 0xFFFFFFFF;		spr[1].dwColor = 0xFFFFFFFF;
		spr[2].dwColor = 0xFFFFFFFF;		spr[3].dwColor = 0xFFFFFFFF;

		Render().SetWorld(Matrix().SetIdentity());
		//Render().DrawSprites(PH.pShdServ->GetLinearZRT()->AsTexture(), spr, 1, "dbgSpritesNoblend");
		Render().DrawSprites(PH.pShdServ->GetPostProcessTexture()->AsTexture(), spr, 1, "dbgSpritesNoblend");
	}
}
#endif	

void _cdecl PSShadowMap::ClearScreen(float fDeltaTime, long level)
{
	if (!EditMode_IsOn())
		PH.pShdServ->ClearScreen();
}

void _cdecl PSShadowMap::Realize(float fDeltaTime, long level)
{
	fZBias = Max(fZBias, 0.001f * 0.1f * 0.25f);

#ifndef STOP_DEBUG
	if (api->DebugKeyState('M') || api->DebugKeyState(ICore::caps_lock_flag)) 
	{
		Render().PushRenderTarget();
		Render().EndScene();

		ShadowMap_Recv_Clear();

		Render().PopRenderTarget(RTO_RESTORE_CONTEXT);
		Render().BeginScene();
		return;
	}
#endif	
	if (!PH.IsInited()) return;
	if (bErrorCreation) return;

	PH.pShdServ->NotifyActivation();

	// if shadowservice off
	if (!PH.pShdServ->IsActiveShadowMap()) return;

	Matrix mView = Render().GetView();
	m_View = mView;
	m_invView = Matrix(m_View).Inverse();
	m_Proj = Render().GetProjection();

	vCamPos = mView.GetCamPos();
	vCamTarget = vCamPos + mView.Inverse().vz * 1.0f;
	vCamUp = Vector(0.0f, 1.0f, 0.0f);

	PH.m_varCamPos->SetVector4(vCamPos);

	// сохраняем все рендерные параметры
	const Plane * frustumPtr = Render().GetFrustum();
	for (long i=0; i<5; i++)
		frustum[i] = frustumPtr[i];

	renderInfo = Render().GetScreenInfo3D();		
	normalViewport = Render().GetViewport();
	Render().PushRenderTarget();
	Render().PushViewport();
	Render().EndScene(null, true);

	fCameraAspect = float(normalViewport.Width) / float(normalViewport.Height);
	//fCameraFOV = m_Proj.m[0][0];//(fabsf(mProj.m[0][0]) > 1e-8f) ? ((180.0f * 2.0f * atanf(1.0f / mProj.m[0][0])) / PI) : 71.0f;
	fCameraFOV = (fabsf(m_Proj.m[0][0]) > 1e-8f) ? ((180.0f * 2.0f * atanf(1.0f / m_Proj.m[0][0])) / PI) : 71.0f;
	if (fabsf(fNearZ) > 1e-5f) fCameraNear = fNearZ;
	
	float smres = float(PH.pShdServ->GetShadowMapRes());
	PH.pVarTexParams->SetVector4(Vector4(smres, (smres > 0.0f) ? 1.0f / smres : 0.0f, 0.0f, 0.0f));

	//PH.pIShadowDistanceStart->SetFloat(fCameraNear);

	enumTimer.Start();
		PH.aSCObjects->Empty();
		GetShadowCastObjects();

		// Выделяем выровненую память для SSE/VMX 
		int newSize = int(1.0f + PH.aSCObjects->Len() / 124.0f) * 124;
		if (PH.m_projNum < newSize)
		{
			PH.m_projNum = newSize;

			PH.m_projBuffer = (char*)api->Reallocate(PH.m_projBuffer, newSize * sizeof(Vector4) * 2 + 16, _FL_);
			PH.m_projMinMax = (Vector4*)(((dword)PH.m_projBuffer + 15) & ~15);
		}

		//m_lightFarPosition = vCamPos - lightDir * 4000.0f;
		//aSCObjects.QSort(SCOData::Sorter);

	enumTimer.Stop();

	dwNumCharactersCasted = 0;

	statDrawSplit[0] = 0;
	statDrawSplit[1] = 0;
	statDrawSplit[2] = 0;

	SetupXBOXFrustumRegisters();

	// Position the camera far plane as near as possible, to minimize the amount of empty space
	AdjustCameraPlanes();
	// Рассчитаываем near/far Z для сплитов 
	CalculateSplitDistances();
	// Рассчитваем минимальный/максимальный Y куда будет падать тень
	CalculateMinMaxReceiversY();

	if (!bPointLight)
	{
		Recv_ZPrePass();

		CalculateCastObjectParameters();

		for (int i=0; i<iNumSplits; i++)
		{
			// Кастим объекты в z-текстуру
			ShadowMap_Cast(i);

			Render().SetView(m_View);
			Render().SetProjection(m_Proj);

			Recv_ShadowPass(i);
		}
	}

	// восстанавливаем предыдущие матрицы
	Render().SetView(m_View);
	Render().SetProjection(m_Proj);

	// Получаем тень на все объекты
	// очищаем принимающую текстуру, если ни один персонаж не отрисовался
	if (bPointLight && !dwNumCharactersCasted)
		ShadowMap_Recv_Clear();

	if (!bPointLight)
		PH.pShdServ->Blur(1);

	// восстанавливаем все рендерные параметры
	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
	Render().PopViewport();
	Render().BeginScene();

	//Render().Print(vCamPos + (!vCamTarget) * 5.0f, 20.0f, 0.0f, 0xFFFFFFFF, "%d, %d", dwTT1, dwTT2);
}

// Создаются стандартные View/Projection матрицы
void PSShadowMap::CalculateViewProj(Matrix & mView, Matrix & mProj,
                       const Vector & vSource, const Vector & vTarget, const Vector & vUpVector,
                       float fFOV, float fNear, float fFar, float fAspect)
{
	// view матрица
	mView.BuildView(vSource, vTarget, vUpVector);

	// projection матрица
	mProj.BuildProjection(Deg2Rad(fFOV), float(shadowMapResolution), float(shadowMapResolution), fNear, fFar);
}

// Adjusts the camera planes to contain the visible scene
// as tightly as possible. This implementation is not very
// accurate.
void PSShadowMap::AdjustCameraPlanes()
{
	// use largest Z coordinate as new far plane
	fCameraFar = fShadowDistance;//fMaxZ + fCameraNear;
}

// Computes corner points of a frustum
void PSShadowMap::CalculateFrustumCorners(	Vector * pVPoints,
								const Vector & vCamSource, const Vector & vCamTarget, const Vector & vCamUp,
								float fNear, float fFar, float fFOV, float fAspect, float fScale)
{
	Vector vZ = !(vCamTarget - vCamSource);
	Vector vX = !(vCamUp ^ vZ);
	Vector vY = vZ ^ vX;

#ifndef _XBOX
	fFOV = Min(100.0f, fFOV);
#endif

	/*Vector vZ = m_invView.vz;//!(vCamTarget - vCamSource);
	Vector vX = m_invView.vx;//!(vCamUp ^ vZ);
	Vector vY = m_invView.vy;//vZ ^ vX;*/

	/*float fNearPlaneHeight = fFOV * fNear;
	//float fNearPlaneHeight = 1.0f / fFOV * fNear;
	float fNearPlaneWidth = fNearPlaneHeight * fAspect;

	float fFarPlaneHeight = fFOV * fFar;
	//float fFarPlaneHeight = 1.0f / fFOV * fFar;
	float fFarPlaneWidth = fFarPlaneHeight * fAspect;*/

	float fNearPlaneHeight = tanf(Deg2Rad(fFOV) * 0.5f) * fNear;
	float fNearPlaneWidth = fNearPlaneHeight * fAspect;

	float fFarPlaneHeight = tanf(Deg2Rad(fFOV) * 0.5f) * fFar;
	float fFarPlaneWidth = fFarPlaneHeight * fAspect;

	Vector vNearPlaneCenter = vCamSource + vZ * fNear;
	Vector vFarPlaneCenter = vCamSource + vZ * fFar;

	pVPoints[0] = vNearPlaneCenter - vX * fNearPlaneWidth - vY * fNearPlaneHeight;
	pVPoints[1] = vNearPlaneCenter - vX * fNearPlaneWidth + vY * fNearPlaneHeight;
	pVPoints[2] = vNearPlaneCenter + vX * fNearPlaneWidth + vY * fNearPlaneHeight;
	pVPoints[3] = vNearPlaneCenter + vX * fNearPlaneWidth - vY * fNearPlaneHeight;

	pVPoints[4] = vFarPlaneCenter - vX * fFarPlaneWidth - vY * fFarPlaneHeight;
	pVPoints[5] = vFarPlaneCenter - vX * fFarPlaneWidth + vY * fFarPlaneHeight;
	pVPoints[6] = vFarPlaneCenter + vX * fFarPlaneWidth + vY * fFarPlaneHeight;
	pVPoints[7] = vFarPlaneCenter + vX * fFarPlaneWidth - vY * fFarPlaneHeight;

	float c = fFarPlaneWidth * 0.5f;
	float b = fNearPlaneWidth * 0.5f;
	float l = fFar - fNear;
	float a = (c * c + l * l - b * b) / (2.0f * l);
	float k = sqrtf(b * b + a * a);

	// равноудаленный центр фрустума
#ifndef _XBOX
	if (a > l)
	{
		m_frustumTarget = vFarPlaneCenter;
		m_frustumDiameter = Max(fFarPlaneHeight, fFarPlaneWidth);
	}
	else
#endif
	{
		m_frustumTarget = vNearPlaneCenter + (vFarPlaneCenter - vNearPlaneCenter) * a / l;
		m_frustumDiameter = k * 2.0f;
	}

	// считаем центр фрустума
	Vector vCenter = 0.0f;
	for (long i=0; i<8; i++) 
		vCenter += pVPoints[i];
	vCenter = vCenter / 8.0f;

	// for each point
	for (long i=0; i<8; i++)
	{
		// scale by adding offset from center
		pVPoints[i] += (pVPoints[i] - vCenter) * (fScale - 1.0f);
	}
}

void PSShadowMap::CalculateLightForFrustum(long iIndex, Vector * pVCorners)
{
	Vector vMin = FLT_MAX;
	Vector vMax = -FLT_MAX;

	for (long i=0; i<8; i++)
	{
		if (pVCorners[i].x < vMin.x) vMin.x = pVCorners[i].x;
		if (pVCorners[i].y < vMin.y) vMin.y = pVCorners[i].y;
		if (pVCorners[i].z < vMin.z) vMin.z = pVCorners[i].z;

		if (pVCorners[i].x > vMax.x) vMax.x = pVCorners[i].x;
		if (pVCorners[i].y > vMax.y) vMax.y = pVCorners[i].y;
		if (pVCorners[i].z > vMax.z) vMax.z = pVCorners[i].z;
	}

	Vector vCenter = 0.0f;
	for (long i=0; i<8; i++) 
		vCenter += pVCorners[i];
	vCenter = vCenter / 8.0f;

	Vector vBox[8];

	vBox[0] = Vector(vMin.x, vMin.y, vMin.z);
	vBox[1] = Vector(vMax.x, vMin.y, vMin.z);
	vBox[2] = Vector(vMin.x, vMin.y, vMax.z);
	vBox[3] = Vector(vMax.x, vMin.y, vMax.z);
	vBox[4] = Vector(vMin.x, vMax.y, vMin.z);
	vBox[5] = Vector(vMax.x, vMax.y, vMin.z);
	vBox[6] = Vector(vMin.x, vMax.y, vMax.z);
	vBox[7] = Vector(vMax.x, vMax.y, vMax.z);

	float fMinDistance = FLT_MAX;
	float fMaxDistance = -FLT_MAX;
	Vector vBoxCenter = (vMin + vMax) / 2.0f;

	for (long i=0; i<8; i++)
	{
		Vector vCube = vBox[i] - vBoxCenter;
		float fDistance = vCube | lightDir;

		if (fDistance < fMinDistance) fMinDistance = fDistance;
		if (fDistance > fMaxDistance) fMaxDistance = fDistance;
	}

	lightSource = m_frustumTarget - (lightDir * fabsf(fMinDistance * 1.2f));
	lightTarget = m_frustumTarget;

	// рассчитываем light view и light projection матрицы
	CalculateViewProj(	mLightView, mLightProj,
						lightSource, lightTarget, Vector(0.0f, 1.0f, 0.0f),
						fLightFOV, fLightNear, fLightFarMax, 1.0f);

	// FIX-ME какое-то волшебство тут
#ifndef _XBOX
	float r = m_frustumDiameter * 1.5f;
#else
	float r = m_frustumDiameter * 1.37f;
#endif

	m_radius[iIndex] = float(long((r + 2.0f) / 2.0f) * 2.0f);

	mLightProj.BuildOrtoProjection(m_radius[iIndex], m_radius[iIndex], -100.0f, 400.0f);

	// убираем subpixel jittering
	double resolution = PH.pShdServ->GetShadowMapRes() * 0.5;

	Vector origin = Vector(0.0f) * (mLightView * mLightProj);

	double texCoordX = origin.x * resolution;
	double texCoordY = origin.y * resolution;

	double dx = floor(texCoordX + 0.5) - texCoordX;
	double dy = floor(texCoordY + 0.5) - texCoordY;

	dx /= resolution;
	dy /= resolution;

	mLightProj *= Matrix().BuildPosition(float(dx), float(dy), 0.0f);
}

Frustum::Frustum()
{
	needUpdate = false;
}

const Plane * Frustum::GetFrustum() const
{
	if (needUpdate)
		Update();
	return &m_planes[0];
}

const Vector4 * Frustum::GetPoints() const
{
	if (needUpdate)
		Update();

	return &m_points[0];
}

void Frustum::Update() const
{
	if (!needUpdate) return;

	m_invView = m_view; m_invView.Inverse();

	const Vector & vZ = m_invView.vz;
	const Vector & vX = m_invView.vx;
	const Vector & vY = m_invView.vy;

	float nearPlaneHeight = m_near / m_proj.m[0][0];
	float nearPlaneWidth = nearPlaneHeight * m_aspect;

	float farPlaneHeight = m_far / m_proj.m[0][0];
	float farPlaneWidth = farPlaneHeight * m_aspect;

	Vector nearCenter = m_invView.pos + vZ * m_near;
	Vector farCenter = m_invView.pos + vZ * m_far;

	m_points[0].v = nearCenter - vX * nearPlaneWidth - vY * nearPlaneHeight; m_points[0].w = 1.0f;
	m_points[1].v = nearCenter - vX * nearPlaneWidth + vY * nearPlaneHeight; m_points[1].w = 1.0f;
	m_points[2].v = nearCenter + vX * nearPlaneWidth + vY * nearPlaneHeight; m_points[2].w = 1.0f;
	m_points[3].v = nearCenter + vX * nearPlaneWidth - vY * nearPlaneHeight; m_points[3].w = 1.0f;

	m_points[4].v = farCenter - vX * farPlaneWidth - vY * farPlaneHeight; m_points[4].w = 1.0f;
	m_points[5].v = farCenter - vX * farPlaneWidth + vY * farPlaneHeight; m_points[5].w = 1.0f;
	m_points[6].v = farCenter + vX * farPlaneWidth + vY * farPlaneHeight; m_points[6].w = 1.0f;
	m_points[7].v = farCenter + vX * farPlaneWidth - vY * farPlaneHeight; m_points[7].w = 1.0f;

	// считаем плоскости

	Vector v[4];
    v[0] = !Vector(m_proj.m[0][0], 0.0f, 1.0f);
    v[1] = !Vector(-m_proj.m[0][0], 0.0f, 1.0f);
    v[2] = !Vector(0.0f, -m_proj.m[1][1], 1.0f);
    v[3] = !Vector(0.0f, m_proj.m[1][1], 1.0f);

	const Matrix & mIV = m_invView;

	m_planes[0].n = mIV.vz;
	m_planes[0].d = mIV.vz | (mIV.vz * m_near + mIV.pos);
	for (dword i=0; i<4; i++) 
	{
		m_planes[1 + i].n = mIV.MulNormal(v[i]);
		m_planes[1 + i].d = m_planes[1 + i].n | mIV.pos;
	}
	m_planes[5].n = -mIV.vz;
	m_planes[5].d = (mIV.vz * m_far + mIV.pos) | (-mIV.vz);

	// calculate center of points
	/*Vector vCenter = 0.0f;
	for (long i=0; i<8; i++) vCenter += pVPoints[i];
	vCenter = vCenter / 8.0f;

	// for each point
	for (long i=0; i<8; i++)
	{
		// scale by adding offset from center
		pVPoints[i] += (pVPoints[i] - vCenter) * (fScale - 1.0f);
	}*/
	
	needUpdate = false;
}

void Frustum::SetView(const Matrix & view)
{
	m_view = view;
	needUpdate = true;
}

void Frustum::SetProjection(const Matrix & proj)
{
	m_proj = proj;
	needUpdate = true;
}

void Frustum::SetAspect(float aspect)
{
	m_aspect = aspect;
	needUpdate = true;
}

void Frustum::SetNearFarZ(float _near, float _far)
{
	m_near = _near;
	m_far = _far;
	needUpdate = true;
}

#ifndef _XBOX
#include <xmmintrin.h>

__forceinline void SSE_GatherMatrixCol_Aligned( const float* col, __m128* pm128Result )
{
	*pm128Result = _mm_load_ps(col);
}

__forceinline void SSE_MatrixMulVertex(Vector4 & result, __m128 * matrix, const float & x, const float & y, const float & z)
{
	__m128 mx = _mm_mul_ps(matrix[0], _mm_set1_ps(x));
	__m128 my = _mm_mul_ps(matrix[1], _mm_set1_ps(y));
	__m128 mz = _mm_mul_ps(matrix[2], _mm_set1_ps(z));
	__m128 res0 = _mm_add_ps(mx, my);
	__m128 res1 = _mm_add_ps(mz, matrix[3]);
	_mm_store_ps((float*)&result, _mm_add_ps(res0, res1));

	// FIX-ME в принципе тут всегда должна быть 1.0f - может можно выкинуть
	//result.w = 1.0f;
}

#endif		// XBOX

#ifdef _XBOX
__forceinline XMVECTOR xmV3Transform(const XMVECTOR & VX, const XMVECTOR & VY, const XMVECTOR & VZ, const XMMATRIX & M)
{
    XMVECTOR X;
    XMVECTOR Y;
    XMVECTOR Result;

    Result = __vspltw(VZ, 2);
    Y = __vspltw(VY, 1);
    X = __vspltw(VX, 0);

    Result = __vmaddcfp(M.r[2], Result, M.r[3]);
    Result = __vmaddfp(Y, M.r[1], Result);
    Result = __vmaddfp(X, M.r[0], Result);

    return Result;
}
#endif

bool OverlapFrustumOBB(const Frustum & frustum, const Matrix & mtx, const Vector4 & min, const Vector4 & max)
{
	const Vector4 * frustumVerts = frustum.GetPoints();
	const Plane * frustumPlanes = frustum.GetFrustum();

#ifndef _XBOX
	__declspec(align(16)) Vector4 boxVerts[8];

	//t1.Start();
	__m128 mat128[4];
	const float * mtx1 = &mtx.matrix[0];
	SSE_GatherMatrixCol_Aligned(&mtx1[0], &mat128[0]);
	SSE_GatherMatrixCol_Aligned(&mtx1[4], &mat128[1]);
	SSE_GatherMatrixCol_Aligned(&mtx1[8], &mat128[2]);
	SSE_GatherMatrixCol_Aligned(&mtx1[12], &mat128[3]);

	SSE_MatrixMulVertex(boxVerts[0], mat128, min.x, min.y, min.z);
	SSE_MatrixMulVertex(boxVerts[1], mat128, max.x, min.y, min.z);
	SSE_MatrixMulVertex(boxVerts[2], mat128, min.x, min.y, max.z);
	SSE_MatrixMulVertex(boxVerts[3], mat128, max.x, min.y, max.z);
	SSE_MatrixMulVertex(boxVerts[4], mat128, min.x, max.y, min.z);
	SSE_MatrixMulVertex(boxVerts[5], mat128, max.x, max.y, min.z);
	SSE_MatrixMulVertex(boxVerts[6], mat128, min.x, max.y, max.z);
	SSE_MatrixMulVertex(boxVerts[7], mat128, max.x, max.y, max.z);
	//t1.Stop();

	// frustum planes vs box points
	//t2.Start();
	for (long i=0; i<6; i++)
	{
		bool show = false;
		for (long j=0; j<8; j++) 
		{
			float dist = frustumPlanes[i].Dist(boxVerts[j].v);
			if (dist > 0.0f)
			{
				show = true;
				break;
			}
		}
		if (!show) return false;
	}
	//t2.Stop();

	//t3.Start();
	// box planes vs frustum points
	Plane boxPlanes[6];
	boxPlanes[0] = Plane(mtx.vx, boxVerts[0].v);
	boxPlanes[1] = Plane(-mtx.vx, boxVerts[1].v);
	boxPlanes[2] = Plane(mtx.vy, boxVerts[0].v);
	boxPlanes[3] = Plane(-mtx.vy, boxVerts[4].v);
	boxPlanes[4] = Plane(mtx.vz, boxVerts[0].v);
	boxPlanes[5] = Plane(-mtx.vz, boxVerts[2].v);

	for (long i=0; i<6; i++)
	{
		bool show = false;
		for (long j=0; j<8; j++)
			if (boxPlanes[i].Dist(frustumVerts[j].v) > 0.0f)
			{
				show = true;
				break;
			}
		if (!show) return false;
	}
	//t3.Stop();

#else
	XMVECTOR xmBoxVerts[8];
	XMMATRIX & xmm = (XMMATRIX &)mtx;
	XMVECTOR xmZero = XMVectorZero();
	// грязный хак, чтобы получить -1 в компоненте w, чтобы потом делать dotproduct с плоскостью
	((Matrix*)&mtx)->m[3][3] = -1.0f;

	XMVECTOR & xmin = (XMVECTOR &)min;
	XMVECTOR & xmax = (XMVECTOR &)max;

	xmBoxVerts[0] = xmV3Transform(xmin, xmin, xmin, xmm);
	xmBoxVerts[1] = xmV3Transform(xmax, xmin, xmin, xmm);
	xmBoxVerts[2] = xmV3Transform(xmin, xmin, xmax, xmm);
	xmBoxVerts[3] = xmV3Transform(xmax, xmin, xmax, xmm);
	xmBoxVerts[4] = xmV3Transform(xmin, xmax, xmin, xmm);
	xmBoxVerts[5] = xmV3Transform(xmax, xmax, xmin, xmm);
	xmBoxVerts[6] = xmV3Transform(xmin, xmax, xmax, xmm);
	xmBoxVerts[7] = xmV3Transform(xmax, xmax, xmax, xmm);

	/*xmBoxVerts[0] = XMVector3Transform(XMVectorSet(min.x, min.y, min.z, 0.0f), xmm);
	xmBoxVerts[1] = XMVector3Transform(XMVectorSet(max.x, min.y, min.z, 0.0f), xmm);
	xmBoxVerts[2] = XMVector3Transform(XMVectorSet(min.x, min.y, max.z, 0.0f), xmm);
	xmBoxVerts[3] = XMVector3Transform(XMVectorSet(max.x, min.y, max.z, 0.0f), xmm);
	xmBoxVerts[4] = XMVector3Transform(XMVectorSet(min.x, max.y, min.z, 0.0f), xmm);
	xmBoxVerts[5] = XMVector3Transform(XMVectorSet(max.x, max.y, min.z, 0.0f), xmm);
	xmBoxVerts[6] = XMVector3Transform(XMVectorSet(min.x, max.y, max.z, 0.0f), xmm);
	xmBoxVerts[7] = XMVector3Transform(XMVectorSet(max.x, max.y, max.z, 0.0f), xmm);*/

	// убираем грязный хак
	((Matrix*)&mtx)->m[3][3] = 1.0f;

	XMVECTOR * xmFrustumPlanes = (XMVECTOR *)&frustumPlanes[0];
	// frustum planes vs box points
	//t2.Start();
	for (long i=0; i<6; i++)
	{
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[0]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[1]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[2]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[3]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[4]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[5]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[6]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmFrustumPlanes[i], xmBoxVerts[7]), xmZero)) continue;
		
		return false;
	}
	//t2.Stop();

	//t3.Start();
	// box planes vs frustum points
	// frustum verts have w = -1.0f
	const XMVECTOR * xmFrustumVerts = (XMVECTOR *)frustumVerts;

	XMVECTOR xmBoxPlanes[6];
	const XMVECTOR * xmMtx = (XMVECTOR*)&mtx;
	xmBoxPlanes[0] = XMPlaneFromPointNormal(xmBoxVerts[0], xmMtx[0]);
	xmBoxPlanes[1] = XMPlaneFromPointNormal(xmBoxVerts[1], -xmMtx[0]);
	xmBoxPlanes[2] = XMPlaneFromPointNormal(xmBoxVerts[0], xmMtx[1]);
	xmBoxPlanes[3] = XMPlaneFromPointNormal(xmBoxVerts[4], -xmMtx[1]);
	xmBoxPlanes[4] = XMPlaneFromPointNormal(xmBoxVerts[0], xmMtx[2]);
	xmBoxPlanes[5] = XMPlaneFromPointNormal(xmBoxVerts[2], -xmMtx[2]);

	/*xmBoxPlanes[0] = *(XMVECTOR*)&Plane(mtx.vx, xmBoxVerts[0].v);
	xmBoxPlanes[1] = *(XMVECTOR*)&Plane(-mtx.vx, xmBoxVerts[1].v);
	xmBoxPlanes[2] = *(XMVECTOR*)&Plane(mtx.vy, xmBoxVerts[0].v);
	xmBoxPlanes[3] = *(XMVECTOR*)&Plane(-mtx.vy, xmBoxVerts[4].v);
	xmBoxPlanes[4] = *(XMVECTOR*)&Plane(mtx.vz, xmBoxVerts[0].v);
	xmBoxPlanes[5] = *(XMVECTOR*)&Plane(-mtx.vz, xmBoxVerts[2].v);*/

	for (long i=0; i<6; i++)
	{	
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[0]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[1]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[2]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[3]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[4]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[5]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[6]), xmZero)) continue;
		if (XMVector4Greater(XMVector4Dot(xmBoxPlanes[i], xmFrustumVerts[7]), xmZero)) continue;

		return false;
	}

#endif

	return true;
}


MOP_BEGINLIST(PSShadowMap, "PSShadowMap", '1.00', 101);
	MOP_ANGLESC("Light Dir", Vector(1.0f, 0.0f, 0.0f), "Угол откуда светит источник")
	MOP_COLORC("Shadow Color", Color(0.0f, 0.0f, 0.0f), "Цвет тени от объектов")
	MOP_FLOATEXC("Shadow power", 1.0f, 0.0f, 50.0f, "Мощность тени на статических/физических объектах");
	MOP_FLOATEXC("Char shadow power", 1.0f, 0.0f, 50.0f, "Мощность тени на персонажах");
	MOP_FLOATEXC("Shd_k1", 1.0f, 0.0f, 1.0f, "Делать меньше если есть артефакты");
	MOP_FLOATEXC("Shd_k2", 1.0f, 0.0f, 1.0f, "");
	MOP_FLOATEXC("Blur", 0.6f, 0.01f, 4.0f, "Сила сглаживания теней")
	MOP_FLOATEX("Blur Angle", 180.0f, 1.0f, 360.0f)
	MOP_LONGEXC("NumSplits", 3, 1, 3, "Кол-во разбиений теней")
	MOP_FLOATC("ZBias", 0.01f, "Делать больше если есть z-алиасинг у статичных объектов")
	MOP_FLOATC("Character ZBias", 0.01f, "Делать больше если есть z-алиасинг у персонажей")
	MOP_FLOATC("Box", 40.0f, "Размер области в которой видны тени, меньше область - тем четче тени")
	MOP_FLOATEXC("kF", 0.75f, 0.0f, 1.0f, "Коэффициент, от него зависит величина областей теней")
	MOP_FLOATC("Near Z", 3.0f, "Начальное положение теней от камеры")
	MOP_FLOATC("Min Shadow Volume Y", -1000.0f, "Мин высота куда падает тень")
	MOP_FLOATC("Max Shadow Volume Y", 1000.0f, "Макс высота куда падает тень")
	MOP_BOOLC("As Point Light", false, "true = Источник света точечный")
	MOP_ARRAYBEG("Point lights array", 0, 10)
		MOP_STRINGC("Name", "", "Имя источника света")
		MOP_FLOATC("Distance", -1.0f, "Размер области в которой видны тени от этого источника, -1.0 = берется параметр Box")
	MOP_ARRAYEND
	MOP_ARRAYBEG("Z Pass Planes", 0, 10000)
		MOP_POSITIONC("Position", 0.0f, "Центр плоскости")
		MOP_ANGLESC("Angles", 0.0f, "Ориентация плоскости")
		MOP_FLOATEXC("Width", 10.0f, 0.01f, 5000.0f, "Ширина плоскости")
		MOP_FLOATEXC("Length", 10.0f, 0.01f, 5000.0f, "Длина плоскости")
	MOP_ARRAYEND
	MOP_ARRAYBEG("Clip planes", 0, 10000)
		MOP_POSITIONC("Position | cp |", 0.0f, "Центр плоскости")
		MOP_ANGLESC("Angles | cp |", 0.0f, "Ориентация плоскости")
	MOP_ARRAYEND
	MOP_BOOL("Show Debug", false)
	MOP_BOOL("Active", true)
	//MOP_FLOATEX("TEST", 0.0f, -40.0f, 40.0f)
MOP_ENDLIST(PSShadowMap)
