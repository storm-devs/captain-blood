#include "enumattr.h"
#include "..\..\missioneditor.h"
#include "..\AttributeList.h"


extern IGUIManager* igui;
extern MissionEditor* sMission;
#include "..\..\forms\globalParams.h"

EnumAttribute & EnumAttribute::operator = (EnumAttribute & source)
{
	values.Clear();
	for (int n = 0; n < source.GetEnumCount (); n++)
	{
		AddEnum (source.GetEnum (n));
	}
	if(values.Size() == 0)
	{
		values.Add("No elements");
	}
	SetName (source.GetName ());
	SetValue (source.GetValue ());
	BaseAttribute::Copy(*this, source);
	return *this;
}

EnumAttribute & EnumAttribute::operator = (const IMOParams::Enum& source)
{
	values.Clear();
	for (int n = 0; n < source.element; n++)
	{
		AddEnum (source.element[n]);
	}
	SetName (source.name);
	SetValue (0);
	return *this;
}


EnumAttribute::EnumAttribute ()
{
	EditedObject = NULL;
	cValues = NULL;


	curvalue = "";
	current_index = -1;
  Type = IMOParams::t_enum;
  values.Clear ();
}

EnumAttribute::~EnumAttribute ()
{
  values.Clear ();
}


void EnumAttribute::SetValue (int index)
{
	if (index < 0) index = 0;
	if(values.Size() > 0)
	{
		curvalue = values[index];
		current_index = index;
	}else{
		curvalue = "";
		current_index = index;
	}
}

int EnumAttribute::GetValue () const
{
	return current_index;
}
  
void EnumAttribute::AddToWriter (MOPWriter& wrt)
{
	//wrt.AddEnum ();
	wrt.AddEnum (curvalue);
}
  

const string& EnumAttribute::GetStringValue () const
{
	return curvalue;
}

void EnumAttribute::PopupEdit (int pX, int pY)
{
	//GUIWindow* wnd = igui->FindWindow (BOOLEDIT_WINDOWNAME);
	//if (wnd) igui->Close (wnd);

	Form = NEW TEnumEdit (0, 0);
	Form->SetPosition (pX, pY);
	//Form->cbValue->Checked = value;
	//Form->cbValue->Caption = this->GetName ();

	for (int n =0; n < values.Size (); n++)
	{
		Form->cbValue->ListBox->Items.Add (values[n]);
	}
	Form->cbValue->Edit->Text = curvalue;
	
	
  Form->MasterAttrib = this;


	igui->ShowModal (Form);

	// Обязательно нужно сделать...
	pForm = Form;


}

void EnumAttribute::AddEnum (const string& val)
{
	values.Add (val);
}

void EnumAttribute::Clear ()
{
	values.Clear ();
}

int EnumAttribute::GetEnumCount () const
{
	return values.Size ();
}

const string& EnumAttribute::GetEnum (int num)
{
	return values.GetConst(num);
}

void EnumAttribute::WriteToFile (IFile* pFile)
{
	DWORD written = 0;
	
	DWORD slen = strlen (GetName ());
	written = pFile->Write(&slen, sizeof (DWORD));
	Assert (written == sizeof (DWORD));
	written = pFile->Write(GetName (), slen);
	Assert (written == slen);


	written = pFile->Write(&current_index, sizeof (int));
	Assert (written == sizeof (int));


	
	DWORD val_count = values.Size ();
	written = pFile->Write(&val_count, sizeof (DWORD));
	Assert (written == sizeof (DWORD));

	for (DWORD n = 0; n < val_count; n++)
	{
		const char* c_value = values[n].GetBuffer ();
		DWORD c_value_len = strlen (c_value);
	
		written = pFile->Write(&c_value_len, sizeof (DWORD));
		Assert (written == sizeof (DWORD));
		
		written = pFile->Write(c_value, c_value_len);
		Assert (written == c_value_len);
	}

}

#pragma warning (disable : 4800)


void EnumAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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

	int ldCurIndex = -1;
	loaded = pFile->Read(&ldCurIndex, sizeof (int));
	Assert (loaded == sizeof (int));


	DWORD ValuesCount = 0;
	loaded = pFile->Read(&ValuesCount, sizeof (DWORD));
	Assert (loaded == sizeof (DWORD));


	for (DWORD n =0; n < ValuesCount; n++)
	{
		DWORD string_len = 0;
		loaded = pFile->Read(&string_len, sizeof (DWORD));
		Assert (loaded == sizeof (DWORD));

		char* c_value = NEW char[string_len+1];
		c_value[string_len] = 0;

		loaded = pFile->Read(c_value, string_len);
		Assert (loaded == string_len);

		AddEnum (c_value);
		DELETE(c_value);
	}


	string SelectedValue = values[ldCurIndex];
//------------------[  fucked bug fix ]-----------------------------

	Clear();
	int i_new_sel_idx = -1;

	int availCount = sMission->AvailableMO.Size ();
	for (int n = 0; n< availCount; n++)
	{
		MissionEditor::tAvailableMO* pMissionObject = &sMission->AvailableMO[n];
		// Если нашли нужный класс
		if (strcmp (pMissionObject->ClassName, ClassName) == 0)
		{
			BaseAttribute* SourceAttr = FindEnumInAttrList(pMissionObject->AttrList, Name);

				//Нашли нужный тип и нужное имя...
				if (SourceAttr != NULL)
				{
					EnumAttribute* SourceAttrT = (EnumAttribute*)SourceAttr;

					int Count = SourceAttrT->GetEnumCount();
					for (int q = 0; q < Count; q++)
					{
						const string& str = SourceAttrT->GetEnum(q);

						if (str == SelectedValue)
						{
							i_new_sel_idx = q;
						}
						
						AddEnum(str.c_str());
					}
					//SetMax (SourceAttrLong->GetMax());
					//SetMin (SourceAttrLong->GetMin());

					//if (value < min) value = min;
					//if (value > max) value = max;
				}
		}
	}




	if (i_new_sel_idx < 0) i_new_sel_idx = 0;
	
	SetValue (i_new_sel_idx);


}

void EnumAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GetName ();
	gp->__tmpText += "#c808080";

	string vl = GetStringValue();
	gp->__tmpText += string (" '");
	gp->__tmpText += vl;
	gp->__tmpText += string ("'"); 

	node->Image->Load("meditor\\enum");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText(gp->__tmpText);
}

void EnumAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode);
	nodes->Add(nNode);


}

/* Начать изменение аттрибута в реалтайме... */
void EnumAttribute::BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent)
{
	EditedObject = MissionObject;
	SavedValue = curvalue;
	SavedIndex = current_index;

	cValues = NEW GUIComboBox (parent, 10, 10, 220, 19);
	cValues->Edit->Text =curvalue;
	cValues->Edit->pFont->SetName("arialcyrsmall");
	cValues->ListBox->pFont->SetName("arialcyrsmall");
	cValues->MakeFlat (true);
	cValues->bUserCanTypeText = false;
	cValues->OnChange = (CONTROL_EVENT)&EnumAttribute::ValueChange;
	
	for (int n =0; n < values.Size (); n++)
	{
		cValues->ListBox->Items.Add (values[n]);
	}


	

}

/* Подтвердить изменение аттрибута... */
void EnumAttribute::ApplyRTEdit ()
{
	CloseRTEdit ();
}

/* Отменить изменения в аттрибуте... */
void EnumAttribute::CancelRTEdit ()
{
	curvalue = SavedValue;
	current_index = SavedIndex;
	SetupMissionObject ();
	CloseRTEdit ();
}

void EnumAttribute::CloseRTEdit ()
{
	if (cValues)
	{
		delete cValues;
		cValues = NULL;
	}
}


void EnumAttribute::SetupMissionObject ()
{
	if (!EditedObject) return;
	MOPWriter wrt(EditedObject->Level, EditedObject->pObject.Ptr()->GetObjectID().c_str());
	EditedObject->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(EditedObject->pObject.Ptr(), wrt);
#endif
	//EditedObject->pObject->EditMode_Update (wrt.Reader ());

}

void _cdecl EnumAttribute::ValueChange (GUIControl* sender)
{
	current_index = cValues->ListBox->SelectedLine;
	curvalue = values[current_index];
	SetupMissionObject ();
}

BaseAttribute* EnumAttribute::FindEnumInAttrList (AttributeList* attlist, const char *szAttrName)
{
	for (int n = 0; n < attlist->GetCount(); n++)
	{
		BaseAttribute* SourceAttr = attlist->Get(n);
		if (SourceAttr->GetType() == IMOParams::t_enum)
		{
			if (crt_stricmp (SourceAttr->GetName(), szAttrName) == 0)
			{
				return SourceAttr;
			}
		}

		if (SourceAttr->GetType() == IMOParams::t_array)
		{
			ArrayAttribute* ArrayAttr = (ArrayAttribute*) SourceAttr;

			BaseAttribute* pRes = FindInArray (ArrayAttr, szAttrName);
			if (pRes) return pRes;
		}
		
		if (SourceAttr->GetType() == IMOParams::t_group)
		{
			GroupAttribute* GroupAttr = (GroupAttribute*) SourceAttr;

			BaseAttribute* pRes = FindInGroup (GroupAttr, szAttrName);
			if (pRes) return pRes;
		}

	}


	return NULL;
}


BaseAttribute* EnumAttribute::FindInArray (ArrayAttribute* pArray, const char *szAttrName)
{
	
	for (DWORD i = 0; i < pArray->GetDefaultElement().elements.Size(); i++)
	{
		BaseAttribute* SourceAttr = pArray->GetDefaultElement().elements[i];

		if (SourceAttr->GetType() == IMOParams::t_enum)
		{
			if (crt_stricmp (SourceAttr->GetName(), szAttrName) == 0)
			{
				return SourceAttr;
			}
		}

		if (SourceAttr->GetType() == IMOParams::t_array)
		{
			ArrayAttribute* ArrayAttr = (ArrayAttribute*) SourceAttr;

			BaseAttribute* pRes = FindInArray (ArrayAttr, szAttrName);
			if (pRes) return pRes;
		}

		if (SourceAttr->GetType() == IMOParams::t_group)
		{
			GroupAttribute* GroupAttr = (GroupAttribute*) SourceAttr;

			BaseAttribute* pRes = FindInGroup (GroupAttr, szAttrName);
			if (pRes) return pRes;
		}

		
	}

	return NULL;
}

BaseAttribute* EnumAttribute::FindInGroup (GroupAttribute* pGroup, const char *szAttrName)
{
	for (DWORD i = 0; i < pGroup->Childs.Size(); i++)
	{
		BaseAttribute* SourceAttr = pGroup->Childs[i];

		if (SourceAttr->GetType() == IMOParams::t_enum)
		{
			if (crt_stricmp (SourceAttr->GetName(), szAttrName) == 0)
			{
				return SourceAttr;
			}
		}

		if (SourceAttr->GetType() == IMOParams::t_array)
		{
			ArrayAttribute* ArrayAttr = (ArrayAttribute*) SourceAttr;

			BaseAttribute* pRes = FindInArray (ArrayAttr, szAttrName);
			if (pRes) return pRes;
		}

		if (SourceAttr->GetType() == IMOParams::t_group)
		{
			GroupAttribute* GroupAttr = (GroupAttribute*) SourceAttr;

			BaseAttribute* pRes = FindInGroup (GroupAttr, szAttrName);
			if (pRes) return pRes;
		}

	}


	return NULL;
}


void EnumAttribute::WriteToXML (TextFile &file, int level)
{
	file.Write(level, "<enum val = \"%s\">\n", GetName ());
		file.Write(level+1, "<selected val = \"%s\" />\n", curvalue.c_str());
	file.Write(level, "</enum>\n");
}

void EnumAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
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
			EnumAttribute* SrcAttr = (EnumAttribute*)SourceAttr;
			*this = *SrcAttr;
		}
	}

	string EnumValue;
	TiXmlElement* node = Root->FirstChildElement("selected");
	if (node) EnumValue = node->Attribute("val");

	//Если можно выбираем загруженное значение из списка...
	for (int q = 0; q < GetEnumCount(); q++)
	{
		const string& str = GetEnum(q);
		if (str == EnumValue)
		{
			SetValue(q);
			break;
		}
	}

}