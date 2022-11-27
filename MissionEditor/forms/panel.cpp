//*
//****************************************************************

#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

//#include "..\vtune\vtuneapi.h"
#include "..\save.h"
#include "..\load.h"
#include "panel.h"
#include "mainwindow.h"
#include "newobject.h"
#include "newfolder.h"
#include "rename.h"
#include "vsslogin.h"
#include "VSSResourceBrowser.h"
#include "..\attributes\AttributeList.h"
#include "..\..\common_h\FreeCamera.h"
#include "..\..\Common_h\InputSrvCmds.h"
#include "..\..\Common_h\corecmds.h"
#include "globalParams.h"

#include "..\SourceSafe/VSSHelper.h"

#include "..\NodesPool.h"

#include "importmis.h"

#define WINDOW_POSX   0
#define WINDOW_POSY   0
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 40

#define GRID_CLR 0xFF626262
#define GRIDZ_CLR 0xFF000000

extern IFreeCamera* pFreeCamera;
extern IGUIManager* igui;
extern MissionEditor* sMission;
extern long pCurrentSelectedCache;

extern int AdditionalWidth;
extern bool bBigPanel;



extern TreeNodesPool* globalNodesPool;

TPanelWindow* panelWnd = NULL;

float fDeltaFromLastStart = 0.0f;

//extern SourceSafeItem VSSRoot;


extern bool bThisMissionIsSourceControlFolder;
//extern bool bVSSAvailable;

extern char* IntToStr (int val);

Color BackgroundColor;
Matrix ViewPortProjectionMatrix;

/* Normal Interval Between buttons */
#define NT 34

/* Big Interval Between buttons */
#define BT 50  




bool LoadCameraPosition = true;
extern char* MakeUniqueName (const char* Name);



extern IRender * pRS;
extern TMainWindow* MainWindow;
extern BaseAttribute* pEditableNode;
extern MOSafePointer pCurrentSelected;
IMission* playing_miss = NULL;
extern int EditorMode;
extern IGUIManager* igui;
extern IMission* miss;
extern MissionEditor* sMission;
extern char* MakeUniqueName (const char* Name);



#include "..\movecontroller.h"
extern MoveController* MoveControl;

#include "..\rotatecontroller.h"
extern RotateController* RotateControl; 

#include "..\selector.h"
extern ObjectSelector* SelectControl; 


extern BaseAttribute* pRTEditedAttribute;
extern float TimeFromLastPress;

bool bWasPressed = false;

class TPanelWindowHandler;
extern TPanelWindowHandler* pHandler;


bool NeedRename = false;


//SourceSafeItem* pCurrentVSSItem = NULL;




class TPanelWindowHandler : public MissionObject
{
public:
	static GUIRectangle viewport;
	

	TPanelWindowHandler()
	{
		viewport.x = 0;
		viewport.y = 0;
		viewport.w = 1;
		viewport.h = 1;

		Assert(!pHandler);
		pHandler = this;
	}

	~TPanelWindowHandler()
	{
		Assert(pHandler == this);
		pHandler = null;
	//	DelUpdate();
	}

	virtual bool Create(MOPReader & reader)
	{		
		SetUpdate(&TPanelWindowHandler::SetViewPort, ML_FIRST - 1);
		SetUpdate(&TPanelWindowHandler::ProcessControl, ML_LAST + 1);
		return true;
	};

	void _cdecl ProcessControl ()
	{
		MoveControl->Draw();
		RotateControl->Draw();
		SelectControl->Draw();
		DrawGrid ();


		//Рисуем выделение...
		if(!miss || !pCurrentSelected.Validate())
		{
			pCurrentSelected.Reset();
		} else
		{
			IRender* pRS = (IRender*)api->GetService ("DX9Render");
			Matrix tMat;
			pCurrentSelected.Ptr()->GetMatrix (tMat);
			Vector boxmin(0.0f), boxmax(0.0f);
			pCurrentSelected.Ptr()->EditMode_GetSelectBox (boxmin, boxmax);
			pRS->DrawBox (boxmin, boxmax, tMat); 
		}



	}

	void _cdecl SetViewPort ()
	{
		pRS->Clear(0, NULL, CLEAR_TARGET, 0xFFCECEE8L, 1.0f, 0x0);

		RENDERVIEWPORT ViewPort;
		ViewPort.X = viewport.x;
		ViewPort.Y = viewport.y;
		ViewPort.Width = viewport.w;
		ViewPort.Height = viewport.h;
		ViewPort.MinZ = 0.0f;
		ViewPort.MaxZ = 1.0f;
		pRS->SetPerspective(1.0f, (float)ViewPort.Width, (float)ViewPort.Height);

		ViewPortProjectionMatrix = pRS->GetProjection();
		pRS->SetViewport(ViewPort);
		pRS->Clear(0, NULL, CLEAR_STENCIL | CLEAR_TARGET | CLEAR_ZBUFFER, BackgroundColor.GetDword(), 1.0f, 0);

		
	}

	void DrawGrid ()
	{
		if (!miss->EditMode_IsAdditionalDraw()) return;
		float StepSize = 1.0f;
		int GridSize = 12;
		float Edge = StepSize*GridSize;


		//virtual void DrawXZCircle (const Vector& center, dword dwColor, float fRadius, const char* szTechnique = "dbgLine") = 0;
		//virtual void DrawSphereGizmo (const Vector& pos, float fRadius, dword dwCOLOR, const char* szTechnique = "dbgLine") = 0;

		//pRS->DrawXZCircle(Vector(0, 1, 0), 0xFFFF0000, 2.0f);
		//pRS->DrawSphereGizmo(Vector(3, 1, 0), 1.0f, 0xFFFFFF00);


		for (int x = 0; x <= GridSize; x++)
		{
			float x1 = x * StepSize;
			float x2 = -x1;

			DWORD clr = GRID_CLR;
			if (x == 0) clr = GRIDZ_CLR;
			pRS->DrawLine(Vector (x1, 0, Edge), clr, Vector (x1, 0, -Edge), clr, false, "EditorGridLine");
			pRS->DrawLine(Vector (x2, 0, Edge), clr, Vector (x2, 0, -Edge), clr, false, "EditorGridLine");
		}

		for (int z = 0; z <= GridSize; z++)
		{
			float z1 = z * StepSize;
			float z2 = -z1;

			DWORD clr = GRID_CLR;
			if (z == 0) clr = GRIDZ_CLR;
			pRS->DrawLine(Vector (Edge, 0, z1), clr, Vector (-Edge, 0, z1), clr, false, "EditorGridLine");
			pRS->DrawLine(Vector (Edge, 0, z2), clr, Vector (-Edge, 0, z2), clr, false, "EditorGridLine");
		}
	}

};

GUIRectangle TPanelWindowHandler::viewport;
TPanelWindowHandler* pHandler = NULL;

MOP_BEGINLIST(TPanelWindowHandler, "", '1.00', 0)
MOP_ENDLIST(TPanelWindowHandler)


TPanelWindow::TPanelWindow () : GUIWindow (NULL, WINDOW_POSX, WINDOW_POSY, WINDOW_WIDTH, WINDOW_HEIGHT)
{
	panelWnd = this;

	IFileService* pFS = (IFileService*)api->GetService("FileService");


	MouseNotMovedTime = 0.0f;
	TimeFromLastAutoSave = 0.0f;
	AutoSaveIndex = 0;
	MaxAutoSaveSlots = 20;
	SaveOKTime = 0;




	bool bLeftLayout = false;

	IIniFile* pEngineIni = pFS->SystemIni();
	if (pEngineIni)
	{
		long dwBackgroundColor = pEngineIni->GetLong("mission_editor", "Background", 0);
		BackgroundColor = (dword)dwBackgroundColor;

		string Layout = pEngineIni->GetString("mission_editor", "Layout", "left");
		if (Layout == "right") bLeftLayout = true;
	}
	
	pRS->SetBackgroundColor(Color (0xFFCECEE8L));
	bMovable = false;
	DrawBackground = false;
	bPopupStyle = true;
	bAlwaysOnTop = true;

	int newWidth = pRS->GetScreenInfo3D().dwWidth;
	int newHeight = pRS->GetScreenInfo3D().dwHeight;
	SetWidth(newWidth);
	SetHeight(newHeight);

	
	
	int XPosViewPort = 249;	//249
	int XPos = 3;	//3

	if (bLeftLayout)
	{
		XPos = newWidth - 246;
		XPosViewPort = 3;	
	}



	TPanel1 = NEW TViewPort(this, XPosViewPort, 43, (newWidth-254), newHeight-60);

	if (pHandler)
	{
		pHandler->viewport = TPanel1->GetDrawRect();
	}
	

	MainWindow = NEW TMainWindow (this, XPos, 43, 253, newHeight-51-253);
	ToolsPanel = NEW GUIPanel (this, XPos+1, newHeight-51-253+40, 241, 243+6);

	PanelMessage = NEW GUILabel (ToolsPanel, 10, 10, 100, 16);
	PanelMessage->pFont->SetName("arialcyrsmall");
	PanelMessage->Caption = "";
	PanelMessage->Layout = GUILABELLAYOUT_Left;

	
	ApplyButton = NEW GUIButton (ToolsPanel, 10, 243-40, 32, 32);
	ApplyButton->Glyph->Load ("meditor\\big_ok");
	ApplyButton->FontColor = 0xFF000000;
	ApplyButton->Hint = "Create new mission (Ctrl+N)";
	ApplyButton->FlatButton = true;
	ApplyButton->FlatButtonPressedColor = 0xFFFFFFFF;
	ApplyButton->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::ApplyChanges;
	

	CancelButton = NEW GUIButton (ToolsPanel, 52, 243-40, 32, 32);
	CancelButton->Glyph->Load ("meditor\\big_cancel");
	CancelButton->FontColor = 0xFF000000;
	CancelButton->Hint = "Create new mission (Ctrl+N)";
	CancelButton->FlatButton = true;
	CancelButton->FlatButtonPressedColor = 0xFFFFFFFF;
	CancelButton->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::CancelChanges;


	dwWindowBackColor = 0xFFCECEE8;
	DrawBackground = false;


	btnCreateNewMission = NEW GUIButton (this, 5, 3, 32, 32);
	btnCreateNewMission->Glyph->Load ("meditor\\big_new");
	btnCreateNewMission->FontColor = 0xFF000000;
	btnCreateNewMission->Hint = "Create new mission (Ctrl+N)";
	btnCreateNewMission->FlatButton = true;
	btnCreateNewMission->FlatButtonPressedColor = 0xFFFFFFFF;
	btnCreateNewMission->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::CreateMissionPressed;

	btnLoadMission = NEW GUIButton (this, 5+BT, 3, 32, 32);
	btnLoadMission->Glyph->Load ("meditor\\big_fileopen");
	btnLoadMission->FontColor = 0xFF000000;
  btnLoadMission->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::LoadMissionPressed;
	btnLoadMission->Hint = "Open mission from XML (Ctrl+O)";
	btnLoadMission->FlatButton = true;
	btnLoadMission->FlatButtonPressedColor = 0xFFFFFFFF;

	/*
//
*/


	btnSaveMission = NEW GUIButton (this, 5+BT+NT, 3, 32, 32);
	btnSaveMission->Glyph->Load ("meditor\\save");
	btnSaveMission->FontColor = 0xFF000000;
  btnSaveMission->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::SaveMissionPressed;
	btnSaveMission->Hint = "Save mission to XML (Ctrl+S)";
	btnSaveMission->FlatButton = true;
	btnSaveMission->FlatButtonPressedColor = 0xFFFFFFFF;

	btnImportMission = NEW GUIButton (this, 5+BT+NT*2, 3, 32, 32);
	btnImportMission->Glyph->Load ("meditor\\big_import");
	btnImportMission->FontColor = 0xFF000000;
	btnImportMission->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::ImportPressed;
	btnImportMission->Hint = "Import mission from MSR";
	btnImportMission->FlatButton = true;
	btnImportMission->FlatButtonPressedColor = 0xFFFFFFFF;



	btnExportToEngine = NEW GUIButton (this, 5+BT+NT*3, 3, 32, 32);
	btnExportToEngine->Glyph->Load ("meditor\\big_export");
	btnExportToEngine->FontColor = 0xFF000000;
  btnExportToEngine->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::Export;
	//btnExportToEngine->OnMousePressed = (CONTROL_EVENT)InsertArrayItemPressed;
	
	btnExportToEngine->Hint = "Export mission to engine (Ctrl+E)";
	btnExportToEngine->FlatButton = true;
	btnExportToEngine->FlatButtonPressedColor = 0xFFFFFFFF;



/*	btnAddToVSS = NEW GUIButton (this, 5+(BT*2)+NT*3, 3, 32, 32);
	btnAddToVSS->Glyph->Load ("meditor\\VSS_Add");
	btnAddToVSS->DisabledGlyph->Load("meditor\\VSS_Add_dis");
	btnAddToVSS->FontColor = 0xFF000000;
	btnAddToVSS->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::VSS_Add_pressed;
	btnAddToVSS->Hint = "Add mission to Source control";
	btnAddToVSS->FlatButton = true;
	btnAddToVSS->FlatButtonPressedColor = 0xFFFFFFFF;
	btnAddToVSS->bEnabled = false;

	btnLoadMissionFromVSS = NEW GUIButton (this, 5+(BT*2)+NT*4, 3, 32, 32);
	btnLoadMissionFromVSS->Glyph->Load ("meditor\\VSS_New");
	btnLoadMissionFromVSS->DisabledGlyph->Load("meditor\\VSS_New_dis");
	btnLoadMissionFromVSS->FontColor = 0xFF000000;
	btnLoadMissionFromVSS->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::OpenFromVSS;
	btnLoadMissionFromVSS->Hint = "Open mission from Source control";
	btnLoadMissionFromVSS->FlatButton = true;
	btnLoadMissionFromVSS->FlatButtonPressedColor = 0xFFFFFFFF;
	btnLoadMissionFromVSS->bEnabled = false;

	btnCheckOut = NEW GUIButton (this, 5+(BT*2)+NT*5, 3, 32, 32);
	btnCheckOut->Glyph->Load ("meditor\\VSS_new_redaction");
	btnCheckOut->DisabledGlyph->Load("meditor\\VSS_new_redaction_dis");
	btnCheckOut->FontColor = 0xFF000000;
	btnCheckOut->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::VSS_CheckOut_pressed;
	btnCheckOut->Hint = "Check Out";
	btnCheckOut->FlatButton = true;
	btnCheckOut->FlatButtonPressedColor = 0xFFFFFFFF;
	btnCheckOut->bEnabled = false;

	btnCheckIn = NEW GUIButton (this, 5+(BT*2)+NT*6, 3, 32, 32);
	btnCheckIn->Glyph->Load ("meditor\\VSS_redaction_add");
	btnCheckIn->DisabledGlyph->Load("meditor\\VSS_redaction_add_dis");
	btnCheckIn->FontColor = 0xFF000000;
	btnCheckIn->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::VSS_CheckIn_pressed;
	btnCheckIn->Hint = "Check In";
	btnCheckIn->FlatButton = true;
	btnCheckIn->FlatButtonPressedColor = 0xFFFFFFFF;
	btnCheckIn->bEnabled = false;

	btnUndoCheckOut = NEW GUIButton (this, 5+(BT*2)+NT*7, 3, 32, 32);
	btnUndoCheckOut->Glyph->Load ("meditor\\VSS_Undo");
	btnUndoCheckOut->DisabledGlyph->Load("meditor\\VSS_Undo_dis");
	btnUndoCheckOut->FontColor = 0xFF000000;
	btnUndoCheckOut->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::VSS_UndoCheckOut_pressed;
	btnUndoCheckOut->Hint = "Undo Check Out";
	btnUndoCheckOut->FlatButton = true;
	btnUndoCheckOut->FlatButtonPressedColor = 0xFFFFFFFF;
	btnUndoCheckOut->bEnabled = false;

	btnGetLatestVersion = NEW GUIButton (this, 5+(BT*2)+NT*8, 3, 32, 32);
	btnGetLatestVersion->Glyph->Load ("meditor\\VSS_renew");
	btnGetLatestVersion->DisabledGlyph->Load("meditor\\VSS_renew_dis");
	btnGetLatestVersion->FontColor = 0xFF000000;
	btnGetLatestVersion->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::VSS_GetLatest_pressed;
	btnGetLatestVersion->Hint = "Get Latest Version";
	btnGetLatestVersion->FlatButton = true;
	btnGetLatestVersion->FlatButtonPressedColor = 0xFFFFFFFF;
	btnGetLatestVersion->bEnabled = false;
*/

	int vOf = 0;

	btnCreateNew = NEW GUIButton (this, vOf+5+(BT*2)+NT*3, 3, 32, 32);
	btnCreateNew->Glyph->Load ("meditor\\newobject");
	btnCreateNew->FontColor = 0xFF000000;
  btnCreateNew->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnCreateObjectPressed;
	btnCreateNew->Hint = "Create new mission object (Ctrl+I)";
	btnCreateNew->FlatButton = true;
	btnCreateNew->FlatButtonPressedColor = 0xFFFFFFFF;

/*
	btnCreateEvent = NEW GUIButton (this, 5+(BT*2)+NT*4, 3, 32, 32);
	btnCreateEvent->Glyph->Load ("meditor\\newevent");
	btnCreateEvent->FontColor = 0xFF000000;
	btnCreateEvent->Hint = "Create new event";
	btnCreateEvent->FlatButton = true;
	btnCreateEvent->FlatButtonPressedColor = 0xFFFFFFFF;
*/



	btnDeleteSelected = NEW GUIButton  (this, vOf+5+(BT*2)+NT*4, 3, 32, 32); 
	btnDeleteSelected->Glyph->Load ("meditor\\big_delete");
	btnDeleteSelected->FontColor = 0xFF000000;
  btnDeleteSelected->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::DeleteSelectedPressed;
	btnDeleteSelected->Hint = "Delete selected object(Del)";
	btnDeleteSelected->FlatButton = true;
	btnDeleteSelected->FlatButtonPressedColor = 0xFFFFFFFF;

	btnCreateFolder = NEW GUIButton (this, vOf+5+(BT*2)+NT*5, 3, 32, 32);
	btnCreateFolder->Glyph->Load ("meditor\\new_folder");
	btnCreateFolder->FontColor = 0xFF000000;
  btnCreateFolder->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::CreateFolderPressed;
	btnCreateFolder->Hint = "Create new folder (F7)";
	btnCreateFolder->FlatButton = true;
	btnCreateFolder->FlatButtonPressedColor = 0xFFFFFFFF;


	btnRenameObjects = NEW GUIButton (this, vOf+5+(BT*2)+NT*6, 3, 32, 32);
	btnRenameObjects->Glyph->Load ("meditor\\big_rename");
	btnRenameObjects->FontColor = 0xFF000000;
  btnRenameObjects->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::RenameObjectPressed;
	btnRenameObjects->Hint = "Rename object (F6)";
	btnRenameObjects->FlatButton = true;
	btnRenameObjects->FlatButtonPressedColor = 0xFFFFFFFF;


	btnPlayGame = NEW GUIButton (this, vOf+5+(BT*3)+NT*6, 3, 32, 32);
	btnPlayGame->Glyph->Load ("meditor\\run_game");
	btnPlayGame->FontColor = 0xFF000000;
  btnPlayGame->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::RunGamePressed;
	btnPlayGame->Hint = "Play game (F1)";
	btnPlayGame->FlatButton = true;
	btnPlayGame->FlatButtonPressedColor = 0xFFFFFFFF;
	
	vOf -= 32;

	btnMoveObjects = NEW GUIButton (this, vOf+5+(BT*4)+NT*8, 3, 32, 32);
	btnMoveObjects->Glyph->Load ("meditor\\big_move");
	btnMoveObjects->FontColor = 0xFF000000;
  btnMoveObjects->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnMoveObjectsPressed;
	btnMoveObjects->Hint = "Move";
	btnMoveObjects->FlatButton = true;
	btnMoveObjects->FlatButtonPressedColor = 0xFFEECC55;
	btnMoveObjects->GroupIndex = 1;



	btnRotateObjects = NEW GUIButton (this, vOf+5+(BT*4)+NT*9, 3, 32, 32);
	btnRotateObjects->Glyph->Load ("meditor\\big_rotate");
	btnRotateObjects->FontColor = 0xFF000000;
  btnRotateObjects->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnRotateObjectsPressed;
	btnRotateObjects->Hint = "Rotate";
	btnRotateObjects->FlatButton = true;
	btnRotateObjects->FlatButtonPressedColor = 0xFFEECC55;
	btnRotateObjects->GroupIndex = 1;


	btnSelectObjects = NEW GUIButton (this, vOf+5+(BT*4)+NT*7, 3, 32, 32);
	btnSelectObjects->Glyph->Load ("meditor\\big_normalmode");
	btnSelectObjects->FontColor = 0xFF000000;
  btnSelectObjects->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnSelectObjectsPressed;
	btnSelectObjects->Hint = "Normal";
	btnSelectObjects->FlatButton = true;
	btnSelectObjects->FlatButtonPressedColor = 0xFFEECC55;
	btnSelectObjects->GroupIndex = 1;
	btnSelectObjects->Down = true;


	btnChangeBackgroundColor = NEW GUIButton (this, vOf+5+(BT*4)+NT*10, 3, 32, 32);
	btnChangeBackgroundColor->Glyph->Load ("meditor\\background");
	btnChangeBackgroundColor->FontColor = 0xFF000000;
	btnChangeBackgroundColor->Hint = "Change backround color";
	btnChangeBackgroundColor->FlatButton = true;
	btnChangeBackgroundColor->FlatButtonPressedColor = 0xFFEECC55;
	btnChangeBackgroundColor->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnSelectBackground;


	btnImportMissionsList = NEW GUIButton (this, vOf+5+(BT*4)+NT*11, 3, 32, 32);
	btnImportMissionsList->Glyph->Load ("meditor\\vss_new");
	btnImportMissionsList->FontColor = 0xFF000000;
	btnImportMissionsList->Hint = "Import missions list";
	btnImportMissionsList->FlatButton = true;
	btnImportMissionsList->FlatButtonPressedColor = 0xFFEECC55;
	btnImportMissionsList->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnImportMissionsListAction;



	vOf -= 20;
	vOf += 30;


	MissionName = NEW GUIEdit (this, vOf+5+(BT*4)+NT*12, 7, 200, 24);
	MissionName->pFont->SetName("arialcyrsmall");
	MissionName->Text = "";
	MissionName->Flat = true;
	MissionName->OnAccept = (CONTROL_EVENT)&TPanelWindow::MissionNameIsChange;

	LabelMissionName = NEW GUILabel (this, vOf+5+(BT*4)+NT*12, 7, 200, 24);
	LabelMissionName->pFont->SetName("arialcyrsmall");
	LabelMissionName->Caption = "";
	LabelMissionName->Visible = false;
	LabelMissionName->Layout = GUILABELLAYOUT_Left;


	chckAdditionalDraw = NEW GUICheckBox (this, vOf+5+(BT*4)+NT*12+ 210, 7, 200, 24);
	chckAdditionalDraw->Checked = miss->EditMode_IsAdditionalDraw();
	chckAdditionalDraw->Caption = "Additional draw";
	chckAdditionalDraw->FontColor = 0xFF000000;
	chckAdditionalDraw->pFont->SetName("arialcyrsmall");
	chckAdditionalDraw->ImageChecked->Load ("checked");
	chckAdditionalDraw->ImageNormal->Load ("normal");
	chckAdditionalDraw->OnChange = (CONTROL_EVENT)&TPanelWindow::ChangeAdditionalDraw;


	btnCreatePKXFromFolder = NEW GUIButton (this, vOf+5+(BT*4)+NT*12+ 210+200, 3, 32, 32);
	btnCreatePKXFromFolder->Glyph->Load ("meditor\\zipfolder");
	btnCreatePKXFromFolder->FontColor = 0xFF000000;
	btnCreatePKXFromFolder->Hint = "Create PKX from Folder";
	btnCreatePKXFromFolder->FlatButton = true;
	btnCreatePKXFromFolder->FlatButtonPressedColor = 0xFFEECC55;
	btnCreatePKXFromFolder->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnCreatePKX;

	btnConvertTexturesInFolder = NEW GUIButton (this, vOf+5+(BT*4)+NT*12+ 210+200+34+4, 3, 32, 32);
	btnConvertTexturesInFolder->Glyph->Load ("meditor\\txx360");
	btnConvertTexturesInFolder->FontColor = 0xFF000000;
	btnConvertTexturesInFolder->Hint = "Convert all textures in folder to X360 format";
	btnConvertTexturesInFolder->FlatButton = true;
	btnConvertTexturesInFolder->FlatButtonPressedColor = 0xFFEECC55;
	btnConvertTexturesInFolder->OnMousePressed = (CONTROL_EVENT)&TPanelWindow::btnConvertTextures;

	


	CreateEmptyMission(this);
}

void _cdecl TPanelWindow::btnConvertTextures (GUIControl* sender)
{
	GUIFileOpen* fo = NEW GUIFileOpen ();

	fo->Filters.Add ("*.txx");
	fo->FiltersDesc.Add ("Any texture file from folder");

	fo->OnOK = (CONTROL_EVENT)&TPanelWindow::convertTexturesEvent;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (0);
	fo->MakeFlat(true);
}

//#include "..\..\System\XRender\X360TexConvertor.h"
//#include "..\..\System\XRender\X360TexConvertor.cpp"


void _cdecl TPanelWindow::convertTexturesEvent (GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;

	string folderPath;
	folderPath.GetFilePath(dialog->FileName);

	api->Trace("'%s'", folderPath.c_str());

	IFileService* pFS = &miss->Files();

	IFinder * fnd = pFS->CreateFinder(folderPath.c_str(), "*.txx", find_no_mirror_files | find_no_files_from_packs, _FL_);
	string destPath;
	for(dword i = 0; i < fnd->Count(); i++)
	{
		destPath = fnd->FilePath(i);
		destPath += "360";
//		XboxConvert_txx(fnd->FilePath(i), destPath.c_str());
	}
	fnd->Release();

	Application->MessageBox("All textures in folder converted from .txx to .txx360", "Work Done", GUIMB_OK);

}


void _cdecl TPanelWindow::btnCreatePKX (GUIControl* sender)
{
	GUIFileOpen* fo = NEW GUIFileOpen ();

	fo->Filters.Add ("*.*");
	fo->FiltersDesc.Add ("Any file from folder");
	
	fo->OnOK = (CONTROL_EVENT)&TPanelWindow::createPKXEvent;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (0);
	fo->MakeFlat(true);
}

void _cdecl TPanelWindow::createPKXEvent (GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;

	string folderPath;
	folderPath.GetFilePath(dialog->FileName);

	api->Trace("'%s'", folderPath.c_str());


	string tmp = folderPath;
	tmp.DeleteLastSymbol();
	tmp.DeleteLastSymbol('/');

	string fakeFileName;
	fakeFileName.GetFileName(tmp);

	

	string pakName = "DataPKX\\";
	pakName += fakeFileName;
	pakName += ".pkx";

	IFileService* pFS = &miss->Files();

	pFS->BuildPack(pakName.c_str(), folderPath.c_str(), "*", pack_cmpr_method_archive);

	
	string message;
	message.Format("Pak file created and placed to '%s'", pakName.c_str());

	Application->MessageBox(message.c_str(), "Work Done", GUIMB_OK);



}


void _cdecl TPanelWindow::MissionNameIsChange (GUIControl* sender)
{
#ifndef NO_TOOLS
	miss->EditorSetPack (MissionName->Text.GetBuffer());
#endif
	MissionName->Enabled = false;
	MissionName->Visible = false;


	LabelMissionName->Caption = MissionName->Text;
	LabelMissionName->Visible = true;

	GetMainControl()->Application->GetCursor ()->Pop ();
}

TPanelWindow::~TPanelWindow ()
{
	miss->DeleteMission();
	miss = NULL;
}


void TPanelWindow::OnCreate()
{
	SetFocus();
}


void _cdecl TPanelWindow::btnCreateObjectPressed (GUIControl* sender)
{
	pEditableNode = NULL;
	TNewObjectWindow* newObjectwnd = NEW TNewObjectWindow ();

	Application->ShowModal (newObjectwnd);

}


void _cdecl TPanelWindow::RunGamePressed (GUIControl* sender)
{
	//GUIMessageBox* mb = igui->MessageBox ("Press 'F1' to return Editor", "Message", GUIMB_OK);

	Sleep (100);
	RunGame (this);
	//mb->OnOK = (CONTROL_EVENT)RunGame;
}

void _cdecl TPanelWindow::ExportPressed (GUIControl* sender)
{
/*
	GUIFileSave* fo = NEW GUIFileSave ();

	fo->Filters.Add (".mis");
	fo->FiltersDesc.Add ("Mission (Engine)");

	
	fo->StartDirectory = StartDirectory + "\\resource\\mission";

	api->fio->_CreateDirectory(StartDirectory, NULL);
	//fo->Filter->ListBox->
	//fo->RefreshFilters ();
	fo->OnOK = (CONTROL_EVENT)Export;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
*/
}


void _cdecl TPanelWindow::Export (GUIControl* sender)
{
	//GUIFileSave* dialog = (GUIFileSave*)sender;

	

	


	MOPCollector collector;

	int count = sMission->GetCreatedMissionObjectsCount();
	for (int n =0; n < count; n++)
	{
		MissionEditor::tCreatedMO* m = &sMission->GetCreatedMissionObjectStructByIndex(n);
		MOPWriter wrt(m->Version, m->pObject.Ptr()->GetObjectID().c_str(), m->ClassName);

		MissionEditor::tCreatedMO& stru = sMission->GetCreatedMissionObjectStructByIndex(n);
		stru.AttrList->AddToWriter (wrt);
		collector.Add(m->Level, wrt);
	}

	//string path = dialog->FileName;
	string path; 
#ifndef NO_TOOLS
	miss->EditorGetMisPath(path);
#endif

	SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	pFS->Delete(path);

	if (pFS->IsExist(path))
	{
		::MessageBox(NULL, "Can't export MSR !!!", "Export error !!", MB_OK);
		throw;
	}

	IFile* pFile = pFS->OpenFile(path, file_create_always, _FL_);

	if (pFile)
	{
		void * data = 0;
		dword size = 0;
		collector.GetResultData(data, size);
		pFile->Write(data, size);
		pFile->Release();
		delete data;
	}

	MGIterator & it = miss->GroupIterator(MG_EXPORT, _FL_);
	bool isExportError = false;
	for(it.Reset(); !it.IsDone(); it.Next())
	{
		if(!it.Get()->EditMode_Export())
		{
			::MessageBox(NULL, "Can't export object data !!!", "Export error !!", MB_OK);
			throw;
		}
	}
	it.Release();
}


bool TPanelWindow::InsideReadOnlyFolder (GUITreeNode * node)
{
	if (!node) return false;

	if (node->bReadOnly) return true;

	if (!node->Parent) return false;

	return InsideReadOnlyFolder(node->Parent);
}


bool TPanelWindow::HaveReadOnlyChilds (GUITreeNode * node)
{
	if (!node) return false;

	if (node->bReadOnly) return true;

	int count = node->Childs.GetCount();
	for (int n = 0; n < count; n++)
	{
		GUITreeNode* nd = node->Childs.Get(n);
		if (HaveReadOnlyChilds(nd) == true) return true;
	}

	return false;
}


void _cdecl TPanelWindow::DeleteSelectedPressed (GUIControl* sender)
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == NULL) return;

	if (sNode->bReadOnly)
	{
		Application->MessageBox("Can't delete read only object", "Warning", GUIMB_OK);
		return;
	}

	if (HaveReadOnlyChilds (sNode))
	{
		Application->MessageBox("Can't delete object that have read only childs !!!", "Warning", GUIMB_OK);
		return;
	}

	
	if ((sNode->Tag != TAG_ARRAYITEM) && (sNode->Tag != TAG_FOLDER) && (sNode->Tag != TAG_OBJECT)) return;
	Application->MessageBox("Delete selected object ?", "Warning", GUIMB_OKCANCEL)->OnOK = (CONTROL_EVENT)&TPanelWindow::RealDeleteSelected;
}

GUITreeNode* TPanelWindow::FindUpperArrayItem (GUITreeNode* m_node)
{
	if (m_node->Tag == TAG_ARRAYITEM) return m_node;
	if (m_node->Parent == NULL) return NULL;
	return FindUpperArrayItem (m_node->Parent);
}

void _cdecl TPanelWindow::RealInsertArrayItem (GUIControl* sender)
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == NULL) return;

	bool isArray = false;
	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		BaseAttribute* battr = (BaseAttribute*)sNode->Data;
		if (battr->GetType() == IMOParams::t_array) isArray = true;;
	}

	if (!isArray)
	{
		GUITreeNode* ArrayItem = FindUpperArrayItem (sNode);
		if (ArrayItem) sNode = ArrayItem;
	}
	

	if ((sNode->Tag != TAG_ARRAYITEM) && (sNode->Tag != TAG_ATTRIBUTE)) return;

	if (sNode->Tag == TAG_ARRAYITEM)
	{
		ArrayAttribute::ArrayItemInfo* ItemInfo = (ArrayAttribute::ArrayItemInfo*)sNode->Data;
		if (ItemInfo == NULL) return;
		ArrayAttribute::ArrayElement* element = (ArrayAttribute::ArrayElement*)ItemInfo->element;
		ArrayAttribute* arrayattrib =	ItemInfo->array;
		if (element == NULL) return;
		if (arrayattrib == NULL) return;
		int index = arrayattrib->GetElementIndex(element);
		if (index == -1) return;
		arrayattrib->InsertValue(index+1, element);

		//
		pEditableNode = arrayattrib;
		MainWindow->TreeView1->SetSelectedNode(sNode, false);
		//MainWindow->TreeView1->ResetSelection(*MainWindow->TreeView1->Items);
	}

	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		BaseAttribute* battr = (BaseAttribute*)sNode->Data;
		if (battr->GetType() != IMOParams::t_array) return;
		ArrayAttribute* arrayattrib =	(ArrayAttribute*)battr;
		arrayattrib->InsertValue(0);
		pEditableNode = arrayattrib;
	}


	
	MainWindow->UpdateTree (this);
}

void _cdecl TPanelWindow::InsertArrayItemPressed (GUIControl* sender)
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == null) return;
	if ((sNode->Tag != TAG_ARRAYITEM) && (sNode->Tag != TAG_ATTRIBUTE)) return;

/*
	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		BaseAttribute* battr = (BaseAttribute*)sNode->Data;
		if (battr->GetType() != IMOParams::t_array) return;
	}
 */
	Application->MessageBox("Insert array item ?", "Warning", GUIMB_OKCANCEL)->OnOK = (CONTROL_EVENT)&TPanelWindow::RealInsertArrayItem;
}

void _cdecl TPanelWindow::RealDeleteSelected (GUIControl* sender)
{
	// Надо просто удалить мою запись и удалить MissionObject
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (!sNode) return;

	GUITreeNode* sParent = sNode->Parent;

	if (sNode == NULL) return;


	if (sNode->Tag == TAG_ARRAYITEM)
	{
		ArrayAttribute::ArrayItemInfo* ItemInfo = (ArrayAttribute::ArrayItemInfo*)sNode->Data;
		if (ItemInfo == NULL) return;
		ArrayAttribute::ArrayElement* element = (ArrayAttribute::ArrayElement*)ItemInfo->element;
		ArrayAttribute* arrayattrib =	ItemInfo->array;
		if (element == NULL) return;
		if (arrayattrib == NULL) return;

		pEditableNode = arrayattrib;
		int index = arrayattrib->GetElementIndex(element);
		if (index == -1) return;
		arrayattrib->RemoveValue(index);
		MainWindow->TreeView1->ResetSelection(*MainWindow->TreeView1->Items);
		MainWindow->TreeView1->SetSelectedNode(sNode->Parent, false);
		MainWindow->UpdateTree (this);
		return;
	}


	// Это объект миссии
	if (sNode->Tag ==TAG_OBJECT)
	{

		//SOME BUG HERE !!!!
		//FIXME !!! AXTUNG !!!
		
		

		MissionEditor::tCreatedMO* pMo = (MissionEditor::tCreatedMO*)sNode->Data;
		if (pMo)
		{
			MOSafePointer objToDelete = pMo->pObject;

			// Удаляем из списка созданных объектов
			
			
			
			DWORD total_nodes = sMission->GetCreatedMissionObjectsCount();
			for (DWORD n = 0; n < total_nodes; n++)
			{
				MissionEditor::tCreatedMO* cObj = &sMission->GetCreatedMissionObjectStructByIndex(n);
				if (cObj->pObject == objToDelete)
				{
					// Убиваем аттрибуты объекта...
					delete cObj->AttrList;
					//sMission->CreatedMO.DelIndex (n);
					sMission->DeleteCreatedMissionObject(n);
					break;
				}
			}
		
			delete objToDelete.Ptr();
			objToDelete.Reset();
		}
		
		

		// Удаляем из дерева...

		sNode->Release ();

		
		MainWindow->TreeView1->ResetSelection(*MainWindow->TreeView1->Items);
		


		// Убиваем текущий выбранный объект...
		pCurrentSelected.Reset();

		MainWindow->TreeView1->SetSelectedNode (sParent, false);
		
		

		return;
	}

		
	// Это папка...
	if (sNode->Tag ==TAG_FOLDER)
	{
		if (sNode->Childs.GetCount () == 0)
		{
			// Удаляем папку...
			sNode->Release ();
		} else
		{
			//igui->MessageBox ("Folder is not empty. Can't delete.", "Error", GUIMB_OK);
			RecursiveDelete (sNode);
			sNode->Release ();
			MainWindow->TreeView1->ResetSelection(*MainWindow->TreeView1->Items);
			return;
		}


	}


	/*if (TreeView1->Items->GetCount () > 0)
		TreeView1->SetSelectedNode (TreeView1->Items->Get (0));
	else*/

	if (sParent) MainWindow->TreeView1->SetSelectedNode (sParent, false);
		

}



void _cdecl TPanelWindow::SaveMissionPressed (GUIControl* sender)
{
	GUIFileSave* fo = NEW GUIFileSave ();
	
	fo->Filters.Add (".xmlz");
	fo->FiltersDesc.Add ("Mission (Source)");

	//fo->Filter->ListBox->
	//fo->RefreshFilters ();
	fo->OnOK = (CONTROL_EVENT)&TPanelWindow::Save;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);

}
	
void _cdecl TPanelWindow::LoadMissionPressed (GUIControl* sender)
{
	pRTEditedAttribute = NULL;
	GUIFileOpen* fo = NEW GUIFileOpen ();

	fo->Filters.Add (".xmlz");
	fo->FiltersDesc.Add ("Mission zip (Source)");
	fo->Filters.Add (".xml");
	fo->FiltersDesc.Add ("Mission (Source)");


//	fo->RefreshFilters ();
	fo->OnOK = (CONTROL_EVENT)&TPanelWindow::Load;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TPanelWindow::ImportPressed (GUIControl* sender)
{
	GUIFileOpen* fo = NEW GUIFileOpen ();

	fo->Filters.Add (".xmlz");
	fo->FiltersDesc.Add ("Mission zip (Source)");
	fo->Filters.Add (".xml");
	fo->FiltersDesc.Add ("Mission (Source)");
	//	fo->RefreshFilters ();
	fo->OnOK = (CONTROL_EVENT)&TPanelWindow::Import;
	Application->ShowModal (fo);
	fo->Filter->SelectItem (1);
	fo->MakeFlat(true);
}

void _cdecl TPanelWindow::CreateFolderPressed (GUIControl* sender)
{
	TNewFolder* cFolder = NEW TNewFolder;
	cFolder->OnClose = (CONTROL_EVENT)&TPanelWindow::CreateFolder;
	igui->ShowModal (cFolder);
}


void _cdecl TPanelWindow::RunGame (GUIControl* sender)
{
	ForceAutosave();



	IFileService* pFS = &miss->Files();




	string path; 
#ifndef NO_TOOLS
	miss->EditorGetMisPath(path);
#endif
	if (path.IsEmpty())
	{
		Application->MessageBox("Mission name is incorrect\n#cFF0000Can't continue", "Error", GUIMB_OK);
		return;
	}


	SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);

	pFS->Delete(path);


	if (pFS->IsExist(path))
	{
		Application->MessageBox("Can't export MSR file !!!\n#cFF0000Can't continue", "Error", GUIMB_OK);
	}


	//string StDir = StartDirectory + "\\resource\\mission";
	//api->fio->_CreateDirectory(StDir, NULL);

	// Экспортим во временную папочку...
	MOPCollector collector;
	
	//int count = sMission->CreatedMO.Size ();
	int count = sMission->GetCreatedMissionObjectsCount();
	for (int n =0; n < count; n++)
	{
		//m->pObject->get
		MissionEditor::tCreatedMO* m = &sMission->GetCreatedMissionObjectStructByIndex(n);
		MOPWriter wrt(m->Version, m->pObject.Ptr()->GetObjectID().c_str(), m->ClassName);

		MissionEditor::tCreatedMO& stru = sMission->GetCreatedMissionObjectStructByIndex(n);
		stru.AttrList->AddToWriter (wrt);
		collector.Add(m->Level, wrt);
	}
	
//	string name = "TempRunGame";
//	string path = "Resource\\Mission\\";
//	path += name;
	//api->fio->_CreateDirectory(path.GetBuffer(), 0);
//  path += ".mis";



	//IFile* pFile = pFS->Write(path, iw_create_always, _FL_);
	//if (pFile)

	/*
	HANDLE hFile = CreateFile(path, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, null);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		void * data = 0;
		dword size = 0;
		collector.GetResultData(data, size);

		DWORD dwWrited = 0;
		WriteFile(hFile, data, size, &dwWrited, null);
		//pFile->Write(data, size);
		//pFile->Release();
		delete data;

		CloseHandle(hFile);
	}
	*/
	void * data = 0;
	dword size = 0;
	collector.GetResultData(data, size);
	pFS->SaveData(path, data, size);
	delete data;






	/*
	MGIterator & it = miss->GroupIterator(MG_OBJECTS, _FL_);
	bool isExportError = false;
	for(it.Reset(); !it.IsDone(); it.Next())
	{
		if(!it.Get()->EditMode_Export())
		{
			::MessageBox(NULL, "Can't export object data !!!", "Export error !!", MB_OK);
			throw;
		}
	}
	it.Release();
	*/
	MGIterator & it = miss->GroupIterator(MG_EXPORT, _FL_);
	bool isExportError = false;
	for(it.Reset(); !it.IsDone(); it.Next())
	{
		if(!it.Get()->EditMode_Export())
		{
			::MessageBox(NULL, "Can't export object data !!!", "Export error !!", MB_OK);
			throw;
		}
	}
	it.Release();

	
	/* ------------- создает pkx для mis файла --------------------------------*/
	string pakName = "DataPKX\\d_";
	pakName += miss->GetMissionName();
	pakName += ".pkx";

	string dir;
	dir.GetFilePath(path);
	pFS->BuildPack(pakName.c_str(), dir.c_str(), "*", pack_cmpr_method_archive);


//тут нужно усыпить миссию.

#ifndef NO_TOOLS
	miss->EditorSetSleep (true);
#endif

	pFreeCamera->Pause (true);


	igui->Enable(false);


	miss->Controls().ExecuteCommand(InputSrvLockMouse(false));
	api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(false));
	//miss->Controls().LockDebugKeys(false);	
	//miss->Controls().LockMouseCursorPos(false);
	TPanel1->isFly = false;
	GetMainControl()->Application->ShowCursor (true);


	// Создали новую, чистенькую...
	playing_miss = (IMission *)api->CreateObject("Mission");
	// Загрузили ее из файла...
	float cnt = 0.0f;
	playing_miss->CreateMission(MissionName->Text.GetBuffer(), 100.0f, cnt);
	
	EditorMode = false;

	miss->Controls().EnableControlGroup ("CDBuilder", false);
	miss->Controls().EnableControlGroup ("GUIADD", false);
	miss->Controls().EnableControlGroup ("GUI", false);
	miss->Controls().EnableControlGroup ("mission", true);
	miss->Controls().EnableControlGroup ("FreeCamera", true);

	//CMResume();
}



void TPanelWindow::SaveProcess (const char* szFileName, bool autosave)
{
	TimeFromLastAutoSave = 0.0f;

	DWORD dwAttr = GetFileAttributes(szFileName);

	if (dwAttr != INVALID_FILE_ATTRIBUTES)
	{
		if ((dwAttr & FILE_ATTRIBUTE_READONLY) > 0)
		{
			igui->MessageBox("This FILE have READ ONLY flag!\nIf this file under source control Checkout file first !\n", "Can't save", GUIMB_OK);
			return;
		}
	}



	gp->LoadedMissionName = szFileName;
	gp->LoadedMissionDir.GetFilePath(gp->LoadedMissionName);


/*	const char* szResult = strstr (szFileName, MissionsSrcLocalFolder.c_str());
	if (szResult == szFileName)
	{
		SetThisMissionIsSourceControlFolder(true);
	} else
	{
		SetThisMissionIsSourceControlFolder(false);
	}
*/

	MissionSave* msaver = NEW MissionSave(sMission);
	msaver->SaveXML (szFileName);

	delete msaver;

	SetFocus();

	SaveOKTime = SAVEDOK_SHOWTIME;


	if (!autosave)
	{
		gp->loadedFileName = szFileName;
	}
//	VSSAfterLoad(szFileName);

}

void _cdecl TPanelWindow::Save (GUIControl* sender)
{
	GUIFileOpen* dialog = (GUIFileOpen*)sender;

	
	gp->SaveBoxText.GetFileName(dialog->FileName);
	SaveProcess(dialog->FileName, false);

/*
	LoadedMissionName = dialog->FileName;
	LoadedMissionDir.GetFilePath(LoadedMissionName);


	const char* szResult = strstr (dialog->FileName.c_str(), MissionsSrcLocalFolder.c_str());
	if (szResult == dialog->FileName.c_str())
	{
		SetThisMissionIsSourceControlFolder(true);
	} else
	{
		SetThisMissionIsSourceControlFolder(false);
	}


	MissionSave* msaver = NEW MissionSave(sMission);
	msaver->SaveXML (dialog->FileName);

	delete msaver;

	SetFocus();

	VSSAfterLoad(dialog->FileName);
*/
}

void _cdecl TPanelWindow::Load (GUIControl* sender)
{
/*
	LoadCameraPosition = true;
	CreateEmptyMission (this);

	IGUIManager::DefferedEvent dEvent;
	dEvent.event.SetHandler(this, (CONTROL_EVENT)DefferedLoad);
	dEvent.frame_to_execute = 5;
	GetMainControl()->Application->AddDefferedEvent(dEvent);

	defer_load_name = dialog->FileName;
*/
	GUIFileOpen* dialog = (GUIFileOpen*)sender;
	StartLoad(dialog->FileName);
}


void TPanelWindow::StartLoad (const char* szMissName)
{

	LoadCameraPosition = true;
	CreateEmptyMission (this);

	/*
	IGUIManager::DefferedEvent dEvent;
	dEvent.event.SetHandler(this, (CONTROL_EVENT)&TPanelWindow::DefferedLoad);
	dEvent.frame_to_execute = 5;
	GetMainControl()->Application->AddDefferedEvent(dEvent);
	*/
	gp->defer_load_name = szMissName;


	gp->loadedFileName = szMissName;

	TPanelWindow::DefferedLoad(NULL);


}

void _cdecl TPanelWindow::Import (GUIControl* sender)
{
	LoadCameraPosition = false;
	//CreateEmptyMission (this);

	IGUIManager::DefferedEvent dEvent;
	dEvent.event.SetHandler(this, (CONTROL_EVENT)&TPanelWindow::DefferedImport);
	dEvent.frame_to_execute = 5;
	GetMainControl()->Application->AddDefferedEvent(dEvent);

	GUIFileOpen* dialog = (GUIFileOpen*)sender;
	gp->defer_load_name = dialog->FileName;
}

void _cdecl TPanelWindow::DefferedImport (GUIControl* sender)
{
	MissionLoad* mloader = NEW MissionLoad(sMission);
	mloader->LoadXML (gp->defer_load_name, LoadCameraPosition, false);
	delete mloader;
	SetFocus();

	MainWindow->SortTree ();
}

void _cdecl TPanelWindow::DefferedLoad (GUIControl* sender)
{
//-----------------------------------------------------------
//MissionsSrcLocalFolder

/*	const char* szResult = strstr (defer_load_name.c_str(), MissionsSrcLocalFolder.c_str());
	if (szResult == defer_load_name.c_str())
	{
		SetThisMissionIsSourceControlFolder(true);
	} else
	{
		SetThisMissionIsSourceControlFolder(false);
	}
*/	

	MissionLoad* mloader = NEW MissionLoad(sMission);
	mloader->LoadXML (gp->defer_load_name, LoadCameraPosition, false);
	delete mloader;
	SetFocus();


//	VSSAfterLoad (defer_load_name);

	gp->LoadedMissionName = gp->defer_load_name;
	gp->LoadedMissionDir.GetFilePath(gp->LoadedMissionName);


	MainWindow->SortTree ();

	miss->Controls().ExecuteCommand(InputSrvLockMouse(false));
	//miss->Controls().LockMouseCursorPos(false);

}

void _cdecl TPanelWindow::CreateFolder (GUIControl* sender)
{
	if (gp->folder_to_create.IsEmpty ()) return;

	GUITreeNode* tn = MainWindow->TreeView1->GetSelectedNode ();
	
	if (tn)
	{

		if (InsideReadOnlyFolder (tn))
		{
			Application->MessageBox("Can't create folder in read only folder !!!", "Warning", GUIMB_OK);
			return;
		}




		if (tn->Tag != TAG_FOLDER)
		{
			if (tn->Parent != NULL) return;
			
			tn = NULL;
		}
	}
	
	
	GUITreeNodes* listToAdd = NULL;
	if (tn)
	{
		tn->Expanded = true;
		MainWindow->TreeView1->SetExpandedOrCollapsedNode (tn);
		listToAdd = &tn->Childs;
	} else
	{
		listToAdd = MainWindow->TreeView1->Items;
	}


	for (int n = 0; n < listToAdd->GetCount (); n++)
	{
		if (gp->folder_to_create == listToAdd->Get (n)->GetText())
		{
			// Одинаковые имена у папок...
			igui->MessageBox ("Folder already exist !", "Error", GUIMB_OK);
			return;
		}
		
	}

	GUITreeNode* newfolder = globalNodesPool->CreateNode();
	newfolder->SetText(gp->folder_to_create);
	newfolder->Image->Load ("folder");
	newfolder->CanCopy = false;
	
	listToAdd->Add (newfolder);

	//TreeView1->SetSelectedNode (newfolder);
}



bool TPanelWindow::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	return GUIWindow::ProcessMessages( message,  lparam,  hparam);
}

void TPanelWindow::ForceAutosave ()
{

	TimeFromLastAutoSave = 0.0f;


	BOOL bFldrRes = CreateDirectory(".\\MissionsAutoSave", 0);

	string autosaveName;
	autosaveName.Format(".\\MissionsAutoSave\\autosave_%03d.xml", AutoSaveIndex);

	AutoSaveIndex++;
	if (AutoSaveIndex > MaxAutoSaveSlots) AutoSaveIndex = 0;


	gp->SaveBoxText.Format("Autosave to autosave_%03d.xml", AutoSaveIndex);
	SaveProcess(autosaveName.c_str(), true);

	/*
	ParticleManager* pManager = (ParticleManager*)pEditor->Manager();
	DWORD SystemCount = pManager->GetCreatedSystemCount();

	if (SystemCount > 0)
	{
	IParticleSystem* pSys = pManager->GetCreatedSystemByIndex(0);
	const char* pSysName = pSys->GetName();
	pManager->WriteSystemCacheAs(pSysName, autosaveName.c_str());
	}
	*/



}

void TPanelWindow::Autosave ()
{
	if (miss)
	{
		Vector v;
		v.x = miss->Controls().GetControlStateFloat ("GUICursor_MoveX");
		v.z = miss->Controls().GetControlStateFloat ("GUICursor_MoveY");
		v.y = 0.0f;

		float MouseDelta = v.GetLengthXZ();

		if (MouseDelta > 20.0f)
		{
			MouseNotMovedTime = 0.0f;
		}

	} else
	{

	}


	TimeFromLastAutoSave += api->GetDeltaTime();
	MouseNotMovedTime += api->GetDeltaTime();

	//Один раз в 3 минуты и уже 2 секунды не двигали мышку - сохраняемся
	if (TimeFromLastAutoSave < 180.0f || MouseNotMovedTime <= 2.0f) return;

	ForceAutosave();
}


void TPanelWindow::Draw ()
{
	

	Autosave();

	GUIRectangle rct = ToolsPanel->GetDrawRect();

/*
	if (!bBigPanel)
	{
		rct.Width = 241;
	} else*/
	{
		rct.Width = 241 + AdditionalWidth;
	}


	
	ToolsPanel->SetDrawRect(rct);


	if (pHandler)
	{
		pHandler->viewport = TPanel1->GetDrawRect();
	}

	if (pRTEditedAttribute && pRTEditedAttribute->NeedApplyCancelButtonInRT())
	{
		ApplyButton->Visible = true;
		CancelButton->Visible = true;
	} else
		{
			ApplyButton->Visible = false;
			CancelButton->Visible = false;
		}

	// Exit from fly mode...
/*		
	if ((miss->Controls().GetControlStateType("ExitFromFreeFly") == CST_ACTIVE) && (TimeFromLastPress > 0.1f))
	{
		if ((TPanel1->bActive) && (!bWasPressed))
		{
			TimeFromLastPress = 0.0f;
			GUIWindow::KeyPressed (VK_ESCAPE, true);
		}
	}
*/

	if (bWasPressed) bWasPressed = false;



	GUIControl::Draw();
}

void TPanelWindow::KeyPressed(int Key, bool bSysKey)
{

	GUIWindow::KeyPressed (Key, bSysKey);
	if (!IsActive) return;
	bool ControlState = false;
	if (GetAsyncKeyState(VK_CONTROL) < 0) 
	{
		ControlState = true;
	}

	

	if (Key == VK_DELETE && bSysKey && !MissionName->CursorInside)
	{
		DeleteSelectedPressed (this);
		Sleep(100);
	}

	if (Key == VK_INSERT && bSysKey)
	{
		InsertArrayItemPressed (this);
		Sleep(100);
	}
	

	if (Key == VK_F1 && bSysKey && fDeltaFromLastStart > 1.0f)
	{
		fDeltaFromLastStart = 0.0f;
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		RunGamePressed (this);
		Sleep(100);
		TimeFromLastPress = 0.0f;
	}


	if (Key == VK_F7 && bSysKey)
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		CreateFolderPressed (this);
		Sleep(300);
	}

	if (Key == VK_F6 && bSysKey)
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		RenameObjectPressed (this);
		Sleep(300);
	}


	if ((Key == 'O') && (ControlState))
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		LoadMissionPressed (this);
		Sleep(100);		
	}

	if (Key == 'W')
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		btnMoveObjects->OnPressed ();
	}

	if (Key == 'Q')
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		btnSelectObjects->OnPressed ();
	}

	if (Key == 'E')
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		btnRotateObjects->OnPressed ();
	}


	if ((Key == 'S') && (ControlState))
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		
		if (gp->loadedFileName.IsEmpty())
		{
			SaveMissionPressed (this);
			Sleep(100);
		} else
		{
			gp->SaveBoxText.GetFileName(gp->loadedFileName.c_str());
			SaveProcess(gp->loadedFileName.c_str(), false);
		}


	}

	if ((Key == 'E') && (ControlState))
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		ExportPressed (this);
		Sleep(300);
	}

	if ((Key == 'N') && (ControlState))
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		CreateMissionPressed (this);
		Sleep(300);
	}

	
	if ((Key == 'I') && (ControlState))
	{
		GUIWindow::KeyPressed (VK_ESCAPE, true);
		btnCreateObjectPressed (this);
		Sleep(300);
	}
	
	
}


void _cdecl TPanelWindow::RenameObjectPressed (GUIControl* sender)
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (!sNode) return;
	if ((sNode->Tag != TAG_OBJECT) && (sNode->Tag != TAG_FOLDER)) return;


	if (sNode->bReadOnly)
	{
		Application->MessageBox("Can't rename read only object", "Warning", GUIMB_OK);
		return;
	}



	
	NeedRename = true;

	TRenameWindow* cWind = NEW TRenameWindow (sNode->GetText());
	cWind->OnClose = (CONTROL_EVENT)&TPanelWindow::RenameObject;
	igui->ShowModal (cWind);

}


void _cdecl TPanelWindow::RenameObject (GUIControl* sender)
{
	if (!NeedRename) return;
	NeedRename = false;
	TRenameWindow* cWind = (TRenameWindow*)sender;
	if (!cWind->ExitByOK) return;
	
	//Max: на пустое имя нельзя переименовывать
	if (cWind->pText->Text.IsEmpty()) return;

	
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (!sNode) return;
	if (sNode->Tag == TAG_OBJECT)
	{
		MissionEditor::tCreatedMO* pMo = (MissionEditor::tCreatedMO*)sNode->Data;


		const char* newname = MakeUniqueName (cWind->pText->Text.GetBuffer());
		if (crt_stricmp(newname, pMo->pObject.Ptr()->GetObjectID().c_str()) == 0) return;
		MOPWriter wrt(pMo->Level, newname);
		pMo->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
		miss->EditorUpdateObject(pMo->pObject.Ptr(), wrt);
#endif
		/*
		
		MOSafePointer objToRename = pMo->pObject;
		if (cWind->pText->Text == objToRename->GetObjectID().c_str()) return;
		const char* newname = MakeUniqueName (cWind->pText->Text.GetBuffer());
		objToRename->EditMode_SetNewObjectID(newname);
		*/


		sNode->SetText(newname);

		RecalcWay (sNode);
		return;
	}

	if (sNode->Tag == TAG_FOLDER)
	{
		GUITreeNode* nparent = sNode->Parent;
		GUITreeNodes* nodes = NULL;

		if (nparent)
			nodes = &nparent->Childs;
		else
			nodes = MainWindow->TreeView1->Items;

		for (int n =0 ; n < nodes->GetCount(); n++)
		{
			if (crt_stricmp(nodes->Get(n)->GetText(), cWind->pText->Text.c_str()) == 0)
			{
				//Application->MessageBox("Folder with this name already exist", "Error", GUIMB_OK);
				return;
			}
		}

		sNode->SetText(cWind->pText->Text.GetBuffer());
		RecalcWay (sNode);
	}

	
}


void TPanelWindow::RecalcWay (GUITreeNode* object)
{
	// Если папка у всех детей пересчитываем путь
	if (object->Tag == TAG_FOLDER) 
	{
		for (int n = 0 ; n < object->Childs.GetCount (); n++)
		{
			RecalcWay (object->Childs.Get(n));
		}
	}

	if (object->Tag != TAG_OBJECT) return;
	MissionEditor::tCreatedMO* pObj = (MissionEditor::tCreatedMO*)object->Data;

	
	if (object->Parent)
	{
		const char* fullname = object->Parent->GetFullPath();
		pObj->PathInTree = fullname;
		//strncpy (pObj->PathInTree, fullname, MAXOBJECTPATH);
	} else
		{
			pObj->PathInTree = "";
		}
	
}




void _cdecl TPanelWindow::btnMoveObjectsPressed (GUIControl* sender)
{
	SelectControl->Activate(false);
	MoveControl->Activate(true);
	RotateControl->Activate(false);
}

void _cdecl TPanelWindow::btnRotateObjectsPressed (GUIControl* sender)
{
	MoveControl->Activate(false);
	RotateControl->Activate(true);	
	SelectControl->Activate(false);
}

void _cdecl TPanelWindow::btnSelectObjectsPressed (GUIControl* sender)
{
	MoveControl->Activate(false);
	RotateControl->Activate(false);	
	SelectControl->Activate(true);
}


void _cdecl TPanelWindow::CreateMissionPressed (GUIControl* sender)
{
	Application->MessageBox("Create new mission ?", "Warning !", GUIMB_OKCANCEL)->OnOK = (CONTROL_EVENT)&TPanelWindow::RealCreateMission;
}

void _cdecl TPanelWindow::CreateEmptyMission (GUIControl* sender)
{
	gp->loadedFileName = "";
	MissionName->Enabled = true;
	MissionName->Visible = true;
	MissionName->Text = "";
	LabelMissionName->Visible = false;

	// Убиваем миссию...
	pCurrentSelected.Reset();
	MainWindow->TreeView1->Items->Clear ();
	// Очищаем список и удаляем объекты из миссии...
	if(sMission)
	{
		for (DWORD z =0; z < sMission->GetCreatedMissionObjectsCount(); z++)
		{
			MissionEditor::tCreatedMO& stru = sMission->GetCreatedMissionObjectStructByIndex(z);
			delete (stru.pObject.Ptr()); stru.pObject.Reset();
			delete (stru.AttrList);
		}
		sMission->DeleteAllCreatedMissionObjects();
	}

	miss->DeleteMission();

	delete globalNodesPool;
	globalNodesPool = NULL;



	globalNodesPool = NEW TreeNodesPool;

	miss = (IMission *)api->CreateObject("Mission");
	miss->CreateObject(MOSafePointer(), "TPanelWindowHandler", ConstString("_editorObject_TPanelWindowHandler_"));

	pRS->SetBackgroundColor(Color (0xFFCECEE8L));


	//api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));
	miss->Controls().ExecuteCommand(InputSrvLockMouse(false));


}

	//
void _cdecl TPanelWindow::RealCreateMission (GUIControl* sender)
{

	CreateEmptyMission (this);
	MainWindow->CreateDefaultFolders (this);


	pFreeCamera->SetPosition(Vector (9.0f, 7.0f, -12.0f));
	pFreeCamera->SetTarget(Vector (0.0f, 0.0f, 0.0f));

}

void _cdecl TPanelWindow::ApplyChanges (GUIControl* sender)
{
	if (!pRTEditedAttribute) return;
	pRTEditedAttribute->ApplyRTEdit();
	pEditableNode =	pRTEditedAttribute;
	pRTEditedAttribute = NULL;
	MainWindow->UpdateTree(NULL);
	
}

void _cdecl TPanelWindow::CancelChanges (GUIControl* sender)
{
	if (!pRTEditedAttribute) return;
	pRTEditedAttribute->CancelRTEdit();
	pEditableNode =	pRTEditedAttribute;
	pRTEditedAttribute = NULL;
	MainWindow->UpdateTree(NULL);
	
}

void TPanelWindow::RecursiveDelete (GUITreeNode* sNode)
{
	if (sNode->Tag == TAG_OBJECT)
	{
		MissionEditor::tCreatedMO* pMo = (MissionEditor::tCreatedMO*)sNode->Data;
		if (pMo)
		{
			MOSafePointer objToDelete = pMo->pObject;

			// Удаляем из списка созданных объектов
			DWORD total_nodes = sMission->GetCreatedMissionObjectsCount();
			for (DWORD n = 0; n < total_nodes; n++)
			{
				MissionEditor::tCreatedMO* cObj = &sMission->GetCreatedMissionObjectStructByIndex(n);
				if (cObj->pObject == pMo->pObject)
				{
					// Убиваем аттрибуты объекта...
					delete (cObj->AttrList);
					sMission->DeleteCreatedMissionObject(n);
					break;
				}
			}
			delete objToDelete.Ptr();
		}

		//sNode->Release ();
		return;
	}

	if (sNode->Tag == TAG_FOLDER)
	{
		for (int n = 0; n < sNode->GetChildsCount(); n++)
		{
			RecursiveDelete	(sNode->Childs[n]);
		}
	}

}

void _cdecl TPanelWindow::btnSelectBackground (GUIControl* sender)
{
	GUIColorPicker* pGUIColor = NEW GUIColorPicker(0, 0, "test", 0, 0);
	pGUIColor->bAlwaysOnTop = true;
	pGUIColor->SetScreenCenter();

	pGUIColor->editA->Text = IntToStr ((int)(BackgroundColor.a * 255.0f));
	pGUIColor->editR->Text = IntToStr ((int)(BackgroundColor.r * 255.0f));
	pGUIColor->editG->Text = IntToStr ((int)(BackgroundColor.g * 255.0f));
	pGUIColor->editB->Text = IntToStr ((int)(BackgroundColor.b * 255.0f));
	pGUIColor->TextIsChanged (NULL);

	pGUIColor->OnApply = (CONTROL_EVENT)&TPanelWindow::ApplyBackgroundColor;


	igui->ShowModal(pGUIColor);
	//Application->ShowModal(pGUIColor);
}

void _cdecl TPanelWindow::ApplyBackgroundColor (GUIControl* sender)
{
	GUIColorPicker* pGUIColor = (GUIColorPicker*)sender;
	BackgroundColor = (dword)pGUIColor->SelectedColor;

	
	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IEditableIniFile* pEngineIni = pFS->OpenEditableIniFile(api->Storage().GetString("system.ini"), file_open_always, _FL_);
	if (pEngineIni)
	{
		pEngineIni->SetLong("mission_editor", "Background", BackgroundColor);
		pEngineIni->Release();
	}



}

/*

void _cdecl TPanelWindow::OpenFromVSS (GUIControl* sender)
{
	TVSSResourceBrowser* cResBrowserWind = NEW TVSSResourceBrowser (VSSUserName.c_str());
	igui->ShowModal (cResBrowserWind);



	int res = _access("\\\\sourceserver\\Sources\\Blood_src\\srcsafe.ini", 0);
	if (res == -1)
	{
		igui->MessageBox("You don't have access to SourceServer\nCheck you network or contact to administrator", "Error", GUIMB_OK, true);
		return;
	}


	TVSSLogin* cWind = NEW TVSSLogin ();
	cWind->OnClose = (CONTROL_EVENT)VSSLoginEntered;
	igui->ShowModal (cWind);

}




void _cdecl TPanelWindow::VSSLoginEntered (GUIControl* sender)
{
	TVSSLogin* cWind = (TVSSLogin*)sender;
	if (!cWind->ExitByOK) return;

	TVSSResourceBrowser* cResBrowserWind = NEW TVSSResourceBrowser (VSSUserName.c_str());
	//cWind->OnClose = (CONTROL_EVENT)VSSLoginEntered;
	igui->ShowModal (cResBrowserWind);

}



void TPanelWindow::SetThisMissionIsSourceControlFolder (bool bVal)
{
	//if (!bVSSAvailable) 
		bVal = false;

	bThisMissionIsSourceControlFolder = bVal;

	btnAddToVSS->bEnabled = bVal; 
	//btnLoadMissionFromVSS->bEnabled = bVal; 
	btnCheckOut->bEnabled = bVal; 
	btnCheckIn->bEnabled = bVal; 
	btnUndoCheckOut->bEnabled = bVal; 
	btnGetLatestVersion->bEnabled = bVal; 

}





void TPanelWindow::VSSAfterLoad(const char* szMissionName)
{
	LoadedMissionName = szMissionName;
	LoadedMissionDir.GetFilePath(LoadedMissionName);

	pCurrentVSSItem = NULL;


	if (bVSSAvailable)
	{
		string tmp_mission_name = szMissionName;
		string mission_file_name;
		mission_file_name.GetFileName(tmp_mission_name);

		SourceSafeItem& root = VSSRoot.Childs[0];
		int idx = root.GetChildIndex(mission_file_name);

		//Файла нету в VSS
		if (idx == -1)
		{
			btnAddToVSS->bEnabled = true; 
			btnCheckOut->bEnabled = false; 
			btnCheckIn->bEnabled = false; 
			btnUndoCheckOut->bEnabled = false; 
			btnGetLatestVersion->bEnabled = false; 
		} else
		{
			
			SourceSafeItem& file = root.Childs[idx];

			pCurrentVSSItem = &file;

			btnAddToVSS->bEnabled = false; 
			//Есть и его кто то забрал...
			if (file.bCheckedOut)
			{
				string WhoIsTheAss = file.WhoCheckedOut();

				if (WhoIsTheAss == VSSUserName)
				{
					//Мы забрали
					btnCheckOut->bEnabled = false; 
					btnCheckIn->bEnabled = true; 
					btnUndoCheckOut->bEnabled = true; 
					btnGetLatestVersion->bEnabled = false; 
				} else
				{
					//Какая то сволочь забрала...
					btnCheckOut->bEnabled = false; 
					btnCheckIn->bEnabled = false; 
					btnUndoCheckOut->bEnabled = false; 
					btnGetLatestVersion->bEnabled = true; 
				}


				//это забрали мы...
			} else
			{
				btnCheckOut->bEnabled = true; 
				btnCheckIn->bEnabled = false; 
				btnUndoCheckOut->bEnabled = false; 
				btnGetLatestVersion->bEnabled = true; 
			}
		}

	}

}


void _cdecl TPanelWindow::VSS_Add_pressed (GUIControl* sender)
{
//	igui->MessageBox("ADD", "error...", GUIMB_OK);

	SaveProcess(LoadedMissionName);
	VSSRoot.Childs[0].AddToProject(LoadedMissionName);

	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());

	VSSAfterLoad(LoadedMissionName);

}

void _cdecl TPanelWindow::VSS_CheckIn_pressed (GUIControl* sender)
{
	if (!pCurrentVSSItem) return;

	SaveProcess(LoadedMissionName);
	pCurrentVSSItem->Checkin(LoadedMissionDir);

	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());

	VSSAfterLoad(LoadedMissionName);

}

void _cdecl TPanelWindow::VSS_CheckOut_pressed (GUIControl* sender)
{
	//igui->MessageBox("CHECK OUT", "error...", GUIMB_OK);

	if (!pCurrentVSSItem) return;


	string ch_bak = LoadedMissionName;
	ch_bak.AddExtention(".co_bak");
	DeleteFile(ch_bak);
	MoveFile(LoadedMissionName, ch_bak);


	pCurrentVSSItem->Checkout(LoadedMissionDir);

	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());

	VSSAfterLoad(LoadedMissionName);

}

void _cdecl TPanelWindow::VSS_UndoCheckOut_pressed (GUIControl* sender)
{
	if (!pCurrentVSSItem) return;
	if (!pCurrentVSSItem->bCheckedOut) return;

	GUIMessageBox* pMB = igui->MessageBox("Undo checkout and lose all changes ?", "WARNING !", GUIMB_YESNO);

	pMB->OnOK = (CONTROL_EVENT)&TPanelWindow::VSS_UndoCheckOut;

}

void _cdecl TPanelWindow::VSS_GetLatest_pressed (GUIControl* sender)
{
	//igui->MessageBox("GET LATEST", "error...", GUIMB_OK);
	if (!pCurrentVSSItem) return;


	string ch_bak = LoadedMissionName;
	ch_bak.AddExtention(".late_bak");
	DeleteFile(ch_bak);
	MoveFile(LoadedMissionName, ch_bak);

	pCurrentVSSItem->GetLatestVersion(LoadedMissionDir);

	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());

	StartLoad(LoadedMissionName);

}


void _cdecl TPanelWindow::VSS_UndoCheckOut (GUIControl* sender)
{
	if (!pCurrentVSSItem) return;
	pCurrentVSSItem->UndoCheckout(LoadedMissionDir);

	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());

	StartLoad(LoadedMissionName);
}





*/

void _cdecl TPanelWindow::ChangeAdditionalDraw (GUIControl* sender)
{
	 bool bValue = miss->EditMode_IsAdditionalDraw();

	 miss->EditMode_AdditionalDraw(!bValue);
}



void _cdecl TPanelWindow::btnImportMissionsListAction (GUIControl* sender)
{
	//Тут показать окно проимпортированных миссий....
	TImportedMistEditor* cWind = NEW TImportedMistEditor (100, 100);
	igui->ShowModal (cWind);

}