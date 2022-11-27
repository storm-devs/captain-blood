#include "PositionAttr.h"
#include "..\..\missioneditor.h"
#include "..\strutil.h"
#include "..\AttributeList.h"
#include "..\..\fast_atof.h"

extern IRender * pRS;
extern IGUIManager* igui;

// Ширина цифрового поля ввода
// в реалтаймовом редакторе...
#define WDTH 71

Vector vVectorClipboard = Vector(0.0f);

#include "..\..\forms\globalParams.h"

PositionAttribute & PositionAttribute::operator = (const PositionAttribute & source)
{
	SetValue (source.GetValue());
	SetMax (source.GetMax());
	SetMin (source.GetMin());
	BaseAttribute::Copy(*this, source);
	return *this;
}

PositionAttribute & PositionAttribute::operator = (const IMOParams::Position& source)
{
	SetValue (source.def);
	SetMax (source.max);
	SetMin (source.min);
	SetName (source.name);
	SetIsLimit (source.isLimit);
	return *this;
}

PositionAttribute::PositionAttribute ()
{
	RealTimeValueX = NULL;
	RealTimeValueY = NULL;
	RealTimeValueZ = NULL;
	RealTimeDesc = NULL;
	EditedObject = NULL;

	btnSetFromCamera = NULL;
	btnSetToZero = NULL;
	btnCopyFrom = NULL;
	btnPasteTo = NULL;



	value = Vector (0.0f);
	min = Vector (0.0f);
	max = Vector (0.0f);

	Type = IMOParams::t_position;


 
}

PositionAttribute::~PositionAttribute ()
{
}

void PositionAttribute::SetValue (const Vector& value)
{
	this->value = value;
	if (RealTimeValueX)	RealTimeValueX->Text = FloatToStr(value.x);
	if (RealTimeValueY)	RealTimeValueY->Text = FloatToStr(value.y);
	if (RealTimeValueZ)	RealTimeValueZ->Text = FloatToStr(value.z);


}

const Vector& PositionAttribute::GetValue () const
{
	return value;
}


void PositionAttribute::SetMax (const Vector& Max)
{
	max = Max;
}

const Vector& PositionAttribute::GetMax () const
{
	return max;
}


void PositionAttribute::SetMin (const Vector& Min)
{
 min = Min;
}

const Vector& PositionAttribute::GetMin () const
{
	return min;
}


  
void PositionAttribute::PopupEdit (int pX, int pY)
{
  //GUIWindow* wnd = igui->FindWindow (POSITIONEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

 Form = NEW TPositionEdit (0, 0);
	Form->SetPosition (pX, pY);
	
//	Form->lDescription->Caption = GetName ();
	Form->eValueX->Text = FloatToStr (value.x);
	Form->eValueY->Text = FloatToStr (value.y);
	Form->eValueZ->Text = FloatToStr (value.z);


	Form->eValueX->Hint = GetName ();
	Form->eValueX->Hint += "\nX component";

	Form->eValueY->Hint = GetName ();
	Form->eValueY->Hint += "\nY component";

	Form->eValueZ->Hint = GetName ();
	Form->eValueZ->Hint += "\nZ component";

	if (GetIsLimit())
	{
		Form->eValueX->Hint += string ("\nmin : ") + string (FloatToStr(min.x));
		Form->eValueX->Hint += string ("\nmax : ") + string (FloatToStr(max.x));

		Form->eValueY->Hint += string ("\nmin : ") + string (FloatToStr(min.y));
		Form->eValueY->Hint += string ("\nmax : ") + string (FloatToStr(max.y));

		Form->eValueZ->Hint += string ("\nmin : ") + string (FloatToStr(min.z));
		Form->eValueZ->Hint += string ("\nmax : ") + string (FloatToStr(max.z));
	} 

	Form->MasterAttrib = this;
	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;
}


  
void PositionAttribute::AddToWriter (MOPWriter& wrt)
{

	wrt.AddPosition (value);
}
  
void PositionAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;
	
	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);


	written = pFile->Write(&value.x, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&value.y, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&value.z, sizeof (float));
	Assert (written == sizeof (float));
	


	
	written = pFile->Write(&min.x, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&min.y, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&min.z, sizeof (float));
	Assert (written == sizeof (float));



	written = pFile->Write(&max.x, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&max.y, sizeof (float));
	Assert (written == sizeof (float));
	
	written = pFile->Write(&max.z, sizeof (float));
	Assert (written == sizeof (float));

	
	DWORD sLimit = IsLimit;
	written = pFile->Write(&sLimit, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

}

#pragma warning (disable : 4800)


void PositionAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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


	float ldValueX, ldValueY, ldValueZ, ldMinX, ldMinY, ldMinZ, ldMaxX, ldMaxY, ldMaxZ;
	ldValueX = ldValueY = ldValueZ = ldMinX = ldMinY = ldMinZ = ldMaxX = ldMaxY = ldMaxZ = 0.0f;
	
	loaded = pFile->Read(&ldValueX, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldValueY, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldValueZ, sizeof (float));
	Assert (loaded == sizeof (float));



	loaded = pFile->Read(&ldMinX, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldMinY, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldMinZ, sizeof (float));
	Assert (loaded == sizeof (float));
	
	
	
	loaded = pFile->Read(&ldMaxX, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldMaxY, sizeof (float));
	Assert (loaded == sizeof (float));
	
	loaded = pFile->Read(&ldMaxZ, sizeof (float));
	Assert (loaded == sizeof (float));

	value = Vector (ldValueX, ldValueY, ldValueZ);
	min = Vector (ldMinX, ldMinY, ldMinZ);
	max = Vector (ldMaxX, ldMaxY, ldMaxZ);



	//---------------------------------
	DWORD sLimit = 0;
	loaded = pFile->Read(&sLimit, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));
	IsLimit = (bool)sLimit;
	

}


void PositionAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	gp->__tmpText += string (" ") + string (FloatToStr(value.x)); 
	gp->__tmpText += string (", ") + string (FloatToStr(value.y)); 
	gp->__tmpText += string (", ") + string (FloatToStr(value.z)); 

	node->Image->Load("meditor\\pos");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText( gp->__tmpText);

}

void PositionAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode);

	nodes->Add(nNode);
}


/* Начать изменение аттрибута в реалтайме... */
void PositionAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	//RealTimeValueX->OnAdvNumChange = (CONTROL_EVENT)OnRTAdvChange;
	//RealTimeValueX->OnAccept = (CONTROL_EVENT)OnRTValueChange;


	EditedObject = MissionObject;
	SavedValue = value;

	RealTimeValueX = NEW GUIEdit (parent, 10, 10, WDTH, 19);
	RealTimeValueX->Text = FloatToStr(value.x);
	RealTimeValueX->pFont->SetName("arialcyrsmall");
	RealTimeValueX->Hint = "Drag value to smooth change";
	RealTimeValueX->OnAdvNumChange = (CONTROL_EVENT)&PositionAttribute::OnRTAdvChangeX;
	RealTimeValueX->OnAccept = (CONTROL_EVENT)&PositionAttribute::OnRTValueChangeX;


	RealTimeValueY = NEW GUIEdit (parent, 10+WDTH+5, 10, WDTH, 19);
	RealTimeValueY->Text = FloatToStr(value.y);
	RealTimeValueY->pFont->SetName("arialcyrsmall");
	RealTimeValueY->Hint = "Drag value to smooth change";
	RealTimeValueY->OnAdvNumChange = (CONTROL_EVENT)&PositionAttribute::OnRTAdvChangeY;
	RealTimeValueY->OnAccept = (CONTROL_EVENT)&PositionAttribute::OnRTValueChangeY;


	RealTimeValueZ = NEW GUIEdit (parent, 10+(WDTH*2)+5+5, 10, WDTH, 19);
	RealTimeValueZ->Text = FloatToStr(value.z);
	RealTimeValueZ->pFont->SetName("arialcyrsmall");
	RealTimeValueZ->Hint = "Drag value to smooth change";
	RealTimeValueZ->OnAdvNumChange = (CONTROL_EVENT)&PositionAttribute::OnRTAdvChangeZ;
	RealTimeValueZ->OnAccept = (CONTROL_EVENT)&PositionAttribute::OnRTValueChangeZ;


	btnSetFromCamera = NEW GUIButton (parent, 10, 150, 32, 32);
	btnSetFromCamera->Glyph->Load("meditor\\big_camera");
	btnSetFromCamera->Hint = "Capture param from camera";
	btnSetFromCamera->FlatButton = true;
	btnSetFromCamera->OnMouseDown = (CONTROL_EVENT)&PositionAttribute::OnCameraCapture;
	
	
	btnSetToZero = NEW GUIButton (parent, 52, 150, 32, 32);
	btnSetToZero->Glyph->Load("meditor\\big_zero");
	btnSetToZero->Hint = "Zero param";
	btnSetToZero->FlatButton = true;
	btnSetToZero->OnMouseDown = (CONTROL_EVENT)&PositionAttribute::OnZero;

	btnCopyFrom = NEW GUIButton (parent, 52+42, 150, 32, 32);
	btnCopyFrom->Glyph->Load("meditor\\big_copy");
	btnCopyFrom->Hint = "Copy";
	btnCopyFrom->FlatButton = true;
	btnCopyFrom->OnMouseDown = (CONTROL_EVENT)&PositionAttribute::OnCopy;

	btnPasteTo = NEW GUIButton (parent, 52+42+42, 150, 32, 32);
	btnPasteTo->Glyph->Load("meditor\\big_paste");
	btnPasteTo->Hint = "Paste";
	btnPasteTo->FlatButton = true;
	btnPasteTo->OnMouseDown = (CONTROL_EVENT)&PositionAttribute::OnPaste;







//-----------------------------------------------------------
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
		RealTimeDesc->Caption += FloatToStr (min.x);
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (min.y);
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (min.z);
		RealTimeDesc->Caption += "\n@cMax: #cFFFFFF";
		RealTimeDesc->Caption += FloatToStr (max.x);
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (max.y);
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (max.z);
	}

	RealTimeDesc->Layout = GUILABELLAYOUT_Left;


}

/* Подтвердить изменение аттрибута... */
void PositionAttribute::ApplyRTEdit ()
{
	CloseRTEdit ();
}

/* Отменить изменения в аттрибуте... */
void PositionAttribute::CancelRTEdit ()
{
	value = SavedValue;
	SetupMissionObject ();
	CloseRTEdit ();
}

void PositionAttribute::CloseRTEdit ()
{
	if (RealTimeValueX)
	{
		delete RealTimeValueX;
		RealTimeValueX = NULL;
	}
	if (RealTimeValueY)
	{
		delete RealTimeValueY;
		RealTimeValueY = NULL;
	}
	if (RealTimeValueZ)
	{
		delete RealTimeValueZ;
		RealTimeValueZ = NULL;
	}

	if (RealTimeDesc)
	{
		delete RealTimeDesc;
		RealTimeDesc = NULL;
	}

	if (btnSetFromCamera)
	{
		delete btnSetFromCamera;
		btnSetFromCamera = NULL;
	}


	if (btnSetToZero)
	{
		delete btnSetToZero;
		btnSetToZero = NULL;
	}

	if (btnCopyFrom)
	{
		delete btnCopyFrom;
		btnCopyFrom = NULL;
	}

	if (btnPasteTo)
	{
		delete btnPasteTo;
		btnPasteTo = NULL;
	}
	

}

void PositionAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());
}


void PositionAttribute::Clamp ()
{
	if (!IsLimit) return;
	if (value.x < min.x) value.x = min.x;
	if (value.x > max.x) value.x = max.x;
	if (value.y < min.y) value.y = min.y;
	if (value.y > max.y) value.y = max.y;
	if (value.z < min.z) value.z = min.z;
	if (value.z > max.z) value.z = max.z;
	if (RealTimeValueX)	RealTimeValueX->Text = FloatToStr(value.x);
	if (RealTimeValueY)	RealTimeValueY->Text = FloatToStr(value.y);
	if (RealTimeValueZ)	RealTimeValueZ->Text = FloatToStr(value.z);
}

void _cdecl PositionAttribute::OnRTValueChangeX (GUIControl* sender)
{
	value.x = fast_atof (RealTimeValueX->Text.GetBuffer());
	Clamp ();
	RealTimeValueX->Text = FloatToStr(value.x);
	SetupMissionObject ();
}

void _cdecl PositionAttribute::OnRTAdvChangeX (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;
	value.x -= (offset / 50.0f);

	Clamp ();
	RealTimeValueX->Text = FloatToStr(value.x);
	SetupMissionObject ();
}

void _cdecl PositionAttribute::OnRTValueChangeY (GUIControl* sender)
{
	value.y = fast_atof (RealTimeValueY->Text.GetBuffer());
	Clamp ();
	RealTimeValueY->Text = FloatToStr(value.y);
	SetupMissionObject ();
}

void _cdecl PositionAttribute::OnRTAdvChangeY (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;
	value.y -= (offset / 50.0f);

	Clamp ();
	RealTimeValueY->Text = FloatToStr(value.y);
	SetupMissionObject ();

}

void _cdecl PositionAttribute::OnRTValueChangeZ (GUIControl* sender)
{
	value.z = fast_atof (RealTimeValueZ->Text.GetBuffer());
	Clamp ();
	RealTimeValueZ->Text = FloatToStr(value.z);
	SetupMissionObject ();
}

void _cdecl PositionAttribute::OnRTAdvChangeZ (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;
	value.z -= (offset / 50.0f);

	Clamp ();
	RealTimeValueZ->Text = FloatToStr(value.z);
	SetupMissionObject ();
}

void _cdecl PositionAttribute::OnCameraCapture (GUIControl* sender)
{
	Matrix mView = pRS->GetView ();
	value = mView.GetCamPos ();
	Clamp ();
	RealTimeValueX->Text = FloatToStr(value.x);
	RealTimeValueY->Text = FloatToStr(value.y);
	RealTimeValueZ->Text = FloatToStr(value.z);
	SetupMissionObject ();

}

void _cdecl PositionAttribute::OnZero (GUIControl* sender)
{
	value = Vector(0.0f);
	Clamp ();
	RealTimeValueX->Text = FloatToStr(value.x);
	RealTimeValueY->Text = FloatToStr(value.y);
	RealTimeValueZ->Text = FloatToStr(value.z);
	SetupMissionObject ();
}


void _cdecl PositionAttribute::OnCopy (GUIControl* sender)
{
	vVectorClipboard.x = fast_atof (RealTimeValueX->Text.GetBuffer());
	vVectorClipboard.y = fast_atof (RealTimeValueY->Text.GetBuffer());
	vVectorClipboard.z = fast_atof (RealTimeValueZ->Text.GetBuffer());
}

void _cdecl PositionAttribute::OnPaste (GUIControl* sender)
{
	value = vVectorClipboard;
	Clamp ();

	RealTimeValueX->Text = FloatToStr(vVectorClipboard.x);
	RealTimeValueY->Text = FloatToStr(vVectorClipboard.y);
	RealTimeValueZ->Text = FloatToStr(vVectorClipboard.z);

	SetupMissionObject ();
}

void PositionAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<position val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value_x val = \"%f\" />\n", value.x);
		file.Write(level+1, "<value_y val = \"%f\" />\n", value.y);
		file.Write(level+1, "<value_z val = \"%f\" />\n", value.z);
	file.Write(level, "</position>\n");
}

void PositionAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
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
			PositionAttribute* SrcAttr = (PositionAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	TiXmlElement* node = Root->FirstChildElement("value_x");
	if (node) value.x = fast_atof(node->Attribute("val"));
	node = Root->FirstChildElement("value_y");
	if (node) value.y = fast_atof (node->Attribute("val"));
	node = Root->FirstChildElement("value_z");
	if (node) value.z = fast_atof (node->Attribute("val"));

}