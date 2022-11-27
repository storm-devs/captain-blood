#include "Sky.h"

#define SKY_GROUP	GroupId('s','y','g','2')

Sky::Sky()
{
	bNoSwing = true;
	fAngleY = 0.0f;
	fAngleY2 = 0.0f;
	pSkyGeo = null;
	pSkyGeo2 = null;
	bReflection = false;
	oceanMode = null;
	pVarSkyRotate = null;
	m_postDraw = false;

	//bShowBackGeometry = true;
	/*iFogNumVerts = 0;
	iFogNumTrgs = 0;

	pFogVB = null;
	pFogIB = null;*/

}

Sky::~Sky()
{
	Release();
}

void Sky::Release()
{
	pVarSkyRotate = NULL;
	RELEASE(pSkyGeo);
	RELEASE(pSkyGeo2);
	RELEASE(oceanMode);
	//RELEASE(pFogVB);
	//RELEASE(pFogIB);
}

bool Sky::Create(MOPReader & reader)
{
	Registry(SKY_GROUP);

	Geometry().GetRenderModeShaderId("SkyFog", SkyFog_id);
	Geometry().GetRenderModeShaderId("SkyFogSecond", SkyFog2_id);

	pVarSkyRotate = Render().GetTechniqueGlobalVariable("Ocean2SkyRotate", _FL_);

	varSkyColor = Render().GetTechniqueGlobalVariable("cSkyColor", _FL_);

//	Registry(MG_DOF, (MOF_EVENT)&Sky::SeaReflection, ML_FILL);

//	Registry("PostGlow",&Sky::Draw,ML_POSTEFFECTS);

	bool result = EditMode_Update(reader);
	startAngleY = vAngle.y;
	return result;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void Sky::PostCreate()
{
	FindObject(connectToName, connectToPtr);
}

bool Sky::EditMode_Create(MOPReader & reader)
{
	return Create(reader);
}

bool Sky::EditMode_Update(MOPReader & reader)
{
	//const char * pNewBackModel = reader.String();
	const char * pNewModelName = reader.String().c_str();
	const char * pNewTexturePath = reader.String().c_str();

	Geometry().SetTexturePath((pNewTexturePath[0]) ? NULL : pNewTexturePath);
	IGMXScene * pSkyGeoNew = Geometry().CreateGMX(pNewModelName, &Mission().Animation(), &Mission().Particles(), &Mission().Sound());
	Geometry().SetTexturePath("");
	RELEASE(pSkyGeo);
	RELEASE(pSkyGeo2);
	pSkyGeo = pSkyGeoNew;

	vPosition = reader.Position();
	vAngle = reader.Angles();
	vScale = reader.Position();
	//vScale = 1.0f;

	connectToName = reader.String();

	cColor = reader.Colors();

	fSkySpeedRotate = reader.Float() * 0.001f;

	bReflDublicate = reader.Bool();

	bNoSwing = reader.Bool();

	// second geometry
	{
		const char * pNewModelName2 = reader.String().c_str();
		IGMXScene * oldSky2Geo = pSkyGeo2;
		if (pNewModelName2 && pNewModelName2[0])
			pSkyGeo2 = Geometry().CreateGMX(pNewModelName2, &Mission().Animation(), &Mission().Particles(), &Mission().Sound());
		else
			pSkyGeo2 = null;
		RELEASE(oldSky2Geo);
		vAngle2 = reader.Angles();
		fSkySpeedRotate2 = reader.Float() * 0.001f;
		cColor2 = reader.Colors();
	}

	colFog = reader.Colors();
	fFogHeight = reader.Float();
	fFogMultiply = reader.Float();

	pFogColor = Render().GetTechniqueGlobalVariable("vSkyFogColor", _FL_);
	pFogCoeff = Render().GetTechniqueGlobalVariable("fSkyFogMultiply", _FL_);

	bool isActive = reader.Bool();

	Activate(isActive);

	//CreateFogSphere(fFogHeight);

	return true;
}

//Получить бокс, описывающий объект в локальных координатах
void Sky::GetBox(Vector & min, Vector & max)
{
	if (pSkyGeo)
	{
		const GMXBoundBox & bb = pSkyGeo->GetLocalBound();
		min = bb.vMin;
		max = bb.vMax;
		return;
	}

	min = 0.0f;
	max = 0.0f;
}

//Получить матрицу объекта
Matrix & Sky::GetMatrix(Matrix & mtx)
{
	mtx.SetIdentity();
	mtx.pos = Render().GetView().GetCamPos() + Vector(vPosition.x, 0.0f, vPosition.z);
	mtx.pos.y = 0.0f;

	return mtx;
}


void Sky::Realize(float fDeltaTime, long level)
{
	Matrix view = Render().GetView();
	if (!bNoSwing)
	{
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
	}

	pFogColor->SetVector(colFog.color);
	pFogCoeff->SetVector(Vector(fFogMultiply, (fabsf(fFogHeight) > 1e-5f) ? 1.0f / fFogHeight : 0.0f, 0.0f));

	Render().SetWorld(Matrix());

	long save_cm = Render().GetStateHack(SH_CULLMODE);
	long save_cp = Render().GetStateHack(SH_CLIPPLANEENABLE);
	long save_cw = Render().GetStateHack(SH_COLORWRITE);
	long save_zw = Render().GetStateHack(SH_ZWRITE);

	fAngleY += fDeltaTime * fSkySpeedRotate;
	fAngleY2 += fDeltaTime * fSkySpeedRotate2;

	Vector vPos = Render().GetView().GetCamPos() + Vector(vPosition.x, 0.0f, vPosition.z);

	Matrix mSky(vAngle.x, fAngleY + vAngle.y, vAngle.z, vPos.x, vPos.y / 6.0f + vPosition.y, vPos.z);
	Matrix mSky2(vAngle2.x, fAngleY2 + vAngle2.y, vAngle2.z, vPos.x, vPos.y / 6.0f + vPosition.y, vPos.z);

	if(connectToPtr.Validate())
	{
		Matrix rot;
		connectToPtr.Ptr()->GetMatrix(rot);
		if(rot.BuildOrient(rot.vz, Vector(0.0f, 1.0f, 0.0f)))
		{
			rot.pos = 0.f;
			mSky = rot * mSky;
			mSky2 = rot * mSky2;
		}
	}

	float angle = vAngle.y + fAngleY - startAngleY;
	float fsin = sinf(angle);
	float fcos = cosf(angle);
	pVarSkyRotate->SetVector4(Vector4(fcos, fsin, -fsin, fcos));

	if (bReflection)
		Render().SetStateHack(SH_CLIPPLANEENABLE, 0);

	IGMXService::RenderModeShaderId * oldRenderMode = Geometry().GetRenderMode();

	if (pSkyGeo)
	{
		varSkyColor->SetVector4(cColor.v4);
		Render().SetStateHack(SH_ZWRITE, SHZW_DISABLE);
		Matrix mWorld = Matrix().BuildScale(vScale) * mSky;
		pSkyGeo->SetTransform(mWorld);
		if (!m_postDraw)
			Geometry().SetRenderMode(&SkyFog_id);

		pSkyGeo->Draw();
	
		if (!m_postDraw)
			Geometry().SetRenderMode(oldRenderMode);
		//DrawFogSphere(mWorld);
		Render().SetStateHack(SH_ZWRITE, save_zw);
	}

	if (pSkyGeo2)
	{
		varSkyColor->SetVector4(cColor2.v4);
		Render().SetStateHack(SH_ZWRITE, SHZW_DISABLE);
		pSkyGeo2->SetTransform(mSky2);
		if (!m_postDraw)
			Geometry().SetRenderMode(&SkyFog2_id);

		pSkyGeo2->Draw();
	
		if (!m_postDraw)
			Geometry().SetRenderMode(oldRenderMode);
		//DrawFogSphere(mWorld);
		Render().SetStateHack(SH_ZWRITE, save_zw);
	}

/*
	if (pBackGeo && bShowBackGeometry)
	{
		bool bCullState = Geometry().GetGlobalFrustumCullState();
		Geometry().SetGlobalFrustumCullState(false);
		pBackGeo->SetTransform(Matrix());
		pBackGeo->Draw();
		Geometry().SetGlobalFrustumCullState(bCullState);
	}
*/

	if (pSkyGeo && bReflection && bReflDublicate)
	{
		varSkyColor->SetVector4(cColor.v4);
		Render().SetStateHack(SH_ZWRITE, SHZW_DISABLE);
		Render().SetStateHack(SH_CULLMODE, SHCM_NONE);
		pSkyGeo->SetTransform((Matrix().BuildScale(vScale.x, -vScale.y, vScale.z) * mSky));
		Geometry().SetRenderMode(&SkyFog_id);
		pSkyGeo->Draw();			
		Geometry().SetRenderMode(oldRenderMode);
		//Восстанавливает CULLMODE в INVERSE - могут быть проблемы
	}

	Render().SetStateHack(SH_CULLMODE, save_cm);
	Render().SetStateHack(SH_CLIPPLANEENABLE, save_cp);
	Render().SetStateHack(SH_COLORWRITE, save_cw);
	Render().SetStateHack(SH_ZWRITE, save_zw);

	if (!bNoSwing)
	{
		Render().SetView(view);
	}
}

// Используется при генерации sphere map'a в Ocean'e, сбрасывает угол текущего поворота в 0.0f, 
// в основном фикс для редактора миссий
void _cdecl Sky::SphereMapGen(GroupId,long)
{
	float oldAY = fAngleY;
	float oldAY2 = fAngleY2;
	fAngleY = 0.0f;
	fAngleY2 = 0.0f;
	SeaReflection();
	fAngleY = oldAY;
	fAngleY2 = oldAY2;
}

void _cdecl Sky::SeaReflection()
{
	bReflection = true;
	Realize(0.0f, 0);
	bReflection = false;
}
/*
void _cdecl Sky::Draw(const char *group, MissionObject *sender)
{
	Realize(0.0f, 0);
}
*/
void Sky::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEqual(id,"PostDraw") )
	{
		//bShowBackGeometry = false;
		m_postDraw = true;
		Realize(0.0f, 0);
		m_postDraw = false;
		//bShowBackGeometry = true;
		return;
	}
}

void Sky::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		SetUpdate((MOF_UPDATE)&Sky::Realize, ML_FILL);
	
		Registry(MG_SEAREFLECTION, (MOF_EVENT)&Sky::SeaReflection, ML_FILL);
		Registry(GroupId('S','M','G','n'), (MOF_EVENT)&Sky::SphereMapGen, ML_FILL);

		// ищем другие небеса и деактивируем их
		MGIterator * iter = &Mission().GroupIterator(SKY_GROUP, _FL_);
		for (iter->Reset();!iter->IsDone();iter->Next())
		{
			MissionObject * other_sky = iter->Get();
			if (other_sky->IsActive() && other_sky != this)
				other_sky->Activate(false);
		}
		iter->Release();	

		Registry(SKY_GROUP);
	}
	else
	{
		DelUpdate();
		Unregistry();
	}

	if (!EditMode_IsOn())
		LogicDebug("Sky '%s' %s", GetObjectID().c_str(), (IsActive()) ? "activated" : "deactivated");
}

MOP_BEGINLISTCG(Sky, "Sky", '1.00', -100, "", "Weather")
	//MOP_STRING("Back Model", "")
	MOP_STRING("Model", "Sky")
	MOP_STRING("Texture Path", "")
	MOP_POSITION("Position", Vector(0.0f, 0.0f, 0.0f))
	MOP_ANGLES("Angle", Vector(0.0f, 0.0f, 0.0f))
	MOP_POSITION("Scale | v2 |", Vector(1.0f, 1.0f, 1.0f))
	MOP_STRINGC("Connect to", "", "Connect sky to object (only Y rotation)")
	MOP_COLORC("Color", Color(1.0f, 1.0f, 1.0f, 1.0f), "Color and transparency of sky");
	MOP_FLOAT("Rotate", 0.0f)
	MOP_BOOL("Reflection duplicate", true);
	MOP_BOOLC("No swing", true, "No swing sky in swing machine")
	MOP_GROUPBEG("Second Geo")
		MOP_STRING("Model2", "")
		MOP_ANGLES("Angle2", Vector(0.0f, 0.0f, 0.0f))
		MOP_FLOAT("Rotate2", 0.0f)
		MOP_COLORC("Color2", Color(1.0f, 1.0f, 1.0f, 1.0f), "Color and transparency of second sky");
	MOP_GROUPEND()
	MOP_GROUPBEG("Fog parameters")
		MOP_COLORC("Fog color", Color(0.54f, 0.8f, 0.86f), "Цвет тумана");
		MOP_FLOATEXC("Fog height", 200.0f, 0.0f, 100000.0f, "Высота тумана");
		MOP_FLOATEXC("Fog multiply", 1.0f, 0.0f, 1.0f, "Коэффициент на туман");
	MOP_GROUPEND()
	MOP_BOOL("Active", true)
MOP_ENDLIST(Sky)
