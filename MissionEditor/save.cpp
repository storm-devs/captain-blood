#include "save.h"
#include "forms\mainwindow.h"
#include "forms\panel.h"
#include "attributes\AttributeList.h"

#include "..\common_h\tinyxml\tinyxml.h"
#include "textfile.h"

extern TMainWindow* MainWindow;
extern TPanelWindow* PanelWindow;

MissionSave::MissionSave (MissionEditor* _editor)
{
	editor = _editor;
}

MissionSave::~MissionSave ()
{
}
 

void MissionSave::SaveMSR (const char* input_filename)
{
	char filename[MAX_PATH];
	if (strstr(input_filename, ".msr") == NULL)
	{
		crt_snprintf (filename, MAX_PATH, "%s.msr", input_filename);
	} else
		{
			crt_snprintf (filename, MAX_PATH, "%s", input_filename);
		}

	string backFile = filename;
	backFile.AddExtention(".bak");
	DeleteFile(backFile);
	MoveFile(filename, backFile);


	IFileService* pFS = (IFileService*)api->GetService("FileService");
	IFile* pFile = pFS->OpenFile(filename, file_create_always, _FL_);

	// Невозможно создать файл...
	if (pFile == NULL) return;

	DWORD written = 0;
	
	written = pFile->Write("MSRC", 4);
	Assert (written == 4);

	written = pFile->Write ("v2.5", 4);
	Assert (written == 4);


//------------- mission name------------------
	string MissionName = PanelWindow->MissionName->Text;
	DWORD string_len = MissionName.Size(); 
	DWORD slen2save = string_len+1;

	written = pFile->Write(&slen2save, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

	written = pFile->Write(MissionName.GetBuffer(), string_len);
	Assert (written == string_len);

	BYTE Zero = 0;
	written = pFile->Write(&Zero, sizeof (BYTE));
	Assert (written == sizeof (BYTE));
//------------- mission name------------------



//------------------- save cam position
	IRender* rs = (IRender*)api->GetService("DX9Render");
	Matrix matCamera = rs->GetView ();
	Matrix matInvCamera = matCamera.Inverse ();
	
	Vector camSource = matInvCamera.pos;
	Vector camTarget = camSource + matInvCamera.vz;

	written = pFile->Write(&camSource.x, sizeof (float));
	Assert (written == sizeof (float));
	written = pFile->Write(&camSource.y, sizeof (float));
	Assert (written == sizeof (float));
	written = pFile->Write(&camSource.z, sizeof (float));
	Assert (written == sizeof (float));

	written = pFile->Write(&camTarget.x, sizeof (float));
	Assert (written == sizeof (float));
	written = pFile->Write(&camTarget.y, sizeof (float));
	Assert (written == sizeof (float));
	written = pFile->Write(&camTarget.z, sizeof (float));
	Assert (written == sizeof (float));
//------------------- save cam position


	WriteTree (pFile, MainWindow->TreeView1);


	
	DWORD total_objects = editor->GetCreatedMissionObjectsCount();

	written = pFile->Write(&total_objects, sizeof (DWORD));
	Assert (written == sizeof (DWORD));


	for (DWORD n = 0; n < total_objects; n++)
	{
		
		MissionEditor::tCreatedMO* cObject = &editor->GetCreatedMissionObjectStructByIndex(n);

		DWORD slen = strlen (cObject->ClassName);

		written = pFile->Write(&slen, sizeof (DWORD));
		Assert (written == sizeof (DWORD));

		written = pFile->Write(cObject->ClassName, slen);
		Assert (written == slen);

		//-add
		slen = strlen (cObject->PathInTree);
//		api->Trace ("Name : '%s', PathLen - %d, Path - '%s'", cObject->pObject->GetObjectID (), slen, cObject->PathInTree);
		written = pFile->Write(&slen, sizeof (DWORD));
		Assert (written == sizeof (DWORD));
		
		written = pFile->Write(cObject->PathInTree, slen);
		Assert (written == slen);
		//-add



		written = pFile->Write(&cObject->Level, sizeof (long));
		Assert (written == sizeof (long));

		written = pFile->Write(&cObject->Version, sizeof (dword));
		Assert (written == sizeof (dword));


		slen = strlen (cObject->pObject.Ptr()->GetObjectID ().c_str());

		written = pFile->Write(&slen, sizeof (DWORD));
		Assert (written == sizeof (DWORD));
		
		written = pFile->Write(cObject->pObject.Ptr()->GetObjectID ().c_str(), slen);
		Assert (written == slen);


		DWORD attrCount = cObject->AttrList->GetCountForSave();

		written = pFile->Write(&attrCount, sizeof (DWORD));
		Assert (written == sizeof (DWORD));


		

		for (DWORD i = 0; i < (DWORD)cObject->AttrList->GetCount(); i++)
		{
			IMOParams::Type t = cObject->AttrList->Get(i)->GetType ();
			DWORD dwType = (DWORD)t;
			
			written = pFile->Write(&dwType, sizeof (DWORD));
			Assert (written == sizeof (DWORD));


			cObject->AttrList->Get(i)->WriteToFile (pFile);
		}

	
	}


	pFile->Release();

}


void MissionSave::WriteTree (IFile* pFile, GUITreeView* TreeView)
{
	DWORD written = 0;
	int RootNodes = TreeView->Items->GetCount ();
	written = pFile->Write(&RootNodes, sizeof (int));
	Assert (written == sizeof (int));

	for (int n = 0; n < RootNodes; n++)
	{
		GUITreeNode* node = TreeView->Items->Get (n);
		WriteNode (pFile, node);
	}
	

}

void MissionSave::WriteNode (IFile* pFile, GUITreeNode* node)
{
//	api->Trace ("Write node %s, (%d), '%s'", node->Text.GetBuffer (), node->Tag, node->Image->GetName ());
	DWORD written = 0;
	DWORD tlen = crt_strlen(node->GetText());

	written = pFile->Write(&tlen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(node->GetText(), tlen);
	Assert (written == tlen);

	tlen = strlen (node->Image->GetName ());
	written = pFile->Write(&tlen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(node->Image->GetName (), tlen);
	Assert (written == tlen);

	written = pFile->Write(&node->Tag, sizeof (int));
	Assert (written == sizeof (int));

	int ch_count = node->Childs.GetCount ();
	int real_ch_count = 0;
	for (int n =0; n < ch_count; n++)
	{
		if (node->Childs.Get (n)->Tag != TAG_ATTRIBUTE) real_ch_count++;
	}
	
	written = pFile->Write(&real_ch_count, sizeof (int));
	Assert (written == sizeof (int));

	for (int i = 0; i < ch_count; i++)
	{
		if (node->Childs.Get (i)->Tag != TAG_ATTRIBUTE)
		{
			WriteNode (pFile, node->Childs.Get (i));
		}
	}
}


void MissionSave::SaveXML (const char* filename)
{
	string SavedFile = filename;
	SavedFile.AddExtention(".xmlz");

	string backFile = filename;
	backFile.AddExtention(".bak");
	DeleteFile(backFile);
	MoveFile(filename, backFile);


	TextFile file(SavedFile.c_str());

	
	file.Write(0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	string MissionName = "";
	MissionName = PanelWindow->MissionName->Text;
	file.Write(1, "<mission val=\"%s\">\n", MissionName.c_str());


	IRender* rs = (IRender*)api->GetService("DX9Render");
	Matrix matCamera = rs->GetView ();
	Matrix matInvCamera = matCamera.Inverse ();
	Vector camSource = matInvCamera.pos;
	Vector camTarget = camSource + matInvCamera.vz;

	array<string> & list = sMission->GetImportList();
	file.Write(2, "<include>\n");

	for (dword i = 0; i < list.Size(); i++)
	{
		file.Write(3, "<inc val = \"%s\" />\n", list[i].c_str());
	}

	file.Write(2, "</include>\n");

/*
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
*/

	file.Write(2, "<camera>\n");
		file.Write(3, "<src_x val = \"%3.2f\" />\n", camSource.x);
		file.Write(3, "<src_y val = \"%3.2f\" />\n", camSource.y);
		file.Write(3, "<src_z val = \"%3.2f\" />\n", camSource.z);

		file.Write(3, "<tgt_x val = \"%3.2f\" />\n", camTarget.x);
		file.Write(3, "<tgt_y val = \"%3.2f\" />\n", camTarget.y);
		file.Write(3, "<tgt_z val = \"%3.2f\" />\n", camTarget.z);
	file.Write(2, "</camera>\n");


	file.Write(2, "<objects>\n");

	WriteTreeToXML (file, MainWindow->TreeView1);

	file.Write(2, "</objects>\n");

	
	file.Write(1, "</mission>\n");


	//doc.RootElement();

	//doc.SaveFile();
}


void MissionSave::WriteTreeToXML (TextFile &file, GUITreeView* TreeView)
{
	int RootNodes = TreeView->Items->GetCount ();
	for (int n = 0; n < RootNodes; n++)
	{
		GUITreeNode* node = TreeView->Items->Get (n);
		if (node->bReadOnly)
		{
			continue;
		}
		WriteTreeNodeToXML (file, node, 2);
	}
}

void MissionSave::WriteTreeNodeToXML (TextFile &file, GUITreeNode* node, int level)
{
	string NodeType = "folder";

	if (node->Tag == TAG_FOLDER) NodeType = "folder";
	if (node->Tag == TAG_OBJECT) NodeType = "object";
	if (node->Tag == TAG_ATTRIBUTE) NodeType = "attribute";
	if (node->Tag == TAG_EVENT) NodeType = "event";
	if (node->Tag == TAG_ARRAYITEM) NodeType = "array_item";







	file.Write(level, "<%s val = \"%s\">\n", NodeType.c_str(), node->GetText());


	if (node->Tag == TAG_OBJECT && !node->bReadOnly)
	{
		MissionEditor::tCreatedMO* pMo = (MissionEditor::tCreatedMO*)node->Data;
		WriteObject (file, pMo, level+1);
	}
	


	//
	//node->Image->GetName ()

	int ch_count = node->Childs.GetCount();
	for (int i = 0; i < ch_count; i++)
	{
		if (node->Childs.Get (i)->Tag == TAG_ATTRIBUTE) continue;
		if (node->Childs.Get (i)->bReadOnly) continue;

		WriteTreeNodeToXML (file, node->Childs.Get (i), level+1);
	}

	file.Write(level, "</%s>\n", NodeType.c_str());
	
}


void MissionSave::WriteObject(TextFile &file, MissionEditor::tCreatedMO* pObject, int level)
{
	file.Write(level, "<class_name val = \"%s\" />\n", pObject->ClassName);
	file.Write(level, "<version val = \"0x%08X\" />\n", pObject->Version);

	pObject->AttrList->WriteToXML (file, level);

	

}