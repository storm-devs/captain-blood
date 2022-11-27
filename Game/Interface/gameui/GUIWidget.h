#pragma once

#include "..\..\..\common_h/Mission.h"
//#include "..\..\..\Common_h/IGUIText.h"

#include "render/QuadRender.h"
#include "BaseGUIElement.h"
#include "..\utils\InterfaceUtils.h"

class BaseGUICursor;
class RenderAdvFont;

class GUIWidget : public BaseGUIElement
{
	struct Params
	{
	//	enum Align		{Left,Center,Right,Top,Bottom};
	//	typedef IGUIText::Align Align;
		typedef IAdvFont::Align Align;

		enum PlayMethod {Continious,OnceOnActivate,OnceOnShow,OnceByCommand};

	//	float width, height;		// размеры эл-та
		bool  useSafeFrame;
		
		long  drawPriority;			// порядок отрисовки

		// text
		const char *stringID;		// локализованная строчка
		string		defaultString;	// строчка по умолчанию

		const char *defaultPtr;

		const char *fontName;		// имя шрифта
		float		size;			// размер шрифта
		float		kerning;			// размер шрифта

		Color color;

		Align horizAlign;			// выравнивание по горизонтали
		Align vertAlign;			// выравнивание по вертикали

		// geometry
		const char *model;			// моделька
		const char *anim;			// анимация

		Vector position;			// положение
		Vector orient;				// ориентация

		float  scale;				// масштаб

		// picture
		const char *texture;		// текстура фона

		long  firstFrame;			// номер первого кадра
		long  framesCount;			// кол-во кадров анимации

		float frameStartU;			// смещение первого кадра
		float frameStartV;			//

		float frameWidth;			// ширина кадра
		float frameHeight;			// высота кадра

		long  fps;					// фпс

		PlayMethod playMethod;		// метод проигрывания
	};

	struct FogParams
	{
		float h_density;
		float h_min;
		float h_max;

		float d_density;
		float d_min;
		float d_max;

		Color color;
	};

	long drawShift;					// смещение уровня отрисовки
	
/*	ShaderId Circular_shadow_id;
	ShaderId Circular_id;*/

//	IGUIText	  *m_text;
	RenderAdvFont *m_text;

	Params		m_params;			// параметры

	string		m_initString;
	bool		m_needUpdate;

	const char *m_initPtr;

	///////////////////////

	bool m_stat;

	int m_statCur;
	int m_statEnd;

	float m_statTime;
	float m_statTimeMax;

	//// Render params ////

	FogParams fog;

	bool  skipFog;		// не учитывать туман при отрисовке модели

	dword drawColor;

	bool m_smoothAlpha;	// использовать мягкую альфу

	bool enableRender;	// разрешить рендер

	bool dynLighting;

	///////////////////////////

//	IBaseTexture *lastImage;		// текстура для контролов с предыдущего кадра
	int m_version;					// номер обновления текстуры контролов

//	BaseGUIElement *m_parent;		// родительский контрол (владелец)
//	long			m_validateHash;

	bool		m_black;			// черно-белое изображение

	float		m_alpha;			// прозрачность

	QuadRender	m_quadRender;		// рендерилка текстуреных квадов

	long		m_curFrame;			// текущий кадр анимации

	float		m_lastTime;			// время последней смены кадров анимации
	float		m_curTime;			// текущее время

	bool		m_shouldPlay;		// нужно ли проигрывать анимацию фона

	bool		m_fading;			// плавное появление/исчезновение
	bool		m_effect;

	dword		m_effBeg;			// часть строки, на которую
	dword		m_effLen;			// применяется спецэффект изменения

	dword		m_effLenNative;

	IGMXScene*	m_model;			// моделька
	IFont*		m_font;				// шрифт
	dword		m_linesCount;		// вол-во строчек текста

	float		m_fade;
	float		m_fadeTime;

	float		m_fadeAlpha;
	float		m_alphaOriginal;

	float		m_frameXOffset;		// смещение графики для 1 кадра
	float		m_frameYOffset;		// смещение графики для 1 кадра

//	float		m_za;

	float		m_effTime;			// проигрывание эффекта

	float		m_strOffX;			// позиция строки при последней отрисовке
	float		m_strOffY;

	float		m_effOffX;			// смещение подстроки для спецэффекта
	float		m_effWidth;			// смещение правого края подстроки

	float		m_asp;

	//// effect ////

	float		effectTime;
	float		effectSize;

	Color		effectColor;

	////////////////

	BaseGUICursor *m_cursor;

	long m_click;			// контролы
	long m_dblcl;			//

	float m_hk;				// коэффициент относительной высоты шрифта

	///////////////

	bool  m_cutsUse;		// использовать брейк-теги

	float m_cutsDelay;		// общее время показа титров
	long  m_cutsPriority;	// приоритет

	bool  m_cutsDebug;

	float m_cutsTime;

/*	struct Info
	{
		ImagePlace p;

		float x; float w;
		float y; float h;

		float k;

		bool anim;
	};*/

/*	array<Info> table;
	array<long> hash;*/

	bool m_parsed;

	bool m_useAnim;

	float m_time;
	float m_animSpeed;

	float m_fontScale;

	struct Vertex
	{
		Vector p;

		float tu;
		float tv;

		float al;
	};

	static char buffer[];

	void UpdateText(bool resetImage = false);

/*	void CreateBuffers();

	void UpdateTable(string &s);
	void UpdateHash (string &s);*/

	void Parse(string &s);

	void ReadMOPs(MOPReader &reader);
	void SkipMOPs(MOPReader &reader);

	void _cdecl InitFunc(float deltaTime, long level);
	void _cdecl Work	(float deltaTime, long level);

	void Restart(); // сбросить состояние

	bool RestartObject(MOPReader &reader);

	void RenderBackGround(const Rect &r/*, float deltaTime*/);
	void RenderGeometry	 (const Rect &r);
	void RenderText		 (const Rect &r);

	virtual void OnParentNotify(Notification event);

	virtual void OnAcquireFocus() { ; }
	virtual void OnLooseFocus()	  { ; }

	virtual void OnChildNotify(BaseGUIElement *p, ChildNotification event)
	{
		if( event == ChildSelected )
			NotifyParent(this,ChildSelected);
	}
/*
	float SC2UIRC(float t);
	float SS2UIRS(float size);
*/
	virtual void PostCreate()
	{
		MOSafePointer sp;

		FindObject(InterfaceUtils::GetCursorName(),sp);

		m_cursor = (BaseGUICursor *)sp.Ptr();

		m_click = Controls().FindControlByName("LeftMouseButton");
		m_dblcl = Controls().FindControlByName("LeftMouseDbl");
	}

public:

	void FogParamsSave()
	{
		if( skipFog )
		{
			Render().getFogParams(fog.h_density,fog.h_min,fog.h_max,fog.d_density,fog.d_min,fog.d_max,fog.color);
			Render().setFogParams(		   0.0f,fog.h_min,fog.h_max,		 0.0f,fog.d_min,fog.d_max,fog.color);
		}
	}

	void FogParamsRestore()
	{
		if( skipFog )
		{
			Render().setFogParams(fog.h_density,fog.h_min,fog.h_max,fog.d_density,fog.d_min,fog.d_max,fog.color);
		}
	}

	virtual void SetDrawUpdate()
	{
		SetUpdate((MOF_UPDATE)&GUIWidget::Work,drawLevel + m_params.drawPriority);

		m_fadeAlpha = m_alphaOriginal = GetNativeAlpha();
	}
	virtual void DelDrawUpdate()
	{
		DelUpdate((MOF_UPDATE)&GUIWidget::Work);

		m_fadeAlpha = m_alphaOriginal = GetNativeAlpha();
		SetFadeState(FadingFinished);
	}

	virtual void ResetDrawUpdate(long priorityShift)
	{
		drawShift = priorityShift;

		if( IsShow())
		{
			DelUpdate((MOF_UPDATE)&GUIWidget::Work);
			SetUpdate((MOF_UPDATE)&GUIWidget::Work,drawLevel + m_params.drawPriority + drawShift);
		}

		ChildDrawUpdate(priorityShift);
	}

	GUIWidget(void);
	virtual ~GUIWidget(void);

	void SetDefaultString(string &val)
	{
		m_params.defaultString = val;
	}

	// установить смещение графики виджета на 1 кадр
	void SetFrameOffset(float x, float y)
	{
		m_frameXOffset = x;
		m_frameYOffset = y;
	}

	virtual bool Create			(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Activate(bool isActive);

	virtual void Command(const char * id, dword numParams, const char ** params);

	MO_IS_FUNCTION(GUIWidget, BaseGUIElement)

	virtual void  Show(bool isShow);
			void _Show(bool isShow, bool animate = true);

	bool GetAnim()
	{
		return m_useAnim;
	}

	void SetAnim(bool en)
	{
		m_useAnim = en;
	}

	//////////

	long GetPriority()
	{
		return m_cutsPriority;
	}

	static float frac(float f);

public:

	void SetAlphaOriginal(float alpha)
	{
		m_alphaOriginal = alpha;
	}

	void SetFading(bool useFading)
	{
		m_fading = useFading;
	}

	void SetBlack(bool black)
	{
		m_quadRender.SetBlack(black);
	}

public:

	virtual void Draw(float x, float y, float alpha = 1.0f, bool inner = false);

	virtual void Draw();
	virtual void Update(float dltTime);

	virtual bool IsPlayingAnimation();

	virtual void SetFontScale(float k)
	{
		m_scale = m_fontScale = k;
	}

private:

	static float font_hk;

private:

	bool m_restart;			// выполняется рестарт объекта
	bool m_initComplete;	// была выполнена инициализация функцией InitFunc()

public:

/*	virtual void SizeTo(float w, float h)
	{
		BaseGUIElement::SizeTo(w,h);

		UpdateText();
	}*/

	bool Complete()
	{
		return !m_stat || m_statCur == m_statEnd;
	}







	static GUIWidget *_curText;
	static ICoreStorageFloat *_sub;
	static long _subUsers;
};
