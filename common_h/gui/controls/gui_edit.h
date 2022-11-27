#ifndef _XBOX
#ifndef GUI_EDIT
#define GUI_EDIT

#include "gui_control.h"
#include "gui_font.h"

#include "gui_events.h"
#include "gui_image.h"

#include "..\..\Templates\string.h"


extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnAdvNumChange \
	t_OnAdvNumChange->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)



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

// Для навешивания обработчика приравниванием :)
#define OnChange \
	t_OnChange->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

// Для навешивания обработчика приравниванием :)
#define OnAccept \
	t_OnAccept->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

class GUIEdit : public GUIControl
{
	
	
	
	
protected:
	

	bool PressedInside;
  int need_accept_message;  
	int same_keys_pressed;
	
	int ScrollOffset;
	
	float BlinkTime;
	
	int sCursor;
	
	int nSelStart;
	int nSelLength;
	
	
	int CursorPosition;
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
	void DeleteSelected ();
	
	void CalcScrollOffset ();
	
	int need_key_event;
	bool bAlreadyPoped;
	
public:
	
	int CursorInside;


	bool Flat;
	bool ChangeCursor;
	GUIImage Image;	
	
	int lastkeypressed; 
	float key_pressed_time;
	
	
	string Text;
	
	DWORD FontColor;
	DWORD SelectionColor;
	
	GUIEdit (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIEdit ();


	void CursorToEnd();

	
	virtual void Draw ();
	virtual void DrawEditBox (long nLeft, long nTop, long Width, long Height);
	
	virtual void OnMDown (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMUp (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMEnter ();
	
	virtual void OnMLeave ();
	
	virtual void OnKPressed (int key, dword hparam);
	
	
	GUIEventHandler* t_OnMouseDown;  
	
	GUIEventHandler* t_OnMouseUp;  
	
	GUIEventHandler* t_OnMouseEnter;  
	
	GUIEventHandler* t_OnMouseLeave;  
	
	
	GUIEventHandler* t_OnChange;  
	
	
	GUIEventHandler* t_OnAccept;  

	GUIEventHandler* t_OnAdvNumChange;  
	
	
	void SelectText (int From, int Length);
	

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIEdit") == 0) return true;
		return GUIControl::Is (className);
	}

	virtual void MouseUp (int button, const GUIPoint& pt);
	virtual void MouseMove (int button, const GUIPoint& pt);

/*	
	virtual bool DragOver (int MouseX, int MouseY, GUIControl* dragfrom)
	{
		return true;
	}
	
	virtual void DragDrop (int MouseX, int MouseY, GUIControl* dragfrom);
*/	
};



#endif

#endif