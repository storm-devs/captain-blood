#include "arrayattr.h"
#include "..\strutil.h"

#include "..\bool\BoolAttr.h"
#include "..\long\LongAttr.h"
#include "..\float\FloatAttr.h"
#include "..\position\positionAttr.h"
#include "..\Rotation\RotationAttr.h"
#include "..\String\StringAttr.h"
#include "..\Color\ColorAttr.h"
#include "..\Enumerate\EnumAttr.h"
#include "..\attrcreator.h"

#include "..\..\missioneditor.h"


#include "..\..\forms\globalParams.h"

extern IGUIManager* igui;
extern MissionEditor* sMission;


ArrayAttribute & ArrayAttribute::operator = (ArrayAttribute & source)
{
  CleanUp ();
  int e_count = source.GetElementsCount();

  /* Копируем элементы массива */
  for (int n = 0; n < e_count; n++)
  {
    BaseAttribute* element = source.GetElement(n);
    BaseAttribute* copy = CreateAttributeCopy (element);
    AddElement(copy);
  }

  /* Установить макс/мин значения */
  SetMaxElementCount(source.GetMaxElementCount());
  SetMinElementCount(source.GetMinElementCount());

  /* Копируем значения массива */
  int val_count = source.GetValuesCount();
  for (int i = 0; i < val_count; i++)
  {
    ArrayElement* element = source.GetValue(i);
    ArrayElement newElement;
    Copy (newElement, *element);
    ArrayValues.Add(newElement);
    newElement.elements.DelAll();
  }
  
 /* копируем базовые аттрибуты */
  BaseAttribute::Copy(*this, source);


  return *this;
}

ArrayAttribute & ArrayAttribute::operator = (const IMOParams::Array& source)
{
  CleanUp ();
  SetName (source.name);
  SetIsLimit (source.isLimit);
  int count = source.element.Size();
  for (int n = 0; n < count; n++)
  {
    IMOParams::Param* c_param = source.element[n];
    BaseAttribute* base_attr = CreateFromMissionParam (c_param);
    if (base_attr) AddElement(base_attr);
  }

  SetMaxElementCount(source.max);
  SetMinElementCount(source.min);

  Resize (min);
  return *this;
}

ArrayAttribute::ArrayAttribute () : ArrayValues (_FL_, 128)
{
  RootNode = NULL;
  RootExpanded = false;
  sysDelete = false;
  Type = IMOParams::t_array;
  min = -1;
  max = -1;
}

ArrayAttribute::~ArrayAttribute ()
{
  CleanUp ();
}

BaseAttribute* ArrayAttribute::GetElement (int index)
{
  return defaultelement.elements[index];
}

void ArrayAttribute::AddElement (BaseAttribute* value)
{
  defaultelement.elements.Add(value);
}


int ArrayAttribute::GetElementsCount ()
{
  return defaultelement.elements.Size();
}

int ArrayAttribute::GetMinElementCount ()
{
  return min;
}

int ArrayAttribute::GetMaxElementCount ()
{
  return max;
}

void ArrayAttribute::SetMinElementCount (int min)
{
  this->min = min;
}

void ArrayAttribute::SetMaxElementCount (int max)
{
  this->max = max;
}

void ArrayAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	tmpText = GetName ();
	tmpText += " #c808080";
	tmpText += IntToStr(ArrayValues.Size());
	tmpText += " items";

	node->Image->Load("meditor\\array");
	node->Tag = TAG_ATTRIBUTE;
	node->Expanded = RootExpanded;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	node->SetText(tmpText);
}

void ArrayAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
  
  GUITreeNode* nNode = nodesPool->CreateNode();
  RootNode = nNode;

  UpdateTree(nNode);

  nodes->Add(nNode);


  

  string t;

  int ArrayValuesSize = ArrayValues.Size ();
  for (int n = 0; n < ArrayValuesSize; n++)
  {
    int el_num = ArrayValues[n].elements.Size();

    ArrayValues[n].info.array = this;
    ArrayValues[n].info.element = &ArrayValues[n];

    
    ArrayValues[n].itemNode = nodesPool->CreateNode();
    ArrayValues[n].itemNode->Tag = TAG_ARRAYITEM;
    ArrayValues[n].itemNode->Expanded = ArrayValues[n].Expanded;
    ArrayValues[n].itemNode->CanDrag = false;
    ArrayValues[n].itemNode->CanDrop = false;
    ArrayValues[n].itemNode->CanCopy = false;
    ArrayValues[n].itemNode->Data = &ArrayValues[n].info;


	t = "";
    for (int i = 0; i < el_num; i++)
    {
      BaseAttribute* attr = ArrayValues[n].elements[i];
      attr->Add2Tree(&ArrayValues[n].itemNode->Childs, nodesPool, &t);
    }

	if (t.IsEmpty())
	{
		gp->__tmpText.Format("item %d", n);
	} else
	{
		gp->__tmpText.Format("item %d (%s)", n, t.c_str());

		if (gp->__tmpText.Len() >= (TREE_VIEW_ITEM_LEN-10))
		{
			gp->__tmpText.Delete((TREE_VIEW_ITEM_LEN-10), gp->__tmpText.Size()- (TREE_VIEW_ITEM_LEN-10));
			gp->__tmpText += "...";
		}

	}


	

	ArrayValues[n].itemNode->SetText(gp->__tmpText.c_str());
	nNode->Childs.Add(ArrayValues[n].itemNode);


  }

}

void ArrayAttribute::PopupEdit (int pX, int pY)
{
  Form = NEW TArrayEdit (0, 0);
  Form->SetPosition (pX, pY);
  Form->MasterAttrib = this;


  Form->eValue->Text = IntToStr (ArrayValues.Size());
  Form->eValue->Hint = GetName ();
  Form->eValue->Hint += "\n----------------------";
  Form->eValue->Hint += string ("\nmin :") + IntToStr (min);
  Form->eValue->Hint += string ("\nmax :") + IntToStr (max);
  Form->eValue->Hint += "\n----------------------";
  for (int n = 0; n < defaultelement.elements; n++)
  {
    IMOParams::Type type = defaultelement.elements[n]->GetType();
    string text = "undefined";
    switch (type)
    {
      case IMOParams::t_bool:
        text = "bool";
        break;
      case IMOParams::t_long:
        text = "long";
        break;
      case IMOParams::t_float:
        text = "float";
        break;
      case IMOParams::t_string:
        text = "string";
        break;
      case IMOParams::t_locstring:
        text = "locstring";
        break;
      case IMOParams::t_position:
        text = "position";
        break;
      case IMOParams::t_angles:
        text = "angles";
        break;
      case IMOParams::t_color:
        text = "color";
        break;
      case IMOParams::t_array:
        text = "array";
        break;
      case IMOParams::t_enum:
        text = "enum";
        break;
    }

    text += string (" ");
    text += defaultelement.elements[n]->GetName ();
    Form->eValue->Hint += string ("\n") + text;
  }

  igui->ShowModal (Form);
  // Обязательно нужно сделать...
  pForm = Form;
}



void ArrayAttribute::Copy (ArrayElement &to, ArrayElement &from)
{
  for (DWORD n = 0; n < from.elements.Size(); n++)
  {
    BaseAttribute* added_attr = NULL;
    added_attr = CreateAttributeCopy (from.elements[n]);
    to.elements.Add(added_attr);
  }

  to.info.array = from.info.array;
  to.info.element = &to;
}

void ArrayAttribute::Resize (int NewSize)
{
  if (!sysDelete)
  {
    if (NewSize < min) NewSize = min;
    if (NewSize > max) NewSize = max;
  }

  if (NewSize < 0) return;
  
  if ((int)ArrayValues.Size() == NewSize) return;

  int OldSize = ArrayValues.Size();

  // Надо убить массив...
  if (NewSize == 0)
  {
    int arraySize = ArrayValues.Size();
    
    for (int n = 0; n < arraySize; n++)
    {
      int el_num = ArrayValues[0].elements.Size();
      for (int i = 0; i < el_num; i++)
      {
        BaseAttribute* attr = ArrayValues[0].elements[i];
				DeleteAttribute(attr);
        //delete ();
      }
      ArrayValues.ExtractNoShift(0);
    }

    SetMasterData (MasterData);
    return;
  }

  // Уменьшить кол-во элементов
  if (OldSize > NewSize)
  {
    int DelCount = OldSize-NewSize;

    for (int n = 0; n < DelCount; n++)
    {
      int last = ArrayValues.Size() - 1;
      int el_num = ArrayValues[last].elements.Size();
      for (int i = 0; i < el_num; i++)
      {
        DeleteAttribute(ArrayValues[last].elements[i]);
      }
      ArrayValues.DelIndex(last);
    }

    SetMasterData (MasterData);
    return;
  }


  // Если нужно увеличить кол-во элементов
  if (OldSize < NewSize)
  {
    int AddElements = (NewSize - OldSize);

    ArrayElement NewElement;
    for (int n = 0; n < AddElements; n++)
    {
      Copy (NewElement, defaultelement);
      ArrayValues.Add(NewElement);
      NewElement.elements.DelAll();
    }
    SetMasterData (MasterData);
    return;
  }

}


void ArrayAttribute::BeforeDelete ()
{
  for (int n = 0; n < ArrayValues; n++)
  {
    GUITreeNode* node = ArrayValues[n].itemNode;
    if (node)
    {
      ArrayValues[n].Expanded = node->Expanded;
      ArrayValues[n].itemNode = NULL;
    }

    int sz = ArrayValues[n].elements.Size();
    for (int j = 0; j < sz; j++)
    {
      ArrayValues[n].elements[j]->BeforeDelete();
    }

    
  }


  if (RootNode)
  {
    RootExpanded = RootNode->Expanded;
    RootNode = NULL;
  }
}

void ArrayAttribute::CleanUp ()
{
  sysDelete = true;
  Resize (0);
  sysDelete = false;

  for (int n = 0; n < defaultelement.elements; n++)
  {
    //delete ;
		DeleteAttribute(defaultelement.elements[n]);
  }
  defaultelement.elements.DelAll ();
}

int ArrayAttribute::GetValuesCount ()
{
  return ArrayValues.Size();
}

ArrayAttribute::ArrayElement* ArrayAttribute::GetValue (int index)
{
  return &ArrayValues[index];
}


void ArrayAttribute::SetMasterData (void* data)
{
  for (int n = 0; n < ArrayValues; n++)
  {
    int el_count = ArrayValues[n].elements.Size();
    for (int i = 0; i < el_count; i++)
    {
      ArrayValues[n].elements[i]->SetMasterData(data);
    }
  }

  BaseAttribute::SetMasterData (data);
}


void ArrayAttribute::AddToWriter (MOPWriter& wrt)
{
  wrt.AddArray(ArrayValues.Size());
  for (int n = 0; n < ArrayValues; n++)
  {
    int el_count = ArrayValues[n].elements.Size();
    for (int i = 0; i < el_count; i++)
    {
      BaseAttribute* base = ArrayValues[n].elements[i];
      base->AddToWriter(wrt);
    }
  }
}


void ArrayAttribute::WriteToFile (IFile* pFile)
{
  DWORD written = 0;

  DWORD slen = strlen (GetName ());
  written = pFile->Write(&slen, sizeof (DWORD));
  Assert (written == sizeof (DWORD));
  written = pFile->Write(GetName (), slen);
  Assert (written == slen);

  /* Записываем общее кол-во элементов */
  DWORD save_val = ArrayValues.Size();
  written = pFile->Write(&save_val, sizeof (DWORD));
  Assert (written == sizeof (DWORD));

  for (int n = 0; n < ArrayValues; n++)
  {
    int el_count = ArrayValues[n].elements.Size();

    /* Записываем кол-во элементов в структуре */
    DWORD save_val2 = el_count;
    written = pFile->Write(&save_val2, sizeof (DWORD));
    Assert (written == sizeof (DWORD));

    /* Записываем структуру */
    for (int i = 0; i < el_count; i++)
    {
      BaseAttribute* base = ArrayValues[n].elements[i];
      //base
      IMOParams::Type t = base->GetType ();
      DWORD dwType = (DWORD)t;
      written = pFile->Write(&dwType, sizeof (DWORD));
      Assert (written == sizeof (DWORD));

      base->WriteToFile(pFile);
    }
  }


}

void ArrayAttribute::LoadFromFile (IFile* pFile, const char* ClassName)
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


  DWORD array_size = 0;
  loaded = pFile->Read(&array_size, sizeof (DWORD));
  Assert (loaded == sizeof (DWORD));
  

  for (DWORD i = 0; i < array_size; i++)
  {
    DWORD elements_count = 0;
    loaded = pFile->Read(&elements_count, sizeof (DWORD));
    Assert (loaded == sizeof (DWORD));


    ArrayElement newElement;

    for (DWORD j = 0; j < elements_count; j++)
    {
      DWORD dwType = 0;
      loaded = pFile->Read(&dwType, sizeof (DWORD));
      Assert (loaded == sizeof (DWORD));

      BaseAttribute* nAttr = CreateEmptyAttribute ((IMOParams::Type)dwType);
      nAttr->LoadFromFile(pFile, ClassName);

      BaseAttribute* added_attr = CreateAttributeCopy (nAttr);
      newElement.elements.Add(added_attr);

      DeleteAttribute(nAttr);

    }

    ArrayValues.Add(newElement);
    newElement.elements.DelAll();
  
  }




  int availCount = sMission->AvailableMO.Size ();
  for (int n = 0; n< availCount; n++)
  {
    MissionEditor::tAvailableMO* pMissionObject = &sMission->AvailableMO[n];
    // Если нашли нужный класс
    if (strcmp (pMissionObject->ClassName, ClassName) == 0)
    {
      BaseAttribute* pEthalonArray = pMissionObject->AttrList->FindInAttrList(Name, Type);
      if (pEthalonArray)
      {
        ArrayAttribute* EthalonArrayAttr = (ArrayAttribute*)pEthalonArray;

        //AfterLoad(EthalonArrayAttr, pMissionObject->AttrList);
      }
    }
  }


}


ArrayAttribute::ArrayElement& ArrayAttribute::GetDefaultElement ()
{
  return defaultelement;
}

void ArrayAttribute::AfterLoad (BaseAttribute* baseattr, AttributeList* OriginalAttrList , const char* szClassName)
{
  if (baseattr->GetType() != IMOParams::t_array) return;




  //api->Trace("array - %s\n", baseattr->GetName());
  

  ArrayAttribute* ArrayAttr = (ArrayAttribute*)baseattr;

  //SetMinElementCount(ArrayAttr->GetMinElementCount());
  //SetMaxElementCount(ArrayAttr->GetMaxElementCount());

  int availCount = sMission->AvailableMO.Size ();
  for (int n = 0; n< availCount; n++)
  {
    MissionEditor::tAvailableMO* pMissionObject = &sMission->AvailableMO[n];
    // Если нашли нужный класс
    if (strcmp (pMissionObject->ClassName, szClassName) == 0)
    {
      BaseAttribute* pEthalonArray = pMissionObject->AttrList->FindInAttrList(Name, Type);
      if (pEthalonArray)
      {
        ArrayAttribute* EthalonArrayAttr = (ArrayAttribute*)pEthalonArray;
        SetMinElementCount(EthalonArrayAttr->GetMinElementCount());
        SetMaxElementCount(EthalonArrayAttr->GetMaxElementCount());

      }
    }
  }


  Copy (defaultelement, ArrayAttr->GetDefaultElement());

  ArrayElement temp;
  for (int n = 0; n < ArrayValues; n++)
  {
    // Копируем в TEMP текущий элемент...
    Copy (temp, ArrayValues[n]);
  
    // Ощицаем текущую позицию в массиве...
    for (DWORD j = 0; j < ArrayValues[n].elements.Size(); j++)
    {
      //delete ArrayValues[n].elements[j];
			DeleteAttribute(ArrayValues[n].elements[j]);
    }
    ArrayValues[n].elements.DelAll();

    // Идем по атрибуту-эталону...
    for (DWORD i = 0; i < defaultelement.elements.Size(); i++)
    {
      BaseAttribute* etalon = defaultelement.elements[i];
      // Идем по загруженному аттрибуту...
      int found_index = -1;
      for (DWORD j = 0; j < temp.elements.Size(); j++)
      {
        BaseAttribute* loaded = temp.elements[j];
        // Они совпадают...
        if (crt_stricmp (loaded->GetName(), etalon->GetName()) == 0)
        {
          found_index = j;
          break;
        }
      } // Для всех элементов в загруженном аттрибуте...
      // found_index индекс нужного нам элемента
      if (found_index == -1)
      {
        BaseAttribute* AttrCopy = CreateAttributeCopy (etalon);
        ArrayValues[n].elements.Add(AttrCopy);
      } else
      {
        temp.elements[found_index]->AfterLoad(etalon, OriginalAttrList, szClassName);
        BaseAttribute* AttrCopy = CreateAttributeCopy (temp.elements[found_index]);
        ArrayValues[n].elements.Add(AttrCopy);
      }

    } // Для всех элементов в эталоне...

    for ( j = 0; j < temp.elements.Size(); j++)
    {
			DeleteAttribute(temp.elements[j]);
      //delete temp.elements[j];
    }
    temp.elements.DelAll();
  } // Для всего массива...


  //api->Trace("array max/min = %d / %d\n", max, min);
  
}


int ArrayAttribute::GetElementIndex (ArrayElement* element)
{
  for (int n = 0; n < ArrayValues; n++)
  {
    if (&ArrayValues[n] == element) return n;
  }
  return -1;
}

void ArrayAttribute::RemoveValue (int index)
{
  if ((int)ArrayValues.Size() <= min) return;
  
  // Убиваем аттрибуты...
  for (DWORD n =0; n < ArrayValues[index].elements.Size(); n++)
  {
	DeleteAttribute(ArrayValues[index].elements[n]);
  }
  
  // Удаляем кусочек...
  ArrayValues.DelIndex(index);
  SetMasterData (MasterData);
}

void ArrayAttribute::InsertValue (int index, ArrayAttribute::ArrayElement* copyfrom)
{
  if ((int)ArrayValues.Size() >= max) return;
  ArrayElement NewElement;

  if (!copyfrom)
    Copy (NewElement, defaultelement);
  else
    Copy (NewElement, *copyfrom);
  
  ArrayValues.Insert(NewElement, index);
  NewElement.elements.DelAll();
  SetMasterData (MasterData);
  return;
}


void ArrayAttribute::DebugLog (int deep)
{
  string mar;
  for (int q = 0; q <= deep; q++)
  {
    mar += "   ";
  }


  for (dword j = 0; j < ArrayValues.Size(); j++)
  {
    for (dword n = 0 ; n < ArrayValues[j].elements.Size(); n++)
    {
      IMOParams::Type t = ArrayValues[j].elements[n]->GetType();

      api->Trace("%s[%d]%s - %s", mar.c_str(), j, ArrayValues[j].elements[n]->GetName(), AttributeList::GetTextType(t));

      if (t == IMOParams::t_array)
      {
        ArrayAttribute* pArray = (ArrayAttribute*)ArrayValues[j].elements[n];
        pArray->DebugLog (deep+1);

      }

      if (t == IMOParams::t_group)
      {
        GroupAttribute* pGroup = (GroupAttribute*)ArrayValues[j].elements[n];
        pGroup->DebugLog (deep+1);

      }
    }
  }
}


void ArrayAttribute::WriteToXML (TextFile &file, int level)
{
  file.Write(level, "<array val = \"%s\">\n", GetName ());
  file.Write(level+1, "<items>\n");

  for (dword i = 0; i < ArrayValues.Size(); i++)
  {
    file.Write(level+2, "<item>\n");
    for (dword j = 0; j < ArrayValues[i].elements.Size(); j++)
    {
      BaseAttribute* base = ArrayValues[i].elements[j];
      base->WriteToXML(file, level+3);
    }
    file.Write(level+2, "</item>\n");
  }

  

  file.Write(level+1, "</items>\n");
  file.Write(level, "</array>\n");
}

void ArrayAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
{
  const char* objectName = Root->Attribute("val");
  SetName(objectName);

  TiXmlElement* node = Root->FirstChildElement("items");

  if (node)
  {
    for(TiXmlElement* child = node->FirstChildElement(); child; child = child->NextSiblingElement())
    {
      ArrayElement newElement;

      const char* val = child->Value();
      if (crt_stricmp(val, "item") == 0)
      {
        //Нашли item массива
        //внутри может быть много разнотипных элементов итерируемся по ним...
        for(TiXmlElement* item_child = child->FirstChildElement(); item_child; item_child = item_child->NextSiblingElement())
        {
          IMOParams::Type type = AttributeList::GetTypeFromXML(item_child);

          BaseAttribute* nAttr = CreateEmptyAttribute (type);
          nAttr->ReadXML(item_child, szMasterClass);
          BaseAttribute* added_attr = CreateAttributeCopy (nAttr);
          newElement.elements.Add(added_attr);

					DeleteAttribute(nAttr);
          //delete nAttr;
        }
      }

      ArrayValues.Add(newElement);
      newElement.elements.DelAll();

    }
  }


}