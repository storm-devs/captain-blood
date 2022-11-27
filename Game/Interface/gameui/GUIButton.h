#pragma once
#include "BaseGUIElement.h"
#include "render/QuadRender.h"

class GUIWidget;
class BaseGUICursor;

class GUIButton : public BaseGUIElement
{
public:

	struct Params
	{
		Params() :
			links  (_FL_),
			hotKeys(_FL_)
		{
		}

	//	float w,h;

		const char *description;			// описание кнопки
		ConstString descObject;				// имя объекта выводящего описание

		ConstString staticWidget;			// общая графика (видна всегда)
		ConstString selectedButtonWidget;	// графика состояния "mouse over"
		ConstString normalButtonWidget;		// графика состояния "normal"
		ConstString focusButtonWidget;		// графика состояния "focused"
		ConstString pressedButtonWidget;	// графика состояния "pressed"
		ConstString disabledButtonWidget;	// графика состояния "disabled"

		float pressXoffset;
		float pressYoffset;

		struct NavigationLink
		{
			const char *control;			// имя контрола
			ConstString object;				// имя объекта для перехода

			float activeTime;
		};

		struct HotKey
		{
			const char *hotKey;
			bool		onlyInFocus;
		};

		array<NavigationLink> links;
		array<HotKey>		  hotKeys;

		bool initiallyFocused;

		MissionTrigger buttonEvent;			// Событие нажатия на активную	 кнопку
	//	MissionTrigger inactiEvent;			// Событие нажатия на неактивную кнопку
	};

	MissionTrigger onGetFocus;
	MissionTrigger onLosFocus;

private:

	BaseGUICursor *m_cursor; long m_cursorHash;

	long m_validateHash;
	bool m_wasPressed;						// флаг нажатия
	bool m_wasReleased;						// флаг отпускания
	bool m_doNotCheckFocusChange;			// не проверять переход фокуса в текущем кадре.

	float m_focusTime;

	Params m_params;

	bool m_autorepeat;						// признак возможности автоповтора
	bool m_autofocus;						// автоматический переход фокуса

	GUIWidget *m_staticWidget;
	GUIWidget *m_selectedWidget;
	GUIWidget *m_normalWidget;
	GUIWidget *m_focusedWidget;
	GUIWidget *m_pressedWidget;
	GUIWidget *m_disabledWidget;

	GUIWidget *m_activeWidget;

	enum State {stStatic,stNormal,stFocused,stPressed,stDisabled};

	State m_state;

	bool m_release;			// срабатывать при отжатии

	bool m_focusByPress;

	float m_pressTime;		// время анимации нажатия/отжатиия кнопки
	float m_pressDelay;		// время до автоповтора нажатия кнопки

	bool m_repeat;			// признак состояния автоповтора
	bool m_pressed;			// была ли нажата кнопка

	bool m_mouseOver;

	void SetState(State val)
	{
		m_state = val;
	}

	long drawPriority;

	GUIWidget* m_tooltipWidget;
	QuadRender m_helperQuad;

	bool m_restart;			// выполняется рестарт объекта
	bool m_initComplete;	// была выполнена инициализация функцией InitFunc()

	virtual void OnParentNotify(BaseGUIElement::Notification);

public:

	virtual void OnLooseFocus();
	virtual void OnAcquireFocus();

private:

	void OnHotKey();

	void ReadMOPs(MOPReader& reader);
	void SkipMOPs(MOPReader& reader);

	void _cdecl Work(float deltaTime, long level);
	void _cdecl InitFunc(float, long);

	void Restart(); // сбросить состояние

	bool RestartObject(MOPReader &reader);

	virtual void UnRegister(BaseGUIElement* object);

	void SetDisabled();
	void SetFocused();
	void SetNormal(bool reset = false);
	void SetPressed();

	void SetSelected();

	bool ProcessKeyboard();
	void ProcessMouse();

	MissionObject *FindObject(const ConstString & id)
	{
		MOSafePointer sp;

		Mission().FindObject(id,sp);

		return sp.Ptr();
	}

public:

	bool GoToLink(const char *control);

public:

	virtual void SetDrawUpdate() { SetUpdate(&GUIButton::Work,drawLevel); }
	virtual void DelDrawUpdate() { DelUpdate(&GUIButton::Work); }

	GUIButton(void);
	virtual ~GUIButton(void);

	virtual bool Create			(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Show	 (bool isShow);
	virtual void Activate(bool isActive);

	virtual void Command(const char *id, dword numParams, const char **params);

	MO_IS_FUNCTION(GUIButton, BaseGUIElement);

public:

	virtual void SetFocus()
	{
		SetFocused();
	}

public:

	virtual void Draw();
	virtual void Update(float dltTime);

};
