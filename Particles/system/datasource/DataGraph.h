#ifndef _PARTICLE_DATA_GRAPH_H_
#define _PARTICLE_DATA_GRAPH_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "..\..\icommon\memfile.h"

#include "..\..\icommon\GraphVertex.h"

class FieldList;
class TextFile;

extern DWORD GraphRead;
extern DWORD OptGraphRead;


#ifndef _XBOX
class TiXmlElement;
#endif

class DataGraph
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;


//С какого времени последний раз забирали значение
	float MaxCachedTime;
	float MinCachedTime;
//Какой был индекс у этого времени
	DWORD MaxCachedIndex;
	DWORD MinCachedIndex;

	//array<GraphVertex> _MinGraph;
	//array<GraphVertex> _MaxGraph;

	


	long MinGraphDataStart;
	long MaxGraphDataStart;
	long MinGraphDataSize;
	long MaxGraphDataSize;
	array<GraphVertex> * GraphData;


	void ResetCachedTime ();

	__forceinline float GetMinAtTime (float Time, float LifeTime)
	{
		if (bRelative) Time = Time / LifeTime * 100.0f;

		DWORD Count = MinGraphDataSize;
		DWORD Index;
		if (MinCachedTime < Time)
			Index = MinCachedIndex;
		else
			Index = 0;


		for ( ;Index < (Count-1); Index++)
		{
			float ToTime = GraphData->GetElement(MinGraphDataStart+Index+1).Time;

			//Если время в нужном диапазоне...
			//if ((Time >= FromTime) && (Time <= ToTime))
			if (Time <= ToTime)
			{
				float FromTime = GraphData->GetElement(MinGraphDataStart+Index).Time;

				float SegmentDeltaTime = ToTime - FromTime;
				float ValueDeltaTime = Time - FromTime;
				float blend_k ;
				if (SegmentDeltaTime > 0.001f)
					blend_k = ValueDeltaTime / SegmentDeltaTime;
				else
					blend_k = 0.0f;

				float ValueFirst = GraphData->GetElement(MinGraphDataStart+Index).Val;
				float ValueSecond = GraphData->GetElement(MinGraphDataStart+Index+1).Val;

				MinCachedTime = Time;
				MinCachedIndex = Index;
				return Lerp (ValueFirst, ValueSecond, blend_k);
			}
		}

		return 0.0f;
	}



	__forceinline float GetMaxAtTime (float Time, float LifeTime)
	{
		if (bRelative)	Time = Time / LifeTime * 100.0f;

		DWORD Count = MaxGraphDataSize;

		DWORD Index;

		if (MaxCachedTime < Time)
			Index = MaxCachedIndex;
		else
			Index = 0;

		for (; Index < (Count-1); Index++)
		{
			float ToTime = GraphData->GetElement(MaxGraphDataStart+Index+1).Time;

			//Если время в нужном диапазоне...
			//if ((Time >= FromTime) && (Time <= ToTime))
			if (Time <= ToTime)
			{
				float FromTime = GraphData->GetElement(MaxGraphDataStart+Index).Time;

				float SegmentDeltaTime = ToTime - FromTime;;
				float ValueDeltaTime = Time - FromTime;
				float blend_k;
				if (SegmentDeltaTime > 0.001f)
					blend_k = ValueDeltaTime / SegmentDeltaTime;
				else
					blend_k = 0.0f;

				float ValueFirst = GraphData->GetElement(MaxGraphDataStart+Index).Val;
				float ValueSecond = GraphData->GetElement(MaxGraphDataStart+Index+1).Val;

				MaxCachedTime = Time;
				MaxCachedIndex = Index;
				return Lerp (ValueFirst, ValueSecond, blend_k);
			}
		} 

		return 0.0f;
	}



	bool bRelative;
	bool bNegative;
	

	void CalculateConstData ();

	float fConstMin;
	float fConstMax;
	bool bConstGraph;

public:

//конструктор/деструктор
	DataGraph ();
	void SetMasterField (FieldList* pMaster);



	DataGraph (FieldList* pMaster);
	virtual ~DataGraph ();


	//Установить/получить могут быть отрицательные значения в графике или нет...
	void SetNegative (bool _bNegative);
	bool GetNegative ();

	//Установить/получить относительный график или нет...
	void SetRelative (bool _bRelative);
	bool GetRelative ();


//Получить значение (Текущее время, Коэфицент рандома[0..1])

	__forceinline float GetValue (float Time, float LifeTime, float K_rand)
	{
		float pMax = fConstMax; 
		float pMin = fConstMin;
		if (!bConstGraph)
		{
			GraphRead++;
			pMax = GetMaxAtTime (Time, LifeTime);
			pMin = GetMinAtTime (Time, LifeTime);
		} else
		{
			OptGraphRead++;
		}

		return Lerp (pMin, pMax, K_rand);
	}

	__forceinline float GetRandomValue (float Time, float LifeTime)
	{

		float pMax = fConstMax; 
		float pMin = fConstMin;
		if (!bConstGraph)
		{
			GraphRead++;
			pMax = GetMaxAtTime (Time, LifeTime);
			pMin = GetMinAtTime (Time, LifeTime);
		} else
		{
			OptGraphRead++;
		}

		//float pMax = GetMaxAtTime (Time, LifeTime);
		//float pMin = GetMinAtTime (Time, LifeTime);
		
		return RRnd(pMin, pMax);
	}



//Установить значения...
	void SetValues (const GraphVertex* MinValues, DWORD MinValuesSize, const GraphVertex* MaxValues, DWORD MaxValuesSize);

	//Устанавливает "значение по умолчанию"
	void SetDefaultValue (float MaxValue, float MinValue);


//Получить кол-во в графике минимума
	DWORD GetMinCount ();

//Получить кол-во в графике максимума
	DWORD GetMaxCount ();

//Получить значение по индексу из графика минимума
	const GraphVertex& GetMinVertex (DWORD Index);

//Получить значение по индексу из графика максимума
	const GraphVertex& GetMaxVertex (DWORD Index);


	void Load (MemFile* File);

	void Write (MemFile* File);


#ifndef _XBOX
	void LoadXML (TiXmlElement* root);

	void WriteXML (TextFile* xmlFile, dword level);
#endif
	
	void SetName (const char* szName, const char* szEditorName);

	const char* GetEditorName ();
	const char* GetName ();


	float GetMaxTime ();


	void ConvertRadToDeg ();
	void ConvertDegToRad ();
	void MultiplyBy (float Val);
	void Clamp (float MinValue, float MaxValue);
	void Reverse ();  //Graphs = 1.0f - Graph
	void NormalToPercent ();
	void PercentToNormal ();
	void NormalToAlpha ();
	void AlphaToNormal ();




};


#endif