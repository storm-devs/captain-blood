
#include "mainwindow.h"
//#include "rename.h"
//#include "projectbrowser.h"
#include "..\EditorMain.h"
#include "..\..\..\..\common_h\particles.h"
#include "..\..\..\..\Common_h\FileService.h"
#include "..\..\..\..\common_h\gmx.h"
#include "..\Camera\Camera.h"


//#include "..\..\..\GMXService\gmx_mesh.h"
//#include "..\..\..\GMXService\gmx_anim_mesh_subset.h"


//#include "choosename.h"


#include "..\..\gizmo\gizmo.h"

#define ZERO_TEXTURE_NAME "not_found"


#define WINDOW_POSX   0
#define WINDOW_POSY   0
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define SAVEDOK_SHOWTIME 0.3f
#define SAVEDOK_MESSAGE "File, saved complete"


extern char IniStringBuffer[];
extern IGUIManager* Application;
extern RagDollED* pEditor;


class MyPanel : public GUIControl
{
public:

	MyPanel(GUIControl *pParent, int Left, int Top, int Width, int Height) : GUIControl(pParent)
	{
		DrawRect.Left = Left;
		DrawRect.Top = Top;
		DrawRect.Width = Width;
		DrawRect.Height = Height;

		ClientRect = DrawRect;
	}

	virtual void Draw()
	{
	//	GUIHelper::Draw2DRect(DrawRect.Left + 1,DrawRect.Top + 2,DrawRect.Width + 1,DrawRect.Height - 1,0x10000000);
	//	GUIHelper::Draw2DRect(DrawRect.Left + 2,DrawRect.Top + 1,DrawRect.Width - 1,DrawRect.Height + 1,0x10000000);
		GUIHelper::Draw2DRect(DrawRect.Left + 1,DrawRect.Top + 1,DrawRect.Width,DrawRect.Height,0x18000000);

		Color col(0xffd4d0c8);

		col.r *= 1.05f;
		col.g *= 1.05f;
		col.b *= 1.05f;

		GUIHelper::Draw2DRect(DrawRect.Left,DrawRect.Top,DrawRect.Width,DrawRect.Height,col);

		GUIControl::Draw();
	}

};


#include "..\..\..\anxeditor\ant.h"

void _cdecl TMainWindow::OnLoadSceletonFromAnt(GUIControl* sender)
{	
	pEditor->bOpenedDialog=true;

	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->Filters.Add (".ant");
	fo->FiltersDesc.Add ("Animation sequnce");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadSceletonFromAntEvent;
	fo->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	fo->StartDirectory = string(string (pEditor->GetStartDir())).GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TMainWindow::LoadSceletonFromAntEvent(GUIControl* sender)
{
	GUIFileOpen* fo = (GUIFileOpen*)sender;
	
	string CropPath = string(pEditor->GetStartDir()) + "\\resource\\models\\";
	pEditor->ANTName = fo->FileName;
	pEditor->ANTName.GetRelativePath(CropPath);

	LoadSceletonFromAnt(pEditor->ANTName);

	pEditor->bOpenedDialog=false;
}

void TMainWindow::LoadSceletonFromAnt(string FileName)
{
	pEditor->ANTName = FileName;

	IFileService* pFS;

	pFS=pEditor->Files ();

	ILoadBuffer * loadBuffer = pFS->LoadData(FileName, _FL_);
	if(!loadBuffer) return;

	dword size = loadBuffer->Size();
	const byte * data = loadBuffer->Buffer();
		
	if (size==0)
	{
		loadBuffer->Release();
		return;
	}

	pEditor->RagDollAnim->Bones.DelAll();	

	AntFileHeader & hdr = *(AntFileHeader *)data;
	if(size < sizeof(hdr))
	{
		throw "Invalide file size";
	}
	

	const byte* ptr;
	
	ptr = data + sizeof(AntFileHeader) + hdr.stringsTableSize;
	AntFileBone* Bones= (AntFileBone*) ptr;

	ptr = data + sizeof(AntFileHeader) + hdr.stringsTableSize + sizeof(AntFileBone) * hdr.bonesCount;
	AntFileTrackElement* KeyFrames = (AntFileTrackElement*) ptr;


	for (dword i=0;i<hdr.bonesCount;i++)
	{		
		Matrix mat;

		Quaternion rot(KeyFrames[i*hdr.framesCount].qx,KeyFrames[i*hdr.framesCount].qy,KeyFrames[i*hdr.framesCount].qz,KeyFrames[i*hdr.framesCount].qw);

		rot.GetMatrix(mat);		

		mat.pos = Vector(KeyFrames[i*hdr.framesCount].px/* * KeyFrames[i*hdr.framesCount].sx*/,
						 KeyFrames[i*hdr.framesCount].py/* * KeyFrames[i*hdr.framesCount].sy*/,
						 KeyFrames[i*hdr.framesCount].pz/* * KeyFrames[i*hdr.framesCount].sz*/);
		
		RDBone RBone;

		//mat.Inverse();

		RBone.InitMat=mat;

		ptr = data + sizeof(AntFileHeader) + Bones[i].nameIndex;

		char * BoneName;

		BoneName = (char*)ptr;
		RBone.Name = BoneName;			

		pEditor->RagDollAnim->Bones.Add(RBone);
	}

	loadBuffer->Release();
	loadBuffer = null;
	data = null;
	
	//pEditor->RagDollAnim->matrices= NEW Matrix[pEditor->RagDollAnim->Bones.Size()];

	int index = 0;
	int nedeed_parent=-1;

	for (dword i=0;i<pEditor->RagDollAnim->Bones.Size()-1;i++)
	{
		for (dword j=index;j<pEditor->RagDollAnim->Bones.Size();j++)
		{
			int parent=	pEditor->RagDollAnim->GetBoneParent(j);

			if (parent==nedeed_parent)
			{
				RDBone tmp_bone = pEditor->RagDollAnim->Bones[j];
				pEditor->RagDollAnim->Bones[j] = pEditor->RagDollAnim->Bones[index];
				pEditor->RagDollAnim->Bones[index] = tmp_bone;

				index++;
			}				
		}

		nedeed_parent++;
	}

	if (pEditor->pBones)
	{
		delete []pEditor->pBones;
		pEditor->pBones = NULL;
	}

	if (pEditor->pAnim)
	{	
		pEditor->pAnim->Release();
		pEditor->pAnim = null;
	}

	pEditor->pBones = NEW IAnimationScene::Bone[pEditor->RagDollAnim->Bones.Size()];

	for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
	{
		int parent=	pEditor->RagDollAnim->GetBoneParent(i);

		if (parent!=-1)
		{
			pEditor->RagDollAnim->Bones[i].InitMat = pEditor->RagDollAnim->Bones[i].InitMat * pEditor->RagDollAnim->Bones[parent].InitMat;
			
			Matrix inv_mat=pEditor->RagDollAnim->Bones[parent].InitMat;

			inv_mat.Inverse();

			pEditor->RagDollAnim->Bones[i].LocalMat=pEditor->RagDollAnim->Bones[i].InitMat * inv_mat;
		}
		else
		{
			pEditor->RagDollAnim->Bones[i].LocalMat=pEditor->RagDollAnim->Bones[i].InitMat;
		}

		pEditor->RagDollAnim->Bones[i].parent=parent;

		pEditor->RagDollAnim->Bones[i].NeedCalclFinalMat=true;

		for (int j=0;j<pEditor->RagDollAnim->NumOptBones;j++)
		{
			if (string::IsEqual(pEditor->RagDollAnim->OptBonesNames[j].Name.c_str(), pEditor->RagDollAnim->Bones[i].Name))
			{
				pEditor->RagDollAnim->matrices[j]=pEditor->RagDollAnim->Bones[i].InitMat;

				break;
			}
		}

		pEditor->pBones[i].name = pEditor->RagDollAnim->Bones[i].Name;	
		pEditor->pBones[i].parentIndex=parent;
		pEditor->pBones[i].mtx = pEditor->RagDollAnim->Bones[i].InitMat;
		pEditor->pBones[i].mtx.pos = pEditor->RagDollAnim->Bones[i].LocalMat.pos;
	}	

	pEditor->pAnim = pEditor->pAnimScene->CreateProcedural(pEditor->pBones,pEditor->RagDollAnim->Bones.Size(), _FL_);

	if (pEditor->pGeom)
	{
		pEditor->pGeom->SetAnimation(pEditor->pAnim);
	}
}

static void SetCaption(const char *text)
{
	static char buf[512];

	if( string::NotEmpty(text))
	{
		sprintf_s(buf,sizeof(buf)," Ragdoll Editor - %s",text);
	}
	else
	{
		sprintf_s(buf,sizeof(buf)," Ragdoll Editor");
	}

	static HWND h = (HWND)api->Storage().GetLong("system.hwnd");

	SetWindowText(h,buf);
}

TMainWindow::TMainWindow () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{	
	pEditor->pGeom = NULL;

	pEditor->bOpenedDialog=false;

	pEditor->bAttachBoneMode=false;

	pEditor->bDragRagDollBone=false;

	pEditor->bEditEnv=true;

	pEditor->bRenderDebugGeom=false;

	pEditor->GMXModelName="";
	pEditor->ANTName="";

	pEditor->PreSelEnvObject=-1;
	pEditor->SelEnvObject=-1;

	pEditor->ProjectFileName="";

	pEditor->ENVProjectFileName="";

	ShowGeom = false;
	ShowSkel = true;

	GZM_move_index = 0;
	GZM_rotate_index = 1;

	SaveOKTime = 0;

	const RENDERSCREEN& ScreenInfo = pEditor->Render()->GetScreenInfo3D();
	SetWidth(ScreenInfo.dwWidth);
	SetHeight(ScreenInfo.dwHeight);


	bPopupStyle = true;
	bSystemButton = false;
	Caption = "RagDoll Editor";
	pFont->SetSize (16);

	pEditor->RootBone.mLocal.SetIdentity();
	pEditor->RootBone.mLocal.pos=Vector(0,0,0);
	pEditor->RootBone.parent=NULL;

	pEditor->SelBone=&pEditor->RootBone;

	pEditor->DestroyPhys(&pEditor->RootBone);	
	pEditor->CreatePhys(&pEditor->RootBone,false);

	pEditor->GetGizmo()->SetTransform(pEditor->RootBone.mLocal);
	pEditor->GetGizmo()->CalcGizmoScale();
	

	ViewPortWidth  = DrawRect.w - 230;
	ViewPortHeight = DrawRect.h -  15; //(int)(ViewPortWidth/1.8f);

	ViewPortHeight += 2;

	ViewPort = NEW GUIViewPort(this,7,5,ViewPortWidth,ViewPortHeight);
	ViewPort->OnRender = (CONTROL_EVENT)&TMainWindow::DrawViewport;


	MyPanel *Panel;

	Panel = NEW MyPanel(this,DrawRect.w - 215,7,209,64);


	btnNewRagDoll = NEW GUIButton(Panel,5,5,24,24);
	btnNewRagDoll->Glyph->Load ("reditor\\create");
	btnNewRagDoll->FontColor = 0xFF000000;
	btnNewRagDoll->pFont->SetSize (16);
	btnNewRagDoll->Hint = "Create new RagDoll";
	btnNewRagDoll->FlatButton = true;
	btnNewRagDoll->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnCreateNewRagDoll;

	btnOpenRagDoll= NEW GUIButton(Panel,5 + 27,5,24,24);
	btnOpenRagDoll->Glyph->Load ("reditor\\open");
	btnOpenRagDoll->FontColor = 0xFF000000;
	btnOpenRagDoll->pFont->SetSize (16);
	btnOpenRagDoll->Hint = "Open existing RagDoll";
	btnOpenRagDoll->FlatButton = true;
	btnOpenRagDoll->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnOpenRagDoll;	

	btnSaveRagDoll= NEW GUIButton(Panel,5 + 27*2,5,24,24);
	btnSaveRagDoll->Glyph->Load ("reditor\\save");
	btnSaveRagDoll->DisabledGlyph->Load ("disabledsave");
	btnSaveRagDoll->FontColor = 0xFF000000;
	btnSaveRagDoll->pFont->SetSize (16);
	btnSaveRagDoll->Hint = "Save RagDoll";
	btnSaveRagDoll->FlatButton = true;
	//btnSaveParticleSystem->bEnabled = false;
	btnSaveRagDoll->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SavePressed;

	btnSaveAsRagDoll= NEW GUIButton(Panel,5 + 27*3,5,24,24);
	btnSaveAsRagDoll->Glyph->Load ("reditor\\saveas");
	btnSaveAsRagDoll->FontColor = 0xFF000000;
	btnSaveAsRagDoll->pFont->SetSize (16);
	btnSaveAsRagDoll->Hint = "Save as...";
	btnSaveAsRagDoll->FlatButton = true;
	btnSaveAsRagDoll->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SaveAsPressed;

	btnExportToBin= NEW GUIButton(Panel,5 + 27*6,5,24,24);
	btnExportToBin->Glyph->Load ("reditor\\selectback"/*"reditor\\ExportProject"*/);
	btnExportToBin->FontColor = 0xFF000000;
	btnExportToBin->pFont->SetSize (16);
	btnExportToBin->Hint = "Export";
	btnExportToBin->FlatButton = true;
	btnExportToBin->OnMousePressed = (CONTROL_EVENT)&TMainWindow::ExportPressed;

	btnBoneAdd = NEW GUIButton(Panel,5 + 27*0,5 + 30,24,24);
	btnBoneAdd->Glyph->Load ("reditor\\addpoint");	
	btnBoneAdd->FontColor = 0xFF000000;
	btnBoneAdd->pFont->SetSize (16);
	btnBoneAdd->Hint = "Create Bone";
	btnBoneAdd->FlatButton = true;
	//button->GroupIndex = 33;
	//button->Down = true;
	btnBoneAdd->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnCreateBone;

	btnBoneDelete = NEW GUIButton(Panel,5 + 27*1,5 + 30,24,24);
	btnBoneDelete->Glyph->Load ("reditor\\delete");
	btnBoneDelete->FontColor = 0xFF000000;
	btnBoneDelete->pFont->SetSize (16);
	btnBoneDelete->Hint = "Delete bone";
	btnBoneDelete->FlatButton = true;
	btnBoneDelete->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnDeleteBone;	

	btnLoadGeom = NEW GUIButton(Panel,5 + 27*3,5 + 30,24,24);
	btnLoadGeom->Glyph->Load ("reditor\\addmodeld");
	btnLoadGeom->FontColor = 0xFF000000;
	btnLoadGeom->pFont->SetSize (16);
	btnLoadGeom->Hint = "Load GMX file";
	btnLoadGeom->FlatButton = true;
	btnLoadGeom->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnLoadGMXPressed;

	btnLoadSceletone = NEW GUIButton(Panel,5 + 27*4,5 + 30,24,24);
	btnLoadSceletone->Glyph->Load ("reditor\\slowmo");
	btnLoadSceletone->FontColor = 0xFF000000;
	btnLoadSceletone->pFont->SetSize (16);
	btnLoadSceletone->Hint = "Load Sceletone";
	btnLoadSceletone->FlatButton = true;
	btnLoadSceletone->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnLoadSceletonFromAnt;


	Panel = NEW MyPanel(this,DrawRect.w - 215,7 + 90,209,64);


	btnStartPhisSimulation = NEW GUIButton(Panel,5 + 27*0,5,24,24);
	btnStartPhisSimulation->Glyph->Load ("reditor\\restart");
	btnStartPhisSimulation->FontColor = 0xFF000000;
	btnStartPhisSimulation->pFont->SetSize (16);
	btnStartPhisSimulation->Hint = "Start Simulation";
	btnStartPhisSimulation->FlatButton = true;
	btnStartPhisSimulation->OnMousePressed = (CONTROL_EVENT)&TMainWindow::StartPhisSimulatione;

	btnStopPhisSimulation = NEW GUIButton(Panel,5 + 27*1,5,24,24);
	btnStopPhisSimulation->Glyph->Load ("reditor\\pause");
	btnStopPhisSimulation->FontColor = 0xFF000000;
	btnStopPhisSimulation->pFont->SetSize (16);
	btnStopPhisSimulation->Hint = "Stop Simulation";
	btnStopPhisSimulation->FlatButton = true;
	btnStopPhisSimulation->OnMousePressed = (CONTROL_EVENT)&TMainWindow::StopPhisSimulation;
	
	btnShowGeom = NEW GUIButton(Panel,5 + 27*3,5,24,24);
	btnShowGeom->FontColor = 0xFF000000;
	btnShowGeom->Glyph->Load ("reditor\\slowmo");
	btnShowGeom->pFont->SetSize (16);
	btnShowGeom->Hint = "Hide/Show GMX Model";
	btnShowGeom->FlatButton = true;
	//btnShowGeom->
	btnShowGeom->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnShowGMXModel;

	btnToAttachBoneMode = NEW GUIButton(Panel,5 + 27*4,5,24,24);
	btnToAttachBoneMode->FontColor = 0xFF000000;
	btnToAttachBoneMode->Glyph->Load ("reditor\\graph");
	btnToAttachBoneMode->pFont->SetSize (16);
	btnToAttachBoneMode->Hint = "To Attach Mode";
	btnToAttachBoneMode->FlatButton = true;
	//btnShowGeom->
	btnToAttachBoneMode->OnMousePressed = (CONTROL_EVENT)&TMainWindow::ToAttachBoneMode;

	btnShowDebugGeom = NEW GUIButton(Panel,5 + 27*6,5,24,24);
	btnShowDebugGeom->FontColor = 0xFF000000;
	btnShowDebugGeom->Glyph->Load ("reditor\\show_dir");
	btnShowDebugGeom->pFont->SetSize (16);
	btnShowDebugGeom->Hint = "Show Debug Geom";
	btnShowDebugGeom->FlatButton = true;
	//btnShowGeom->
	btnShowDebugGeom->OnMousePressed = (CONTROL_EVENT)&TMainWindow::ShowDebugGeom;

	btnMoveMode = NEW GUIButton(Panel,5 + 27*0,5 + 30,24,24);
	btnMoveMode->Glyph->Load ("reditor\\m_move");
	btnMoveMode->FontColor = 0xFF000000;
	btnMoveMode->pFont->SetSize (16);
	btnMoveMode->Hint = "Move geometry mode";
	btnMoveMode->FlatButton = true;
	btnMoveMode->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnMovePressed;
	btnMoveMode->GroupIndex = 33;
	btnMoveMode->Down=true;

	btnRotateMode = NEW GUIButton(Panel,5 + 27*1,5 + 30,24,24);
	btnRotateMode->Glyph->Load ("reditor\\m_rotate");
	btnRotateMode->FontColor = 0xFF000000;
	btnRotateMode->pFont->SetSize (16);
	btnRotateMode->Hint = "Rotate geometry mode";
	btnRotateMode->FlatButton = true;
	btnRotateMode->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnRotatePressed;
	btnRotateMode->GroupIndex = 33;

/*	bGizmoMode = NEW GUIComboBox(Panel,5 + 27*2 + 15,5 + 33,100,16);
	bGizmoMode->ListBox->Items.Add("World");
	bGizmoMode->ListBox->Items.Add("Local");
	bGizmoMode->MakeFlat(true);
	bGizmoMode->OnChange = (CONTROL_EVENT)&TMainWindow::btnGizmoTypeChanged;
	bGizmoMode->SelectItem(0);*/

	btnWorld = NEW GUIButton(Panel,5 + 27*3,5 + 30,24,24);
//	btnWorld->Glyph->Load("reditor\\m_move");
	btnWorld->Caption = "W";
	btnWorld->FontColor = 0xff000000;
	btnWorld->pFont->SetSize(16);
	btnWorld->Hint = "World transform";
	btnWorld->FlatButton = true;
	btnWorld->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnWorldPressed;
	btnWorld->GroupIndex = 66;
	btnWorld->Down = true;

	btnLocal = NEW GUIButton(Panel,5 + 27*4,5 + 30,24,24);
//	btnLocal->Glyph->Load("reditor\\m_rotate");
	btnLocal->Caption = "L";
	btnLocal->FontColor = 0xff000000;
	btnLocal->pFont->SetSize(16);
	btnLocal->Hint = "Local tranform";
	btnLocal->FlatButton = true;
	btnLocal->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnLocalPressed;
	btnLocal->GroupIndex = 66;


	Panel = NEW MyPanel(this,DrawRect.w - 215,7 + 180,209,34 + 30);


	btnNewEnv= NEW GUIButton(Panel,5,5,24,24);
	btnNewEnv->Glyph->Load ("reditor\\create");
	btnNewEnv->FontColor = 0xFF000000;
	btnNewEnv->pFont->SetSize (16);
	btnNewEnv->Hint = "Create new Enviroment";
	btnNewEnv->FlatButton = true;
	btnNewEnv->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnCreateNewEnv;

	btnOpenEnv= NEW GUIButton(Panel,5 + 27,5,24,24);
	btnOpenEnv->Glyph->Load ("reditor\\open");
	btnOpenEnv->FontColor = 0xFF000000;
	btnOpenEnv->pFont->SetSize (16);
	btnOpenEnv->Hint = "Open existing Enviroment";
	btnOpenEnv->FlatButton = true;
	btnOpenEnv->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnOpenEnv;	

	btnSaveEnv= NEW GUIButton(Panel,5 + 27*2,5,24,24);
	btnSaveEnv->Glyph->Load ("reditor\\save");
	btnSaveEnv->DisabledGlyph->Load ("disabledsave");
	btnSaveEnv->FontColor = 0xFF000000;
	btnSaveEnv->pFont->SetSize (16);
	btnSaveEnv->Hint = "Save Enviromnet";
	btnSaveEnv->FlatButton = true;
	//btnSaveParticleSystem->bEnabled = false;
	btnSaveEnv->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SaveEnvPressed;

	btnSaveAsEnv= NEW GUIButton(Panel,5 + 27*3,5,24,24);
	btnSaveAsEnv->Glyph->Load ("reditor\\saveas");
	btnSaveAsEnv->FontColor = 0xFF000000;
	btnSaveAsEnv->pFont->SetSize (16);
	btnSaveAsEnv->Hint = "Save as...";
	btnSaveAsEnv->FlatButton = true;
	btnSaveAsEnv->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SaveAsEnvPressed;
	
	btnAddEnvObj = NEW GUIButton(Panel,5 + 27*0,5 + 30,24,24);
	btnAddEnvObj->Glyph->Load ("reditor\\addmpart");	
	btnAddEnvObj->FontColor = 0xFF000000;
	btnAddEnvObj->pFont->SetSize (16);
	btnAddEnvObj->Hint = "Add Env Object";
	btnAddEnvObj->FlatButton = true;
	//button->GroupIndex = 33;
	//button->Down = true;
	btnAddEnvObj->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnAddEnvObject;

	btnDelEnvObj = NEW GUIButton(Panel,5 + 27*1,5 + 30,24,24);
	btnDelEnvObj->Glyph->Load("reditor\\delete");
	btnDelEnvObj->FontColor = 0xFF000000;
	btnDelEnvObj->pFont->SetSize (16);
	btnDelEnvObj->Hint = "Delete Env Object";
	btnDelEnvObj->FlatButton = true;
	btnDelEnvObj->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnDeleteEnvObject;

	btnLoadEnvGeom = NEW GUIButton(Panel,5 + 27*3,5 + 30,24,24);
	btnLoadEnvGeom->Glyph->Load ("reditor\\m_gmx");
	btnLoadEnvGeom->FontColor = 0xFF000000;
	btnLoadEnvGeom->pFont->SetSize (16);
	btnLoadEnvGeom->Hint = "Load Env GMX file";
	btnLoadEnvGeom->FlatButton = true;
	btnLoadEnvGeom->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnLoadEnvGMXPressed;

	btnAutoLook = NEW GUIButton(Panel,5 + 27*4,5 + 30,24,24);
	btnAutoLook->FontColor = 0xFF000000;
//	btnAutoLook->Glyph->Load ("reditor\\slowmo");
	btnAutoLook->Caption = "A";
	btnAutoLook->pFont->SetSize (16);
	btnAutoLook->Hint = "Auto camera on/off";
	btnAutoLook->FlatButton = true;
	btnAutoLook->OnMousePressed = (CONTROL_EVENT)&TMainWindow::AutoLookPressed;

	btnExit = NEW GUIButton(Panel,5 + 27*6,5/* + 30*/,24,24);
	btnExit->Glyph->Load("reditor\\exit");
	btnExit->FontColor = 0xFF000000;
	btnExit->pFont->SetSize(16);
	btnExit->Hint = "Exit";
	btnExit->FlatButton = true;
	btnExit->OnMousePressed = (CONTROL_EVENT)&TMainWindow::ExitPressed;


	dword off = 3;

	int GraphEditorHeight = (int)(DrawRect.h*0.5f) - 20 - 38;

	Panel = NEW MyPanel(this,DrawRect.w - 215,7 + 270,209,GraphEditorHeight + off - 24 + 3/* - 25*/);


	btnShowSkel = NEW GUIButton(Panel,5 + 27*6,5,24,24);
	btnShowSkel->FontColor = 0xFF000000;
	btnShowSkel->Glyph->Load ("reditor\\slowmo");
	btnShowSkel->pFont->SetSize (16);
	btnShowSkel->Hint = "Hide/Show Skeleton";
	btnShowSkel->FlatButton = true;
	btnShowSkel->OnMousePressed = (CONTROL_EVENT)&TMainWindow::ShowSkeletonPressed;

	btnDrawOrder = NEW GUIButton(Panel,5 + 27*5,5,24,24);
	btnDrawOrder->FontColor = 0xFF000000;
//	btnDrawOrder->Glyph->Load ("reditor\\slowmo");
	btnDrawOrder->Caption = "O";
	btnDrawOrder->pFont->SetSize (16);
	btnDrawOrder->Hint = "Change draw order";
	btnDrawOrder->FlatButton = true;
	btnDrawOrder->OnMousePressed = (CONTROL_EVENT)&TMainWindow::DrawOrderPressed;

	btnDrawPower = NEW GUIButton(Panel,5 + 27*4,5,24,24);
	btnDrawPower->FontColor = 0xFF000000;
//	btnDrawPower->Glyph->Load ("reditor\\slowmo");
	btnDrawPower->Caption = "T";
	btnDrawPower->pFont->SetSize (16);
	btnDrawPower->Hint = "Transparent draw";
	btnDrawPower->FlatButton = true;
	btnDrawPower->OnMousePressed = (CONTROL_EVENT)&TMainWindow::DrawPowerPressed;


	bCapsuleBoneType = NEW GUIRadioButton2(Panel,5,5 + 25,100,24,true);
//	bCapsuleBoneType->ImageChecked->Load("reditor\\scheckbox_ch");
//	bCapsuleBoneType->ImageNormal->Load("reditor\\scheckbox_uch");
	bCapsuleBoneType->OnClick=(CONTROL_EVENT)&TMainWindow::OnChangeBoneType;
	bCapsuleBoneType->Caption="Capsule";

	bBoxBoneType = NEW GUIRadioButton2(Panel,5 + 60,5 + 25,100,24,true);
//	bBoxBoneType->ImageChecked->Load("reditor\\scheckbox_ch");
//	bBoxBoneType->ImageNormal->Load("reditor\\scheckbox_uch");
	bBoxBoneType->OnClick=(CONTROL_EVENT)&TMainWindow::OnChangeBoneType;
	bBoxBoneType->Caption="Box";

	off += 1;

	pScrollBoneHeight = NEW GUIScrollBar2(Panel,GUISBKIND_Horizontal,5,6 + 50,150,19);
	pScrollBoneHeight->Min=0;
	pScrollBoneHeight->Max=100;
	pScrollBoneHeight->MakeFlat(true);
	pScrollBoneHeight->OnChange = (CONTROL_EVENT)&TMainWindow::OnChangeBoneHeihgt;

	pScrollBoneLenght = NEW GUIScrollBar2(Panel,GUISBKIND_Horizontal,5,6 + 75,150,19);
	pScrollBoneLenght->Min=0;
	pScrollBoneLenght->Max=100;
	pScrollBoneLenght->MakeFlat(true);
	pScrollBoneLenght->OnChange = (CONTROL_EVENT)&TMainWindow::OnChangeBoneLenght;

	pScrollBoneWidth = NEW GUIScrollBar2(Panel,GUISBKIND_Horizontal,5,6 + 100,150,19);
	pScrollBoneWidth->Min=0;
	pScrollBoneWidth->Max=100;
	pScrollBoneWidth->MakeFlat(true);
	pScrollBoneWidth->OnChange = (CONTROL_EVENT)&TMainWindow::OnChangeBoneWidth;

	off -= 24;

	pBoneMass = NEW GUIEdit2(Panel,/*10 + 50*/5 + 115,5 + 125/* - 24*/,60,24);
	pBoneMass->IsNumerical=true;
	pBoneMass->OnlyInteger=false;
	pBoneMass->Flat = true;
	pBoneMass->OnChange=(CONTROL_EVENT)&TMainWindow::OnChangePhysParams;

	labelMass = NEW GUILabel(Panel,5 + 60,5 + 125/* - 24*/,54,24);
	labelMass->Caption="Mass";
	labelMass->Layout=GUILABELLAYOUT_Left;

	off += 1;
//	off -= 25;

	bSphericalJointType = NEW GUIRadioButton2(Panel,5,5 + 150 + 25 + off,100,24,true);
//	bSphericalJointType->ImageChecked->Load("reditor\\scheckbox_ch");
//	bSphericalJointType->ImageNormal->Load("reditor\\scheckbox_uch");
	bSphericalJointType->OnClick=(CONTROL_EVENT)&TMainWindow::OnChangeJointType;
	bSphericalJointType->Caption="Spherical";// Joint";
	bSphericalJointType->GroupID=1;

	bRevoltJointType = NEW GUIRadioButton2(Panel,5 + 70,5 + 175 + off,100,24,true);
//	bRevoltJointType->ImageChecked->Load("reditor\\scheckbox_ch");
//	bRevoltJointType->ImageNormal->Load("reditor\\scheckbox_uch");
	bRevoltJointType->OnClick=(CONTROL_EVENT)&TMainWindow::OnChangeJointType;
	bRevoltJointType->Caption="Revolt";// Joint";
	bRevoltJointType->GroupID=1;

	off += 2;

	SecktorAnchorA = NEW GUISecktor(Panel,5,5 + 200 + off,54,54);
	SecktorAnchorA->SetType(2);
	SecktorAnchorA->SetNeedDrag(true);

	labelSpring = NEW GUILabel(Panel,5 + 60,5 + 205 + off,54,24);
	labelSpring->Caption="Spring";
	labelSpring->Layout=GUILABELLAYOUT_Left;

	pBoneSpringA = NEW GUIEdit2(Panel,5 + 115,5 + 205 + off,60,24);
	pBoneSpringA->IsNumerical=true;
	pBoneSpringA->OnlyInteger=false;
	pBoneSpringA->Flat = true;
	pBoneSpringA->OnChange=(CONTROL_EVENT)&TMainWindow::OnChangePhysParams;

	labelDumper = NEW GUILabel(Panel,5 + 60,5 + 230 - 2 + off,54,24);
	labelDumper->Caption="Dumper";
	labelDumper->Layout=GUILABELLAYOUT_Left;

	pBoneDamperA = NEW GUIEdit2(Panel,5 + 115,5 + 230 - 2 + off,60,24);
	pBoneDamperA->IsNumerical=true;
	pBoneDamperA->OnlyInteger=false;
	pBoneDamperA->Flat = true;
	pBoneDamperA->OnChange=(CONTROL_EVENT)&TMainWindow::OnChangePhysParams;

	SecktorAnchorB = NEW GUISecktor(Panel,5,5 + 260 + off,54,54);
	SecktorAnchorB->SetType(1);
	SecktorAnchorB->SetNeedDrag(false);

	pLabelSpringB = NEW GUILabel(Panel,5 + 60,5 + 265 + off,54,24);
	pLabelSpringB->Caption="Spring";
	pLabelSpringB->Layout=GUILABELLAYOUT_Left;

	pBoneSpringB = NEW GUIEdit2(Panel,5 + 115,5 + 265 + off,60,24);
	pBoneSpringB->IsNumerical=true;
	pBoneSpringB->OnlyInteger=false;
	pBoneSpringB->Flat = true;
	pBoneSpringB->OnChange=(CONTROL_EVENT)&TMainWindow::OnChangePhysParams;

	pLabelDamperB = NEW GUILabel(Panel,5 + 60,5 + 290 - 2 + off,54,24);
	pLabelDamperB->Caption="Dumper";
	pLabelDamperB->Layout=GUILABELLAYOUT_Left;

	pBoneDamperB = NEW GUIEdit2(Panel,5 + 115,5 + 290 - 2 + off,60,24);
	pBoneDamperB->IsNumerical=true;
	pBoneDamperB->OnlyInteger=false;
	pBoneDamperB->Flat = true;
	pBoneDamperB->OnChange=(CONTROL_EVENT)&TMainWindow::OnChangePhysParams;

	BottomPanel = Panel;

	//rbutton->

/*	Secktor = NEW GUISecktor(this, DrawRect.w-(5+210)+25, (int)(DrawRect.h*0.5f)+150, 54, 54);
	Secktor->SetType(0);
	Secktor->SetNeedDrag(false);

	Secktor = NEW GUISecktor(this, DrawRect.w-(5+210)+25, (int)(DrawRect.h*0.5f)+270, 54, 54);
	Secktor->SetType(2);
	Secktor->SetNeedDrag(true);

	Secktor = NEW GUISecktor(this, DrawRect.w-(5+210)+25, (int)(DrawRect.h*0.5f)+210, 54, 54);
	Secktor->SetType(1);
	Secktor->SetNeedDrag(true);*/

	//LoadGMX ("\\resource\\models\\blood_new.gmx");

	UpdateScrolls();

	pEditor->GetGizmo()->Enable(true);
	pEditor->GetGizmo()->SetType(RGDTransformGizmo::GT_MOVE);

//	bGizmoMode->SelectItem(GZM_move_index);

	btnWorld->Down = GZM_move_index == 0;
	btnLocal->Down = GZM_move_index == 1;

	pEditor->GetGizmo()->SetMode((RGDTransformGizmo::GizmoMode)GZM_move_index);

	drawOrder = false;
	autoLook  = false;

	SetCaption("");

	api_exit = 0;
}

TMainWindow::~TMainWindow()
{
}

void TMainWindow::Draw()
{
	btnDelEnvObj->bEnabled = btnDelEnvObj->Enabled = !pEditor->bSimulating && pEditor->SelEnvObject >= 0;

	//////////////////

	GUIWindow::Draw();

	//////////////////

	GUIButton *bt = btnDelEnvObj; GUIRectangle r; const dword color = 0x40ffffff;

	if(!bt->bEnabled )
	{
		r = bt->GetDrawRect(); bt->ClientToScreen(r);

		GUIHelper().DrawSprite(r.x + (r.w - 16)/2 - 1,r.y + (r.h - 16)/2,16,16,bt->Glyph,color);
	}

	bt = btnNewRagDoll;

	if(!bt->bEnabled )
	{
		r = bt->GetDrawRect(); bt->ClientToScreen(r);

		GUIHelper().DrawSprite(r.x + (r.w - 16)/2 - 1,r.y + (r.h - 16)/2,16,16,bt->Glyph,color);
	}

	bt = btnBoneAdd;

	if(!bt->bEnabled )
	{
		r = bt->GetDrawRect(); bt->ClientToScreen(r);

		GUIHelper().DrawSprite(r.x + (r.w - 16)/2 - 1,r.y + (r.h - 16)/2,16,16,bt->Glyph,color);
	}

	bt = btnBoneDelete;

	if(!bt->bEnabled )
	{
		r = bt->GetDrawRect(); bt->ClientToScreen(r);

		GUIHelper().DrawSprite(r.x + (r.w - 16)/2 - 1,r.y + (r.h - 16)/2,16,16,bt->Glyph,color);
	}

	bt = btnExportToBin;

	if(!bt->bEnabled )
	{
		r = bt->GetDrawRect(); bt->ClientToScreen(r);

		GUIHelper().DrawSprite(r.x + (r.w - 16)/2 - 1,r.y + (r.h - 16)/2,16,16,bt->Glyph,color);
	}

	////////////////////////

	dword back = 0xffd4d0c8;

	GUIHelper::DrawHorizLine(1,DrawRect.Width,1,back);

	GUIHelper::DrawHorizLine(0,DrawRect.Width,DrawRect.Height - 1,back);
	GUIHelper::DrawHorizLine(0,DrawRect.Width,DrawRect.Height - 2,back);

	GUIHelper::DrawVertLine(1,DrawRect.Height,1,back);

	GUIHelper::DrawVertLine(0,DrawRect.Height,DrawRect.Width - 1,back);
	GUIHelper::DrawVertLine(0,DrawRect.Height,DrawRect.Width - 2,back);

	if(!pEditor->SelBone )
		return;

	dword col = 0xff404040;

	int val;
	int off;

	val = int(100*pEditor->SelBone->fHeight);
	off = 0;

	if( val >  9 ) off++;
	if( val > 99 ) off++;

	pFont->Print(966 + off,333,col,"%3dcm",val);

	val = int(100*pEditor->SelBone->fLenght);
	off = 0;

	if( val >  9 ) off++;
	if( val > 99 ) off++;

	pFont->Print(966 + off,358,col,"%3dcm",val);

	if(!pScrollBoneWidth->Visible )
		return;

	val = int(100*pEditor->SelBone->fWidth);
	off = 0;

	if( val >  9 ) off++;
	if( val > 99 ) off++;

	pFont->Print(966 + off,383,col,"%3dcm",val);
}

void TMainWindow::KeyPressed(int Key, bool bSysKey)
{
	GUIWindow::KeyPressed (Key, bSysKey);
	
	if (!IsActive) return;

	bool bControlState = false;
	bool bAltState = false;
	bool bShiftState = false;
	if (GetAsyncKeyState(VK_CONTROL) < 0) bControlState = true;
	if (GetAsyncKeyState(VK_MENU) < 0) bAltState = true;
	if (GetAsyncKeyState(VK_SHIFT) < 0) bShiftState = true;


	if (GetAsyncKeyState('P') < 0)
	{
		pEditor->bAttachBoneMode=!pEditor->bAttachBoneMode;

		if (!pEditor->bAttachBoneMode)
		{
			pEditor->CalcLocalChildMatrix(&pEditor->RootBone);

			pEditor->SetWorldMatrixAsLocal(&pEditor->RootBone);
		}

		if (pEditor->SelBone!=NULL)
		{
			if (pEditor->bAttachBoneMode)
			{
				pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mGlobal);
				pEditor->GetGizmo()->CalcGizmoScale();
			}
			else
			{
				pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mWorldGlobal);
				pEditor->GetGizmo()->CalcGizmoScale();
			}
		}		

		Sleep(150);	
	}


	/*if (GetAsyncKeyState('Q') < 0)
	{
		if (pEditor->bDoTransformInGlobal) pEditor->CalcLocalChildMatrix(&pEditor->RootBone);

		pEditor->bDoTransformInGlobal=!pEditor->bDoTransformInGlobal;		
		Sleep(150);			
	}*/

	
	/*if (GetAsyncKeyState('Q') < 0)
	{
		btnSimpleMode->OnPressed();
		Sleep(100);		
	}
	if (GetAsyncKeyState('W') < 0)
	{
		btnMoveMode->OnPressed();
		Sleep(100);		
	}
	if (GetAsyncKeyState('E') < 0)
	{
		btnRotateMode->OnPressed();
		Sleep(100);		
	}


	if (GetAsyncKeyState(VK_F6) < 0)
	{
		//api->Trace("F6 - pressed\n");
		RenameObjectPressed (this);
		Sleep(120);
	}


	if (GetAsyncKeyState(VK_RETURN) < 0)
	{
		ApplyChangeSystem ();
	}

	if (bAltState)
	{
		if (GetAsyncKeyState('P') < 0)
		{
			CreatePointEmitterPressed (NULL);
			Sleep (150);
		}
		if (GetAsyncKeyState('B') < 0)
		{
			CreateBillboardPressed (NULL);
			Sleep (150);
		}
		if (GetAsyncKeyState('M') < 0)
		{
			CreateModelPressed (NULL);
			Sleep (150);
		}
	}


	if (bControlState)
	{

		if (GetAsyncKeyState('P') < 0)
		{
			PausePressed(NULL);
			Sleep (80);
		}

		if (GetAsyncKeyState('O') < 0)
		{
			SlowMotionPressed(NULL);
			Sleep (80);
		}

		if (GetAsyncKeyState('V') < 0)
		{
			Sleep (80);
			RestartSystem(NULL);
		}

		if (GetAsyncKeyState(VK_DELETE) < 0)
		{
			DeleteObjectPressed(NULL);
			Sleep (80);
		}

		if (GetAsyncKeyState('S') < 0)
		{
			SavePressed (NULL);
			Sleep (80);
		}

		if (GetAsyncKeyState('Z') < 0)
		{
			Undo ();
			Sleep (150);
		}
		if (GetAsyncKeyState('Y') < 0)
		{
			Redo ();
			Sleep (150);
		}
	}*/
}


string GetPrnt(const char* BoneName, int Depth)
{
	string TempName;
	TempName = BoneName;
	dword len = TempName.Size();
	if (len == 0) return string();

	len--;
	
	for (dword n = len; n > 0; n--)
	{
		if (TempName[n] == '|')
		{
			TempName.Delete(n, len-n+1);

			len = TempName.Size();

			Depth--;

			if (Depth<0) return TempName;			
		}
	}

	return "-1";
}


void _cdecl TMainWindow::DrawViewport (GUIControl* sender)
{
	bool shift_pressed = GetAsyncKeyState(VK_SHIFT) < 0;

//	if( pEditor->bSimulating )
//	{
		float dt = api->GetDeltaTime();

	/*	if( GetAsyncKeyState('Y') < 0 )
			dt *= 0.50f;
		if( GetAsyncKeyState('U') < 0 )
			dt *= 0.25f;*/

		pEditor->pPhysScene->Update(dt);
//	}

	cliper.Push();

	pEditor->Render()->Clear(0,null,CLEAR_STENCIL|CLEAR_TARGET|CLEAR_ZBUFFER,pEditor->GetBackgroundColor(),1.0f,0);

	RENDERVIEWPORT vp = pEditor->Render()->GetViewport();
	Matrix  matOldPrj = pEditor->Render()->GetProjection();

	Matrix view;

	const Vector *to = null;

	if( pEditor->bSimulating && !ViewPort->m_look && /*shift_pressed*/autoLook )
	{
		if( pEditor->RagDollAnim->Bones )
		{
			to = &pEditor->RagDollAnim->Bones[0].FinalMat.pos;
		}
	}

	float k = dt*0.25f;

	if( k > 1.0f )
		k = 1.0f;

	pEditor->ArcBall()->BuildViewMatrix(view,!pEditor->bOpenedDialog,to,k);
	pEditor->Render()->SetView(view);

	pEditor->Render()->SetPerspective(1.0f,(float)vp.Width,(float)vp.Height);

	Matrix mat;

	//pEditor->DrawCapsule(0.75f, 1.0, 0xaaff00ff,mat);

	//DrawAxisNavigator ();

	const Matrix &matProjection = pEditor->Render()->GetProjection();
	const Matrix &matView		= pEditor->Render()->GetView();

	pEditor->GetGizmo()->SetView(matView);
	pEditor->GetGizmo()->SetProjection(matProjection);

	pEditor->GetGizmo()->SetViewPort(pEditor->Render()->GetViewport());

	Matrix mCubeTransform = pEditor->GetGizmo()->GetTransform();
	//pEditor->Render()->DrawSolidBox(Vector(-1.0f), Vector(1.0f), mCubeTransform);

	pEditor->ModelSelBone = false;

	if( pEditor->pGeom )
	{
		//pGeom->SetTransform(mCubeTransform);
				
		if (pEditor->bSimulating)
		{
			pEditor->SetRagDollAnimBoneMatrix(&pEditor->RootBone);

			pEditor->CalcRagDollAnimBoneMatrix(-1);

			for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
			{			
				for (int j=0;j<pEditor->RagDollAnim->NumOptBones;j++)
				{
					if (string::IsEqual(pEditor->RagDollAnim->OptBonesNames[j].Name.c_str(), pEditor->RagDollAnim->Bones[i].Name.c_str()))
					{
						pEditor->RagDollAnim->matrices[j]=pEditor->RagDollAnim->Bones[i].FinalMat;

						break;
					}
				}
			}
		}		
				
		if( ShowGeom /*|| pEditor->bSimulating*/ )
		{
			for( int i = 0 ; i < pEditor->RagDollAnim->Bones ; i++ )
			{
				if(!pEditor->bSimulating )
				{
					pEditor->pAnim->SetBoneMatrix(i,pEditor->RagDollAnim->Bones[i].InitMat);
				}
				else
				{
					pEditor->pAnim->SetBoneMatrix(i,pEditor->RagDollAnim->Bones[i].FinalMat);
				}
			}

			if( drawOrder )
			{
				Color col(0.0f,pEditor->alphaGeom);

				pEditor->pGeom->SetUserColor(col);

				pEditor->pGeom->Draw();
			}
		}
	}

	if( pEditor->pGeom /*&& !pEditor->bSimulating*/ && pEditor->bAttachBoneMode )
	{
		//pEditor->Render()->Print(Vector(0,1,0),10000.0f, -4.0f,0xFFFF0000,"sfsadfsad");

		//IGMXIterator* pIterator;
		
		//pIterator=pEditor->pGeom->CreateIterator ( "333", 7);

		//pIterator->Begin();

		//pEditor->Render()->Print(Vector(0,1,0),10000.0f, -4.0f,0xFFFF0000,"%i",pIterator->GetCount());

		GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();
		GUIPoint LocalMousePos = MousePos;

		ViewPort->ScreenToClient(LocalMousePos);

		float sX = (float)LocalMousePos.x;
		float sY = (float)LocalMousePos.y;

		Vector rayStart;
		Vector rayEnd;
	
		Vector v;
		v.x =  ( ( ( 2.0f * sX ) / ViewPortWidth  ) - 1 ) / matProjection.m[0][0];
		v.y = -( ( ( 2.0f * sY ) / ViewPortHeight ) - 1 ) / matProjection.m[1][1];
		v.z =  1.0f;

		Vector tmp = matView.GetCamPos();

		Matrix mView = matView;
		mView.Inverse ();

		Vector rayDir;
		Vector rayOrig;
		rayOrig = mView.pos;
	
		rayDir = mView.MulNormal(v);

		rayStart = rayOrig + (rayDir * 0.0f);
		rayEnd = (rayOrig + (rayDir * 150.0f));		

		Sphere Sphr;

		pEditor->ModelSelBone = false;

		for( dword i = 0 ; i < pEditor->RagDollAnim->Bones.Size() ; i++ )
		{		
			RDBone *pBone = &pEditor->RagDollAnim->Bones[i];
	
			const char* szName = pBone->Name.c_str();					

			int Depth=0;

			string ParentName="";

			long parent=-1;

			while (strcmp (ParentName.c_str(), "-1") != 0 && parent==-1)
			{					
				ParentName = GetPrnt(szName, Depth);						
					
				for (dword j = 0; j < pEditor->RagDollAnim->Bones.Size(); j++)
				{
					if (j == i) continue;

					RDBone* pPrntBone=&pEditor->RagDollAnim->Bones[j];

					const char* szParentName = pPrntBone->Name.c_str();
						
					if (strcmp (ParentName.c_str(), szParentName) == 0)
					{
						parent=j;
							
						break;
					}
				}

				if (parent==-1)
				{
					Depth++;
				}
			}
					
			Matrix m1 = pEditor->bSimulating ? pBone->FinalMat : pBone->InitMat;

			Sphr.r = 0.025f;

			Sphr.pos = m1.pos;

			dword Color=0xffffffff;

			dword Color2=0xff00ff00;

			if (pEditor->SelBone!=NULL)
			{				
				if (parent==-1)
				{
					if (string::IsEqual(pEditor->SelBone->AssignedBoneName.c_str(),pBone->Name.c_str()))
					{
						Color=0xff0000ff;
						Color2=0xff0000ff;
					}
				}
				else
				{
					RDBone* pParentBone=&pEditor->RagDollAnim->Bones[parent];

					if (string::IsEqual(pEditor->SelBone->AssignedBoneName.c_str(),pParentBone->Name.c_str()))
					{
						Color=0xff0000ff;
						Color2=0xff0000ff;
					}
				}
			}

			bool bTmp = pEditor->ModelSelBone;

			if(!pEditor->ModelSelBone &&
				pEditor->bAttachBoneMode &&
			   !pEditor->bOpenedDialog && !ViewPort->m_look && Sphr.Intersection(rayStart,rayEnd))
			{
				Color=0xffff0000;
				Color2=0xffff0000;

				if (parent!=-1)
				{
					RDBone* pParentBone=&pEditor->RagDollAnim->Bones[parent];

					Matrix m2 = pEditor->bSimulating ? pParentBone->FinalMat : pParentBone->InitMat;
										
					Vector LookFrom = Vector(0,0,0);							
					Vector LookTo=m1.pos-m2.pos;
					Vector vecUp=Vector(0,1,0);
							
					pEditor->fLenght=(m1.pos-m2.pos).GetLength();

					Matrix m3,m4;
							
					m4.SetIdentity();
					m4.RotateX(PI/2);
							
					m3.BuildOriented(LookFrom,LookTo,vecUp);

					m3.pos=Vector(0,0,0);

					m3 = m4 * m3;

				//	Matrix temp = pEditor->bSimulating ? pBone->FinalMat : pBone->InitMat;

					m3.vx.Normalize();
					m3.vy.Normalize();
					m3.vz.Normalize();
					
					pEditor->BoneMatrix=m3;
					
					pEditor->BoneMatrix.pos=m2.pos;

					pEditor->ModelSelBone=true;

					pEditor->BoneName=pParentBone->Name;

					if( shift_pressed )
					{
						pEditor->BoneMatrix.pos = m1.pos;

						pEditor->BoneName = pBone->Name;
					}
				}
				else
				{	
					pEditor->BoneMatrix.SetIdentity();
					pEditor->BoneMatrix.pos = m1.pos;
					
					pEditor->BoneName = pBone->Name;

					pEditor->fLenght = 0;

					pEditor->ModelSelBone = true;
				}
			}

			////////////////////
		
			{
				if( ViewPort->m_look )
				{
					pEditor->Render()->DrawSphere(m1.pos,0.025f,Color);
				}
				else
				{
					pEditor->Render()->DrawSphere(m1.pos,0.025f,Color);

					if(!bTmp && pEditor->ModelSelBone )
					{
						pEditor->Render()->Print(10,10,/*0xffff0000*/0xffffffff,"%s",pBone->Name);
					}
				}
			}					
						
			if (parent!=-1)
			{						
				RDBone* pParentBone=&pEditor->RagDollAnim->Bones[parent];
						
				{
					Matrix m2 = pEditor->bSimulating ? pParentBone->FinalMat : pParentBone->InitMat;
							
					pEditor->Render()->DrawLine(m1.pos, Color2, m2.pos, Color2);
				}
			}
			else
			{
				//api->Trace(pBone->Name);
			}
		}
	}	
	else
	{
		pEditor->ModelSelBone=false;
	}

	DrawGrid();

	if( pEditor->pGeom && !drawOrder )
	{
		if( ShowGeom /*|| pEditor->bSimulating*/ )
		{
			Color col(0.0f,pEditor->alphaGeom);

			pEditor->pGeom->SetUserColor(col);

			pEditor->pGeom->Draw();
		}
	}

	pEditor->GetGizmo()->Draw();

	//pEditor->Render()->DrawMatrix(pEditor->LocMatrix);
	//pEditor->Render()->Print(Vector(0,0,0),1000,0,0xff00ff00,"%f",pEditor->fLenght);

	//pEditor->Manager()->Execute(DeltaTime);


	/*if (bShowAxis)
	{
		GUITreeNode* TNode = TreeView1->GetSelectedNode();
		if (TNode && TNode->Tag == ET_POINT_EMITTER)
		{
			IEmitter* pEmiter = (IEmitter*)TNode->Data;

			Matrix EmissionDir = pEmiter->GetTransform();
			pEditor->Render()->DrawMatrix(EmissionDir);
			pEditor->Render()->Print(Vector(0.0f, 1.2f, 0.0f)*EmissionDir, 100.0f, 0, 0xFFFF0000, "Y");
			pEditor->Render()->Print(Vector(1.2f, 0.0f, 0.0f)*EmissionDir, 100.0f, 0, 0xFF00FF00, "X");
			pEditor->Render()->Print(Vector(0.0f, 0.0f, 1.2f)*EmissionDir, 100.0f, 0, 0xFF0000FF, "Z");
		}
	}*/


	if( pEditor->bSimulating )
	{
	//	pEditor->Render()->Print(10,10,0xff00ff00,"Simulating");

		const GUIRectangle &r = GetDrawRect();

		int x = r.x + 16;
		int y = r.y - 15 + r.h;

		GUIHelper::Draw2DRect(x,y - 50,110,50,0x60000000);

		pEditor->Render()->Print(x + 6.0f,y - 10.0f - 35.0f,0xff00ff00,"Simulating");
	}

	pEditor->Render()->SetProjection(matOldPrj);

	cliper.Pop();

	if (SaveOKTime > 0)
	{
		float Alpha = (SaveOKTime / (SAVEDOK_SHOWTIME*2.0f));
		Color BackColor(1.0f, 1.0f, 1.0f, 0.5f + Alpha);
		Color BlackColor(0.0f, 0.0f, 0.0f, Alpha);
		int sLeft = 0;
		int sTop = 0;
		int sWidth = 300;
		int sHeight = 80;

		float sLeftAdd = (pEditor->Render()->GetScreenInfo3D().dwWidth - sWidth) / 2.0f;
		float sTopAdd = (pEditor->Render()->GetScreenInfo3D().dwHeight - sHeight) / 2.0f;
		sLeft += (int)sLeftAdd;
		sTop += (int)sTopAdd;
		GUIHelper::Draw2DRectAlpha( sLeft, sTop, sWidth, sHeight, BackColor.GetDword());
		GUIHelper::DrawWireRect( sLeft, sTop, sWidth, sHeight, BlackColor.GetDword());
		int fntHeight = pFont->GetHeight();
		int fntWidth = pFont->GetWidth(SAVEDOK_MESSAGE);
		int fntAddWidth = (sWidth - fntWidth) / 2;
		int fntAddHeight = (sHeight - fntHeight) / 2;
		pFont->Print(sLeft+fntAddWidth, sTop+fntAddHeight, 0x0, SAVEDOK_MESSAGE);
		SaveOKTime -= api->GetDeltaTime();
	}	

	if( pEditor->pAnimScene )
	{
		for( dword i = 0 ; i < pEditor->RagDollAnim->Bones.Size() ; i++ )
		{			
			int parent = pEditor->RagDollAnim->GetBoneParent(i);

			if( pEditor->bSimulating )
			{			
				if( parent != -1 )
				{
					Matrix inv_mat = pEditor->RagDollAnim->Bones[parent].FinalMat;

					inv_mat.Inverse();

					pEditor->RagDollAnim->Bones[i].BlenderLocalMat = pEditor->RagDollAnim->Bones[i].FinalMat*inv_mat;
				}
				else
				{
					pEditor->RagDollAnim->Bones[i].BlenderLocalMat = pEditor->RagDollAnim->Bones[i].FinalMat;
				}			
			}
			else
			{
				if( parent != -1 )
				{
					Matrix inv_mat = pEditor->RagDollAnim->Bones[parent].InitMat;

					inv_mat.Inverse();

					pEditor->RagDollAnim->Bones[i].BlenderLocalMat = pEditor->RagDollAnim->Bones[i].InitMat*inv_mat;
				}
				else
				{
					pEditor->RagDollAnim->Bones[i].BlenderLocalMat = pEditor->RagDollAnim->Bones[i].InitMat;
				}			
			}
		}
	}

	if( api_exit > 0 )
	{
		if( !--api_exit )
		{
			api->Exit();
		}
	}
}

void TMainWindow::DrawGrid ()
{
	bool shift_pressed = GetAsyncKeyState(VK_SHIFT) < 0;

	if(!pEditor->bOpenedDialog )
	{
		pEditor->PreSelBone	= null;
	//	pEditor->BoneID		= null;
	}

	if(!pEditor->bOpenedDialog && !ViewPort->m_look )
	{
		GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();

		GUIPoint LocalMousePos = MousePos;
		ViewPort->ScreenToClient(LocalMousePos);

		Matrix matProjection=pEditor->Render()->GetProjection();
		Matrix matView=pEditor->Render()->GetView();

		float sX=(float)LocalMousePos.x;
		float sY=(float)LocalMousePos.y;

		Vector rayStart;
		Vector rayEnd;	
	
		Vector v;
		v.x =  ( ( ( 2.0f * sX ) / ViewPortWidth  ) - 1 ) / matProjection.m[0][0];
		v.y = -( ( ( 2.0f * sY ) / ViewPortHeight ) - 1 ) / matProjection.m[1][1];
		v.z =  1.0f;

		Vector tmp = matView.GetCamPos();

		Matrix mView = matView;
		mView.Inverse ();

		Vector rayDir;
		Vector rayOrig;
		rayOrig = mView.pos;
	
		rayDir = mView.MulNormal(v);

		rayStart = rayOrig + (rayDir * 0.0f);
		rayEnd = (rayOrig + (rayDir * 150.0f));

		/*static Vector s(0.0f), e(0.0f);
		if(api->DebugKeyState('1'))
		{
			s = rayStart;
			e = rayEnd;
		}*/
		
		//pEditor->Render()->DrawSphere(rayEnd, 0.5f, 0xff00ffff);
		//pEditor->Render()->DrawSphere(rayStart, 0.01f, 0xff00ff00);
		//pEditor->Render()->DrawVector(s, e, 0xffff0000);
		

		IPhysicsScene::RaycastResult Result;

		IPhysBase* pPhys=NULL;

		pEditor->PreSelBone=NULL;
				
		if (pEditor->bSimulating&&!pEditor->bDragRagDollBone)
		{
			pPhys=pEditor->pPhysScene->Raycast(rayStart,rayEnd,phys_mask(phys_ragdoll),&Result);

			pEditor->BoneID=NULL;

			if (pPhys!=NULL&&Result.id!=0)
			{
				IPhysEditableRagdoll::IBone* pRagDollBone=pEditor->pPhysRagdoll->GetBone(Result.id);

				if (pRagDollBone!=NULL)
				{			
					pEditor->BoneID=Result.id;

					Matrix mat;

					pRagDollBone->GetWorldTransform(mat);

					mat.Inverse();

					pEditor->BoneLocalPoint=mat*Result.position;

					pEditor->fDistance=Result.distance;
				}
			}
		}
		else
		if (pEditor->bEditEnv)
		{
			pPhys=pEditor->pPhysScene->Raycast(rayStart,rayEnd,phys_mask(phys_world),&Result);

			pEditor->PreSelEnvObject=-1;

			for (dword i=0;i<pEditor->EnvObjects.Size();i++)
			{
				if ((IPhysBase*)pEditor->EnvObjects[i].pPhys==pPhys)
				{
					pEditor->PreSelEnvObject=i;

					break;
				}
			}
		/*}
		else
		{*/						
			if (pEditor->PreSelEnvObject==-1)
			{			
				if (!pEditor->bAttachBoneMode) pEditor->SelectBone(&pEditor->RootBone,pEditor->PreSelBone,pPhys);		
			}
		}

		if (pEditor->bDragRagDollBone)
		{
			IPhysEditableRagdoll::IBone* pRagDollBone=pEditor->pPhysRagdoll->GetBone(pEditor->BoneID);

			if (pRagDollBone!=NULL)
			{	
				Matrix mat;
					
				pRagDollBone->GetWorldTransform(mat);

				Vector vNeedFrom = pEditor->BoneLocalPoint * mat;

				Vector vNeedTo = (rayOrig + (rayDir * pEditor->fDistance));

				Vector vDisiredForce= (vNeedTo-vNeedFrom);

				float mass = pEditor->CountMass(&pEditor->RootBone);

				float fStrenght=vDisiredForce.GetLength()*mass*6.5f;

				//if (fStrenght<10) fStrenght=10;

				//if (fStrenght<70) fStrenght=70;

				vDisiredForce.x*=fStrenght;
				vDisiredForce.y*=fStrenght;
				vDisiredForce.z*=fStrenght;
				
				pRagDollBone->ApplyForce( vDisiredForce,pEditor->BoneLocalPoint);

				pEditor->Render()->DrawLine(vNeedFrom, 0xff00ff00,vNeedTo, 0xff00ff00);
			}
		}
	}


	if (pEditor->bSimulating&&pEditor->bRenderDebugGeom) 
	{
		pEditor->pPhysScene->DebugDraw(*pEditor->Render());
	}

	if (pEditor->bSimulating||!pEditor->bAttachBoneMode)
	{	
		for (dword i=0;i<pEditor->EnvObjects.Size();i++)
		{
			Matrix mat;
			if (pEditor->EnvObjects[i].pPhys!=NULL)
			{
				pEditor->EnvObjects[i].pPhys->GetTransform(mat);
			}

			//pEditor->EnvObjects[i].pGeom->SetTransform(pEditor->EnvObjects[i].mTransform);
			pEditor->EnvObjects[i].pGeom->SetTransform(mat);

			pEditor->EnvObjects[i].pGeom->Draw();

			if (i==pEditor->PreSelEnvObject&&pEditor->bEditEnv&&!pEditor->bSimulating)
			{
				pEditor->Render()->DrawBox(Vector(-0.5f,-0.5f,-0.5f),
										   Vector( 0.5f, 0.5f, 0.5f),
										   pEditor->EnvObjects[i].mTransform, 0xff00ff00);
			}
			else
			if (i==pEditor->SelEnvObject&&pEditor->bEditEnv&&!pEditor->bSimulating)
			{
				pEditor->Render()->DrawBox(Vector(-0.5f,-0.5f,-0.5f),
										   Vector( 0.5f, 0.5f, 0.5f),
										   pEditor->EnvObjects[i].mTransform, 0xffff00ff);
			}
		}
	}

	if( pEditor->SelBone != null )
	{		
		pEditor->SelBone->fAnchorA1 = SecktorAnchorA->fStartAngle;
		pEditor->SelBone->fAnchorA2 = SecktorAnchorA->fEndAngle;

		pEditor->SelBone->fAnchorB = 180 - SecktorAnchorB->fStartAngle;
	}

	//pEditor->DrawCone(1,Secktor->fStartAngle,0xaa00ffff,mat);

	/*if (pEditor->SelBone!=NULL)
	{
		pEditor->GetGizmo()->Enable(true);
	}
	else
	{
		pEditor->GetGizmo()->Enable(false);
	}*/

	if( pEditor->GetGizmo()->bHighlighted )
	{
		pEditor->PreSelBone = null;
	}

	/*if (pEditor->SelBone!=NULL)
	{
		if (pEditor->SelBone->Type==state_capsule)
		{
			bBoneType->SelectItem(0);
		}
		else
		if (pEditor->SelBone->Type==state_box)
		{
			bBoneType->SelectItem(1);
		}
	}*/

	if( pEditor->bEditEnv && !pEditor->bSimulating )
	{
		if( pEditor->SelEnvObject != -1 )
		{
			Matrix mat=pEditor->GetGizmo()->GetTransform();			

			if (pEditor->EnvObjects[pEditor->SelEnvObject].mTransform != mat)
			{			
				pEditor->EnvObjects[pEditor->SelEnvObject].mTransform = mat;

				if (pEditor->EnvObjects[pEditor->SelEnvObject].pPhys!=NULL) pEditor->EnvObjects[pEditor->SelEnvObject].pPhys->Release();

				pEditor->EnvObjects[pEditor->SelEnvObject].pPhys = pEditor->EnvObjects[pEditor->SelEnvObject].pGeom->CreatePhysicsActor(*pEditor->pPhysScene, false);

				if (pEditor->EnvObjects[pEditor->SelEnvObject].pPhys)
				{				
					pEditor->EnvObjects[pEditor->SelEnvObject].pPhys->SetTransform(mat);
				}
			}
		}	
		else
		if( pEditor->SelBone != null )
		{
			TRagDollBone *Bone = pEditor->SelBone;

			Matrix mat = pEditor->GetGizmo()->GetTransform();

			if( pEditor->bAttachBoneMode)
			{
				Bone->mGlobal = mat;

				if( Bone->parent )
				{
					Matrix inv_mat;

					inv_mat.Inverse(Bone->parent->mGlobal);

					mat = mat*inv_mat;
				}

				Bone->mLocal = mat;

				if( shift_pressed )
				{
					Matrix inv_mat = Bone->mGlobal;

					inv_mat.Inverse();

					for( int i = 0 ; i < Bone->childs ; i++ )
					{
						Matrix mat = Bone->childs[i]->mGlobal;

						mat = mat*inv_mat;

						Bone->childs[i]->mLocal = mat;
					}
				}
				else
				{
					pEditor->CalcBoneMatrix(/*&pEditor->RootBone*/Bone,false);
				}
			}
			else
			{
				Bone->mWorldGlobal = mat;

				if( Bone->parent )
				{
					Matrix inv_mat;

					inv_mat.Inverse(Bone->parent->mWorldGlobal);

					mat = mat*inv_mat;
				}

				Bone->mWorldLocal = mat;

				if( shift_pressed )
				{
					Matrix inv_mat = Bone->mWorldGlobal;

					inv_mat.Inverse();

					for( int i = 0 ; i < Bone->childs ; i++ )
					{
						Matrix mat = Bone->childs[i]->mWorldGlobal;

						mat = mat*inv_mat;

						Bone->childs[i]->mWorldLocal = mat;
					}
				}
				else
				{
					pEditor->CalcBoneMatrix(/*&pEditor->RootBone*/Bone,true);
				}

				pEditor->DestroyPhys(&pEditor->RootBone);	
				pEditor-> CreatePhys(&pEditor->RootBone,false);
			}
		}
	}

//	if(!pEditor->bSimulating )
	{	
		//if (/*!ShowGeom||*/pEditor->pGeom==NULL)
		{
			byte power = pEditor->bAttachBoneMode ? 30 : 250;

			pEditor->RenderBone(&pEditor->RootBone,power,pEditor->bAttachBoneMode);
		}
	}
/*	else
	{
	//	if( ShowGeom || pEditor->pGeom == null )
		{		
		//	pEditor->RenderBone(&pEditor->RootBone,150,true);

			byte power = pEditor->bAttachBoneMode ? 30 : 250;

			pEditor->RenderBone(&pEditor->RootBone,power,pEditor->bAttachBoneMode);
		}		
	}*/

	if(!pEditor->bSimulating )
	{
		pEditor->RenderSkeleton(&pEditor->RootBone,!pEditor->bAttachBoneMode);
	}

	/*RS_SPRITE spr[4];

	spr[0].vPos = Vector (pEditor->x1, pEditor->y1+0.1f, 0.0f);
	spr[1].vPos = Vector (pEditor->x1+0.1f, pEditor->y1+0.1f, 0.0f);
	spr[2].vPos = Vector (pEditor->x1+0.1f, pEditor->y1, 0.0f);
	spr[3].vPos = Vector (pEditor->x1, pEditor->y1, 0.0f);

	spr[0].dwColor = 0xFFFFFFFF;
	spr[1].dwColor = 0xFFFFFFFF;
	spr[2].dwColor = 0xFFFFFFFF;
	spr[3].dwColor = 0xFFFFFFFF;

	pEditor->Render()->DrawSprites(spr, 1);*/

	//pEditor->Render()->Print(Vector(0,0,0),10000.0f, -4.0f,0xFFFF0000,"mx=%f my=%f x1=%f x2=%f y1=%f y2=%f",pEditor->mx,pEditor->my,pEditor->x1,pEditor->x2,pEditor->y1,pEditor->y2);

	//pEditor->Render()->Print(Vector(0,0,0),10000.0f, -4.0f,0xFFFF0000,"mx=%f my=%f x1=%f x2=%f y1=%f y2=%f",pEditor->mx,pEditor->my,pEditor->x1,pEditor->x2,pEditor->y1,pEditor->y2);

	//pEditor->Render()->DrawMatrix(pEditor->Render()->GetView(), 10.0f);

	/*pEditor->Render()->DrawBox(pEditor->GetGizmo()->GetTransform().pos-Vector(1,1,1),		
		                            pEditor->GetGizmo()->GetTransform().pos,
									pEditor->GetGizmo()->GetTransform(),
									0xffffffff, NULL,NULL);*/

	float StepSize = 1.0f;
	int GridSize = 12;
	float Edge = StepSize*GridSize;

	for (int x = 0; x <= GridSize; x++)
	{
		float x1 = x * StepSize;
		float x2 = -x1;

		DWORD clr = pEditor->GetGridColor();
		if (x == 0) clr = pEditor->GetGridZeroColor();
		pEditor->Render()->DrawLine(Vector (x1, -4.5f, Edge), clr, Vector (x1, -4.5f, -Edge), clr);
		pEditor->Render()->DrawLine(Vector (x2, -4.5f, Edge), clr, Vector (x2, -4.5f, -Edge), clr);
	}

	for (int z = 0; z <= GridSize; z++)
	{
		float z1 = z * StepSize;
		float z2 = -z1;

		DWORD clr = pEditor->GetGridColor();
		if (z == 0) clr = pEditor->GetGridZeroColor();
		pEditor->Render()->DrawLine(Vector (Edge, -4.5f, z1), clr, Vector (-Edge, -4.5f, z1), clr);
		pEditor->Render()->DrawLine(Vector (Edge, -4.5f, z2), clr, Vector (-Edge, -4.5f, z2), clr);
	}	
}


void TMainWindow::MouseMove (int button, const GUIPoint& pt)
{
	GUIWindow::MouseMove (button, pt);
}

void TMainWindow::MouseUp (int button, const GUIPoint& pt)
{
	GUIWindow::MouseUp (button, pt);
}


bool TMainWindow::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	/*int Key = (int)lparam;

	if (Key == 'W')
	{
		pEditor->Bone2.mLocal.pos=Vector(0,0,4);

		pEditor->GetGizmo()->SetTransform(pEditor->Bone2.mLocal);
	}*/

	return GUIWindow::ProcessMessages(message, lparam, hparam);
}

void _cdecl TMainWindow::ExitPressed(GUIControl* sender)
{
//	api->Exit();
	api_exit = 2;
}

void _cdecl TMainWindow::OnCreateBone (GUIControl* sender)
{
	if (pEditor->SelBone!=NULL)
	{
		TRagDollBone* pBone = NEW TRagDollBone;
		
		pBone->parent=pEditor->SelBone;	
		pEditor->SelBone->childs.Add(pBone);

		pBone->mLocal.SetIdentity();
		pBone->mLocal.pos=Vector(0,0.5f,0);

		pBone->mGlobal=  pBone->mLocal * pBone->parent->mGlobal;
		pBone->mWorldGlobal=  pBone->mLocal * pBone->parent->mGlobal;

		

		pEditor->SelBone=pBone;
		pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mGlobal);
		pEditor->GetGizmo()->CalcGizmoScale();

		pEditor->UpdateScrolls();

		pEditor->DestroyPhys(&pEditor->RootBone);	
		pEditor->CreatePhys(&pEditor->RootBone,false);
	}
}

void _cdecl TMainWindow::OnDeleteBone (GUIControl* sender)
{
	if (pEditor->SelBone!=NULL&&pEditor->SelBone!=&pEditor->RootBone)
	{
		pEditor->DestroyPhys(&pEditor->RootBone);	

		pEditor->SelBone->childs.DelAll();

		pEditor->DeleteBone(pEditor->SelBone, true);

		pEditor->SelBone=NULL;

		pEditor->GetGizmo()->Enable(false);
		
		pEditor->CreatePhys(&pEditor->RootBone,false);
	}
}

__inline void MoveControl(int dy, GUIControl *p)
{
	GUIRectangle d = p->GetDrawRect();
	GUIRectangle c = p->GetClientRect();

	d.y += dy;
	c.y += dy;

	p->SetDrawRect(d);
	p->SetClientRect(c);
}

void TMainWindow::MoveBottoms(int dy)
{
	MoveControl(dy,pBoneMass);
	MoveControl(dy,labelMass);
	MoveControl(dy,bSphericalJointType);
	MoveControl(dy,bRevoltJointType);
	MoveControl(dy,SecktorAnchorA);
	MoveControl(dy,labelSpring);
	MoveControl(dy,pBoneSpringA);
	MoveControl(dy,labelDumper);
	MoveControl(dy,pBoneDamperA);
	MoveControl(dy,SecktorAnchorB);
	MoveControl(dy,pLabelSpringB);
	MoveControl(dy,pBoneSpringB);
	MoveControl(dy,pLabelDamperB);
	MoveControl(dy,pBoneDamperB);
}

void _cdecl TMainWindow::OnChangeBoneType(GUIControl *sender)
{
	if( pEditor->SelBone == null )
		return;	

	if( bCapsuleBoneType->Checked && pEditor->SelBone->Type != state_capsule )
	{	
		pEditor->SelBone->Type = state_capsule;
		pScrollBoneWidth->Visible = false;

		GUIRectangle r = BottomPanel->GetDrawRect(); r.h -= 24;

		BottomPanel->SetDrawRect(r);
		BottomPanel->SetClientRect(r);

		MoveBottoms(-24);
	}
	else
	if( bBoxBoneType->Checked && pEditor->SelBone->Type != state_box )
	{			
		pEditor->SelBone->Type = state_box;
		pScrollBoneWidth->Visible = true;

		GUIRectangle r = BottomPanel->GetDrawRect(); r.h += 24;

		BottomPanel->SetDrawRect(r);
		BottomPanel->SetClientRect(r);

		MoveBottoms( 24);
	}
	
	pEditor->DestroyPhys(&pEditor->RootBone);	
	pEditor->CreatePhys(&pEditor->RootBone,false);
}

void _cdecl TMainWindow::OnChangeBoneHeihgt (GUIControl* sender)
{
	if (pEditor->SelBone!=NULL)
	{
		pEditor->SelBone->fHeight=(float)pScrollBoneHeight->Position/100.0f+0.0f;
	}
}

void _cdecl TMainWindow::OnChangeBoneLenght (GUIControl* sender)
{
	if (pEditor->SelBone!=NULL)
	{
		pEditor->SelBone->fLenght=(float)pScrollBoneLenght->Position/100.0f+0.0f;
	}
}

void _cdecl TMainWindow::OnChangeBoneWidth (GUIControl* sender)
{
	if (pEditor->SelBone!=NULL)
	{
		pEditor->SelBone->fWidth=(float)pScrollBoneWidth->Position/100.0f+0.0f;
	}
}

void _cdecl TMainWindow::OnChangeJointType(GUIControl *sender)
{
	if( pEditor->SelBone != null )
	{
		if( bSphericalJointType->Checked && pEditor->SelBone->iJointType != 0 )
		{
			pEditor->SelBone->iJointType = 0;
			SecktorAnchorB->Visible = true;
			
			pLabelSpringB->Visible = true;
			pBoneSpringB->Visible = true;
			
			pLabelDamperB->Visible = true;
			pBoneDamperB->Visible = true;

			pEditor->SelBone->fSpringA = 5.5f;
			pEditor->SelBone->fDamperA = 0.5f;

			pEditor->SelBone->fSpringB = 0.15f;
			pEditor->SelBone->fDamperB = 0.05f;

			GUIRectangle r = BottomPanel->GetDrawRect(); r.h += 60;

			BottomPanel->SetDrawRect(r);
			BottomPanel->SetClientRect(r);
		}
		else
		if( bRevoltJointType->Checked && pEditor->SelBone->iJointType != 1 )
		{
			pEditor->SelBone->iJointType = 1;
			SecktorAnchorB->Visible = false;
			
			pLabelSpringB->Visible = false;
			pBoneSpringB->Visible = false;

			pLabelDamperB->Visible = false;
			pBoneDamperB->Visible = false;

			pEditor->SelBone->fSpringA = 0.5f;
			pEditor->SelBone->fDamperA = 0.1f;

			GUIRectangle r = BottomPanel->GetDrawRect(); r.h -= 60;

			BottomPanel->SetDrawRect(r);
			BottomPanel->SetClientRect(r);
		}

		char str[256];					

		crt_snprintf(str,sizeof(str) - 1,"%4.4f",pEditor->SelBone->fSpringA);
		pBoneSpringA->Text = str;

		crt_snprintf(str,sizeof(str) - 1,"%4.4f",pEditor->SelBone->fDamperA);
		pBoneDamperA->Text = str;

		crt_snprintf(str,sizeof(str) - 1,"%4.4f",pEditor->SelBone->fSpringB);
		pBoneSpringB->Text = str;

		crt_snprintf(str,sizeof(str) - 1,"%4.4f",pEditor->SelBone->fDamperB);
		pBoneDamperB->Text = str;
	}
}

void _cdecl TMainWindow::StartPhisSimulatione (GUIControl* sender)
{
	for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
	{			
		for (int j=0;j<pEditor->RagDollAnim->NumOptBones;j++)
		{
			if (string::IsEqual(pEditor->RagDollAnim->OptBonesNames[j].Name.c_str(), pEditor->RagDollAnim->Bones[i].Name.c_str()))
			{
				pEditor->RagDollAnim->matrices[j]=pEditor->RagDollAnim->Bones[i].InitMat;

				//break;
			}
		}
	}

	for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
	{		
		pEditor->RagDollAnim->Bones[i].NeedCalclFinalMat=true;
	}

	{	
		for (dword i=0;i<pEditor->EnvObjects.Size();i++)
		{			
			if (pEditor->EnvObjects[i].pPhys) pEditor->EnvObjects[i].pPhys->Release();
			
			pEditor->EnvObjects[i].pGeom->SetTransform(pEditor->EnvObjects[i].mTransform);

			pEditor->EnvObjects[i].pPhys = pEditor->EnvObjects[i].pGeom->CreatePhysicsActor(*pEditor->pPhysScene, false);

			if (pEditor->EnvObjects[i].pPhys)
			{			
				pEditor->EnvObjects[i].pPhys->SetTransform(pEditor->EnvObjects[i].mTransform);			
			}
		}
	}	

	pEditor->pPhysRagdoll->Clear();	
	pEditor->DestroyPhys(&pEditor->RootBone);
	pEditor->pPhysScene->Update(api->GetDeltaTime());
	
	pEditor->RootBone.RagDollBone=&pEditor->pPhysRagdoll->GetRootBone();
	
	//pEditor->SkinRagDollAnimBoneMatrix(&pEditor->RootBone);
	
	pEditor->CreatePhysRagDoll(&pEditor->RootBone);	
	//pEditor->pPhysRagdoll->SetBlendStage(pEditor->RagDollAnim, 15);


	pEditor->GetGizmo()->Enable(false);
	
	pEditor->bSimulating = true;


	btnNewRagDoll->Enabled = btnNewRagDoll->bEnabled = false;
//	btnOpenRagDoll->Enabled = false;
//	btnSaveRagDoll->Enabled = false;
//	btnSaveAsRagDoll->Enabled = false;

	btnExportToBin->Enabled = btnExportToBin->bEnabled = false;

	btnBoneAdd->Enabled = btnBoneAdd->bEnabled = false;
	btnBoneDelete->Enabled = btnBoneDelete->bEnabled = false;

	pScrollBoneHeight->Enabled = false;
	pScrollBoneLenght->Enabled = false;
	pScrollBoneWidth->Enabled = false;
}

void _cdecl TMainWindow::StopPhisSimulation (GUIControl* sender)
{
	for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
	{			
		for (int j=0;j<pEditor->RagDollAnim->NumOptBones;j++)
		{
			if (string::IsEqual(pEditor->RagDollAnim->OptBonesNames[j].Name.c_str(), pEditor->RagDollAnim->Bones[i].Name.c_str()))
			{
				pEditor->RagDollAnim->matrices[j]=pEditor->RagDollAnim->Bones[i].InitMat;

				//break;
			}
		}
	}

	pEditor->pPhysRagdoll->Clear();
	pEditor->pPhysScene->Update(api->GetDeltaTime());
	//pEditor->DestroyPhys(&pEditor->RootBone);
	
	pEditor->CreatePhys(&pEditor->RootBone,false);

	if (pEditor->SelBone!=NULL||pEditor->SelEnvObject!=-1)
	{	
		pEditor->GetGizmo()->Enable(true);
	}

	pEditor->bSimulating=false;

	btnNewRagDoll->Enabled = btnNewRagDoll->bEnabled = true;

	btnOpenRagDoll->Enabled=true;
	btnSaveRagDoll->Enabled=true;
	btnSaveAsRagDoll->Enabled=true;

	btnBoneAdd->Enabled = btnBoneAdd->bEnabled = true;
	btnBoneDelete->Enabled = btnBoneDelete->bEnabled = true;

	btnExportToBin->Enabled = btnExportToBin->bEnabled = true;

	pScrollBoneHeight->Enabled=true;
	pScrollBoneLenght->Enabled=true;
	pScrollBoneWidth->Enabled=true;

}

void TMainWindow::UpdateScrolls()
{
	if( pEditor->SelBone != null )
	{
		pScrollBoneHeight->Position=(int)(((float)pEditor->SelBone->fHeight-0.0f)*100.0f);
		pScrollBoneLenght->Position=(int)(((float)pEditor->SelBone->fLenght-0.0f)*100.0f);
		pScrollBoneWidth->Position=(int)(((float)pEditor->SelBone->fWidth-0.0f)*100.0f);
		

		if (pEditor->SelBone->Type==state_capsule)
		{
			if( pScrollBoneWidth->Visible )
			{
				GUIRectangle r = BottomPanel->GetDrawRect(); r.h -= 24;

				BottomPanel->SetDrawRect(r);
				BottomPanel->SetClientRect(r);

				MoveBottoms(-24);
			}

			//bBoneType->SelectItem(0);
			pScrollBoneWidth->Visible=false;	

			bCapsuleBoneType->Checked=true;
			bBoxBoneType->Checked=false;
		}
		else
		if (pEditor->SelBone->Type==state_box)
		{
			if(!pScrollBoneWidth->Visible )
			{
				GUIRectangle r = BottomPanel->GetDrawRect(); r.h += 24;

				BottomPanel->SetDrawRect(r);
				BottomPanel->SetClientRect(r);

				MoveBottoms( 24);
			}

			//bBoneType->SelectItem(1);
			pScrollBoneWidth->Visible=true;

			bCapsuleBoneType->Checked=false;
			bBoxBoneType->Checked=true;
		}		
	

		pEditor->DestroyPhys(&pEditor->RootBone);	
		pEditor->CreatePhys(&pEditor->RootBone,false);

		SecktorAnchorA->fStartAngle=pEditor->SelBone->fAnchorA1;
		SecktorAnchorA->fEndAngle=pEditor->SelBone->fAnchorA2;

		SecktorAnchorB->fStartAngle=180-pEditor->SelBone->fAnchorB;
		SecktorAnchorB->fEndAngle=360-SecktorAnchorB->fStartAngle;		

		if (pEditor->SelBone->iJointType==0)
		{
			if(!SecktorAnchorB->Visible )
			{
				GUIRectangle r = BottomPanel->GetDrawRect(); r.h += 60;

				BottomPanel->SetDrawRect(r);
				BottomPanel->SetClientRect(r);
			}

			//SecktorAnchorA->SetNeedDrag(false);
			SecktorAnchorB->Visible=true;
			bSphericalJointType->Checked=true;
			bRevoltJointType->Checked=false;
			
			pLabelSpringB->Visible=true;
			pBoneSpringB->Visible=true;

			pLabelDamperB->Visible=true;
			pBoneDamperB->Visible=true;
		}
		else
		{
			if( SecktorAnchorB->Visible )
			{
				GUIRectangle r = BottomPanel->GetDrawRect(); r.h -= 60;

				BottomPanel->SetDrawRect(r);
				BottomPanel->SetClientRect(r);
			}

			SecktorAnchorB->SetNeedDrag(true);
			SecktorAnchorB->Visible=false;
			bSphericalJointType->Checked=false;
			bRevoltJointType->Checked=true;

			pLabelSpringB->Visible=false;
			pBoneSpringB->Visible=false;

			pLabelDamperB->Visible=false;
			pBoneDamperB->Visible=false;
		}

		char str[256];		
		
		crt_snprintf(str, sizeof(str) - 1, "%4.4f",pEditor->SelBone->fMass);
		pBoneMass->Text=str;

		crt_snprintf(str, sizeof(str) - 1,"%4.4f",pEditor->SelBone->fSpringA);
		pBoneSpringA->Text=str;

		crt_snprintf(str, sizeof(str) - 1,"%4.4f",pEditor->SelBone->fDamperA);
		pBoneDamperA->Text=str;

		crt_snprintf(str, sizeof(str) - 1,"%4.4f",pEditor->SelBone->fSpringB);
		pBoneSpringB->Text=str;

		crt_snprintf(str, sizeof(str) - 1,"%4.4f",pEditor->SelBone->fDamperB);
		pBoneDamperB->Text=str;
	}
}

void _cdecl TMainWindow::SavePressed (GUIControl* sender)
{		
	if( pEditor->ProjectFileName == "" )
	{	
		pEditor->bOpenedDialog = true;

		GUIFileSave *save_dialog = NEW GUIFileSave();

		save_dialog->Filters.Add(".rdl");
		save_dialog->FiltersDesc.Add("RagDoll Data");
		save_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::SaveSkeleton;
		save_dialog->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
		save_dialog->StartDirectory = string(string(pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
		Application->ShowModal(save_dialog);
		save_dialog->Filter->SelectItem (1);
	}
	else
	{
		///////////////

		if(!pEditor->bAttachBoneMode )
		{
			pEditor->SetLocalMatrixAsWorld(&pEditor->RootBone);
		}

	//	pEditor->CalcLocalChildMatrix(&pEditor->RootBone);

		///////////////

		pEditor->SaveSkeletonData(pEditor->ProjectFileName);

		pEditor->bOpenedDialog = false;

		SaveOKTime = SAVEDOK_SHOWTIME;
	}
}

void _cdecl TMainWindow::SaveSkeleton(GUIControl *sender)
{
	GUIFileSave *so = (GUIFileSave *)sender;

	string FileName = so->FileName;

	FileName.AddExtention(".rdl");

	///////////////

	if(!pEditor->bAttachBoneMode )
	{
		pEditor->SetLocalMatrixAsWorld(&pEditor->RootBone);
	}

	pEditor->CalcLocalChildMatrix(&pEditor->RootBone);

	///////////////

	pEditor->ProjectFileName = FileName;

	pEditor->SaveSkeletonData(FileName);

	SaveOKTime = SAVEDOK_SHOWTIME;

	pEditor->bOpenedDialog = false;

	SetCaption(FileName);
}

void _cdecl TMainWindow::OnOpenRagDoll (GUIControl* sender)
{
	pEditor->bOpenedDialog=true;

	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->Filters.Add (".rdl");
	fo->FiltersDesc.Add ("RagDoll Data");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadSkeleton;
	fo->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	fo->StartDirectory = string(string (pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TMainWindow::LoadSkeleton (GUIControl* sender)
{
	StopPhisSimulation(null);

	pEditor->DestroyPhys(&pEditor->RootBone);
	
	GUIFileOpen* fo = (GUIFileOpen*)sender;
	
	string FileName = fo->FileName;

	pEditor->ProjectFileName=FileName;
	
	pEditor->RootBone.childs.DelAll();

	pEditor->LoadSkeletonData(FileName.c_str());
	
	pEditor->SelBone=NULL;
	pEditor->GetGizmo()->Enable(false);

	pEditor->SelEnvObject=-1;	
	
	pEditor->CreatePhys(&pEditor->RootBone,false);

	pEditor->bOpenedDialog=false;	
		
	LoadGMX(pEditor->GMXModelName);
	
	LoadSceletonFromAnt(pEditor->ANTName);

	SetCaption(FileName);
}

void _cdecl TMainWindow::OnAddEnvObject (GUIControl* sender)
{
	if (pEditor->bSimulating) return;

//	const char *gmxName = "barrel_cyl_v20.gmx";
	const char *gmxName = "chest_big_1.gmx";

	TEnvObjects EnvObject;

	EnvObject.mTransform.pos = Vector(0.0f,-2.0f,0.0f);

	IGMXService* pGEOServ = (IGMXService*)api->GetService("GMXService");
	EnvObject.pGeom = pGEOServ->CreateGMX(gmxName,0,NULL,NULL);

	EnvObject.ModelName = gmxName;

	EnvObject.pGeom->SetTransform(EnvObject.mTransform);

	GMXBoundBox box = EnvObject.pGeom->GetLocalBound();

	Vector size(box.vMax.x - box.vMin.x,box.vMax.y - box.vMin.y,box.vMax.z - box.vMin.z);

//	EnvObject.pPhys = EnvObject.pGeom->CreatePhysicsActor(*pEditor->pPhysScene, false);
	EnvObject.pPhys = pEditor->pPhysScene->CreateBox(_FL_,size,EnvObject.mTransform,true);

	if (EnvObject.pPhys)
	{
		EnvObject.pPhys->SetTransform(EnvObject.mTransform);
	}

	EnvObject.pPhys->Activate(true);
	EnvObject.pPhys->EnableCollision(true);

	pEditor->EnvObjects.Add(EnvObject);	
	
	pEditor->SelEnvObject= pEditor->EnvObjects.Size()-1;

	pEditor->GetGizmo()->SetTransform(pEditor->EnvObjects[pEditor->SelEnvObject].mTransform);
	pEditor->GetGizmo()->CalcGizmoScale();

	pEditor->GetGizmo()->Enable(true);
}

void _cdecl TMainWindow::OnDeleteEnvObject(GUIControl *sender)
{
	Assert(pEditor->bSimulating == false)
	Assert(pEditor->SelEnvObject >= 0)

	RELEASE(pEditor->EnvObjects[pEditor->SelEnvObject].pPhys)

	pEditor->EnvObjects.DelIndex(pEditor->SelEnvObject);

	pEditor->SelEnvObject= -1;

	pEditor->GetGizmo()->Enable(false);
}

void _cdecl TMainWindow::OnCloseManager (GUIControl* sender)
{		
}

void _cdecl TMainWindow::OnCreateNewRagDoll (GUIControl* sender)
{	
	pEditor->DestroyPhys(&pEditor->RootBone);
	
	pEditor->ProjectFileName="";

	pEditor->RootBone.childs.DelAll();
	
	pEditor->SelEnvObject=-1;

	pEditor->SelBone=NULL;
	pEditor->GetGizmo()->Enable(false);

	if (pEditor->pGeom)
	{
		pEditor->pGeom->Release();

		pEditor->pGeom=NULL;

		if (pEditor->RagDollAnim->matrices) delete []pEditor->RagDollAnim->matrices;

		pEditor->RagDollAnim->Bones.DelAll();
	}

	pEditor->GMXModelName="";		
	pEditor->ANTName="";	

	TRagDollBone RagDollBone;

	pEditor->RootBone=RagDollBone;

	pEditor->CreatePhys(&pEditor->RootBone,false);

	pEditor->bOpenedDialog=false;

	SetCaption("");
}

void _cdecl TMainWindow::SaveAsPressed (GUIControl* sender)
{
	pEditor->bOpenedDialog=true;

	GUIFileSave* save_dialog = NEW GUIFileSave ();
	save_dialog->Filters.Add(".rdl");
	save_dialog->FiltersDesc.Add("RagDoll Data");
	save_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::SaveSkeleton;
	save_dialog->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	save_dialog->StartDirectory = string(string (pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
	Application->ShowModal(save_dialog);
	save_dialog->Filter->SelectItem (1);
}

void _cdecl TMainWindow::ExportPressed (GUIControl* sender)
{
	pEditor->bOpenedDialog=true;

	GUIFileSave* save_dialog = NEW GUIFileSave ();
	save_dialog->Filters.Add(".rdb");
	save_dialog->FiltersDesc.Add("Binary RagDoll Data");
	save_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::ExportRagDoll;
	save_dialog->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	save_dialog->StartDirectory = string(string (pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
	Application->ShowModal(save_dialog);
	save_dialog->Filter->SelectItem (1);
}

void _cdecl TMainWindow::ExportRagDoll(GUIControl *sender)
{
	for( int i = 0 ; i < pEditor->RagDollAnim->Bones; i++ )
	{			
		for( int j = 0 ; j < pEditor->RagDollAnim->NumOptBones ; j++ )
		{
			if( string::IsEqual(pEditor->RagDollAnim->OptBonesNames[j].Name.c_str(),pEditor->RagDollAnim->Bones[i].Name.c_str()))
			{
				pEditor->RagDollAnim->matrices[j] = pEditor->RagDollAnim->Bones[i].InitMat;

				break;
			}
		}
	}

	{	
		for( int i = 0 ; i < pEditor->EnvObjects ; i++ )
		{	
			RELEASE(pEditor->EnvObjects[i].pPhys)

			pEditor->EnvObjects[i].pGeom->SetTransform(pEditor->EnvObjects[i].mTransform);

			pEditor->EnvObjects[i].pPhys = pEditor->EnvObjects[i].pGeom->CreatePhysicsActor(*pEditor->pPhysScene,false);

			if( pEditor->EnvObjects[i].pPhys )
				pEditor->EnvObjects[i].pPhys->SetTransform(pEditor->EnvObjects[i].mTransform);
		}
	}	

	pEditor->pPhysRagdoll->Clear();
	pEditor->DestroyPhys(&pEditor->RootBone);

	pEditor->pPhysScene->Update(api->GetDeltaTime());

	pEditor->RootBone.RagDollBone=&pEditor->pPhysRagdoll->GetRootBone();

	pEditor->CreatePhysRagDoll(&pEditor->RootBone);	

	pEditor->pPhysScene->Update(api->GetDeltaTime());

	array<byte> data(_FL_);
	pEditor->pPhysRagdoll->BuildSaveData(data);

	int len  = data.GetDataSize();
	int len2 = data.Size();

	IFileService *pFS = pEditor->Files();

	GUIFileSave *so = (GUIFileSave *)sender;

	string FileName = so->FileName;

	FileName.AddExtention(".rdb");

	pFS->SaveData(FileName.GetBuffer(),data.GetBuffer(),data.GetDataSize());

	for( int i = 0; i < pEditor->RagDollAnim->Bones ; i++ )
	{			
		for( int j = 0 ; j < pEditor->RagDollAnim->NumOptBones ; j++ )
		{
			if( string::IsEqual(pEditor->RagDollAnim->OptBonesNames[j].Name.c_str(),pEditor->RagDollAnim->Bones[i].Name.c_str()))
			{
				pEditor->RagDollAnim->matrices[j] = pEditor->RagDollAnim->Bones[i].InitMat;

				break;
			}
		}
	}

	pEditor->pPhysRagdoll->Clear();
	pEditor->CreatePhys(&pEditor->RootBone,false);

	pEditor->pPhysScene->Update(api->GetDeltaTime());

	pEditor->bOpenedDialog = false;
}

void _cdecl TMainWindow::OnCreateNewEnv (GUIControl* sender)
{
	pEditor->ENVProjectFileName="";

	for (dword i=0;i<pEditor->EnvObjects.Size();i++)
	{			
		if (pEditor->EnvObjects[i].pPhys!=NULL)
		{
			pEditor->EnvObjects[i].pPhys->Release();
			pEditor->EnvObjects[i].pPhys=NULL;
		}

		if (pEditor->EnvObjects[i].pGeom!=NULL)
		{
			pEditor->EnvObjects[i].pGeom->Release();
			pEditor->EnvObjects[i].pGeom=NULL;
		}
	}

	pEditor->EnvObjects.DelAll();
}

void _cdecl TMainWindow::OnOpenEnv (GUIControl* sender)
{
	pEditor->bOpenedDialog=true;

	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->Filters.Add (".env");
	fo->FiltersDesc.Add ("Env Data");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadEnv;
	fo->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	fo->StartDirectory = string(string (pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TMainWindow::SaveEnvPressed (GUIControl* sender)
{
	if (pEditor->ENVProjectFileName=="")
	{	
		pEditor->bOpenedDialog=true;

		GUIFileSave* save_dialog = NEW GUIFileSave ();
		save_dialog->Filters.Add(".env");
		save_dialog->FiltersDesc.Add("Env Data");
		save_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::SaveEnv;
		save_dialog->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
		save_dialog->StartDirectory = string(string (pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
		Application->ShowModal(save_dialog);
		save_dialog->Filter->SelectItem (1);
	}
	else
	{
		pEditor->SaveEnviroment(pEditor->ENVProjectFileName);

		pEditor->bOpenedDialog=false;

		SaveOKTime = SAVEDOK_SHOWTIME;
	}
}

void _cdecl TMainWindow::SaveAsEnvPressed (GUIControl* sender)
{
	pEditor->bOpenedDialog=true;

	GUIFileSave* save_dialog = NEW GUIFileSave ();
	save_dialog->Filters.Add(".env");
	save_dialog->FiltersDesc.Add("Env Data");
	save_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::SaveEnv;
	save_dialog->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	save_dialog->StartDirectory = string(string (pEditor->GetStartDir()) + "\\RagDoll\\").GetBuffer();
	Application->ShowModal(save_dialog);
	save_dialog->Filter->SelectItem (1);
}

void _cdecl TMainWindow::LoadEnv (GUIControl* sender)
{
	for (dword i=0;i<pEditor->EnvObjects.Size();i++)
	{			
		if (pEditor->EnvObjects[i].pPhys!=NULL)
		{
			pEditor->EnvObjects[i].pPhys->Release();
			pEditor->EnvObjects[i].pPhys=NULL;
		}

		if (pEditor->EnvObjects[i].pGeom!=NULL)
		{
			pEditor->EnvObjects[i].pGeom->Release();
			pEditor->EnvObjects[i].pGeom=NULL;
		}
	}

	pEditor->EnvObjects.DelAll();

	GUIFileOpen* fo = (GUIFileOpen*)sender;

	string FileName = fo->FileName;

	pEditor->ENVProjectFileName=FileName;
	

	pEditor->LoadEnviroment(FileName.c_str());

	pEditor->SelBone=NULL;
	pEditor->SelEnvObject=-1;

	pEditor->GetGizmo()->Enable(false);


	IGMXService* pGEOServ = (IGMXService*)api->GetService("GMXService");	

	for (dword i=0;i<pEditor->EnvObjects.Size();i++)
	{
		pEditor->EnvObjects[i].pGeom = pGEOServ->CreateGMX(pEditor->EnvObjects[i].ModelName, 0, NULL, NULL);

		pEditor->EnvObjects[i].pGeom->SetTransform(pEditor->EnvObjects[i].mTransform);

		pEditor->EnvObjects[i].pPhys = pEditor->EnvObjects[i].pGeom->CreatePhysicsActor(*pEditor->pPhysScene, false);

		if (pEditor->EnvObjects[i].pPhys!=NULL)
		{
			pEditor->EnvObjects[i].pPhys->SetTransform(pEditor->EnvObjects[i].mTransform);
		}

		pEditor->SelEnvObject= pEditor->EnvObjects.Size()-1;		
	}

	pEditor->SelEnvObject=-1;

	pEditor->bOpenedDialog=false;
}

void _cdecl TMainWindow::SaveEnv (GUIControl* sender)
{
	GUIFileSave* so = (GUIFileSave*)sender;

	string FileName = so->FileName;

	FileName.AddExtention(".env");
	
	pEditor->SaveEnviroment(FileName);

	pEditor->ENVProjectFileName=FileName;

	SaveOKTime = SAVEDOK_SHOWTIME;

	pEditor->bOpenedDialog=false;
}

void _cdecl TMainWindow::btnMovePressed (GUIControl* sender)
{
	//pEditor->GetGizmo()->Enable(true);
	pEditor->GetGizmo()->SetType(RGDTransformGizmo::GT_MOVE);

//	bGizmoMode->SelectItem(GZM_move_index);

	btnWorld->Down = (GZM_move_index == 0);
	btnLocal->Down = (GZM_move_index == 1);

	pEditor->GetGizmo()->SetMode((RGDTransformGizmo::GizmoMode)GZM_move_index);
}

void _cdecl TMainWindow::btnRotatePressed (GUIControl* sender)
{
	//pEditor->GetGizmo()->Enable(true);
	pEditor->GetGizmo()->SetType(RGDTransformGizmo::GT_ROTATE);

//	bGizmoMode->SelectItem(GZM_rotate_index);

	btnWorld->Down = (GZM_rotate_index == 0);
	btnLocal->Down = (GZM_rotate_index == 1);

	pEditor->GetGizmo()->SetMode((RGDTransformGizmo::GizmoMode)GZM_rotate_index);
}
/*
void _cdecl TMainWindow::btnGizmoTypeChanged (GUIControl* sender)
{
	if (bGizmoMode->ListBox->SelectedLine == 0)
	{
		pEditor->GetGizmo()->SetMode(RGDTransformGizmo::GM_WORLD);
	} else
	{
		pEditor->GetGizmo()->SetMode(RGDTransformGizmo::GM_LOCAL);
	}

	if (pEditor->GetGizmo()->GetType() == RGDTransformGizmo::GT_ROTATE)
	{
		GZM_rotate_index = bGizmoMode->ListBox->SelectedLine;
	} else
	{
		GZM_move_index = bGizmoMode->ListBox->SelectedLine;
	}

}
*/
void _cdecl TMainWindow::btnWorldPressed(GUIControl *sender)
{
	pEditor->GetGizmo()->SetMode(RGDTransformGizmo::GM_WORLD);

	if( pEditor->GetGizmo()->GetType() == RGDTransformGizmo::GT_ROTATE )
	{
		GZM_rotate_index = 0;
	}
	else
	{
		GZM_move_index = 0;
	}
}

void _cdecl TMainWindow::btnLocalPressed(GUIControl *sender)
{
	pEditor->GetGizmo()->SetMode(RGDTransformGizmo::GM_LOCAL);

	if( pEditor->GetGizmo()->GetType() == RGDTransformGizmo::GT_ROTATE )
	{
		GZM_rotate_index = 1;
	}
	else
	{
		GZM_move_index = 1;
	}
}

void _cdecl TMainWindow::btnLoadGMXPressed (GUIControl* sender)
{
	pEditor->bOpenedDialog=true;

	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->bShowReadOnly = true;
	fo->Filters.Add (".gmx");
	fo->FiltersDesc.Add ("Geometry file");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadGMXFile;
	fo->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	fo->StartDirectory = string(string (pEditor->GetStartDir()) + "\\resource\\models\\").GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void TMainWindow::OnChangePhysParams (GUIControl* sender)
{
	pEditor->SelBone->fMass=(float)atof(pBoneMass->Text.c_str());

	pEditor->SelBone->fSpringA=(float)atof(pBoneSpringA->Text.c_str());
	pEditor->SelBone->fDamperA=(float)atof(pBoneDamperA->Text.c_str());
	
	pEditor->SelBone->fSpringB=(float)atof(pBoneSpringB->Text.c_str());
	pEditor->SelBone->fDamperB=(float)atof(pBoneDamperB->Text.c_str());	
}

void _cdecl TMainWindow::btnShowGMXModel (GUIControl* sender)
{
	ShowGeom = !ShowGeom;
}

void _cdecl TMainWindow::ShowSkeletonPressed (GUIControl* sender)
{
	ShowSkel = !ShowSkel;
}

void _cdecl TMainWindow::DrawOrderPressed (GUIControl* sender)
{
	drawOrder = !drawOrder;
}

void _cdecl TMainWindow::DrawPowerPressed (GUIControl* sender)
{
	if( pEditor->alphaGeom > 0.7f )
		pEditor->alphaGeom = 0.5f;
	else
		pEditor->alphaGeom = 1.0f;
}

void _cdecl TMainWindow::AutoLookPressed (GUIControl* sender)
{
	autoLook = !autoLook;
}

void _cdecl TMainWindow::ShowDebugGeom (GUIControl* sender)
{
	pEditor->bRenderDebugGeom=!pEditor->bRenderDebugGeom;
}

void _cdecl TMainWindow::ToAttachBoneMode (GUIControl* sender)
{
	pEditor->bAttachBoneMode=!pEditor->bAttachBoneMode;

	if (!pEditor->bAttachBoneMode)
	{
		pEditor->SetWorldMatrixAsLocal(&pEditor->RootBone);
	}
	else
	{
		pEditor->SetLocalMatrixAsWorld(&pEditor->RootBone);
	}

	if (pEditor->SelBone!=NULL)
	{
		if (pEditor->bAttachBoneMode)
		{
			pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mGlobal);
			pEditor->GetGizmo()->CalcGizmoScale();
		}
		else
		{
			pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mWorldGlobal);
			pEditor->GetGizmo()->CalcGizmoScale();
		}
	}	
}

void _cdecl TMainWindow::LoadGMXFile (GUIControl* sender)
{
	GUIFileOpen* fo = (GUIFileOpen*)sender;

	string CropPath = string(pEditor->GetStartDir()) + "\\resource\\models\\";
	string ModelFileName = fo->FileName;
	ModelFileName.GetRelativePath(CropPath);

	LoadGMX (ModelFileName);

	pEditor->bOpenedDialog=false;
}

void TMainWindow::LoadGMX (string ModelFileName)
{
	if (pEditor->pGeom)
	{
		pEditor->pGeom->Release ();

		pEditor->pGeom=NULL;

		if (pEditor->RagDollAnim->matrices)
		{
			delete []pEditor->RagDollAnim->matrices;
			pEditor->RagDollAnim->matrices=NULL;
		}

		if (pEditor->RagDollAnim->OptBonesNames)
		{
			delete []pEditor->RagDollAnim->OptBonesNames;
			pEditor->RagDollAnim->OptBonesNames=NULL;
		}

        pEditor->RagDollAnim->Bones.DelAll();
	}	

	IGMXService* pGEOServ = (IGMXService*)api->GetService("GMXService");

	pEditor->pGeom = pGEOServ->CreateGMX(ModelFileName.c_str(), 0, NULL, NULL);


	if (pEditor->pAnim)
	{	
		pEditor->pAnim->Release();
		pEditor->pAnim = null;
	}

	if (pEditor->pBones)
	{
		delete []pEditor->pBones;
		pEditor->pBones = NULL;
	}

	if (pEditor->pGeom)
	{	
		pEditor->GMXModelName=ModelFileName;

		//IGMXIterator* pIterator;


		boneDataReadOnly * dataPtr = null;
		boneMtxInputReadOnly * mtxPtr = null;
		dword masterSkeletonBonesCount = pEditor->pGeom->GetBonesArray(&dataPtr, &mtxPtr);

		for (dword i=0;i< masterSkeletonBonesCount;i++)
		{				
			//pBone->mInitial
			Matrix m1 = mtxPtr[i].mtxBindPose;

			m1.Inverse();

			RDBone RBone;

			RBone.InitMat=m1;
			RBone.Name=dataPtr[i].name.c_str();

			pEditor->RagDollAnim->Bones.Add(RBone);
		}


		pEditor->RagDollAnim->matrices = NEW Matrix[pEditor->RagDollAnim->Bones.Size()];
		pEditor->RagDollAnim->OptBonesNames = NEW TOptBonesName[pEditor->RagDollAnim->Bones.Size()];
		
		pEditor->pBones = NEW IAnimationScene::Bone[pEditor->RagDollAnim->Bones.Size()];

		/*for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
		{
			pEditor->RagDollAnim->matrices[i]=pEditor->RagDollAnim->Bones[i].InitMat;

			int parent=	pEditor->RagDollAnim->GetBoneParent(i);

			if (parent!=-1)
			{
				Matrix inv_mat=pEditor->RagDollAnim->Bones[parent].InitMat;

				inv_mat.Inverse();

				pEditor->RagDollAnim->Bones[i].LocalMat=pEditor->RagDollAnim->Bones[i].InitMat * inv_mat;
			}
			else
			{
				pEditor->RagDollAnim->Bones[i].LocalMat=pEditor->RagDollAnim->Bones[i].InitMat;
			}

			pEditor->RagDollAnim->Bones[i].parent=parent;

			pEditor->RagDollAnim->Bones[i].NeedCalclFinalMat=true;

			pEditor->RagDollAnim->OptBonesNames[i].Name = pEditor->RagDollAnim->Bones[i].Name;
		}

		pEditor->RagDollAnim->NumOptBones = pEditor->RagDollAnim->Bones.Size();

		//pEditor->SkinRagDollAnimBoneMatrix(&pEditor->RootBone);

		//pEditor->pGeom->SetAnimation((IAnimation*)pEditor->RagDollAnim);	
		*/				
		
		int index = 0;
		int nedeed_parent=-1;

		for (dword i=0;i<pEditor->RagDollAnim->Bones.Size()-1;i++)
		{
			for (dword j=index;j<pEditor->RagDollAnim->Bones.Size();j++)
			{
				int parent=	pEditor->RagDollAnim->GetBoneParent(j);

				if (parent==nedeed_parent)
				{
					RDBone tmp_bone = pEditor->RagDollAnim->Bones[j];
					pEditor->RagDollAnim->Bones[j] = pEditor->RagDollAnim->Bones[index];
					pEditor->RagDollAnim->Bones[index] = tmp_bone;

					index++;
				}				
			}

			nedeed_parent++;
		}

		for (dword i=0;i<pEditor->RagDollAnim->Bones.Size();i++)
		{
			pEditor->RagDollAnim->matrices[i]=pEditor->RagDollAnim->Bones[i].InitMat;			


			int parent=	pEditor->RagDollAnim->GetBoneParent(i);

			if (parent!=-1)
			{
				Matrix inv_mat=pEditor->RagDollAnim->Bones[parent].InitMat;

				inv_mat.Inverse();

				pEditor->RagDollAnim->Bones[i].LocalMat=pEditor->RagDollAnim->Bones[i].InitMat * inv_mat;
			}
			else
			{
				pEditor->RagDollAnim->Bones[i].LocalMat=pEditor->RagDollAnim->Bones[i].InitMat;
			}

			pEditor->RagDollAnim->Bones[i].parent=parent;
			pEditor->RagDollAnim->Bones[i].NeedCalclFinalMat=true;
			pEditor->RagDollAnim->OptBonesNames[i].Name = pEditor->RagDollAnim->Bones[i].Name;

			pEditor->pBones[i].name = pEditor->RagDollAnim->Bones[i].Name;	
			pEditor->pBones[i].parentIndex=parent;			
			pEditor->pBones[i].mtx = pEditor->RagDollAnim->Bones[i].InitMat;
			pEditor->pBones[i].mtx.pos = pEditor->RagDollAnim->Bones[i].LocalMat.pos;
		}

		pEditor->pAnim = pEditor->pAnimScene->CreateProcedural(pEditor->pBones,pEditor->RagDollAnim->Bones.Size(), _FL_);
		pEditor->RagDollAnim->NumOptBones = pEditor->RagDollAnim->Bones.Size();

		//pEditor->SkinRagDollAnimBoneMatrix(&pEditor->RootBone);

		//pEditor->pGeom->SetAnimation((IAnimation*)pEditor->RagDollAnim);			
		pEditor->pGeom->SetAnimation(pEditor->pAnim);


		//LoadSceletonFromAnt(string(pEditor->GetStartDir()) + "\\resource\\models\\" + "Idle.ant");

		//pEditor->pGeom->SetAnimationFile(string(pEditor->GetStartDir()) + "\\resource\\models\\" + "pit.anx");
		//pEditor->pGeom->a
	}		
}

void _cdecl TMainWindow::btnLoadEnvGMXPressed (GUIControl* sender)
{
	if (pEditor->SelEnvObject==-1) return;

	pEditor->bOpenedDialog=true;

	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->Filters.Add (".gmx");
	fo->FiltersDesc.Add ("Geometry file");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadEnvGMXFile;
	fo->OnCancel = (CONTROL_EVENT)&TMainWindow::OnFCancel;
	fo->StartDirectory = string(string (pEditor->GetStartDir()) + "\\resource\\models\\").GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TMainWindow::LoadEnvGMXFile (GUIControl* sender)
{
	GUIFileOpen* fo = (GUIFileOpen*)sender;

	string CropPath = string(pEditor->GetStartDir()) + "\\resource\\models\\";
	string ModelFileName = fo->FileName;
	ModelFileName.GetRelativePath(CropPath);

	
	
	
	if (pEditor->EnvObjects[pEditor->SelEnvObject].pPhys!=NULL)
	{
		pEditor->EnvObjects[pEditor->SelEnvObject].pPhys->Release();
		pEditor->EnvObjects[pEditor->SelEnvObject].pPhys=NULL;
	}

	if (pEditor->EnvObjects[pEditor->SelEnvObject].pGeom!=NULL)
	{
		pEditor->EnvObjects[pEditor->SelEnvObject].pGeom->Release();
		pEditor->EnvObjects[pEditor->SelEnvObject].pGeom=NULL;
	}

	IGMXService* pGEOServ = (IGMXService*)api->GetService("GMXService");
	pEditor->EnvObjects[pEditor->SelEnvObject].pGeom = pGEOServ->CreateGMX(ModelFileName.c_str(), 0, NULL, NULL);

	pEditor->EnvObjects[pEditor->SelEnvObject].pPhys = pEditor->EnvObjects[pEditor->SelEnvObject].pGeom->CreatePhysicsActor(*pEditor->pPhysScene, false);

	if (pEditor->EnvObjects[pEditor->SelEnvObject].pPhys!=NULL)
	{	
		pEditor->EnvObjects[pEditor->SelEnvObject].pPhys->SetTransform(pEditor->EnvObjects[pEditor->SelEnvObject].mTransform);	
	}


	pEditor->EnvObjects[pEditor->SelEnvObject].ModelName=ModelFileName;

	
	pEditor->bOpenedDialog=false;
}

void _cdecl TMainWindow::OnFCancel (GUIControl* sender)
{
	pEditor->bOpenedDialog=false;
}
