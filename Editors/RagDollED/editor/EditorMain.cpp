
#include "EditorMain.h"
#include "Forms\mainwindow.h"
#include "..\gizmo\gizmo.h"
#include "..\..\..\Common_h\InputSrvCmds.h"



#ifndef NO_TOOLS
CREATE_CLASS(RagDollED)
#endif

IGUIManager* Application = NULL;

RagDollED* pEditor = NULL;


RagDollED::RagDollED() : EnvObjects(_FL_)
{	
	pCtrl = NULL;	
	pFS = NULL;
	
	GridColor = 0xFF626262;
	BackgroundColor = 0xFF878787;
	GridZeroLinesColor = 0xFF000000;
	
	Application = NULL;
	FormMain = NULL;
	Gizmo = NULL;

	pAnimScene = null;

	bSimulating = false;
	
	int a1 = 12;
	int a2 = a1/2 + 1;

	pVectors = NEW Vector[a1*a2*6*2];

	alphaBone = 1.0f;
	alphaGeom = 1.0f;
}

RagDollED::~RagDollED()
{
	DestroyPhys(&RootBone);

	for( int i = 0 ; i < EnvObjects ; i++ )
	{			
		RELEASE(EnvObjects[i].pPhys)
		RELEASE(EnvObjects[i].pGeom)
	}

	EnvObjects.DelAll();	

	pEditor->RootBone.childs.DelAll();

	if( pGeom )
	{
		RELEASE(pGeom)

		DELETE_ARRAY(pEditor->RagDollAnim->matrices)

		pEditor->RagDollAnim->Bones.DelAll();
	}

	RELEASE(pCtrl)
	RELEASE(pAnim)

	DELETE(Gizmo)
}

RGDTransformGizmo* RagDollED::GetGizmo ()
{
	return Gizmo;
}

bool RagDollED::Init()
{


	IControlsService * ctrlsrv = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrlsrv);
	pCtrl = ctrlsrv->CreateInstance(_FL_);
			
	pCtrl->ExecuteCommand(InputSrvLockMouse(false));

	pFS = (IFileService*)api->GetService("FileService");
	Assert (pFS != NULL);

	pRS = (IRender*)api->GetService("DX9Render");
	Assert (pRS);
	pRS->SetBackgroundColor(Color(0x50505050L));


	pRP_Capsule = NEW TCapsule (pRS);
	pRP_Box = NEW TBox (pRS);
	pRP_Cone = NEW TCone (pRS);
	pRP_ConeSecktor = NEW TConeSecktor (pRS);
	pRP_Pie = NEW TPie (pRS);
	
	Matrix m;	
	Gizmo = NEW RGDTransformGizmo (RGDTransformGizmo::GT_MOVE, RGDTransformGizmo::GM_LOCAL, m);
	Gizmo->Enable(false);

	SelBone=NULL;
	PreSelBone=NULL;

	pBones = NULL;	
	pAnim = NULL;

	pAnimServ = (IAnimationService*)api->GetService("AnimationService");
	pAnimScene = pAnimServ->CreateScene(_FL_);		

	RagDollAnim = NEW RagDollAnimation;

	pPhysics = (IPhysics*)api->GetService("PhysicsService");
	Assert (pPhysics);

	pPhysScene=pPhysics->CreateScene();
	Assert (pPhysScene);
	
	pPhysRagdoll=pPhysScene->CreateEditableRagdoll();

	Matrix mat;
	mat.SetIdentity();
	mat.pos=Vector(0,-5,0);

	IPhysBox* pBox = pPhysScene->CreateBox(_FL_, Vector(400,1,400),mat,false);


	pEditor = this;
	pCtrl->EnableControlGroup ("GUI");

	
	Application = (IGUIManager *)api->CreateObject("GUIManager");
	Assert(Application);

	StartDirectory = Application->GetCurrentDir();

	FormMain = NEW TMainWindow;
	Application->ShowModal(FormMain);	
	FormMain->OnClose = (CONTROL_EVENT)&RagDollED::OnCloseManager;
	
	return true;
}

void RagDollED::UpdateScrolls()
{
	FormMain->UpdateScrolls();
}

void RagDollED::Realize(dword Delta_Time)
{
}

void RagDollED::Execute(dword Delta_Time)
{	
}

IRender* RagDollED::Render()
{
	return pRS;
}

DWORD RagDollED::GetBackgroundColor ()
{
	return BackgroundColor;
}

void RagDollED::SetBackgroundColor (DWORD Color)
{
	BackgroundColor = Color;
}

DWORD RagDollED::GetGridColor ()
{
	return GridColor;
}

void RagDollED::SetGridColor (DWORD Color)
{
	GridColor = Color;
}

DWORD RagDollED::GetGridZeroColor ()
{
	return GridZeroLinesColor;
}

void RagDollED::SetGridZeroColor (DWORD Color)
{
	GridZeroLinesColor = Color;
}

TCamera* RagDollED::ArcBall ()
{
	return &Camera;
}

const char* RagDollED::GetStartDir ()
{
	return StartDirectory.GetBuffer ();
}

void _cdecl RagDollED::OnCloseManager (GUIControl* sender)
{	
}

IPhysics *RagDollED::GetPhysics()
{
	return pPhysics;
}

void RagDollED::CalcBoneMatrix(TRagDollBone *Bone, bool UseWorldMatrix)
{
	if( Bone->parent )
	{
		if( UseWorldMatrix )
		{
			Bone->mWorldGlobal = Bone->mWorldLocal*Bone->parent->mWorldGlobal;
		}
		else
		{			
			Bone->mGlobal = Bone->mLocal*Bone->parent->mGlobal;
		}
	}
	else
	{
		if( UseWorldMatrix )
		{
			Bone->mWorldGlobal = Bone->mWorldLocal;
		}
		else
		{
			Bone->mGlobal = Bone->mLocal;
		}
	}

	for( int i = 0 ; i < Bone->childs ; i++ )
	{
		CalcBoneMatrix(Bone->childs[i],UseWorldMatrix);
	}
}

void RagDollED::CalcLocalChildMatrix(TRagDollBone *Bone)
{	
	if( Bone->parent == null )
	{
		Bone->mLocal = Bone->mGlobal;
	}

	Matrix inv_mat = Bone->mGlobal;

	inv_mat.Inverse();

	for( int i = 0 ; i < Bone->childs ; i++ )
	{
		Matrix mat = Bone->childs[i]->mGlobal;

		mat = mat*inv_mat;

		Bone->childs[i]->mLocal = mat;

		CalcLocalChildMatrix(Bone->childs[i]);
	}
}

void RagDollED::SetWorldMatrixAsLocal(TRagDollBone *Bone)
{
	Bone->mWorldGlobal = Bone->mGlobal;
	Bone->mWorldLocal  = Bone->mLocal;

	for( int i = 0 ; i < Bone->childs ; i++ )
	{
		SetWorldMatrixAsLocal(Bone->childs[i]);
	}
}

void RagDollED::SetLocalMatrixAsWorld(TRagDollBone *Bone)
{
	Bone->mGlobal = Bone->mWorldGlobal;
	Bone->mLocal  = Bone->mWorldLocal;

	for( int i = 0 ; i < Bone->childs ; i++ )
	{
		SetLocalMatrixAsWorld(Bone->childs[i]);
	}
}

void RagDollED::RenderBone(TRagDollBone* Bone, byte pow, bool UseWorldMatrix)
{	
	Matrix BoneMat;

	
	if (SelBone==Bone&&!bSimulating&&Bone->parent!=NULL)//&&!pEditor->bAttachBoneMode)
	{
		if (UseWorldMatrix)
		{
			BoneMat=Bone->mGlobal;
		}
		else
		{
			BoneMat=Bone->mWorldGlobal;
		}		

		if (Bone->iJointType==0)
		{		
		//	Matrix inv_mat = BoneMat;

		//	inv_mat.Inverse();

		//	BoneMat  = Bone->mLocal * Bone->parent->mWorldGlobal;

			pRP_Cone->Draw(0.15f, Bone->fAnchorB,0xaa0066ff,BoneMat);

			float fAngle=Bone->fAnchorB;

			fAngle+=2;

			if (fAngle>180) fAngle=180;

			pRP_ConeSecktor->Draw(0.2f, fAngle, Bone->fAnchorA1, Bone->fAnchorA2, 0x9900eeee,BoneMat);						
		}
		else
		{
			pRP_Pie->Draw(0.2f, Bone->fAnchorA1-180, Bone->fAnchorA2-180, 0x9900eeee,BoneMat);
		}		
	}

	if (UseWorldMatrix)
	{
		BoneMat=Bone->mGlobal;
	}
	else
	{
		BoneMat=Bone->mWorldGlobal;
	}
	
	{		
		if (bSimulating)
		{				
			IPhysEditableRagdoll::IBone* pRagDollBone=Bone->RagDollBone;

			pRagDollBone->GetWorldTransform(BoneMat);

			Matrix offset;
			Matrix mat;

			{
				mat.pos=Vector(0,0,0);

				Vector vShift=Vector(0,Bone->fHeight*0.5f,0);

				if (Bone->Type==state_capsule)
				{				
					float fHeight=(Bone->fHeight)*0.5f;

					if (fHeight<Bone->fLenght*0.5f) fHeight=Bone->fLenght*0.5f;

					vShift=Vector(0,fHeight,0);
				}

				mat=Bone->mWorldGlobal;

				offset.pos=vShift;
			}

			Matrix gmx_bone;

			for (dword i=0;i<RagDollAnim->Bones.Size();i++)
			{
				if (string::IsEqual(Bone->AssignedBoneName.c_str(),RagDollAnim->Bones[i].Name.c_str()))
				{
					gmx_bone=RagDollAnim->Bones[i].InitMat;					

					break;
				}
			}

			Matrix delta = mat*Matrix(gmx_bone).Inverse();

			mat = gmx_bone;

			offset = offset * delta;

			BoneMat = offset * BoneMat;
		}
		else
		{			
			BoneMat.pos=Vector(0,0,0);

			Vector vShift=Vector(0, Bone->fHeight*0.5f, 0);

			if (Bone->Type==state_capsule)
			{
				float fHeight=(Bone->fHeight)*0.5f;

				if (fHeight<Bone->fLenght*0.5f) fHeight=Bone->fLenght*0.5f;

				vShift=Vector(0,fHeight,0);
			}

			vShift=BoneMat*vShift;			

			if (UseWorldMatrix)
			{
				BoneMat=Bone->mGlobal;
			}
			else
			{
				BoneMat=Bone->mWorldGlobal;
			}

			BoneMat.pos+=vShift;
		}
	}
	
	byte pow2=byte(pow*0.75f);

	dword dwPow=pow2<<24;

	dword Color=0x00ffffff;

	if (PreSelBone==Bone)
	{
		Color=0x00ff0000;
	}
	else
	if (SelBone==Bone)
	{		
		Color=0x00ff00ff;
	}
	else
	if (SelBone!=NULL)
	{
		if (SelBone->parent==Bone)
		{
			Color=0x00ffff99;
		}
	}

	if (bSimulating) Color=0x00ffffff;

	Color = Color|dwPow;

	if(!bAttachBoneMode )
	{
		::Color col(Color);

		col.a *= alphaBone;

		Color = col;
	}

	if( FormMain->ShowSkel )
	{
		if (Bone->Type==state_box)
		{		
			{
				pRP_Box->Draw(Vector(-Bone->fWidth*0.5f,-Bone->fHeight*0.5f,-Bone->fLenght*0.5f),
							  Vector( Bone->fWidth*0.5f, Bone->fHeight*0.5f, Bone->fLenght*0.5f),
							  Color, BoneMat);

				dword dwBoxColor=0x00ffffff;

				pEditor->Render()->DrawBox(Vector(-Bone->fWidth*0.5f,-Bone->fHeight*0.5f,-Bone->fLenght*0.5f),
										   Vector( Bone->fWidth*0.5f, Bone->fHeight*0.5f, Bone->fLenght*0.5f),
										   BoneMat, dwBoxColor|dwPow);
			}
		}
		else
		if (Bone->Type==state_capsule)
		{
			float fHeight=Bone->fHeight-Bone->fLenght;

			if (fHeight<0) fHeight=0;

			pRP_Capsule->Draw(Bone->fLenght*0.5f, fHeight, Color, BoneMat);
		}	
	}		

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		RenderBone(Bone->childs[i],pow, UseWorldMatrix);
	}
}

void RagDollED::SelectBone(TRagDollBone* Bone, TRagDollBone* &SelectedBone, IPhysBase* pPhys)
{
	if (Bone->pPhys==pPhys)
	{
		SelectedBone=Bone;
	}

	if (SelectedBone!=NULL) return;

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		SelectBone(Bone->childs[i], SelectedBone, pPhys);

		if (SelectedBone!=NULL) return;
	}
}

void RagDollED::RenderSkeleton(TRagDollBone* Bone, bool UseWorldMatrix)
{	
	if (Bone->parent!=NULL)
	{
		if (UseWorldMatrix)
		{
			pEditor->Render()->DrawLine( Bone->parent->mWorldGlobal.pos, 0xffffff00, Bone->mWorldGlobal.pos, 0xffffff00);
		}
		else
		{
			pEditor->Render()->DrawLine( Bone->parent->mGlobal.pos, 0xffffff00, Bone->mGlobal.pos, 0xffffff00);
		}
	}
	
	for (dword i=0;i<Bone->childs.Size();i++)
	{
		RenderSkeleton(Bone->childs[i], UseWorldMatrix);
	}
}

void RagDollED::DeleteBone(TRagDollBone* Bone, bool SelfDelete)
{
	for (dword i=0;i<Bone->childs.Size();i++)
	{
		DeleteBone( Bone->childs[i], true);
	}

	if (Bone->parent!=NULL)
	{	
		for (dword i=0;i<Bone->parent->childs.Size();i++)
		{
			if (Bone->parent->childs[i]==Bone)
			{
				Bone->parent->childs.DelIndex(i);
			
				if (Bone->pPhys!=NULL)
				{
					Bone->pPhys->Release();
					Bone->pPhys=NULL;
				}			
			}
		}
	}

	if (SelfDelete) delete Bone;
}

void RagDollED::LoadEnviroment(const char* FileName)
{
	TiXmlDocument msr( FileName );

	bool loadOkay = msr.LoadFile();	


	TiXmlElement* EnviromentProject = msr.FirstChildElement( "EnviromentProject" );

	TiXmlElement* Elem;


	TiXmlElement* XMLEnvObject=EnviromentProject->FirstChildElement( "EnvObject" );

	while (XMLEnvObject)
	{
		TEnvObjects EnvObject;

		Elem=XMLEnvObject->FirstChildElement( "EnvModelName" );
		EnvObject.ModelName=Elem->Attribute("val");


		Elem=XMLEnvObject->FirstChildElement( "x" );
		EnvObject.mTransform.pos.x=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "y" );
		EnvObject.mTransform.pos.y=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "z" );
		EnvObject.mTransform.pos.z=(float)atof(Elem->Attribute("val"));		


		Elem=XMLEnvObject->FirstChildElement( "m0" );
		EnvObject.mTransform.matrix[0]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m1" );
		EnvObject.mTransform.matrix[1]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m2" );
		EnvObject.mTransform.matrix[2]=(float)atof(Elem->Attribute("val"));


		Elem=XMLEnvObject->FirstChildElement( "m4" );
		EnvObject.mTransform.matrix[4]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m5" );
		EnvObject.mTransform.matrix[5]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m6" );
		EnvObject.mTransform.matrix[6]=(float)atof(Elem->Attribute("val"));


		Elem=XMLEnvObject->FirstChildElement( "m8" );
		EnvObject.mTransform.matrix[8]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m9" );
		EnvObject.mTransform.matrix[9]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m10" );
		EnvObject.mTransform.matrix[10]=(float)atof(Elem->Attribute("val"));

		EnvObjects.Add(EnvObject);

		EnviromentProject->RemoveChild(XMLEnvObject);

		XMLEnvObject=EnviromentProject->FirstChildElement( "EnvObject" );
	}
}

void RagDollED::LoadSkeletonData(const char* FileName)
{
	TiXmlDocument msr( FileName );
	
	bool loadOkay = msr.LoadFile();	

	
	TiXmlElement* RagDollProject = msr.FirstChildElement( "RagDollProject" );

	TiXmlElement* Elem;


	Elem=RagDollProject->FirstChildElement( "ModelName" );
	GMXModelName=Elem->Attribute("val");	

	Elem=RagDollProject->FirstChildElement( "ANTName" );
	if (Elem) ANTName=Elem->Attribute("val");	
	

	TiXmlElement* XMLEnvObject=RagDollProject->FirstChildElement( "EnvObject" );

	while (XMLEnvObject)
	{
		TEnvObjects EnvObject;

		Elem=XMLEnvObject->FirstChildElement( "EnvModelName" );
		EnvObject.ModelName=Elem->Attribute("val");

		
		Elem=XMLEnvObject->FirstChildElement( "x" );
		EnvObject.mTransform.pos.x=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "y" );
		EnvObject.mTransform.pos.y=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "z" );
		EnvObject.mTransform.pos.z=(float)atof(Elem->Attribute("val"));		
		
		
		Elem=XMLEnvObject->FirstChildElement( "m0" );
		EnvObject.mTransform.matrix[0]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m1" );
		EnvObject.mTransform.matrix[1]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m2" );
		EnvObject.mTransform.matrix[2]=(float)atof(Elem->Attribute("val"));

		
		Elem=XMLEnvObject->FirstChildElement( "m4" );
		EnvObject.mTransform.matrix[4]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m5" );
		EnvObject.mTransform.matrix[5]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m6" );
		EnvObject.mTransform.matrix[6]=(float)atof(Elem->Attribute("val"));

		
		Elem=XMLEnvObject->FirstChildElement( "m8" );
		EnvObject.mTransform.matrix[8]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m9" );
		EnvObject.mTransform.matrix[9]=(float)atof(Elem->Attribute("val"));

		Elem=XMLEnvObject->FirstChildElement( "m10" );
		EnvObject.mTransform.matrix[10]=(float)atof(Elem->Attribute("val"));

		EnvObjects.Add(EnvObject);

		RagDollProject->RemoveChild(XMLEnvObject);

		XMLEnvObject=RagDollProject->FirstChildElement( "EnvObject" );
	}

	TiXmlElement* BoneNode = RagDollProject->FirstChildElement( "bone" );

	if (BoneNode)
	{
		LoadBoneData(&RootBone,BoneNode);
	}



}

void RagDollED::LoadBoneData(TRagDollBone* Bone,TiXmlElement* BoneElem)
{
	TiXmlElement* Elem;
	
	Elem=BoneElem->FirstChildElement( "Type" );
	int BoneType=(int)atoi (Elem->Attribute("val"));

	switch (BoneType)
	{
		case 0:
		{
			Bone->Type=state_capsule;
		}
		break;
		case 1:
		{
			Bone->Type=state_box;
		}
		break;		
	}	

	Elem=BoneElem->FirstChildElement( "Height" );
	Bone->fHeight=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "Lenght" );
	Bone->fLenght=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "Width" );
	Bone->fWidth=(float)atof (Elem->Attribute("val"));


	Elem=BoneElem->FirstChildElement( "Mass" );
	Bone->fMass=(float)atof (Elem->Attribute("val"));
	
	Elem=BoneElem->FirstChildElement( "JointType" );
	Bone->iJointType=(int)atoi(Elem->Attribute("val"));	

	Elem=BoneElem->FirstChildElement( "AnchorA1" );
	Bone->fAnchorA1=(float)atof(Elem->Attribute("val"));	

	Elem=BoneElem->FirstChildElement( "AnchorA2" );
	Bone->fAnchorA2=(float)atof(Elem->Attribute("val"));	

	Elem=BoneElem->FirstChildElement( "SpringA" );
	Bone->fSpringA=(float)atof(Elem->Attribute("val"));	
	
	Elem=BoneElem->FirstChildElement( "DamperB" );
	Bone->fDamperA=(float)atof(Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "SpringB" );
	Bone->fSpringB=(float)atof(Elem->Attribute("val"));
	
	Elem=BoneElem->FirstChildElement( "DamperB" );
	Bone->fDamperB=(float)atof(Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "AnchorB" );
	Bone->fAnchorB=(float)atof(Elem->Attribute("val"));	
		
	Elem=BoneElem->FirstChildElement( "AssignedBoneName" );	
	Bone->AssignedBoneName=Elem->Attribute("val");	

	Elem=BoneElem->FirstChildElement( "x" );
	Bone->mLocal.pos.x=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "y" );
	Bone->mLocal.pos.y=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "z" );
	Bone->mLocal.pos.z=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "m0" );
	Bone->mLocal.matrix[0]=(float)atof (Elem->Attribute("val"));
	Elem=BoneElem->FirstChildElement( "m1" );
	Bone->mLocal.matrix[1]=(float)atof (Elem->Attribute("val"));
	Elem=BoneElem->FirstChildElement( "m2" );
	Bone->mLocal.matrix[2]=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "m4" );
	Bone->mLocal.matrix[4]=(float)atof (Elem->Attribute("val"));
	Elem=BoneElem->FirstChildElement( "m5" );
	Bone->mLocal.matrix[5]=(float)atof (Elem->Attribute("val"));
	Elem=BoneElem->FirstChildElement( "m6" );
	Bone->mLocal.matrix[6]=(float)atof (Elem->Attribute("val"));

	Elem=BoneElem->FirstChildElement( "m8" );
	Bone->mLocal.matrix[8]=(float)atof (Elem->Attribute("val"));
	Elem=BoneElem->FirstChildElement( "m9" );
	Bone->mLocal.matrix[9]=(float)atof (Elem->Attribute("val"));
	Elem=BoneElem->FirstChildElement( "m10" );
	Bone->mLocal.matrix[10]=(float)atof (Elem->Attribute("val"));

	Bone->mLocal.vx.Normalize();
	Bone->mLocal.vy.Normalize();
	Bone->mLocal.vz.Normalize();
	
	if (Bone->parent!=NULL)
	{
		Bone->mGlobal= Bone->mLocal * Bone->parent->mGlobal;
	}
	else
	{
		Bone->mGlobal=Bone->mLocal;
	}


	Bone->mWorldLocal=Bone->mLocal;

	if (Bone->parent!=NULL)
	{
		Bone->mWorldGlobal= Bone->mWorldLocal * Bone->parent->mWorldGlobal;
	}
	else
	{
		Bone->mWorldGlobal=Bone->mWorldLocal;
	}


	TiXmlElement* Childs=BoneElem->FirstChildElement( "childs" );

	TiXmlElement* ChildsBone=Childs->FirstChildElement( "bone" );

	while (ChildsBone)
	{
		TRagDollBone* pChildBone = NEW TRagDollBone;
		
		pChildBone->parent=Bone;	
		Bone->childs.Add(pChildBone);

		pChildBone->mLocal.SetIdentity();
		pChildBone->mLocal.pos=Vector(3,0,0);

		//pBone->mGlobal=  pBone->mLocal * pBone->parent->mGlobal;

		LoadBoneData(pChildBone, ChildsBone);

		Childs->RemoveChild(ChildsBone);

		//ChildsBone=NULL;
		ChildsBone=Childs->FirstChildElement( "bone" );
	}
}

void RagDollED::SaveEnviroment(const char* FileName)
{
	TextFile file(FileName);		

	file.Write(0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	file.Write(1, "<EnviromentProject>\n");
	

	for (dword i=0;i<EnvObjects.Size();i++)
	{
		file.Write(2, "<EnvObject>\n");		

		file.Write(3, "<EnvModelName val=\"%s\">\n</EnvModelName>\n", EnvObjects[i].ModelName.c_str());

		file.Write(3, "<x val=\"%3.12f\">\n</x>\n", EnvObjects[i].mTransform.pos.x);
		file.Write(3, "<y val=\"%3.12f\">\n</y>\n", EnvObjects[i].mTransform.pos.y);
		file.Write(3, "<z val=\"%3.12f\">\n</z>\n", EnvObjects[i].mTransform.pos.z);

		file.Write(3, "<m0 val=\"%3.12f\">\n</m0>\n", EnvObjects[i].mTransform.matrix[0]);
		file.Write(3, "<m1 val=\"%3.12f\">\n</m1>\n", EnvObjects[i].mTransform.matrix[1]);
		file.Write(3, "<m2 val=\"%3.12f\">\n</m2>\n", EnvObjects[i].mTransform.matrix[2]);

		file.Write(3, "<m4 val=\"%3.12f\">\n</m4>\n", EnvObjects[i].mTransform.matrix[4]);
		file.Write(3, "<m5 val=\"%3.12f\">\n</m5>\n", EnvObjects[i].mTransform.matrix[5]);
		file.Write(3, "<m6 val=\"%3.12f\">\n</m6>\n", EnvObjects[i].mTransform.matrix[6]);

		file.Write(3, "<m8 val=\"%3.12f\">\n</m8>\n", EnvObjects[i].mTransform.matrix[8]);
		file.Write(3, "<m9 val=\"%3.12f\">\n</m9>\n", EnvObjects[i].mTransform.matrix[9]);
		file.Write(3, "<m10 val=\"%3.12f\">\n</m10>\n", EnvObjects[i].mTransform.matrix[10]);

		file.Write(2, "</EnvObject>\n");
	}

	file.Write(1, "</EnviromentProject>\n");
}

void RagDollED::SaveSkeletonData(const char *fileName)
{
	TextFile file(fileName);

	file.Write(0,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	file.Write(0,"<RagDollProject>\n");

	file.Write(1,"<ModelName val=\"%s\" />\n",GMXModelName.c_str());

	file.Write(1,"<ANTName val=\"%s\" />\n",ANTName.c_str());

	SaveBoneData(&RootBone,file,1);

	file.Write(0,"</RagDollProject>\n");
}

void RagDollED::SaveBoneData(TRagDollBone *Bone, TextFile &file, int index)
{
	file.Write(index,"<bone>\n");
	
	int BoneType = 0;

	if( Bone->Type == state_capsule )
	{
		BoneType = 0;
	}
	else
	if( Bone->Type == state_box )
	{
		BoneType = 1;
	}

	index++;

	file.Write(index,"<Type val=\"%i\" />\n",BoneType);

	file.Write(index,"<Height val=\"%3.3f\" />\n",Bone->fHeight);
	file.Write(index,"<Lenght val=\"%3.3f\" />\n",Bone->fLenght);
	file.Write(index,"<Width  val=\"%3.3f\" />\n",Bone->fWidth);

	file.Write(index,"<Mass val=\"%3.3f\" />\n",Bone->fMass);

	file.Write(index,"<JointType val=\"%i\" />\n",Bone->iJointType);

	file.Write(index,"<AnchorA1 val=\"%3.3f\" />\n",Bone->fAnchorA1);
	file.Write(index,"<AnchorA2 val=\"%3.3f\" />\n",Bone->fAnchorA2);

	file.Write(index,"<SpringA val=\"%3.3f\" />\n",Bone->fSpringA);
	file.Write(index,"<DamperA val=\"%3.3f\" />\n",Bone->fDamperA);

	file.Write(index,"<SpringB val=\"%3.3f\" />\n",Bone->fSpringB);
	file.Write(index,"<DamperB val=\"%3.3f\" />\n",Bone->fDamperB);

	file.Write(index,"<AnchorB val=\"%3.3f\" />\n",Bone->fAnchorB);

	file.Write(index,"<AssignedBoneName val=\"%s\" />\n",Bone->AssignedBoneName.c_str());	

	file.Write(index,"<x val=\"%3.12f\" />\n",Bone->mLocal.pos.x);
	file.Write(index,"<y val=\"%3.12f\" />\n",Bone->mLocal.pos.y);
	file.Write(index,"<z val=\"%3.12f\" />\n",Bone->mLocal.pos.z);

	file.Write(index,"<m0 val=\"%3.12f\" />\n",Bone->mLocal.matrix[0]);
	file.Write(index,"<m1 val=\"%3.12f\" />\n",Bone->mLocal.matrix[1]);
	file.Write(index,"<m2 val=\"%3.12f\" />\n",Bone->mLocal.matrix[2]);

	file.Write(index,"<m4 val=\"%3.12f\" />\n",Bone->mLocal.matrix[4]);
	file.Write(index,"<m5 val=\"%3.12f\" />\n",Bone->mLocal.matrix[5]);
	file.Write(index,"<m6 val=\"%3.12f\" />\n",Bone->mLocal.matrix[6]);

	file.Write(index,"<m8 val=\"%3.12f\" />\n",Bone->mLocal.matrix[8]);
	file.Write(index,"<m9 val=\"%3.12f\" />\n",Bone->mLocal.matrix[9]);
	file.Write(index,"<m10 val=\"%3.12f\" />\n",Bone->mLocal.matrix[10]);

	file.Write(index,"<childs>\n");

	index++;

	for( int i = 0 ; i < Bone->childs ; i++ )
	{
		SaveBoneData(Bone->childs[i],file,index);
	}

	index--;

	file.Write(index,"</childs>\n");

	index--;
	
	file.Write(index,"</bone>\n");
}

void RagDollED::CreatePhys(TRagDollBone* Bone, bool IsDynamic)
{	
	if (Bone->pPhys!=NULL)
	{
		Bone->pPhys->Release();
		Bone->pPhys=NULL;
	}

	Matrix mat=Bone->mWorldGlobal;

	{
		mat.pos=Vector(0,0,0);

		Vector vShift=Vector(0,Bone->fHeight*0.5f,0);

		vShift=mat*vShift;
	
		mat=Bone->mWorldGlobal;

		mat.pos+=vShift;
	}

	/*Vector scale = mat.GetScale();
	scale = Vector(1.0f/scale.x,1.0f/scale.y,1.0f/scale.z);
	mat = mat * scale;*/

	mat.vx.Normalize();
	mat.vy.Normalize();
	mat.vz.Normalize();

	if (Bone->Type==state_capsule)
	{
		float fHeight=Bone->fHeight-Bone->fLenght;

		if (fHeight<0.01f) fHeight=0.01f;			

		Bone->pPhys = (IPhysBase*)pPhysScene->CreateCapsule(_FL_, Bone->fLenght*0.5f,fHeight, mat,IsDynamic);
	}
	else
	if (Bone->Type==state_box)
	{
		Bone->pPhys = (IPhysBase*)pPhysScene->CreateBox(_FL_, Vector(Bone->fWidth,Bone->fHeight,Bone->fLenght),mat,true);
	}

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		CreatePhys(Bone->childs[i],IsDynamic);
	}
}

void RagDollED::CreatePhysRagDoll(TRagDollBone* Bone)
{
	Matrix mat=Bone->mWorldGlobal;
	
	Matrix offset;
	
	{
		mat.pos=Vector(0,0,0);

		Vector vShift=Vector(0,Bone->fHeight*0.5f,0);

		if (Bone->Type==state_capsule)
		{				
			float fHeight=(Bone->fHeight)*0.5f;

			if (fHeight<Bone->fLenght*0.5f) fHeight=Bone->fLenght*0.5f;

			vShift=Vector(0,fHeight,0);
		}

		mat=Bone->mWorldGlobal;

		offset.pos=vShift;
	}

	Matrix gmx_bone,gmx_bone2;

	Matrix WorldOffeset = pEditor->RootBone.mGlobal;

	WorldOffeset.Inverse();

	WorldOffeset = WorldOffeset * pEditor->RootBone.mWorldGlobal;
	
	for (dword i=0;i<RagDollAnim->Bones.Size();i++)
	{
		if (string::IsEqual(Bone->AssignedBoneName.c_str(),RagDollAnim->Bones[i].Name.c_str()))
		{
			gmx_bone=RagDollAnim->Bones[i].InitMat * WorldOffeset;

			//gmx_bone2=Matrix(gmx_bone).Inverse();

			break;
		}
	}

	Matrix delta = mat*Matrix(gmx_bone).Inverse();
          
	mat = gmx_bone;
    
    offset = offset * delta;              


	if (Bone->Type==state_capsule)
	{
		float fHeight=Bone->fHeight-Bone->fLenght;		

		if (fHeight<0.01f) fHeight=0.01f;

		Matrix tmp_mat;

		tmp_mat.SetIdentity();
		tmp_mat.RotateX(90*PI/180);
		
		Matrix IdentityMat;
		IdentityMat.SetIdentity();

		Bone->RagDollBone->SetShape(mat,offset, fHeight,Bone->fLenght*0.5f, Bone->fMass);				
	}
	else
	if (Bone->Type==state_box)
	{
		Matrix IdentityMat;
		IdentityMat.SetIdentity();

		Bone->RagDollBone->SetShape(mat,offset,Vector(Bone->fWidth,Bone->fHeight,Bone->fLenght), Bone->fMass);		
	}

	Bone->RagDollBone->SetBoneName(Bone->AssignedBoneName);

	

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		Bone->childs[i]->RagDollBone=&Bone->RagDollBone->AddChild();		

		CreatePhysRagDoll(Bone->childs[i]);

		if (Bone->childs[i]->iJointType==0)
		{
			IPhysEditableRagdoll::SphericalJointParams Params;
			
			Matrix m1=Bone->childs[i]->mGlobal;			

			m1.pos=Vector(0,0,0);

			Params.swingAxisInParentSystem = Vector(0,1,0) * m1;
			Params.swingAxisInParentSystem.Normalize();

			Params.worldJointPosition=Vector(Bone->childs[i]->mWorldGlobal.pos);

			
			Params.swingLimit=Bone->childs[i]->fAnchorB*PI/180;
				
			Params.swingSpring=Bone->childs[i]->fSpringB;
			Params.swingDamper=Bone->childs[i]->fDamperB;		



			float fStartAngle=Bone->childs[i]->fAnchorA1;
			float fEndAngle=Bone->childs[i]->fAnchorA2;

			if (fStartAngle>fEndAngle) fStartAngle-=360;

			float fCenterAngle=fStartAngle+(fEndAngle-fStartAngle)*0.5f;

			Params.twistAxisInParentSystem = Vector(cos(fCenterAngle*PI/180), 0, sin(fCenterAngle*PI/180)) * m1;
			Params.twistAxisInParentSystem.Normalize();
				
			Params.twistMin=(fStartAngle-fCenterAngle)*PI/180;
			Params.twistMax=(fEndAngle-fCenterAngle)*PI/180;

			Params.twistSpring=Bone->fSpringA;
			Params.twistDamper=Bone->fDamperA;		

			Bone->RagDollBone->CreateJoint(*Bone->childs[i]->RagDollBone, Params);
		}
		else
		{
			IPhysEditableRagdoll::RevoluteJointParams Params;

			Params.worldJointPosition=Vector(Bone->childs[i]->mWorldGlobal.pos);
			
			

			float fStartAngle=Bone->childs[i]->fAnchorA1;
			float fEndAngle=Bone->childs[i]->fAnchorA2;

			if (fStartAngle>fEndAngle) fStartAngle-=360;

			float fCenterAngle=fStartAngle+(fEndAngle-fStartAngle)*0.5f;

			
			Matrix tmp_mat=Bone->childs[i]->mGlobal;

			tmp_mat.pos=Vector(0,0,0);

			Params.axisInParentSystem = Vector(1,0,0) * tmp_mat;
			Params.axisInParentSystem.Normalize();

			Params.normalInParentSystem = Vector(0,1, 0) * tmp_mat;
			Params.normalInParentSystem.Normalize();
			
			

			Params.minAngle= (fStartAngle-180)*PI/180;
			Params.maxAngle= (fEndAngle-180)*PI/180;

			Params.spring=Bone->fSpringA;
			Params.damper=Bone->fDamperA;

			Bone->RagDollBone->CreateJoint(*Bone->childs[i]->RagDollBone, Params);
		}
	}	
}

void RagDollED::DestroyPhys(TRagDollBone* Bone)
{	
	if (Bone->pPhys!=NULL)
	{
		Bone->pPhys->Release();
		Bone->pPhys=NULL;
	}
	
	for (dword i=0;i<Bone->childs.Size();i++)
	{
		DestroyPhys(Bone->childs[i]);
	}
}

float RagDollED::CountMass(TRagDollBone* Bone)
{
	float mass=0;

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		mass+=CountMass(Bone->childs[i]);
	}

	return mass+Bone->fMass;
}

void RagDollED::CalcRagDollAnimBoneMatrix(int parent_index)
{
	for (dword i=0;i<RagDollAnim->Bones.Size();i++)
	{		
		if (RagDollAnim->Bones[i].parent==parent_index)
		{
			if (RagDollAnim->Bones[i].NeedCalclFinalMat)
			{			
				if (parent_index!=-1)
				{
					RagDollAnim->Bones[i].FinalMat=RagDollAnim->Bones[i].LocalMat * RagDollAnim->Bones[parent_index].FinalMat;
				}
				else
				{
					RagDollAnim->Bones[i].FinalMat=RagDollAnim->Bones[i].LocalMat;
				}
			}

			CalcRagDollAnimBoneMatrix(i);
		}
	}
}

void RagDollED::SkinRagDollAnimBoneMatrix(TRagDollBone* Bone)
{
	Matrix mat=Bone->mGlobal;

	mat.Inverse();

	for (dword i=0;i<RagDollAnim->Bones.Size();i++)
	{
		if (string::IsEqual(Bone->AssignedBoneName.c_str(),RagDollAnim->Bones[i].Name.c_str()))
		{
			RagDollAnim->Bones[i].NeedCalclFinalMat=false;

			RagDollAnim->Bones[i].SkinMat = RagDollAnim->Bones[i].InitMat * mat;
		}
		else RagDollAnim->Bones[i].NeedCalclFinalMat=false;
	}

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		SkinRagDollAnimBoneMatrix(Bone->childs[i]);
	}
}

void RagDollED::SetRagDollAnimBoneMatrix(TRagDollBone* Bone)
{
	for (dword i=0;i<RagDollAnim->Bones.Size();i++)
	{
		if (string::IsEqual(Bone->AssignedBoneName.c_str(),RagDollAnim->Bones[i].Name.c_str()))
		{
			RagDollAnim->Bones[i].NeedCalclFinalMat=false;

			IPhysEditableRagdoll::IBone* pRagDollBone=Bone->RagDollBone;

			pRagDollBone->GetWorldTransform(RagDollAnim->Bones[i].FinalMat);
			
			RagDollAnim->Bones[i].FinalMat = RagDollAnim->Bones[i].FinalMat;
			
			break;
		}		
	}

	for (dword i=0;i<Bone->childs.Size();i++)
	{
		SetRagDollAnimBoneMatrix(Bone->childs[i]);
	}
}

IFileService* RagDollED::Files ()
{
	return pFS;
}