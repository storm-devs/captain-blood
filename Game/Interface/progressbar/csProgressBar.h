#ifndef COOL_THINGS_PROGRESS_BAR
#define COOL_THINGS_PROGRESS_BAR

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\common_h\QSort.h"

#include "..\GameUI\BaseGUIElement.h"
#include "..\GameUI\Render\QuadRender.h"

class csProgressBar : public BaseGUIElement
{
	struct Vertex
	{
		float x,y,z;
		float a,u,v;
	};


	ShaderId GUI_Quad_id;

	void Calculate(float x, float y, float w, float h, Vertex &v, float ang, float a, float wk);

	struct Params
	{
	//	float	fX;
	//	float	fY;

	//	float	fWidth;
	//	float	fHeight;

		float	fStartBar;
		float	fEndBar;

		float	fMaxValue;

		ConstString	HPObject;
		string	HPObjectStr;

		bool	doShowEffect;

		long	drawPriority;
	};
	
	Params m_params;

	const char *back_t;
	const char *fron_t;

	MOSafePointer m_pHPObject;
	
	float m_fPercent;

	const char *m_res;

	float parent_bottom;
	
	float m_fValue;
	float m_fPrevValue;

	float m_addDecEffectTime;
	float m_showEffectTime;

	float m_addDecEffectStartU;
	float m_addDecEffectEndU;

	enum EffectType {
		AddHP	= 1,
		DecHP	= 2,
		ShowBar = 4};

	dword m_effects;

	bool m_showAddDecEffect;

	float MaxAddDecEffectTime;
	float MaxShowEffectTime;

	float m_X, m_Y;

	bool m_bottom;

	float fMinBorder;
	float fMaxBorder;

	QuadRender  m_backRender;
	QuadRender m_frontRender;
	
	void DrawBar(float wk, float x, float y, float w, float h, float beg, float end, float alpha, float xs, float ys);

	void _cdecl Work(float fDeltaTime, long level);

	void Draw(float wScale, float hScale, float fMaximalVal, float globalAlpha);

	void _cdecl InitFunc(float deltaTime, long level);

	void Update();

	bool circular;
	bool reverse;	// инвертирует направление указателя в круговом режиме

	bool vertical;

	bool inverse;	// зеркалит по вертикали индикатор в круговом режиме

	IRender	  *pRS;
	IVariable *barTexture;

	ITexture *texture;

	bool m_restart;	// выполняем рестарт объекта

public:

	virtual void OnAcquireFocus() {}
	virtual void OnLooseFocus  () {}

	void OnParentNotify(Notification event);

	// извращенный расчет ректа
	virtual void GetRect(Rect &rect, bool useOffset = true) /*const*/;

public:

	virtual void SetDrawUpdate() { SetUpdate(&csProgressBar::Work,drawLevel + m_params.drawPriority); }
	virtual void DelDrawUpdate() { DelUpdate(&csProgressBar::Work); }

	csProgressBar();
	virtual ~csProgressBar();

	// Вызывается в режиме игры после создания всех объектов
	virtual void PostCreate();

	// Создание объекта
	virtual bool Create			(MOPReader & reader);
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Show(bool isShow);

	MO_IS_FUNCTION(csProgressBar, BaseGUIElement);

	virtual void Command(const char *id, dword numParams, const char **params);

	virtual void Restart();

	float ScaleToBorder(float hp);

public:

	void SetMax(float val);
	void SetPos(float val);

public:

	virtual void Draw();
	virtual void Update(float dltTime);

public:

	virtual void GetPosition(float &x, float &y)
	{
		x = m_X*0.01f/m_aspect;
		y = m_Y*0.01f;
	}

	virtual void SetPosition(float  x, float  y)
	{
		m_X = x*100.0f;
		m_Y = y*100.0f;
	}

	virtual void GetSize(float &w, float &h)
	{
		GetNativeSize(w,h);

		w *= 0.01f;
		h *= 0.01f;
	}

	virtual void SetSize(float  w, float  h)
	{
		SizeTo(w*100.0f,h*100.0f);
	}
/*
public:

	virtual void SetAlign(Layout align)
	{
		m_layout = align;

		Update();
	}
*/
};

#endif
