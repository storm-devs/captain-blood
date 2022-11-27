#ifndef ___MESH_INTERFACE___
#define ___MESH_INTERFACE___

#include "gmx_obb.h"
#include "meshStructure.h"
#include "..\core.h"


#include "..\render.h"



struct GMXBoundBox;
struct GMXBoundSphere;
struct boneMtxInputReadOnly;
struct boneDataReadOnly;


class ISoundScene;
class IParticleManager;
class IAnimationScene;
class IAnimation;
class IAnimationTransform;
class IPhysRigidBody;
class IPhysicsScene;

typedef void (_cdecl Object::*GMX_EVENT)();


typedef enum GMXFINDTYPE
{
	GMXFINDMETHOD_POSTFIX = 0,
	GMXFINDMETHOD_FULL = 1,
	GMXFINDMETHOD_FORCE_DWORD = 0x7fffffff
};


typedef enum _GMXENTITYTYPE
{
	GMXET_UNDEFINED = 0,
	GMXET_GEOMETRY = 1,
	GMXET_LOCATOR = 2,
	GMXET_LOCATORPARTICLES = 3,

	GMXSNT_FORCE_DWORD = 0x7fffffff
} GMXENTITYTYPE;









class IGMXScene
{
protected:

	virtual ~IGMXScene() {};

public:

	IGMXScene() {};


	virtual void ForceRelease () = 0;
	virtual bool Release () = 0;
	virtual void AddRef () = 0;



	virtual void Draw () = 0;





	virtual bool IsAnimated () = 0;
	virtual const Color& GetUserColor() = 0;
	virtual const Matrix& GetTransform () = 0;
	
	virtual IAnimationTransform* GetAnimationTransform () = 0;

	IAnimation* GetAnimation()
	{
		//FIXME !! Это только для совместимости...
		return (IAnimation*)GetAnimationTransform();
	}

	virtual bool GetDynamicLightState () = 0;
	virtual bool GetShadowReceiveState () = 0;
	virtual dword GetBlendShapesCount() = 0;






	virtual void SetTransform (const Matrix& mWorld) = 0;
	virtual void SetAnimationFile (const char* szAnimationFile) = 0;
	virtual void SetAnimation (IAnimationTransform* _pAnimation) = 0;
	virtual void SetBoneAlpha (dword dwBoneAnimationIdx, float fAlpha) = 0;
	virtual void SetBoneScale (dword dwBoneAnimationIdx, float fScale) = 0;
	virtual void SetManagers (IAnimationScene * animationScene, IParticleManager* pManager, ISoundScene* pSound) = 0;
	virtual void SetUserColor(const Color& clr) = 0;
	virtual void SetFloatAlphaReference(float alphaRef) = 0;
	virtual void SetDynamicLightState (bool bState) = 0;
	virtual void SetShadowReceiveState (bool bState) = 0;







	virtual IAnimationScene * AnimationScene() = null;
	virtual IParticleManager* Particles() = 0;
	virtual ISoundScene* Sound() = 0;



	virtual GMXHANDLE FindEntityByHashedString (GMXENTITYTYPE type, const char* szName, dword dwStringHash, dword dwLen) = 0;

	__forceinline GMXHANDLE FindEntity (GMXENTITYTYPE type, const char* szName, GMXFINDTYPE method = GMXFINDMETHOD_FULL)
	{
		dword dwLen = 0;
		dword dwHash = string::HashNoCase(szName, dwLen);

		return FindEntityByHashedString(type, szName, dwHash, dwLen);
	}



	virtual const Matrix& GetNodeLocalTransform (GMXHANDLE handle) = 0;
	virtual const Matrix& GetNodeWorldTransform(GMXHANDLE handle, Matrix & mtx) = 0;
	virtual const char* GetNodeName (GMXHANDLE handle) = 0;


	virtual void SubscribeDeletionEvent (Object* _class, GMX_EVENT _method) = 0;
	virtual void UnSubscribeDeletionEvent (Object* _class) = 0;



	virtual IPhysRigidBody* CreatePhysicsActor (IPhysicsScene & scene, bool bDynamic) = 0;




	virtual void SetBlendShapeMorphTargetWeight (dword dwMorphtargetIndex, float fWeight) = 0;


	//virtual void AnimationIsSpoil () = 0;

	virtual void SpoilAnimationCache () = 0;
	virtual void SpoilTransformationsCache () = 0;


	//Возвращает количество костей, и заполняет указатели информационная часть кости и трансформация кости...
	virtual dword GetBonesArray(boneDataReadOnly ** boneData, boneMtxInputReadOnly ** boneTransformations) = 0;

	virtual void GetAllLocators(array<GMXHANDLE> & locators) = 0;



	virtual const GMXBoundSphere& GetLocalBoundSphere() = 0;
	virtual const GMXBoundSphere& GetBoundSphere() = 0;
	virtual const GMXBoundBox& GetLocalBound() = 0;
	virtual const GMXBoundBox& GetBound() = 0;


	virtual const char* GetFileName() = 0;

};


typedef void (_cdecl Object::*GMX_EVENT)();

#define _IGMXSCENE_DECLARED_

#include "MeshStructure.h"


#endif
