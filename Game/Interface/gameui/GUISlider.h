#pragma once

#include "BaseGUIElement.h"
#include "Render/QuadRender.h"

class GUIWidget;

class GUISlider : public BaseGUIElement
{
	struct Params
	{
		Params() : links(_FL_)
		{
		}

		struct NavigationLink
		{
			const char *control;			// имя контрола
			const char *object;				// имя объекта для перехода

			float activeTime;
		};

		array<NavigationLink> links;

		bool initiallyFocused;
	};

	Params m_params;

	MissionTrigger onGetFocus;
	MissionTrigger onLosFocus;

	MissionTrigger onChange;
	MissionTrigger onTry;

private:

	void SkipMOPs(MOPReader &reader);
	void ReadMOPs(MOPReader &reader);

	void _cdecl InitFunc(float, long);
	void _cdecl Work(float deltaTime, long level);

	void Restart(); // сбросить состояние

	bool RestartObject(MOPReader &reader);

	virtual void PostCreate()
	{
		m_cursor = (BaseGUICursor *)FindObject("Cursor");
		m_baseCursor = m_cursor;

		m_click = Controls().FindControlByName("LeftMouseButton");

		m_left  = Controls().FindControlByName("Menu_Left");
		m_right = Controls().FindControlByName("Menu_Right");

	//	if( m_objectName[0] )
	//		m_object = (BaseGUIElement *)FindObject(m_objectName);

		m_val = api->Storage().GetItemFloat(m_val_name,_FL_);

		GetValue();
	}

	void SetValue();
	void GetValue();

	void StepUp();
	void StepDown();

	MissionObject *FindObject(const char *id)
	{
		MOSafePointer sp;

		Mission().FindObject(ConstString(id),sp);

		return sp.Ptr();
	}

public:

	virtual void OnLooseFocus  ();
	virtual void OnAcquireFocus();

	bool GoToLink(const char *control);

	void SetFocused();
	void SetNormal();

	bool ProcessKeyboard();

	////

	virtual void SetCursor();

	virtual void SetDrawUpdate() { SetUpdate(&GUISlider::Work,drawLevel + drawPriority); }
	virtual void DelDrawUpdate() { DelUpdate(&GUISlider::Work); }

	GUISlider(void);
	virtual ~GUISlider(void);

	virtual bool Create			(MOPReader &reader);
	virtual bool EditMode_Update(MOPReader &reader);

	virtual void Command(const char *id, dword numParams, const char **params);

	virtual void Show	 (bool isShow);
	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(GUISlider, BaseGUIElement);

public:

	virtual void Draw();
	virtual void Update(float dltTime);

private:

	long drawPriority;

	float btnWidth;
	float btnHeight;

	const char *backName;
	const char *foreName;

	const char *backNameSel;
	const char *foreNameSel;

	const char *backNameDis;
	const char *foreNameDis;

	GUIWidget *m_back;
	GUIWidget *m_fore;

	GUIWidget *m_back_sel;
	GUIWidget *m_fore_sel;

	GUIWidget *m_back_dis;
	GUIWidget *m_fore_dis;

	float m_al_back;
	float m_al_fore;

	float m_al_back_sel;
	float m_al_fore_sel;

	float m_al_back_dis;
	float m_al_fore_dis;

	BaseGUICursor *m_cursor;

	//// контролы ////

	long m_click;

	long m_left;
	long m_right;

	//////////////////

	float m_time;

	float m_timeMax;
	float m_timeMaxBeg;

	bool m_repeat;

	float m_px; // координаты курсора с предыдущего кадра
	float m_py;

	float m_ox; // оригинальная позиция бокса (до начала смещения)
	float m_oy;

	float m_lx; // координаты курсора последнего смещения
	float m_ly;

	float m_xx; // сохраненная текущая позиция бокса при сбросе
	float m_yy;

	bool m_reseted;

	float m_dx; // локальное смещение курсора внутри бокса
	float m_dy;

	float m_bx; // текущая позиция бокса
	float m_by;

	bool m_moved;

	//// смещение границ ползунка ////

	float priLo;
	float priHi;

	float secLo;
	float secHi;

	//////////////////////////////////

	const char *m_val_name;

	ICoreStorageFloat *m_val;

	float m_val_min;
	float m_val_max;

	float m_val_def;

	float m_val_step;
	float m_val_t;

	////

	bool m_vertical;

	const char *m_objectName;
	BaseGUIElement *m_object;

	float m_value;

/*	float m_min;
	float m_max;

	float m_step;*/

	////

	bool m_doNotCheckFocusChange; // не проверять переход фокуса в текущем кадре.
	float m_focusTime;

	bool m_autofocus;
	bool m_focusByPress;

	float m_dt;

	bool m_onMin;
	bool m_onMax;

	bool m_restart; // выполняется рестарт объекта

};
