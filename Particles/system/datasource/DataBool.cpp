#include "DataBool.h"
#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"
#include "..\..\TextFile.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "fieldlist.h"

#include "..\..\..\Common_h\data_swizzle.h"

#pragma warning (disable : 4800)

//конструктор/деструктор
DataBool::DataBool (FieldList* pMaster)
{
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
	Value = false;
}

DataBool::~DataBool ()
{
}

//Получить значение
bool DataBool::GetValue ()
{
	return Value;
}

//Установить значение
void DataBool::SetValue (bool val)
{
	Value = val;

	if (Master) Master->UpdateCache();
}

void DataBool::Load (MemFile* File)
{
	dword dwValue = 0;
	File->ReadType(dwValue);
	XSwizzleDWord(dwValue);

	SetValue (dwValue);

	//static char AttribueName[128];
	dword NameLength = 0;
	File->ReadType(NameLength);

	XSwizzleDWord(NameLength);


	Assert (NameLength < 128);

	const char* AttribueName = File->GetPointerToString(NameLength);
	//File->Read(AttribueName, NameLength);

	SetName (AttribueName, "a");

	if (Master) Master->UpdateCache();
}


void DataBool::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataBool::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataBool::GetName ()
{
	return szName;
}

void DataBool::Write (MemFile* File)
{
	DWORD dwValue = GetValue();
	File->WriteType(dwValue);


	//save name
	DWORD NameLength = crt_strlen (szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();
}

const char* DataBool::GetEditorName ()
{
	return szEditorName;
}

#ifndef _XBOX
void DataBool::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);
	xmlFile->Write((level+1), "<Value val = \"%d\" />\n", GetValue());
}



void DataBool::LoadXML (TiXmlElement* root)
{
	TiXmlElement* name = root->FirstChildElement("Name");
	TiXmlElement* value = root->FirstChildElement("Value");


	if (name)
	{
		SetName (name->Attribute("val"), "a");
	}

	if (value)
	{
		SetValue (atoi (value->Attribute("val")));
	}

	
}

#endif