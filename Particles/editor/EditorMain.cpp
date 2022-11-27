#ifndef _XBOX

#include "EditorMain.h"
#include "Forms\mainwindow.h"
#include "Forms\projectbrowser.h"
#include "..\..\common_h\gui_controls.h" 
#include "..\..\common_h\particles.h"
#include "..\manager\particlemanager.h"
#include "..\gizmo\gizmo.h"

#include "..\TextFile.h"


#include "..\..\Common_h\InputSrvCmds.h"
#include "..\..\Common_h\corecmds.h"


string PakName;

#ifndef NO_TOOLS
CREATE_CLASS(ParticleEditor)
#endif

IGUIManager* Application = NULL;

ParticleEditor* pEditor = NULL;

GUIGraphLine*	CopyGraphLineMax = NULL;
GUIGraphLine*	CopyGraphLineMin = NULL;
bool CopyBooleanRelative = false;


ParticleEditor::ParticleEditor()
{
	pCtrl = NULL;
	pParticleManager = NULL;
	pFS = NULL;
	pParticleManager = NULL;
//	pSys = NULL;
	GridColor = 0xFF626262;
	BackgroundColor = 0xFF878787;
	GridZeroLinesColor = 0xFF000000;
	pPS = NULL;
	Application = NULL;
	FormMain = NULL;
	pParticleManager = NULL;
	Gizmo = NULL;
}


ParticleEditor::~ParticleEditor()
{
	delete CopyGraphLineMax;
	delete CopyGraphLineMin;

	if (pParticleManager)	pParticleManager->Release();
	pParticleManager = NULL;

	if (Gizmo)
	{
		delete Gizmo;
	}

	Gizmo = NULL;

	RELEASE(pCtrl);

}

TransformGizmo* ParticleEditor::GetGizmo ()
{
	return Gizmo;
}

bool ParticleEditor::Init()
{
	

	CopyGraphLineMax = NEW GUIGraphLine (Vector (0.0f, 1.0f, 0.0f), Vector(99999.0f, 1.0f, 0.0f));
	CopyGraphLineMin = NEW GUIGraphLine (Vector (0.0f), Vector(99999.0f, 0.0f, 0.0f));

	IControlsService * ctrlsrv = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrlsrv);
	pCtrl = ctrlsrv->CreateInstance(_FL_);
	


	//pCtrl->LockDebugKeys(true);
	api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(false));
	if (pCtrl) pCtrl->ExecuteCommand(InputSrvLockMouse(false));
	

	//pCtrl->LockMouseCursorPos(false);
	
	pFS = (IFileService*)api->GetService("FileService");
	Assert (pFS != NULL);

	PakName = PARTICLES_PLACE;

	pRS = (IRender*)api->GetService("DX9Render");
	Assert (pRS);
	pRS->SetBackgroundColor(Color(0x50505050L));

	//Matrix m(Vector (13, 0, 0), Vector (0, 2, 0));
	Matrix m;
	Gizmo = NEW TransformGizmo (TransformGizmo::GT_MOVE, TransformGizmo::GM_LOCAL, m);
	Gizmo->Enable(false);


	pPS = (IParticleService*)api->GetService("ParticleService");
	Assert (pPS);
	//pParticleManager = pPS->CreateManagerEx(NULL, _FL_);

	pParticleManager = (ParticleManager*)pPS->CreateManagerEx(NULL, _FL_);
	Assert (pParticleManager);

	//pParticleManager

	pEditor = this;
	if (pCtrl) pCtrl->EnableControlGroup ("GUI", true);

	
	Application = (IGUIManager *)api->CreateObject("GUIManager");
	Assert(Application);

	StartDirectory = Application->GetCurrentDir();


	
	char EmptyFile[12];
	memset (EmptyFile, 0, 12);
	crt_strcpy (EmptyFile, 9, "PSYSv3.5");
	pEditor->Files()->SaveData(".\\resource\\particles\\_empty.xps", EmptyFile, 12);


	TextFile* xmlFile = NEW TextFile(".\\resource\\particles\\_empty.xml");
	xmlFile->Write(0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	xmlFile->Write(0, "<Particles>\n");
	xmlFile->Write(0, "</Particles>\n");
	delete xmlFile;
	xmlFile = NULL;


//	pEditor->Files()->RescanAllPaks();

	IParticleSystem* pLoadedSys = pParticleManager->CreateParticleSystemEx("_empty", _FL_);


	FormMain = NEW TMainWindow ();
	Application->Show(FormMain);


/*
	TProjectBrowser* FirstSelector = NEW TProjectBrowser;
	Application->ShowModal(FirstSelector);
	FirstSelector->OnClose = (CONTROL_EVENT)OnCloseManager;
*/
	
	return true;
}

void ParticleEditor::Realize(dword Delta_Time)
{
}

void ParticleEditor::Execute(dword Delta_Time)
{
}

IRender* ParticleEditor::Render()
{
	return pRS;
}

IParticleManager* ParticleEditor::Manager ()
{
	return (IParticleManager*)pParticleManager;
}

DWORD ParticleEditor::GetBackgroundColor ()
{
	return BackgroundColor;
}

void ParticleEditor::SetBackgroundColor (DWORD Color)
{
	BackgroundColor = Color;
}

DWORD ParticleEditor::GetGridColor ()
{
	return GridColor;
}

void ParticleEditor::SetGridColor (DWORD Color)
{
	GridColor = Color;
}

DWORD ParticleEditor::GetGridZeroColor ()
{
	return GridZeroLinesColor;
}

void ParticleEditor::SetGridZeroColor (DWORD Color)
{
	GridZeroLinesColor = Color;
}

STORM3_ArcBall* ParticleEditor::ArcBall ()
{
	return &ST3ArcBall;
}

const char* ParticleEditor::GetStartDir ()
{
	return StartDirectory.GetBuffer ();
}

void _cdecl ParticleEditor::OnCloseManager (GUIControl* sender)
{
	TProjectBrowser* FirstSelector = (TProjectBrowser*)sender;
	if (!FirstSelector->UserSelectSystemToOpen)
	{
		api->Exit();
		return;
	}

	//pParticleManager->OpenProject(FirstSelector->CurrentProjectNameShort.GetBuffer());

	IParticleSystem* pSys = (IParticleSystem*)GetSystem ();
	if (pSys)
	{
		pSys->Release ();
		pSys = NULL;
	}
	
	const char* szFileName = FirstSelector->CurrentSystemName.GetBuffer();

	pParticleManager->LoadAllSystems();

	IParticleSystem* pLoadedSys = pParticleManager->CreateParticleSystemEx(szFileName, _FL_);



	FormMain = NEW TMainWindow ();
	Application->Show(FormMain);
}

IParticleSystem* ParticleEditor::GetSystem ()
{
	DWORD SystemCount = pParticleManager->GetCreatedSystemCount();
	if (SystemCount == 0)
	{
		//Application->MessageBox("No particle system available !!!", "Error", GUIMB_OK, true);
		return NULL;
	}


	if (SystemCount > 1)
	{
		throw ("To many systems detected !!!");
		//Application->MessageBox("To many systems detected !!!", "Developer message", GUIMB_OK, true);
		return NULL;
	}

	ParticleSystemProxy* pSystem = pParticleManager->GetCreatedSystemByIndex(0);
	//Assert (pSystem);

	return pSystem;

}


IFileService* ParticleEditor::Files ()
{
	return pFS;
}

#endif