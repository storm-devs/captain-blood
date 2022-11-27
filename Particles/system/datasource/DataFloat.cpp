#include "DataFloat.h"
#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"

#include "fieldlist.h"

#include "..\..\TextFile.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "..\..\..\Common_h\data_swizzle.h"

//конструктор/деструктор
DataFloat::DataFloat (FieldList* pMaster)
{
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
	Value = 0.0f;
}

DataFloat::~DataFloat ()
{
}

	//Получить значение
float DataFloat::GetValue ()
{
	return Value;
}

	//Установить значение
void DataFloat::SetValue (float val)
{
	Value = val;

	if (Master) Master->UpdateCache();
}

void DataFloat::Load (MemFile* File)
{
	float fValue = 0.0f;
	File->ReadType(fValue);

	XSwizzleFloat(fValue);


	SetValue (fValue);

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

void DataFloat::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataFloat::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataFloat::GetName ()
{
	return szName;
}

void DataFloat::Write (MemFile* File)
{
	float fValue = GetValue();
	File->WriteType(fValue);

	//save name
	DWORD NameLength = crt_strlen (szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();
}

const char* DataFloat::GetEditorName ()
{
	return szEditorName;
}

#ifndef _XBOX
void DataFloat::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);
	xmlFile->Write((level+1), "<Value val = \"%f\" />\n", GetValue());
}


void DataFloat::LoadXML (TiXmlElement* root)
{
	TiXmlElement* name = root->FirstChildElement("Name");
	TiXmlElement* value = root->FirstChildElement("Value");


	if (name)
	{
		SetName (name->Attribute("val"), "a");
	}

	if (value)
	{
		SetValue ((float)atof (value->Attribute("val")));
	}


}

#endif