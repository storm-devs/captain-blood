#include "DataColor.h"
#include "..\..\icommon\graphtime.h"
#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"
#include "..\..\TextFile.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "fieldlist.h"

#include "..\..\..\Common_h\data_swizzle.h"

//конструктор/деструктор
DataColor::DataColor (FieldList* pMaster) : ColorGraph(_FL_), ZeroColor(0xFFFFFFFFL)
{
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
}

DataColor::~DataColor ()
{
}


//Устанавливает "значение по умолчанию"
//два индекса, Min=Max=Value
void DataColor::SetDefaultValue (const Color& Value)
{
	ColorGraph.DelAll();

	ColorVertex pMinVertex;
	pMinVertex.Time = MIN_GRAPH_TIME;
	pMinVertex.MinValue = Value;
	pMinVertex.MaxValue = Value;
	ColorGraph.Add(pMinVertex);

	ColorVertex pMaxVertex;
	pMinVertex.Time = 1.0f;
	pMinVertex.MinValue = Value;
	pMinVertex.MaxValue = Value;
	ColorGraph.Add(pMinVertex);

}

//Установить значения
void DataColor::SetValues (const ColorVertex* Values, DWORD Count)
{
	ColorGraph.DelAll();
	for (DWORD n = 0; n < Count; n++)
	{
		ColorGraph.Add(Values[n]);
	}

	if (Master) Master->UpdateCache();
}

//Получить кол-во значений
DWORD DataColor::GetValuesCount ()
{
	return ColorGraph.Size();
}


//Получить мин. значение (по индексу)
const Color& DataColor::GetMinValue (DWORD Index)
{
	return ColorGraph[Index].MinValue;
}

//Получить макс. значение (по индексу)
const Color& DataColor::GetMaxValue (DWORD Index)
{
	return ColorGraph[Index].MaxValue;
}


void DataColor::Load (MemFile* File)
{
	dword dwColorCount = 0;
	File->ReadType(dwColorCount);

	XSwizzleDWord(dwColorCount);

	for (DWORD n = 0; n < dwColorCount; n++)
	{
		float Time = 0.0f;
		File->ReadType(Time);

		XSwizzleFloat(Time);

		Color clrMax;
		File->ReadType(clrMax);

		XSwizzleFloat(clrMax.r);
		XSwizzleFloat(clrMax.g);
		XSwizzleFloat(clrMax.b);
		XSwizzleFloat(clrMax.a);


		Color clrMin;
		File->ReadType(clrMin);

		XSwizzleFloat(clrMin.r);
		XSwizzleFloat(clrMin.g);
		XSwizzleFloat(clrMin.b);
		XSwizzleFloat(clrMin.a);


		ColorVertex pColor;
		pColor.Time = Time;
		pColor.MinValue = clrMin;
		pColor.MaxValue = clrMax;
		ColorGraph.Add(pColor);
	}

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

void DataColor::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataColor::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataColor::GetName ()
{
	return szName;
}

const ColorVertex& DataColor::GetByIndex (DWORD Index)
{
	return ColorGraph[Index];
}

void DataColor::Write (MemFile* File)
{
	DWORD dwColorCount = ColorGraph.Size();
	File->WriteType(dwColorCount);

	for (DWORD n = 0; n < dwColorCount; n++)
	{
		float Time = ColorGraph[n].Time;
		File->WriteType(Time);

		Color clrMax = ColorGraph[n].MaxValue;
		File->WriteType(clrMax);

		Color clrMin = ColorGraph[n].MinValue;
		File->WriteType(clrMin);
	}

	//save name
	DWORD NameLength = crt_strlen (szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();
}

const char* DataColor::GetEditorName ()
{
	return szEditorName;
}


#ifndef _XBOX
void DataColor::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);
	
	xmlFile->Write((level+1), "<Graph>\n");
	for (dword n = 0; n < ColorGraph.Size(); n++)
	{
		xmlFile->Write((level+1), "<Key>\n");

		xmlFile->Write((level+2), "<Time val = \"%f\" />\n", ColorGraph[n].Time);
		xmlFile->Write((level+2), "<Max val = \"0x%08X\" />\n", ColorGraph[n].MaxValue.GetDword());
		xmlFile->Write((level+2), "<Min val = \"0x%08X\" />\n", ColorGraph[n].MinValue.GetDword());

		xmlFile->Write((level+1), "</Key>\n");
	}
	xmlFile->Write((level+1), "</Graph>\n");
}


void DataColor::LoadXML (TiXmlElement* root)
{

	TiXmlElement* name = root->FirstChildElement("Name");
	if (name)
	{
		SetName (name->Attribute("val"), "a");
	}

	TiXmlElement* graphNode = NULL;
	graphNode = root->FirstChildElement("Graph");

	if (graphNode)
	{
		for(TiXmlElement* child = graphNode->FirstChildElement(); child; child = child->NextSiblingElement())
		{
			string NodeName = child->Value();
			
			if (NodeName == "Key")
			{

				ColorVertex pColor;

				TiXmlElement* time = child->FirstChildElement("Time");
				TiXmlElement* min = child->FirstChildElement("Min");
				TiXmlElement* max = child->FirstChildElement("Max");

				
				if (time)
				{
					pColor.Time = (float)atof (time->Attribute("val"));
				}
				if (min)
				{
					const char* hexVal = min->Attribute("val");
					hexVal += 4;
					long dwValue = strtol(hexVal, NULL, 16);
					pColor.MinValue = (dword)dwValue;
				}
				if (max)
				{
					const char* hexVal = max->Attribute("val");
					hexVal += 4;
					long dwValue = strtol(hexVal, NULL, 16);
					pColor.MaxValue = (dword)dwValue;
				}

				ColorGraph.Add(pColor);

			}
		}
	}

}

#endif