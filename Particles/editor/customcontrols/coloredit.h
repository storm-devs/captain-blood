#ifndef GUI_COLOREDIT
#define GUI_COLOREDIT

#include "..\..\..\common_h\gui.h"
//#include "attrcolor.h"



class GUIColorEditor : public GUIPanel
{
public:
	struct ColorPoint
	{
		float pos; // 0..1
		Color max;
		Color min;
	};

private:
	bool mbBorned;

	int NewIndex;
	float NewPos;

	bool EditedLowest;
	int EditedIndex;

	int DragedIndex;	
	GUIPoint mouse_hotspot;

public:

	array<GUIColorEditor::ColorPoint> Points;

	GUIColorEditor (GUIControl* pParent, int Left, int Top, int Width, int Height);
	~GUIColorEditor ();


	void Draw ();


	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

	void MouseLMBPressed ();
	void MouseLMBDBLClick ();

	int GetPointIndex (const GUIPoint& point, bool& minpointpressed);

	void _cdecl ColorIsChange (GUIControl* sender);

	void _cdecl CreateNewPoint (GUIControl* sender);


	virtual void MouseUp (int button, const GUIPoint& pt);
	virtual void MouseMove (int button, const GUIPoint& pt);

	void _cdecl OnClearYes (GUIControl* sender);
	void _cdecl OnClearNo (GUIControl* sender);

	



};

#endif