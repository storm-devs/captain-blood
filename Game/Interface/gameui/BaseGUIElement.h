#pragma once

#include "..\..\..\common_h\IGUIElement.h"

class BaseGUIElement;
class BaseGUICursor;

class GUIFocusHolder : public MissionObject
{
	BaseGUIElement *m_focusedElement;

public:

	GUIFocusHolder()
	{
		m_focusedElement = null;
	}

	void SetFocused(BaseGUIElement *p)
	{
		m_focusedElement = p;
	}

	BaseGUIElement *GetFocused() const
	{
		return m_focusedElement;
	}

};

// базовый класс элементов GUI
class BaseGUIElement : public IGUIElement
{
public:

	// запретить явное (в редакторе) изменение параметров объекта
	enum EditRestricts
	{
		preserveNone   = 0,
		preservePos	   = 1,
		preserveSize   = 2,
		preserveParent = 4,
		preserveLayout = 8
	};

	struct Rect
	{
		float l;
		float t;
		float r;
		float b;
	};

	struct Point
	{
		float x;
		float y;
	};

	struct Size
	{
		float w;
		float h;
	};

	enum FadeState {FadingIn,FadingFinished,FadingOut};
//	enum Layout	   {OnCenter,OnLeft,OnRight};

	struct Params
	{
		float x,y;
		float w,h;

		float alpha;

		ConstString parent;
	};

	bool nativeShow; long drawLevel;

	struct Child
	{
		Child()
		{
			object.Reset();
		}

		Child(BaseGUIElement *p)
		{
			Assert(p)

		//	p->FindObject(p->GetObjectID().c_str(),object);
			p->BuildSafePointer(p,object);
		}

	/*	bool operator ==(const Child &other) const
		{
			return object == other.object;
		}*/

		MOSafePointer object;
	};

	short data_a;
	short data_b;

private:

	Params			m_params;		// параметры

	MOSafePointer	m_parent;		// родитель

	array<Child>	m_children;		// потомки

	FadeState		m_fadeState;
	MOSafePointer	m_focusHolder;	// объект для получения элемента в фокусе

	void ReadMOPs(MOPReader &reader);

	bool m_clipping;

	dword m_restricts;		// ограничения

protected:

	bool m_isLooped;

	Point m_natPos;			// оригинальные размеры элемента
	Size  m_natSize;		// заданные в редакторе

	Layout m_natLay;

	BaseGUICursor *m_baseCursor;

	float m_offsetX;		// относительное смещение дочерних
	float m_offsetY;		// элементов при скроллинге

	float m_scale;			// коэффициент масштабирования

	float m_aspect;			// аспект с учетом подгонки элементов на широкий экран 16:9
	float m_aspect_native;	// аспект

	bool m_shiftAspect;
	bool m_widthAspect;
	float m_widthAspectKoef;

	Layout m_layout;

	void InitAspect();		// посчитать текущий аспект

protected:

	void SkipMOPs(MOPReader &reader);

	virtual float GetFullW() { return m_params.w; }
	virtual float GetFullH() { return m_params.h; }

	virtual void SetDrawUpdate() = 0;
	virtual void DelDrawUpdate() = 0;

	// список событий от родителя
	enum Notification {ParentShowed,ParentHided,ParentDeleted,ParentChanged,SilentShow,SilentHide};

	// обработчик событий от родителя
	virtual void OnParentNotify(Notification event);

	// броадкастер событий потомкам
	void NotifyChildren(Notification event);

	enum ChildNotification {ChildSelected,ChildClicked,ChildDblClicked};

	virtual void OnChildNotify(BaseGUIElement *p, ChildNotification event) {}

	void NotifyParent(BaseGUIElement *p, ChildNotification event);

	// движение элемента
	void Move(float dx, float dy) { m_params.x += dx; m_params.y += dy; }

	void SetFadeState(FadeState state)
	{
		m_fadeState = state;
	}

	BaseGUIElement *FindParent();

	// скроллинг дочерних элементов
	void Scroll  (float dx, float dy) { m_offsetX += dx; m_offsetY += dy; }
	void ScrollTo(float  x, float  y) { m_offsetX  =  x; m_offsetY  =  y; }

	// accessor'ы для доступа к элементу в фокусе
	void SetElementInFocus(BaseGUIElement *p);
	BaseGUIElement *GetElementInFocus();

	GUIFocusHolder *GetFocusHolder();

	void GetClipRect	  (Rect &rect);
	void GetParentClipRect(Rect &rect);

	void SetClipping(bool isEnabled)
	{
		m_clipping = isEnabled;
	}

	bool Restricted(dword what)
	{
		return (m_restricts&what) != 0;
	}

	void ValidateParent();

public:

	BaseGUIElement(void);
	virtual ~BaseGUIElement(void);

	void Notify(Notification event)
	{
		OnParentNotify(event);
	}

/*	float GetAlpha()
	{
		return m_params.alpha;
	}*/

	void SetAlpha(float alpha)
	{
		m_params.alpha = alpha;
	}

	void SetParent(const ConstString & parent);

	void SetClipRect(Rect *r);

	void Restrict(dword what)
	{
		m_restricts = what;
	}

	float GetWidth()
	{
		return m_params.w;
	}

	float GetHeight()
	{
		return m_params.h;
	}

	float GetX(bool useOffset = true) /*const*/;
	float GetY(bool useOffset = true) /*const*/;

	float GetOffsetX() const { return m_offsetX; }
	float GetOffsetY() const { return m_offsetY; }

//	virtual void GetParentRect(RECT & rect) const;

//			void GetRect(Rect &rect, bool useOffset = true) /*const*/;
	virtual void GetRect(Rect &rect, bool useOffset = true) /*const*/;

	void GetParentRect(Rect &rect, bool useOffset = true) /*const*/;

	void GetPanelRect(Rect &rect);

	float GetAlpha();
	float GetNativeAlpha()
	{
		return m_params.alpha;
	}

	BaseGUIElement *GetParent()// const
	{
		m_parent.Validate();

		return (BaseGUIElement *)m_parent.Ptr();
	}

	bool ParentIsActive();

	FadeState GetFadeState() const { return m_fadeState; }

	bool HaveFocus()
	{
		return this == GetElementInFocus();
	}

	// вызывается для s_inFocus при получении им фокуса
	virtual void OnAcquireFocus() = 0;
	// вызывается для s_inFocus при потере им фокуса
	virtual void OnLooseFocus() = 0;

	// регистрация/отрегистрация дочерних элементов
	virtual void   Register(BaseGUIElement* object);
	virtual void UnRegister(BaseGUIElement* object);

	virtual void UnRegisterAll()
	{
		m_children.DelAll();
	}

	void GetNativePos (float &x, float &y)
	{
		x = m_params.x;
		y = m_params.y;
	}

	void GetNativeSize(float &w, float &h)
	{
		w = m_params.w;
		h = m_params.h;
	}

	float GetNativeX() { return m_params.x; }
	float GetNativeY() { return m_params.y; }

	// движение элемента
	void MoveTo(float x, float y) { m_params.x = x; m_params.y = y; }
	void SizeTo(float w, float h) { m_params.w = w; m_params.h = h; }

/*	long GetDrawLevel() const
	{
		if( Mission().ValidatePointer(m_parent,m_parentHash))
			return m_parent->GetDrawLevel() + 100;
		else
			return drawLevel;
	}*/

	void SetShiftAspect(bool on) { m_shiftAspect = on; }
	void SetWidthAspect(bool on) { m_widthAspect = on; }

	Layout GetLayout()
	{
		return m_layout;
	}

	void SetLayout(Layout val)
	{
		m_layout = val;
	}

	/////////////////////////////////////////////////////////
	// реализация/переопределение наследства MissionObject //
	/////////////////////////////////////////////////////////

	virtual bool Create			(MOPReader &reader);
	virtual bool EditMode_Update(MOPReader &reader);

	virtual void PostCreate();

	virtual void Show	 (bool isShow);
	virtual void Activate(bool isActive);

	virtual void Command(const char *id, dword numParams, const char **params);

	MO_IS_FUNCTION(BaseGUIElement, IGUIElement);

public:

	bool PointInRect(float x, float y, const Rect &r);
	bool RectInRect(
		float l1, float t1, float r1, float b1,
		float l2, float t2, float r2, float b2);

	Rect GetFullRect();

public:

	virtual void ResetDrawUpdate(long priorityShift)
	{
		DelDrawUpdate();
		SetDrawUpdate();

	//	ChildDrawUpdate(priorityShift);
	}

	virtual void ChildDrawUpdate(long priorityShift)
	{
		for( dword i = 0; i < m_children.Size() ; i++ )
		{
			if( m_children[i].object.Validate())
				((BaseGUIElement *)m_children[i].object.Ptr())->ResetDrawUpdate(priorityShift + 1);
		}
	}

	virtual void SetFocus() {};

public:

	virtual void GetPosition(float &x, float &y)
	{
		x = m_params.x*0.01f;
		y = m_params.y*0.01f;
	}

	virtual void SetPosition(float  x, float  y)
	{
		MoveTo(x*100.0f,y*100.0f);

		NotifyChildren(ParentChanged);
	}

	virtual void GetSize(float &w, float &h)
	{
		w = m_params.w*0.01f;
		h = m_params.h*0.01f;
	}

	virtual void SetSize(float  w, float  h)
	{
		SizeTo(w*100.0f,h*100.0f);
	}

	////

	virtual void GetNatPosition(float &x, float &y)
	{
		x = m_natPos.x*0.01f;
		y = m_natPos.y*0.01f;
	}

	virtual void GetNatSize	   (float &w, float &h)
	{
		w = m_natSize.w*0.01f;
		h = m_natSize.h*0.01f;
	}

	virtual void GetNatRect    (float &x, float &y, float &w, float &h)
	{
		x = m_natPos.x;
		y = m_natPos.y;

		w = m_natSize.w;
		h = m_natSize.h;
	}

	virtual Layout GetNatAlign()
	{
		return m_natLay;
	}

	////

public:

	virtual Layout GetAlign()
	{
		return GetLayout();
	}

	virtual void SetAlign(Layout align)
	{
		SetLayout(align);
	}

	virtual float GetAspect()
	{
		return m_aspect;
	}

public:

	virtual void SetCursor();

};
