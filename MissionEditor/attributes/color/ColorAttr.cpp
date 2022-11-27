#include "colorattr.h"
#include "..\strutil.h"
#include "..\..\..\common_h\gui.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"
#include "..\..\forms\mainwindow.h"

extern IGUIManager* igui;
extern TMainWindow* MainWindow;
extern BaseAttribute* pEditableNode;
extern BaseAttribute* pRTEditedAttribute;

#include "..\..\forms\globalParams.h"

ColorAttribute & ColorAttribute::operator = (const ColorAttribute & source)
{
	SetValue (source.GetValue());
	SetMax (source.GetMax());
	SetMin (source.GetMin());
	BaseAttribute::Copy(*this, source);
	return *this;
}

ColorAttribute & ColorAttribute::operator = (const IMOParams::Colors& source)
{
	SetValue (source.def);
	SetMax (source.max);
	SetMin (source.min);
	SetName (source.name);
	SetIsLimit (source.isLimit);
	return *this;
}

ColorAttribute::ColorAttribute ()
{
	EditedObject = NULL;
  value = Color (0, 0, 0);
  min = Color (0, 0, 0);
  max = Color (255, 255, 255);
  
  Type = IMOParams::t_color;

	SetIsLimit (true);

	ColorEdit = NULL;
  
}

ColorAttribute::~ColorAttribute ()
{
}


void ColorAttribute::SetValue (const Color& val)
{
 value = val;
}

const Color&  ColorAttribute::GetValue () const
{
 return value;
}

void ColorAttribute::SetMin (const Color& val)
{
 min = val;
}

const Color&  ColorAttribute::GetMin () const
{
 return min;
}

void ColorAttribute::SetMax (const Color& val)
{
 max = val;
}

const Color&  ColorAttribute::GetMax () const
{
 return max;
}

void ColorAttribute::PopupEdit (int pX, int pY)
{
	ColorEdit = NEW TColorEdit (pX, pY);

	ColorEdit->editA->Text = IntToStr ((int)(value.a * 255.0f));
	ColorEdit->editR->Text = IntToStr ((int)(value.r * 255.0f));
	ColorEdit->editG->Text = IntToStr ((int)(value.g * 255.0f));
	ColorEdit->editB->Text = IntToStr ((int)(value.b * 255.0f));
	ColorEdit->TextIsChanged (NULL);
	ColorEdit->MasterAttrib = this;

//	ColorEdit->lDescription->Caption = GetName ();

/*
	if (GetIsLimit())
	{
		ColorEdit->lMin->Caption.Format ("Min : 0x%08X", min.GetDword());
		ColorEdit->lMax->Caption.Format ("Max : 0x%08X", max.GetDword());
	} else
		{
			ColorEdit->lMin->Caption = " ";
			ColorEdit->lMax->Caption = " ";
		}
*/

	igui->ShowModal (ColorEdit);

	// Обязательно нужно сделать...
	pForm = ColorEdit;



	//GUIWindow* wnd = igui->FindWindow (LONGEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

/*
	Form = NEW TLongEdit (0, 0);
	Form->SetPosition (pX, pY);

	Form->lDescription->Caption = GetName ();
	Form->eValue->Text = IntToStr (value);

	if (GetIsLimit())
	{
	Form->lMin->Caption = string ("min:") + string (IntToStr(min));
	Form->lMax->Caption = string ("max:") + string (IntToStr(max));
	} else
		{
			Form->lMin->Caption = " ";
			Form->lMax->Caption = " ";
		}


	Form->MasterAttrib = this;
	


	igui->ShowModal (Form);
*/
}
  
void ColorAttribute::AddToWriter (MOPWriter& wrt)
{
  wrt.AddColor (value);
}
  
void ColorAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;
	
	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);

	
	DWORD s_val = value.GetDword();
	written = pFile->Write(&s_val, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	
	s_val = min.GetDword();
	written = pFile->Write(&s_val, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	
	s_val = max.GetDword();
	written = pFile->Write(&s_val, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	
	
	
	DWORD sLimit = IsLimit;
	written = pFile->Write(&sLimit, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

}

#pragma warning (disable : 4800)


void ColorAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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


	DWORD ldVal, ldMin, ldMax;
	ldVal, ldMin, ldMax = 0;
	loaded = pFile->Read(&ldVal, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));

	loaded = pFile->Read(&ldMin, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));

	loaded = pFile->Read(&ldMax, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));

	value = Color (ldVal);
	min = Color (ldMin);
	max = Color (ldMax);

	//---------------------------------
	DWORD sLimit = 0;
	loaded = pFile->Read(&sLimit, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	IsLimit = (bool)sLimit;
	

}


void ColorAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	DWORD d_color = value.GetDword();

	DWORD d_color2 = d_color & 0xFFFFFF;


	gp->__tmpText = GetName ();

	string tVal;
	tVal.Format ("#c808080R %d, G %d, B %d", (int)(value.r*255.0f), (int)(value.g*255.0f), (int)(value.b*255.0f));


	if (v && v->IsEmpty())
	{
		*v = tVal;
	}



	gp->__tmpText += " #c" + string().Format("%06X", d_color2);
	gp->__tmpText += "№@c";
	gp->__tmpText += string (" '");
	gp->__tmpText += tVal;
	gp->__tmpText += string ("'"); 

	node->Image->Load("meditor\\color");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText(gp->__tmpText);
}

void ColorAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode, v);
	nodes->Add(nNode);


}


/* Начать изменение аттрибута в реалтайме... */
void ColorAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	EditedObject = MissionObject;
	Saved = value;
	PopupEdit (100, 100);
	ColorEdit->OnCancel = (CONTROL_EVENT)&ColorAttribute::OnColorCancel;
	ColorEdit->OnChange = (CONTROL_EVENT)&ColorAttribute::OnColorChange;
	ColorEdit->OnApply = (CONTROL_EVENT)&ColorAttribute::OnColorApply;
}

/* Подтвердить изменение аттрибута... */
void ColorAttribute::ApplyRTEdit ()
{
	CloseRTEdit ();
}

/* Отменить изменения в аттрибуте... */
void ColorAttribute::CancelRTEdit ()
{
	value = Saved;
	SetupMissionObject ();
	CloseRTEdit ();
}

void ColorAttribute::CloseRTEdit ()
{
	pEditableNode = this;
	MainWindow->UpdateTree(NULL);
	if (ColorEdit) ColorEdit->Close(NULL);
	pRTEditedAttribute = NULL;
}



void ColorAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());
}

void _cdecl ColorAttribute::OnColorApply (GUIControl* sender)
{
	ApplyRTEdit ();
	ColorEdit = NULL;
}

void _cdecl ColorAttribute::OnColorCancel (GUIControl* sender)
{
	CancelRTEdit ();
	ColorEdit = NULL;
}

void _cdecl ColorAttribute::OnColorChange (GUIControl* sender)
{
	value = Color(ColorEdit->SelectedColor);
	SetupMissionObject ();
}


void ColorAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<color val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value val = \"0x%08X\" />\n", value.GetDword());
	file.Write(level, "</color>\n");
}


void ColorAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
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
			ColorAttribute* SrcAttr = (ColorAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	TiXmlElement* node = Root->FirstChildElement("value");
	if (node) value = Color (strtoul (node->Attribute("val") + 2, NULL, 16));

}