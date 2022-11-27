#ifndef _XBOX
#ifndef GUI_MEMO
#define GUI_MEMO

#include "gui_control.h"
#include "gui_font.h"
#include "gui_stringlist.h"
#include "gui_scrollbar.h"

#include "gui_events.h"
#include "gui_image.h"

#include "..\..\Templates\string.h"


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

// Для навешивания обработчика приравниванием :)
#define OnChange \
t_OnChange->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


class GUIMemo : public GUIControl
{
	
	
	
	
protected:
	
	
	GUIScrollBar* sbHoriz;
	GUIScrollBar* sbVert;
	
	
	int SelectFromY;
	int SelectSizeY;
	int MultiLine;
	
	
	int same_keys_pressed;
	
	int ScrollOffset;
	
	int OffsetY;
	
	float BlinkTime;
	
	int sCursor;
	
	int nSelStart;
	int nSelLength;
	int nSelEnd;
	
	int CursorInside;
	
	int CursorPosition;
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
	void DeleteSelected ();
	
	void CalcScrollOffset ();
	
	void CalcYOffset ();
	
	int need_key_event;
	
	int cur_line;
	
	
	
	void CopyToClipboard ();
	void PasteFromClipboard ();
	void DeleteToClipboard ();
	
	void ResetMultiline ();
	
	
	void UpdateScrollBars ();
	
public:
	
	
	int lastkeypressed; 
	float key_pressed_time;
	
	GUIStringList Lines;
	
	
	DWORD FontColor;
	
	GUIMemo (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIMemo ();
	
	virtual void Draw ();
	
	virtual void OnMDown (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMUp (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMEnter ();
	
	virtual void OnMLeave ();
	
	virtual void OnKPressed (int key);
	
	
	GUIEventHandler* t_OnMouseDown;  
	
	GUIEventHandler* t_OnMouseUp;  
	
	GUIEventHandler* t_OnMouseEnter;  
	
	GUIEventHandler* t_OnMouseLeave;  
	
	
	GUIEventHandler* t_OnChange;  
	
	void _cdecl sbHorizChange (GUIControl* sender);
	
	void _cdecl sbVertChange (GUIControl* sender);
	
	void _cdecl OnStrAdd (GUIControl* sender);
	
	
	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIMemo") == 0) return true;
		return GUIControl::Is (className);
	}

	
};



#endif

#endif