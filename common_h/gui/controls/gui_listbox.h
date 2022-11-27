#ifndef _XBOX
#ifndef GUI_LISTBOX
#define GUI_LISTBOX

#include "gui_control.h"
#include "gui_font.h"
#include "gui_stringlist.h"
#include "gui_scrollbar.h"

#include "gui_events.h"
#include "gui_image.h"

#include "..\..\Templates\string.h"


// Для навешивания обработчика приравниванием :)
#define OnMouseDown \
t_OnMouseDown->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

// Для навешивания обработчика приравниванием :)
#define OnMouseDBLClick \
t_OnMouseDBLClick->GetThis (hack_temp_ptr); \
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


class GUIListBox : public GUIControl
{
	
	int OffsetY;
	
	
	
	
	float key_pressed_time;
	
	int m_nMouseWeelUpCounter;
	int m_nMouseWeelDownCounter;
	
protected:


	
	
	GUIScrollBar* sbVert;
	
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
	void CalcYOffset ();
	
	int CursorInside;
	
	void UpdateScrollBars ();

	string SearchString;
	
public:
	
	bool bCaptureKeyboard;
	bool AutoSelectMode;
	
	int SelectedLine;
	GUIStringList Items;
	DWORD FontColor;
	DWORD SelectColor;

	array <GUIImage> Images;
	
	GUIListBox (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIListBox ();
	
	virtual void OnMDown (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMUp (int MouseKey, const GUIPoint& pt);
	
	virtual void OnMEnter ();
	
	virtual void OnMLeave ();
	
	virtual void OnKPressed (int key, dword hparam);
	
	virtual void Draw ();
	virtual void DrawListBox (long nLeft, long nTop, long Width, long Height);
	
	void _cdecl sbVertChange (GUIControl* sender);
	
	void _cdecl OnStrAdd (GUIControl* sender);

	void _cdecl ListBeforeClear (GUIControl* sender);
	
	
	GUIEventHandler* t_OnMouseDown;  
	
	GUIEventHandler* t_OnMouseUp;  
	
	GUIEventHandler* t_OnMouseEnter;  
	
	GUIEventHandler* t_OnMouseLeave;  

	GUIEventHandler* t_OnChange;  

	GUIEventHandler* t_OnMouseDBLClick;  

	void SetDrawRect (const GUIRectangle& rect);

	virtual void MakeFlat (bool Flat);
	

	/* Получить строку находящуюся под координатами pt (в screen space)*/
	virtual int GetElementNumAtPos(const GUIPoint& pt);

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GuiListBox") == 0) return true;
		return GUIControl::Is (className);
	}

};



#endif

#endif