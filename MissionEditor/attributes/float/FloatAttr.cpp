#include "floatattr.h"
#include "..\strutil.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"
#include "..\..\fast_atof.h"

extern IGUIManager* igui;


#include "..\..\forms\globalParams.h"

FloatAttribute & FloatAttribute::operator = (const FloatAttribute & source)
{
	SetValue (source.GetValue());
	SetMax (source.GetMax());
	SetMin (source.GetMin());
	BaseAttribute::Copy(*this, source);
	return *this;
}

FloatAttribute & FloatAttribute::operator = (const IMOParams::Float& source)
{
	SetValue (source.def);
	SetMax (source.max);
	SetMin (source.min);
	SetName (source.name);
	SetIsLimit (source.isLimit);
	return *this;
}

FloatAttribute::FloatAttribute ()
{
	EditedObject = NULL;
	RealTimeValue = NULL;
	RealTimeDesc = NULL;

  value = 33.0f;
	min = 0.0f;
  max = 100.0f;

  Type = IMOParams::t_float;


  
}

FloatAttribute::~FloatAttribute ()
{
}


void FloatAttribute::SetValue (float val)
{
 value = val;
}

float FloatAttribute::GetValue () const
{
 return value;
}

void FloatAttribute::SetMin (float val)
{
 min = val;
}

float FloatAttribute::GetMin () const
{
 return min;
}

void FloatAttribute::SetMax (float val)
{
 max = val;
}

float FloatAttribute::GetMax () const
{
 return max;
}

void FloatAttribute::PopupEdit (int pX, int pY)
{
	//GUIWindow* wnd = igui->FindWindow (FLOATEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

	Form = NEW TFloatEdit (0, 0);
	Form->SetPosition (pX, pY);

	
//	Form->lDescription->Caption = GetName ();
	Form->eValue->Text = FloatToStr (value);

	Form->eValue->Hint = GetName ();
	
	if (GetIsLimit())
	{
	  Form->eValue->Hint += string ("\nmin:") + string (FloatToStr(min));
		Form->eValue->Hint += string ("\nmax:") + string (FloatToStr(max));
	}

	Form->MasterAttrib = this;


	


	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;


}
  
void FloatAttribute::AddToWriter (MOPWriter& wrt)
{
	wrt.AddFloat (value);
}
  

void FloatAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;
	
	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);


	written = pFile->Write(&value, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&min, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&max, sizeof (float));
	Assert (written == sizeof (float));
	
	
	
	DWORD sLimit = IsLimit;
	written = pFile->Write(&sLimit, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

}

#pragma warning (disable : 4800)


void FloatAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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

	float ldValue, ldMin, ldMax;
	ldValue = ldMin = ldMax = 0.0f;
	
	loaded = pFile->Read(&ldValue, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldMin, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldMax, sizeof (float));
	Assert (loaded == sizeof (float));
	
	value = ldValue;
	min = ldMin;
	max = ldMax;
	


	//---------------------------------
	DWORD sLimit = 0;
	loaded = pFile->Read(&sLimit, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	IsLimit = (bool)sLimit;
	

}

void FloatAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	gp->__tmpText += string (" ") + string (FloatToStr(value)); 

	if (v && v->IsEmpty())
	{
		*v = string (FloatToStr(value));
	}

	node->Image->Load("meditor\\float");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText( gp->__tmpText);
}

void FloatAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode, v);
	nodes->Add(nNode);
}

void FloatAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	EditedObject = MissionObject;
	SavedValue = value;
	RealTimeValue = NEW GUIEdit (parent, 10, 10, 220, 19);
	RealTimeValue->Text = FloatToStr(value);
	RealTimeValue->pFont->SetName("arialcyrsmall");
	RealTimeValue->OnAdvNumChange = (CONTROL_EVENT)&FloatAttribute::OnRTAdvChange;
	RealTimeValue->OnAccept = (CONTROL_EVENT)&FloatAttribute::OnRTValueChange;
	
	RealTimeValue->Hint = "Drag value to smooth change";

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
		RealTimeDesc->Caption += FloatToStr (min);
		RealTimeDesc->Caption += "\n@cMax: #cFFFFFF";
		RealTimeDesc->Caption += FloatToStr (max);
	}
	
	RealTimeDesc->Layout = GUILABELLAYOUT_Left;
}


void FloatAttribute::ApplyRTEdit ()
{
	CloseRTEdit ();
}

void FloatAttribute::CancelRTEdit ()
{
	value = SavedValue;
	SetupMissionObject ();
	CloseRTEdit ();
}

void FloatAttribute::CloseRTEdit ()
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

void _cdecl FloatAttribute::OnRTValueChange (GUIControl* sender)
{
	value = fast_atof (RealTimeValue->Text.GetBuffer());
	Clamp ();
	RealTimeValue->Text = FloatToStr(value);
	SetupMissionObject ();
}

void FloatAttribute::Clamp ()
{
	if (!IsLimit) return;
	if (value < min) value = min;
	if (value > max) value = max;
	if (RealTimeValue) RealTimeValue->Text = FloatToStr(value);
}

void FloatAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());
}

void _cdecl FloatAttribute::OnRTAdvChange (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;


	if (api->DebugKeyState(VK_SHIFT))
	{
		offset = offset * 10.0f;
	}

	// Вано добавил, чтобы от 0.0 можно было мышкой увеличивать/уменьшать аттрибут
	if (fabsf(value) < 0.0001f)
	{
		value = 0.0001f * ((value < 0.0f) ? -1.0f : 1.0f);
	}

	value -= ((offset / 50.0f) * value) / 10.0f;


	Clamp ();
	RealTimeValue->Text = FloatToStr(value);
	SetupMissionObject ();
}

void FloatAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<float val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value val = \"%f\" />\n", value);
	file.Write(level, "</float>\n");
}

void FloatAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
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
			FloatAttribute* SrcAttr = (FloatAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	TiXmlElement* node = Root->FirstChildElement("value");
	if (node) value = fast_atof (node->Attribute("val"));



}