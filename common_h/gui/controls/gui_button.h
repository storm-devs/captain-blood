#ifndef _XBOX
#ifndef GUI_BUTTON
#define GUI_BUTTON

#include "gui_control.h"
#include "gui_font.h"

#include "gui_events.h"
#include "gui_image.h"



enum GUIButtonLayout
{
	GUIBUTTONLAYOUT_GlyphLeft = 0,
	GUIBUTTONLAYOUT_GlyphRight,
	GUIBUTTONLAYOUT_GlyphTop,
	GUIBUTTONLAYOUT_GlyphBottom,

	GUIBUTTONLAYOUT_FORCE_DWORD = 0x7fffffff
};



extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnMousePressed \
t_OnMousePressed->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define OnMouseClick \
t_OnMouseClick->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)



// Для навешивания обработчика приравниванием :)
#define OnMouseDown \
t_OnMouseDown->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


// Для навешивания обработчика приравниванием :)
#define OnMouseEnter \
t_OnMouseEnter->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

// Для навешивания обработчика приравниванием :)
#define OnMouseLeave \
t_OnMouseLeave->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)




class GUIButton : public GUIControl
{



	DWORD color;

	DWORD line_color1;
	DWORD line_color2;
	DWORD line_color3;

	float MD_Time;

	int MousePressed;

	int TempDown;
	
	int ButtonIsPressed;

protected:
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

public:

	DWORD FlatButtonActiveColor;
	DWORD FlatButtonPassiveColor;
	DWORD FlatButtonPressedColor;
	bool FlatButton;
  bool AutoRelax;
	bool Down;
	bool bEnabled;


	int GroupIndex;


	GUIAlignment Align;

	DWORD FontColor;
	DWORD DisabledFontColor;

	GUIImage* Glyph;
	GUIImage* DisabledGlyph;

	GUIButtonLayout Layout;

	GUIButton (GUIControl* pParent, int Left, int Top, int Width, int Height);

	virtual ~GUIButton ();

	virtual void Draw ();

	virtual void DrawButtonDown (int Left, int Top, int Width, int Height);

	virtual void DrawButtonUp (int Left, int Top, int Width, int Height);

	virtual void OnMDown (int MouseKey, const GUIPoint& pt);

	virtual void OnMUp (int MouseKey, const GUIPoint& pt);

	virtual void OnMEnter ();

	virtual void OnMLeave ();

	virtual void MouseUp (int button, const GUIPoint& pt);

public:
	virtual void OnPressed ();

	GUIEventHandler* t_OnMouseClick;
	GUIEventHandler* t_OnMousePressed;
	//  virtual void SetOnPressed(CONTROL_EVENT event_handler, GUIControl* handler_class);

	GUIEventHandler* t_OnMouseEnter;  

	GUIEventHandler* t_OnMouseLeave;  

	GUIEventHandler* t_OnMouseDown;


	virtual void ChangePos (int posX, int posY);

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIButton") == 0) return true;
		return GUIControl::Is (className);
	}
};



#endif

#endif