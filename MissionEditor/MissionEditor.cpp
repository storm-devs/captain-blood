#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>



#include "missioneditor.h"
#include "..\common_h\gui.h"
#include "..\common_h\FreeCamera.h"
#include "Attributes\BaseAttr.h"
#include "attributes\attributes.h"
#include "..\common_h\mission.h"
#include "..\common_h\gui_controls.h" 
#include "forms\mainwindow.h"
#include "forms\panel.h"
#include "forms\VSSLogin.h"
#include "attributes\attributes.h"
#include "movecontroller.h"
#include "rotatecontroller.h"
#include "selector.h"
#include "forms/globalParams.h"

#include "..\Common_h\corecmds.h"
#include "..\Common_h\InputSrvCmds.h"

//#include "vtune\vtuneapi.h"

//#include "SourceSafe/VSSHelper.h"

INTERFACE_FUNCTION
#ifndef NO_TOOLS
CREATE_CLASS(MissionEditor)
#endif

globalParams * gp = NULL;


extern IMission* playing_miss;              // Миссия в которую играем
extern MOSafePointer pCurrentSelected;		// Текущий выбранный объект
long pCurrentSelectedCache = -1;			// Кэшь значение для быстрой проверки текущего выбранного объекта

IConsole* pConsole = NULL;
IGUIManager* igui = NULL;  // GUI мэнеджер
IMission* miss = NULL; //Миссия для редактора
IRender * pRS = NULL; // Рендер
TPanelWindow* PanelWindow = NULL; // Главное окошко с панелью
TMainWindow* MainWindow = NULL;   // Субокно с тривиев
MissionEditor* sMission = NULL;   // Сам редактор
int EditorMode = true;           // Режим редактирования или игры
IFreeCamera* pFreeCamera = NULL;


TreeNodesPool* globalNodesPool = NULL;

extern float fDeltaFromLastStart;

extern void FreeAllAttributesPools();

MoveController* MoveControl = NULL; // Контроллер для движения объектов
RotateController* RotateControl = NULL; // Контроллер для вращения объектов
ObjectSelector* SelectControl = NULL; // Контроллер для выбора объектов 


DWORD dwRealizeLevel = 0x1001;

//extern string VSSUserName;


//bool bVSSAvailable = false;
//SourceSafeItem VSSRoot;


bool bThisMissionIsSourceControlFolder = false;



void _cdecl EditorFindObject(const ConsoleStack& stack)
{
	const char* szObjectName = stack.GetParam(0);
	if (!szObjectName)
	{
		pConsole->Trace(COL_CMD_OUTPUT, "Задайте имя объекта для поиска!");
		return;
	}

	DWORD total_objects = sMission->GetCreatedMissionObjectsCount();

	for (DWORD i = 0; i < total_objects; i++)
	{
		MissionEditor::tCreatedMO* cObject = &sMission->GetCreatedMissionObjectStructByIndex(i);

		if (crt_stricmp(cObject->pObject.Ptr()->GetObjectID().c_str(), szObjectName) == 0)
		{
			pConsole->Trace(COL_CMD_OUTPUT, "Объект '%s' находится '%s'", szObjectName, cObject->PathInTree);
			return;
		}
	}


}

bool MissionEditor::Init()
{
	gp = NEW globalParams();
	


	globalNodesPool = NEW TreeNodesPool;

//	CSSConnection	m_vssConnection;
	//SourceSafeItem root;
	//bool bDatabase = VSSAcess::VSSEnumDatabase(root);

	MoveControl = NEW MoveController ();
	RotateControl = NEW RotateController (); 
	SelectControl = NEW ObjectSelector ();
	SelectControl->Activate(true);
	EntitysMO.DelAll ();
	AvailableMO.DelAll ();
	CreatedMO.DelAll ();



	pConsole = (IConsole*)api->GetService("Console");
	pConsole->Register_PureC_Command("EditorFind", "Ищет объект по имени в редакторе\nВозвращает путь в дереве до объекта\nВ параметрах имя объекта", EditorFindObject);


  pRS = (IRender*)api->GetService("DX9Render");
  pRS->SetGlobalLight(Vector(0.0f, 5000.0f, 0.0f), false, Color(3.8f), 0.8f);
	pRS->SetBackgroundColor(Color (0xFFCECEE8L));
	


// Создаем камеру...
	pFreeCamera = (IFreeCamera*)api->CreateObject("FreeCamera");

	pFreeCamera->SetPosition(Vector (9.0f, 7.0f, -12.0f));
	pFreeCamera->SetTarget(Vector (0.0f, 0.0f, 0.0f));


//Создаем миссию
	miss = (IMission *)api->CreateObject("Mission");;
	if(!miss) return false;

	

	//lll
	//miss->Controls().
	//miss->Controls().LockMouseCursorPos(false);


	api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(false));
	miss->Controls().ExecuteCommand(InputSrvLockMouse(false));


// Создаем менеджер GUI...
	igui = (IGUIManager *)api->CreateObject("GUIManager");;
	if (!igui) return false;


	//Создаем директорию где все миссии лежат...
	static char sysStartDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, sysStartDirectory);

	//crt_strcpy (sysStartDirectory, 200, "c:\\blood\\engine\\");


	gp->MissionsSrcLocalFolder = sysStartDirectory;
	gp->MissionsSrcLocalFolder += "\\MissionsSRC";

	


	BOOL bFldrRes = CreateDirectory(gp->MissionsSrcLocalFolder, 0);


	gp->StartDirectory = igui->GetCurrentDir();
	

	igui->PrecacheImages("");
	igui->PrecacheImages("meditor");

// Создаем окошки
	PanelWindow = NEW TPanelWindow;
	igui->Show (PanelWindow);

/*
	int res = _access("\\\\sourceserver\\Sources\\Blood_src\\srcsafe.ini", 0);
	if (res != -1)
	{
		bool bConnetedWithIniUserName = false;
		string INI_VSS_Login = "";

		IFileService* pFS = (IFileService*)api->GetService("FileService");
		IIniFile* pEngineIni = pFS->OpenIniFile(api->GetVarString("Ini"), _FL_);
		if (pEngineIni)
		{
			const char* szTempIniLogin = pEngineIni->GetString("VSS", "Login", NULL);
			INI_VSS_Login = szTempIniLogin;
			const char* szMachneName = pEngineIni->GetString("VSS", "MachineName", NULL);

			


			static char CurrentMachineName[8192];
			DWORD dwMachineBufferSize = 8000;
			HRESULT hr = GetComputerName(CurrentMachineName, &dwMachineBufferSize);

			

			if (!INI_VSS_Login.IsEmpty() && szMachneName)
			{
				//Если с той же машины входят...
				if (crt_stricmp (CurrentMachineName, szMachneName) == 0)
				{
					VSSUserName = INI_VSS_Login;

					bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());
					if (bDatabase == false)
					{
						//igui->MessageBox("Can't open VSS database\nCheck you name !", "Error", GUIMB_OK, true);
						SetVSSAvailable(false);
					} else
					{
						Assert (VSSRoot.Childs.Size() > 0);
						SetVSSAvailable(true);
						bConnetedWithIniUserName = true;
					}
				}
			}

			pEngineIni->Release();
		}
		

		if (!bConnetedWithIniUserName)
		{
			VSSUserName = "VSS_User_name";
			if (!INI_VSS_Login.IsEmpty()) VSSUserName = INI_VSS_Login;
			TVSSLogin* cWind = NEW TVSSLogin ();
			cWind->OnClose = (CONTROL_EVENT)&MissionEditor::VSSLoginEntered;
			igui->ShowModal (cWind);
		}

	} else
	{
		api->Trace("Netword path not available !!!!\n VSS disabled !!!");
		SetVSSAvailable(false);
	}

*/




//Включаем GUI контролы и отключаем MISSION
	miss->Controls().EnableControlGroup ("CDBuilder", true);
	miss->Controls().EnableControlGroup ("GUIADD", true);
	miss->Controls().EnableControlGroup ("GUI", true);
	miss->Controls().EnableControlGroup ("mission", false);
	miss->Controls().EnableControlGroup ("FreeCamera", false);

	//miss->Controls().LockMouseCursorPos(false);

	miss->Controls().ExecuteCommand(InputSrvLockMouse(false));
	

	CreateEntitysMOList ();
	CreateAvailableMOList ();

	sMission = this;


	api->SetObjectExecution(this, "missioneditor", 0x10001, &MissionEditor::Execute);




	//==================
	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IIniFile* pEngineIni = pFS->SystemIni();
	if (pEngineIni)
	{
		bool bNeedConvert = (pEngineIni->GetLong("XMLConvertor", "Convert", 0) != 0);

		if (bNeedConvert)
		{
			array<string> xml2Convert(_FL_);
			pEngineIni->GetStrings("XMLConvertor", "XML", xml2Convert);

			for (dword n = 0; n < xml2Convert.Size(); n++)
			{
				api->Trace("Convert %d of %d - '%s'\n", n, xml2Convert.Size(), xml2Convert[n].c_str());
				printf ("Convert %d of %d - '%s'\n", n, xml2Convert.Size(), xml2Convert[n].c_str());
				ConvertXML2MIS (xml2Convert[n].c_str());

			}

			xml2Convert.DelAll();
			pEngineIni->Release();
			//api->Exit();
			exit(0);
		} else
		{
			pEngineIni->Release();
		}
	}
	
	IRender * render = (IRender *)api->GetService("DX9Render");
	Assert(render);
	render->EnableLoadingScreen(false);



  return true;
}

MissionEditor::MissionEditor() : EntitysMO (_FL_, 2048),
                                 AvailableMO(_FL_, 2048),
																 CreatedMO(_FL_, 40000),
																 CreatedMOReal (_FL_, 2048),
																 importedMissions(_FL_, 128)
{
}

array<string> & MissionEditor::GetImportList()
{
	return importedMissions;
}

MissionEditor::~MissionEditor()
{
 
 MainWindow->TreeView1->Items->Clear ();


 delete SelectControl;
 delete RotateControl;
 delete MoveControl;
 DWORD n = 0;


 for (n =0; n < AvailableMO.Size(); n++)
 {
   delete (AvailableMO[n].AttrList);
 }
 
 AvailableMO.DelAll ();


 for (n =0; n < CreatedMO.Size(); n++)
 {
	 if (CreatedMO[n].bDeleted) continue;
	 if (CreatedMO[n].AttrList)  delete CreatedMO[n].AttrList;
 }
 
 CreatedMO.DelAll ();
 EntitysMO.DelAll ();

 //
 FreeAllAttributesPools();

 delete globalNodesPool;
 globalNodesPool = NULL;

 delete gp;
 gp = NULL;


 delete igui;
 igui = NULL;

 sMission = NULL;
 
}


// Если в режиме игры тут проверяем нажатие F1 и 
// переключаемся в редактор...
void _fastcall MissionEditor::Execute(float dltTime)
{
	fDeltaFromLastStart += dltTime;
	if (EditorMode == false)
	{
		//pRS->PrintBuffered((float)(pRS->GetScreenInfo3D().dwWidth - 300), 0, 0xFFFFFFFF, "GAME MODE, Press 'F1' to return EDITOR MODE");
	}
	if (GetAsyncKeyState (VK_F1) < 0)
	{
//		CMPause();

		// Переключиться в режим редактора...
		if (EditorMode == false)
		{
			pFreeCamera->Pause (false);
			fDeltaFromLastStart = 0.0f;
			pRS->SetBackgroundColor(Color (0xFFC5C5C5L));


			miss->Controls().ClearKeyBuffer();


			Sleep (300);
			api->Trace("Delete mission !!!");

			array<Object*> ObjList(_FL_);
			api->FindObject("Mission", ObjList);

			for (DWORD i = 0; i < ObjList.Size(); i++)
			{
				IMission* tMission = (IMission*)ObjList[i];
				if (tMission != miss)
				{
					tMission->DeleteMission();
				}

			}
			
		
			playing_miss = NULL;
			EditorMode = true;

#ifndef NO_TOOLS
			miss->EditorSetSleep(false);
#endif
			//miss->Controls().LockMouseCursorPos(false);
			miss->Controls().ExecuteCommand(InputSrvLockMouse(false));


			igui->Enable(true);



			
//Удалить временную директорию и файл миссии
			string name = "TempRunGame";
			string path = "Resource\\Mission\\";
			path += name;
			string file = path + ".mis";

			IFileService* pFS = (IFileService*)api->GetService("FileService");
			pFS->Delete(file);

			miss->Controls().EnableControlGroup ("CDBuilder", true);
			miss->Controls().EnableControlGroup ("GUIADD", true);
			miss->Controls().EnableControlGroup ("GUI", true);
			miss->Controls().EnableControlGroup ("mission", false);
			miss->Controls().EnableControlGroup ("FreeCamera", false);
			//miss->Controls().LockDebugKeys(true);
			api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));

			miss->Controls().ClearKeyBuffer();

		}
	}
}



//создать список Ентитей являющихся объектами миссии...
void MissionEditor::CreateEntitysMOList ()
{
	array<string> EngineObjects(_FL_);
	api->GetRegistryObjectsList(EngineObjects);
	for (dword i = 0; i < EngineObjects.Size(); i++)
	{
		if (strstr(EngineObjects[i].c_str(), MOP_ID) != NULL)
		{
			tEntitysMO* pNew = &EntitysMO[EntitysMO.Add ()];
			//strncpy (pNew->Name, EngineObjects[i].c_str(), MAX_ENTITY_NAME);
			pNew->Name = EngineObjects[i].c_str();
		}
  }
}

// Создать список доступных объектов миссии
void MissionEditor::CreateAvailableMOList ()
{
	for (DWORD n  = 0; n < EntitysMO.Size (); n++)
	{
		string e_id = EntitysMO[n].Name;
		IMOParams* Params = (IMOParams* )api->CreateObject(e_id.GetBuffer ()); 
		if (string (Params->GetName()).IsEmpty()) continue;


		


		tAvailableMO* pNewEntry = &AvailableMO[AvailableMO.Add ()];
		const char* szComment = Params->GetComment();

		if (szComment)
		{
			pNewEntry->Comment = szComment;
		} else
		{
			pNewEntry->Comment = "No comment available !";
		}
		
		pNewEntry->Params = Params;
		crt_strncpy (pNewEntry->Name, MAX_ENTITY_NAME-1, pNewEntry->Params->GetName(), MAX_ENTITY_NAME);
		crt_strncpy (pNewEntry->ClassName, MAX_ENTITY_NAME-1, pNewEntry->Params->GetEntity(), MAX_ENTITY_NAME);
	  pNewEntry->AttrList = NEW AttributeList;

		//api->Trace("OBJECT : '%s'", e_id.c_str());
		
	  pNewEntry->AttrList->CreateFromParams (pNewEntry->Params);
	}
}

const char* MissionEditor::GetCommentForName (const char* szTextName)
{
	for (DWORD i = 0; i < AvailableMO.Size(); i++)
	{
		if (crt_stricmp (AvailableMO[i].Name, szTextName) == 0)
		{
			return AvailableMO[i].Comment.c_str();
		}
	}


	return "Name not found !!!";

}

const char* MissionEditor::GetCommentForClassName (const char* szClassName)
{

	for (DWORD i = 0; i < AvailableMO.Size(); i++)
	{
		if (crt_stricmp (AvailableMO[i].ClassName, szClassName) == 0)
		{
			return AvailableMO[i].Comment.c_str();
		}
	}


	return "Class not found !!!";
}

MissionEditor::tCreatedMO* MissionEditor::GetObjectByTreePath (const char* szTreePath)
{
	for (DWORD i  = 0;  i < CreatedMO.Size(); i++)
	{
		if (crt_stricmp(szTreePath, CreatedMO[i].PathInTree) == 0) return &CreatedMO[i];
	}

	return NULL;
}

MissionEditor::tAvailableMO* MissionEditor::GetAvailableClassByName (const char* szClassName)
{
	for (DWORD i  = 0;  i < AvailableMO.Size(); i++)
	{
		if (crt_stricmp(szClassName, AvailableMO[i].ClassName) == 0) return &AvailableMO[i];
	}

	return NULL;
	
}

void _cdecl MissionEditor::VSSLoginEntered (GUIControl* sender)
{
	TVSSLogin* cWind = (TVSSLogin*)sender;
	if (!cWind->ExitByOK)
	{
		SetVSSAvailable(false);
		return;
	}


/*	bool bDatabase = VSSAcess::VSSEnumDatabase(VSSRoot, VSSUserName.c_str());


	if (bDatabase == false)
	{
		igui->MessageBox("Can't open VSS database\nCheck you name !", "Error", GUIMB_OK, true);
		SetVSSAvailable(false);
		return;
	}

	Assert (VSSRoot.Childs.Size() > 0);

	SetVSSAvailable(true);


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IIniFile* pEngineIni = pFS->OpenIniFile(api->GetVarString("Ini"), _FL_);
	if (pEngineIni)
	{
		pEngineIni->SetString("VSS", "Login", VSSUserName.c_str());


		static char CurrentMachineNameToSave[8192];
		DWORD dwMachineBufferSize = 8000;
		HRESULT hr = GetComputerName(CurrentMachineNameToSave, &dwMachineBufferSize);


		pEngineIni->SetString("VSS", "MachineName", CurrentMachineNameToSave);

		pEngineIni->Release();
	}
	*/
}

void MissionEditor::SetVSSAvailable (bool bEnabled)
{
	/*
	if (bEnabled)
	{
		bVSSAvailable = true;
	} else
	{
		bVSSAvailable = false;
	}

	PanelWindow->btnLoadMissionFromVSS->bEnabled = bVSSAvailable;
	*/
	//PanelWindow->btnLoadMissionFromVSS->bEnabled = false;

/*
	PanelWindow->btnAddToVSS->bEnabled = false;
	PanelWindow->btnCheckOut->bEnabled = false;
	PanelWindow->btnCheckIn->bEnabled = false;
	PanelWindow->btnUndoCheckOut->bEnabled = false;
	PanelWindow->btnGetLatestVersion->bEnabled = false;
*/
	

}



long MissionEditor::GetCreatedMissionObjectIndex (MOSafePointer obj)
{
	if(!obj.Validate()) return -1;
	for (dword n = 0; n < CreatedMOReal.Size(); n++)
	{
		if (CreatedMOReal[n]->pObject.Ptr() == obj.Ptr())
		{
			return n;
		}
	}

	return -1;
/*
	DWORD total_nodes = CreatedMO.Size();
	for (DWORD n = 0; n < total_nodes; n++)
	{
		MissionEditor::tCreatedMO* cObj = &CreatedMO[n];
		if (cObj->pObject == obj)
		{
			return n;
		}
	}

	return -1;
*/
}


dword MissionEditor::GetCreatedMissionObjectsCount ()
{
	return CreatedMOReal.Size();
	//return CreatedMO.Size();
}

MOSafePointer MissionEditor::GetCreatedMissionObjectByIndex (dword dwIndex)
{
	return CreatedMOReal[dwIndex]->pObject;

	//return CreatedMO[dwIndex].pObject;
}

MissionEditor::tCreatedMO& MissionEditor::GetCreatedMissionObjectStructByIndex (dword dwIndex)
{
	return *CreatedMOReal[dwIndex];
	//return CreatedMO[dwIndex];
}

MissionEditor::tCreatedMO& MissionEditor::AddCreatedMissionObjectStruct()
{
/*
	for (dword n = 0; n < CreatedMO.Size(); n++)
	{
		if (CreatedMO[n].bDeleted)
		{

			CreatedMOReal.Add(&CreatedMO[n]);

			return CreatedMO[n];
		}
	}
*/

	//Если больше произойдет переаллокация в массиве и все поинтеры станут невалидные
	Assert (CreatedMO.Size() < 39999);

	MissionEditor::tCreatedMO& ref = CreatedMO[CreatedMO.Add()];
	CreatedMOReal.Add(&ref);
	return ref;
}

void MissionEditor::DeleteCreatedMissionObject(dword dwIndex)
{
	tCreatedMO* p = CreatedMOReal[dwIndex];

	for (dword n = 0; n < CreatedMO.Size(); n++)
	{
		if (&CreatedMO[n] == p)
		{
			CreatedMO[n].bDeleted = true;
			CreatedMOReal.DelIndex(dwIndex);
		}
	}
}

void MissionEditor::DeleteAllCreatedMissionObjects()
{
	CreatedMO.DelAll();
	CreatedMOReal.DelAll();
}


void MissionEditor::ConvertXML2MIS (const char* xmlName)
{
	api->Trace("Convert XML '%s' to MIS file", xmlName);

	PanelWindow->StartLoad(xmlName);
	PanelWindow->Export(NULL);

	api->Trace("Mission converted !!!");
	
}