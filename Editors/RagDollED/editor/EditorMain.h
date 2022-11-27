#ifndef _RAGDOLL_ED_H_
#define _RAGDOLL_ED_H_

#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\render.h"
#include "..\..\..\common_h\Controls.h"
#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\particles.h"
#include "Camera\Camera.h"

#include "..\..\..\common_h\Physics.h"

#include "..\textfile.h"
#include "..\..\..\common_h\tinyxml\tinyxml.h"

#include "..\RagDollAnimation.h"

#include "..\..\..\common_h\gmx.h"

#include "..\PrimitiveRender\Capsule.h"
#include "..\PrimitiveRender\Box.h"
#include "..\PrimitiveRender\Cone.h"
#include "..\PrimitiveRender\ConeSecktor.h"
#include "..\PrimitiveRender\Pie.h"

class RagDollED;
class TMainWindow;

extern RagDollED* pEditor;

class RGDTransformGizmo;


enum TRagDollBoneType
{
	state_capsule = 0,
	state_box
};

struct TRagDollBone
{
	Matrix mLocal;
	Matrix mGlobal;

	Matrix mWorldLocal;
	Matrix mWorldGlobal;

	TRagDollBoneType Type;	

	float  fHeight;
	float  fLenght;
	float  fWidth;

	TRagDollBone* parent;
	array<TRagDollBone*> childs;
	
	//array<ModelRagDollBone*> childs;

	int    iJointType;
	
	float  fAnchorA1,fAnchorA2;
	float  fAnchorB;

	float  fMass;
	
	float  fSpringA;
	float  fDamperA;

	float  fSpringB;
	float  fDamperB;

	string AssignedBoneName;

	IPhysBase* pPhys;

	IPhysEditableRagdoll::IBone* RagDollBone;

	TRagDollBone() : childs (_FL_)
	{
		fHeight=1.5f*0.3f;
		fLenght=0.5f*0.3f;
		fWidth=0.5f*0.3f;

		Type=state_capsule;
		
		iJointType=0;

		fAnchorA1=0;
		fAnchorA2=360;

		fAnchorB=60;

		pPhys=NULL;

		AssignedBoneName="-1";

		fMass=0.4f;
		
		fSpringA=5.5f;
		fDamperA=0.5f;

		fSpringB=0.15f;
		fDamperB=0.05f;

		RagDollBone=NULL;

		parent=NULL;
	};

	
};


struct TEnvObjects
{	
	Matrix     mTransform;	
	IGMXScene* pGeom;
	IPhysRigidBody* pPhys;
	string     ModelName;

	TEnvObjects()
	{
		pGeom = NULL;

		pPhys = NULL;

		mTransform.SetIdentity();		

		ModelName="";
	};
};

class RagDollED : public RegObject
{
	string  StartDirectory;
	TCamera Camera;

	DWORD BackgroundColor;
	DWORD GridColor;
	DWORD GridZeroLinesColor;

    IFileService* pFS;
	IRender* pRS;
	IControls* pCtrl;	

	RGDTransformGizmo* Gizmo;
				
public:	
	
	TCapsule*      pRP_Capsule;
	TBox*          pRP_Box;
	TCone*         pRP_Cone;
	TConeSecktor*  pRP_ConeSecktor;
	TPie*          pRP_Pie;

	array<TEnvObjects> EnvObjects;

	IPhysEditableRagdoll* pPhysRagdoll;

	bool bAttachBoneMode;

	float alphaBone;
	float alphaGeom;

	Vector *pVectors;

	TMainWindow* FormMain;

	IPhysics* pPhysics;

	IPhysicsScene*  pPhysScene;

	TRagDollBone RootBone;

	TRagDollBone *SelBone;	
	TRagDollBone *PreSelBone;

	RagDollAnimation* RagDollAnim;

	IAnimationService* pAnimServ;
	IAnimationScene* pAnimScene;
	IAnimationScene::Bone* pBones;	
	IAnimationProcedural* pAnim;

	bool  bSimulating;

	bool  ModelSelBone;		
	
	string GMXModelName;
	string ANTName;

	Vector           BoneLocalPoint;
	PhysRaycastId    BoneID;

	float  fDistance;
	Matrix BoneMatrix;	
	string BoneName;
	float  fLenght;

	bool   bEditEnv; 
	int    PreSelEnvObject;
	int    SelEnvObject;

	bool   bRenderDebugGeom;

	bool   bOpenedDialog;

	bool   bDragRagDollBone;

	string ProjectFileName;

	string ENVProjectFileName;

	IGMXScene* pGeom;

	virtual ~RagDollED();
	RagDollED();
	
	bool    Init();
	void    Realize(dword Delta_Time);
	void    Execute(dword Delta_Time);


	IRender* Render();
	//IParticleManager* Manager ();

	DWORD GetBackgroundColor ();
	void SetBackgroundColor (DWORD Color);

	DWORD GetGridColor ();
	void SetGridColor (DWORD Color);

	DWORD GetGridZeroColor ();
	void SetGridZeroColor (DWORD Color);


	TCamera* ArcBall ();

	const char* GetStartDir ();


	void _cdecl OnCloseManager (GUIControl* sender);

	
	void CalcBoneMatrix(TRagDollBone* Bone, bool UseWorldMatrix);

	void CalcLocalChildMatrix(TRagDollBone* Bone);

	void RenderBone(TRagDollBone* Bone, byte pow, bool UseWorldMatrix);
	
	void RenderSkeleton(TRagDollBone* Bone, bool UseWorldMatrix);
	
	void SelectBone(TRagDollBone* Bone, TRagDollBone* &SelectedBone, IPhysBase* pPhys);

	void DeleteBone(TRagDollBone* Bone, bool SelfDelete);

	void UpdateScrolls();

	void SaveSkeletonData(const char* FileName);
	void SaveBoneData(TRagDollBone* Bone,TextFile &file,int index);


	void LoadSkeletonData(const char* FileName);
	void LoadBoneData(TRagDollBone* Bone,TiXmlElement* BoneElem);
	
	void LoadEnviroment(const char* FileName);
	void SaveEnviroment(const char* FileName);		

	IPhysics* GetPhysics();


	void CreatePhys(TRagDollBone* Bone, bool IsDynamic);
	void DestroyPhys(TRagDollBone* Bone);

	void CreatePhysRagDoll(TRagDollBone* Bone);



	void CalcRagDollAnimBoneMatrix(int parent_index);

	void SkinRagDollAnimBoneMatrix(TRagDollBone* Bone);
	void SetRagDollAnimBoneMatrix(TRagDollBone* Bone);	


	void SetWorldMatrixAsLocal(TRagDollBone *Bone);
	void SetLocalMatrixAsWorld(TRagDollBone *Bone);

	float CountMass(TRagDollBone* Bone);

	//ParticleSystem* GetSystem ();

	//ParticleSystem* GetSystem ();

	IFileService* Files ();

	RGDTransformGizmo* GetGizmo ();
};

#endif