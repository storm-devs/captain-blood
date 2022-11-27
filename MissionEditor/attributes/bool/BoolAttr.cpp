#include "boolattr.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"


extern IGUIManager* igui;

#include "..\..\forms\globalParams.h"

BoolAttribute& BoolAttribute::operator = (const BoolAttribute & source)
{
	SetValue (source.GetValue());
	BaseAttribute::Copy(*this, source);
	return *this;
}

BoolAttribute& BoolAttribute::operator = (const IMOParams::Bool& source)
{
	SetName (source.name);
	SetIsLimit (source.isLimit);
	SetValue (source.def);
	return *this;
}

BoolAttribute::BoolAttribute ()
{
	EditedObject = NULL;
	cbRealTimeValue = NULL;
  Type = IMOParams::t_bool;
  value = false;



	//const IMOParams::Param* param
}

BoolAttribute::~BoolAttribute ()
{

}


void BoolAttribute::SetValue (bool val)
{
	value = val;
	if (cbRealTimeValue)
	{
		cbRealTimeValue->Checked = value;
		SetupMissionObject ();
	}
}

bool BoolAttribute::GetValue () const
{
	return value;
}

  
  
void BoolAttribute::AddToWriter (MOPWriter& wrt)
{
	wrt.AddBool (value);
}
  

void BoolAttribute::PopupEdit (int pX, int pY)
{
	//GUIWindow* wnd = igui->FindWindow (BOOLEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

	Form = NEW TBoolEdit (0, 0);
	Form->SetPosition (pX, pY);
	Form->cbValue->Checked = value;
	Form->cbValue->Caption = GetName ();
	Form->MasterAttrib = this;


	igui->ShowModal (Form);
	// Обязательно нужно сделать...
	pForm = Form;
}

void BoolAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;

	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);

	

	DWORD save_val = value;
	written = pFile->Write(&save_val, sizeof (DWORD));

	Assert (written == sizeof (DWORD));
}


#pragma warning (disable : 4800)


void BoolAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
{
	DWORD loaded = 0;
	
	DWORD slen = 0;
	loaded = pFile->Read(&slen, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	
	char* ldName = NEW char[slen+1];
	ldName[slen] = 0;
	loaded = pFile->Read(ldName, slen);
	Assert (loaded == slen);
	SetName (ldName);

	delete ldName;


	DWORD load_val = 0;
	loaded = pFile->Read(&load_val, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	value = (bool)load_val;



}

void BoolAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	//tmpText = string ("#b") + GetName () + string ("#n");
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	if (value == true) 
		gp->__tmpText += " 'true'"; 
	else
		gp->__tmpText += " 'false'"; 

	node->Image->Load("meditor\\bool");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText( gp->__tmpText);
}

void BoolAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode);
	nodes->Add(nNode);
	

}



void BoolAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	EditedObject =	MissionObject;
	SavedValue = value;
	//Form = NEW TBoolEdit (0, 0, parent);
	cbRealTimeValue = NEW GUICheckBox (parent, 10, 10, 100, 20);
	cbRealTimeValue->FontColor = 0xFF000000;
	cbRealTimeValue->Caption = GetName ();
	cbRealTimeValue->pFont->SetName("arialcyrsmall");
	cbRealTimeValue->ImageChecked->Load ("checked");
	cbRealTimeValue->ImageNormal->Load ("normal");
	cbRealTimeValue->OnChange = (CONTROL_EVENT)&BoolAttribute::OnRTValueChange;
	cbRealTimeValue->Checked = value;

}
 
void BoolAttribute::ApplyRTEdit ()
{
	CloseRTEdit ();
}

void BoolAttribute::CancelRTEdit ()
{
	value = SavedValue;
	SetupMissionObject ();
	CloseRTEdit ();
}

void BoolAttribute::CloseRTEdit ()
{
	if (cbRealTimeValue) delete cbRealTimeValue;
	cbRealTimeValue = NULL;
}

void _cdecl BoolAttribute::OnRTValueChange (GUIControl* sender)
{
	value = cbRealTimeValue->Checked;
	SetupMissionObject ();
}

void BoolAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());
}


void BoolAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<bool val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value val = \"%d\" />\n", value);
	file.Write(level, "</bool>\n");
}

void BoolAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
{
	const char* objectName = Root->Attribute("val");
	SetName(objectName);

	TiXmlElement* node = Root->FirstChildElement("value");
	if (node) value = atoi (node->Attribute("val")) == 1;
}