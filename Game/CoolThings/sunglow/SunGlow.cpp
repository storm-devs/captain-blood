#include "..\..\..\common_h/mission/Mission.h"
#include "SunGlow.h"




#define MAX_BB_COUNT 32


//Конструктор
SunGlow::SunGlow() : Billboards(_FL_, 8)
														
{
	fArea = 10.0f;
	pRS = (IRender*)api->GetService("DX9Render");
	Assert(pRS);

	//vBBCameraPos = pRS->GetTechniqueGlobalVariable("vBBCameraPos", _FL_);
	SunTexture = pRS->GetTechniqueGlobalVariable("SunTexture", _FL_);
	
	m_newPowDelta = 0.0f;
	m_newPowTime = 0.0f;

	pVBuffer = NULL;
	pIBuffer = NULL;

	pTextureFrame1 = NULL;


}

//Деструктор
SunGlow::~SunGlow()
{	
	SunTexture = NULL;
	//vBBCameraPos = NULL;

	if (pVBuffer) pVBuffer->Release();
	pVBuffer = NULL;

	if (pIBuffer) pIBuffer->Release();
	pIBuffer = NULL;

	if (pTextureFrame1) pTextureFrame1->Release();
	pTextureFrame1 = NULL;

}



void SunGlow::CreateBuffers ()
{
	if (pVBuffer) pVBuffer->Release();
	if (pIBuffer) pIBuffer->Release();

	pVBuffer = NULL;
	pIBuffer = NULL;

	int RectVertexSize =  sizeof(RECT_VERTEX);
	
	pVBuffer = pRS->CreateVertexBuffer(MAX_BB_COUNT * 4 * RectVertexSize, RectVertexSize, _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
	Assert (pVBuffer != NULL);

	pIBuffer = pRS->CreateIndexBuffer(MAX_BB_COUNT * 6 * sizeof(WORD), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC); 
	Assert (pIBuffer != NULL);
	WORD * pTrgs = (WORD*)pIBuffer->Lock(); 
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
	pIBuffer->Unlock();


}

//Создание объекта
bool SunGlow::Create(MOPReader & reader)
{
	CreateBuffers();

	EditMode_Update (reader);

	return true;
}

void SunGlow::LoadTexture(const char* szTex1)
{
	if (sTextureName1 != szTex1)
	{
		if (pTextureFrame1) pTextureFrame1->Release();
		pTextureFrame1 = pRS->CreateTexture(_FL_, "%s", szTex1);

		sTextureName1 = szTex1;
	}
}




//Обновление параметров
bool SunGlow::EditMode_Update(MOPReader & reader)
{
	pRS->GetShaderId("BBCollection_Add2X", techId);
	
	
	vPosition = reader.Position();
	fSize = reader.Float();
	powScale = reader.Float();


	LoadTexture (reader.String().c_str());

	glowColor = reader.Colors();
	bShow = reader.Bool();
	bActive = reader.Bool();

	ChangeStatus();


	Billboards.DelAll();


	BillBoard* pBill = &Billboards[Billboards.Add()];

	pBill->fSizeX = fSize;
	pBill->fSizeY = fSize;
	pBill->fSpeed = 0.0f;
	pBill->vPos = vPosition;

	return true;
}

void SunGlow::ChangeStatus ()
{
	if (bShow && bActive)
	{
		SetUpdate((MOF_UPDATE)&SunGlow::Realize, ML_WEATHER5);
		return;
	}

	DelUpdate((MOF_UPDATE)&SunGlow::Realize);


}



void _cdecl SunGlow::Realize(float fDeltaTime, long level)
{
	if (!pVBuffer) return;



	pRS->SetWorld(Matrix());

	RECT_VERTEX * pVerts = (RECT_VERTEX*)pVBuffer->Lock(0, 0, 0);

	if (!pVerts) return;

	Vector vCamPos = Render().GetView().GetCamPos();
	Vector vPosition = Billboards[0].vPos;

	Vector vDir = vPosition - vCamPos;
	vDir.Normalize();


	Matrix m = Render().GetView();
	m.Inverse();
	Vector vCamDir = m.vz;
	vCamDir.Normalize();


	float fBright = Clampf (vDir | vCamDir);
	
	// Вано добавил
	if (m_newPowTime > 0.0f)
	{
		float time = Min(fDeltaTime, m_newPowTime);
		powScale += time * m_newPowDelta;
		m_newPowTime -= time;
	}

	fBright = pow(fBright,  powScale);

	//Render().PrintBuffered(0, 0, 0xFFFFFFFF, "Sun flare bright %f", fBright);
/*
	

	Render().DrawLine(Vector(0.0f), 0xFF00FF00, vDir * 10.0f, 0xFF00FF00);
	Render().DrawLine(Vector(0.0f), 0xFFFFFF00, vCamDir * 10.0f, 0xFFFFFF00);
*/

	Color curColor = glowColor;

	curColor *= fBright;
	dwColor = curColor.GetDword();

	long Index = 0;
	DWORD RenderBillboardsCount = 0;
	for (DWORD j = 0; j <  Billboards.Size(); j++)
	{
		Vector vPos = Billboards[j].vPos;

		RECT_VERTEX	* pV = &pVerts[Index * 4];
		Index++;

		float fSizeX = Billboards[j].fSizeX;
		float fSizeY = Billboards[j].fSizeY;
		float fAngle = PI;
		float fDissolve = 1.0f;


		pV[0].vRelativePos = Vector(-fSizeX, -fSizeY, 0.0f);
		pV[0].dwColor = dwColor;
		pV[0].tu1 = 0.0f; pV[0].tv1 = 0.0f;
		pV[0].angle = fAngle; 
		pV[0].vParticlePos = vPos;
		pV[0].fDisolve = fDissolve;

		pV[1].vRelativePos = Vector(-fSizeX, fSizeY, 0.0f);
		pV[1].dwColor = dwColor;
		pV[1].tu1 = 0.0f; pV[1].tv1 = 1.0f;
		pV[1].angle = fAngle; 
		pV[1].vParticlePos = vPos;
		pV[1].fDisolve = fDissolve;

		pV[2].vRelativePos = Vector(fSizeX, fSizeY, 0.0f);
		pV[2].dwColor = dwColor;
		pV[2].tu1 = 1.0f; pV[2].tv1 = 1.0f;
		pV[2].angle = fAngle; 
		pV[2].vParticlePos = vPos;
		pV[2].fDisolve = fDissolve;

		pV[3].vRelativePos = Vector(fSizeX, -fSizeY, 0.0f);
		pV[3].dwColor = dwColor;
		pV[3].tu1 = 1.0f; pV[3].tv1 = 0.0f;
		pV[3].angle = fAngle; 
		pV[3].vParticlePos = vPos;
		pV[3].fDisolve = fDissolve;




		RenderBillboardsCount++;
	}

	pVBuffer->Unlock();


	//if (vBBCameraPos) vBBCameraPos->SetVector(vCamPos);

	//pRS->SetTexture(0, pTextureFrame1);
	pRS->SetStreamSource(0, pVBuffer);
	pRS->SetIndices(pIBuffer, 0);

	if (SunTexture)
	{
		if(pTextureFrame1)
		{
			SunTexture->SetTexture(pTextureFrame1);
		}else{
			SunTexture->ResetTexture();
		}
	}

	pRS->DrawIndexedPrimitive(techId, PT_TRIANGLELIST, 0, RenderBillboardsCount * 4, 0, RenderBillboardsCount * 2);
}


void SunGlow::Activate(bool isActive)
{
	bActive = isActive;
	ChangeStatus();

}

void SunGlow::Command(const char * id, dword numParams, const char ** params)
{
	if (string::IsEqual(id, "changepower"))
	{
		if (numParams != 2)
		{
			LogicDebugError("Not enough parameters");
			return;
		}

		m_newPowTime = (float)atof(params[1]);
		float newPowScale = (float)atof(params[0]);
		if (m_newPowTime <= 1e-7f)
		{
			powScale = (float)atof(params[0]);
			m_newPowTime = 0.0f;
			return;
		}
		m_newPowDelta = (newPowScale - powScale) / m_newPowTime;
	}
}

MOP_BEGINLISTG(SunGlow, "Sun", '1.00', 100, "Weather")
  MOP_POSITION("Position", Vector(0.0f));
	MOP_FLOAT ("Size", 10.0f);
	MOP_FLOAT ("Attenuation power", 32.0f);
	MOP_STRING ("Texture", "")
	MOP_COLOR("Color", Color(1.0f, 1.0f, 1.0f))
	MOP_BOOL("Show", true)
	MOP_BOOL("Active", true)
MOP_ENDLIST(SunGlow)



