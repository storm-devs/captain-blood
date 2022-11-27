#include "DataGraph.h"
#include "..\..\icommon\memfile.h"
#include "..\..\icommon\types.h"
#include "..\..\icommon\graphtime.h"
#include "..\..\icommon\names.h"
#include "..\..\..\common_h\core.h"

#include "..\..\TextFile.h"

#include "..\..\service\particleservice.h"
extern ParticleService* PService;

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "fieldlist.h"

#include "..\..\..\Common_h\data_swizzle.h"

#pragma warning (disable : 4800)

//Взять случайное число из диапазона
__forceinline float RandomRange (float Min, float Max)      
{
	return RRnd(Min, Max);
	/*
	float Temp;
	if (Min>Max)
	{
		Temp = Max;
		Max = Min;
		Min = Temp;
	}
	
	float Rand = (float)(rand()%1000) / 1000.0f;
	Rand *= (Max - Min);
	return Rand + Min;*/
}


DataGraph::DataGraph () /*: _MinGraph(_FL_), _MaxGraph(_FL_)*/
{
	GraphData = NULL;
	MinGraphDataStart = -1;
	MaxGraphDataStart = -1;
	MinGraphDataSize = 0;
	MaxGraphDataSize = 0;




	bConstGraph = false;
	szName = NULL;
	szEditorName = NULL;

	Master = NULL;
	bRelative = false;
	bNegative = false;

	ResetCachedTime ();
}

void DataGraph::SetMasterField (FieldList* pMaster)
{
	Master = pMaster;
}


//конструктор/деструктор
DataGraph::DataGraph (FieldList* pMaster) /*: _MinGraph(_FL_), _MaxGraph(_FL_)*/
{
	GraphData = NULL;
	MinGraphDataStart = -1;
	MaxGraphDataStart = -1;
	MinGraphDataSize = 0;
	MaxGraphDataSize = 0;

	bConstGraph = false;
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
	bRelative = false;
	bNegative = false;

	ResetCachedTime ();
}

DataGraph::~DataGraph ()
{
}

void DataGraph::CalculateConstData ()
{

	if (MinGraphDataSize == 2 && MaxGraphDataSize == 2)
	{
		

		if (GraphData->GetElement(MinGraphDataStart+1).Time >= 7000.0f && GraphData->GetElement(MaxGraphDataStart+1).Time >= 7000.0f)
		{
			bConstGraph = true;
		}
	} else
	{
		if (MinGraphDataSize == 1 && MaxGraphDataSize == 1)
		{
			bConstGraph = true;
		}
	}

	if (bConstGraph)
	{
		if (MinGraphDataSize > 0  && MaxGraphDataSize > 0)
		{
			fConstMin = GraphData->GetElement(MinGraphDataStart).Val;
			fConstMax = GraphData->GetElement(MaxGraphDataStart).Val;
		} else
		{
			fConstMin = 0.0f;
			fConstMax = 0.0f;
		}
	}
}

//Установить значения...
void DataGraph::SetValues (const GraphVertex* MinValues, DWORD MinValuesSize, const GraphVertex* MaxValues, DWORD MaxValuesSize)
{
	GraphData = PService->AllocateDataForGraphs(MinValuesSize, MaxValuesSize, MinGraphDataStart, MinGraphDataSize, MaxGraphDataStart, MaxGraphDataSize);

	DWORD n  = 0;
	for (n = 0; n < MinValuesSize; n++)
	{
		GraphData->GetElement(MinGraphDataStart+n) = MinValues[n];

		//MinGraph[n] = MinValues[n];
	}

	for (n = 0; n < MaxValuesSize; n++)
	{
		GraphData->GetElement(MaxGraphDataStart+n) = MaxValues[n];

		//MaxGraph[n] = MaxValues[n];
	}

	ResetCachedTime ();

	if (Master) Master->UpdateCache();


	CalculateConstData();

}

//Устанавливает "значение по умолчанию"
void DataGraph::SetDefaultValue (float MaxValue, float MinValue)
{
	//AllocateDataForGraphs(2, 2);

	GraphData = PService->AllocateDataForGraphs(2, 2, MinGraphDataStart, MinGraphDataSize, MaxGraphDataStart, MaxGraphDataSize);


	GraphVertex Min;
	Min.Val = MinValue;
	Min.Time = MIN_GRAPH_TIME;
	//MinGraph.Add(Min);
	GraphData->GetElement(MinGraphDataStart) = Min;
	Min.Time = MAX_GRAPH_TIME;
	//MinGraph.Add(Min);
	GraphData->GetElement(MinGraphDataStart+1) = Min;

	GraphVertex Max;
	Max.Val = MinValue;
	Max.Time = MIN_GRAPH_TIME;
	//MaxGraph.Add(Max);
	GraphData->GetElement(MaxGraphDataStart) = Max;
	Max.Time = MAX_GRAPH_TIME;
	//MaxGraph.Add(Max);
	GraphData->GetElement(MaxGraphDataStart) = Max;

	ResetCachedTime ();

	CalculateConstData();
}


//Получить кол-во в графике минимума
DWORD DataGraph::GetMinCount ()
{
	return MinGraphDataSize;
}

//Получить кол-во в графике максимума
DWORD DataGraph::GetMaxCount ()
{
	return MaxGraphDataSize;
}

//Получить значение по индексу из графика минимума
const GraphVertex& DataGraph::GetMinVertex (DWORD Index)
{
	return GraphData->GetElement(MinGraphDataStart+Index);

	//return MinGraph[Index];
}

//Получить значение по индексу из графика максимума
const GraphVertex& DataGraph::GetMaxVertex (DWORD Index)
{
	return GraphData->GetElement(MaxGraphDataStart+Index);

	//return MaxGraph[Index];
}

	
void DataGraph::ResetCachedTime ()
{
	MaxCachedTime = NOT_INITED_CACHE_VALUE;
	MinCachedTime = NOT_INITED_CACHE_VALUE;
}



void DataGraph::Load (MemFile* File)
{
	//MinGraph.DelAll();
	//MaxGraph.DelAll();

	dword dwNegative = 0;
	File->ReadType(dwNegative);
	XSwizzleDWord(dwNegative);

	SetNegative(dwNegative);

	dword dwRelative = 0;
	File->ReadType(dwRelative);
	XSwizzleDWord(dwRelative);

	SetRelative(dwRelative);

	dword MaxGraphItemsCount = 0;
	File->ReadType(MaxGraphItemsCount);
	XSwizzleDWord(MaxGraphItemsCount);


	DWORD i = 0;
	//MaxGraph.Reserve(MaxGraphItemsCount);


	//AllocateDataForGraphs(-1, MaxGraphItemsCount);

	GraphData = PService->AllocateDataForGraphs(-1, MaxGraphItemsCount, MinGraphDataStart, MinGraphDataSize, MaxGraphDataStart, MaxGraphDataSize);

	for (i = 0; i < MaxGraphItemsCount; i++)
	{
		float fTime = 0.0f;
		File->ReadType(fTime);
		XSwizzleFloat(fTime);

		float fValue = 0.0f;
		File->ReadType(fValue);
		XSwizzleFloat(fValue);

		if (!bNegative && fValue < 0.0f) fValue = 0.0f;


		GraphVertex MaxVertex;
		MaxVertex.Time = fTime;
		MaxVertex.Val = fValue;
		//MaxGraph.Add(MaxVertex);


		GraphData->GetElement(MaxGraphDataStart+i) = MaxVertex;

		//api->Trace("Max value %d = %3.2f, %3.2f", i, fTime, fValue);
	}

	dword MinGraphItemsCount = 0;
	File->ReadType(MinGraphItemsCount);
	XSwizzleDWord(MinGraphItemsCount);
	//MinGraph.Reserve(MinGraphItemsCount);

	//AllocateDataForGraphs(MinGraphItemsCount, -1);

	GraphData = PService->AllocateDataForGraphs(MinGraphItemsCount, -1, MinGraphDataStart, MinGraphDataSize, MaxGraphDataStart, MaxGraphDataSize);

	

	for (i = 0; i < MinGraphItemsCount; i++)
	{
		float fTime = 0.0f;
		File->ReadType(fTime);
		XSwizzleFloat(fTime);

		float fValue = 0.0f;
		File->ReadType(fValue);
		XSwizzleFloat(fValue);

		if (!bNegative && fValue < 0.0f) fValue = 0.0f;


		GraphVertex MinVertex;
		MinVertex.Time = fTime;
		MinVertex.Val = fValue;
		//MinGraph.Add(MinVertex);

		GraphData->GetElement(MinGraphDataStart+i) = MinVertex;

		//api->Trace("Min value %d = %3.2f, %3.2f", i, fTime, fValue);


	}

	//static char AttribueName[128];
	dword NameLength = 0;
	File->ReadType(NameLength);
	XSwizzleDWord(NameLength);

	Assert (NameLength < 128);
	//File->Read(AttribueName, NameLength);
	const char* AttribueName = File->GetPointerToString(NameLength);

	//api->Trace("Name %s", AttribueName);

	SetName (AttribueName, "a");


	//HACK ! Для совместимости со старой версией...
	//Конвертим после загрузки графики в нужный формат...
	//if (crt_stricmp (AttribueName, EMISSION_DIR_X) == 0)	ConvertDegToRad ();
	//if (crt_stricmp (AttribueName, EMISSION_DIR_Y) == 0)	ConvertDegToRad ();
	//if (crt_stricmp (AttribueName, EMISSION_DIR_Z) == 0)	ConvertDegToRad ();
	//if (crt_stricmp (AttribueName, PARTICLE_DRAG) == 0)	NormalToPercent();
	//if (crt_stricmp (AttribueName, PARTICLE_TRANSPARENCY) == 0)	NormalToAlpha();

	if (Master) Master->UpdateCache();

	CalculateConstData();

}


//Установить/получить могут быть отрицательные значения в графике или нет...
void DataGraph::SetNegative (bool _bNegative)
{
	bNegative = _bNegative;
}

bool DataGraph::GetNegative ()
{
	return bNegative;
}

//Установить/получить относительный график или нет...
void DataGraph::SetRelative (bool _bRelative)
{
	bRelative = _bRelative;
}

bool DataGraph::GetRelative ()
{
	return bRelative;
}

void DataGraph::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataGraph::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataGraph::GetName ()
{
	return szName;
}



void DataGraph::ConvertRadToDeg ()
{
	MultiplyBy (MUL_RADTODEG);
}

void DataGraph::ConvertDegToRad ()
{
	MultiplyBy (MUL_DEGTORAD);
}

void DataGraph::MultiplyBy (float Val)
{
	long n;
	for (n = 0; n < MaxGraphDataSize; n++)
	{
		float & v = GraphData->GetElement(MaxGraphDataStart+n).Val;

		v *= Val;

		//MaxGraph[n].Val *= Val;
	}

	for (n = 0; n < MinGraphDataSize; n++)
	{
		float & v = GraphData->GetElement(MinGraphDataStart+n).Val;

		v *= Val;

		//MinGraph[n].Val *= Val;
	}
}



void DataGraph::Clamp (float MinValue, float MaxValue)
{
	long n;
	for (n = 0; n < MaxGraphDataSize; n++)
	{
		float & v = GraphData->GetElement(MaxGraphDataStart+n).Val;

		if (v > MaxValue) v = MaxValue;
		if (v < MinValue) v = MinValue;
	}

	for (n = 0; n < MinGraphDataSize; n++)
	{
		float & v = GraphData->GetElement(MinGraphDataStart+n).Val;

		if (v > MaxValue) v = MaxValue;
		if (v < MinValue) v = MinValue;
	}

	CalculateConstData ();
}


void DataGraph::Reverse ()
{
	long n;
	for (n = 0; n < MaxGraphDataSize; n++)
	{
		float & v = GraphData->GetElement(MaxGraphDataStart+n).Val;

		v = 1.0f - v;
		//MaxGraph[n].Val = 1.0f - MaxGraph[n].Val;
	}

	for (n = 0; n < MinGraphDataSize; n++)
	{
		float & v = GraphData->GetElement(MinGraphDataStart+n).Val;

		v = 1.0f - v;
		//MinGraph[n].Val = 1.0f - MinGraph[n].Val;
	}

	CalculateConstData ();
}

void DataGraph::NormalToPercent ()
{
	MultiplyBy (0.01f);
	Reverse ();
	Clamp (0.0f, 1.0f);

	CalculateConstData ();
}

void DataGraph::PercentToNormal ()
{
	Reverse ();
	MultiplyBy (100.0f);

	CalculateConstData ();
}

void DataGraph::NormalToAlpha ()
{
	NormalToPercent ();
	MultiplyBy (255.0f);

	CalculateConstData();
}

void DataGraph::AlphaToNormal ()
{
	MultiplyBy (0.00392156862745098f);
	PercentToNormal ();

	CalculateConstData();
}

float DataGraph::GetMaxTime ()
{
	float MaxVal = 10.0f;
	float MinVal = 10.0f;
	DWORD MaxCount = MaxGraphDataSize;
	DWORD MinCount = MinGraphDataSize;

	if (MaxCount > 2)	MaxVal = GraphData->GetElement(MaxGraphDataStart+MaxCount-2).Time;
	if (MinCount > 2)	MinVal = GraphData->GetElement(MinGraphDataStart+MaxCount-2).Time;

	if (MaxVal > MinVal) return MaxVal;
	return MinVal;
}

void DataGraph::Write (MemFile* File)
{
	DWORD dwNegative = GetNegative();
	File->WriteType(dwNegative);

	DWORD dwRelative = GetRelative();
	File->WriteType(dwRelative);


	
	DWORD MaxGraphItemsCount = MaxGraphDataSize;
	File->WriteType(MaxGraphItemsCount);

	DWORD i = 0;
	for (i = 0; i < MaxGraphItemsCount; i++)
	{

		
		float fTime = GraphData->GetElement(MaxGraphDataStart+i).Time;
		File->WriteType(fTime);

		float fValue = GraphData->GetElement(MaxGraphDataStart+i).Val;
		File->WriteType(fValue);
	}

	DWORD MinGraphItemsCount = MinGraphDataSize;
	File->WriteType(MinGraphItemsCount);

	for (i = 0; i < MinGraphItemsCount; i++)
	{
		float fTime = GraphData->GetElement(MinGraphDataStart+i).Time;
		File->WriteType(fTime);

		float fValue = GraphData->GetElement(MinGraphDataStart+i).Val;
		File->WriteType(fValue);
	}


	//save name
	DWORD NameLength = crt_strlen (szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();

}

const char* DataGraph::GetEditorName ()
{
	return szEditorName;
}





#ifndef _XBOX
void DataGraph::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);

	xmlFile->Write((level+1), "<Negative val = \"%d\" />\n", GetNegative());
	xmlFile->Write((level+1), "<Relative val = \"%d\" />\n", GetRelative());



	xmlFile->Write((level+1), "<MaxGraph>\n");
	for (long n = 0; n < MaxGraphDataSize; n++)
	{
		xmlFile->Write((level+1), "<Key>\n");

		xmlFile->Write((level+2), "<Time val = \"%f\" />\n", GraphData->GetElement(MaxGraphDataStart+n).Time);
		xmlFile->Write((level+2), "<Value val = \"%f\" />\n", GraphData->GetElement(MaxGraphDataStart+n).Val);

		xmlFile->Write((level+1), "</Key>\n");
	}
	xmlFile->Write((level+1), "</MaxGraph>\n");




	xmlFile->Write((level+1), "<MinGraph>\n");
	for (long n = 0; n < MinGraphDataSize; n++)
	{
		xmlFile->Write((level+1), "<Key>\n");

		xmlFile->Write((level+2), "<Time val = \"%f\" />\n", GraphData->GetElement(MinGraphDataStart+n).Time);
		xmlFile->Write((level+2), "<Value val = \"%f\" />\n", GraphData->GetElement(MinGraphDataStart+n).Val);

		xmlFile->Write((level+1), "</Key>\n");
	}
	xmlFile->Write((level+1), "</MinGraph>\n");

}


void DataGraph::LoadXML (TiXmlElement* root)
{
/* FIXME: to using GraphData

	MinGraph.DelAll();
	MaxGraph.DelAll();


	TiXmlElement* name = root->FirstChildElement("Name");
	if (name)
	{
		SetName (name->Attribute("val"), "a");
	}


	TiXmlElement* neg = root->FirstChildElement("Negative");
	if (neg)
	{
		SetNegative (atoi (neg->Attribute("val")) == 1);
	}

	TiXmlElement* rel = root->FirstChildElement("Relative");
	if (rel)
	{
		SetRelative (atoi (rel->Attribute("val")) == 1);
	}




	TiXmlElement* maxGraphNode = NULL;
	maxGraphNode = root->FirstChildElement("MaxGraph");
	if (maxGraphNode)
	{
		for(TiXmlElement* child = maxGraphNode->FirstChildElement(); child; child = child->NextSiblingElement())
		{
			string NodeName = child->Value();

			if (NodeName == "Key")
			{

				GraphVertex MaxVertex;


				TiXmlElement* time = child->FirstChildElement("Time");
				TiXmlElement* val = child->FirstChildElement("Value");

				if (time)
				{
					MaxVertex.Time = (float)atof (time->Attribute("val"));
				}
				if (val)
				{
					MaxVertex.Val = (float)atof (val->Attribute("val"));
				}

				MaxGraph.Add(MaxVertex);

			}
		}
	}


	TiXmlElement* minGraphNode = NULL;
	minGraphNode = root->FirstChildElement("MinGraph");
	if (minGraphNode)
	{
		for(TiXmlElement* child = minGraphNode->FirstChildElement(); child; child = child->NextSiblingElement())
		{
			string NodeName = child->Value();

			if (NodeName == "Key")
			{

				GraphVertex MinVertex;


				TiXmlElement* time = child->FirstChildElement("Time");
				TiXmlElement* val = child->FirstChildElement("Value");

				if (time)
				{
					MinVertex.Time = (float)atof (time->Attribute("val"));
				}
				if (val)
				{
					MinVertex.Val = (float)atof (val->Attribute("val"));
				}

				MinGraph.Add(MinVertex);

			}
		}
	}

	CalculateConstData();
*/
}

#endif