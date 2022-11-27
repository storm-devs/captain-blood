#ifndef _XBOX
#ifndef GUI_SCROLLBAR
#define GUI_SCROLLBAR

#include "gui_control.h"
#include "gui_button.h"



extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnChange \
t_OnChange->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)



enum GUIScrollBarKind
{
	GUISBKIND_Horizontal    = 0,
	GUISBKIND_Vertical,
	
	GUISBKIND_FORCE_DWORD = 0x7fffffff
};


class GUIScrollBar : public GUIControl
{
	GUIButton* button_less;
	GUIButton* button_great;
	
	GUIScrollBarKind Kind;
	
	
	GUIRectangle slider_rect;
	int mouseisdown;
	bool MouseInsideSlider;
	bool Flat;

public:
	int PageSize;
	int Max;
	int Min;
	int Position;

	GUIScrollBar (GUIControl* pParent, GUIScrollBarKind sbKind, int Left, int Top, int Width, int Height);
	virtual ~GUIScrollBar ();

	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

	virtual void Draw ();

	void _cdecl MoreButtonDown (GUIControl* sender);
	void _cdecl LessButtonDown (GUIControl* sender);
	
	void OnMDown (int MouseKey, const GUIPoint& pt);

	virtual void MouseUp (int button, const GUIPoint& pt);
	
	GUIEventHandler* t_OnChange;

	virtual void MakeScroll (const GUIPoint& pt);

	virtual void MouseMove (int button, const GUIPoint& pt);

	virtual void ChangeSize (int Width, int Height);

	void SetDrawRect (const GUIRectangle& rect);

	virtual void MakeFlat (bool enable);

	virtual void DrawSlider ();

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIScrollBar") == 0) return true;
		return GUIControl::Is (className);
	}

	void OwnerRescaled(int x, int y, int width, int height);

	bool IsMouseMoveNowProcessing() {return mouseisdown!=0;}

};


#endif

#endif