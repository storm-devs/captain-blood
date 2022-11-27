#include "data_desc.h"


	 
DataDescripion::DataDescripion ()
{
	ItemsCount = 0;
}

DataDescripion::~DataDescripion ()
{
}
 
void DataDescripion::AddField (FieldType Type, dword guid, const char* Name, const char* EditorName)
{
	Assert (ItemsCount < MAX_DESC_COUNT);
	Fields[ItemsCount].Type = Type;
	Fields[ItemsCount].Name = Name;
	Fields[ItemsCount].EditorName = EditorName;
	Fields[ItemsCount].guid = guid;
	ItemsCount++;
}
 
dword DataDescripion::GetFieldGUID (DWORD Index)
{
	return Fields[Index].guid;
}

const char* DataDescripion::GetFieldName (DWORD Index)
{
	return Fields[Index].Name;
}

const char* DataDescripion::GetFieldEditorName (DWORD Index)
{
	return Fields[Index].EditorName;
}

FieldType DataDescripion::GetFieldType (DWORD Index)
{
	return Fields[Index].Type;
}


int DataDescripion::GetFieldCount ()
{
  return ItemsCount;
}

void DataDescripion::Clear ()
{
	ItemsCount = 0;
}

