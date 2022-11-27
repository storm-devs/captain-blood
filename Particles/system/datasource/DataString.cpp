#include "DataString.h"
#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"

#include "fieldlist.h"

#include "..\..\TextFile.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "..\..\..\Common_h\data_swizzle.h"

//конструктор/деструктор
DataString::DataString (FieldList* pMaster)
{
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
}

DataString::~DataString ()
{
}

//Получить значение
const char* DataString::GetValue ()
{
	return Value.GetBuffer();
}

//Установить значение
void DataString::SetValue (const char* val)
{
	Value = val;

	if (Master) Master->UpdateCache();
}

void DataString::Load (MemFile* File)
{
	static char TempString[128];
	File->Read(TempString, 128);
	SetValue(TempString);

	//static char AttribueName[128];
	dword NameLength = 0;
	File->ReadType(NameLength);
	XSwizzleDWord(NameLength);
	Assert (NameLength < 128);
	//File->Read(AttribueName, NameLength);
	const char* AttribueName = File->GetPointerToString(NameLength);

	SetName (AttribueName, "a");

	if (Master) Master->UpdateCache();
}

void DataString::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataString::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataString::GetName ()
{
	return szName;
}

void DataString::Write (MemFile* File)
{
	static char WriteTempString[128];
	memset (WriteTempString, 0, 128);
	crt_strncpy (WriteTempString, 128, GetValue (), 127);
	File->Write(WriteTempString, 128);

	//save name
	DWORD NameLength = crt_strlen(szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();
}

const char* DataString::GetEditorName ()
{
	return szEditorName;
}

#ifndef _XBOX
void DataString::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);
	xmlFile->Write((level+1), "<Value val = \"%s\" />\n", GetValue ());
}

void DataString::LoadXML (TiXmlElement* root)
{
	TiXmlElement* name = root->FirstChildElement("Name");
	TiXmlElement* value = root->FirstChildElement("Value");


	if (name)
	{
		const char* strValue = name->Attribute("val");
		SetName (strValue, "a");
	}

	if (value)
	{
		const char* strValue =value->Attribute("val");
		SetValue (strValue);
	}

}
#endif