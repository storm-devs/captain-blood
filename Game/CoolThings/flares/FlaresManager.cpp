
#include "Flares.h"
#include "FlaresManager.h"

#define ToSignRange(x) (x  - 0.5f) * 2.0f

//Utill
void DrawSprite(float x, float y, float sx, float sy,float u, float v,dword color,IBaseTexture* tex,IRender* renderer)
{	
	RS_SPRITE spr[4];		

	spr[0].vPos = Vector (  x ,  y, 0.0f);
	spr[1].vPos = Vector (x+sx,  y, 0.0f);
	spr[2].vPos = Vector (x+sx, y-sy , 0.0f);
	spr[3].vPos = Vector (  x , y-sy , 0.0f);

	spr[0].tv = 0.0f;
	spr[0].tu = 0.0f;

	spr[1].tv = 0.0f;
	spr[1].tu = 1.0f;

	spr[2].tv = 1.0f;
	spr[2].tu = 1.0f;

	spr[3].tv = 1.0f;
	spr[3].tu = 0.0f;	

	for (int j=0;j<4;j++)
	{
		spr[j].dwColor = color;		
	}

	renderer->DrawSprites(tex,spr, 1);	
}

void AddSprite(FlaresManager::FLARE2D_VERTEX* pVert, int index, const Vector & pos, float x, float y, float sx, float sy,float u, float v, float fDet,float u2, float v2)
{		
	pVert[0 + index*4].vPos = Vector (  x ,  y, 0.0f);
	pVert[1 + index*4].vPos = Vector (x+sx,  y, 0.0f);
	pVert[2 + index*4].vPos = Vector (x+sx, y-sy , 0.0f);
	pVert[3 + index*4].vPos = Vector (  x , y-sy , 0.0f);
	
	for (int j=0;j<4;j++)
	{		
		pVert[j + index*4].wpos = pos;

		pVert[j + index*4].tu = u;
		pVert[j + index*4].tv = v;		
		pVert[j + index*4].tw = fDet;

		pVert[j + index*4].tu2 = u2;
		pVert[j + index*4].tv2 = v2;
	}	
}

bool FlaresManager::InFrustrum(const Plane * frustum,Vector pos,float radius)
{			
	for(long p = 0; p < 5; p++)
		if(((pos | frustum[p].n) - frustum[p].d) < -radius) return false;

	return true;
}

FlaresManager::FlaresManager() : flares(_FL_, 128)
{
	texture = null;	
	
	pFlareVBuffer = null;
	pFlareIBuffer = null;
	pFlare2DVBuffer = null;

	pMask = NULL;
	pZMask = NULL;

	pVarTex = null;
	pVarMask = null;
	pFlareColor = null;
	pFlareVBuffer = null;
	pFlareTexSzU = null;
	pFlareTexSzV = null;	

	pPowMask = null;
	pPowSecMask = null;

	first_time = true;
}

FlaresManager::~FlaresManager()
{		
	Release();	
}

void FlaresManager::Activate(Flares* flare,bool activate)
{
	dword index = flares.Find(flare);

	if (activate)
	{
		if(index == INVALID_ARRAY_INDEX)
		{
			flares.Add(flare);						
		}		

		int flare_index=0;

		for (int j=0;j<(int)flares.Size();j++)
		{
			for (int i=0;i<(int)flares[j]->flares.Size();i++)			
			{
				int k = (int)(flare_index/32.0f);

				flares[j]->flares[i].u = (flare_index - k*32.0f)/32.0f;
				flares[j]->flares[i].v = k/32.0f;
				
				flare_index++;
			}	
		}		
	}
	else
	{		
		if(index != INVALID_ARRAY_INDEX)
		{					
			flares.Del(flare);			
		}		
	}

	if ((int)flares.Size()>0)
	{
		SetUpdate(&FlaresManager::Flicker, ML_EXECUTE1);
		SetUpdate(&FlaresManager::Draw, ML_PARTICLES4);
	}
	else
	{						
		DelUpdate(&FlaresManager::Flicker);
		DelUpdate(&FlaresManager::Draw);
	}	
}

//Обновить параметры
bool FlaresManager::Create(MOPReader & reader)
{
	Release();

	scr_width = 1.0f/Render().GetFullScreenViewPort_3D().Width;
	scr_height = 1.0f/Render().GetFullScreenViewPort_3D().Height;

	pVarTex = Render().GetTechniqueGlobalVariable("FlareTexture", _FL_);
	pVarMask = Render().GetTechniqueGlobalVariable("FlareMaskTex", _FL_);
	pVarPowSecMask = Render().GetTechniqueGlobalVariable("FlareSecMaskTex", _FL_);

	pFlareColor =  Render().GetTechniqueGlobalVariable("flare_color", _FL_);
	pFlareTexSzU = Render().GetTechniqueGlobalVariable("flare_u_texel_size", _FL_);
	pFlareTexSzV = Render().GetTechniqueGlobalVariable("flare_v_texel_size", _FL_);
		

	Render().GetShaderId("FlareNoZ", flareNoZ_id);
	Render().GetShaderId("FlareMask", flareMask_id);
	Render().GetShaderId("Flare9", flare9_id);

#ifdef _XBOX
	Render().GetShaderId("XBFast_Flare2D9Mask", XBFastFlare2D9Mask_id);
#endif

	Render().GetShaderId("Flare2D9Mask", flare2D9Mask_id);

	pPowMask = Render().CreateRenderTarget(32,32,_FL_,FMT_A8R8G8B8);
	pPowSecMask = Render().CreateRenderTarget(32,32,_FL_,FMT_A8R8G8B8);

	bool xboxHack = false;
#ifdef _XBOX
	xboxHack = pPowMask->FitEDRAMWithMainRT();
#endif

	if (!xboxHack)
		pMask = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D, TRC_FIXED_RGBA_8, _FL_);

	pZMask = Render().CreateTempDepthStencil(TRS_SCREEN_FULL_3D,_FL_);

	texture = Render().CreateTexture(_FL_, "weather\\sun\\sunglow.txx");

	int max_rects = 1024;

	pFlareVBuffer = Render().CreateVertexBuffer( max_rects * sizeof(FLARE_VERTEX) * 4, sizeof(FLARE_VERTEX), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
	pFlare2DVBuffer = Render().CreateVertexBuffer( max_rects * sizeof(FLARE2D_VERTEX) * 4, sizeof(FLARE2D_VERTEX), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);

	pFlareIBuffer = Render().CreateIndexBuffer(max_rects * 6 * sizeof(WORD), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC);
			
	WORD * pTrgs = (WORD*)pFlareIBuffer->Lock(); 
	
	if (pTrgs)
	{
		for (dword i = 0; i < (dword)max_rects; i++)
		{
			pTrgs[i * 6 + 0] = WORD(i * 4 + 0);
			pTrgs[i * 6 + 1] = WORD(i * 4 + 1);
			pTrgs[i * 6 + 2] = WORD(i * 4 + 2);
			pTrgs[i * 6 + 3] = WORD(i * 4 + 0);
			pTrgs[i * 6 + 4] = WORD(i * 4 + 2);
			pTrgs[i * 6 + 5] = WORD(i * 4 + 3);
		}

		pFlareIBuffer->Unlock(); 
	}

	return true;
}

void _cdecl FlaresManager::Flicker(float dltTime, long level)
{
	for (int i=0;i<(int)flares.Size();i++)
	{		
		for (int j=0;j<(int)flares[i]->flares.Size();j++)
		{
			Flares::Flare* flare = &flares[i]->flares[j];

			if (!flare->isFlicker) continue;

			float intenc = 0.0f;
			float kAmp = 1.0f;
	
			for(long i = 0; i < ARRSIZE(flare->osc); i++)
			{
				Flares::Oscilator & o = flare->osc[i];
				o.k += o.step*dltTime;
			
				if(o.k >= 1.0f)
				{
					if(o.k < 2.0f)
					{
						o.k -= 1.0f;
						o.oldv = o.newv;
						o.newv = RRnd(-o.amp, o.amp);
					}
					else
					{
						o.k = 0.0f;
						o.oldv = RRnd(-o.amp, o.amp);
						o.newv = RRnd(-o.amp, o.amp);
					}
				}
			
				float ins = o.oldv + (o.newv - o.oldv)*o.k;
				intenc += ins*kAmp;
				kAmp -= ins*o.kamp;
				if(kAmp < 0.0f) break;
			}
	
			intenc = 1.0f + Clampf(intenc, -1.0f, 1.0f);		
			flare->intenc = 1.0f + (intenc - 1.0f)*flare->flicker;
		}
	}
}

void FlaresManager::CalcFlare(Flares::Flare* flare,float dltTime, Matrix& flare_mat)
{			
	float dist = (Render().GetView().GetCamPos() - flare->pos * flare_mat).GetLength();
	
	if (dist > flare->range)
	{
		flare->dist = 0.0f;	
	}
	else
	if (dist > flare->range * 0.75f)
	{
		flare->dist = (dist - flare->range) * 4.0f / flare->range;
	}
	else
	{
		flare->dist = 1.0f;
	}
	
	if (flare->dist>0.0f)
	{	
		flare->intencVis += dltTime*7.0f;
		if(flare->intencVis > 1.0f) flare->intencVis = 1.0f;
	}
	else
	{
		flare->intencVis -= dltTime*7.0f;
		if (flare->intencVis < 0.0f) flare->intencVis = 0.0f;
	}	
}

void FlaresManager::DrawFlares(FLARE_VERTEX * pFlareVerts, int poly_count, float fDelta)
{
	if (!pFlareVerts) return;

	bool xboxHack = false;
#ifdef _XBOX
	xboxHack = pPowMask->FitEDRAMWithMainRT();
#endif

	pFlareVBuffer->Unlock();	

	Matrix mOldView = Render().GetView();
	Matrix mOldWorld = Render().GetWorld();

	if (poly_count>0)
	{
		dword dwId = Render().pixBeginEvent(_FL_, "FlaresManager");

		RENDERVIEWPORT vp;				

		vp = Render().GetViewport();			
		
		#ifndef _XBOX
			Render().EndScene(null);
			Render().ResolveDepth(pZMask);			
		#else			
			Render().ResolveDepth(pZMask);
			if (!xboxHack)
				Render().EndScene(null);			
		#endif			
		
		pFlareColor->SetVector4(0.0f);

		Render().PushRenderTarget();
		Render().PushViewport();

		if (!xboxHack)
		{
			Render().SetRenderTarget(RTO_RESTORE_CONTEXT, pMask, pZMask);
			Render().BeginScene();

			Render().SetViewport(vp);
		
			Render().SetStreamSource(0, pFlareVBuffer);
			Render().SetIndices(pFlareIBuffer, 0);

			Render().SetWorld(Matrix());
			Render().SetView(Matrix());
			
			Render().DrawIndexedPrimitive(flareNoZ_id, PT_TRIANGLELIST, 0, poly_count * 4, 0, poly_count * 2);

			pFlareColor->SetVector4(1.0f);
			Render().DrawIndexedPrimitive(flareMask_id, PT_TRIANGLELIST, 0, poly_count * 4, 0, poly_count * 2);		

			// рисуется 32x32 текстурка
			Render().EndScene(null, false, true);
		}
		else
		{
			// заканчиваем отрисовку без всяких resolve
			Render().EndScene(null, true, true);
		}

		if (first_time)
		{
			Render().SetRenderTarget(RTO_RESTORE_CONTEXT, pPowSecMask, NULL);
			Render().Clear(0, null, CLEAR_TARGET, 0xFF000000, 1.0f, 0x0);				
		}

		Render().SetRenderTarget(RTO_RESTORE_CONTEXT, pPowMask, NULL);

		if (first_time)
		{
			Render().Clear(0, null, CLEAR_TARGET, 0xFF000000, 1.0f, 0x0);
			first_time = false;
		}

		Render().BeginScene();
	
		RENDERVIEWPORT vp2;

		vp2.X = 0;
		vp2.Y = 0;

		vp2.Width = 32;
		vp2.Height = 32;

		vp2.MinZ = 0.0f;
		vp2.MaxZ = 1.0f;
					
		Render().SetViewport(vp2);			

		FLARE2D_VERTEX* pVerts = (FLARE2D_VERTEX*)pFlare2DVBuffer->Lock(0, 0, LOCK_DISCARD);

		int index = 0;

		for (int i=0; i<(int)flares.Size(); i++)
		{
			bool noSwing = flares[i]->noSwing;

			for (int j=0;j<(int)flares[i]->flares.Size();j++)
			{					
				Flares::Flare & flare2 = flares[i]->flares[j];
				//-----------------------
				Matrix flare_mat;
				if (flare2.scene && flare2.connectedLocator.isValid())
				{
					//flare_mat = 
					flare2.scene->GetNodeWorldTransform(flare2.connectedLocator, flare_mat);//->GetWorldTransform();
				} else
				{
					if (flares[i]->flares[j].connectedObject.Validate())
					{
						flares[i]->flares[j].connectedObject.Ptr()->GetMatrix(flare_mat);
					}
				}
				//-----------------------

				Vector vFalrePos = flares[i]->flares[j].pos * flare_mat;

				if (noSwing) 
					if (!InFrustrum(frustumNoSwing, vFalrePos, 1.0f)) continue;
				else
					if (!InFrustrum(frustum, vFalrePos, 1.0f)) continue;

				Vector dir = 0.0f;
				
				if (noSwing)
					dir = vFalrePos - mInvViewNoSwing.pos;
				else
					dir = vFalrePos - mInvView.pos;
				
				dir.Normalize();
				
				Vector vFlarePos = vFalrePos + flares[i]->zOffset * dir;

				Vector4 vec = Vector4(vFlarePos.x, vFlarePos.y, vFlarePos.z, 1.0f);
				
				if (noSwing)
					vec = vec * mrNoSwing;					
				else
					vec = vec * mr;

				vec.x /= vec.w;
				vec.y /= vec.w;

				float u = (vp.X + vp.Width * (0.5f+vec.x*0.5f)) * scr_width;
				float v = (vp.Y + vp.Height * (0.5f-vec.y*0.5f)) * scr_height;

				AddSprite(pVerts, index, flares[i]->flares[j].vFlarePos,
						  -1.0f + flares[i]->flares[j].u * 2.0f, 1 - flares[i]->flares[j].v * 2.0f,
						  2.0f * 1.0f / 32.0f, 2.0f * 1.0f / 32.0f, u, v, flares[i]->fade_speed * fDelta, flares[i]->flares[j].u + 0.5f/32.0f, flares[i]->flares[j].v + 0.5f/32.0f);

				index++;
			}
		}


		pFlare2DVBuffer->Unlock();

		Render().SetStreamSource(0, pFlare2DVBuffer);
		Render().SetIndices(pFlareIBuffer, 0);
#ifdef _XBOX
		pVarMask->SetTexture((xboxHack) ? pZMask->AsTexture() : pMask->AsTexture());
#else
		pVarMask->SetTexture(pMask->AsTexture());
#endif
		pVarPowSecMask->SetTexture(pPowSecMask->AsTexture());

		ShaderId & shader = (xboxHack) ? XBFastFlare2D9Mask_id : flare2D9Mask_id;

		Render().DrawIndexedPrimitive(shader, PT_TRIANGLELIST, 0, index * 4, 0, index * 2);			

		Render().EndScene(null);

		Render().PopRenderTarget((xboxHack) ? RTO_DONTOCH_CONTEXT : RTO_RESTORE_CONTEXT);
		Render().PopViewport();			

		Render().BeginScene();

		pVarTex->SetTexture(texture);
		pVarMask->SetTexture(pPowMask->AsTexture());

		Render().SetStreamSource(0, pFlareVBuffer);
		Render().SetIndices(pFlareIBuffer, 0);

		pFlareTexSzU->SetFloat(scr_width * 2.0f);
		pFlareTexSzV->SetFloat(scr_height * 2.0f);			
		
		Render().SetWorld(Matrix());
		Render().SetView(Matrix());

		Render().DrawIndexedPrimitive(flare9_id, PT_TRIANGLELIST, 0, poly_count * 4, 0, poly_count * 2);

		IRenderTarget* tmp = pPowMask;
		pPowMask = pPowSecMask;	
		pPowSecMask = tmp;

		Render().pixEndEvent(_FL_, dwId);
	}

	if (api->DebugKeyState('K'))
	{
		pVarMask->SetTexture(pPowMask->AsTexture());
		DrawSprite( 0.5f, -0.5f, 0.5f,0.5f, -1, -1, 0xffffffff,pPowMask->AsTexture(),&Render());
	}

	Render().SetWorld(mOldWorld);
	Render().SetView(mOldView);

	pFlareVerts = NULL;
}

void FlaresManager::ConnectLocator (Flares::Flare* flare)
{
	flare->connectedLocator.reset();
	
	if (flare->connectedObject.Validate())
	{
		MissionObject * mobject = flare->connectedObject.Ptr();
		MO_IS_IF(tid, "GeometryObject", mobject)
		{
			GeometryObject* geom = (GeometryObject*)mobject;
			IGMXScene * scene = geom->GetScene();
			if (scene)
			{
				flare->scene = scene;
				flare->connectedLocator = scene->FindEntity(GMXET_LOCATOR, flare->locatorName);
			}
		}
	}
	//-----------------------------------
}

void _cdecl FlaresManager::Draw(float dltTime, long level)
{
	RENDERVIEWPORT vp = Render().GetViewport();

	mView = Render().GetView();	

	//mr = Render().GetWorld();
	mr = Render().GetView();
	mr *= Render().GetProjection();	
	
	mInvView = mView;
	mInvView.Inverse();

	mViewNoSwing = mView;	
	mViewNoSwing = (Mission().GetInverseSwingMatrix() * Matrix(mViewNoSwing).Inverse()).Inverse();

	mInvViewNoSwing = mViewNoSwing;
	mInvViewNoSwing.Inverse();

	//mrNoSwing = Render().GetWorld();
	mrNoSwing = mViewNoSwing;
	mrNoSwing *= Render().GetProjection();		

	frustum = Render().GetFrustum();
	
	Render().SetView(mViewNoSwing);
	frustumNoSwing = Render().GetFrustum();
	Render().SetView(mView);	

	#ifndef _XBOX
	DWORD dwFlags = LOCK_DISCARD;
	#else
	DWORD dwFlags = 0;
	#endif

	Matrix mTemp;
	FLARE_VERTEX * pFlareVerts = NULL;

	int index = 0;

	dltTime = api->GetDeltaTime();

	for (int i=0;i<(int)flares.Size();i++)
	{
		bool noSwing = flares[i]->noSwing;

		float det = cosf(flares[i]->size_angle * PI/180.0f);

		for (int j=0;j<(int)flares[i]->flares.Size();j++)
		{
			Flares::Flare* flare = &flares[i]->flares[j];			

			//-----------------------
			Matrix flare_mat;

			if (EditMode_IsOn())
			{
				flare->connectedLocator.reset();
				if (flare->connectedObject.Validate())
				{
					FindObject(flare->connectedObjectName, flare->connectedObject);
					ConnectLocator(flare);
				}

			}



			if (flare->scene && flare->connectedLocator.isValid())
			{
				//flare_mat = 
				flare->scene->GetNodeWorldTransform(flare->connectedLocator, flare_mat);//->GetWorldTransform();
			} else
			{
				if (flare->connectedObject.Validate())
				{
					flare->connectedObject.Ptr()->GetMatrix(flare_mat);
				} else
				{
					if (flare->alreadyTryToFind == false)
					{
						flare->alreadyTryToFind = true;
						FindObject(flare->connectedObjectName, flare->connectedObject);
						ConnectLocator(flare);
					}
				}
			}
			//-----------------------


			CalcFlare(flare, dltTime, flare_mat);

			Vector vFalrePos = flare->pos * flare_mat;


			//Render().DrawSphere(vFalrePos, 0.1f);


			if (noSwing) 
			{
				if (!InFrustrum(frustumNoSwing,vFalrePos, 1.0f)) continue;
			}
			else
			{
				if (!InFrustrum(frustum,vFalrePos, 1.0f)) continue;
			}		

			if (flare->intencVis>0.0f)
			{			
				if (!pFlareVerts)
				{
					pFlareVerts = (FLARE_VERTEX*)pFlareVBuffer->Lock(0, 0, dwFlags);
				}

				FLARE_VERTEX* pV = &pFlareVerts[index * 4];								

				Vector vPos = 0.0f;
				
				if (noSwing) 
				{
					vPos = mViewNoSwing * vFalrePos;
				}
				else
				{
					vPos = mView * vFalrePos;
				}

				dword dwColor = flare->color*(flare->dist*flare->dist*flare->intencVis*flare->intenc);

				Vector dir = 0.0f;
				
				if (noSwing)
				{
					dir = vFalrePos - mInvViewNoSwing.pos;
				}
				else
				{
					dir = vFalrePos - mInvView.pos;
				}

				dir.Normalize();
				
				float ang = 0.0f;
				
				if (noSwing)
				{
					ang = (dir | mInvViewNoSwing.vz);
				}
				else
				{
					ang = (dir | mInvView.vz);
				}

				float fSizeX = flares[i]->size*flare->intenc;
				float fSizeY = flares[i]->size*flare->intenc;				

				if (flares[i]->autosize)
				{
					float k = 0.0f;

					if (ang>det)
					{
						k= (fabsf(ang) - det)/(1-det);						
					}					

					fSizeX *= k;
					fSizeY *= k;
				}				

				float fCos = cosf(ang);
				float fSin = sinf(ang);

				Vector vFlarePos = vFalrePos + flares[i]->zOffset * dir;

				Vector4 vec = Vector4(vFlarePos.x,vFlarePos.y,vFlarePos.z,1.0f);

				if (noSwing)
				{
					vec = vec * mrNoSwing;					
				}
				else
				{
					vec = vec * mr;
				}

				vec = vec * mr;
				vec.x /= vec.w;
				vec.y /= vec.w;				

				//if (vec.z<0.0f) continue;

				if (noSwing)
				{
					vFlarePos = mViewNoSwing * vFlarePos;
				}
				else
				{
					vFlarePos = mView * vFlarePos;
				}

				flare->vFlarePos = vFlarePos;
												
				float scale = 0.15f * vFlarePos.z * tanf(30.0f/PI);			
				
				for (int k=0;k<4;k++)
				{
					if (k==0)
					{
						pV[k].vPos = Vector(-fSizeX, -fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
						pV[k].vMaskPos = Vector(-scale, -scale, 0.0f) + vFlarePos;
						pV[k].tu = 0.0f;	
						pV[k].tv = 1.0f;
					}
					else
					if (k==1)
					{
						pV[k].vPos = Vector(-fSizeX, fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
						pV[k].vMaskPos = Vector(-scale, scale, 0.0f) + vFlarePos;
						pV[k].tu = 0.0f;	
						pV[k].tv = 0.0f;
					}
					else
					if (k==2)
					{
						pV[k].vPos = Vector(fSizeX, fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
						pV[k].vMaskPos = Vector(scale, scale, 0.0f) + vFlarePos;
						pV[k].tu = 1.0f;	
						pV[k].tv = 0.0f;
					}
					else
					if (k==3)
					{
						pV[k].vPos = Vector(fSizeX, -fSizeY, 0.0f).RotateXY(fCos, fSin) + vPos;
						pV[k].vMaskPos = Vector(scale, -scale, 0.0f) + vFlarePos;
						pV[k].tu = 1.0f;	
						pV[k].tv = 1.0f;
					}
																			
					pV[k].dwColor = dwColor;
					pV[k].tu2 = flare->u + 0.5f / 32.0f;
					pV[k].tv2 = flare->v + 0.5f / 32.0f;								
				}											

				index++;

				if (index>=1024)
				{
					break;					
				}
			}
		}
	}

	DrawFlares(pFlareVerts, index, dltTime);

	for (int i=0;i<(int)flares.Size();i++)
	{
		if (!flares[i]->showDebug) continue;

		for (int j=0;j<(int)flares[i]->flares.Size();j++)
		{
			Render().Print(flares[i]->flares[j].pos,10.0f,0,0xff00aa00,"%s - %i",flares[i]->GetObjectID().c_str(),j);
		}
	}
}


void FlaresManager::Release()
{
	RELEASE(texture);
	RELEASE(pMask);
	RELEASE(pZMask);
	RELEASE(pPowMask);
	RELEASE(pPowSecMask);
	
	pVarTex = NULL;
	pVarMask = NULL;
	pFlareColor = NULL;
	pFlareTexSzU = NULL;
	pFlareTexSzV = NULL;

	RELEASE(pFlareVBuffer);
	RELEASE(pFlareIBuffer);
	RELEASE(pFlare2DVBuffer);	

	first_time = true;
}

MOP_BEGINLIST(FlaresManager, "", '1.00', 0)
MOP_ENDLIST(FlaresManager)




