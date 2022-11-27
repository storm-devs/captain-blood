//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************

#ifndef BILLBOARD_PARTICLE_PROCESSOR
#define BILLBOARD_PARTICLE_PROCESSOR

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\..\common_h\render.h"
//#include "..\..\..\common_h\QSort.h"


#include "..\..\icommon\particle.h"
#include "..\datasource\fieldlist.h"
#include "nameparser.h"

#include "..\..\service\particleservice.h"

#include "..\..\manager\syncParams.h"


class ParticleSystem;


#define PARTICLE_CHUNK (2048)

#define DISTORTION_NEAR (10.0f * 10.0f)
#define DISTORTION_MAX_DIST ((60.0f * 60.0f) + DISTORTION_NEAR)
#define DISTORTION_RADIUS (1.0f / DISTORTION_MAX_DIST)


#define UV_TX1 0
#define UV_TX2 2
#define UV_TY1 3
#define UV_TY2 1


//! Проверка видимости сферы в frustum'e камеры
__forceinline bool SphereIsVisibleInFrustum (const Plane *frustum, const Vector& SphereCenter, float SphereRadius)
{
	//Отсекаем только левый/правый/нижний плоскости
	for(long p = 0; p < 3; p++)
		if(((SphereCenter | frustum[p].n) - frustum[p].d) < -SphereRadius) return false;

	return true;
}


template <class TYPE>
class FastQSortParticles
{

public:

	__forceinline void QSort(int (*compare_f)(TYPE,TYPE,void* ptr), TYPE* ArrayToSort, int uCount, void* ptr)
	{
		if (uCount <= 0) return;
		QSortRecursive (compare_f, ArrayToSort, 0, (uCount-1), ptr);
	}

protected:

	__forceinline void QSortRecursive(int (*compare_func)(TYPE,TYPE,void* ptr),  TYPE* pArr, int d, int h, void* ptr)
	{
		int i,j;
		TYPE str;

begin:

		i = h;
		j = d;

		str = pArr[(d+h)>>1];

		do 
		{
			while (compare_func (pArr[j], str, ptr) && (j < h)) j++;
			while (compare_func (str, pArr[i], ptr) && (i > d)) i--;

			if ( i >= j )
			{
				if ( i != j )
				{
					TYPE zal;
					zal     = pArr[i];
					pArr[i] = pArr[j];
					pArr[j] = zal;
				}
				i--;
				j++;
			}
		} while (j <= i);

		if (d < i)
		{
			QSortRecursive(compare_func, pArr,d,i, ptr);
		}

		if (j < h)
		{
			d = j;
			goto begin;
		}
	}
};


class IGMXService;

enum ParticleTypeForTemplateFunc
{
	PTYPE_NORMAL = 0,
	PTYPE_NORMAL_XZ = 1,

	PTYPE_DISTORTED = 2,
	PTYPE_DISTORTED_XZ = 3,

	PTYPE_FORCE_DWORD = 0x7fffffff
};

class BillBoardProcessor
{
	stack<dword> freeParticlesIdx;

	IGMXService* pGEOServ;

	GeomNameParser Parser;

	array<BB_ParticleData> particlesData;

	array<dword> ParticlesIdx;

	FastQSortParticles<dword> ParticleSorter;

	__forceinline void AllocParticles ()
	{
		DWORD dwStart = particlesData.Size();
		particlesData.AddElements(PARTICLE_CHUNK);
		for (DWORD n = dwStart; n < particlesData.Size(); n++)
		{
			freeParticlesIdx.Push(n);
		}
	}


	//Считает расстояние до билбоардов
	DWORD CalcDistanceToCamera (const Matrix& mView);

	//Функция сравнения при сортировке
	__forceinline static BOOL CompareFunction (dword e1, dword e2, void* ptr)
	{
		BillBoardProcessor* proc = (BillBoardProcessor*)ptr;

		if (proc->particlesData[e1].CamDistance > proc->particlesData[e2].CamDistance)
		{
			return true;
		}
		return false;
	}


	dword AllocParticle ();
	void FreeParticle (dword dwIndex);



	template <ParticleTypeForTemplateFunc _particleType>
	__forceinline bool UniformBuild(const Plane* Frustum, const Matrix& matView, RECT_VERTEX * __restrict vertsData, DWORD & particlesNeedToRender)
	{
		bool bHaveDistortParticles = false;
		DWORD ParticlesCount = 0;
		Vector SpeedVector;
		Color particleColor;
		Vector4 UV_WH1;
		Vector4 UV_WH2;

		for (DWORD j = 0; j <  ParticlesIdx.Size(); j++)
		{
			dword dwParticleIdx = ParticlesIdx[j];
			BB_ParticleData* pR = &particlesData[dwParticleIdx];

			if (pR->Visible == false) continue;

			const Vector &vPos = pR->RenderPos;
			if (!SphereIsVisibleInFrustum(Frustum, vPos, 1.8f))
			{
				continue;
			}


			//этого if не должно быть... так как он определен на этапе компиляции...
			//------------------------------------------------------------------------------------
			if (_particleType == PTYPE_NORMAL)
			{
				if (ParticlesCount >= MAX_BILLBOARDS_TYPE0)
				{
					break;
				}

				if (pR->distorted == true)
				{
					bHaveDistortParticles = true;
					continue;
				}

				 if (pR->xz_align == true)
				 {
					 continue;
				 }
			}
			//------------------------------------------------------------------------------------


			//этого if не должно быть... так как он определен на этапе компиляции...
			//------------------------------------------------------------------------------------
			if (_particleType == PTYPE_DISTORTED)
			{
				if (ParticlesCount >= MAX_BILLBOARDS_TYPE2)
				{
					break;
				}

				if (pR->distorted == false)
				{
					continue;
				}

				if (pR->xz_align == true)
				{
					continue;
				}

				if (pR->CamDistance > DISTORTION_MAX_DIST)
				{
					continue;
				}
			}
			//------------------------------------------------------------------------------------

			//этого if не должно быть... так как он определен на этапе компиляции...
			//------------------------------------------------------------------------------------
			if (_particleType == PTYPE_NORMAL_XZ)
			{
				if (ParticlesCount >= MAX_BILLBOARDS_TYPE1)
				{
					break;
				}


				if (pR->distorted == true)
				{
					bHaveDistortParticles = true;
					continue;
				}

				if (pR->xz_align == false)
				{
					continue;
				}
			}
			//------------------------------------------------------------------------------------


			//этого if не должно быть... так как он определен на этапе компиляции...
			//------------------------------------------------------------------------------------
			if (_particleType == PTYPE_DISTORTED_XZ)
			{
				if (ParticlesCount >= MAX_BILLBOARDS_TYPE3)
				{
					break;
				}

				if (pR->distorted == false)
				{
					continue;
				}

				if (pR->xz_align != true)
				{
					continue;
				}

				if (pR->CamDistance > DISTORTION_MAX_DIST)
				{
					continue;
				}
			}
			//------------------------------------------------------------------------------------


			bool SpeedOriented = pR->SpeedOriented;
			float fSize = pR->Graph_Size->GetValue(pR->ElapsedTime, pR->LifeTime, pR->SizeK);
			float fIncSize = pR->Graph_SizeInc->GetValue(pR->ElapsedTime, pR->LifeTime, pR->SizeK);
			fSize = (pR->fInitialSize * (fSize * 0.01f)) + fIncSize;

			fSize = fSize * pR->fAllsystemScale;

			if (fSize <= 0.001f) continue;

			float fAngle = pR->RenderAngle;

			particleColor = pR->Graph_Color->GetValue(pR->ElapsedTime, pR->LifeTime, pR->ColorK);
			float particleColorMul = pR->Graph_ColorMul->GetValue(pR->ElapsedTime, pR->LifeTime, pR->AlphaK);

			particleColor = particleColor * particleColorMul;

			float Alpha = pR->Graph_Transparency->GetValue(pR->ElapsedTime, pR->LifeTime, pR->AlphaK);
			Alpha = Alpha * 0.01f;
			Alpha = 1.0f - Alpha;
			if (Alpha < 0.0f) Alpha = 0.0f;
			if (Alpha > 1.0f) Alpha = 1.0f;
			particleColor.a = Alpha;


			
			
			float AddPower;

			//этого if не должно быть... так как он определен на этапе компиляции...
			//------------------------------------------------------------------------------------
			if (_particleType == PTYPE_NORMAL || _particleType == PTYPE_NORMAL_XZ)
			{
				//обычние партиклы

				AddPower = pR->graph_AddPower->GetValue(pR->ElapsedTime, pR->LifeTime, pR->AddPowerK);
				AddPower = AddPower * 0.01f;
				AddPower = 1.0f - AddPower;
				if (AddPower < 0.0f) AddPower = 0.0f;
				if (AddPower > 1.0f) AddPower = 1.0f;
			} else
			{
				//distorted партиклы

				//Скалируем силу бампа от дистанции так, что бы на расстоянии от 10 до 60 метров...
				AddPower = (pR->distorted_pow->GetValue(pR->ElapsedTime, pR->LifeTime, pR->distorted_pow_K) * 0.01f);
				float distanceMultipler = 1.0f - Clampf((pR->CamDistance - DISTORTION_NEAR) * DISTORTION_RADIUS);
				AddPower = AddPower * distanceMultipler;
			}

			



			float LightPower = pR->Graph_LightK->GetValue(pR->ElapsedTime, pR->LifeTime, pR->LightK_K);
			LightPower = LightPower * 0.01f;
			if (LightPower > 1.0f) LightPower = 1.0f;
			if (LightPower < 0.0f) LightPower = 0.0f;

			float AmbientPower = pR->Graph_AmbientK->GetValue(pR->ElapsedTime, pR->LifeTime, pR->LightK_K);
			AmbientPower = AmbientPower * 0.01f;
			if (AmbientPower > 1.0f) AmbientPower = 1.0f;
			if (AmbientPower < 0.0f) AmbientPower = 0.0f;

			float FrameIndex = pR->Graph_Frames->GetValue(pR->ElapsedTime, pR->LifeTime, pR->FrameK);

			float DirAngle = 0.0f;
			float ScaleF = 1.0f;

			if (SpeedOriented)
			{
				//этого if не должно быть... так как он определен на этапе компиляции...
				//------------------------------------------------------------------------------------
				if (_particleType == PTYPE_NORMAL || _particleType == PTYPE_DISTORTED)
				{
					SpeedVector = pR->Velocity;
					if (pR->localMode)
					{
						pR->EmitterGUID->GetSystemTransform(pR->matWorld);
						SpeedVector = pR->matWorld.MulNormal(SpeedVector);
					}

					SpeedVector = matView.MulNormal(SpeedVector);
					SpeedVector.Normalize();
					FrameIndex *= (1.0f + SpeedVector.z)*0.5f;
					SpeedVector.z = SpeedVector.y;
					DirAngle = SpeedVector.GetAY(pR->OldRenderAngle);
					pR->OldRenderAngle = DirAngle;
				}
				//------------------------------------------------------------------------------------


				//этого if не должно быть... так как он определен на этапе компиляции...
				//------------------------------------------------------------------------------------
				if (_particleType == PTYPE_NORMAL_XZ || _particleType == PTYPE_DISTORTED_XZ)
				{
					SpeedVector = pR->Velocity;

					if (pR->localMode)
					{
						pR->EmitterGUID->GetSystemTransform(pR->matWorld);
						SpeedVector = pR->matWorld.MulNormal(SpeedVector);
					}

					SpeedVector.y = 0.0f;
					DirAngle = SpeedVector.GetAY(pR->OldRenderAngle);
					fAngle = DirAngle;
				}
				//------------------------------------------------------------------------------------

			}

			long FrameIndexLong = fftol(FrameIndex);
			float FrameBlendK = 1.0f - (FrameIndex - FrameIndexLong);

			

			//этого if не должно быть... так как он определен на этапе компиляции...
			//------------------------------------------------------------------------------------
			if (_particleType == PTYPE_NORMAL || _particleType == PTYPE_NORMAL_XZ)
			{
				//обычние партиклы
				UV_WH1 = pR->Graph_UV->GetValue(FrameIndexLong);
				UV_WH2 = pR->Graph_UV->GetValue(FrameIndexLong+1);
			} else
			{
				//distorted партиклы
				float slide = pR->ElapsedTime * 0.2f;
				UV_WH1 = Vector4(slide+0.0f, slide+0.0f, slide+1.0f, slide+1.0f);
				UV_WH2 = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
			}







#ifndef _XBOX
			//Указатель получаем на текущую частицу
			RECT_VERTEX * pV = vertsData;

			//Считаем на следущую частицу
			vertsData += 4;

			pV[0].vRelativePos = Vector(-fSize, -fSize, 0.0f);
			pV[0].vColor = particleColor.v4;
			pV[0].tu1 = UV_WH1.v4[UV_TX1]; pV[0].tv1 = UV_WH1.v4[UV_TY1];
			pV[0].tu2 = UV_WH2.v4[UV_TX1]; pV[0].tv2 = UV_WH2.v4[UV_TY1];
			pV[0].angle = fAngle; 
			pV[0].BlendK = FrameBlendK;
			pV[0].vParticlePos = vPos;
			pV[0].AddPowerK = AddPower;
			pV[0].LightPower = LightPower;
			pV[0].AlwaysAmbientK = AmbientPower;

			if (SpeedOriented)
			{
				pV[0].angle = DirAngle;
				pV[0].vRelativePos.y *= ScaleF;
			}

			pV[1].vRelativePos = Vector(-fSize, fSize, 0.0f);
			pV[1].vColor = particleColor.v4;
			pV[1].tu1 = UV_WH1.v4[UV_TX1]; pV[1].tv1 = UV_WH1.v4[UV_TY2];
			pV[1].tu2 = UV_WH2.v4[UV_TX1]; pV[1].tv2 = UV_WH2.v4[UV_TY2];
			pV[1].angle = fAngle; 
			pV[1].BlendK = FrameBlendK;
			pV[1].vParticlePos = vPos;
			pV[1].AddPowerK = AddPower;
			pV[1].LightPower = LightPower;
			pV[1].AlwaysAmbientK = AmbientPower;

			if (SpeedOriented)
			{
				pV[1].angle = DirAngle;
				pV[1].vRelativePos.y *= ScaleF;
			}

			pV[2].vRelativePos = Vector(fSize, fSize, 0.0f);
			pV[2].vColor = particleColor.v4;
			pV[2].tu1 = UV_WH1.v4[UV_TX2]; pV[2].tv1 = UV_WH1.v4[UV_TY2];
			pV[2].tu2 = UV_WH2.v4[UV_TX2]; pV[2].tv2 = UV_WH2.v4[UV_TY2];
			pV[2].angle = fAngle; 
			pV[2].BlendK = FrameBlendK;
			pV[2].vParticlePos = vPos;
			pV[2].AddPowerK = AddPower;
			pV[2].LightPower = LightPower;
			pV[2].AlwaysAmbientK = AmbientPower;

			if (SpeedOriented)
			{
				pV[2].angle = DirAngle;
				pV[2].vRelativePos.y *= ScaleF;
			}

			pV[3].vRelativePos = Vector(fSize, -fSize, 0.0f);
			pV[3].vColor = particleColor.v4;
			pV[3].tu1 = UV_WH1.v4[UV_TX2]; pV[3].tv1 = UV_WH1.v4[UV_TY1];
			pV[3].tu2 = UV_WH2.v4[UV_TX2]; pV[3].tv2 = UV_WH2.v4[UV_TY1];
			pV[3].angle = fAngle; 
			pV[3].BlendK = FrameBlendK;
			pV[3].vParticlePos = vPos;
			pV[3].AddPowerK = AddPower;
			pV[3].LightPower = LightPower;
			pV[3].AlwaysAmbientK = AmbientPower;

			if (SpeedOriented)
			{
				pV[3].angle = DirAngle;
				pV[3].vRelativePos.y *= ScaleF;
			}


#else

			//Указатель получаем на текущую частицу
			RECT_VERTEX * __restrict pV = vertsData;

			//Считаем на следущую частицу
			vertsData++;

			float uv1_xs = (UV_WH1.v4[UV_TX2] - UV_WH1.v4[UV_TX1]) * 0.5f;
			float uv1_ys = (UV_WH1.v4[UV_TY1] - UV_WH1.v4[UV_TY2]) * 0.5f;
			float uv1_xc = UV_WH1.v4[UV_TX1] + uv1_xs;
			float uv1_yc = UV_WH1.v4[UV_TY2] + uv1_ys;

			float uv2_xs = (UV_WH2.v4[UV_TX2] - UV_WH2.v4[UV_TX1]) * 0.5f;
			float uv2_ys = (UV_WH2.v4[UV_TY1] - UV_WH2.v4[UV_TY2]) * 0.5f;
			float uv2_xc = UV_WH2.v4[UV_TX1] + uv2_xs;
			float uv2_yc = UV_WH2.v4[UV_TY2] + uv2_ys;


			pV->vRelativePos = Vector(fSize, uv1_xs, uv2_xs);
			pV->vColor = particleColor.v4;

			pV->tu1 = uv1_xc; pV->tv1 = uv1_yc;
			pV->tu2 = uv2_xc; pV->tv2 = uv2_yc;

			pV->angle = fAngle; 
			pV->BlendK = FrameBlendK;
			pV->vParticlePos = vPos;
			pV->AddPowerK = AddPower;
			pV->LightPower = LightPower;
			pV->AlwaysAmbientK = AmbientPower;

			if (SpeedOriented)
			{
				pV->angle = DirAngle;
				pV->vRelativePos.y *= ScaleF;
			}


#endif


			ParticlesCount++;

		} //Цикл по всем частицам...



		particlesNeedToRender = ParticlesCount;
		return bHaveDistortParticles;
	}


	CritSection access_to_particles;



public:

	BillBoardProcessor (); 
	~BillBoardProcessor (); 

	void DeleteDeadParticles ();


	//Из потока обновления
	void UpdateParticles (float RealDeltaTime);

	//не const SyncParams, потому что количество частиц там обновляет
	void BuildVertexBuffers (SyncParams & threadParams);


	//Из разных потоков (надо синхронизировать....)
	void AddParticle (const Vector& addVel, ParticleSystem* pSystem, const Vector& velocity_dir, const Vector& pos, const Matrix& matWorld, float EmitterTime, float EmitterLifeTime, FieldList* pFields, DWORD* pActiveCount, IEmitter* dwGUID, float fTimeScale, float fScale, bool bForceLocalMode);
	DWORD GetCount ();
	void DeleteWithGUID (IEmitter* dwGUID);
	void Clear ();





	//из основного потока
	bool Draw (const ParticleVB & renderVB, IIBuffer * pIBuffer, bool bSoftParticlesSupport);
	void DrawDistorted (const ParticleVB & renderVB, IIBuffer * pIBuffer);



	static BillBoardProcessor* singleton;
	static IRender* pRS;
	ShaderId shader_particles[3];
	ShaderId shader_particlesXZ[3];
	ShaderId shader_particles_distor[2];
	ShaderId shader_particlesXZ_distor[2];

	static CritSection draw_particles;



};


#endif