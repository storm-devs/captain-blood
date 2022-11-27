#ifndef __ADV_FONT_IMPL___
#define __ADV_FONT_IMPL___


#include "..\..\..\common_h\Render.h"
#include "..\..\..\common_h\templates\array.h"
#include "..\..\..\common_h\templates\stack.h"
#include "..\..\..\common_h\templates\string.h"
#include "..\..\..\common_h\templates\htable.h"
#include "..\..\..\common_h\templates\map.h"
#include "..\Resource.h"

#include "..\..\..\common_h\controls.h"


const dword temp_buffer_size = 1024;
const int icon_max_count = 16;


class RenderAdvFont : public IAdvFont
{
	static float round(float x)
	{
		float f = floorf(x);

		if( x - f > 0.5f )
		{
			return f + 1.0f;
		}
		else
		{
			return f;
		}
	}

public:

	struct Vertex
	{
		Vector p;

		float tu;
		float tv;

		float al;
	};

private:

	struct Params
	{
		string defaultString;

		IAdvFont::Align horizAlign;
		IAdvFont::Align  vertAlign;

		bool useSafeFrame;

		Color color;

		Params()
		{
			horizAlign = IAdvFont::Left;
			vertAlign = IAdvFont::Top;
			useSafeFrame = true;
			color = Color (0xFFFFFFFF);
		}
	};

	Params m_params;


	struct Info
	{
		ImagePlace p;

		float x; float w;
		float y; float h;

		float k;

		bool anim;
	};

	array<Info> table;
	array<long> hash;

//	string sName;


	//// цветовой тег ////

	struct Dec
	{
		bool def; // используется дефолтный цвет

		dword col;

		int i;	  // строка
		int j;	  // позиция в стоке
	};

	array<Dec>  decs;
	array<long> dech;

	//////////////////////

	struct Cut
	{
		float time;	// время отображения сегмента

		int i;		// строка
		int j;		// номер позиции последнего символа (сквозной, внутри буфера)

		int n;		// размер фрагмента в символах

		float beg;	// время включения
		float end;	// время выключения
	};

	array<Cut> cuts;

	//////////////////////

	bool useCuts; float totalDelay;

	float m_cutsTime;

	bool drawDebug;

	bool playCuts; // запускать механизм смены фрагментов

//	long priority;


	const char *m_pointer; bool m_usePointer;


public:

/*	long GetPriority()
	{
		return priority;
	}*/

	virtual bool IsTextActive()
	{
		return !useCuts || m_cutsTime < totalDelay;
	}

private:

	void UpdateTable(string &s);

	//ВНИМАНИЕ: обязательно нужен buffer и buffer_size заполненый из UpdateTable
	void UpdateHash (string &s);

	dword WordWrapString(string &text, IFont *font, float outputPixelWidth);

	void UpdateCuts();


public:

	RenderAdvFont(CritSection *section, string *buffer1, string *buffer2, IVBuffer* _pVB, IIBuffer* _pIB, const char * pFontName, float fHeight, dword dwColor, const char * fntShader, const char * circularShader, const char * circularShdwShader);
	~RenderAdvFont();


	void PostInit();


	virtual bool Prepare(bool coreThread, float w, float h, const char *text, Align hor, Align ver, bool useSafeFrame, bool copyText = true);
	virtual void Draw	(bool coreThread, float x, float y, bool animate, Effect  *eff);

	// Background scale for Draw()
	virtual void SetFakeScale(float k);

	virtual void SetCutsDelay(bool use, float time)
	{
		useCuts = use;		// использовать брейк-теги
		totalDelay = time;	// общее время показа субтитров
	}

	virtual void UpdateCutsTime(float time)
	{
		m_cutsTime = time;
	}

	virtual void DrawDebug(bool en)
	{
		drawDebug = en;
	}

	virtual void SetPlayCuts(bool en)
	{
		playCuts = en;
	}

/*	virtual void SetProirity(long value)
	{
		priority = value;
	}*/


	//Координаты относительно экрана 0.0, левый край и верх, 1.0 правый край и низ
	virtual void Print (bool coreThread, float relative_x, float relative_y, float relative_w, float relative_h, const char *text, Align hor = Center, Align ver = Center, bool useSafeFrame = true, bool animate = true, Effect * eff = null);



	virtual void Update(float time);
	virtual void SetScale(float k);

	virtual void SetColor(const Color &c);

	virtual void SetAlpha(float a);

	virtual void GetPos(float &x, float &y);

	virtual void ResetImage();


	bool Release();
	bool ForceRelease();

	void SetKerning (float kerningValue);


//	const string & GetName() const { return sName; };

private:

	DX8_RESOURCE_IMPLEMENT


	IFont *m_font;

	dword m_linesCount;

	float m_fontScale;

	float m_w;
	float m_h;

	float m_fakeScale;

	float m_strOffX;
	float m_strOffY;

	float m_time;

	float m_alpha;

	float __aspect;
	float __asp;


	CritSection *critSection;

	string &textBuffer;
	string &destBuffer;


	IBaseTexture * buttonsImage;

	IControlsService* srvControls;

	ShaderId Circular_shadow_id;
	ShaderId Circular_id;

	IVariable * ButtonsTexture;

	IVBuffer* pVB;
	IIBuffer* pIB;

	int printedChars;

};




#endif