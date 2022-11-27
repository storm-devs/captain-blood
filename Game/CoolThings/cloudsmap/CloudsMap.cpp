#include "CloudsMap.h"






//Конструктор
CloudsMap::CloudsMap() 
{
	pTex = NULL;
	CloudsMapParams = NULL;
	CloudsMapOffsetX = NULL;
	CloudsMapOffsetZ = NULL;
	CloudsMapTexture = NULL;
	CloudsColor = NULL;

	fSpeedX = 0.0f;
	fSpeedZ = 0.0f;

	curOffset = 0.0f;

	multiplerX = 1.0f;
	multiplerZ = 1.0f;


}

//Деструктор
CloudsMap::~CloudsMap()
{
	if (pTex) pTex->Release();
	pTex = NULL;

	CloudsMapParams = NULL;
	CloudsMapTexture = NULL;
	CloudsMapOffsetX = NULL;
	CloudsMapOffsetZ = NULL;
	CloudsColor = NULL;

}


//Создание объекта
bool CloudsMap::Create(MOPReader & reader)
{
	CloudsMapParams = Render().GetTechniqueGlobalVariable("CloudsMapParams", _FL_);
	CloudsMapTexture = Render().GetTechniqueGlobalVariable("CloudsMapTexture", _FL_);
	
	CloudsMapOffsetX = Render().GetTechniqueGlobalVariable("CloudsMapOffsetX", _FL_);
	CloudsMapOffsetZ = Render().GetTechniqueGlobalVariable("CloudsMapOffsetZ", _FL_);

	CloudsColor = Render().GetTechniqueGlobalVariable("CloudsColor", _FL_);


	EditMode_Update (reader);
	SetUpdate((MOF_UPDATE)&CloudsMap::Realize, ML_GEOMETRY1);
	return true;
}


//Обновление параметров
bool CloudsMap::EditMode_Update(MOPReader & reader)
{

	curOffset = 0.0f;
	vPos = reader.Position();

	Width = reader.Float();
	Depth = reader.Float();

	pTex = NULL;
	pTex = Render().CreateTexture(_FL_, "%s", reader.String());

	fSpeedX = reader.Float();
	fSpeedZ = reader.Float();

	cloudShadowColor = reader.Colors();

	float fPower = reader.Float();
	cloudShadowColor = cloudShadowColor * fPower;

	bVis = reader.Bool();
	bActive = reader.Bool();

	Activate(bActive);


	return true;
}

void CloudsMap::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if (IsActive())
	{
		SetupVariables ();
		Console().Trace(COL_ALL, "Lighting map '%s' enabled", GetObjectID().c_str());
	} else
	{
		Console().Trace(COL_ALL, "Lighting map '%s' disabled", GetObjectID().c_str());
	}

}


void _cdecl CloudsMap::Realize(float fDeltaTime, long level)
{
	if (!EditMode_IsOn()) return;


	curOffset.x += ((fSpeedX*multiplerX) * fDeltaTime);
	curOffset.z += ((fSpeedZ*multiplerZ) * fDeltaTime);


	if (CloudsMapOffsetX)
	{
		CloudsMapOffsetX->SetFloat(curOffset.x);
	}

	if (CloudsMapOffsetZ)
	{
		CloudsMapOffsetZ->SetFloat(curOffset.z);
	}

	if (!bVis) return;
	Quad[0].vPos = vPos + Vector (-Width, 0, -Depth);
	Quad[1].vPos = vPos + Vector (Width, 0, -Depth);
	Quad[2].vPos = vPos + Vector (-Width, 0, Depth);
	Quad[3].vPos = vPos + Vector (Width, 0, Depth);

	Quad[0].u = 1.0f; Quad[0].v = 0.0f;
	Quad[1].u = 0.0f; Quad[1].v = 0.0f;
	Quad[2].u = 1.0f; Quad[2].v = 1.0f;
	Quad[3].u = 0.0f; Quad[3].v = 1.0f;

	// fix me !!!
	//Render().SetTexture(0, pTex);

	Render().SetWorld(Matrix());

	ShaderId LMPreview_id;
	Render().GetShaderId("LMPreview", LMPreview_id);
	Render().DrawPrimitiveUP(LMPreview_id, PT_TRIANGLESTRIP, 2, Quad, sizeof(PreviewVertex));

}

void CloudsMap::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = vPos + Vector (-Width, -0.1f, -Depth);
	max = vPos + Vector (Width, 0.1f, Depth);
}


void CloudsMap::SetupVariables ()
{
	float fSize1 = Width * 2.0f;
	float fMul1 = 1.0f / fSize1;
	//float fAdd1 = -(1.0f / (vPos.x - Width));
	float fAdd1 = 0.5f - (-vPos.x / fSize1);

	float fSize2 = Depth * 2.0f;
	float fMul2 = 1.0f / fSize2;
	//float fAdd2 = -(1.0f / (vPos.z - Depth));
	float fAdd2 = 0.5f - (vPos.z / fSize2);

	Vector4 vConst(fMul1, fAdd1, fMul2, fAdd2);

	multiplerX = fMul1;
	multiplerZ = fMul2;

	if (CloudsMapParams)
	{
		CloudsMapParams->SetVector4(vConst);
	}

	if (CloudsMapTexture)
	{
		CloudsMapTexture->SetTexture(pTex);
	}

	if (CloudsColor)
	{
		CloudsColor->SetVector4(cloudShadowColor.v4);
	}

}


MOP_BEGINLIST(CloudsMap, "CloudsMap", '1.00', 100)

	MOP_POSITION("Position", Vector(0.0f))
	MOP_FLOAT("Width", 10.0f)
	MOP_FLOAT("Depth", 10.0f)
	MOP_STRING("Texture", "");
	MOP_FLOAT("Speed X", 1);
	MOP_FLOAT("Speed Z", 1);
	MOP_COLOR("Color", Color(0.0f, 0.0f, 0.0f));
	MOP_FLOAT("Intensivity", 1.0f);
	MOP_BOOL("Visible", true);
	MOP_BOOL("Active", true);

MOP_ENDLIST(CloudsMap)





