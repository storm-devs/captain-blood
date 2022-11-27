#include "LocStringAttr.h"
#include "..\strutil.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"


#include "..\..\forms\globalParams.h"
extern IGUIManager* igui;
ILocStrings * pLocStr = NULL;

LocStringAttribute & LocStringAttribute::operator = (const LocStringAttribute & source)
{
	SetValue (source.GetValue());
	BaseAttribute::Copy(*this, source);
	return *this;
}

LocStringAttribute & LocStringAttribute::operator = (const IMOParams::LocString& source)
{
	SetName (source.name);

	return *this;
}

LocStringAttribute::LocStringAttribute ()
{
	pLocStr = (ILocStrings*)api->GetService("LocStrings");

	currentID = -1;

  Type = IMOParams::t_locstring;


}

LocStringAttribute::~LocStringAttribute ()
{
	//api->Trace("string deleted\n");
}

void LocStringAttribute::SetValue (long strID)
{
	currentID = strID;
}

long LocStringAttribute::GetValue () const
{
	return currentID;
}



void LocStringAttribute::PopupEdit (int pX, int pY)
{
  //GUIWindow* wnd = igui->FindWindow (STRINGEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

	
	Form = NEW TLocStringEdit (0, 0);	
	Form->SetPosition (pX, pY);

//	Form->lDescription->Caption = GetName ();
	Form->TextValue->Text = pLocStr->GetString(currentID);
	Form->TextValue->Hint = GetName ();

	Form->MasterAttrib = this;

	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;


}
  
void LocStringAttribute::AddToWriter (MOPWriter& wrt)
{
	wrt.AddLocString(currentID);
}
  

void LocStringAttribute::WriteToFile (IFile* pFile)
{

}

#pragma warning (disable : 4800)


void LocStringAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
{
}


void LocStringAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";


	const char* currentStr = pLocStr->GetString(currentID);

	gp->__tmpText += string (" '") + string(currentStr) + string ("'");

	if (v && currentStr && v->IsEmpty())
	{
		*v = currentStr;
	}

	node->Image->Load("meditor\\string");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;


	if (gp->__tmpText.Size() > 120)
	{
		gp->__tmpText.Delete(120, gp->__tmpText.Size());
		gp->__tmpText += "...";
	}


	//nNode->Text = ;
	node->SetText(gp->__tmpText);
}

void LocStringAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode, v);

	nodes->Add(nNode);
}


void LocStringAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<locstring val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value val = \"%d\" />\n", currentID);
	file.Write(level, "</locstring>\n");
}

void LocStringAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
{
	const char* objectName = Root->Attribute("val");
	SetName(objectName);

	TiXmlElement* node = Root->FirstChildElement("value");
	if (node) currentID = atoi (node->Attribute("val"));


}