#ifndef _XBOX
#ifndef GUI_WINDOW
#define GUI_WINDOW

#include "gui_control.h"
#include "gui_font.h"
#include "gui_button.h"
#include "gui_image.h"

extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnClose \
t_OnClose->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

// Для навешивания обработчика приравниванием :)
#define OnBeforeClose \
t_OnBeforeClose->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)



class GUIWindow : public GUIControl
{
	
	DWORD color;
	DWORD title_color1;
	DWORD title_color2;
	
	
	DWORD title_caption_color;
	
	
	DWORD line_color1;
	DWORD line_color2;
	DWORD line_color3;
	
	
	int title_height;
	

	GUIButton* close_button;
	
	GUIPoint m_pressed;
	int draged;
	
protected: 
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:
	

	bool bMovable;
	bool DrawBackground;
	GUIImage* icon;
	DWORD dwWindowBackColor;

	
	GUIWindow (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIWindow ();
	
	virtual void Draw ();
	
	virtual void OnMDown (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMUp (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMEnter ();
	
	virtual void OnMLeave ();
	
	virtual void OnMMove (const GUIPoint& pt);
	
	
	virtual void MouseMove (int button, const GUIPoint& pt);
	
	void SetPosition (int pX, int pY);
	
	void SetWidth (int width);
	
	void SetHeight (int height);
	
	
public:

	
	bool bCloseCanceled;
	bool bPopupStyle;
	bool bSystemButton;
	
	void _cdecl Close (GUIControl* Sender);
	
	virtual void Show ();
	virtual void ShowModal ();



	GUIEventHandler* t_OnClose;
	GUIEventHandler* t_OnBeforeClose;

	virtual void SetScreenCenter ();



	virtual void UpdatePopupStyle ();


	
	
	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIWindow") == 0) return true;
		return GUIControl::Is (className);
	}

	void SetAllocID (int id);

	virtual bool BeforeClose ();
};



#endif

#endif