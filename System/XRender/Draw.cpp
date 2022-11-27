#include "Render.h"
#include "Capsule/Capsule.h"

#include "MaxLights.h"

#include "Shaders/Shader.h"
#include "Shaders/ShaderVar.h"












__forceinline bool NeedDrawRotateGizmoLine (const Vector& vCenter, const Vector &v1, const Vector &v2, const Vector& vCamDir)
{
	//return true;

	Vector dir1 = v1 - vCenter;
	Vector dir2 = v1 - vCenter;

	if ((vCamDir | dir1) > 0) return false;
	if ((vCamDir | dir2) > 0) return false;

	return true;
}


__forceinline bool LightIsUsed (const Vector & camPos, ILight* light, const GMXOBB &boundBox, const Plane* Frustum, Color & recalculatedColor)
{
	//получаем позицию источника в пространстве коробки....
	const Vector& vLightPos = light->GetPos();
	float fRadius = light->GetRadius();


	float fLodMinDist = light->getLOD_minDist_Squared();
	float fLodMaxDist = light->getLOD_maxDist_Squared();
	float fLodLen = light->getLOD_len_Squared();
	recalculatedColor = light->GetColor();

	Vector dist = vLightPos - camPos;
	float fDistToLightSquared = dist | dist;


	//TODO: тут float compare на 260 тыщ тактов...

	if (fDistToLightSquared > fLodMinDist) 
	{
		if (fDistToLightSquared > fLodMaxDist) 
		{
			return false;
		}

		float fColorScaleK = 1.0f - ((fDistToLightSquared - fLodMinDist) * fLodLen);
		recalculatedColor = recalculatedColor * fColorScaleK;
	}


	for(long p = 0; p < 5; p++)
	{
		if(((vLightPos | Frustum[p].n) - Frustum[p].d) < -fRadius)
		{
			return false;
		}
	}


	Vector vLightPosInBoxCoord = boundBox.GetTransform().MulVertexByInverse(vLightPos);
	const Vector& extents = boundBox.GetExtents();

	if (vLightPosInBoxCoord.x >= 0)
	{
		//по оси X вышли за пределы источника
		if ((vLightPosInBoxCoord.x - extents.x) > fRadius) return false;
	} else
	{
		//по оси X вышли за пределы источника
		if ((vLightPosInBoxCoord.x + extents.x) < -fRadius)  return false;
	}


	if (vLightPosInBoxCoord.y >= 0)
	{
		//по оси Y вышли за пределы источника
		if ((vLightPosInBoxCoord.y - extents.y) > fRadius)  return false;
	} else
	{
		//по оси Y вышли за пределы источника
		if ((vLightPosInBoxCoord.y + extents.y) < -fRadius)  return false;
	}

	if (vLightPosInBoxCoord.z >= 0)
	{
		//по оси Z вышли за пределы источника
		if ((vLightPosInBoxCoord.z - extents.z) > fRadius)  return false;
	} else
	{
		//по оси Z вышли за пределы источника
		if ((vLightPosInBoxCoord.z + extents.z) < -fRadius)  return false;
	}


	return true;
	
}


void NGRender::SetLightVariables (ShaderId shaderId, ILight** lightsArray, dword dwArraySize)
{
	SHADER* shader = (SHADER*)(shaderId);
	

	if (lightsArray == NULL || dwArraySize == 0)
	{
		//Надо направленный источник, на все остальное насрать
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
		m_ShaderManager.getVarDatabase()->ApplyLightingVariables30(stdLightVariables, 0);
#else
		shader->ApplyLightingVariables30(stdLightVariables, 0);
#endif
		return;
	}

	if( dwArraySize > MAX_LIGHTS_IN_30_MODEL)
	{
		dwArraySize = MAX_LIGHTS_IN_30_MODEL;
	}

	dword n = 0;
	for (; n < dwArraySize; n++)
	{
		ILight* light = lightsArray[n];

		const Vector4& col = light->GetRenderColor().v4;
		float FarRadius = light->GetRadius();


		stdLightVariables.pointLights.vPointLightPos[n] = light->GetPos();
		stdLightVariables.pointLights.vPointLightColor[n] = col;
		stdLightVariables.pointLights.vPointLightColorSpecular[n] = col;

		Vector & specular = stdLightVariables.pointLights.vPointLightColorSpecular[n].v;

		if ((specular | specular) > 1.0f)
		{
			specular.Normalize();
		}

		stdLightVariables.pointLights.vPointLightColorSpecular[n].v *= SPECULAR_STRENGHT;

		/*x: OneDivPointLightNearPow, y:PointLightNear, z:PointLightFar, w:PointLightAffectedShadow*/
		stdLightVariables.pointLights.vPointLightsParams[n] = Vector4(1.0f / (FarRadius * FarRadius), 0.0f, FarRadius, light->getAffectedByShadowK());
	}

	for (; n < MAX_LIGHTS_IN_30_MODEL; n++)
	{
		stdLightVariables.pointLights.vPointLightPos[n] = 0.0f;
		stdLightVariables.pointLights.vPointLightColor[n] = 0.0f;
		stdLightVariables.pointLights.vPointLightColorSpecular[n] = 0.0f;

		/*x: OneDivPointLightNearPow, y:PointLightNear, z:PointLightFar, w:PointLightAffectedShadow*/
		stdLightVariables.pointLights.vPointLightsParams[n] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	}


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	m_ShaderManager.getVarDatabase()->ApplyLightingVariables30(stdLightVariables, dwArraySize);
#else
	shader->ApplyLightingVariables30(stdLightVariables, dwArraySize);
#endif
}


void NGRender::DebugIntersectLights (const GMXOBB &boundBox, array<ILight*> & affectedLights)
{
	AssertCoreThread;

	const Plane* Frustum = GetFrustumFast();


	dword VisibleLightCount = 0;
	int LastUsedLightIndex = -1;


	affectedLights.DelAll();

	Color newColor;
	ILight* lightPtr = NULL;

	//Проверяем источники
	for (dword i=0; i < activeLights.Size(); i++)
	{
		lightPtr = activeLights[i];

		newColor = lightPtr->GetColor();
		if (!LightIsUsed(vCameraPosition_WorldSpace.v, lightPtr, boundBox, Frustum, newColor))
		{
			continue;
		}

		dword dwOldValue = lightPtr->debugGetValue();
		lightPtr->debugSetValue(dwOldValue+1);


		affectedLights.Add(lightPtr);
		lightPtr->SetRenderColor(newColor);
		LastUsedLightIndex = i;
		VisibleLightCount++;
	}


}

bool NGRender::DrawIndexedPrimitiveLighted(const ShaderLightingId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount, const GMXOBB &boundBox)
{
	AssertCoreThread;

#ifndef _XBOX
	#ifdef KEY_U_DISABLE_RENDER
		if (!bDisableHacks)
		{
			if (api->DebugKeyState('U'))
			{
				return true;
			}
		}
	#endif

		if (bUseMipFillColor)
		{
			return DrawIndexedPrimitiveSingleThread(id.off, Type, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);
		}
#endif


	const Plane* Frustum = GetFrustumFast();


	dword VisibleLightCount = 0;
	int LastUsedLightIndex = -1;

	
	passLights.DelAll();

	Color newColor;
	ILight* lightPtr = NULL;

	//Проверяем источники
	for (dword i=0; i < activeLights.Size(); i++)
	{
		lightPtr = activeLights[i];

		newColor = lightPtr->GetColor();
		if (!LightIsUsed(vCameraPosition_WorldSpace.v, lightPtr, boundBox, Frustum, newColor))
		{
			continue;
		}

		passLights.Add(lightPtr);
		lightPtr->SetRenderColor(newColor);
		LastUsedLightIndex = i;
		VisibleLightCount++;
	}


	if (passLights.Size() > MAX_LIGHTS_IN_30_MODEL)
	{
		//Слишком много источников на батч
		return DrawIndexedPrimitiveSingleThread(id.off, Type, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);
	}

	if (id.on == NULL)
	{
		return false;
	}


	stats.LightCountVisible = coremax (stats.LightCountVisible, VisibleLightCount);

	if (passLights.Size() > 0)
	{
		// directional и < MAX_LIGHTS_PER_PASS point
		SetLightVariables(id.on, &passLights[0], VisibleLightCount);

		dword dwID = pixBeginEvent(_FL_, "%d lights", VisibleLightCount);

		bool bRes = DrawIndexedPrimitiveSingleThread(id.on, Type, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);

		pixEndEvent(_FL_, dwID);

		passLights.DelAll();
		return bRes;
	}


	SetLightVariables(id.on, NULL, 0);
	return DrawIndexedPrimitiveSingleThread(id.on, Type, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);
}

bool NGRender::DrawPrimitiveLighted(const ShaderLightingId & id, RENDERPRIMITIVETYPE Type, dword StartVertex, dword PrimitiveCount, const GMXOBB & boundBox)
{
	AssertCoreThread;

#ifndef _XBOX
	#ifdef KEY_U_DISABLE_RENDER
		if (!bDisableHacks)
		{
			if (api->DebugKeyState('U'))
			{
				return true;
			}
		}
	#endif

		if (bUseMipFillColor)
		{
			return DrawPrimitive(id.off, Type, StartVertex, PrimitiveCount);
		}
#endif


	const Plane* Frustum = GetFrustumFast();


	dword VisibleLightCount = 0;
	int LastUsedLightIndex = -1;

	passLights.DelAll();

	Color newColor;
	ILight* lightPtr = NULL;

	//Проверяем источники
	for (dword i=0; i < activeLights.Size(); i++)
	{
		lightPtr = activeLights[i];

		newColor = lightPtr->GetColor();
		if (!LightIsUsed(vCameraPosition_WorldSpace.v, lightPtr, boundBox, Frustum, newColor))
		{
			continue;
		}

		passLights.Add(lightPtr);
		lightPtr->SetRenderColor(newColor);
		LastUsedLightIndex = i;
		VisibleLightCount++;
	}


	if (passLights.Size() > MAX_LIGHTS_IN_30_MODEL)
	{
		//Слишком много источников на батч
		return DrawPrimitive(id.off, Type, StartVertex, PrimitiveCount);
	}

	if (id.on == NULL)
	{
		return false;
	}


	stats.LightCountVisible = coremax (stats.LightCountVisible, VisibleLightCount);

	if (passLights.Size() > 0)
	{
		// directional и < MAX_LIGHTS_PER_PASS point
		SetLightVariables(id.on, &passLights[0], VisibleLightCount);

		bool bRes = DrawPrimitive(id.on, Type, StartVertex, PrimitiveCount);
		passLights.DelAll();
		return bRes;
	}


	SetLightVariables(id.on, NULL, 0);
	return DrawPrimitive(id.on, Type, StartVertex, PrimitiveCount);
}

bool NGRender::DrawLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return false;

	RS_LINE line[2];

	line[0].vPos = v1;
	line[0].dwColor = dwColor1;
	line[1].vPos = v2;
	line[1].dwColor = dwColor2;

	Matrix mWorldSave;
	if (!bWorldMatrix)
	{
		mWorldSave = GetWorld();
		SetWorld(mIdentity);
	}
	bool bSuccess = DrawLines(line, 1, pTechniqueName, NULL, 0);
	if (!bWorldMatrix)
		SetWorld(mWorldSave);
	return bSuccess;
}

bool NGRender::DrawBufferedLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix, const char* szTechnique)
{
	AssertCoreThread;


	RS_LINE * p1 = &aBufferedLines[aBufferedLines.Add()];
	RS_LINE * p2 = &aBufferedLines[aBufferedLines.Add()];

	if (bWorldMatrix)
	{
		p1->vPos = v1 * GetWorld();
		p2->vPos = v2 * GetWorld();
	}
	else
	{
		p1->vPos = v1;
		p2->vPos = v2;
	}

	p1->dwColor = dwColor1;
	p2->dwColor = dwColor2;

	if (aBufferedLines() >= 2048) FlushBufferedLines(bWorldMatrix, szTechnique);

	return true;
}

bool NGRender::FlushBufferedLines(bool bIdentityMatrix, const char* szTechnqiue)
{
	AssertCoreThread;


	if (aBufferedLines())
	{
		Matrix mWorldSave = GetWorld();
		if (bIdentityMatrix) SetWorld(mIdentity);
		DrawLines(&aBufferedLines[0], aBufferedLines() / 2, szTechnqiue);
		if (bIdentityMatrix) SetWorld(mWorldSave);
	}
	aBufferedLines.DelAll();
	return true;
}


bool NGRender::DrawLines(RS_LINE * pRSL, dword dwLinesNum, const char * pTechniqueName, void * pObsoletteMustBeNULL, dword dwStride)
{
	AssertCoreThread;

	ShaderId id;
	GetShaderId(pTechniqueName, id);


	if (IsRenderDisabled()) return false;
	if (!pRSL || !dwLinesNum) return false;
	if (!dwStride) dwStride = sizeof(RS_LINE);

	bool bSuccess = DrawPrimitiveUP(id, PT_LINELIST, dwLinesNum, pRSL, dwStride);

	return bSuccess;
}

bool NGRender::DrawSolidBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	Vector vScale = vMax - vMin;
	Vector vCenter = vMin + (vMax - vMin) / 2.0f;



	SHADER* shader = m_ShaderManager.FindShaderByName(pTechniqueName);
	if (!shader)
	{
		api->Trace("Can't draw box, shader '%s' not found", pTechniqueName);
		return false;
	}


	Matrix mSavedWorld = GetWorld();
	SetWorld(mMatrix);

	SHADERVAR* sv_vLightDir = (SHADERVAR*)GetTechniqueGlobalVariable("vLightDir", _FL_);
	if (sv_vLightDir)
	{
		Vector vLightDir = -Vector(GetView().m[0][2], GetView().m[1][2], GetView().m[2][2]);
		const Matrix& mWorldInv = mtxWorldInv;
		vLightDir = mWorldInv.MulNormal(vLightDir);
		vLightDir.Normalize();


		sv_vLightDir->SetVector(vLightDir);
		sv_vLightDir = NULL;
	}

	SHADERVAR* sv_vCenter = (SHADERVAR*)GetTechniqueGlobalVariable("vCenter", _FL_);
	if (sv_vCenter)
	{
		sv_vCenter->SetVector(vCenter);
		sv_vCenter = NULL;
	}

	SHADERVAR* sv_vScale = (SHADERVAR*)GetTechniqueGlobalVariable("vScale", _FL_);
	if (sv_vScale)
	{
		sv_vScale->SetVector(vScale);
		sv_vScale = NULL;
	}

	SHADERVAR* sv_vColor = (SHADERVAR*)GetTechniqueGlobalVariable("vColor", _FL_);
	if (sv_vColor)
	{
		sv_vColor->SetVector4(Color(dwColor).v4);
		sv_vColor = NULL;
	}


	SetStreamSource(0, pVSolidBox);
	SetIndices(pISolidBox, 0);

	ShaderId id;
	GetShaderId(pTechniqueName, id);


	DrawIndexedPrimitive(id, PT_TRIANGLELIST, 0, pVSolidBox->GetLength() / sizeof(SolidBoxVertex), 0, pISolidBox->GetLength() / (sizeof(short) * 3));

	SetWorld(mSavedWorld);

	return true;
}

bool NGRender::DrawBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	RS_LINE		RSL[12 * 2];

	Vector v0 = vMin;
	Vector v1 = Vector(vMax.x, vMin.y, vMin.z);
	Vector v2 = Vector(vMax.x, vMin.y, vMax.z);
	Vector v3 = Vector(vMin.x, vMin.y, vMax.z);
	Vector v4 = Vector(vMin.x, vMax.y, vMin.z);
	Vector v5 = Vector(vMax.x, vMax.y, vMin.z);
	Vector v6 = Vector(vMax.x, vMax.y, vMax.z);
	Vector v7 = Vector(vMin.x, vMax.y, vMax.z);

	long i = 0;

	// down
	RSL[i++].vPos = v0;		RSL[i++].vPos = v1; 
	RSL[i++].vPos = v1;		RSL[i++].vPos = v2; 
	RSL[i++].vPos = v2;		RSL[i++].vPos = v3; 
	RSL[i++].vPos = v3;		RSL[i++].vPos = v0; 

	// up
	RSL[i++].vPos = v4;		RSL[i++].vPos = v5; 
	RSL[i++].vPos = v5;		RSL[i++].vPos = v6; 
	RSL[i++].vPos = v6;		RSL[i++].vPos = v7; 
	RSL[i++].vPos = v7;		RSL[i++].vPos = v4; 

	// edges
	RSL[i++].vPos = v0;		RSL[i++].vPos = v4; 
	RSL[i++].vPos = v1;		RSL[i++].vPos = v5; 
	RSL[i++].vPos = v2;		RSL[i++].vPos = v6; 
	RSL[i++].vPos = v3;		RSL[i++].vPos = v7; 

	for (i=0; i<12 * 2; i++) RSL[i].dwColor = dwColor;

	Matrix mWorldSave = GetWorld();
	SetWorld(mMatrix);
	bool bSuccess = DrawLines(RSL, 12, pTechniqueName, NULL);
	SetWorld(mWorldSave);

	return bSuccess;
}

bool NGRender::DrawRects(IBaseTexture* pTexture, RS_RECT * pRects, dword dwRectsNum, const char * pTechniqueName, void * pObsoletteMustBeNULL, dword dwStride, dword dwSubTexX, dword dwSubTexY)
{
	AssertCoreThread;

	if (!pRects || dwRectsNum == 0) return false;
	if (IsRenderDisabled()) return false;



	SHADER* shader = m_ShaderManager.FindShaderByName(pTechniqueName);
	if (!shader)
	{
		api->Trace("Can't draw rects, shader '%s' not found", pTechniqueName);
		return false;
	}

	IVariable* var = GetTechniqueGlobalVariable("RectTexture", _FL_);
	if (var)
	{
		if (pTexture)
			var->SetTexture(pTexture);
		else
			var->ResetTexture();
		var = NULL;
	}




	Matrix mView = GetView();
	bool bUseSubTextures = (dwSubTexX > 1 || dwSubTexY > 1);

	for (dword i=0; i<(dwRectsNum + MAX_RECTS_NUM - 1) / MAX_RECTS_NUM; i++)
	{
		dword dwNum = ((i + 1) * MAX_RECTS_NUM > dwRectsNum) ? dwNum = dwRectsNum - (dwRectsNum / MAX_RECTS_NUM) * MAX_RECTS_NUM : MAX_RECTS_NUM;

#ifndef _XBOX
		DWORD dwFlags = D3DLOCK_DISCARD;
#else
		DWORD dwFlags = 0;
#endif

		RECT_VERTEX * pRectsVerts = (RECT_VERTEX*)pRectsVBuffer->Lock(0, 0, dwFlags);

		for (dword j=0; j<dwNum; j++)
		{
			dword dwIdx = i * MAX_RECTS_NUM + j;

			RECT_VERTEX	* pV = &pRectsVerts[j * 4];
			RS_RECT * pR = (RS_RECT*)&(((char*)pRects)[dwStride * dwIdx]);

			float fCos = cosf(pR->fAngle);
			float fSin = sinf(pR->fAngle);

			float fSizeX = pR->fSizeX;
			float fSizeY = pR->fSizeY;
	
			Vector vPos = mView * pR->vPos;

			float dx, dy, sx, sy;
			if (bUseSubTextures)
			{
				dx = 1.0f / float(dwSubTexX);
				dy = 1.0f / float(dwSubTexY);
				sx = dx * (pR->dwSubTexture - dwSubTexX * (pR->dwSubTexture / dwSubTexX));
				sy = dy * (pR->dwSubTexture / dwSubTexY);
			}
			else
			{
				sx = sy = 0.0f;
				dx = dy = 1.0f;
			}

			pV[0].vPos = Vector(-fSizeX, -fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
			pV[0].dwColor = pR->dwColor;
			pV[0].tu = 0.0f * dx + sx;	
			pV[0].tv = 1.0f * dy + sy;

			pV[1].vPos = Vector(-fSizeX, fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
			pV[1].dwColor = pR->dwColor;
			pV[1].tu = 0.0f * dx + sx;	
			pV[1].tv = 0.0f * dy + sy;

			pV[2].vPos = Vector(fSizeX, fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
			pV[2].dwColor = pR->dwColor;
			pV[2].tu = 1.0f * dx + sx;	
			pV[2].tv = 0.0f * dy + sy;

			pV[3].vPos = Vector(fSizeX, -fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
			pV[3].dwColor = pR->dwColor;
			pV[3].tu = 1.0f * dx + sx;	
			pV[3].tv = 1.0f * dy + sy;
		}
		pRectsVBuffer->Unlock();

		SetStreamSource(0, pRectsVBuffer);
		SetIndices(pRectsIBuffer, 0);

		Matrix mOldView = GetView();
		Matrix mOldWorld = GetWorld();

		SetWorld(Matrix());
		SetView(Matrix());



		ShaderId id;
		GetShaderId(pTechniqueName, id);


		DrawIndexedPrimitive(id, PT_TRIANGLELIST, 0, dwNum * 4, 0, dwNum * 2);

		SetWorld(mOldWorld);
		SetView(mOldView);

	}

	return true;
}

bool NGRender::DrawSprites(IBaseTexture* pTexture, RS_SPRITE * pSprites, dword dwSpritesNum, const char * pTechniqueName, void * pObsoletteMustBeNULL, dword dwStride)
{
	AssertCoreThread;

	if (!pSprites || dwSpritesNum == 0)
	{
		return false;
	}

	if (IsRenderDisabled())
	{
		return false;
	}


	SHADER* shader = m_ShaderManager.FindShaderByName(pTechniqueName);
	if (!shader)
	{
		api->Trace("Can't draw sprites, shader '%s' not found", pTechniqueName);
		return false;
	}

	IVariable* var = GetTechniqueGlobalVariable("Texture", _FL_);
	if (var)
	{
		if (pTexture == NULL)
		{
			var->ResetTexture();
		} else
		{
			var->SetTexture(pTexture);
		}
		var = NULL;
	}


	for (dword i=0; i<(dwSpritesNum + MAX_RECTS_NUM - 1) / MAX_RECTS_NUM; i++)
	{
		dword dwNum = ((i + 1) * MAX_RECTS_NUM > dwSpritesNum) ? dwNum = dwSpritesNum - (dwSpritesNum / MAX_RECTS_NUM) * MAX_RECTS_NUM : MAX_RECTS_NUM;

#ifndef _XBOX
		DWORD dwFlags = LOCK_DISCARD;
#else
		DWORD dwFlags = 0;
#endif

		RS_SPRITE * pSpritesVerts = (RS_SPRITE*)pSpritesVBuffer->Lock(0, 0, dwFlags);

		for (dword j=0; j<dwNum; j++)
		{
			dword dwIdx = i * MAX_RECTS_NUM + j;

			RS_SPRITE * pV = &pSpritesVerts[j * 4];
			RS_SPRITE * pS = (RS_SPRITE*)&(((char*)pSprites)[dwStride * dwIdx]);

			pV[0] = pS[0];	pV[1] = pS[1];	pV[2] = pS[2];	pV[3] = pS[3];
		}

		pSpritesVBuffer->Unlock();

		SetStreamSource(0, pSpritesVBuffer);
		SetIndices(pRectsIBuffer, 0);

		ShaderId id;
		GetShaderId(pTechniqueName, id);

		DrawIndexedPrimitive(id, PT_TRIANGLELIST, 0, dwNum * 4, 0, dwNum * 2);

		SetStreamSource(0, null);
	}

	return true;
}

bool NGRender::DrawNormals(IVBuffer * pVBuffer, dword dwStartVertex, dword dwNumVertices, dword dwColor1, dword dwColor2, float fScale, dword dwStride, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (!pVBuffer) return false;

	if (!dwStride) dwStride = pVBuffer->GetStride();
	if (!dwStride) return false;

	char * pBuffer = (char*)pVBuffer->Lock();
	if (!pBuffer) return false;

	RS_LINE * pLines = NEW RS_LINE[dwNumVertices * 2];
	for (dword i=0; i<dwNumVertices; i++)
	{
		Vector * pVector = (Vector*)(pBuffer + dwStartVertex * dwStride);
		Vector * pNormal = (Vector*)(pBuffer + dwStartVertex * dwStride + sizeof(Vector));

		pLines[i * 2 + 0].vPos = *pVector;
		pLines[i * 2 + 0].dwColor = dwColor1;

		pLines[i * 2 + 1].vPos = (*pVector) + fScale * (*pNormal);
		pLines[i * 2 + 1].dwColor = dwColor2;

		pBuffer += dwStride;
	}
	pVBuffer->Unlock();

	bool bSuccess = DrawLines(pLines, dwNumVertices, pTechniqueName, NULL);

	DELETE(pLines);

	return bSuccess;
}


bool NGRender::DrawSphere(const Vector & vPos, float fRadius, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return false;

	Matrix mPos;
	mPos.BuildPosition(vPos);
	mPos.m[0][0] = fRadius;
	mPos.m[1][1] = fRadius;
	mPos.m[2][2] = fRadius;

	return DrawSphere(mPos, dwColor, pTechniqueName, NULL);
}

bool NGRender::DrawSphere(const Matrix & mPos, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return false;

	Vector vRadius = mPos.GetScale();
	float fRadius = coremax(vRadius.x, vRadius.y);
	fRadius = coremax(fRadius, vRadius.z);

	const Plane * frustum = GetFrustum();
	for(long i = 0; i < 4; i++)
	{
		if(frustum[i].Dist(mPos.pos) < -fRadius) return true;
	}

	Matrix mWorldSave = GetWorld();
	SetWorld(mPos);

	SHADER* sphereShader = m_ShaderManager.FindShaderByName(pTechniqueName);
	if (!sphereShader)
	{
		api->Trace("Can't draw sphere, shader '%s' not found", pTechniqueName);
		return false;
	}

	IVariable* var = GetTechniqueGlobalVariable("SphereColor", _FL_);
	if (var)
	{
		var->SetVector4(Color(dwColor).v4);
		var = NULL;
	}


	ShaderId id;
	GetShaderId(pTechniqueName, id);


	SetStreamSource(0, pVSphereBuffer);
	DrawPrimitive(id, PT_TRIANGLELIST, 0, dwNumSphereTrgs);

	SetWorld(mWorldSave);

	return true;
}

void NGRender::MakeDrawVector(RS_LINE * pLines, dword dwNumSubLines, const Matrix & mMatrix, Vector vUp, Vector v1, Vector v2, float fScale, dword dwColor)
{
	AssertCoreThread;


	dword i;

	for (i=0; i<dwNumSubLines * 2 + 2; i++) pLines[i].dwColor = dwColor;
	pLines[0].vPos = v1;
	pLines[1].vPos = v1 + (fScale * v2);

	float fRadius = 0.03f * fScale;
	float fDist = 0.85f * fScale * sqrtf(~v2);

	for (i=0; i<dwNumSubLines; i++)
	{
		float fAng = PIm2 * float(i) / float(dwNumSubLines);
		
		float x = fRadius * sinf(fAng);
		float z = fRadius * cosf(fAng);

		Vector vRes;

		if (fabsf(vUp.x) < 1e-5f) vRes = Vector(fDist, x, z);
		if (fabsf(vUp.y) < 1e-5f) vRes = Vector(x, fDist, z);
		if (fabsf(vUp.z) < 1e-5f) vRes = Vector(x, z, fDist);
		vRes = vRes * mMatrix;
		pLines[2 + i * 2 + 0].vPos = vRes;
		pLines[2 + i * 2 + 1].vPos = pLines[1].vPos;
	}
}

bool NGRender::DrawMatrix(const Matrix & mMatrix, float fScale, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return false;

	RS_LINE lines[3 * 51 * 2];

	MakeDrawVector(&lines[0], 50, mMatrix, Vector(0.0f, 1.0f, 1.0f), mMatrix.pos, mMatrix.vx, fScale, 0xFF00FF00);
	MakeDrawVector(&lines[102], 50, mMatrix, Vector(1.0f, 0.0f, 1.0f), mMatrix.pos, mMatrix.vy, fScale, 0xFFFF0000);
	MakeDrawVector(&lines[204], 50, mMatrix, Vector(1.0f, 1.0f, 0.0f), mMatrix.pos, mMatrix.vz, fScale, 0xFF0000FF);

	Matrix mWorldSave = GetWorld();	SetWorld(mIdentity);
	bool bSuccess = DrawLines(lines, 3 * 51, pTechniqueName, NULL);
	SetWorld(mWorldSave);
	return bSuccess;
}

bool NGRender::DrawVector(const Vector & v1, const Vector & v2, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return false;

	RS_LINE lines[51 * 2];
	Matrix	mView;

	float fScale = sqrtf(~(v2 - v1));
	if (!mView.BuildView(v1, v2, Vector(0.0f, 1.0f, 0.0f))) 
		if (!mView.BuildView(v1, v2, Vector(1.0f, 0.0f, 0.0f))) return false;
	mView.Inverse();

	MakeDrawVector(&lines[0], 50, mView, Vector(1.0f, 1.0f, 0.0f), mView.pos, mView.vz, fScale, dwColor);

	Matrix mWorldSave = GetWorld();	SetWorld(mIdentity);
	bool bSuccess = DrawLines(lines, 51, pTechniqueName, NULL);
	SetWorld(mWorldSave);
	return bSuccess;
}



bool NGRender::DrawPolygon(const Vector * pVectors, dword dwNumPoints, const Color & color, const Matrix& matrix, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return false;

	if (!pVectors || dwNumPoints < 3) return false;

	aPolygons.Empty();
	for (dword i=0; i<dwNumPoints; i++)
	{
		POLYGON_VERTEX * pV = &aPolygons[aPolygons.Add()];
		
		pV->dwColor = color;
		pV->vPos = pVectors[i];
	}

	SetWorld(matrix);

	ShaderId id;
	GetShaderId(pTechniqueName, id);


	return DrawPrimitiveUP(id, PT_TRIANGLEFAN, dwNumPoints - 2, &aPolygons[0], sizeof(POLYGON_VERTEX));
}





void NGRender::DrawXZCircle (const Vector& center, float fRadius, dword dwColor, const char* szTechnique)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return;

	Matrix prevWorld = GetWorld();
	SetWorld(mIdentity);



	float fDelta = 0.2f;
	RS_LINE line[2];
	Vector vStart;
	Vector vEnd;
	Vector vStartPoint;

	vStartPoint = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius);
	vStartPoint += center;
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(cosf(Angle)*fRadius, 0.0f, sinf(Angle)*fRadius);
		vPoint += center;

		vStart = vStartPoint;
		vEnd = vPoint;

		line[0].vPos = vStart;
		line[0].dwColor = dwColor;
		line[1].vPos = vEnd;
		line[1].dwColor = dwColor;
		DrawLines(line, 1, szTechnique);
		//pRS->DrawVector(vStart, vEnd, dwColor);
		vStartPoint = vPoint;
	}

	vStart = vStartPoint;
	vEnd = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius);
	vEnd += center;

	line[0].vPos = vStart;
	line[0].dwColor = dwColor;
	line[1].vPos = vEnd;
	line[1].dwColor = dwColor;
	DrawLines(line, 1, szTechnique);

	SetWorld(prevWorld);
}






void NGRender::DrawSphereGizmo (const Vector& pos, float fRadius, dword dwColor1, dword dwColor2, const char* szTechnique)
{
	AssertCoreThread;


	if (IsRenderDisabled()) return;


	//Matrix mTransform;
	//GetMatrix(mTransform);


	Matrix mCurView = GetView();
	mCurView.Inverse();
	Vector vCamDir = mCurView.vz;


	Matrix wrld = Matrix();

	float fDelta = 0.2f;


	Vector vStart;
	Vector vEnd;

	Vector mFrom = pos;


	Vector vStartPoint = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(sinf(Angle)*fRadius, cosf(Angle)*fRadius, 0.0f);

		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;

		vStart += mFrom;
		vEnd += mFrom;

		if (NeedDrawRotateGizmoLine(pos, vStart, vEnd, vCamDir))
		{
			DrawBufferedLine(vStart, dwColor1, vEnd, dwColor1, false, szTechnique);
		}


		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f)*wrld;
	vStart += mFrom;
	vEnd += mFrom;
	if (NeedDrawRotateGizmoLine(pos, vStart, vEnd, vCamDir))
	{
		DrawBufferedLine(vStart, dwColor1, vEnd, dwColor1, false, szTechnique);
	}




	vStartPoint = Vector(0.0f, cosf(0)*fRadius, sinf(0)*fRadius);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(0.0f, cosf(Angle)*fRadius, sinf(Angle)*fRadius);
		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;
		vStart += mFrom;
		vEnd += mFrom;

		if (NeedDrawRotateGizmoLine(pos, vStart, vEnd, vCamDir))
		{
			DrawBufferedLine(vStart, dwColor1, vEnd, dwColor1, false, szTechnique);
		}

		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(0.0f, cosf(0)*fRadius, sinf(0)*fRadius)*wrld;
	vStart += mFrom;
	vEnd += mFrom;
	if (NeedDrawRotateGizmoLine(pos, vStart, vEnd, vCamDir))
	{
		DrawBufferedLine(vStart, dwColor1, vEnd, dwColor1, false, szTechnique);
	}




	vStartPoint = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(cosf(Angle)*fRadius, 0.0f, sinf(Angle)*fRadius);

		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;
		vStart += mFrom;
		vEnd += mFrom;
		if (NeedDrawRotateGizmoLine(pos, vStart, vEnd, vCamDir))
		{
			DrawBufferedLine(vStart, dwColor1, vEnd, dwColor1, false, szTechnique);
		}

		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius)*wrld;
	vStart += mFrom;
	vEnd += mFrom;

	if (NeedDrawRotateGizmoLine(pos, vStart, vEnd, vCamDir))
	{
		DrawBufferedLine(vStart, dwColor1, vEnd, dwColor1, false, szTechnique);
	}





	Matrix mView = GetView();
	mView.pos = 0.0f;
	mView.Inverse();
	vStartPoint = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(sinf(Angle)*fRadius, cosf(Angle)*fRadius, 0.0f);

		vStart = vStartPoint*mView;
		vEnd = vPoint*mView;
		vStart += mFrom;
		vEnd += mFrom;

		DrawBufferedLine(vStart, dwColor2, vEnd, dwColor2, false, szTechnique);
		vStartPoint = vPoint;
	}

	vStart = vStartPoint*mView;
	vEnd = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f)*mView;
	vStart += mFrom;
	vEnd += mFrom;
	DrawBufferedLine(vStart, dwColor2, vEnd, dwColor2, false, szTechnique);

	FlushBufferedLines(true, szTechnique);

}



bool NGRender::DrawCapsule(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
	AssertCoreThread;


	if (!capsuleRender) return false;

	capsuleRender->Draw( fRadius,  fHeight, dwColor, WorldMat, pTechniqueName);

	return true;
}