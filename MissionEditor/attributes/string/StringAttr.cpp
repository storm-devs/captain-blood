#include "StringAttr.h"
#include "..\strutil.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"


extern IGUIManager* igui;

#include "..\..\forms\globalParams.h"

StringAttribute & StringAttribute::operator = (const StringAttribute & source)
{
	SetValue (source.GetValue());
	SetMaxChars (source.GetMaxChars());
	SetMinChars (source.GetMinChars());
	flags = source.flags;
	BaseAttribute::Copy(*this, source);
	return *this;
}

StringAttribute & StringAttribute::operator = (const IMOParams::String& source)
{
	SetValue (source.def);
	SetMaxChars (source.maxChars);
	SetMinChars (source.minChars);
	flags = source.flags;
	SetName (source.name);
	SetIsLimit (source.isLimit);
	return *this;
}

StringAttribute::StringAttribute ()
{
	data_storage[0] = 0;

  minChars = 0;
  maxChars = 100;
	flags = 0;

  Type = IMOParams::t_string;


}

StringAttribute::~StringAttribute ()
{
	//api->Trace("string deleted\n");
}

void StringAttribute::SetValue (const char* value)
{
	crt_strncpy(data_storage, 100, value, 100);

//	this->value = value;
}

const char* StringAttribute::GetValue () const
{
	return data_storage;

	//return value;
}


void StringAttribute::SetMaxChars (int Max)
{
	maxChars = Max;
	if (maxChars > 100) maxChars = 100;
}

int StringAttribute::GetMaxChars () const
{
	return maxChars;
}

void StringAttribute::SetMinChars (int Min)
{
	minChars = Min;
}

int StringAttribute::GetMinChars () const
{
	return minChars;
}

void StringAttribute::PopupEdit (int pX, int pY)
{
  //GUIWindow* wnd = igui->FindWindow (STRINGEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

	
	Form = NEW TStringEdit (0, 0);	
	Form->SetPosition (pX, pY);

//	Form->lDescription->Caption = GetName ();
	Form->eValue->Text = data_storage;

	Form->eValue->Hint = GetName ();

	if (GetIsLimit())
	{
		
		Form->eValue->Hint = string ("\nMinChars : ") + string (IntToStr (minChars));
		Form->eValue->Hint = string ("\nMaxChars : ") + string (IntToStr (maxChars));

		string sflag = "\nFlags : ";
		for (int n = 0; n < 16; n++)
		{
			if (flags & 1) sflag += "1"; else sflag += "0";
		}
		Form->eValue->Hint = sflag;
		
	} 
	Form->MasterAttrib = this;

	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;


}
  
void StringAttribute::AddToWriter (MOPWriter& wrt)
{
	wrt.AddString (data_storage);
}
  

void StringAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;

	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);

	
	DWORD sLen = strlen (data_storage);
	written = pFile->Write(&sLen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	
	written = pFile->Write(&minChars, sizeof (int));
	Assert (written == sizeof (int));
	
	written = pFile->Write(&maxChars, sizeof (int));
	Assert (written == sizeof (int));

	written = pFile->Write(&flags, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

	written = pFile->Write(data_storage, sLen);
	Assert (written == sLen);


	
	DWORD sLimit = IsLimit;
	written = pFile->Write(&sLimit, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

}

#pragma warning (disable : 4800)


void StringAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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


	DWORD sLen = 0;
	loaded = pFile->Read(&sLen, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));

	int ldMinChars, ldMaxChars;
	ldMinChars, ldMaxChars = 0;
	loaded = pFile->Read(&ldMinChars, sizeof (int));
	Assert (loaded == sizeof (int));

	loaded = pFile->Read(&ldMaxChars, sizeof (int));
	Assert (loaded == sizeof (int));

	minChars = ldMinChars;
	ldMaxChars = ldMaxChars;

	DWORD ldFlags = 0;
	loaded = pFile->Read(&ldFlags, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	flags = ldFlags;

	char* ldValue = NEW char [sLen+1];
	ldValue[sLen] = 0;
	loaded = pFile->Read(ldValue, sLen);
	Assert (loaded == sLen);
	SetValue (ldValue);

	delete ldValue;




	//---------------------------------
	DWORD sLimit = 0;
	loaded = pFile->Read(&sLimit, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	IsLimit = (bool)sLimit;
	

}


void StringAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	gp->__tmpText += string (" '");
	gp->__tmpText += data_storage;
	gp->__tmpText += string (" '");

	if (v && v->IsEmpty())
	{
		*v = data_storage;
	}

	node->Image->Load("meditor\\string");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText(gp->__tmpText);


}

void StringAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{

	GUITreeNode* nNode = nodesPool->CreateNode();;

	UpdateTree(nNode, v);

	nodes->Add(nNode);
}


void StringAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<string val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value val = \"%s\" />\n", data_storage);
	file.Write(level, "</string>\n");
}

void StringAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
{
	const char* objectName = Root->Attribute("val");
	SetName(objectName);

	//Минимум и максимум берем из исходного
	MissionEditor::tAvailableMO* pObject = sMission->GetAvailableClassByName(szMasterClass);
	if (pObject)
	{
		BaseAttribute* SourceAttr = pObject->AttrList->FindInAttrList(GetName(), GetType());
		if (SourceAttr)
		{
			StringAttribute* SrcAttr = (StringAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	TiXmlElement* node = Root->FirstChildElement("value");
	if (node)
	{
		SetValue(node->Attribute("val"));
		//value = ;
	}
}