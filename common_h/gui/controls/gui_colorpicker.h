#ifndef _XBOX
#ifndef COLOR_PICKER_DIALOG
#define COLOR_PICKER_DIALOG


#include "gui_window.h"
#include "gui_label.h"
#include "gui_edit.h"


#define OnApply \
	t_OnApply->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)

#define OnCancel \
	t_OnCancel->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)


#define OnChange \
	t_OnChange->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)



class GUIColorPicker : public GUIWindow
{

	GUIImage* Target;
//	GUIImage* Colorizer;
	GUIImage* Arrows;

	float posX;
	float posY;

	int AlphaPosition;
	float MainColorPosition;

	bool MouseP;
	bool MouseS;
	bool MouseA;



	DWORD MainColor;



	
public:

	DWORD CalcCurrentColor ();

	void CalcMainColor ();


	GUIButton* OKButton;
	GUIButton* CancelButton;


	GUILabel* labelR;
	GUILabel* labelG;
	GUILabel* labelB;
	GUILabel* labelA;
	GUILabel* labelHex;

	GUIEdit* editR;
	GUIEdit* editG;
	GUIEdit* editB;
	GUIEdit* editA;
	GUIEdit* HexValue;

	GUIEdit* HDRPower;
	GUILabel* labelHDR;


	DWORD SelectedColor;
	
	GUIColorPicker (int X, int Y, const char* caption, int AddWidth, int AddHeight);
	~GUIColorPicker ();

	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);


	void Draw ();

	void OnMDown (int MouseKey, const GUIPoint& pt);
	
	virtual void _cdecl OKClick (GUIControl* sender);
	virtual void _cdecl CloseClick (GUIControl* sender);

	void MouseUp (int button, const GUIPoint& pt);

	void MouseMove (int button, const GUIPoint& pt);


	void UpdateTextValues ();

	virtual void _cdecl TextIsChanged (GUIControl* sender);
	virtual void _cdecl HexTextIsChanged (GUIControl* sender);

	void ProcessTextIsChanged (BYTE _R, BYTE _G, BYTE _B, BYTE _A);

	virtual void CalculateColorPositions (BYTE c1, BYTE c2, BYTE c3, int ColorSegment, bool StepUp = false);


	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIColorPicker") == 0) return true;
		return GUIWindow::Is (className);
	}

	GUIEventHandler* t_OnApply;
	GUIEventHandler* t_OnCancel;
	GUIEventHandler* t_OnChange;

};



#endif

#endif