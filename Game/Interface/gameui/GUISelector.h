#pragma once

#include "BaseGUIElement.h"
#include "render/QuadRender.h"

class csProgressBar;

class GUIWidget;
class GUIButton;

class BaseGUICursor;

class GUISelector : public BaseGUIElement
{
	enum SelectorState {Normal,VScrolling,HScrolling };

	struct Point
	{
		float x; float y;
	};

	struct ListElement
	{
		ListElement()
		{
			p.Reset();
		}

		ConstString name;
		mutable MOSafePointer p;
	};	

	struct Item
	{
		struct Widget : public ListElement
		{
		};

		Widget normal;
		Widget closed;

		BaseGUIElement *q;

	//	bool isButton;

		MissionTrigger onFoc;
		MissionTrigger onLea;

		bool open;

		MissionTrigger onSel;
		MissionTrigger onTry;

		float a;
		float k;

		long draw;
	};

	struct Line
	{
		array<Item> items;

		Line() : items(_FL_)
		{
		}
	};

	struct Button : public ListElement
	{
	};

	struct Progress : public ListElement
	{
	};

	struct ButInfo
	{
		bool en;

		float time;
		float timeMax;

		float t;

		ButInfo()
		{
			en = false;
		}

		void start(float t)
		{
			en = true;

			time = 0.0f;
			timeMax = t;
		}
	};

private:

	void SkipMOPs(MOPReader &reader);
	void ReadMOPs(MOPReader &reader);

	void _cdecl Work(float deltaTime,long level);
	void _cdecl Draw(float deltaTime,long level);

	void Restart(); // сбросить состояние

	bool RestartObject(MOPReader &reader);

//	virtual void OnParentNotify(Notification event) {}

	virtual void OnChildNotify(BaseGUIElement *p, ChildNotification event);

	virtual void OnLooseFocus  () {}
	virtual void OnAcquireFocus() {}

	virtual float GetFullW()
	{
		float w = maxWidth*(itemWidth + horSpace);

	//	if( w < GetWidth())
	//		w = GetWidth();

		return w;
	}

	virtual float GetFullH()
	{
		float h = lines.Size()*(itemHeight + verSpace);

		if( h < GetHeight())
			h = GetHeight();

		return h;
	}

	Point m_from;			// начальная позиция скроллинга
	Point m_to;				// конечная  позиция скроллинга

	float m_xv;
	float m_xa;

	float m_yv;
	float m_ya;

	float m_tt;
	float m_tmax;

	float m_fadeTime;
	float m_fade;

	float m_fadeAlpha;

	float m_alphaOriginal;	// оригинальная альфа для fade-in

	QuadRender m_helperQuad;

	SelectorState m_state;

	float m_scrollTime;
	float m_scrollLength;

	float m_curScrollTime;

	float m_clipX;
	float m_clipY;
	float m_clipWidth;
	float m_clipHeight;

	Rect m_rect;			// занимаемая всеми дочерними элементами область

	bool m_useClipping;

	const char *m_texture;	// текстура фона
	float		m_texAlpha;	// прозрачность фона

	long drawPriority;		// порядок отрисовки

	QuadRender m_quadRender;

	MOSafePointer m_cursor;

	void _cdecl Pre_Func(float, long);
	void _cdecl InitFunc(float, long);

	void ScrollHor(float dx);
	void ScrollVer(float dy);

	void UpdatePos(bool left, bool up, bool init = false, bool onFocActivate = true);

	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();

	void MoveFocus  (int i, int j, bool trigger);
	void OpenElement(int i, int j, bool open);

public:

	virtual void SetDrawUpdate() { SetUpdate(&GUISelector::Work,drawLevel + drawPriority); }
	virtual void DelDrawUpdate() { DelUpdate(&GUISelector::Work); }

	GUISelector(void);
	virtual ~GUISelector(void);

	virtual bool Create			(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual void Show	 (bool isShow);
	virtual void Activate(bool isActive);

	virtual void PostCreate();

	MO_IS_FUNCTION(GUISelector, BaseGUIElement);

public:

	virtual void Draw();
	virtual void Update(float dltTime);

private:

	void ReleaseItems();
	void   ResetItems();

	void UpdateElements(float deltaTime, bool instant);

private:

	array<Line> lines;

	float itemWidth;
	float itemHeight;

	float horSpace;
	float verSpace;

	Progress progs[2];
	Button	 butts[4];

	long startLine;
	long startItem;

	int curLine;
	int befLine;

	int curItem;
	int befItem; int maxItem;

	ConstString markerName;

	MOSafePointer marker;

	float markerScale;

	int maxWidth;

	float btnWidth;
	float btnHeight;

	bool centered;

	float scroll_x;
	float scroll_y;

	float m_dx; // осталось проскроллировать по x
	float m_dy; // осталось проскроллировать по y

	//// контролы ////

	long m_up;
	long m_down;

	long m_left;
	long m_right;

	long m_sel;

	//////////////////

	ButInfo btU;
	ButInfo btD;
	ButInfo btL;
	ButInfo btR;

	array<float> m_scales;
	array<float> m_alphas;

	array<long> m_drawps;

	float m_scaleSpeed;
	float m_scrolSpeed;

	float m_delTime;

	float m_begTime;
	float m_endTime;

	float m_accTime;

	bool m_bottom;	// content align (center/bottom)

	bool m_silentActivate;

	bool m_restart;			// выполняется рестарт объекта
	bool m_initComplete;	// выполнена инициализация функцией InitFunc()

};
