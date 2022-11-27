#include "longattr.h"
#include "..\strutil.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"

extern IGUIManager* igui;
extern MissionEditor* sMission;

#include "..\..\forms\globalParams.h"

LongAttribute& LongAttribute::operator = (const LongAttribute & source)
{
	SetValue (source.GetValue());
	SetMax (source.GetMax());
	SetMin (source.GetMin());
	BaseAttribute::Copy(*this, source);
	return *this;
}

LongAttribute& LongAttribute::operator = (const IMOParams::Long& source)
{
	SetValue (source.def);
	SetMax (source.max);
	SetMin (source.min);
	SetName (source.name);
	SetIsLimit (source.isLimit);
	return *this;
}

LongAttribute::LongAttribute ()
{
	RealTimeDesc = NULL;
	ost = 0.0f;
	EditedObject = NULL;
	RealTimeValue = NULL;
  value = 33;
	min = 0;
  max = 100;
  
  Type = IMOParams::t_long;

	SetIsLimit (true);


  
}

LongAttribute::~LongAttribute ()
{
}


void LongAttribute::SetValue (long val)
{
 value = val;
}

long LongAttribute::GetValue () const
{
 return value;
}

void LongAttribute::SetMin (long val)
{
 min = val;
}

long LongAttribute::GetMin () const
{
 return min;
}

void LongAttribute::SetMax (long val)
{
 max = val;
}

long LongAttribute::GetMax () const
{
 return max;
}

void LongAttribute::PopupEdit (int pX, int pY)
{
	//GUIWindow* wnd = igui->FindWindow (LONGEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

		Form = NEW TLongEdit (0, 0);
	Form->SetPosition (pX, pY);

	
	Form->eValue->Text = IntToStr (value);
	Form->eValue->Hint = GetName ();

	if (GetIsLimit())
	{
		Form->eValue->Hint += string ("\nmin:") + string (IntToStr(min));
		Form->eValue->Hint += string ("\nmax:") + string (IntToStr(max));
	}


	Form->MasterAttrib = this;
	


	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;


}
  
void LongAttribute::AddToWriter (MOPWriter& wrt)
{
	wrt.AddLong (value);
}
  

void LongAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;
	
	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);
	
	
	written = pFile->Write(&value, sizeof (long));
	Assert (written == sizeof (long));
	
	written = pFile->Write(&min, sizeof (long));
	Assert (written == sizeof (long));
	
	written = pFile->Write(&max, sizeof (long));
	Assert (written == sizeof (long));
	
	
	
	DWORD sLimit = IsLimit;
	written = pFile->Write(&sLimit, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

}

#pragma warning (disable : 4800)


void LongAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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

	long ldValue, ldMin, ldMax;
	ldValue = ldMin = ldMax = 0;

	loaded = pFile->Read(&ldValue, sizeof (long));
	Assert (loaded == sizeof (long));

	loaded = pFile->Read(&ldMin, sizeof (long));
	Assert (loaded == sizeof (long));

	loaded = pFile->Read(&ldMax, sizeof (long));
	Assert (loaded == sizeof (long));

	value = ldValue;
	min = 0;
	max = 13;

	DWORD sLimit = 0;
	loaded = pFile->Read(&sLimit, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	IsLimit = (bool)sLimit;



	if (!IsLimit) return;

//------------------[  Берем минимум и максимум из исходного ]-----------------------------
	int availCount = sMission->AvailableMO.Size ();
	for (int n = 0; n< availCount; n++)
	{
		MissionEditor::tAvailableMO* pMissionObject = &sMission->AvailableMO[n];
		// Если нашли нужный класс
		if (strcmp (pMissionObject->ClassName, ClassName) == 0)
		{
			for (int n = 0; n < pMissionObject->AttrList->GetCount(); n++)
			{
				BaseAttribute* SourceAttr = pMissionObject->AttrList->Get(n);
				IMOParams::Type SourceType = SourceAttr->GetType ();
				//Нашли нужный тип и нужное имя...
				if (SourceType == Type &&	crt_stricmp (SourceAttr->GetName(), Name) == 0)
				{
					LongAttribute* SourceAttrLong = (LongAttribute*)SourceAttr;
					SetMax (SourceAttrLong->GetMax());
					SetMin (SourceAttrLong->GetMin());

					if (value < min) value = min;
					if (value > max) value = max;
				}
			}
		}
	}
//------------------[  Берем минимум и максимум из исходного ]-----------------------------


}


void LongAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	gp->__tmpText += string (" ") + string (IntToStr(value)); 

	node->Image->Load("meditor\\num");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText(gp->__tmpText);


}

void LongAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode);

	nodes->Add(nNode);
}



void LongAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	EditedObject = MissionObject;
  SavedValue = value;
	RealTimeValue = NEW GUIEdit (parent, 10, 10, 220, 19);
	RealTimeValue->Text = IntToStr(value);
	RealTimeValue->pFont->SetName("arialcyrsmall");
	RealTimeValue->OnAdvNumChange = (CONTROL_EVENT)&LongAttribute::OnRTAdvChange;
	RealTimeValue->OnAccept = (CONTROL_EVENT)&LongAttribute::OnRTValueChange;
	RealTimeValue->Hint = "Drag value to smooth change";
	ost = 0.0f;


	RealTimeDesc = NEW GUILabel (parent, 10, 40, 220, 19);
	RealTimeDesc->Caption = "#b#cFFFFFF";
	RealTimeDesc->Caption += MissionObject->pObject.Ptr()->GetObjectID().c_str();
	RealTimeDesc->Caption += "#cC0FFFF.";
	RealTimeDesc->Caption += GetName ();
	RealTimeDesc->pFont->SetName("arialcyrsmall");

	RealTimeDesc->Caption += "@c@b";
	if (!IsLimit)
	{
		RealTimeDesc->Caption += "\nNot limited attribute";
	} else
	{
		RealTimeDesc->Caption += "\nLimited attribute";
		RealTimeDesc->Caption += "\nMin: #cFFFFFF";
		RealTimeDesc->Caption += IntToStr (min);
		RealTimeDesc->Caption += "\n@cMax: #cFFFFFF";
		RealTimeDesc->Caption += IntToStr (max);
	}

	RealTimeDesc->Layout = GUILABELLAYOUT_Left;

}


void LongAttribute::ApplyRTEdit ()
{

	CloseRTEdit ();
}

void LongAttribute::CancelRTEdit ()
{
  value = SavedValue;
	SetupMissionObject ();
	CloseRTEdit ();
}

void LongAttribute::CloseRTEdit ()
{
	if (RealTimeValue)
	{
		delete RealTimeValue;
		RealTimeValue = NULL;
	}

	if (RealTimeDesc)
	{
		delete RealTimeDesc;
		RealTimeDesc = NULL;
	}

}

void _cdecl LongAttribute::OnRTValueChange (GUIControl* sender)
{
	value = atoi (RealTimeValue->Text.GetBuffer());
	Clamp ();
	RealTimeValue->Text = IntToStr(value);
	SetupMissionObject ();
}

void LongAttribute::Clamp ()
{
	if (!IsLimit) return;
	if (value < min) value = min;
	if (value > max) value = max;
	if (RealTimeValue) RealTimeValue->Text = IntToStr(value);
}

void LongAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());
}

void _cdecl LongAttribute::OnRTAdvChange (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);

	ost =+ igui->GetCursor()->DeltaY;
	int real = (int)(ost / 2.0f);
	ost = ost - real*2.0f;
	value -= real;

	Clamp ();
	RealTimeValue->Text = IntToStr(value);
	SetupMissionObject ();
}


void LongAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<long val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value val = \"%d\" />\n", value);
	file.Write(level, "</long>\n");
}

void LongAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
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
			LongAttribute* SrcAttr = (LongAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	TiXmlElement* node = Root->FirstChildElement("value");
	if (node) value = atoi (node->Attribute("val"));

}