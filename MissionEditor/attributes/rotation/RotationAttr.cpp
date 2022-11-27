#include "RotationAttr.h"
#include "..\strutil.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"
#include "..\..\fast_atof.h"

extern IGUIManager* igui;
extern IRender * pRS;

// Ширина цифрового поля ввода
// в реалтаймовом редакторе...
#define WDTH 71

extern Vector vVectorClipboard;

#include "..\..\forms\globalParams.h"


RotationAttribute & RotationAttribute::operator = (const RotationAttribute & source)
{
	SetValue (source.GetValue());
	SetMax (source.GetMax());
	SetMin (source.GetMin());
	BaseAttribute::Copy(*this, source);
	return *this;
}

RotationAttribute & RotationAttribute::operator = (const IMOParams::Angles& source)
{
	SetValue (source.def);
	SetMax (source.max);
	SetMin (source.min);
	SetName (source.name);
	SetIsLimit (source.isLimit);
	return *this;

}

RotationAttribute::RotationAttribute ()
{
	RealTimeValueX = NULL;
	RealTimeValueY = NULL;
	RealTimeValueZ = NULL;
	RealTimeDesc = NULL;
	EditedObject = NULL;

	RealTimeDesc = NULL;
	EditedObject = NULL;

	btnCopyFrom = NULL;
	btnPasteTo = NULL;
	btnSetFromCamera = NULL;
	btnSetToZero = NULL;



 value = Vector (0.0f);
 min = Vector (0.0f);
 max = Vector (0.0f);
 
 Type = IMOParams::t_angles;


}

RotationAttribute::~RotationAttribute ()
{
}

void RotationAttribute::SetValue (const Vector& value)
{
	this->value = value;

	if (RealTimeValueX)	RealTimeValueX->Text = FloatToStr(Rad2Deg(value.x));
	if (RealTimeValueY)	RealTimeValueY->Text = FloatToStr(Rad2Deg(value.y));
	if (RealTimeValueZ)	RealTimeValueZ->Text = FloatToStr(Rad2Deg(value.z));

}

const Vector& RotationAttribute::GetValue () const
{
	return value;
}


void RotationAttribute::SetMax (const Vector& Max)
{
	max = Max;
}

const Vector& RotationAttribute::GetMax () const
{
	return max;
}


void RotationAttribute::SetMin (const Vector& Min)
{
 min = Min;
}

const Vector& RotationAttribute::GetMin () const
{
	return min;
}


  
void RotationAttribute::PopupEdit (int pX, int pY)
{
  //GUIWindow* wnd = igui->FindWindow (ROTATIONEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

  Form = NEW TRotationEdit (0, 0);	
	Form->SetPosition (pX, pY);
	Form->eValueX->Text = FloatToStr (Rad2Deg(value.x));
	Form->eValueY->Text = FloatToStr (Rad2Deg(value.y));
	Form->eValueZ->Text = FloatToStr (Rad2Deg(value.z));

	Form->eValueX->Hint = GetName ();
	Form->eValueX->Hint += "\nX angle";

	Form->eValueY->Hint = GetName ();
	Form->eValueY->Hint += "\nY angle";

	Form->eValueZ->Hint = GetName ();
	Form->eValueZ->Hint += "\nZ angle";

	if (GetIsLimit())
	{
		Form->eValueX->Hint += string ("\nmin : ") + string (FloatToStr(Rad2Deg(min.x)));
		Form->eValueX->Hint += string ("\nmax : ") + string (FloatToStr(Rad2Deg(max.x)));

		Form->eValueY->Hint += string ("\nmin : ") + string (FloatToStr(Rad2Deg(min.y)));
		Form->eValueY->Hint += string ("\nmax : ") + string (FloatToStr(Rad2Deg(max.y)));

		Form->eValueZ->Hint += string ("\nmin : ") + string (FloatToStr(Rad2Deg(min.z)));
		Form->eValueZ->Hint += string ("\nmax : ") + string (FloatToStr(Rad2Deg(max.z)));
	} 



	Form->MasterAttrib = this;

	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;


}


  
void RotationAttribute::AddToWriter (MOPWriter& wrt)
{
	wrt.AddAngles (value);
}
  
void RotationAttribute::WriteToFile (IFile* pFile)
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


void RotationAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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

void RotationAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	gp->__tmpText += string (" ");
	gp->__tmpText += string (FloatToStr(Rad2Deg(value.x))); 
	gp->__tmpText += string (", ");
	gp->__tmpText += string (FloatToStr(Rad2Deg(value.y))); 
	gp->__tmpText += string (", ");
	gp->__tmpText +=  string (FloatToStr(Rad2Deg(value.z))); 

	node->Image->Load("meditor\\rot");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText( gp->__tmpText);

}

void RotationAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode);

	nodes->Add(nNode);
}

/* Начать изменение аттрибута в реалтайме... */
void RotationAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	//RealTimeValueX->OnAdvNumChange = (CONTROL_EVENT)OnRTAdvChange;
	//RealTimeValueX->OnAccept = (CONTROL_EVENT)OnRTValueChange;


	EditedObject = MissionObject;
	SavedValue = value;

	RealTimeValueX = NEW GUIEdit (parent, 10, 10, WDTH, 19);
	RealTimeValueX->Text = FloatToStr(Rad2Deg(value.x));
	RealTimeValueX->pFont->SetName("arialcyrsmall");
	RealTimeValueX->Hint = "Drag value to smooth change";
	RealTimeValueX->OnAdvNumChange = (CONTROL_EVENT)&RotationAttribute::OnRTAdvChangeX;
	RealTimeValueX->OnAccept = (CONTROL_EVENT)&RotationAttribute::OnRTValueChangeX;


	RealTimeValueY = NEW GUIEdit (parent, 10+WDTH+5, 10, WDTH, 19);
	RealTimeValueY->Text = FloatToStr(Rad2Deg(value.y));
	RealTimeValueY->pFont->SetName("arialcyrsmall");
	RealTimeValueY->Hint = "Drag value to smooth change";
	RealTimeValueY->OnAdvNumChange = (CONTROL_EVENT)&RotationAttribute::OnRTAdvChangeY;
	RealTimeValueY->OnAccept = (CONTROL_EVENT)&RotationAttribute::OnRTValueChangeY;


	RealTimeValueZ = NEW GUIEdit (parent, 10+(WDTH*2)+5+5, 10, WDTH, 19);
	RealTimeValueZ->Text = FloatToStr(Rad2Deg(value.z));
	RealTimeValueZ->pFont->SetName("arialcyrsmall");
	RealTimeValueZ->Hint = "Drag value to smooth change";
	RealTimeValueZ->OnAdvNumChange = (CONTROL_EVENT)&RotationAttribute::OnRTAdvChangeZ;
	RealTimeValueZ->OnAccept = (CONTROL_EVENT)&RotationAttribute::OnRTValueChangeZ;

	btnSetFromCamera = NEW GUIButton (parent, 10, 150, 32, 32);
	btnSetFromCamera->Glyph->Load("meditor\\big_camera");
	btnSetFromCamera->Hint = "Capture param from camera";
	btnSetFromCamera->FlatButton = true;
	btnSetFromCamera->OnMouseDown = (CONTROL_EVENT)&RotationAttribute::OnCameraCapture;


	btnSetToZero = NEW GUIButton (parent, 52, 150, 32, 32);
	btnSetToZero->Glyph->Load("meditor\\big_zero");
	btnSetToZero->Hint = "Zero param";
	btnSetToZero->FlatButton = true;
	btnSetToZero->OnMouseDown = (CONTROL_EVENT)&RotationAttribute::OnZero;

	btnCopyFrom = NEW GUIButton (parent, 52+42, 150, 32, 32);
	btnCopyFrom->Glyph->Load("meditor\\big_copy");
	btnCopyFrom->Hint = "Copy";
	btnCopyFrom->FlatButton = true;
	btnCopyFrom->OnMouseDown = (CONTROL_EVENT)&RotationAttribute::OnCopy;

	btnPasteTo = NEW GUIButton (parent, 52+42+42, 150, 32, 32);
	btnPasteTo->Glyph->Load("meditor\\big_paste");
	btnPasteTo->Hint = "Paste";
	btnPasteTo->FlatButton = true;
	btnPasteTo->OnMouseDown = (CONTROL_EVENT)&RotationAttribute::OnPaste;




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
		RealTimeDesc->Caption += FloatToStr (Rad2Deg(min.x));
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (Rad2Deg(min.y));
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (Rad2Deg(min.z));
		RealTimeDesc->Caption += "\n@cMax: #cFFFFFF";
		RealTimeDesc->Caption += FloatToStr (Rad2Deg(max.x));
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (Rad2Deg(max.y));
		RealTimeDesc->Caption += ", ";
		RealTimeDesc->Caption += FloatToStr (Rad2Deg(max.z));
	}

	RealTimeDesc->Layout = GUILABELLAYOUT_Left;


}

/* Подтвердить изменение аттрибута... */
void RotationAttribute::ApplyRTEdit ()
{
	CloseRTEdit ();
}

/* Отменить изменения в аттрибуте... */
void RotationAttribute::CancelRTEdit ()
{
	value = SavedValue;
	SetupMissionObject ();
	CloseRTEdit ();
}

void RotationAttribute::CloseRTEdit ()
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

void RotationAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());
}


void RotationAttribute::Clamp ()
{
	ClampAndLimit (value.x, min.x, max.x, IsLimit);
	ClampAndLimit (value.y, min.y, max.y, IsLimit);
	ClampAndLimit (value.z, min.z, max.z, IsLimit);
	if (RealTimeValueX)	RealTimeValueX->Text = FloatToStr(Rad2Deg(value.x));
	if (RealTimeValueY)	RealTimeValueY->Text = FloatToStr(Rad2Deg(value.y));
	if (RealTimeValueZ)	RealTimeValueZ->Text = FloatToStr(Rad2Deg(value.z));
}

void _cdecl RotationAttribute::OnRTValueChangeX (GUIControl* sender)
{
	value.x = Deg2Rad(fast_atof (RealTimeValueX->Text.GetBuffer()));
	Clamp ();
	RealTimeValueX->Text = FloatToStr(Rad2Deg(value.x));
	SetupMissionObject ();
}

// Вано изменил, было: offset / 100.0f, стало: offset / 600.0f
// А то углы прокручивались до 30 единиц даже при 50 фпсах
void _cdecl RotationAttribute::OnRTAdvChangeX (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;
	value.x -= (offset / 600.0f);

	Clamp ();
	RealTimeValueX->Text = FloatToStr(Rad2Deg(value.x));
	SetupMissionObject ();
}

void _cdecl RotationAttribute::OnRTValueChangeY (GUIControl* sender)
{
	value.y = Deg2Rad(fast_atof (RealTimeValueY->Text.GetBuffer()));
	Clamp ();
	RealTimeValueY->Text = FloatToStr(Rad2Deg(value.y));
	SetupMissionObject ();
}

void _cdecl RotationAttribute::OnRTAdvChangeY (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;
	value.y -= (offset / 600.0f);

	Clamp ();
	RealTimeValueY->Text = FloatToStr(Rad2Deg(value.y));
	SetupMissionObject ();

}

void _cdecl RotationAttribute::OnRTValueChangeZ (GUIControl* sender)
{
	value.z = Deg2Rad(fast_atof (RealTimeValueZ->Text.GetBuffer()));
	Clamp ();
	RealTimeValueZ->Text = FloatToStr(Rad2Deg(value.z));
	SetupMissionObject ();
}

void _cdecl RotationAttribute::OnRTAdvChangeZ (GUIControl* sender)
{
	igui->SetKeyboardFocus(NULL);
	float offset = igui->GetCursor()->DeltaY;
	value.z -= (offset / 600.0f);

	Clamp ();
	RealTimeValueZ->Text = FloatToStr(Rad2Deg(value.z));
	SetupMissionObject ();
}

void RotationAttribute::ClampAndLimit (float& angle, float minLimit, float maxLimit, bool IsLimit)
{
	float DegAng = Rad2Deg(angle);

	// clamp angles
	int cX = (int)DegAng / 360;
	float ostX = (DegAng  - (cX * 360));
	if (ostX < 0) ostX = 360.0f + ostX;
	// clamp angles

	DegAng = ostX;
	angle = Deg2Rad (DegAng);


	if (IsLimit)
	{
		if (angle < minLimit) angle = minLimit;
		if (angle > maxLimit) angle = maxLimit;
	}

}

void _cdecl RotationAttribute::OnCameraCapture (GUIControl* sender)
{
	Matrix mView = pRS->GetView ();
	mView.Inverse();
	value = mView.GetAngles();
	Clamp ();
	RealTimeValueX->Text = FloatToStr(value.x);
	RealTimeValueY->Text = FloatToStr(value.y);
	RealTimeValueZ->Text = FloatToStr(value.z);
	SetupMissionObject ();

}

void _cdecl RotationAttribute::OnZero (GUIControl* sender)
{
	value = Vector(0.0f);
	Clamp ();
	RealTimeValueX->Text = FloatToStr(value.x);
	RealTimeValueY->Text = FloatToStr(value.y);
	RealTimeValueZ->Text = FloatToStr(value.z);
	SetupMissionObject ();
}


void _cdecl RotationAttribute::OnCopy (GUIControl* sender)
{
	vVectorClipboard.x = fast_atof (RealTimeValueX->Text.GetBuffer());
	vVectorClipboard.y = fast_atof (RealTimeValueY->Text.GetBuffer());
	vVectorClipboard.z = fast_atof (RealTimeValueZ->Text.GetBuffer());
}

void _cdecl RotationAttribute::OnPaste (GUIControl* sender)
{
	value = vVectorClipboard;

	value.x = Deg2Rad(value.x);
	value.y = Deg2Rad(value.y);
	value.z = Deg2Rad(value.z);

	RealTimeValueX->Text = FloatToStr(vVectorClipboard.x);
	RealTimeValueY->Text = FloatToStr(vVectorClipboard.y);
	RealTimeValueZ->Text = FloatToStr(vVectorClipboard.z);

	Clamp ();


	SetupMissionObject ();
}

void RotationAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<rotation val = \"%s\">\n", GetName ());
		file.Write(level+1, "<value_x val = \"%f\" />\n", value.x);
		file.Write(level+1, "<value_y val = \"%f\" />\n", value.y);
		file.Write(level+1, "<value_z val = \"%f\" />\n", value.z);
	file.Write(level, "</rotation>\n");
}

void RotationAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
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
			RotationAttribute* SrcAttr = (RotationAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	TiXmlElement* node = Root->FirstChildElement("value_x");
	if (node) value.x = fast_atof (node->Attribute("val"));
	node = Root->FirstChildElement("value_y");
	if (node) value.y = fast_atof (node->Attribute("val"));
	node = Root->FirstChildElement("value_z");
	if (node) value.z = fast_atof (node->Attribute("val"));
}