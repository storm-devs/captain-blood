#include "DataPosition.h"
#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"

#include "..\..\TextFile.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "fieldlist.h"

#include "..\..\..\Common_h\data_swizzle.h"


//конструктор/деструктор
DataPosition::DataPosition (FieldList* pMaster)
{
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
	Value = Vector(0, 0, 0);
}

DataPosition::~DataPosition ()
{
}

//Получить значение (Текущее время, Коэфицент рандома[0..1])
const Vector& DataPosition::GetValue ()
{
	return Value;
}

//Установить значение
void DataPosition::SetValue (const Vector& val)
{
	Value = val;

	if (Master) Master->UpdateCache();
}


void DataPosition::Load (MemFile* File)
{
	Vector vValue;
	File->ReadType(vValue.x);
	XSwizzleFloat(vValue.x);
	File->ReadType(vValue.y);
	XSwizzleFloat(vValue.y);
	File->ReadType(vValue.z);
	XSwizzleFloat(vValue.z);
	//api->Trace("Read position %3.2f, %3.2f, %3.2f", vValue.x, vValue.y, vValue.z);
	SetValue (vValue);

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

void DataPosition::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataPosition::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataPosition::GetName ()
{
	return szName;
}

void DataPosition::Write (MemFile* File)
{
	Vector vValue = GetValue();
	//api->Trace("Write position %3.2f, %3.2f, %3.2f", vValue.x, vValue.y, vValue.z);
	File->WriteType(vValue.x);
	File->WriteType(vValue.y);
	File->WriteType(vValue.z);

	//save name
	DWORD NameLength = crt_strlen(szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();
}

const char* DataPosition::GetEditorName ()
{
	return szEditorName;
}

#ifndef _XBOX
void DataPosition::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);
	xmlFile->Write((level+1), "<Value_x val = \"%f\" />\n", GetValue().x);
	xmlFile->Write((level+1), "<Value_y val = \"%f\" />\n", GetValue().y);
	xmlFile->Write((level+1), "<Value_z val = \"%f\" />\n", GetValue().z);
}


void DataPosition::LoadXML (TiXmlElement* root)
{
	TiXmlElement* name = root->FirstChildElement("Name");
	TiXmlElement* value_x = root->FirstChildElement("Value_x");
	TiXmlElement* value_y = root->FirstChildElement("Value_y");
	TiXmlElement* value_z = root->FirstChildElement("Value_z");


	if (name)
	{
		SetName (name->Attribute("val"), "a");
	}

	Vector vl = 0.0f;

	if (value_x)
	{
		vl.x = (float)atof (value_x->Attribute("val"));
	}

	if (value_y)
	{
		vl.y = (float)atof (value_y->Attribute("val"));
	}

	if (value_z)
	{
		vl.z = (float)atof (value_z->Attribute("val"));
	}

	SetValue(vl);

}

#endif