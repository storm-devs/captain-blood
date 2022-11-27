#ifndef _PARTICLE_DATA_COLOR_H_
#define _PARTICLE_DATA_COLOR_H_


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "..\..\icommon\memfile.h"
#include "..\..\icommon\colorvertex.h"

class FieldList;
class TextFile;

#ifndef _XBOX
class TiXmlElement;
#endif

class DataColor
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;

	const Color ZeroColor;

	array<ColorVertex> ColorGraph;


	Color MaxVal;
	Color MinVal;
	Color Result;


public:

//конструктор/деструктор
	DataColor (FieldList* pMaster);
	virtual ~DataColor ();

//Получить значение (Текущее время, Общее время жизни, Коэфицент рандома[0..1])
	//Color GetValue (float Time, float LifeTime, float K_rand);

	//Получить значение (Текущее время, Общее время жизни, Коэфицент рандома[0..1])
	__forceinline Color GetValue (float Time, float LifeTime, float K_rand)
	{
		//Время у графика цвета всегда относительное...
		Time = (Time / LifeTime);

		DWORD Count = ColorGraph.Size();
		DWORD StartIndex = 0;
		for (DWORD n = StartIndex; n < (Count-1); n++)
		{
			float FromTime = ColorGraph[n].Time;
			float ToTime = ColorGraph[n+1].Time;

			//Если время в нужном диапазоне...
			if ((Time >= FromTime)	&& (Time <= ToTime))
			{
				float SegmentDeltaTime = ColorGraph[n+1].Time - ColorGraph[n].Time;
				float ValueDeltaTime = Time - ColorGraph[n].Time;
				float blend_k = 0.0f;
				if (SegmentDeltaTime > 0.001f)	blend_k = ValueDeltaTime / SegmentDeltaTime;

				const Color & ValueFirstMax = ColorGraph[n].MaxValue;
				const Color & ValueSecondMax = ColorGraph[n+1].MaxValue;
				const Color & ValueFirstMin = ColorGraph[n].MinValue;
				const Color & ValueSecondMin = ColorGraph[n+1].MinValue;

				MaxVal.Lerp(ValueFirstMax, ValueSecondMax, blend_k);
				MinVal.Lerp(ValueFirstMin, ValueSecondMin, blend_k);
				Result.Lerp(MinVal, MaxVal, K_rand);

				return Result;
			}
		}

		return ZeroColor;
	}


//Устанавливает "значение по умолчанию"
//два индекса, Min=Max=Value
	void SetDefaultValue (const Color& Value);

//Установить значения
	void SetValues (const ColorVertex* Values, DWORD Count);

//Получить кол-во значений
	DWORD GetValuesCount ();

//Получить мин. значение (по индексу)
	const Color& GetMinValue (DWORD Index);

//Получить макс. значение (по индексу)
	const Color& GetMaxValue (DWORD Index);



	void Load (MemFile* File);

	void Write (MemFile* File);


#ifndef _XBOX
	void LoadXML (TiXmlElement* root);


	void WriteXML (TextFile* xmlFile, dword level);
#endif


	void SetName (const char* szName, const char* szEditorName);

	const char* GetEditorName ();
	const char* GetName ();


	const ColorVertex& GetByIndex (DWORD Index);

};


#endif