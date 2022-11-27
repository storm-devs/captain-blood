#pragma once

#include "BaseGUIElement.h"
#include "render/QuadRender.h"

class csProgressBar;

class GUIPanel : public BaseGUIElement
{
	enum PanelState {Normal,VScrolling,HScrolling };

	struct Point
	{
		float x; float y;
	};

private:

	void ReadMOPs(MOPReader &reader);

	void _cdecl Work(float deltaTime,long level);

//	virtual void OnParentNotify(Notification event) {}

	virtual void OnChildNotify(BaseGUIElement *p, ChildNotification event)
	{
		if( event == ChildSelected )
			NotifyParent(this,ChildSelected);
	}

	virtual void OnLooseFocus  () {}
	virtual void OnAcquireFocus() {}

	Point m_from;			// начальная позиция скроллинга
	Point m_to;				// конечная  позиция скроллинга

	float m_fadeTime;
	float m_fade;

	float m_fadeAlpha;

	float m_alphaOriginal;	// оригинальная альфа для fade-in

	QuadRender m_helperQuad;

	PanelState m_state;

	float m_scrollTime;
	float m_scrollLength;

	float m_curScrollTime;

	float preTime;
	float preTimeMax;

	Point preFrom;
	Point preTo;

	float m_clipX;
	float m_clipY;
	float m_clipWidth;
	float m_clipHeight;

	Rect m_rect;			// занимаемая всеми дочерними элементами область

	long drawPriority;		// порядок отрисовки

	csProgressBar *m_hor;	// гор прогресс-бар
	csProgressBar *m_ver;	// вер прогресс-бар

	bool m_useClipping;

	const char *m_texture;	// текстура фона
	float		m_texAlpha;	// прозрачность фона

	QuadRender m_quadRender;

	bool m_show;

	void _cdecl InitFunc(float, long);

public:

	virtual void SetDrawUpdate() { SetUpdate(&GUIPanel::Work,drawLevel + drawPriority); }
	virtual void DelDrawUpdate() { DelUpdate(&GUIPanel::Work); }

	GUIPanel(void);
	virtual ~GUIPanel(void);

	virtual bool Create			(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual void Show	 (bool isShow);
	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(GUIPanel, BaseGUIElement);

	void Restart();

public:

	virtual void Draw();
	virtual void Update(float dltTime);

};
