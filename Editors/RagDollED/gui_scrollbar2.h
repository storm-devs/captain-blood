#ifndef _XBOX
#ifndef GUI_SCROLLBAR2
#define GUI_SCROLLBAR2

#include "..\..\common_h\gui.h"


extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnChange \
t_OnChange->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


class GUIScrollBar2 : public GUIControl
{
	
	class GUIButton2 *button_less;
	class GUIButton2 *button_great;
	
	GUIScrollBarKind Kind;
	
	
	GUIRectangle slider_rect;
	int mouseisdown;
	bool MouseInsideSlider;
	bool Flat;

	int x_off;
	
public:
	

	int PageSize;
	
	int Max;
	int Min;
	int Position;
	
	
	
	GUIScrollBar2 (GUIControl* pParent, GUIScrollBarKind sbKind, int Left, int Top, int Width, int Height);
	virtual ~GUIScrollBar2 ();
	
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


	virtual void DrawSlider(const GUIRectangle &rect);


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