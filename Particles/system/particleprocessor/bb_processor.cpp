
#ifndef _XBOX
#include <xmmintrin.h>
#endif

#include "bb_processor.h"
#include "..\..\icommon\names.h"
#include "..\..\icommon\IEmitter.h"
#include "..\datasource\datagraph.h"
#include "..\datasource\datacolor.h"
#include "..\datasource\datauv.h"
#include "..\..\..\common_h\render.h"
#include "..\..\..\common_h\core.h"
#include "..\particlesystem\particlesystem.h"
#include "..\..\manager\particlemanager.h"
#include "physic.h"
#include "nameparser.h"
#include "..\..\..\common_h\gmx.h"



extern ParticleService* PService;




/* 
Коэфицент лодирования партиклов, чем выше тем меньше нагрузка на Filrate
Должен быть > 0 !!!!!!
0.001 нет лодов .. 10000000 супер лоды (все партиклы в 1 пиксель)
*/
//=============================================================
//#define PLOD 0.001f 
//=============================================================


BillBoardProcessor* BillBoardProcessor::singleton = NULL;
IRender* BillBoardProcessor::pRS = NULL;



CritSection BillBoardProcessor::draw_particles;





BillBoardProcessor::BillBoardProcessor () : ParticlesIdx(_FL_, PARTICLE_CHUNK),
                                            freeParticlesIdx(_FL_, PARTICLE_CHUNK),
                                            particlesData(_FL_, PARTICLE_CHUNK)
{

	pGEOServ = NULL;


	//pMemArray = NEW BB_ParticleData[_MAX_BILLBOARDS];

	AllocParticles();



	BillBoardProcessor::pRS = (IRender*)api->GetService("DX9Render");
	Assert (BillBoardProcessor::pRS);

	pRS->GetShaderId("Particles", shader_particles[0]);
	pRS->GetShaderId("ParticlesXZ", shader_particlesXZ[0]);
	pRS->GetShaderId("ParticlesDistor", shader_particles_distor[0]);
	pRS->GetShaderId("ParticlesDistXZ", shader_particlesXZ_distor[0]);



	pRS->GetShaderId("Particles_overdraw", shader_particles[1]);
	pRS->GetShaderId("ParticlesXZ_overdraw", shader_particlesXZ[1]);
	pRS->GetShaderId("ParticlesDistor_overdraw", shader_particles_distor[1]);
	pRS->GetShaderId("ParticlesDistXZ_overdraw", shader_particlesXZ_distor[1]);

	pRS->GetShaderId("Particles_simple", shader_particles[2]);
	pRS->GetShaderId("ParticlesXZ_simple", shader_particlesXZ[2]);



	int RectVertexSize =  sizeof(RECT_VERTEX);

	BillBoardProcessor::singleton = this;


}

BillBoardProcessor::~BillBoardProcessor ()
{
} 


//"Выделить" память для хранения партикла
dword BillBoardProcessor::AllocParticle ()
{
	if (freeParticlesIdx.IsEmpty())
	{
		if(particlesData.Size() > 16384)
		{
			return NULL;
		}


		AllocParticles();

		if (freeParticlesIdx.IsEmpty())
		{
			return NULL;
		}
	}

	dword dwIdx = freeParticlesIdx.Top();
	freeParticlesIdx.Pop();

	particlesData[dwIdx].index_in_array = dwIdx;

	return dwIdx;
}

//"Убить" партикл
void BillBoardProcessor::FreeParticle (dword dwIndex)
{
	freeParticlesIdx.Push(dwIndex);
}


void BillBoardProcessor::AddParticle (const Vector& addVel, ParticleSystem* pSystem, const Vector& velocity_dir, const Vector& pos, const Matrix& matWorld, float EmitterTime, float EmitterLifeTime, FieldList* pFields, DWORD* pActiveCount, IEmitter* dwGUID, float fTimeScale, float fScale, bool bForceLocalMode)
{
	SyncroCode sync(access_to_particles);

	dword dwParticleIdx = AllocParticle ();
	BB_ParticleData* pData = &particlesData[dwParticleIdx];

	//Сработает если партиклов будет > MAX_BILLBOARDS, столько их быть не должно :))))
	if (!pData)
	{
		*(pActiveCount) = (*(pActiveCount)-1);
		return;
	}


	pData->localMode = pFields->GetBool(GUID_PARTICLE_LOCAL_SYSTEM, false);;

	if (bForceLocalMode)
	{
		pData->localMode = true;
	}


	pData->fTimeScale = fTimeScale;
	pData->fAllsystemScale = fScale;

/*
	//запоминаем все force field'ы
	dword dwForceFieldActive = pFields->GetForceFieldsCountFromCache();
	for (dword q = 0; q < 4; q++)
	{
		dword dwGuid = 0xFFFFFFFF;
		if (q < dwForceFieldActive) dwGuid = pFields->GetForceFieldGUIDFromCache(q);
		pData->dwForceFields_GUID[q] = dwGuid;
	}
*/

	Vector initPos;
	initPos.x = pFields->GetRandomGraphVal(GUID_PARTICLE_INITPOSX, EmitterTime, EmitterLifeTime);
	initPos.y = pFields->GetRandomGraphVal(GUID_PARTICLE_INITPOSY, EmitterTime, EmitterLifeTime);
	initPos.z = pFields->GetRandomGraphVal(GUID_PARTICLE_INITPOSZ, EmitterTime, EmitterLifeTime);

	pData->distorted = pFields->GetBool(GUID_PARTICLE_DISTORTED, false);


	pData->distorted_pow = pFields->FindGraphByGUID(GUID_PARTICLE_DISTORTED_POWER);
	pData->Graph_SpinDrag = pFields->FindGraphByGUID(GUID_PARTICLE_SPIN_DRAG);
	pData->Graph_Size = pFields->FindGraphByGUID(GUID_PARTICLE_SIZE);
	pData->Graph_SizeInc = pFields->FindGraphByGUID(GUID_PARTICLE_SIZEINC);
	pData->Graph_LightK = pFields->FindGraphByGUID(GUID_PARTICLE_LIGHTPOWER);
	pData->Graph_AmbientK = pFields->FindGraphByGUID(GUID_PARTICLE_AMBIENTPOWER);
	pData->Graph_Frames = pFields->FindGraphByGUID(GUID_PARTICLE_ANIMFRAME);
	pData->Graph_Color = pFields->FindColorByGUID(GUID_PARTICLE_COLOR);
	pData->Graph_ColorMul = pFields->FindGraphByGUID(GUID_PARTICLE_COLORMULTIPLY);
	pData->Graph_UV = pFields->FindUVByGUID(GUID_PARTICLE_FRAMES);
	pData->Graph_Transparency = pFields->FindGraphByGUID(GUID_PARTICLE_TRANSPARENCY);
	pData->Graph_Drag = pFields->FindGraphByGUID(GUID_PARTICLE_DRAG);
	pData->graph_GravK = pFields->FindGraphByGUID(GUID_PARTICLE_GRAVITATION_K);
	pData->graph_AddPower = pFields->FindGraphByGUID(GUID_PARTICLE_ADDPOWER);






	pData->distorted_pow_K = FRAND (1.0f);


	pData->xz_align = pFields->GetBool(GUID_PARTICLE_ZXALIGN, false);

/*
	if (pFields->GetBool(GUID_PARTICLE_AFFECT_AMBIENT, false))
	{
		pData->alwaysAmbient = 0.0f;
	} else
	{
		pData->alwaysAmbient = 1.0f;
	}
*/

	pData->exForce = pFields->GetPosition(GUID_PARTICLE_EXTERNALFORCE);

	pData->SpeedOriented = pFields->GetBool(GUID_PARTICLE_DIR_ORIENT, false);
	pData->EmitterGUID = dwGUID;
	pData->ActiveCount = pActiveCount;

	if (!pData->localMode)
	{
		pData->RenderPos = (pos+initPos) * matWorld;
		pData->Velocity = matWorld.MulNormal(velocity_dir);
		pData->matWorld = matWorld;

		pData->matWorldInv = matWorld;
		pData->matWorldInv.Inverse();

	} else
	{
		pData->RenderPos = (pos+initPos);
		pData->Velocity = velocity_dir;
		pData->matWorld = Matrix();

		pData->matWorldInv = Matrix();
	}


	
	pData->ElapsedTime = 0.0f;
	


	pData->ExternalForce = Vector(0.0f, 0.0f, 0.0f);
	pData->PhysPos = pData->RenderPos;

	pData->OldRenderPos = pData->RenderPos;
	pData->OldRenderAngle = pData->RenderAngle;

	pData->LifeTime = pFields->GetRandomGraphVal(GUID_PARTICLE_LIFE_TIME, EmitterTime, EmitterLifeTime);
	pData->Mass = pFields->GetRandomGraphVal(GUID_PARTICLE_MASS, EmitterTime, EmitterLifeTime);
	if (pData->Mass < 0.01) pData->Mass = 0.01f;

	float VelocityPower = pFields->GetRandomGraphVal(GUID_PARTICLE_VELOCITY_POWER, EmitterTime, EmitterLifeTime);
	pData->Velocity = (pData->Velocity * VelocityPower) + addVel;

	pData->fInitialSize = pFields->GetRandomGraphVal(GUID_PARTICLE_INITSIZE, EmitterTime, EmitterLifeTime);

	pData->Spin = pFields->GetRandomGraphVal(GUID_PARTICLE_SPIN, EmitterTime, EmitterLifeTime);
	pData->Spin = pData->Spin * MUL_DEGTORAD;


	pData->Angle = pFields->GetRandomGraphVal(GUID_PARTICLE_INITIALSPIN, EmitterTime, EmitterLifeTime);
	pData->Angle = pData->Angle * MUL_DEGTORAD;

	pData->RenderAngle = pData->Angle;



	pData->DragK = FRAND (1.0f);
	pData->SpinDragK = FRAND (1.0f);
	pData->SizeK = FRAND (1.0f);
	pData->ColorK = FRAND (1.0f);
	pData->AlphaK = FRAND (1.0f);
	pData->FrameK = FRAND (1.0f);
	pData->GravKK = FRAND (1.0f);
	pData->AddPowerK = FRAND (1.0f);

	pData->TrackXK = FRAND (1.0f);
	pData->TrackYK = FRAND (1.0f);
	pData->TrackZK = FRAND (1.0f);
	pData->PhysBlendK = FRAND (1.0f);
	pData->LightK_K = FRAND (1.0f);



	const char* pEmitterName = pFields->GetString(GUID_ATTACHEDEMITTER_NAME);

	if (pEmitterName[0] == 'n' && pEmitterName[1] == 'o' && pEmitterName[2] == 'n' && pEmitterName[3] == 'e')
		//if (crt_stricmp (pEmitterName, "none") == 0)
	{
		pData->AttachedEmitter = NULL;
	} else
	{
		pData->AttachedEmitter = pSystem->FindEmitter(pEmitterName);
		if (pData->AttachedEmitter) pData->AttachedEmitter->SetAttachedFlag(true);
	}

	ParticlesIdx.Add(dwParticleIdx);
}


void BillBoardProcessor::DeleteDeadParticles ()
{

	for (DWORD n = 0; n < ParticlesIdx.Size(); n++)
	{
		dword dwParticleIdx = ParticlesIdx[n];
		BB_ParticleData* pData = &particlesData[dwParticleIdx];

		float Time = pData->ElapsedTime;
		float LifeTime = pData->LifeTime;

		//Сразу убиваем дохлые...
		if (Time > LifeTime)
		{
			*(pData->ActiveCount) = (*(pData->ActiveCount)-1);
			FreeParticle (dwParticleIdx);
			ParticlesIdx.ExtractNoShift(n);
			n--;
			continue;
		}

	}


}

//Считает физику, треки  и т.д.
void BillBoardProcessor::UpdateParticles (float RealDeltaTime)
{
	DWORD dwFrom = 0;
	DWORD dwTo = ParticlesIdx.Size();

	Matrix m_temp(false);
	DWORD processCount = 0;
	for (DWORD n = dwFrom; n < dwTo; n++)
	{
		dword dwParticleIdx = ParticlesIdx[n];
		BB_ParticleData* pData = &particlesData[dwParticleIdx];

		float DeltaTime = RealDeltaTime * pData->fTimeScale;

		pData->ElapsedTime += DeltaTime;

		float Time = pData->ElapsedTime;
		float LifeTime = pData->LifeTime;

		//Сразу дохлые пропускаем...
		if (Time > LifeTime)
		{
			continue;
		}

		float Drag = pData->Graph_Drag->GetValue(Time, LifeTime, pData->DragK);




		float GravK = pData->graph_GravK->GetValue(Time, LifeTime, pData->GravKK);

		AddGravityForce (pData->ExternalForce, pData->Mass, GravK);
		//pData->ExternalForce.y += GravK*pData->Mass;

		//внешняя сила
		pData->ExternalForce += pData->exForce;

		SolvePhysic (pData->PhysPos, pData->Velocity, pData->ExternalForce, pData->Mass, Drag, DeltaTime);
		pData->ExternalForce = 0.0f;

		float SpinDrag = pData->Graph_SpinDrag->GetValue(Time, LifeTime, pData->SpinDragK);
		SpinDrag = 1.0f -(SpinDrag * 0.01f);
		if (SpinDrag < 0.0f) SpinDrag = 0.0f;
		if (SpinDrag > 1.0f) SpinDrag = 1.0f;
		pData->Angle += (pData->Spin * SpinDrag) * DeltaTime;

		//Save old positions
		pData->OldRenderPos = pData->RenderPos;

		//JOKER SCALE


		

/*		
		pData->EmitterGUID->GetSystemTransform(pData->matWorld);
		pData->RenderPos = (pData->PhysPos * pData->fAllsystemScale) * pData->matWorld;
*/


		if (!pData->localMode)
		{
			pData->EmitterGUID->GetSystemTransform(m_temp);
			pData->RenderPos = ((pData->PhysPos - m_temp.pos) * pData->fAllsystemScale) + m_temp.pos;
		}  else
		{
			pData->EmitterGUID->GetSystemTransform(pData->matWorld);
			pData->RenderPos = (pData->PhysPos * pData->fAllsystemScale) * pData->matWorld;
		}


		pData->RenderAngle = pData->Angle;

		/*
		if (pData->dwForceFields_GUID[0] != 0xFFFFFFFF ||
		pData->dwForceFields_GUID[1] != 0xFFFFFFFF ||
		pData->dwForceFields_GUID[2] != 0xFFFFFFFF ||
		pData->dwForceFields_GUID[3] != 0xFFFFFFFF)
		{
		bool bNeedKill = pMasterManager->ExecuteForceFields( pData->dwForceFields_GUID[0],
		pData->dwForceFields_GUID[1],
		pData->dwForceFields_GUID[2],
		pData->dwForceFields_GUID[3], 
		pData->OldRenderPos,
		pData->RenderPos,
		pData->ExternalForce);

		if (bNeedKill)
		{
		*(pData->ActiveCount) = (*(pData->ActiveCount)-1);
		FreeParticle (pData);
		Particles.ExtractNoShift(n);
		n--;
		continue;
		}
		}
		*/


		processCount++;

	} // цикл на Particles.Size()


	//Рождаем партиклы, которые привязанны к нашему партиклу...

	Matrix mat;
	Vector dir;


	processCount = 0;
	for (DWORD n = dwFrom; n < dwTo; n++)
	{
		dword dwParticleIdx = ParticlesIdx[n];
		BB_ParticleData* pData = &particlesData[dwParticleIdx];

		if (pData->AttachedEmitter)
		{
			dir = pData->OldRenderPos - pData->RenderPos;
			mat.BuildView(Vector(0.0f, 0.0f, 0.0f), dir, Vector(0.0f, 1.0f, 0.0f));

			mat.pos = pData->OldRenderPos;
			pData->AttachedEmitter->Teleport(mat);

			mat.pos = pData->RenderPos;
			pData->AttachedEmitter->SetTransform(mat);

			pData->AttachedEmitter->BornParticles(RealDeltaTime, pData->fTimeScale, pData->fAllsystemScale, 0.0f);

		}

		processCount++;

	} // цикл на Particles.Size()


	//RDTSC_E (t);
	//api->Trace("Time - %d", t);
}

//Считает расстояние до билбоардов
DWORD BillBoardProcessor::CalcDistanceToCamera (const Matrix& mView)
{
	DWORD VisParticles = 0;
	Vector vCamPos = mView.GetCamPos();
	for (DWORD j = 0; j <  ParticlesIdx.Size(); j++)
	{
		dword dwParticleIdx = ParticlesIdx[j];
		BB_ParticleData* pData = &particlesData[dwParticleIdx];


		pData->CamDistance = ((pData->RenderPos - vCamPos) | (pData->RenderPos - vCamPos));
		pData->Visible = true;
	}
	return ParticlesIdx.Size();
}






void BillBoardProcessor::BuildVertexBuffers(SyncParams & threadParams)
{
	const Matrix & matView = threadParams.mView;
	const Plane* pFrustum = threadParams.planes;

	RECT_VERTEX * pParticlesVerts = threadParams.updateData->pParticlesVerts;
	RECT_VERTEX * pParticlesXZVerts = threadParams.updateData->pParticlesXZVerts;
	RECT_VERTEX * pDistortedParticlesVerts = threadParams.updateData->pDistortedParticlesVerts;
	RECT_VERTEX * pDistortedParticlesXZVerts = threadParams.updateData->pDistortedParticlesXZVerts;

	bool bHaveDistortParticles = false;
	if (CalcDistanceToCamera(matView) == 0)
	{
		//нет партиклов вообще...
		return;
	}
	ParticleSorter.QSort(CompareFunction, &ParticlesIdx[0], ParticlesIdx.Size(), this);

	VbMarkup & markup = threadParams.updateData->markup;

	Assert(pParticlesVerts);
	Assert(pParticlesXZVerts);
	Assert(pDistortedParticlesVerts);
	Assert(pDistortedParticlesXZVerts);

	bHaveDistortParticles = UniformBuild<PTYPE_NORMAL>(pFrustum, matView, pParticlesVerts, markup.particles_count);
	UniformBuild<PTYPE_NORMAL_XZ>(pFrustum, matView, pParticlesXZVerts, markup.particlesXZ_count);

	if (bHaveDistortParticles)
	{
		UniformBuild<PTYPE_DISTORTED>(pFrustum, matView, pDistortedParticlesVerts, markup.distortedParticles_count);
		UniformBuild<PTYPE_DISTORTED_XZ>(pFrustum, matView, pDistortedParticlesXZVerts, markup.distortedParticlesXZ_count);
	}

}

DWORD BillBoardProcessor::GetCount ()
{
	SyncroCode sync(access_to_particles);

	return ParticlesIdx.Size();
}

void BillBoardProcessor::DeleteWithGUID (IEmitter* dwGUID)
{
	SyncroCode sync(access_to_particles);

	for (DWORD j = 0; j <  ParticlesIdx.Size(); j++)
	{
		dword dwParticleIdx = ParticlesIdx[j];
		BB_ParticleData* pR = &particlesData[dwParticleIdx];


		if (pR->EmitterGUID == dwGUID)
		{
			*(pR->ActiveCount) = (*(pR->ActiveCount)-1);
			FreeParticle (dwParticleIdx);
			ParticlesIdx.ExtractNoShift(j);
			j--;
		}
	}
}

void BillBoardProcessor::Clear ()
{
	SyncroCode sync(access_to_particles);

	for (DWORD j = 0; j <  ParticlesIdx.Size(); j++)
	{
		dword dwParticleIdx = ParticlesIdx[j];
		BB_ParticleData* pR = &particlesData[dwParticleIdx];

		*(pR->ActiveCount) = (*(pR->ActiveCount)-1);
		FreeParticle (dwParticleIdx);
	}
	ParticlesIdx.DelAll();
}



bool BillBoardProcessor::Draw (const ParticleVB & renderVB, IIBuffer * pIBuffer, bool bSoftParticlesSupport)
{
	SyncroCode sync(BillBoardProcessor::draw_particles);

	BillBoardProcessor::pRS->SetWorld(Matrix());

	long mode = 0;

	if (bSoftParticlesSupport == false)
	{
		mode = 2;
	}

#ifndef STOP_DEBUG
	if (pGEOServ)
	{
		IGMXService::HackMode hackMode = pGEOServ->GetHackMode();
		if (hackMode == IGMXService::HM_SHOW_OVERDRAW)
		{
			mode = 1;
		}
	} else
	{
		pGEOServ = (IGMXService*)api->GetService("GMXService");
	}
#endif

	
	DWORD dwParticlesCount = renderVB.markup.particles_count;
	if (dwParticlesCount > 0)
	{
		dword dwEvt = BillBoardProcessor::pRS->pixBeginEvent(_FL_, "SimpleParticles");

		BillBoardProcessor::pRS->SetStreamSource(0, renderVB.pParticles_VB);
#ifndef _XBOX
		BillBoardProcessor::pRS->SetIndices(pIBuffer, 0);
		BillBoardProcessor::pRS->DrawIndexedPrimitive(shader_particles[mode], PT_TRIANGLELIST, 0, dwParticlesCount*4, 0, dwParticlesCount*2);
#else
		BillBoardProcessor::pRS->DrawPrimitive(shader_particles[mode], PT_QUADLIST, 0, dwParticlesCount);
#endif



		BillBoardProcessor::pRS->pixEndEvent(_FL_, dwEvt);
	}
	//api->SetPerformanceCounter("Particles::Billboards draw", (float)drawParticleBuffer.ParticlesCount0);


	DWORD dwParticlesXZCount = renderVB.markup.particlesXZ_count;
	if (dwParticlesXZCount > 0)
	{
		dword dwEvt = BillBoardProcessor::pRS->pixBeginEvent(_FL_, "XZSimpleParticles");

		BillBoardProcessor::pRS->SetStreamSource(0, renderVB.pParticlesXZ_VB);

#ifndef _XBOX
		BillBoardProcessor::pRS->SetIndices(pIBuffer, 0);
		BillBoardProcessor::pRS->DrawIndexedPrimitive(shader_particlesXZ[mode], PT_TRIANGLELIST, 0, dwParticlesXZCount*4, 0, dwParticlesXZCount*2);
#else
		BillBoardProcessor::pRS->DrawPrimitive(shader_particlesXZ[mode], PT_QUADLIST, 0, dwParticlesXZCount);
#endif


		BillBoardProcessor::pRS->pixEndEvent(_FL_, dwEvt);
	}
	//api->SetPerformanceCounter("Particles::ZBillboards draw", (float)drawParticleBuffer.ParticlesCount1);


	BillBoardProcessor::pRS->SetStreamSource(0, NULL);

	if (renderVB.markup.distortedParticles_count > 0) return true;
	if (renderVB.markup.distortedParticlesXZ_count > 0) return true;

	return false;
}

void BillBoardProcessor::DrawDistorted (const ParticleVB & renderVB, IIBuffer * pIBuffer)
{
	SyncroCode sync(BillBoardProcessor::draw_particles);

	BillBoardProcessor::pRS->SetWorld(Matrix());

	long mode = 0;

#ifndef STOP_DEBUG
	if (pGEOServ)
	{
		IGMXService::HackMode hackMode = pGEOServ->GetHackMode();
		if (hackMode == IGMXService::HM_SHOW_OVERDRAW)
		{
			mode = 1;
		}
	} else
	{
		pGEOServ = (IGMXService*)api->GetService("GMXService");
	}
#endif


	DWORD dwDistortedParticlesCount = renderVB.markup.distortedParticles_count;
	if (dwDistortedParticlesCount > 0)
	{
		dword dwEvt = BillBoardProcessor::pRS->pixBeginEvent(_FL_, "DistorParticles");

		BillBoardProcessor::pRS->SetStreamSource(0, renderVB.pDistortedParticles_VB);
#ifndef _XBOX
		BillBoardProcessor::pRS->SetIndices(pIBuffer, 0);
		BillBoardProcessor::pRS->DrawIndexedPrimitive(shader_particles_distor[mode], PT_TRIANGLELIST, 0, dwDistortedParticlesCount*4, 0, dwDistortedParticlesCount*2);
#else
		BillBoardProcessor::pRS->DrawPrimitive(shader_particles_distor[mode], PT_QUADLIST, 0, dwDistortedParticlesCount);
#endif


		BillBoardProcessor::pRS->pixEndEvent(_FL_, dwEvt);
	}
	//api->SetPerformanceCounter("Particles::Billboards distorted draw", (float)drawParticleBuffer.ParticlesCount2);

	DWORD dwDistortedParticlesXZCount = renderVB.markup.distortedParticlesXZ_count;
	if (dwDistortedParticlesXZCount > 0)
	{
		dword dwEvt = BillBoardProcessor::pRS->pixBeginEvent(_FL_, "XZDistorParticles");

		BillBoardProcessor::pRS->SetStreamSource(0, renderVB.pDistortedParticlesXZ_VB);

#ifndef _XBOX
		BillBoardProcessor::pRS->SetIndices(pIBuffer, 0);
		BillBoardProcessor::pRS->DrawIndexedPrimitive(shader_particlesXZ_distor[mode], PT_TRIANGLELIST, 0, dwDistortedParticlesXZCount*4, 0, dwDistortedParticlesXZCount*2);
#else

		BillBoardProcessor::pRS->DrawPrimitive(shader_particlesXZ_distor[mode], PT_QUADLIST, 0, dwDistortedParticlesXZCount);

#endif


		BillBoardProcessor::pRS->pixEndEvent(_FL_, dwEvt);
	}
	//api->SetPerformanceCounter("Particles::ZBillboards distorted draw", (float)drawParticleBuffer.ParticlesCount3);

	BillBoardProcessor::pRS->SetStreamSource(0, NULL);
}


