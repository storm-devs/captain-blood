#include "AttributeList.h"
#include "strutil.h"
#include "..\missioneditor.h"
#include "attrcreator.h"
#include "..\textfile.h"
#include "..\..\common_h\tinyxml\tinyxml.h"

extern MissionEditor* sMission;

AttributeList::AttributeList () : Storage (_FL_, 128)
{
  Clear ();
}

AttributeList::~AttributeList ()
{
  for (DWORD n = 0; n < Storage.Size (); n++)
  {
   BaseAttribute* pNew = Storage[n];

	 DeleteAttribute(pNew);

   //delete (pNew);
  }
  Clear ();
}
  
//int AttributeList::Add (BaseAttribute* pPtr)
  
void AttributeList::Clear ()
{
 Storage.DelAll ();
}
  
//int AttributeList::GetCount ()

int AttributeList::GetCountForSave ()
{
  int cnt = 0;

  for (DWORD i = 0; i < Storage.Size(); i++)
  {
    IMOParams::Type tp = Storage[i]->GetType();
    if (tp == IMOParams::t_group)
    {
      cnt += GetAttributesInGroup ((GroupAttribute*)Storage[i]);
      cnt++;
    } else
    {
      cnt++;
    }
  }

  return cnt;
}
  
int AttributeList::GetAttributesInGroup(GroupAttribute* pattr)
{
  int cnt = 0;
  for (dword i = 0; i < pattr->Childs.Size(); i++)
  {
    IMOParams::Type tp = pattr->Childs[i]->GetType();
    if (tp == IMOParams::t_group)
    {
      cnt += GetAttributesInGroup ((GroupAttribute*)pattr->Childs[i]);
      cnt++;
    } else
    {
      cnt++;
    }

  }
  
  return cnt;
}


//BaseAttribute* AttributeList::Get(int index)
  
//void AttributeList::Insert (int index, BaseAttribute* pPtr)
  
//void AttributeList::Delete (int index)
  
//BaseAttribute* AttributeList::operator[] (int index)


BaseAttribute* AttributeList::CreateFromParam (const IMOParams::Param* temp)
{
  BaseAttribute* newAttr = NULL;
  newAttr = CreateFromMissionParam (temp);
  return newAttr;
}

void AttributeList::CreateFromParams (IMOParams* params)
{
  int num = params->GetNumParams ();
  if (num <= 0) return;

  for (int n = 0; n < num; n++)
  {
    const IMOParams::Param* temp = params->GetParam (n);
    //api->Trace ("%s - %d", temp->name, temp->type);

    if (temp->type == 11)
    {
      int a = 0;
    }
    BaseAttribute* battr = CreateFromParam (temp);
    if (battr) Add (battr);
  }

}





void AttributeList::AddToWriter (MOPWriter& wrt)
{
  for (int n =0; n < GetCount (); n++)
  {
    BaseAttribute* base_attr = Get (n);
    base_attr->AddToWriter (wrt);
  }
}


void AttributeList::AddToTree (GUITreeNode* node, TreeNodesPool* nodesPool)
{
  for (int n = 0; n < GetCount (); n++)
  {
    BaseAttribute* base_attr = Get (n);
    base_attr->Add2Tree(&node->Childs, nodesPool);
  }
}


void AttributeList::AddParam (BaseAttribute* attr)
{
//----------------------------------------------------------
  BaseAttribute* newAttr = CreateAttributeCopy (attr);
  if (newAttr) Add (newAttr);
//----------------------------------------------------------
}

void AttributeList::CreateFromList (AttributeList* list)
{
  int num = list->GetCount ();
  if (num <= 0) return;

  for (int n = 0; n < num; n++)
  {
    BaseAttribute* attr = list->Get(n);
    AddParam (attr);
  }
}

void AttributeList::BeforeDelete ()
{
  int num = GetCount ();
  if (num <= 0) return;

  for (int n = 0; n < num; n++)
  {
    BaseAttribute* attr = Get(n);
    attr->BeforeDelete();
  }
}

void AttributeList::SetMasterData (void* data)
{
  for (int n = 0; n < GetCount(); n++)
  {
    Get(n)->SetMasterData (data);
  }
}


BaseAttribute* AttributeList::FindInArray (ArrayAttribute* pArray, const char *szAttrName, IMOParams :: Type Type)
{
  for (DWORD i = 0; i < pArray->GetDefaultElement().elements.Size(); i++)
  {
    BaseAttribute* SourceAttr = pArray->GetDefaultElement().elements[i];

    if (SourceAttr->GetType() == Type)
    {
      if (crt_stricmp (SourceAttr->GetName(), szAttrName) == 0)
      {
        return SourceAttr;
      }
    }

    if (SourceAttr->GetType() == IMOParams::t_array)
    {
      ArrayAttribute* ArrayAttr = (ArrayAttribute*) SourceAttr;

      BaseAttribute* pRes = FindInArray (ArrayAttr, szAttrName, Type);
      if (pRes) return pRes;
    }

    if (SourceAttr->GetType() == IMOParams::t_group)
    {
      GroupAttribute* GroupAttr = (GroupAttribute*) SourceAttr;

      BaseAttribute* pRes = FindInGroup (GroupAttr, szAttrName, Type);
      if (pRes) return pRes;
    }


  }

  return NULL;
}

BaseAttribute* AttributeList::FindInGroup (GroupAttribute* pGroup, const char *szAttrName, IMOParams :: Type Type)
{
  for (DWORD i = 0; i < pGroup->Childs.Size(); i++)
  {
    BaseAttribute* SourceAttr = pGroup->Childs[i];

    if (SourceAttr->GetType() == Type)
    {
      if (crt_stricmp (SourceAttr->GetName(), szAttrName) == 0)
      {
        return SourceAttr;
      }
    }

    if (SourceAttr->GetType() == IMOParams::t_array)
    {
      ArrayAttribute* ArrayAttr = (ArrayAttribute*) SourceAttr;

      BaseAttribute* pRes = FindInArray (ArrayAttr, szAttrName, Type);
      if (pRes) return pRes;
    }

    if (SourceAttr->GetType() == IMOParams::t_group)
    {
      GroupAttribute* GroupAttr = (GroupAttribute*) SourceAttr;

      BaseAttribute* pRes = FindInGroup (GroupAttr, szAttrName, Type);
      if (pRes) return pRes;
    }

  }


  return NULL;
}



const char* AttributeList::GetTextType (IMOParams::Type type)
{
  switch (type)
  {
  case IMOParams::t_bool:
    return "bool";
  case IMOParams::t_long:
    return "long";
  case IMOParams::t_float:
    return "float";
  case IMOParams::t_string:
    return "string";
  case IMOParams::t_locstring:
    return "locstring";
  case IMOParams::t_position:
    return "position";
  case IMOParams::t_angles:
    return "rotation";
  case IMOParams::t_color:
    return "color";
  case IMOParams::t_array:
    return "array";
  case IMOParams::t_enum:
    return "enum";
  case IMOParams::t_group:
    return "group";
  }

  return "unknown";
}

void AttributeList::DebugLog ()
{
  for (dword i = 0; i < Storage.Size(); i++)
  {
    IMOParams::Type t = Storage[i]->GetType();
    
    api->Trace("%s - %s", Storage[i]->GetName(), GetTextType(t));

    if (t == IMOParams::t_array)
    {
      ArrayAttribute* pArray = (ArrayAttribute*)Storage[i];
      pArray->DebugLog (0);

    }

    if (t == IMOParams::t_group)
    {
      GroupAttribute* pGroup = (GroupAttribute*)Storage[i];
      pGroup->DebugLog (0);

    }
  }
}


void AttributeList::GenerateFlatList(array<BaseAttribute*> &FlatList)
{
  for (dword i = 0; i < Storage.Size(); i++)
  {
    IMOParams::Type t = Storage[i]->GetType();

    if (t == IMOParams::t_group)
    {
      GroupAttribute* pGroup = (GroupAttribute*)Storage[i];
      pGroup->GenerateFlatList(FlatList);
      continue;
    }

    FlatList.Add(Storage[i]);
  }

}

void AttributeList::WriteToXML (TextFile &file, int level)
{
  file.Write(level, "<attributes>\n");

  for (int n =0; n < GetCount (); n++)
  {
    BaseAttribute* base_attr = Get (n);
    base_attr->WriteToXML (file, level+1);
  }


  file.Write(level, "</attributes>\n");
}

IMOParams::Type AttributeList::GetTypeFromXML (TiXmlElement* Root)
{
	const char* v = Root->Value();

	if (v[0] == 'b') return IMOParams::t_bool;
	
	
	if (v[0] == 'l' && v[1] == 'o' && v[2] == 'n') return IMOParams::t_long;
	if (v[0] == 'l' && v[1] == 'o' && v[2] == 'c') return IMOParams::t_locstring;

	if (v[0] == 'f') return IMOParams::t_float;
	if (v[0] == 'p') return IMOParams::t_position;
	if (v[0] == 'r') return IMOParams::t_angles;
	if (v[0] == 's') return IMOParams::t_string;
	
	if (v[0] == 'c') return IMOParams::t_color;
	if (v[0] == 'e') return IMOParams::t_enum;
	if (v[0] == 'a') return IMOParams::t_array;


/*	
  if (crt_stricmp(Root->Value(),"bool") == 0) return IMOParams::t_bool;
  if (crt_stricmp(Root->Value(),"long") == 0) return IMOParams::t_long;
  if (crt_stricmp(Root->Value(),"float") == 0) return IMOParams::t_float;
  if (crt_stricmp(Root->Value(),"position") == 0) return IMOParams::t_position;
  if (crt_stricmp(Root->Value(),"rotation") == 0) return IMOParams::t_angles;
  if (crt_stricmp(Root->Value(),"string") == 0) return IMOParams::t_string;
  if (crt_stricmp(Root->Value(),"locstring") == 0) return IMOParams::t_locstring;
  if (crt_stricmp(Root->Value(),"color") == 0) return IMOParams::t_color;
  if (crt_stricmp(Root->Value(),"enum") == 0) return IMOParams::t_enum;
  if (crt_stricmp(Root->Value(),"array") == 0) return IMOParams::t_array;
*/
  throw ("unknown type");
  return IMOParams::t_bool;

}

void AttributeList::ReadXML (TiXmlElement* Root, const char* szMasterClass)
{
	MissionEditor::tAvailableMO* objDesc = sMission->GetAvailableClassByName(szMasterClass);



  for(TiXmlElement* child = Root->FirstChildElement(); child; child = child->NextSiblingElement())
  {
    IMOParams::Type Type = GetTypeFromXML(child);

    BaseAttribute* newAttr = CreateEmptyAttribute ((IMOParams::Type)Type);

    newAttr->ReadXML (child, szMasterClass);
    //newAttr->LoadFromFile(pFile, ClassName);
    Add (newAttr);

		MissionEditor::tAvailableMO* objDesc = sMission->GetAvailableClassByName (szMasterClass);

		if (objDesc)
		{
			//New
			const IMOParams::Param* temp = objDesc->Params->FindParam(newAttr->GetName());
			if(temp)
			{
				newAttr->SetHint(temp->comment);
			}

			/*
			//Old
			long iNumParams = objDesc->Params->GetNumParams();
			for (long j = 0; j < iNumParams; j++)
			{
				const IMOParams::Param* temp = objDesc->Params->GetParam (j);

				if (crt_stricmp(newAttr->GetName(), temp->name) == 0)
				{
					newAttr->SetHint(temp->comment);
					break;
				}
			}
			*/
		}
		


  }
}