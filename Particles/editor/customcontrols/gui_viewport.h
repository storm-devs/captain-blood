//****************************************************************
#ifndef GUI_VIEWPORT
#define GUI_VIEWPORT

#include "..\..\..\common_h\gui.h"

extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnRender \
t_OnRender->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

class GUIViewPort : public GUIControl
{
	
	
	
protected:
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:


	GUIViewPort (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIViewPort ();
	
	virtual void Draw ();
	
	GUIEventHandler* t_OnRender;

	virtual void MouseUp (int button, const GUIPoint& pt);
	virtual void MouseMove (int button, const GUIPoint& pt);

};



#endif