
#ifndef GUI_SECKTOR
#define GUI_SECKTOR

#include "..\..\..\..\common_h\gui.h"


class GUISecktor : public GUIControl
{
public:

	enum ESecktorType
	{
		secktor_circle=0,
		secktor_simetric,
		secktor_assimetric		
	};

	ESecktorType Type;	

	dword dwSecktorColor;

	bool NeedDrag;

	int iMode;

	bool IsDragged;	

	float fStartAngle;
	float fEndAngle;

	int   SelAngle;
	GUIPoint Points[3000];

	IVariable *Range;

protected:
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
public:


	GUISecktor (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUISecktor ();
	
	virtual void Draw ();

	virtual void MouseUp (int button, const GUIPoint& pt);
	virtual void MouseMove (int button, const GUIPoint& pt);
	virtual void MouseDown (int button, const GUIPoint& pt);


	// Setup controller functions
	virtual void SetType(int _Type);
	virtual void SetNeedDrag(bool _Need);
	virtual void SetSecktorColor(dword dwColor);
};



#endif