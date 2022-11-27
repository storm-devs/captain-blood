#ifndef _XBOX
#ifndef GUI_PANEL
#define GUI_PANEL

#include "..\..\..\common_h\gui.h"

// Для навешивания обработчика приравниванием :)
#define OnBeforeDraw \
	t_OnBeforeDraw->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)

#define OnMouseDown \
	t_OnMouseDown->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)


class GUIPanel : public GUIControl
{
	
	
	
protected:
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:

	GUIPanel (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIPanel ();
	
	virtual void Draw ();

	GUIEventHandler* t_OnBeforeDraw;

	GUIEventHandler* t_OnMouseDown;
	

};



#endif

#endif