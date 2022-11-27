#include "fast_atof.h"
#include "load.h"
#include "forms\mainwindow.h"
#include "attributes\attrcreator.h"
#include "forms\panel.h"
#include "forms\globalParams.h"
#include "..\common_h\freecamera.h"
#include "..\common_h\corecmds.h"

//#define LOAD_DEBUG

#ifdef LOAD_DEBUG
	#include "CAPROFAPI.h"
	#pragma comment(lib, "CAProfAPI32.lib")
#endif



extern IRender * pRS;

extern IMission* miss;
extern TMainWindow* MainWindow;
extern MOSafePointer pCurrentSelected;
extern TPanelWindow* PanelWindow;

extern MissionEditor* sMission;


extern char* MakeUniqueName (const char* Name);


extern IFreeCamera* pFreeCamera;


extern TreeNodesPool* globalNodesPool;


FILE * duplicate_file = NULL;


void DuplicateOut(const char * str,...)
{
	static char tempTraceBuffer[16384];

	va_list args;
	va_start(args, str);
	crt_vsnprintf(tempTraceBuffer, sizeof(tempTraceBuffer), str, args);
	va_end(args);

	if (duplicate_file == NULL)
	{
		duplicate_file = crt_fopen("import_duplicates.txt", "w+");
	}
	 
	if (duplicate_file)
	{
		fprintf(duplicate_file, "%s", tempTraceBuffer);
	}


	PanelWindow->SaveOKTime = 0.8f;
	gp->SaveBoxText = "See 'import_duplicates.txt'";

}




MOSafePointer FindInMission (const char* name)
{
	MOSafePointer mo;
	miss->FindObject(ConstString(name), mo);
	return mo;
/*	
	MGIterator& iterator = miss->GroupIterator (MG_OBJECTS, _FL_);
	for (;!iterator.IsDone();iterator.Next())
	{
		MissionObject* mo = iterator.Get ();
		const char* mName = mo->GetObjectID ();
		if (crt_stricmp (mName, name) == 0)
		{
			iterator.Release ();
			return mo;
		}
	}
	iterator.Release ();
	return NULL;
*/	

}


MissionLoad::MissionLoad (MissionEditor* _editor) : obj (_FL_),
                                                    JLLoadedNode(_FL_)
{
	editor = _editor;
}

MissionLoad::~MissionLoad ()
{
}

//void MissionLoad::Load ()

void MissionLoad::LoadMSR (const char* filename, bool SetCameraPos)
{
	IRender* pRS = (IRender*)api->GetService ("DX9Render");
	pRS->Print(100, 100, 0xFFFFFFFF, "LOADING.....");

	Loading (filename, SetCameraPos);
}

void MissionLoad::Loading (const char* filename, bool SetCameraPos)
{

	obj.DelAll();

	IFileService* pFS = (IFileService*)api->GetService("FileService");

	IFile* pFile = pFS->OpenFile(filename, file_open_existing, _FL_);

	// Невозможно открыть файл...
	if (pFile == NULL) return;

	DWORD loaded = 0;

	char Id[5];
	Id[4] = 0;
	loaded = pFile->Read(Id, 4);
	Assert (loaded == 4);
	if (strcmp (Id, "MSRC") != 0)
	{
		api->Trace ("MissionEditor: Incorrect file type");
		pFile->Release();
		return;
	}

	char Ver[5];
	Ver[4] = 0;
	loaded = pFile->Read(Ver, 4);
	Assert (loaded == 4);

	int version = -1;
	if (strcmp (Ver, "v1.5") == 0)
	{
		version = 0;
	}
	if (strcmp (Ver, "v2.5") == 0)
	{
		version = 1;
	}

	if (version == -1)
	{
		api->Trace ("MissionEditor: Incorrect file version");
		pFile->Release();
		return;
	}


// mission name----------------
	if (version == 1)
	{
		DWORD strsize = 0;
		loaded = pFile->Read(&strsize, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));
		char* TempStr = NEW char[strsize];
		loaded = pFile->Read(TempStr, strsize);
		Assert (loaded == strsize);

		if (SetCameraPos)
		{
			PanelWindow->MissionName->Text = TempStr;
			PanelWindow->MissionNameIsChange(NULL);
		}
		delete TempStr; 
	}
// mission name----------------

	Vector camSource(0.0f);
	Vector camTarget(0.0f);

	loaded = pFile->Read(&camSource.x, sizeof (float));
	Assert (loaded == sizeof (float));
	loaded = pFile->Read(&camSource.y, sizeof (float));
	Assert (loaded == sizeof (float));
	loaded = pFile->Read(&camSource.z, sizeof (float));
	Assert (loaded == sizeof (float));


	loaded = pFile->Read(&camTarget.x, sizeof (float));
	Assert (loaded == sizeof (float));
	loaded = pFile->Read(&camTarget.y, sizeof (float));
	Assert (loaded == sizeof (float));
	loaded = pFile->Read(&camTarget.z, sizeof (float));
	Assert (loaded == sizeof (float));

	if (SetCameraPos)
	{
		pFreeCamera->SetPosition (camSource);
		pFreeCamera->SetTarget(camTarget);
	}

	LoadTree (pFile, MainWindow->TreeView1);


	DWORD total_objects = 0;
	loaded = pFile->Read(&total_objects, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));


	// Загружаем объеты...
	for (DWORD n = 0; n < total_objects; n++)
	{

		DWORD s_len = 0;
		loaded = pFile->Read(&s_len, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));

		Assert (s_len < 512);

		char* ClassName = NEW char[s_len+1];
		ClassName[s_len] = 0;
		loaded = pFile->Read(ClassName, s_len);
		Assert (loaded == s_len);

		

		s_len = 0;
		loaded = pFile->Read(&s_len, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));

		Assert (s_len < 512);
		
		char* PathInTree = NEW char[s_len+1];
		PathInTree[s_len] = 0;
		loaded = pFile->Read(PathInTree, s_len);
		Assert (loaded == s_len);


		long level;
		loaded = pFile->Read(&level, sizeof (long));
		Assert (loaded == sizeof (long));

		DWORD version = 0;
		loaded = pFile->Read(&version, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));

		loaded = pFile->Read(&s_len, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));

		Assert (s_len < 512);
		
		char* ObjectName = NEW char[s_len+1];
		ObjectName[s_len] = 0;
		loaded = pFile->Read(ObjectName, s_len);
		Assert (loaded == s_len);

		DWORD attr_count = 0;
		loaded = pFile->Read(&attr_count, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));

		AttributeList* ldAttrList = NEW AttributeList;
		for (DWORD i = 0; i < attr_count; i++)
		{
			DWORD dwType = 0;
			loaded = pFile->Read(&dwType, sizeof (DWORD));
			Assert (loaded == sizeof (DWORD));

			//api->Trace("found attr - %d\n", dwType);

			if (dwType == IMOParams::t_group)
			{
				continue;
			}

			BaseAttribute* nAttr = CreateEmptyAttribute ((IMOParams::Type)dwType);
			nAttr->LoadFromFile(pFile, ClassName);
			//AttributeList* OriginalAttrList
			ldAttrList->Add (nAttr);
		} // loop attributes load


		MissionEditor::tCreatedMO* pCMO = CreateObject (ClassName, level, version, ObjectName, ldAttrList, PathInTree);

		delete PathInTree;
		delete ClassName;
		delete ObjectName;
		delete ldAttrList;
	} // loop objects load


	pFile->Release();
	MakeLoadedObjectsUniqueNames ();

	//UpdateAllObjects ();

}

void MissionLoad::UpdateAllObjects ()
{
	
	
	for (DWORD n  =0; n < editor->GetCreatedMissionObjectsCount(); n++)
	{
		MissionEditor::tCreatedMO* EditedObject = &editor->GetCreatedMissionObjectStructByIndex(n);
		
		MOPWriter wrt(EditedObject->Level, EditedObject->pObject.SPtr()->GetObjectID().c_str());
		EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
		miss->EditorUpdateObject(EditedObject->pObject.SPtr(), wrt);
#endif
		//EditedObject->pObject->EditMode_Update (wrt.Reader ());
	}
}

MissionEditor::tCreatedMO* MissionLoad::CreateObject (const char* ClassName, int level, DWORD version, const char* _ObjectName, AttributeList* ldAttrList, const char* PathInTree)
{
	const char* ObjectName = _ObjectName;

	if (strlen (ObjectName) <= 0) return NULL;


	//api->Trace ("Object - %s, Path - %s", ObjectName, PathInTree);
	string FullPath = PathInTree;
	FullPath += ObjectName;
	FullPath += "\\";

	GUITreeNode* fItem = NULL;
		//MainWindow->TreeView1->FindItem (FullPath.GetBuffer ());

	for (int n = 0; n < JLLoadedNode; n++)
	{
		const char* node_full_name = JLLoadedNode[n].node_fullname.GetBuffer();
		if (FullPath == node_full_name)
		{
			fItem = JLLoadedNode[n].node;
			break;
		}
	}

	Assert (fItem != NULL);
	Assert (fItem->Tag == TAG_OBJECT);

	fItem->CanDrop = false;

	

	
	int availCount = editor->AvailableMO.Size ();
	for (int n = 0; n< availCount; n++)
	{
		MissionEditor::tAvailableMO* pMissionObject = &editor->AvailableMO[n];
		// Если нашли нужный класс
		if (strcmp (pMissionObject->ClassName, ClassName) == 0)
		{

			AttributeList* compAL = CreateNecessaryAL (ClassName, version, pMissionObject->Params->GetVersion (), ldAttrList, pMissionObject->AttrList);


			//compAL->DebugLog();

			MOPWriter wrt(pMissionObject->Params->GetVersion (), ObjectName);
			compAL->AddToWriter (wrt);
			
			MOSafePointer mo;			
			
			if (!miss->CreateObject(mo, pMissionObject->ClassName, wrt.Reader ()))
			{
				api->Trace ("Mission Editor: ERROR Can't create object '%s' '%s'", ClassName, ObjectName);
				return NULL;
			}

			MissionEditor::tCreatedMO* nEntry = &editor->AddCreatedMissionObjectStruct();

			ObjectJustLoaded newEntry;
			newEntry.pObject = mo;
			newEntry.node =	 fItem;
			newEntry.pCreated = nEntry;
			obj.Add(newEntry);

		
			nEntry->Version = pMissionObject->Params->GetVersion ();
			nEntry->pObject = mo;
			nEntry->AttrList = NEW AttributeList;
			nEntry->Level = pMissionObject->Params->GetLevel ();
			nEntry->ClassName = pMissionObject->ClassName;
			nEntry->PathInTree = PathInTree;
			//strncpy (nEntry->ClassName, pMissionObject->ClassName, MAX_ENTITY_NAME);

			//strncpy (nEntry->PathInTree, PathInTree, MAXOBJECTPATH);


			
			nEntry->AttrList->CreateFromList (compAL);

			nEntry->AttrList->AddToTree (fItem, globalNodesPool);
      fItem->Data = nEntry;

      nEntry->AttrList->SetMasterData (nEntry);
      /*                  
			int c = nEntry->AttrList->GetCount ();
			for (int i = 0; i < c; i++)
			{
				nEntry->AttrList->Get(i)->MasterData = nEntry;
			} */
			

			delete compAL;
			return nEntry;


		}

	}
 //MissionEditor::tCreatedMO* cObject = editor->CreatedMO.Add ();;

	return NULL;
}


AttributeList* MissionLoad::CreateNecessaryAL (const char* ClassName, DWORD LoadedVersion, DWORD OriginalVersion, AttributeList* LoadedAttrList, AttributeList* OriginalAttrList)
{
	
	AttributeList* NecessaryAttrList = NEW AttributeList;

	//api->Trace ("SEARCH");
// Если версии не совпадают отрапортавать...
	if (LoadedVersion != OriginalVersion)
	{
		api->Trace ("Mission Editor: version is different !");
		//NecessaryAttrList->CreateFromList (OriginalAttrList);
		//return NecessaryAttrList;
	}

	array<BaseAttribute*> FlatList(_FL_, 2048);
	OriginalAttrList->GenerateFlatList(FlatList);

	for (int n = 0; n < (int)FlatList.Size(); n++)
	{
		BaseAttribute* ba = FlatList[n];
		IMOParams::Type tp = ba->GetType ();
		//api->Trace ("Work for attribute '%s'", ba->GetName ());

		BaseAttribute* ldattr = LoadedAttrList->FindInAttrList(ba->GetName (), tp);
		if (ldattr)
		{
			//IsCreated = true;
			ldattr->AfterLoad(ba, OriginalAttrList, ClassName);
			//NecessaryAttrList->AddParam (ldattr);
		}
	} // for all OriginalAttrList


	for (int n = 0; n < OriginalAttrList->GetCount(); n++)
	{
		BaseAttribute* ba = OriginalAttrList->Get(n);
		IMOParams::Type tp = ba->GetType ();
		//api->Trace ("Work for attribute '%s'", ba->GetName ());

		BaseAttribute* ldattr = LoadedAttrList->FindInAttrList(ba->GetName (), tp);
		if (ldattr)
		{
			//IsCreated = true;
			//ldattr->AfterLoad(ba, OriginalAttrList);
			NecessaryAttrList->AddParam (ldattr);
		} else
		{
			NecessaryAttrList->AddParam (ba);
		}
	} // for all OriginalAttrList



	//hack for groups...
	for (int q = 0; q < NecessaryAttrList->GetCount(); q++)
	{
		BaseAttribute* pBase = NecessaryAttrList->Get(q);
		if (pBase->GetType() == IMOParams::t_group)
		{
			pBase->GroupHack(LoadedAttrList);
		}
	}
	

	return NecessaryAttrList;
}


void MissionLoad::LoadTree (IFile* pFile, GUITreeView* TreeView)
{
	DWORD loaded = 0;
	int NodesInRoot = 0;
	loaded = pFile->Read(&NodesInRoot, sizeof (int));
	Assert (loaded == sizeof (int));

	for (int n = 0; n < NodesInRoot; n++)
	{
		GUITreeNode* ld_node = LoadNode (pFile, TreeView->Items);
	//if (n == 0)	TreeView->Items->Add (LoadedNode);
	}

}

GUITreeNode* MissionLoad::LoadNode (IFile* pFile, GUITreeNodes* nodes, int n)
{
	DWORD loaded = 0;

	DWORD s_len = 0;
	loaded = pFile->Read(&s_len, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	
	char* NodeName = NEW char[s_len+1];
	NodeName[s_len] = 0;
	loaded = pFile->Read(NodeName, s_len);
	Assert (loaded == s_len);


	s_len = 0;
	loaded = pFile->Read(&s_len, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	
	char* NodeImageName = NEW char[s_len+1];
	NodeImageName[s_len] = 0;
	loaded = pFile->Read(NodeImageName, s_len);
	Assert (loaded == s_len);

	int NodeTag = 0;
	loaded = pFile->Read(&NodeTag, sizeof (int));
	Assert (loaded == sizeof (int));

//	api->Trace ("[%d] Loaded node '%s' '%s' %d", n, NodeName, NodeImageName, NodeTag);
	GUITreeNode* LoadedNode = globalNodesPool->CreateNode();
	//LoadedNode->Text = ;
	LoadedNode->SetText(NodeName);
	LoadedNode->Image->Load (NodeImageName);
	LoadedNode->Tag = NodeTag;

	if (LoadedNode->Tag == TAG_FOLDER)
	{
		LoadedNode->Image->Load ("folder");
		LoadedNode->CanCopy = false;
		bool NeedCreate = true;
		for (int n = 0 ; n < nodes->GetCount(); n++)
		{
			if (crt_stricmp(nodes->Get(n)->GetText(), LoadedNode->GetText()) == 0)
			{
				LoadedNode->Release();
				LoadedNode = nodes->Get(n);
				NeedCreate = false;
			}
		}

		if (NeedCreate)
		{
			nodes->Add(LoadedNode);
		}
	}

	if (LoadedNode->Tag == TAG_OBJECT)
	{
		LoadedNode->Image->Load ("meditor\\mobject");
		//MakeUni
		//LoadedNode->Text = MakeUniqueName (LoadedNode->Text);
		nodes->Add(LoadedNode);
	}

	delete NodeName;
	delete NodeImageName;

	TreeNodeJustLoaded jlNode;
	jlNode.node = LoadedNode;
	jlNode.node_fullname = LoadedNode->GetFullPath();
	JLLoadedNode.Add(jlNode);


	int NodeChildCount = 0;
	loaded = pFile->Read(&NodeChildCount, sizeof (int));
	Assert (loaded == sizeof (int));
	//api->Trace ("Child count - %d", NodeChildCount);

	for (int i = 0; i < NodeChildCount; i++)
	{
		GUITreeNode* ch_node = LoadNode (pFile, &LoadedNode->Childs, (n+1));
		//LoadedNode->Childs.Add (ch_node);
	}

	return LoadedNode;
}


void MissionLoad::MakeLoadedObjectsUniqueNames ()
{

	for (int n = 0; n < obj; n++)
	{
		MOSafePointer finded = FindInMission (obj[n].pObject.Ptr()->GetObjectID().c_str());
		// finded не может быть NULL хотя бы сам себя должен найти...
		Assert (finded.Validate());

		//Если нашел не сам себя...
		if (finded != obj[n].pObject)
		{

			ObjectJustLoaded & jl = obj[n];
			const char * tmpName = MakeUniqueName(jl.pObject.Ptr()->GetObjectID().c_str());
			if(crt_stricmp(tmpName, jl.pObject.Ptr()->GetObjectID().c_str()) != 0)
			{
				MOPWriter wrt(jl.pCreated->Level, tmpName);
				jl.pCreated->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
				miss->EditorUpdateObject(jl.pObject.Ptr(), wrt);
#endif
				//jl.node->Text = ;
				jl.node->SetText(jl.pObject.Ptr()->GetObjectID().c_str());
			}
//			obj[n].pObject->EditMode_SetNewObjectID(MakeUniqueName(obj[n].pObject.Ptr()->GetObjectID().c_str()));
//			obj[n].node->Text = obj[n].pObject.Ptr()->GetObjectID();
		}
	}

	obj.DelAll();

}

void MissionLoad::LoadXML (const char* filename, bool SetCameraPos, bool bAsReadOnly)
{
#ifdef LOAD_DEBUG

	CoreCommand_GetMemStat memStat;
	api->ExecuteCoreCommand(memStat);
	api->Trace("=========> Allocations on start loading : %d (%fMb)", memStat.allocsPerFrame, memStat.totalAllocSize /1024.0f /1024.0f);
	


	CAProfResume();
#endif


	//pRS->EnableLoadingScreen(true);

	if (SetCameraPos)
	{
		array<string> & list = sMission->GetImportList();
		list.DelAll();
	}
	

	/* Max - теперь надо проверить не на созданность а попробывать загрузить файл (см. дальше)
	if (bAsReadOnly)
	{

		IFileService* pFS = (IFileService*)api->GetService("FileService");
		

		if (!pFS->IsExist(filename))
		{
			return;
		}

		globalNodesPool->DefaultReadOnly(true);
	}
	*/


	DWORD timeStart = GetTickCount();

	TiXmlDocument msr( filename );
	
	//------------------------------------
	//Max
	ZipMe file;
	bool loadOkay = file.ReadFile(filename);
	//Проверяем загрузился ли файл
	if (bAsReadOnly)
	{
		if (!loadOkay)
		{
			return;
		}
		globalNodesPool->DefaultReadOnly(true);
	}
	file.ClearText();
	msr.Parse((const char *)file.Data());
	//------------------------------------

	//bool loadOkay = msr.LoadFile();

	DWORD timeEnd1 = GetTickCount();

	if (!loadOkay)
	{
		throw ("this is temp gag, sorry !");
	}

	TiXmlElement* MisNode = msr.FirstChildElement( "mission" );

	if (!MisNode)
	{
		throw ("not mission file, sorry !");
	}
	const char* szMissionPakName = MisNode->Attribute("val");

	if (SetCameraPos)
	{
		PanelWindow->MissionName->Text = szMissionPakName;
		PanelWindow->MissionNameIsChange(NULL);
	}

	TiXmlElement* incNode = MisNode->FirstChildElement("include");
	if (incNode)
	{
		for(TiXmlElement* child = incNode->FirstChildElement(); child; child = child->NextSiblingElement())
		{
			const char* missName = child->Attribute("val");

			array<string> & list = sMission->GetImportList();

			list.Add(missName);
		}
	}



	TiXmlElement* CamNode = MisNode->FirstChildElement("camera");

	if (CamNode)
	{
		Vector vLookFrom;
		Vector vLookTo;

		TiXmlElement* node = CamNode->FirstChildElement("src_x");
		if (node) vLookFrom.x = fast_atof (node->Attribute("val"));

		node = CamNode->FirstChildElement("src_y");
		if (node) vLookFrom.y = fast_atof (node->Attribute("val"));

		node = CamNode->FirstChildElement("src_z");
		if (node) vLookFrom.z = fast_atof (node->Attribute("val"));

		node = CamNode->FirstChildElement("tgt_x");
		if (node) vLookTo.x = fast_atof (node->Attribute("val"));

		node = CamNode->FirstChildElement("tgt_y");
		if (node) vLookTo.y = fast_atof (node->Attribute("val"));

		node = CamNode->FirstChildElement("tgt_z");
		if (node) vLookTo.z = fast_atof (node->Attribute("val"));

		if (SetCameraPos)
		{
			pFreeCamera->SetPosition (vLookFrom);
			pFreeCamera->SetTarget(vLookTo);
		}
	}


	TiXmlElement* Objects = MisNode->FirstChildElement("objects");

	if (Objects)
	{
		bool bRes = ReadXMLTree (Objects, MainWindow->TreeView1->Items, globalNodesPool);
		if (!bRes)
		{
			//PanelWindow->CreateEmptyMission(NULL);
		}
	}

	if (duplicate_file)
	{
		fclose(duplicate_file);
	}

	globalNodesPool->DefaultReadOnly(false);


	dword timeEnd = GetTickCount();
	api->Trace("Mission load from XML %f sec.", (float)(timeEnd - timeStart) * 0.001f);
	api->Trace("XML parse time: %f sec.", (float)(timeEnd1 - timeStart) * 0.001f);
	api->Trace("Mission load without xml parse %f sec.", (float)(timeEnd - timeEnd1) * 0.001f);
	//api->Trace("Tree nodes created: %d", globalNodesPool->nodesCreated);


#ifdef LOAD_DEBUG
	CAProfPause();

	CoreCommand_GetMemStat memStat2;
	api->ExecuteCoreCommand(memStat2);

	
	api->Trace("=========> Allocations on end loading : %d", memStat2.allocsPerFrame);
	api->Trace("=========> Allocations while loading : %d", memStat2.allocsPerFrame - memStat.allocsPerFrame);

	api->Trace("=========> Allocations size while loading : %fMb", (memStat2.totalAllocSize - memStat.totalAllocSize) / 1024.0f / 1024.0f);


	

	

#endif


	if (!bAsReadOnly)
	{
		MainWindow->RefreshImportedMissions();
	}

	//pRS->EnableLoadingScreen(false);

}


bool MissionLoad::ReadXMLTree (TiXmlElement* Root, GUITreeNodes* nodes, TreeNodesPool* nodesPool)
{
	for(TiXmlElement* child = Root->FirstChildElement(); child; child = child->NextSiblingElement())
	{
		const char* val = child->Value();
		const char* ObjectName = child->Attribute("val");

		
		GUITreeNode* NewTreeNode = NULL;

		if (val[0] == 'f')
		{
			NewTreeNode = nodes->FindItemByName(ObjectName);
			if (NewTreeNode && NewTreeNode->Tag != TAG_FOLDER)
			{
				NewTreeNode = NULL;
			}
		}

		

		if (NewTreeNode == NULL)
		{
			NewTreeNode = nodesPool->CreateNode();
			//perf: string allocation !!!!
			//NewTreeNode->Text = ;
			NewTreeNode->SetText(ObjectName);
			nodes->Add(NewTreeNode);
		}
		
		//if (crt_stricmp("object", val) == 0)
		if (val[0] == 'o')
		{
			NewTreeNode->Tag = TAG_OBJECT;
			NewTreeNode->Image->Load ("meditor\\mobject");
			NewTreeNode->CanDrop = false;

			bool bRes = ReadXMLObject(child, NewTreeNode, nodesPool);

			if (!bRes)
			{
				NewTreeNode->Release();
				continue;
			}
		}

		//if (crt_stricmp("folder", val) == 0)
		if (val[0] == 'f')
		{
			NewTreeNode->Tag = TAG_FOLDER;
			NewTreeNode->Image->Load ("folder");
			NewTreeNode->CanCopy = false;
			//go to childs...
			bool q = ReadXMLTree(child, &NewTreeNode->Childs, nodesPool);
			if (!q)
			{
				return false;
			}
		}
	}

	return true;
}

bool MissionLoad::ReadXMLObject (TiXmlElement* Root, GUITreeNode* pTreeNode, TreeNodesPool* nodesPool)
{
	const char* ObjectName = Root->Attribute("val");
	const char* ClassName = NULL;
	DWORD dwVersion;

	TiXmlElement* node = Root->FirstChildElement("class_name");
	if (node)
	{
		ClassName = node->Attribute("val");
	}

	node = Root->FirstChildElement("version");
	if (node) dwVersion = strtoul (node->Attribute("val") + 2, NULL, 16);


	node = Root->FirstChildElement("attributes");

	if (node)
	{
		AttributeList* pLoadedList = ReadXMLAttributes(node, ClassName);


		MissionEditor::tAvailableMO* pObj = editor->GetAvailableClassByName(ClassName);
		if (!pObj)
		{
			api->Trace("Can't create object ! Class '%s' not found", ClassName);
			return false;
		}
		AttributeList* NewAttributeList = CreateNecessaryAL (ClassName, dwVersion, pObj->Params->GetVersion (), pLoadedList, pObj->AttrList);

		MOSafePointer existedObject;
		miss->FindObject(ConstString(ObjectName), existedObject);

		if ((ObjectName == NULL || ObjectName[0] == 0))
		{
			DuplicateOut("Nameless object '%s' renamed to ", ObjectName);
			ObjectName = MakeUniqueName(ClassName);
			DuplicateOut("'%s'\n", ObjectName);
			//pTreeNode->Text = ;
			pTreeNode->SetText(ObjectName);
		}

		if (existedObject.Validate())
		{
			DuplicateOut("Duplicated object '%s' renamed to ", ObjectName);
			ObjectName = MakeUniqueName(ObjectName);
			DuplicateOut("'%s'\n", ObjectName);
			//pTreeNode->Text = ;
			pTreeNode->SetText(ObjectName);
		}


		MOPWriter wrt(pObj->Params->GetVersion (), ObjectName);
		NewAttributeList->AddToWriter (wrt);


		

		MOSafePointer mo;
		if (!miss->CreateObject (mo, pObj->ClassName, wrt.Reader ()))
		{
			return false;
		}
		

		//MissionObject* mo = NULL;

		

		MissionEditor::tCreatedMO* nEntry = &editor->AddCreatedMissionObjectStruct();
		nEntry->Version = pObj->Params->GetVersion ();
		nEntry->pObject = mo;
		nEntry->Level = pObj->Params->GetLevel ();
		nEntry->ClassName = pObj->ClassName;
		//strncpy (nEntry->ClassName, pObj->ClassName, MAX_ENTITY_NAME);
		
		if (pTreeNode->Parent)
		{
			nEntry->PathInTree = pTreeNode->Parent->GetFullPath();
			//strncpy (nEntry->PathInTree, pTreeNode->Parent->GetFullPath(), MAXOBJECTPATH);
		} else
		{
			nEntry->PathInTree = "\\";
			//strncpy (nEntry->PathInTree, "\\", MAXOBJECTPATH);
		}
		
		//

		nEntry->AttrList = NULL;
		nEntry->AttrList = NEW AttributeList;
		nEntry->AttrList->CreateFromList (NewAttributeList);
		nEntry->AttrList->AddToTree (pTreeNode, nodesPool);
		nEntry->AttrList->SetMasterData (nEntry);
		pTreeNode->Data = nEntry;

		delete pLoadedList;
		delete NewAttributeList;
	}
	

	return true;
}

AttributeList* MissionLoad::ReadXMLAttributes (TiXmlElement* Root, const char* szMasterClass)
{
	AttributeList* pAttrList = NEW AttributeList;

	pAttrList->ReadXML (Root, szMasterClass);

	return pAttrList;
}