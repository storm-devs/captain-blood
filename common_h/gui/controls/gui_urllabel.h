#ifndef _XBOX
#ifndef GUI_URLLABEL
#define GUI_URLLABEL

#include "gui_control.h"
#include "gui_font.h"

#include "gui_events.h"
#include "gui_image.h"


extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnMouseDown \
t_OnMouseDown->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

// Для навешивания обработчика приравниванием :)
#define OnMouseUp \
t_OnMouseUp->GetThis (hack_temp_ptr); \
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



class GUIURLLabel : public GUIControl
{
	
	
	
protected:
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:
	
	DWORD FontColor;
	
	DWORD SelectedFontColor;
	
	GUIURLLabel (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIURLLabel ();
	
	virtual void Draw ();
	
	virtual void OnMDown (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMUp (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMEnter ();
	
	virtual void OnMLeave ();
	
	
	GUIEventHandler* t_OnMouseDown;  
	
	GUIEventHandler* t_OnMouseUp;  
	
	GUIEventHandler* t_OnMouseEnter;  
	
	GUIEventHandler* t_OnMouseLeave;  
	

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIURLLabel") == 0) return true;
		return GUIControl::Is (className);
	}

};



#endif

#endif