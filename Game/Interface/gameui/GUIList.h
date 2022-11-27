#pragma once

#include "BaseGUIElement.h"
#include "render/QuadRender.h"

class csProgressBar;

class GUIWidget;
class GUIButton;

class BaseGUICursor;

class GUIList : public BaseGUIElement
{
	enum ListState {Normal,VScrolling,HScrolling };

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

	struct Item : public ListElement
	{


	//	bool isButton;

		MissionTrigger onFoc;
		MissionTrigger onLea;
		MissionTrigger onSel;
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

	Point m_from;			// начальная позиция скроллинга
	Point m_to;				// конечная  позиция скроллинга

	float m_fadeTime;
	float m_fade;

	float m_fadeAlpha;

	float m_alphaOriginal;	// оригинальная альфа для fade-in

	QuadRender m_helperQuad;

	ListState m_state;

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

	BaseGUICursor *m_cursor;

	void _cdecl InitFunc(float, long);

	void ScrollHor(float dx);
	void ScrollVer(float dy);

	void UpdatePos(bool init = false);

public:

	virtual void SetDrawUpdate() { SetUpdate(&GUIList::Work,drawLevel + drawPriority); }
	virtual void DelDrawUpdate() { DelUpdate(&GUIList::Work); }

	GUIList(void);
	virtual ~GUIList(void);

	virtual bool Create			(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual void Show	 (bool isShow);
	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(GUIList, BaseGUIElement);

public:

	virtual void Draw();
	virtual void Update(float dltTime);

private:

	void ReleaseItems();
	void   ResetItems();

private:

	array<Line> lines;

	float itemWidth;
	float itemHeight;

	Progress progs[2];
	Button	 butts[4];

	long startLine;
	long startItem;

	int curLine;
	int befLine;

	int curItem;
	int befItem; int maxItem;

	int maxWidth;

	float btnWidth;
	float btnHeight;

	bool centered;

	float scroll_x;
	float scroll_y;

	bool m_restart; // выполняется рестарт объекта

};
