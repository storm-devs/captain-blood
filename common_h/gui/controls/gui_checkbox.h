#ifndef _XBOX
#ifndef GUI_CHECKBOX
#define GUI_CHECKBOX

#include "gui_control.h"
#include "gui_font.h"

#include "gui_events.h"
#include "gui_image.h"

extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnChange \
t_OnChange->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)



class GUICheckBox : public GUIControl
{
	
	
	
protected:
	
	float MD_Time;
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:
	
	bool Checked;
	
	DWORD FontColor;
	
	GUIImage* ImageNormal;
	GUIImage* ImageChecked;
	
	
	virtual void OnMDown (int MouseKey, const GUIPoint& pt);
	
	
	GUICheckBox (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUICheckBox ();
	
	virtual void Draw ();
	
	
	GUIEventHandler* t_OnChange;
	
	
	
	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUICheckBox") == 0) return true;
		return GUIControl::Is (className);
	}

	
};



#endif

#endif