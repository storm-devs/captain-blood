#ifndef _XBOX
//****************************************************************


#include "mainwindow.h"
#include "rename.h"
#include "projectbrowser.h"
#include "..\EditorMain.h"
#include "..\..\..\common_h\particles.h"
#include "..\..\..\Common_h\FileService.h"
#include "..\..\..\common_h\gmx.h"
#include "..\arcball\ArcBall.h"
#include "..\..\manager\particlemanager.h"
#include "..\..\system\ParticleSystem\particlesystem.h"
#include "..\..\icommon\iemitter.h"
#include "..\..\icommon\names.h"
#include "..\..\icommon\editortypes.h"

#include "..\..\system\datasource\databool.h"
#include "..\..\system\datasource\datacolor.h"
#include "..\..\system\datasource\datafloat.h"
#include "..\..\system\datasource\datagraph.h"
#include "..\..\system\datasource\dataposition.h"
#include "..\..\system\datasource\datauv.h"
#include "..\..\system\datasource\datastring.h"


#include "..\editors\graph.h"
#include "..\editors\uv.h"
#include "..\editors\color.h"
#include "..\editors\bool.h"
#include "..\editors\float.h"
#include "..\editors\position.h"
#include "..\editors\string.h"
#include "..\customcontrols\strutil.h"


#include "choosename.h"


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
extern ParticleEditor* pEditor;


string __tmp;


bool TMainWindow::FuncCompare (GUITreeNode* const &a1, GUITreeNode* const &a2)
{
	if (crt_stricmp(a1->GetText(), a2->GetText()) < 0) return true;
	return false;
}





TMainWindow::TMainWindow () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT), UndoEntry(_FL_),
																																																			 ExpandedNode(_FL_)
{
	fTimeAfterRestart = 999.0f;
	bLowFPS = false;
	TimeFromLastAutoSave = 0.0f;
	AutoSaveIndex = 0;
	MaxAutoSaveSlots = 20;


	bDrawGrid = true;

	pGeom = NULL;


	GZM_move_index = 0;
	GZM_rotate_index = 1;

	bShowAxis = false;
	SystemIsPaused = false;
	SystemIsSlowMode = false;
	SaveOKTime = 0;
	UndoSelectNode = false;
	PositionInUndoBuffer = -1;
	NowEdited = NULL;

	pBackgroundTexture = NULL;


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IIniFile* pIni = pFS->SystemIni();
	Assert(pIni);

	const char* szBackground = pIni->GetString("peditor", "backgroundname", NULL);
	if (szBackground)
	{
		pBackgroundTexture = pEditor->Render()->CreateTexture(_FL_, szBackground);

		if (pBackgroundTexture->GetWidth() <= 4 && pBackgroundTexture->GetHeight() <= 4)
		{
			pBackgroundTexture->Release();
			pBackgroundTexture = NULL;
		}
	}




	Matrix mGizmoTransform;



	mGizmoTransform.m[0][0] = pIni->GetFloat("peditor", "m_0_0", 1.0f);
	mGizmoTransform.m[0][1] = pIni->GetFloat("peditor", "m_1_0", 0.0f);
	mGizmoTransform.m[0][2] = pIni->GetFloat("peditor", "m_2_0", 0.0f);

	mGizmoTransform.m[1][0] = pIni->GetFloat("peditor", "m_0_1", 0.0f);
	mGizmoTransform.m[1][1] = pIni->GetFloat("peditor", "m_1_1", 1.0f);
	mGizmoTransform.m[1][2] = pIni->GetFloat("peditor", "m_2_1", 0.0f);

	mGizmoTransform.m[2][0] = pIni->GetFloat("peditor", "m_0_2", 0.0f);
	mGizmoTransform.m[2][1] = pIni->GetFloat("peditor", "m_1_2", 0.0f);
	mGizmoTransform.m[2][2] = pIni->GetFloat("peditor", "m_2_2", 1.0f);

	mGizmoTransform.m[3][0] = pIni->GetFloat("peditor", "m_0_3", 0.0f);
	mGizmoTransform.m[3][1] = pIni->GetFloat("peditor", "m_1_3", 0.0f);
	mGizmoTransform.m[3][2] = pIni->GetFloat("peditor", "m_2_3", 0.0f);



	pEditor->GetGizmo()->SetTransform(mGizmoTransform);




	
	pIni->Release();

	const RENDERSCREEN& ScreenInfo = pEditor->Render()->GetScreenInfo3D();
	SetWidth(ScreenInfo.dwWidth);
	SetHeight(ScreenInfo.dwHeight);

//	pArcBall.SetRadius(10.0f);
//	SetWidth(pRS->GetScreenInfo3D().dwWidth);
//	SetHeight(pRS->GetScreenInfo3D().dwHeight);

	bPopupStyle = true;
	bSystemButton = false;
	Caption = "Particles editor";
	pFont->SetSize (16);

  TreeView1 = NEW GUITreeView (this, 5, 4, 208, DrawRect.h-20);
	TreeView1->FontColor = 0xFF000000;
	TreeView1->SetImagesArray ("gtree");
	TreeView1->Items->Clear ();
	TreeView1->bDragAndDrop = false;
	TreeView1->pFont->SetName("arialcyrsmall");
	TreeView1->MakeFlat(true);
	TreeView1->OnDblClick = (CONTROL_EVENT)&TMainWindow::ShowHideParticle;
	TreeView1->OnChange = (CONTROL_EVENT)&TMainWindow::TreeViewChangeNode;

	btnNewParticleSystem= NEW GUIButton(this, 218, 5, 24, 24);
	btnNewParticleSystem->Glyph->Load ("peditor\\create");
	btnNewParticleSystem->FontColor = 0xFF000000;
	btnNewParticleSystem->pFont->SetSize (16);
	btnNewParticleSystem->Hint = "Create empty system (Ctrl+N)";
	btnNewParticleSystem->FlatButton = true;
	btnNewParticleSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OnCreateNewSystem;
	

	btnOpenParticleSystem= NEW GUIButton(this, 218+27, 5, 24, 24);
	btnOpenParticleSystem->Glyph->Load ("peditor\\open");
	btnOpenParticleSystem->FontColor = 0xFF000000;
	btnOpenParticleSystem->pFont->SetSize (16);
	btnOpenParticleSystem->Hint = "Open existing particle system (Ctrl+O)";
	btnOpenParticleSystem->FlatButton = true;
	btnOpenParticleSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::OpenNewSystemPressed;

	

	
	btnSaveParticleSystem= NEW GUIButton(this, 218+(27*2), 5, 24, 24);
	btnSaveParticleSystem->Glyph->Load ("peditor\\save");
	btnSaveParticleSystem->DisabledGlyph->Load ("disabledsave");
	btnSaveParticleSystem->FontColor = 0xFF000000;
	btnSaveParticleSystem->pFont->SetSize (16);
	btnSaveParticleSystem->Hint = "Save particle system (Ctrl+S)";
	btnSaveParticleSystem->FlatButton = true;
	//btnSaveParticleSystem->bEnabled = false;
	btnSaveParticleSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SavePressed;

	btnSaveAsParticleSystem= NEW GUIButton(this, 218+(27*3), 5, 24, 24);
	btnSaveAsParticleSystem->Glyph->Load ("peditor\\saveas");
	btnSaveAsParticleSystem->FontColor = 0xFF000000;
	btnSaveAsParticleSystem->pFont->SetSize (16);
	btnSaveAsParticleSystem->Hint = "Save as... (Ctrl+Shift+S)";
	btnSaveAsParticleSystem->FlatButton = true;
	btnSaveAsParticleSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SaveAsPressed;

	btnCreatePointEmitter = NEW GUIButton(this, 218+5+(27*4), 5, 24, 24);
	btnCreatePointEmitter->Glyph->Load ("peditor\\addpoint");
	btnCreatePointEmitter->FontColor = 0xFF000000;
	btnCreatePointEmitter->pFont->SetSize (16);
	btnCreatePointEmitter->Hint = "Create point emitter (Alt+P)";
	btnCreatePointEmitter->FlatButton = true;
	btnCreatePointEmitter->OnMousePressed = (CONTROL_EVENT)&TMainWindow::CreatePointEmitterPressed;
	
/*
	btnCreateBoxEmitter = NEW GUIButton(this, 218+5+(27*5), 5, 24, 24);
	btnCreateBoxEmitter->Glyph->Load ("peditor\\addbox");
	btnCreateBoxEmitter->DisabledGlyph->Load ("peditor\\addboxd");
	btnCreateBoxEmitter->FontColor = 0xFF000000;
	btnCreateBoxEmitter->pFont->SetSize (16);
	btnCreateBoxEmitter->Hint = "Create box emitter (Alt+B)";
	btnCreateBoxEmitter->FlatButton = true;
	btnCreateBoxEmitter->bEnabled = false;

	btnCreateSphereEmitter = NEW GUIButton(this, 218+5+(27*6), 5, 24, 24);
	btnCreateSphereEmitter->Glyph->Load ("peditor\\addsphere");
	btnCreateSphereEmitter->DisabledGlyph->Load ("peditor\\addsphered");
	btnCreateSphereEmitter->FontColor = 0xFF000000;
	btnCreateSphereEmitter->pFont->SetSize (16);
	btnCreateSphereEmitter->Hint = "Create sphere emitter (Alt+S)";
	btnCreateSphereEmitter->FlatButton = true;
	btnCreateSphereEmitter->bEnabled = false;

	btnCreateLineEmitter = NEW GUIButton(this, 218+5+(27*7), 5, 24, 24);
	btnCreateLineEmitter->Glyph->Load ("peditor\\addline");
	btnCreateLineEmitter->DisabledGlyph->Load ("peditor\\addlined");
	btnCreateLineEmitter->FontColor = 0xFF000000;
	btnCreateLineEmitter->pFont->SetSize (16);
	btnCreateLineEmitter->Hint = "Create line emitter";
	btnCreateLineEmitter->FlatButton = true;
	btnCreateLineEmitter->bEnabled = false;
	
	btnCreateModelEmitter = NEW GUIButton(this, 218+5+(27*8), 5, 24, 24);
	btnCreateModelEmitter->Glyph->Load ("peditor\\addmodel");
	btnCreateModelEmitter->DisabledGlyph->Load ("peditor\\addmodeld");
	btnCreateModelEmitter->FontColor = 0xFF000000;
	btnCreateModelEmitter->pFont->SetSize (16);
	btnCreateModelEmitter->Hint = "Create model emitter (Alt+M)";
	btnCreateModelEmitter->FlatButton = true;
	btnCreateModelEmitter->bEnabled = false;
*/

	btnDelete = NEW GUIButton(this, 218+15+(27*5), 5, 24, 24);
	btnDelete->Glyph->Load ("peditor\\delete");
	btnDelete->FontColor = 0xFF000000;
	btnDelete->pFont->SetSize (16);
	btnDelete->Hint = "Delete item (Ctrl+Del)";
	btnDelete->FlatButton = true;
	btnDelete->OnMousePressed = (CONTROL_EVENT)&TMainWindow::DeleteObjectPressed;

	

	btnCreateBillBoardParticle = NEW GUIButton(this, 218+15+(27*6), 5, 24, 24);
	btnCreateBillBoardParticle->Glyph->Load ("peditor\\addbillboard");
	btnCreateBillBoardParticle->FontColor = 0xFF000000;
	btnCreateBillBoardParticle->pFont->SetSize (16);
	btnCreateBillBoardParticle->Hint = "Create billboard particle\nin selected emitter (Alt+B)";
	btnCreateBillBoardParticle->FlatButton = true;
	btnCreateBillBoardParticle->OnMousePressed = (CONTROL_EVENT)&TMainWindow::CreateBillboardPressed;

	btnCreateModelParticle = NEW GUIButton(this, 218+15+(27*7), 5, 24, 24);
	btnCreateModelParticle->Glyph->Load ("peditor\\addmpart");
	btnCreateModelParticle->FontColor = 0xFF000000;
	btnCreateModelParticle->pFont->SetSize (16);
	btnCreateModelParticle->Hint = "Create model particle\nin selected emitter (Alt+M)";
	btnCreateModelParticle->FlatButton = true;
	btnCreateModelParticle->OnMousePressed = (CONTROL_EVENT)&TMainWindow::CreateModelPressed;

	
	

	btnShowGrid = NEW GUIButton(this, 218+20+(27*8), 5, 24, 24);
	btnShowGrid->Glyph->Load ("peditor\\grid");
	btnShowGrid->DisabledGlyph->Load ("peditor\\grid");
	btnShowGrid->FontColor = 0xFF000000;
	btnShowGrid->pFont->SetSize (16);
	btnShowGrid->Hint = "Show grid";
	btnShowGrid->FlatButton = true;
	btnShowGrid->GroupIndex = 33;
	btnShowGrid->Down = true;
	btnShowGrid->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnShowGridPressed;

	

	btnShowDirection = NEW GUIButton(this, 218+20+(27*9), 5, 24, 24);
	btnShowDirection->Glyph->Load ("peditor\\show_dir");
	btnShowDirection->DisabledGlyph->Load ("peditor\\show_dird");
	btnShowDirection->FontColor = 0xFF000000;
	btnShowDirection->pFont->SetSize (16);
	btnShowDirection->Hint = "Show emission direction\nOn current emitter";
	btnShowDirection->FlatButton = true;
	btnShowDirection->GroupIndex = 1;
	btnShowDirection->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnShowDirPressed;


	btnRenameObject = NEW GUIButton(this, 218+20+(27*10), 5, 24, 24);
	btnRenameObject->Glyph->Load ("peditor\\rename");
	btnRenameObject->FontColor = 0xFF000000;
	btnRenameObject->pFont->SetSize (16);
	btnRenameObject->Hint = "Rename current object (F6)";
	btnRenameObject->FlatButton = true;
	btnRenameObject->OnMousePressed = (CONTROL_EVENT)&TMainWindow::RenameObjectPressed;

	btnEmulateLOWFps = NEW GUIButton(this, 218+10+(27*12), 5, 24, 24);
	btnEmulateLOWFps->Glyph->Load ("peditor\\undo");
	btnEmulateLOWFps->DisabledGlyph->Load ("peditor\\undod");
	btnEmulateLOWFps->FontColor = 0xFF000000;
	btnEmulateLOWFps->pFont->SetSize (16);
	btnEmulateLOWFps->Hint = "Undo (Ctrl+Z)";
	btnEmulateLOWFps->FlatButton = true;
	btnEmulateLOWFps->OnMousePressed = (CONTROL_EVENT)&TMainWindow::UndoPressed;

	btnSelectAmbientColor = NEW GUIButton(this, 218+10+(27*13), 5, 24, 24);
	btnSelectAmbientColor->Glyph->Load ("peditor\\colorch");
	btnSelectAmbientColor->DisabledGlyph->Load ("peditor\\colorch");
	btnSelectAmbientColor->FontColor = 0xFF000000;
	btnSelectAmbientColor->pFont->SetSize (16);
	btnSelectAmbientColor->Hint = "Select ambient color";
	btnSelectAmbientColor->FlatButton = true;
	btnSelectAmbientColor->OnMousePressed = (CONTROL_EVENT)&TMainWindow::AmbientSelect;
	//btnSelectAmbientColor->bEnabled = false;



	btnSelectDirectionColor = NEW GUIButton(this, 218+10+(27*14), 5, 24, 24);
	btnSelectDirectionColor->Glyph->Load ("peditor\\colorch");
	btnSelectDirectionColor->DisabledGlyph->Load ("peditor\\colorch");
	btnSelectDirectionColor->FontColor = 0xFF000000;
	btnSelectDirectionColor->pFont->SetSize (16);
	btnSelectDirectionColor->Hint = "Select directional color";
	btnSelectDirectionColor->FlatButton = true;
	btnSelectDirectionColor->OnMousePressed = (CONTROL_EVENT)&TMainWindow::DirectionalSelect;





	btnRestartSystem = NEW GUIButton(this, 218+30+(27*15), 5, 24, 24);
	btnRestartSystem->Glyph->Load ("peditor\\restart");
	btnRestartSystem->FontColor = 0xFF000000;
	btnRestartSystem->pFont->SetSize (16);
	btnRestartSystem->Hint = "Restart system (Ctrl+P)";
	btnRestartSystem->FlatButton = true;
	btnRestartSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::RestartSystem;
	//btnRestartSystem->GroupIndex = 33;

	btnPauseSystem = NEW GUIButton(this, 218+30+(27*16), 5, 24, 24);
	btnPauseSystem->Glyph->Load ("peditor\\pause");
	btnPauseSystem->FontColor = 0xFF000000;
	btnPauseSystem->pFont->SetSize (16);
	btnPauseSystem->Hint = "Pause system (Ctrl+I)";
	btnPauseSystem->FlatButton = true;
	btnPauseSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::PausePressed;
	btnPauseSystem->GroupIndex = 33;

	btnSlowMotionSystem = NEW GUIButton(this, 218+30+(27*17), 5, 24, 24);
	btnSlowMotionSystem->Glyph->Load ("peditor\\slowmo");
	btnSlowMotionSystem->FontColor = 0xFF000000;
	btnSlowMotionSystem->pFont->SetSize (16);
	btnSlowMotionSystem->Hint = "Enable SlowMotion (Ctrl+O)";
	btnSlowMotionSystem->FlatButton = true;
	btnSlowMotionSystem->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SlowMotionPressed;
	btnSlowMotionSystem->GroupIndex = 33;

	btnSelectBackgroundTexture = NEW GUIButton(this, 218+30+15+(27*18), 5, 24, 24);
	btnSelectBackgroundTexture->Glyph->Load ("peditor\\selectback");
	btnSelectBackgroundTexture->FontColor = 0xFF000000;
	btnSelectBackgroundTexture->pFont->SetSize (16);
	btnSelectBackgroundTexture->Hint = "Select background texture";
	btnSelectBackgroundTexture->FlatButton = true;
	btnSelectBackgroundTexture->OnMousePressed = (CONTROL_EVENT)&TMainWindow::SelectBackgroundPressed;
	//btnSelectBackgroundTexture->GroupIndex = 33;

	
	btnSimpleMode = NEW GUIButton(this, 218+38+15+(27*19), 5, 24, 24);
	btnSimpleMode->Glyph->Load ("peditor\\m_simple");
	btnSimpleMode->FontColor = 0xFF000000;
	btnSimpleMode->pFont->SetSize (16);
	btnSimpleMode->Hint = "Simple mode";
	btnSimpleMode->FlatButton = true;
	btnSimpleMode->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnSimplePressed;
	btnSimpleMode->GroupIndex = 33;
	btnSimpleMode->Down = true;

	btnMoveMode = NEW GUIButton(this, 218+38+15+(27*20), 5, 24, 24);
	btnMoveMode->Glyph->Load ("peditor\\m_move");
	btnMoveMode->FontColor = 0xFF000000;
	btnMoveMode->pFont->SetSize (16);
	btnMoveMode->Hint = "Move geometry mode";
	btnMoveMode->FlatButton = true;
	btnMoveMode->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnMovePressed;
	btnMoveMode->GroupIndex = 33;

	btnRotateMode = NEW GUIButton(this, 218+38+15+(27*21), 5, 24, 24);
	btnRotateMode->Glyph->Load ("peditor\\m_rotate");
	btnRotateMode->FontColor = 0xFF000000;
	btnRotateMode->pFont->SetSize (16);
	btnRotateMode->Hint = "Rotate geometry mode";
	btnRotateMode->FlatButton = true;
	btnRotateMode->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnRotatePressed;
	btnRotateMode->GroupIndex = 33;


	bGizmoMode = NEW GUIComboBox(this, 218+38+15+(27*22), 10, 100, 16);
	bGizmoMode->ListBox->Items.Add("World");
	bGizmoMode->ListBox->Items.Add("Local");
	bGizmoMode->MakeFlat(true);
	bGizmoMode->OnChange = (CONTROL_EVENT)&TMainWindow::btnGizmoTypeChanged;
	bGizmoMode->SelectItem(0);


	btnLoadGeom = NEW GUIButton(this, 218+38+15+(27*26), 5, 24, 24);
	btnLoadGeom->Glyph->Load ("peditor\\m_gmx");
	btnLoadGeom->FontColor = 0xFF000000;
	btnLoadGeom->pFont->SetSize (16);
	btnLoadGeom->Hint = "Load GMX file";
	btnLoadGeom->FlatButton = true;
	btnLoadGeom->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnLoadGMXPressed;
	
	btnResetTransform = NEW GUIButton(this, 218+38+15+(27*27), 5, 24, 24);
	btnResetTransform->Glyph->Load ("peditor\\m_reset");
	btnResetTransform->FontColor = 0xFF000000;
	btnResetTransform->pFont->SetSize (16);
	btnResetTransform->Hint = "Reset transform";
	btnResetTransform->FlatButton = true;
	btnResetTransform->OnMousePressed = (CONTROL_EVENT)&TMainWindow::btnResetTransformPressed;



//	
	


	ViewPortWidth = DrawRect.w - 230;
	ViewPortHeight = (int)(ViewPortWidth / 1.8f);
	ViewPort1 = NEW GUIViewPort (this, 220, 40, ViewPortWidth, ViewPortHeight);
	ViewPort1->OnRender = (CONTROL_EVENT)&TMainWindow::DrawViewport;

	int GraphEditorHeight = DrawRect.h - ViewPortHeight - 65;
	Panel1 = NEW GUIPanel (this, 220, ViewPortHeight+40+5, ViewPortWidth, GraphEditorHeight);

	BuildBrowserTree ();

	MaxUndoPos = 0;
	InitUndoSystem (128);



	IIniFile* pEngineIni = pFS->SystemIni();
	if (pEngineIni)
	{
		Color amb;
		amb.r = pEngineIni->GetFloat("Ambient", "R", 1.0f);
		amb.g = pEngineIni->GetFloat("Ambient", "G", 1.0f);
		amb.b = pEngineIni->GetFloat("Ambient", "B", 1.0f);

		Color dir;
		dir.r = pEngineIni->GetFloat("Directional", "R", 1.0f);
		dir.g = pEngineIni->GetFloat("Directional", "G", 1.0f);
		dir.b = pEngineIni->GetFloat("Directional", "B", 1.0f);




		IRender* pRS = (IRender*)api->GetService("DX9Render");

		pRS->SetAmbient(amb);


		pRS->SetGlobalLight(Vector(0.0f, 1.0f, 0.0f), false, dir, Color(1.0f, 1.0f, 1.0f));



		pEngineIni->Release();
		pEngineIni = NULL;
	}


	CreateUndo ();
}

TMainWindow::~TMainWindow ()
{
	if (pBackgroundTexture) pBackgroundTexture->Release();
	pBackgroundTexture = NULL;
	ShutdownUndoSystem ();
	//TreeView1->Items->Clear();
}

void TMainWindow::Draw ()
{
	if (api->DebugKeyState(VK_CONTROL, 'O'))
	{
		OpenNewSystemPressed(NULL);
	}


	fTimeAfterRestart += api->GetDeltaTime();
	if (bLowFPS)
	{
		Sleep(60);
	}
	Autosave();
	GUIWindow::Draw ();
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


	if (GetAsyncKeyState('Q') < 0)
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
		if ((GetAsyncKeyState('P') < 0) && (fTimeAfterRestart > 1.0f))
		{
			CreatePointEmitterPressed (NULL);
			fTimeAfterRestart = 0.0f;
			//Sleep (150);
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

		if (GetAsyncKeyState('I') < 0)
		{
			PausePressed(NULL);
			Sleep (80);
		}

		if (GetAsyncKeyState('O') < 0)
		{
			OpenNewSystemPressed(NULL);
			Sleep (80);
		}

		if (GetAsyncKeyState('P') < 0)
		{
			Sleep (80);
			ApplyChangeSystem ();		
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
	}
}

void _cdecl TMainWindow::DrawViewport (GUIControl* sender)
{
	cliper.Push();
	pEditor->Render()->Clear(0, NULL, CLEAR_STENCIL | CLEAR_TARGET | CLEAR_ZBUFFER, pEditor->GetBackgroundColor(), 1.0f, 0);

	RENDERVIEWPORT vp = pEditor->Render()->GetViewport();
	Matrix matOldPrj = pEditor->Render()->GetProjection();
	
	Matrix matView;
	pEditor->ArcBall()->BuildViewMatrix(matView);
	pEditor->Render()->SetView(matView);

	pEditor->Render()->SetPerspective(1.0f, (float)vp.Width, (float)vp.Height);

	if (pBackgroundTexture && !pBackgroundTexture->IsError())
	{
		RS_SPRITE spr[4];
		float x1 = -1.0f; 	float x2 = 1.0f;
		float y1 = 1.0f;	 float y2 = -1.0f;

		spr[0].vPos = Vector(x1, y1, 1.0f);
		spr[1].vPos = Vector(x2, y1, 1.0f);
		spr[2].vPos = Vector(x2, y2, 1.0f);
		spr[3].vPos = Vector(x1, y2, 1.0f);
		spr[0].dwColor = 0xFFFFFFFF;
		spr[1].dwColor = 0xFFFFFFFF;
		spr[2].dwColor = 0xFFFFFFFF;
		spr[3].dwColor = 0xFFFFFFFF;
		spr[0].tu = 0.0f; spr[0].tv = 0.0f;
		spr[1].tu = 1.0f; spr[1].tv = 0.0f;
		spr[2].tu = 1.0f; spr[2].tv = 1.0f;
		spr[3].tu = 0.0f; spr[3].tv = 1.0f;		
		pEditor->Render()->DrawSprites(pBackgroundTexture,spr, 1);
	}


	DrawGrid ();
	//DrawAxisNavigator ();

	
	pEditor->GetGizmo()->SetView(pEditor->Render()->GetView());
	pEditor->GetGizmo()->SetProjection(pEditor->Render()->GetProjection());
	pEditor->GetGizmo()->SetViewPort(pEditor->Render()->GetViewport());
	
	Matrix mCubeTransform = pEditor->GetGizmo()->GetTransform();
	//pEditor->Render()->DrawSolidBox(Vector(-1.0f), Vector(1.0f), mCubeTransform);

	if (pGeom)
	{
		


		pGeom->SetDynamicLightState(false);
		pGeom->SetTransform(mCubeTransform);
		pGeom->Draw();
	}

	pEditor->GetGizmo()->Draw();


	float DeltaTime = 0.0f;
	if (!SystemIsPaused) DeltaTime = api->GetDeltaTime();
	if (SystemIsSlowMode) DeltaTime *= 0.1f;
	
	pEditor->Manager()->Execute(DeltaTime);
	pEditor->Manager()->DrawAllParticles();


	if (bShowAxis)
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

		//pEditor->Render()->SetTexture(0, NULL);
		//pEditor->Render()->SetTexture(1, NULL);


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


	//


	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	DWORD SystemCount = pManager->GetCreatedSystemCount();
	if (SystemCount > 0)
	{
		ParticleSystemProxy* pSys = pManager->GetCreatedSystemByIndex(0);

		ParticleSystem* pRealSystem = pSys->Lock();
		pEditor->Render()->Print(0, 0, 0xFF000000, "%3.2f sec", pRealSystem->GetSystemTime());
		pSys->Unlock();
	}



}

void TMainWindow::DrawGrid ()
{

	if (!bDrawGrid) return;
	float StepSize = 1.0f;
	int GridSize = 12;
	float Edge = StepSize*GridSize;

	for (int x = 0; x <= GridSize; x++)
	{
		float x1 = x * StepSize;
		float x2 = -x1;

		DWORD clr = pEditor->GetGridColor();
		if (x == 0) clr = pEditor->GetGridZeroColor();
		pEditor->Render()->DrawLine(Vector (x1, 0, Edge), clr, Vector (x1, 0, -Edge), clr);
		pEditor->Render()->DrawLine(Vector (x2, 0, Edge), clr, Vector (x2, 0, -Edge), clr);
	}

	for (int z = 0; z <= GridSize; z++)
	{
		float z1 = z * StepSize;
		float z2 = -z1;

		DWORD clr = pEditor->GetGridColor();
		if (z == 0) clr = pEditor->GetGridZeroColor();
		pEditor->Render()->DrawLine(Vector (Edge, 0, z1), clr, Vector (-Edge, 0, z1), clr);
		pEditor->Render()->DrawLine(Vector (Edge, 0, z2), clr, Vector (-Edge, 0, z2), clr);
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
	return GUIWindow::ProcessMessages(message, lparam, hparam);
}


void TMainWindow::BuildBrowserTree ()
{
	TreeView1->Items->Clear();

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();

	DWORD SystemCount = pManager->GetCreatedSystemCount();
	if (SystemCount == 0)
	{
		::Application->MessageBox("No particle system available !!!", "Error", GUIMB_OK, true);
		return;
	}

	if (SystemCount > 1)
	{
		::Application->MessageBox("To many systems detected !!!", "Developer message", GUIMB_OK, true);
		return;
	}

	ParticleSystemProxy* pS = pManager->GetCreatedSystemByIndex(0);
	Assert (pS);

	ParticleSystem* pSystem = pS->Lock();

	



	DWORD EmittersCount = pSystem->GetEmittersCount();
	for (DWORD n = 0; n < EmittersCount; n++)
	{
		IEmitter* pEmitter = pSystem->GetEmitterByIndex(n);

		GUITreeNode* pTreeNode = NEW GUITreeNode;
		pTreeNode->Data = pEmitter;
		pTreeNode->Tag = ET_UNKNOWN;
		
		//Назначем уникальный индентификатор
		if (pSystem->GetEmitterTypeByIndex(n) == POINT_EMITTER) pTreeNode->Tag = ET_POINT_EMITTER;

		__tmp = "#b";
		__tmp += pEmitter->GetName ();

		pTreeNode->SetText(__tmp.c_str());
		pTreeNode->Image->Load("peditor\\emitter");
		TreeView1->Items->Add(pTreeNode);

		AddFieldListToTree (&pTreeNode->Childs, pEmitter->GetData());

		pTreeNode->Childs.Sort (FuncCompare);

		

		DWORD ParticleTypesCount = pEmitter->GetParticleTypesCount();
		for (DWORD i = 0; i < ParticleTypesCount; i++)
		{
			FieldList* pParticleInfo = pEmitter->GetParticleTypeDataByIndex(i);
			ParticleType Type = pEmitter->GetParticleTypeByIndex(i);

			GUITreeNode* pSubeTreeNode = NEW GUITreeNode;
			pSubeTreeNode->Data = pParticleInfo;
			
			//Присваиваем уникальный тип для индентификации
			pSubeTreeNode->Tag = ET_UNKNOWN;
			if (Type == BILLBOARD_PARTICLE )
			{
				pSubeTreeNode->Tag = ET_BILLBOARD_PARTICLE;
				pSubeTreeNode->Image->Load("peditor\\particle");
			}

			if (Type == MODEL_PARTICLE )
			{
				pSubeTreeNode->Tag = ET_MODEL_PARTICLE;
				pSubeTreeNode->Image->Load("peditor\\particlem");
			}
				

			__tmp = "#b";
			__tmp += pParticleInfo->GetString(GUID_PARTICLE_NAME, "Not found");;

			pSubeTreeNode->SetText(__tmp);
			
			pTreeNode->Childs.Add(pSubeTreeNode);

			AddFieldListToTree (&pSubeTreeNode->Childs, pParticleInfo);

			pSubeTreeNode->Childs.Sort (FuncCompare);
		}
	}

	pS->Unlock();
}

void TMainWindow::AddFieldListToTree (GUITreeNodes* Parent, FieldList* pParticleInfo)
{
	//api->Trace("--------------------------------------------------------------\n");

	DWORD FieldCount = pParticleInfo->_GetFieldCount();

	for (DWORD n = 0; n < FieldCount; n++)
	{
		const FieldList::FieldDesc &Field = pParticleInfo->_GetFieldByIndex(n);

		if (Field.Type == FIELD_UNKNOWN) continue;

		GUITreeNode* pTreeNode = NEW GUITreeNode;
		Parent->Add(pTreeNode);

		//api->Trace("param, type = %d, 0x%p", Field.Type, Field.pPointer);

		switch (Field.Type)
		{
			case FIELD_BOOL:
				{
					DataBool* pBoolField = (DataBool*)Field.pPointer;
					//api->Trace("bool, name = '%s'", pBoolField->GetEditorName());
					pTreeNode->SetText(pBoolField->GetEditorName());
					pTreeNode->Image->Load("peditor\\bool");
					pTreeNode->Data = pBoolField;
					pTreeNode->Tag = ET_FIELD_BOOL;
					break;
				}
			case FIELD_FLOAT:
				{
					DataFloat* pFloatField = (DataFloat*)Field.pPointer;
					//api->Trace("float, name = '%s'", pFloatField->GetEditorName());
					pTreeNode->SetText(pFloatField->GetEditorName());
					pTreeNode->Image->Load("peditor\\float");
					pTreeNode->Data = pFloatField;
					pTreeNode->Tag = ET_FIELD_FLOAT;
					break;
				}
			case FIELD_GRAPH:
				{
					DataGraph* pGraphField = (DataGraph*)Field.pPointer;
					//api->Trace("graph, name = '%s'", pGraphField->GetEditorName());
					pTreeNode->SetText(pGraphField->GetEditorName());
					pTreeNode->Image->Load("peditor\\graph");
					pTreeNode->Data = pGraphField;
					pTreeNode->Tag = ET_FIELD_GRAPH;
					break;
				}
			case FIELD_POSITION:
				{
					DataPosition* pPositionField = (DataPosition*)Field.pPointer;
					//api->Trace("position, name = '%s'", pPositionField->GetEditorName());
					pTreeNode->SetText(pPositionField->GetEditorName());
					pTreeNode->Image->Load("peditor\\pos");
					pTreeNode->Data = pPositionField;
					pTreeNode->Tag = ET_FIELD_POSITION;
					break;
				}
			case FIELD_STRING:
				{

					DataString* pStringField = (DataString*)Field.pPointer;
					//api->Trace("string, name = '%s'", pStringField->GetEditorName());
					pTreeNode->SetText(pStringField->GetEditorName());
					if (crt_stricmp (pTreeNode->GetText(), "Name") == 0)
					{
						pTreeNode->Release();
						continue;
					}
					pTreeNode->Image->Load("peditor\\string");
					pTreeNode->Data = pStringField;
					pTreeNode->Tag = ET_FIELD_STRING;

					break;
				}
			case FIELD_UV:
				{
					DataUV* pUVField = (DataUV*)Field.pPointer;
					//api->Trace("uv, name = '%s'", pUVField->GetEditorName());
					pTreeNode->SetText(pUVField->GetEditorName());
					pTreeNode->Image->Load("peditor\\texture");
					pTreeNode->Data = pUVField;
					pTreeNode->Tag = ET_FIELD_UV;
					break;
				}
			case FIELD_COLOR:
				{

					DataColor* pColorField = (DataColor*)Field.pPointer;
					//api->Trace("color, name = '%s'", pColorField->GetEditorName());
					pTreeNode->SetText(pColorField->GetEditorName());
					pTreeNode->Image->Load("peditor\\color");
					pTreeNode->Data = pColorField;
					pTreeNode->Tag = ET_FIELD_COLOR;
					break;
				}
			default:
				throw ("Unknown Attribute type");
		}	 //switch

//		api->Trace("Node = %s", pTreeNode->Text);
	}
}


void _cdecl TMainWindow::ShowHideParticle (GUIControl* sender)
{
	GUITreeNode* TNode = TreeView1->GetSelectedNode();
	if (!TNode) return;

	if (TNode->Tag == ET_POINT_EMITTER)
	{
		IEmitter* pEmiter = (IEmitter*)TNode->Data;
		bool EmitterEnabled = pEmiter->SetEnable(!pEmiter->GetEnable());

		if (EmitterEnabled)
			BoldBranch (TNode);
		else
			UnBoldBranch (TNode);
	}


	if (TNode->Tag == ET_BILLBOARD_PARTICLE || TNode->Tag == ET_MODEL_PARTICLE)
	{
		IEmitter* pEmiter = (IEmitter*)TNode->Parent->Data;
		int Index = pEmiter->GetParticleTypeIndex((FieldList*)TNode->Data);
		Assert (Index >= 0);

		bool ParticleTypeEnabled = pEmiter->SetParticleTypeEnable(!pEmiter->GetParticleTypeEnable(Index), Index);

		if (ParticleTypeEnabled)
			BoldBranch (TNode);
		else
			UnBoldBranch (TNode);

	}


}

void TMainWindow::BoldBranch (GUITreeNode* pNode)
{
	if ( pNode->Tag == ET_POINT_EMITTER || 
		   pNode->Tag == ET_BILLBOARD_PARTICLE ||
			 pNode->Tag == ET_MODEL_PARTICLE )
	{
		if (pNode->GetText()[DWORD(0)] != '#' && pNode->GetText()[DWORD(1)] != 'b')
		{
			__tmp = string("#b") + pNode->GetText(); 
			pNode->SetText(__tmp);
		}

		for (int n = 0; n < pNode->Childs.GetCount(); n++)
		{
			BoldBranch (pNode->Childs[n]);
		}
	}

}

void TMainWindow::UnBoldBranch (GUITreeNode* pNode)
{
	if ( pNode->Tag == ET_POINT_EMITTER || 
		pNode->Tag == ET_BILLBOARD_PARTICLE ||
		pNode->Tag == ET_MODEL_PARTICLE )
	{
		if (pNode->GetText()[DWORD(0)] == '#' && pNode->GetText()[DWORD(1)] == 'b')
		{
			__tmp = pNode->GetText();
			__tmp.Delete(0, 2) ;
			pNode->SetText(__tmp);
		}

		for (int n = 0; n < pNode->Childs.GetCount(); n++)
		{
			UnBoldBranch (pNode->Childs[n]);
		}
	}
}

void _cdecl TMainWindow::TreeViewChangeNode (GUIControl* sender)
{
	GUITreeNode* SelNode = TreeView1->GetSelectedNode();
	if (!SelNode) return;

	if (SelNode->bReadOnly)
	{
		return;
	}




	if (NowEdited)
	{
		ApplyChangeSystem ();
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	} 
		

	if (SelNode->Tag == ET_FIELD_GRAPH)
	{
		GraphEditor* pNewGraphEdit = NEW GraphEditor ();
		pNewGraphEdit->BeginEdit(Panel1, (DataGraph*)SelNode->Data);

		NowEdited = pNewGraphEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}

	if (SelNode->Tag == ET_FIELD_UV)
	{
		UVEditor* pNewUVEdit = NEW UVEditor ();
		pNewUVEdit->BeginEdit(Panel1, (DataUV*)SelNode->Data);

		NowEdited = pNewUVEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}

	if (SelNode->Tag == ET_FIELD_COLOR)
	{
		ColorEditor* pNewColorEdit = NEW ColorEditor ();
		pNewColorEdit->BeginEdit(Panel1, (DataColor*)SelNode->Data);

		NowEdited = pNewColorEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}

	if (SelNode->Tag == ET_FIELD_BOOL)
	{
		BoolEditor* pNewBoolEdit = NEW BoolEditor ();
		pNewBoolEdit->BeginEdit(Panel1, (DataBool*)SelNode->Data);

		NowEdited = pNewBoolEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}

	if (SelNode->Tag == ET_FIELD_FLOAT)
	{
		FloatEditor* pNewFloatEdit = NEW FloatEditor ();
		pNewFloatEdit->BeginEdit(Panel1, (DataFloat*)SelNode->Data);

		NowEdited = pNewFloatEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}
	
	if (SelNode->Tag == ET_FIELD_POSITION)
	{
		PositionEditor* pNewPositionEdit = NEW PositionEditor ();
		pNewPositionEdit->BeginEdit(Panel1, (DataPosition*)SelNode->Data);

		NowEdited = pNewPositionEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}

	if (SelNode->Tag == ET_FIELD_STRING)
	{
		StringEditor* pNewStringEdit = NEW StringEditor ();
		pNewStringEdit->BeginEdit(Panel1, (DataString*)SelNode->Data);

		NowEdited = pNewStringEdit;

		if (!UndoSelectNode) CreateUndo (false);
	}


}

void _cdecl TMainWindow::RenameObjectPressed (GUIControl* sender)
{
	//api->Trace("Rename pressed\n");
	GUITreeNode* SelNode = TreeView1->GetSelectedNode();
	if (!SelNode)
	{
		//api->Trace("Node not selected\n");
		return;
	}

	if ( SelNode->Tag == ET_POINT_EMITTER)
	{
		//api->Trace("Point emitter\n");
		IEmitter* pEmitter = (IEmitter*)SelNode->Data;
		TRenameWindow* pRenamer = NEW TRenameWindow(pEmitter->GetName());
		Application->ShowModal(pRenamer);
		pRenamer->OnClose = (CONTROL_EVENT)&TMainWindow::EmitterRename;
	}

	if ( SelNode->Tag == ET_BILLBOARD_PARTICLE || SelNode->Tag == ET_MODEL_PARTICLE )
	{
		//api->Trace("Particle\n");
		FieldList* pParticle = (FieldList*)SelNode->Data;
		TRenameWindow* pRenamer = NEW TRenameWindow(pParticle->GetString(GUID_PARTICLE_NAME, "Not found"));
		Application->ShowModal(pRenamer);
		pRenamer->OnClose = (CONTROL_EVENT)&TMainWindow::ParticleRename;
	}


}

void _cdecl TMainWindow::EmitterRename (GUIControl* sender)
{
	TRenameWindow* pRenamer = (TRenameWindow*)sender;
	if (!pRenamer->ExitByOK) return;

	GUITreeNode* SelNode = TreeView1->GetSelectedNode();
	if (!SelNode) return;

	if ( SelNode->Tag != ET_POINT_EMITTER) return;

	IEmitter* pEmitter = (IEmitter*)SelNode->Data;

	pEmitter->SetName(pRenamer->pText->Text.GetBuffer());

	SelNode->SetText(pRenamer->pText->Text);
	if (pEmitter->GetEnable())	BoldBranch (SelNode);

	CreateUndo ();
}

void _cdecl TMainWindow::ParticleRename (GUIControl* sender)
{
	TRenameWindow* pRenamer = (TRenameWindow*)sender;
	if (!pRenamer->ExitByOK) return;

	GUITreeNode* SelNode = TreeView1->GetSelectedNode();
	if (!SelNode) return;


	if ( SelNode->Tag != ET_BILLBOARD_PARTICLE && SelNode->Tag != ET_MODEL_PARTICLE) return;

	FieldList* pParticle = (FieldList*)SelNode->Data;

	DataString* pName = pParticle->FindStringByGUID(GUID_PARTICLE_NAME);
	pName->SetValue(pRenamer->pText->Text.GetBuffer());

	SelNode->SetText(pRenamer->pText->Text);

	IEmitter* pEmiter = (IEmitter*)SelNode->Parent->Data;
	int Index = pEmiter->GetParticleTypeIndex(pParticle);
	Assert (Index >= 0);
	if (pEmiter->GetParticleTypeEnable(Index))
	{
		BoldBranch (SelNode);
	}


	CreateUndo ();
}



void _cdecl TMainWindow::SavePressed (GUIControl* sender)
{
//SaveAsPressed
	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	DWORD SystemCount = pManager->GetCreatedSystemCount();


	bool bNeedSaveAS = false;
	for (DWORD n = 0; n < SystemCount; n++)
	{
		ParticleSystemProxy* pSys = pManager->GetCreatedSystemByIndex(n);
		string pSysName = pSys->GetName();
		if (pSysName == "_empty.xps" || pSysName == "_empty.xml")
		{
			bNeedSaveAS = true;
		}
	}

	if (bNeedSaveAS)
	{
		SaveAsPressed(sender);
	}



	for (DWORD n = 0; n < SystemCount; n++)
	{
		ParticleSystemProxy* pSys = pManager->GetCreatedSystemByIndex(n);
		const char* pSysName = pSys->GetName();
		pManager->WriteSystemCache(pSysName);
	}

	ClearUndoSystem ();

	SaveOKTime = SAVEDOK_SHOWTIME;
}

void _cdecl TMainWindow::UndoPressed (GUIControl* sender)
{
	bLowFPS = !bLowFPS;
/*
	if (NowEdited)
	{
		ApplyChangeSystem (false);
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	}
	Undo ();
*/
}

void _cdecl TMainWindow::RedoPressed (GUIControl* sender)
{
	if (NowEdited)
	{
		ApplyChangeSystem (false);
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	}
	Redo ();
}

//============================ undo =====================================
void TMainWindow::InitUndoSystem(DWORD UndoSteps)
{
	ShutdownUndoSystem ();
	PositionInUndoBuffer = -1;
	MaxUndoPos = UndoSteps;
}

void TMainWindow::Undo ()
{
	return;
	if (PositionInUndoBuffer <= 0) return;

	IParticleSystem* pSystem = pEditor->GetSystem();
	if (!pSystem) return;
	const char* SystemName = pSystem->GetName();

	if (NowEdited)
	{
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	}



	PositionInUndoBuffer--;
	SetUndoButtonsState ();

	MemFile* pUndoMem = UndoEntry[PositionInUndoBuffer];
	pUndoMem->Seek(0, VFSEEK_SET);
	((ParticleManager*)pEditor->Manager())->LoadSystemCache(SystemName, pUndoMem);
	//pEditor->Manager()->Editor_UpdateCachedData();

	BuildBrowserTree ();
	RestoreTreeExpansion(pUndoMem);
}

void TMainWindow::SetUndoButtonsState ()
{
/*
	if (PositionInUndoBuffer <= 0)
		btnUndoAction->bEnabled = false;
	else
		btnUndoAction->bEnabled = true;
*/
/*
	if (PositionInUndoBuffer >= (int)(UndoEntry.Size()-1))
		btnRedoAction->bEnabled = false;
	else
		btnRedoAction->bEnabled = true;
*/
}

void TMainWindow::Redo ()
{
	return;

	if (PositionInUndoBuffer >= (int)(UndoEntry.Size()-1)) return;
	
	IParticleSystem* pSystem = pEditor->GetSystem();;
	if (!pSystem) return;
	const char* SystemName = pSystem->GetName();

	if (NowEdited)
	{
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	}


	PositionInUndoBuffer++;
	SetUndoButtonsState ();

	MemFile* pUndoMem = UndoEntry[PositionInUndoBuffer];
	pUndoMem->Seek(0, VFSEEK_SET);
	((ParticleManager*)pEditor->Manager())->LoadSystemCache(SystemName, pUndoMem);
	
	//pEditor->Manager()->Editor_UpdateCachedData();

	BuildBrowserTree ();
	RestoreTreeExpansion(pUndoMem);
}

void TMainWindow::CreateUndo (bool KillSameUndos)
{
	//bug HERE !!!
	return;

	//api->Trace("============ undo ==================");
	IParticleSystem* pSystem = pEditor->GetSystem();;
	if (!pSystem) return;
	const char* SystemName = pSystem->GetName();

	//Redo больше невозможно становиться...

	if (PositionInUndoBuffer < (int)(UndoEntry.Size()-1))
	{
		for (DWORD i = PositionInUndoBuffer+1; i < UndoEntry.Size(); i++)
		{
			delete UndoEntry[i];
		}

		UndoEntry.DelRange(PositionInUndoBuffer+1, UndoEntry.Size()-1);

	}

	SetUndoButtonsState ();
	//Создаем UNDO
	MemFile* pUndoMem = NEW MemFile;
	pUndoMem->OpenWrite(300000);
	DWORD Index1 = UndoEntry.Add(pUndoMem);
	((ParticleManager*)pEditor->Manager())->WriteSystemCache(SystemName, pUndoMem);
	SaveTreeExpansion(pUndoMem);
	PositionInUndoBuffer++;

	DWORD Index2 = Index1-1;

	
	if (Index1 > 0 && KillSameUndos)
	{
		//Если ничего не поменялось...
		if (UndoEntry[Index1]->Compare(UndoEntry[Index2]) == true)
		{
			//api->Trace("============ cancel create undo operation ==================");
			delete UndoEntry[Index1];
			UndoEntry.Extract(Index1);
			PositionInUndoBuffer--;
		}
	}


	//api->Trace("undo count %d", UndoEntry.Size());
	SetUndoButtonsState ();


	//Если UNDO слишком большой подрезаем его...
	if (UndoEntry.Size() > MaxUndoPos)
	{
		delete UndoEntry[0];
		UndoEntry.Extract(0);
		PositionInUndoBuffer--;
	}

	//if (PositionInUndoBuffer > 0)	btnUndoAction->bEnabled = true;
}

void TMainWindow::ShutdownUndoSystem ()
{
	for (DWORD n = 0; n < UndoEntry.Size(); n++)
	{
		delete UndoEntry[n];
	}

	UndoEntry.DelAll();
}

void TMainWindow::ClearUndoSystem ()
{
	ShutdownUndoSystem ();
	InitUndoSystem (MaxUndoPos);
}

//============================ undo =====================================

void TMainWindow::ApplyChangeSystem (bool NeedCreateUndo)
{
	if (NowEdited)
	{
		NowEdited->Apply();
		pEditor->Manager()->Editor_UpdateCachedData();

		if (NeedCreateUndo) CreateUndo ();
	}
}


void TMainWindow::SaveTreeExpansion (MemFile* pFile)
{
	GUITreeNode* pCurSelNode = TreeView1->GetSelectedNode();
	DWORD RootConut = TreeView1->Items->GetCount();
	pFile->WriteType(RootConut);
	for (DWORD n =0; n < RootConut; n++)
	{
		GUITreeNode* pNode = TreeView1->Items->Get(n);
		BYTE ExpValue = pNode->Expanded; 
		pFile->WriteType(ExpValue);

		DWORD ChildsCount = pNode->Childs.GetCount();
		pFile->WriteType(ChildsCount);
		for (DWORD i = 0; i < ChildsCount; i++)
		{
			GUITreeNode* pSubNode = pNode->Childs.Get(i);
			BYTE ExpSubValue = pSubNode->Expanded; 
			pFile->WriteType(ExpSubValue);

			DWORD AttrCount = pSubNode->Childs.GetCount();
			pFile->WriteType(AttrCount);
			for (DWORD j = 0; j < AttrCount; j++)
			{
				GUITreeNode* pAttrNode = pSubNode->Childs.Get(j);
				BYTE ExpAttrSel = pAttrNode->Selected; 
				pFile->WriteType(ExpAttrSel);
			}
		}
	}
}

void TMainWindow::RestoreTreeExpansion (MemFile* pFile)
{
	GUITreeNode* pSelectedNode = NULL;
	DWORD RootConut;
	pFile->ReadType(RootConut);

	for (DWORD n =0; n < RootConut; n++)
	{
		BYTE ExpValue; 
		pFile->ReadType(ExpValue);

		GUITreeNode* pNode = TreeView1->Items->Get(n);
		pNode->Expanded = (ExpValue != 0);

		DWORD ChildsCount;
		pFile->ReadType(ChildsCount);

		for (DWORD i = 0; i < ChildsCount; i++)
		{
			BYTE ExpSubValue; 
			pFile->ReadType(ExpSubValue);
			GUITreeNode* pSubNode = pNode->Childs.Get(i);
			pSubNode->Expanded = (ExpSubValue != 0);

			DWORD AttrCount;
			pFile->ReadType(AttrCount);

			for (DWORD j = 0; j < AttrCount; j++)
			{
				BYTE ExpAttrSel; 
				pFile->ReadType(ExpAttrSel);
				GUITreeNode* pAttrNode = pSubNode->Childs.Get(j);

				if (ExpAttrSel != 0)
				{
					pSelectedNode = pAttrNode;
				}
			}
		}
	}

	if (pSelectedNode)
	{
		UndoSelectNode = true;
		TreeView1->SetSelectedNode(pSelectedNode, false);
		UndoSelectNode = false;
	}
}


void _cdecl TMainWindow::CreatePointEmitterPressed (GUIControl* sender)
{
	IParticleSystem* pSys = pEditor->GetSystem();
	if (!pSys) return;
	const char* pSystemName = pSys->GetName();
	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	if (!pManager) return;

	TempSystemName = pSystemName;

/*
	pManager->Editor_CreatePointEmitter (pSystemName);
	CreateUndo ();
	BuildBrowserTree ();
*/
	TChooseWindow* pNewChooseName = NEW TChooseWindow("PointEmitter");
	pNewChooseName->pDesc->Caption = "Type new emitter name";
	pNewChooseName->OnClose = (CONTROL_EVENT)&TMainWindow::RealCreatePointEmitter;
	Application->ShowModal(pNewChooseName);
}

void _cdecl TMainWindow::CreateBillboardPressed (GUIControl* sender)
{
	IParticleSystem* pSys = pEditor->GetSystem();
	if (!pSys) return;
	const char* pSystemName = pSys->GetName();
	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	if (!pManager) return;

	GUITreeNode* pTreeNode = TreeView1->GetSelectedNode();
	if (!pTreeNode)
	{
		Application->MessageBox("Select emitter first !", "Error", GUIMB_OK);
		return;
	}

	if (pTreeNode->Tag != ET_POINT_EMITTER)
	{
		Application->MessageBox("Select emitter first !", "Error", GUIMB_OK);
		return;
	}

	const char* EmitterName = NULL;

	if (pTreeNode->GetText()[0] == '#')
		EmitterName = pTreeNode->GetText() + 2;
	else
		EmitterName = pTreeNode->GetText();


	TempSystemName = pSystemName;
	TempEmitterName = EmitterName;
	TempNodeToAdd = pTreeNode;


/*
	pManager->Editor_CreateBillBoardParticle (pSystemName, EmitterName);
	CreateUndo ();
	BuildBrowserTree ();
*/

	TChooseWindow* pNewChooseName = NEW TChooseWindow("Billboard");
	pNewChooseName->pDesc->Caption = "Type new particle name";
	pNewChooseName->OnClose = (CONTROL_EVENT)&TMainWindow::RealCreateBillboard;
	Application->ShowModal(pNewChooseName);

}

void _cdecl TMainWindow::CreateModelPressed (GUIControl* sender)
{
	IParticleSystem* pSys = pEditor->GetSystem();
	if (!pSys) return;
	const char* pSystemName = pSys->GetName();
	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	if (!pManager) return;

	GUITreeNode* pTreeNode = TreeView1->GetSelectedNode();
	if (!pTreeNode)
	{
		Application->MessageBox("Select emitter first !", "Error", GUIMB_OK);
		return;
	}

	if (pTreeNode->Tag != ET_POINT_EMITTER)
	{
		Application->MessageBox("Select emitter first !", "Error", GUIMB_OK);
		return;
	}
	const char* EmitterName = NULL;

	if (pTreeNode->GetText()[0] == '#')
		EmitterName = pTreeNode->GetText() + 2;
	else
		EmitterName = pTreeNode->GetText();


	TempSystemName = pSystemName;
	TempEmitterName = EmitterName;
	TempNodeToAdd = pTreeNode;

/*
	pManager->Editor_CreateModelParticle (pSystemName, EmitterName);
	CreateUndo ();
	BuildBrowserTree ();
*/
	TChooseWindow* pNewChooseName = NEW TChooseWindow("Model");
	pNewChooseName->pDesc->Caption = "Type new particle name";
	pNewChooseName->OnClose = (CONTROL_EVENT)&TMainWindow::RealCreateModel;
	Application->ShowModal(pNewChooseName);

}

void _cdecl TMainWindow::RealCreatePointEmitter (GUIControl* sender)
{
	TChooseWindow* pNewChooseName =	(TChooseWindow*)sender;
	if (!pNewChooseName->ExitByOK) return;

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	FieldList* pData = pManager->Editor_CreatePointEmitter (TempSystemName.GetBuffer(), pNewChooseName->pText->Text.GetBuffer());


	ParticleSystemProxy* pSys = (ParticleSystemProxy*)pEditor->GetSystem();
	ParticleSystem* realPS = pSys->Lock();
	IEmitter* pEmitter = realPS->FindEmitterByData(pData);
	pSys->Unlock();

	Assert (pEmitter);

/*
	GUITreeNode* pTreeNode = NEW GUITreeNode;
	pTreeNode->Data = pEmitter;
	pTreeNode->Tag = ET_POINT_EMITTER;
	pTreeNode->Text = "#b";
	pTreeNode->Text += pEmitter->GetName ();
	pTreeNode->Image->Load("peditor\\emitter");
	TreeView1->Items->Add(pTreeNode);

	AddFieldListToTree (&pTreeNode->Childs, pEmitter->GetData());

	pTreeNode->Childs.Sort (FuncCompare);
*/
	BuildBrowserTree ();

	CreateUndo ();

}

void _cdecl TMainWindow::RealCreateBillboard (GUIControl* sender)
{
	TChooseWindow* pNewChooseName =	(TChooseWindow*)sender;
	if (!pNewChooseName->ExitByOK) return;

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	FieldList* pParticleInfo = pManager->Editor_CreateBillBoardParticle (TempSystemName.GetBuffer(), TempEmitterName.GetBuffer(), pNewChooseName->pText->Text.GetBuffer());


/*
	GUITreeNode* pSubeTreeNode = NEW GUITreeNode;
	pSubeTreeNode->Data = pParticleInfo;
	pSubeTreeNode->Tag = ET_BILLBOARD_PARTICLE;
	pSubeTreeNode->Image->Load("peditor\\particle");
	pSubeTreeNode->Text = "#b";
	pSubeTreeNode->Text += pParticleInfo->GetString(PARTICLE_NAME, "Not found");
	TempNodeToAdd->Childs.Add(pSubeTreeNode);
	AddFieldListToTree (&pSubeTreeNode->Childs, pParticleInfo);
	pSubeTreeNode->Childs.Sort (FuncCompare);
*/

	BuildBrowserTree ();

	CreateUndo ();


}

void _cdecl TMainWindow::RealCreateModel (GUIControl* sender)
{
	TChooseWindow* pNewChooseName =	(TChooseWindow*)sender;
	if (!pNewChooseName->ExitByOK) return;

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	FieldList* pParticleInfo = pManager->Editor_CreateModelParticle (TempSystemName.GetBuffer(), TempEmitterName.GetBuffer(), pNewChooseName->pText->Text.GetBuffer());

/*
	GUITreeNode* pSubeTreeNode = NEW GUITreeNode;
	pSubeTreeNode->Data = pParticleInfo;
	pSubeTreeNode->Tag = ET_MODEL_PARTICLE;
	pSubeTreeNode->Image->Load("peditor\\particlem");
	pSubeTreeNode->Text = "#b";
	pSubeTreeNode->Text += pParticleInfo->GetString(PARTICLE_NAME, "Not found");
	TempNodeToAdd->Childs.Add(pSubeTreeNode);
	AddFieldListToTree (&pSubeTreeNode->Childs, pParticleInfo);
	pSubeTreeNode->Childs.Sort (FuncCompare);
*/
	BuildBrowserTree ();

	CreateUndo ();

}

void _cdecl TMainWindow::RestartSystem (GUIControl* sender)
{
	ApplyChangeSystem ();		
	IParticleSystem* pSys = pEditor->GetSystem ();
	if (pSys)	pSys->Restart(rand());
}

void _cdecl TMainWindow::DeleteObjectPressed (GUIControl* sender)
{
	GUITreeNode* pTreeNode = TreeView1->GetSelectedNode();
	if (!pTreeNode) return;

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	IParticleSystem* pSys = pEditor->GetSystem();
	if (!pSys) return;
	const char* pSystemName = pSys->GetName();


	if (pTreeNode->Tag == ET_POINT_EMITTER)
	{
		// EmitterName
		pManager->DeletePointEmitter (pSystemName, (IEmitter*)pTreeNode->Data);
		BuildBrowserTree ();
		//pTreeNode->Release();
		CreateUndo ();
	}

	if (pTreeNode->Tag == ET_BILLBOARD_PARTICLE)
	{
		pManager->DeleteBillboard (pSystemName, (IEmitter*)pTreeNode->Parent->Data, (FieldList*)pTreeNode->Data);
		BuildBrowserTree ();
		//pTreeNode->Release();
		CreateUndo ();
	}

	if (pTreeNode->Tag == ET_MODEL_PARTICLE)
	{
		pManager->DeleteModel (pSystemName, (IEmitter*)pTreeNode->Parent->Data, (FieldList*)pTreeNode->Data);
		BuildBrowserTree ();
		//pTreeNode->Release();
		CreateUndo ();
	}


}

void _cdecl TMainWindow::OpenNewSystemPressed (GUIControl* sender)
{
	GUIFileOpen* open_dialog = NEW GUIFileOpen ();
	
#ifdef USE_XML_FILE_FORMAT
	open_dialog->Filters.Add(".xml");
	open_dialog->FiltersDesc.Add("eXtended Particle System (XML)");
#else
	open_dialog->Filters.Add(".xps");
	open_dialog->FiltersDesc.Add("eXtended Particle System");
#endif



	open_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::RealLoadSystem;
	open_dialog->StartDirectory = string(string (pEditor->GetStartDir()) + "\\resource\\particles\\").GetBuffer();
	Application->ShowModal(open_dialog);
	open_dialog->Filter->SelectItem (1);

/*
	if (NowEdited)
	{
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	} 

	TProjectBrowser* FirstSelector = NEW TProjectBrowser;
	Application->ShowModal(FirstSelector);
	FirstSelector->OnClose = (CONTROL_EVENT)OnCloseManager;
*/
}


void _cdecl TMainWindow::RealLoadSystem (GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;
	string file;
	file.GetFileTitle(dialog->FileName);

	file.AddExtention(".xps");

	LoadToEditor(file);

}

void _cdecl TMainWindow::OnCloseManager (GUIControl* sender)
{

	Assert(false);
/*
	TProjectBrowser* FirstSelector = (TProjectBrowser*)sender;
	if (!FirstSelector->UserSelectSystemToOpen) return;

	pEditor->Manager()->OpenProject(FirstSelector->CurrentProjectNameShort.GetBuffer());

	IParticleSystem* pSys = (IParticleSystem*)pEditor->GetSystem ();
	if (pSys)
	{
		pSys->Release ();
		pSys = NULL;
	}

	IParticleSystem* pSYS = pEditor->Manager()->CreateParticleSystemEx(FirstSelector->CurrentSystemName.GetBuffer(), _FL_);

	if (!pSYS) throw ("Can't create system !!!!!");

	ClearUndoSystem();
	BuildBrowserTree ();

	CreateUndo ();
*/
}

void _cdecl TMainWindow::OnCreateNewSystem (GUIControl* sender)
{
	char EmptyFile[12];
	memset (EmptyFile, 0, 12);
	crt_strcpy (EmptyFile, 9, "PSYSv3.5");
	pEditor->Files()->SaveData(".\\resource\\particles\\_empty.xps", EmptyFile, 12);

	ParticleManager* pParticleManager = (ParticleManager*)pEditor->Manager();

	LoadToEditor("_empty");
	//IParticleSystem* pLoadedSys = pParticleManager->CreateParticleSystemEx("_empty.xps", _FL_);
/*
	TChooseWindow* pNewChooseName = NEW TChooseWindow("New Particle System");
	pNewChooseName->pDesc->Caption = "Type system name";
	pNewChooseName->OnClose = (CONTROL_EVENT)OnCreateNewSystemReal;
	Application->ShowModal(pNewChooseName);
*/
}

void _cdecl TMainWindow::OnCreateNewSystemReal (GUIControl* sender)
{
	TChooseWindow* pNewChooseName =	(TChooseWindow*)sender;
	if (!pNewChooseName->ExitByOK) return;

	const char* NewSystemName = pNewChooseName->pText->Text.GetBuffer();

	TempLongFileName = "resource\\particles\\";
	TempLongFileName+=NewSystemName;
	TempLongFileName.AddExtention(".xps");


	if (pEditor->Files()->IsExist(TempLongFileName.GetBuffer()))
	{
		string Mess;
		Mess.Format("File '%s' already exist\nReplace ?", TempLongFileName.GetBuffer());
		GUIMessageBox* pMB = Application->MessageBox(Mess.GetBuffer(), "File exist", GUIMB_OKCANCEL);
		pMB->OnOK = (CONTROL_EVENT)&TMainWindow::RealCreateEmptySystem;
		return;
	}

	RealCreateEmptySystem (NULL);
}

void _cdecl TMainWindow::RealCreateEmptySystem (GUIControl* sender)
{
	/*
	char EmptyFile[12];
	memset (EmptyFile, 0, 12);
	strcpy (EmptyFile, "PSYSv3.5");

	IWrite* pFile = pEditor->Files()->Write(TempLongFileName.GetBuffer(), iw_create_always, _FL_);
	pFile->Write(EmptyFile, 12);
	pFile->Release();

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();

	

	string Mess;
	Mess.Format("Add system '%s'\nto current project '%s' and load it ?", TempLongFileName.GetBuffer(), "FIXME!");
	GUIMessageBox* pMB = Application->MessageBox(Mess.GetBuffer(), "Question", GUIMB_OKCANCEL);
	pMB->OnOK = (CONTROL_EVENT)AddToCurrentProject;
*/
}



void TMainWindow::LoadToEditor (const char* pSysName)
{
	if (NowEdited)
	{
		NowEdited->EndEdit();
		NowEdited->Release ();
		NowEdited = NULL;
	} 


	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();

/*
	string LongProjectFileName = "resource\\particles\\";
	LongProjectFileName += "FIXME!";//pManager->GetProjectFileName();
	LongProjectFileName.AddExtention(".prj");


	IIniFile* IniFile = pEditor->Files()->OpenIniFile(LongProjectFileName.GetBuffer(), _FL_);
	Assert (IniFile);

	bool NeedAddToProject = true;
	//Загружаем данные 
	string Section;
	for (int n = 0; n < 9999; n++)
	{
		Section.Format("System_%04d", n);
		const char* szManager = IniFile->GetString("Manager", Section.GetBuffer(), NULL);
		if (szManager == NULL) break;

		if (crt_stricmp (szManager, pSysName) == 0)
		{
			NeedAddToProject = false;
			break;
		}
	}

	if (NeedAddToProject)
	{
		IniFile->SetString("Manager", (char*)Section.GetBuffer(), (char*)pSysName);
	}

	IniFile->Release();

	//========= load system.....

	Assert(false);
	//pEditor->Manager()->OpenProject(pManager->GetProjectFileName());
*/

	IParticleSystem* pSys = (IParticleSystem*)pEditor->GetSystem ();
	if (pSys)
	{
		pSys->Release ();
		pSys = NULL;
	}

	((ParticleManager*)pEditor->Manager())->LoadAllSystems();

	pEditor->Manager()->CreateParticleSystemEx(pSysName, _FL_);

	ClearUndoSystem();
	BuildBrowserTree ();

	CreateUndo ();
}

void _cdecl TMainWindow::SaveAsPressed (GUIControl* sender)
{
	GUIFileSave* save_dialog = NEW GUIFileSave ();
	save_dialog->Filters.Add(".xps");
	save_dialog->FiltersDesc.Add("eXtended Particle System");
	save_dialog->OnOK = (CONTROL_EVENT)&TMainWindow::RealSaveAs;
	save_dialog->StartDirectory = string(string (pEditor->GetStartDir()) + "\\resource\\particles\\").GetBuffer();
	Application->ShowModal(save_dialog);
	save_dialog->Filter->SelectItem (1);
}

void _cdecl TMainWindow::RealSaveAs (GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;
	string file = dialog->FileName;
	file.AddExtention(".xps");

	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	DWORD SystemCount = pManager->GetCreatedSystemCount();

	if (SystemCount <= 0) return;

	IParticleSystem* pSys = pManager->GetCreatedSystemByIndex(0);
	const char* pSysName = pSys->GetName();
	pManager->WriteSystemCacheAs(pSysName, file.GetBuffer());

	ClearUndoSystem ();

	string SystemName; 
	SystemName.GetFileName(file);
	LoadToEditor (SystemName);

	SaveOKTime = SAVEDOK_SHOWTIME;
}

void _cdecl TMainWindow::PausePressed (GUIControl* sender)
{
	SystemIsPaused = !SystemIsPaused;

	if (!SystemIsPaused)
	{
		btnPauseSystem->Down = false;
	} else
	{
		SystemIsSlowMode = false;
		btnPauseSystem->Down = true;
	}
}

void _cdecl TMainWindow::SlowMotionPressed (GUIControl* sender)
{
	SystemIsSlowMode = !SystemIsSlowMode;

	if (!SystemIsSlowMode)
	{
		btnSlowMotionSystem->Down = false;
	} else
	{
		SystemIsPaused = false;
		btnSlowMotionSystem->Down = true;
	}

}


void _cdecl TMainWindow::SelectBackgroundPressed (GUIControl* sender)
{
	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->Filters.Add (".txx");
	fo->FiltersDesc.Add ("Texture file");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadBackgroundTexture;
	fo->StartDirectory = string(string (pEditor->GetStartDir()) + "\\resource\\textures\\").GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TMainWindow::LoadBackgroundTexture (GUIControl* sender)
{
	GUIFileOpen* fo = (GUIFileOpen*)sender;

	string CropPath = string(pEditor->GetStartDir()) + "\\resource\\textures\\";
	string TextureName = fo->FileName;
	TextureName.GetRelativePath(CropPath);


	if (pBackgroundTexture) pBackgroundTexture->Release ();

	pBackgroundTexture = pEditor->Render()->CreateTexture(_FL_, TextureName.GetBuffer());

	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile* pIni = pFS->OpenEditableIniFile(api->Storage().GetString("system.ini"), file_open_always, _FL_);
	Assert (pIni);
	pIni->SetString("peditor", "backgroundname", (char*)TextureName.GetBuffer());
	pIni->Release();

}


void _cdecl TMainWindow::btnShowDirPressed (GUIControl* sender)
{
	bShowAxis = !bShowAxis;
	if (!bShowAxis) btnShowDirection->Down = false;
}

void _cdecl TMainWindow::btnShowGridPressed (GUIControl* sender)
{
	bDrawGrid = !bDrawGrid;
	if (!bDrawGrid) btnShowGrid->Down = false;
}



void _cdecl TMainWindow::btnSimplePressed (GUIControl* sender)
{
	pEditor->GetGizmo()->Enable(false);
}

void _cdecl TMainWindow::btnMovePressed (GUIControl* sender)
{
	pEditor->GetGizmo()->Enable(true);
	pEditor->GetGizmo()->SetType(TransformGizmo::GT_MOVE);
	bGizmoMode->SelectItem(GZM_move_index);
}

void _cdecl TMainWindow::btnRotatePressed (GUIControl* sender)
{
	pEditor->GetGizmo()->Enable(true);
	pEditor->GetGizmo()->SetType(TransformGizmo::GT_ROTATE);
	bGizmoMode->SelectItem(GZM_rotate_index);
}

void _cdecl TMainWindow::btnGizmoTypeChanged (GUIControl* sender)
{
	if (bGizmoMode->ListBox->SelectedLine == 0)
	{
		pEditor->GetGizmo()->SetMode(TransformGizmo::GM_WORLD);
	} else
	{
		pEditor->GetGizmo()->SetMode(TransformGizmo::GM_LOCAL);
	}

	if (pEditor->GetGizmo()->GetType() == TransformGizmo::GT_ROTATE)
	{
		GZM_rotate_index = bGizmoMode->ListBox->SelectedLine;
	} else
	{
		GZM_move_index = bGizmoMode->ListBox->SelectedLine;
	}

}


void _cdecl TMainWindow::btnResetTransformPressed (GUIControl* sender)
{
	pEditor->GetGizmo()->SetTransform(Matrix());


	SaveGizmoMatrix();


}


void _cdecl TMainWindow::btnLoadGMXPressed (GUIControl* sender)
{
	GUIFileOpen* fo = NEW GUIFileOpen ();
	fo->Filters.Add (".gmx");
	fo->FiltersDesc.Add ("Geometry file");
	fo->OnOK = (CONTROL_EVENT)&TMainWindow::LoadGMXFile;
	fo->StartDirectory = string(string (pEditor->GetStartDir()) + "\\resource\\models\\").GetBuffer();
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);

}

void _cdecl TMainWindow::LoadGMXFile (GUIControl* sender)
{
	GUIFileOpen* fo = (GUIFileOpen*)sender;

	string CropPath = string(pEditor->GetStartDir()) + "\\resource\\models\\";
	string TextureName = fo->FileName;
	TextureName.GetRelativePath(CropPath);


	if (pGeom) pGeom->Release ();


	IGMXService* pGEOServ = (IGMXService*)api->GetService("GMXService");

	pGeom = pGEOServ->CreateGMX(TextureName.c_str(), null, NULL, NULL);


}

void _cdecl TMainWindow::DirectionalSelect (GUIControl* sender)
{
	IRender* pRS = (IRender*)api->GetService("DX9Render");
	Color directionalColor = pRS->GetGlobalLightColor();

	GUIColorPicker* pGUIColor = NEW GUIColorPicker(0, 0, "test", 0, 0);
	pGUIColor->bAlwaysOnTop = true;
	pGUIColor->SetScreenCenter();
	pGUIColor->labelHDR->Visible = true;
	pGUIColor->HDRPower->Visible = true;

	Vector vCol = directionalColor.v4.v;
	float HDRK = vCol.Normalize();

	pGUIColor->editA->Text = IntToStr ((int)255.0f);
	pGUIColor->editR->Text = IntToStr ((int)(vCol.x * 255.0f));
	pGUIColor->editG->Text = IntToStr ((int)(vCol.y * 255.0f));
	pGUIColor->editB->Text = IntToStr ((int)(vCol.z * 255.0f));
	pGUIColor->HDRPower->Text = FloatToStr (HDRK);
	pGUIColor->TextIsChanged (NULL);

	pGUIColor->OnApply = (CONTROL_EVENT)&TMainWindow::ApplyDirectionalColor;


	//igui->ShowModal(pGUIColor);
	Application->ShowModal(pGUIColor);
}

void _cdecl TMainWindow::AmbientSelect (GUIControl* sender)
{
	IRender* pRS = (IRender*)api->GetService("DX9Render");
	Color BackgroundColor = pRS->GetAmbient();

	GUIColorPicker* pGUIColor = NEW GUIColorPicker(0, 0, "test", 0, 0);
	pGUIColor->bAlwaysOnTop = true;
	pGUIColor->SetScreenCenter();

	pGUIColor->editA->Text = IntToStr ((int)(BackgroundColor.a * 255.0f));
	pGUIColor->editR->Text = IntToStr ((int)(BackgroundColor.r * 255.0f));
	pGUIColor->editG->Text = IntToStr ((int)(BackgroundColor.g * 255.0f));
	pGUIColor->editB->Text = IntToStr ((int)(BackgroundColor.b * 255.0f));
	pGUIColor->TextIsChanged (NULL);

	pGUIColor->OnApply = (CONTROL_EVENT)&TMainWindow::ApplyBackgroundColor;


	//igui->ShowModal(pGUIColor);
	Application->ShowModal(pGUIColor);
}

void _cdecl TMainWindow::ApplyBackgroundColor (GUIControl* sender)
{
	GUIColorPicker* pGUIColor = (GUIColorPicker*)sender;
	Color BackgroundColor = (dword)pGUIColor->SelectedColor;

	IRender* pRS = (IRender*)api->GetService("DX9Render");


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile* pEngineIni = pFS->OpenEditableIniFile(api->Storage().GetString("system.ini"), file_open_always, _FL_);
	if (pEngineIni)
	{
		pEngineIni->SetFloat("Ambient", "R", BackgroundColor.r);
		pEngineIni->SetFloat("Ambient", "G", BackgroundColor.g);
		pEngineIni->SetFloat("Ambient", "B", BackgroundColor.b);

		pEngineIni->Release();
		pEngineIni = NULL;
	}

	pRS->SetAmbient(BackgroundColor);

}

void _cdecl TMainWindow::ApplyDirectionalColor (GUIControl* sender)
{
	GUIColorPicker* pGUIColor = (GUIColorPicker*)sender;
	Color directionalColor = (dword)pGUIColor->SelectedColor;

	float HDRPower = (float)atof (pGUIColor->HDRPower->Text.c_str());


	directionalColor = directionalColor * HDRPower;

	IRender* pRS = (IRender*)api->GetService("DX9Render");


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile* pEngineIni = pFS->OpenEditableIniFile(api->Storage().GetString("system.ini"), file_open_always, _FL_);
	if (pEngineIni)
	{
		pEngineIni->SetFloat("Directional", "R", directionalColor.r);
		pEngineIni->SetFloat("Directional", "G", directionalColor.g);
		pEngineIni->SetFloat("Directional", "B", directionalColor.b);

		pEngineIni->Release();
		pEngineIni = NULL;
	}



	pRS->SetGlobalLight(pRS->GetGlobalLightDirection(), false, directionalColor, Color(0x0L));
}


void TMainWindow::Autosave ()
{
	TimeFromLastAutoSave += api->GetDeltaTime();
	if (TimeFromLastAutoSave < 180.0f) return;

	TimeFromLastAutoSave = 0.0f;


  BOOL bFldrRes = CreateDirectory(".\\ParticlesAutoSave", 0);

	string autosaveName;
	autosaveName.Format(".\\ParticlesAutoSave\\autosave_%03d.xps", AutoSaveIndex);

	AutoSaveIndex++;
	if (AutoSaveIndex > MaxAutoSaveSlots) AutoSaveIndex = 0;


	
	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	DWORD SystemCount = pManager->GetCreatedSystemCount();

	if (SystemCount > 0)
	{
		IParticleSystem* pSys = pManager->GetCreatedSystemByIndex(0);
		const char* pSysName = pSys->GetName();
		pManager->WriteSystemCacheAs(pSysName, autosaveName.c_str());
	}

}

void TMainWindow::SaveGizmoMatrix ()
{
	if (!pEditor->GetGizmo()) return;

	Matrix mGizmoTransform = pEditor->GetGizmo()->GetTransform();


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile* pIni = pFS->OpenEditableIniFile(api->Storage().GetString("system.ini"), file_open_always, _FL_);
	if (!pIni)
	{
		return;
	}


	pIni->SetFloat("peditor", "m_0_0", mGizmoTransform.m[0][0]);
	pIni->SetFloat("peditor", "m_1_0", mGizmoTransform.m[0][1]);
	pIni->SetFloat("peditor", "m_2_0", mGizmoTransform.m[0][2]);

	pIni->SetFloat("peditor", "m_0_1", mGizmoTransform.m[1][0]);
	pIni->SetFloat("peditor", "m_1_1", mGizmoTransform.m[1][1]);
	pIni->SetFloat("peditor", "m_2_1", mGizmoTransform.m[1][2]);

	pIni->SetFloat("peditor", "m_0_2", mGizmoTransform.m[2][0]);
	pIni->SetFloat("peditor", "m_1_2", mGizmoTransform.m[2][1]);
	pIni->SetFloat("peditor", "m_2_2", mGizmoTransform.m[2][2]);

	pIni->SetFloat("peditor", "m_0_3", mGizmoTransform.m[3][0]);
	pIni->SetFloat("peditor", "m_1_3", mGizmoTransform.m[3][1]);
	pIni->SetFloat("peditor", "m_2_3", mGizmoTransform.m[3][2]);


	/*
	const char* szBackground = pIni->GetString("peditor", "backgroundname", NULL);
	if (szBackground)
	{
		pBackgroundTexture = pEditor->Render()->CreateTexture(_FL_, szBackground);
	}
	*/

	pIni->Release();



}

#endif