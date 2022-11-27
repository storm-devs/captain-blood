/********************************************************************
	created:	2009 dec
	author:		Sergey Makeev
	purpose:	mesh instance class
*********************************************************************/
#ifndef _MESH_INSTANCE_CLASS_
#define _MESH_INSTANCE_CLASS_


#include "..\..\common_h\render.h"
#include "..\..\common_h\templates.h"
#include "..\..\common_h\gmxservice\igmx_scene.h"
//#include "..\..\common_h\ishadows.h"


#include "..\..\common_h\gmxservice\MeshStructure.h"
#include "mesh_data.h"

class IShadowsService;
class IAnimationScene;
class IAnimationTransform;
class MeshContainer;
class MeshInstance;
class IParticleSystem;
class GMXBlendShapeMixer;

typedef void (__fastcall MeshInstance::*DRAW_FUNCTION)();


struct MeshParticleOnLocator
{
	IParticleSystem* pSystem;
	GMXHANDLE pMasterLocator;
};

class MeshInstance : public IGMXScene
{

protected:

	//static[0], static_lighted[1], static_custom_shader[2], static_custom_shader_lighted[3]
	//animated[4], animated_lighted[5], animated_custom_shader[6], animated_custom_shader_lighted[7]
	static DRAW_FUNCTION drawFunctions[8];
	static dword bDrawFunctionsInited;

	boneGpuData * pBonesGPUdata;
	long * pGeomToAnim_TranslateTable;
	long * pAnimToGeom_TranslateTable;
	long * pDynamicTransofrmsBonesID;
	Matrix * pDynamicTransofrmsBonesMatrixCache;
	MeshParticleOnLocator * pParticlesPtrCache;
	float * pMorphTargetsWeights;
	long * pAnimToGeomBlendShapes_TranslateTable;



	byte localBoundBoxStorage[sizeof(GMXBoundBox) + 16];
	GMXBoundBox * localBoundBoxAligned;

	byte localBoundSphereStorage[sizeof(GMXBoundSphere) + 16];
	GMXBoundSphere * localBoundSphereAligned;


	byte globalBoundBoxStorage[sizeof(GMXBoundBox) + 16];
	GMXBoundBox * globalBoundBoxAligned;

	byte globalBoundSphereStorage[sizeof(GMXBoundSphere) + 16];
	GMXBoundSphere * globalBoundSphereAligned;




	struct SubscribeStruct
	{
		Object* object;
		GMX_EVENT event;
	};
	array <SubscribeStruct> subscribedToDeletionEvent;

	int findSubscribedClass (Object* _class);

	int refCount;

	IAnimationTransform * pAnimation;

	IRender* pRS;
	IAnimationScene * pAnimManager;
	IParticleManager* pParticleManager;
	ISoundScene* pSoundManager;

	GMXBlendShapeMixer * blendShapeMixer;

	const header & msh;

	void SendSkeletonToGPU(subsetAnim * subset, const TTABLEITEM * pTTableLocalToGlobal, dword dwTTableSize) const;
	void UpdateAnimation(IAnimationTransform * pAnim) const;
	void UpdateTransformations(IAnimationTransform * pAnim) const;


	void ResetAllMorphTargets();
	void ResetReverseTranslateTables();
	

	void CreateBlendShapesIfNeed();

#ifndef STOP_DEBUG
	void debugDrawBlendShapes();
#endif

public:


	MeshInstance(IRender* renderService, MeshContainer * _container, const char * debugMeshName, const char* pSourceFile, int Line,
		         boneGpuData * _pBonesGPUdata, long * _pGeomToAnim_TranslateTable, long * _pAnimToGeom_TranslateTable,
				 long * _pDynamicTransofrmsBonesID, Matrix * _pDynamicTransofrmsBonesMatrixCache,
				 MeshParticleOnLocator* _pParticlesPtrCache,
				 float * _pMorphTargetsWeights, long * _pAnimToGeomBlendShapes_TranslateTable);


	virtual ~MeshInstance();


	void CreateAnimation(IAnimationScene * pAnimScene, const char * fileName);


public:
	static IVariable * texStage0;
	static IVariable * texStage1;
	static IVariable * texStage2;
	static IVariable * texStage3;

	static IVariable* pShadowColorVar;
	static IVariable* pShadowVar;
	static IShadowsService* shadowSRV;

#ifndef _XBOX
	static IVariable* skinBonesVar;
#endif



public:

	virtual void ForceRelease ();
	virtual bool Release ();
	virtual void AddRef ();

	virtual void Draw ();

	virtual bool IsAnimated ();
	virtual const Color& GetUserColor();
	virtual const Matrix& GetTransform ();
	virtual IAnimationTransform* GetAnimationTransform ();
	virtual bool GetDynamicLightState ();
	virtual bool GetShadowReceiveState ();
	virtual dword GetBlendShapesCount();






	virtual void SetTransform (const Matrix& mWorld);
	virtual void SetAnimationFile (const char* szAnimationFile);
	virtual void SetAnimation (IAnimationTransform* _pAnimation);
	virtual void SetBoneAlpha (dword dwBoneAnimationIdx, float fAlpha);
	virtual void SetBoneScale (dword dwBoneAnimationIdx, float fScale);
	virtual void SetManagers (IAnimationScene * animationScene, IParticleManager* pManager, ISoundScene* pSound);
	virtual void SetUserColor(const Color& clr);
	virtual void SetFloatAlphaReference(float alphaRef);
	virtual void SetDynamicLightState (bool bState);
	virtual void SetShadowReceiveState (bool bState);







	virtual IAnimationScene * AnimationScene();
	virtual IParticleManager* Particles();
	virtual ISoundScene* Sound();




	virtual GMXHANDLE FindEntityByHashedString (GMXENTITYTYPE type, const char* szName, dword dwStringHash, dword dwLen);


	virtual IPhysRigidBody* CreatePhysicsActor (IPhysicsScene & scene, bool bDynamic);




	virtual void SetBlendShapeMorphTargetWeight (dword dwMorphtargetIndex, float fWeight);


	virtual void SpoilAnimationCache ();
	virtual void SpoilTransformationsCache ();

	//Возвращает количество костей, и заполняет указатели информационная часть кости и трансформация кости...
	virtual dword GetBonesArray(boneDataReadOnly ** boneData, boneMtxInputReadOnly ** boneTransformations);

	virtual void GetAllLocators(array<GMXHANDLE> & locators);


	virtual const Matrix& GetNodeLocalTransform (GMXHANDLE handle);
	virtual const Matrix& GetNodeWorldTransform(GMXHANDLE handle, Matrix & mtx);
	virtual const char* GetNodeName (GMXHANDLE handle);

	virtual void SubscribeDeletionEvent (Object* _class, GMX_EVENT _method);
	virtual void UnSubscribeDeletionEvent (Object* _class);




	virtual const GMXBoundSphere& GetLocalBoundSphere();
	virtual const GMXBoundSphere& GetBoundSphere();
	virtual const GMXBoundBox& GetLocalBound();
	virtual const GMXBoundBox& GetBound();

	virtual const char* GetFileName();



	void CreateParticles();
	void DeleteParticles();
	void UpdateParticles();
	void SendDeleteNotifiaction();


protected:

#ifndef STOP_DEBUG
	string debugName;
#endif


	MeshContainer * container;

	float sceneAlphaRef;
	Color clrUser;
	Matrix mtxWorld;

	DWORD bDynamicLight;
	DWORD bFrustumCull;
	DWORD bShadowReceive;

protected:

	const char* szSourceFile;
	int iSourceLine;

	MeshInstance * pPrev;
	MeshInstance * pNext;

public:

	MESH_INLINE const char * getSourceFile()
	{
		return szSourceFile;
	}

	MESH_INLINE int getSourceLine()
	{
		return iSourceLine;
	}


	MESH_INLINE MeshInstance * next()
	{
		return pNext;
	}

	MESH_INLINE MeshInstance * prev()
	{
		return pPrev;
	}


private:

	mutable dword dwLocalBSUpdateCacheIdx;
	mutable dword dwAnimationUpdateCacheIdx;
	mutable dword dwTransformationsUpdateCacheIdx;
	dword dwCurrentDrawFunc;
	void SelectDrawFunction();

	template <bool templateDynamicLight, bool templateUseCustomShaders>
	void __fastcall DrawAnimated()
	{
		Assert(msh.bAnimated);


		UpdateAnimation(pAnimation);
		UpdateTransformations(pAnimation);
		UpdateParticles();

		setIndices(msh);

		GMXOBB worldOBB;


		renderOrder * __restrict order = msh.pRenderOrders.ptr;
		dword dwOrdersCount = msh.dwOrdersCount;
		for (dword i = 0; i < dwOrdersCount; i++, order++)
		{
			subsetAnim * __restrict subset = (subsetAnim *)order->pSubset.ptr;

			SetTextures(&subset->tex[0]);

			setAnimStream0(msh, subset->offsetInStream0InBytes);

			for (dword p = 0; p < subset->dwPassesCount; p++)
			{
				worldOBB.SetFromMinMax(localBoundBoxAligned->vMin, localBoundBoxAligned->vMax);
				worldOBB.TransformInplace(mtxWorld);

				subSubsetAnim * subSub = (subset->pSubSubsets.ptr + p);

				setAnimStream1(msh, subSub->offsetInStream1InBytes);

				SendSkeletonToGPU(subset, subSub->pTTableLocalToGlobal.ptr, subSub->dwTranslateTableSize);

				//все условия из темплейтных параметров - оптимизатор удалит
				//--------------------------------------------------------------------------------
				if (templateUseCustomShaders)
				{
					IGMXService::RenderModeShaderId * RMode = GMXService::pGS->GetRenderMode_Fast();
					IGMXService::RenderModeShaderId * RModeNoAlpha = GMXService::pGS->GetRenderModeNoAlpha_Fast();
					dword dwBonesInSkin = subset->dwBonesInSkin;
					ShaderLightingId * customShader = &RMode->bones[dwBonesInSkin];
					if (subset->isNoAlpha)
					{
						customShader = &RModeNoAlpha->bones[dwBonesInSkin];
					}

					/* custom шейдеры всегда как static light рисуются

					if (bDynamicLight)
					{
						pRS->DrawIndexedPrimitiveLighted(*customShader, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount, worldOBB);
					} else
					{
						pRS->DrawIndexedPrimitive(customShader->off, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount);
					}
					*/

					pRS->DrawIndexedPrimitive(customShader->off, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount);

				} else
				{

#ifndef STOP_DEBUG
					IGMXService::HackMode hm = GMXService::pGS->GetHackMode();
					if (hm != GMXService::HM_OFF)
					{
						ShaderLightingId debugShader = GMXService::pGS->getDebugShaderByIndex(hm).bones[subset->dwBonesInSkin];
						pRS->DrawIndexedPrimitive(debugShader.off, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount);
					} else
					{
						if (templateDynamicLight)
						{
							pRS->DrawIndexedPrimitiveLighted(subset->tech, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount, worldOBB);
						} else
						{
							pRS->DrawIndexedPrimitive(subset->tech.off, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount);
						}
					}
#else
					if (templateDynamicLight)
					{
						pRS->DrawIndexedPrimitiveLighted(subset->tech, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount, worldOBB);
					} else
					{
						pRS->DrawIndexedPrimitive(subset->tech.off, PT_TRIANGLELIST, 0, subset->numVertexes, subSub->offsetInIndexBufferInInds, subSub->triCount);
					}
#endif
				}
				//--------------------------------------------------------------------------------

			}
		}
	}



	template <bool templateDynamicLight, bool templateUseCustomShaders>
	void __fastcall DrawStatic()
	{
		Assert(!msh.bAnimated);

		UpdateParticles();

		setIndices(msh);

		GMXOBB worldOBB;

		pRS->SetStreamSource(1, NULL, 0);
		renderOrder * __restrict order = msh.pRenderOrders.ptr;
		dword dwOrdersCount = msh.dwOrdersCount;
		for (dword i = 0; i < dwOrdersCount; i++, order++)
		{
			subsetStatic * __restrict subset = (subsetStatic *)order->pSubset.ptr;

			SetTextures(&subset->tex[0]);

			setStaticStream(msh, subset->offsetInVertexBufferInBytes);

			worldOBB.SetFromMinMax(subset->min, subset->max);
			worldOBB.TransformInplace(mtxWorld);


			//все условия из темплейтных параметров - оптимизатор удалит
			//--------------------------------------------------------------------------------
			if (templateUseCustomShaders)
			{
				IGMXService::RenderModeShaderId * RMode = GMXService::pGS->GetRenderMode_Fast();
				IGMXService::RenderModeShaderId * RModeNoAlpha = GMXService::pGS->GetRenderModeNoAlpha_Fast();
				dword dwBonesInSkin = subset->dwBonesInSkin;
				ShaderLightingId * customShader = &RMode->bones[dwBonesInSkin];
				if (subset->isNoAlpha)
				{
					customShader = &RModeNoAlpha->bones[dwBonesInSkin];
				}

				/* custom шейдеры всегда как static light рисуются
				if (bDynamicLight)
				{
					pRS->DrawIndexedPrimitiveLighted(*customShader, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount, worldOBB);
				} else
				{
					pRS->DrawIndexedPrimitive(customShader->off, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount);
				}
				*/

				pRS->DrawIndexedPrimitive(customShader->off, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount);
			} else
			{
#ifndef STOP_DEBUG
				IGMXService::HackMode hm = GMXService::pGS->GetHackMode();
				if (hm != GMXService::HM_OFF)
				{
					ShaderLightingId debugShader = GMXService::pGS->getDebugShaderByIndex(hm).bones[subset->dwBonesInSkin];
					pRS->DrawIndexedPrimitive(debugShader.off, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount);
				} else
				{
					if (templateDynamicLight)
					{
						pRS->DrawIndexedPrimitiveLighted(subset->tech, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount, worldOBB);
					} else
					{
						pRS->DrawIndexedPrimitive(subset->tech.off, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount);
					}
				}

#else
				if (templateDynamicLight)
				{
					pRS->DrawIndexedPrimitiveLighted(subset->tech, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount, worldOBB);
				} else
				{
					pRS->DrawIndexedPrimitive(subset->tech.off, PT_TRIANGLELIST, 0, subset->numVertexes, subset->offsetInIndexBufferInInds, subset->triCount);
				}
#endif
			}
			//--------------------------------------------------------------------------------



		}
	}



};

#endif 