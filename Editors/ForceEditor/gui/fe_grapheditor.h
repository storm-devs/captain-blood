//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*  description: GRAPHEDITOR - для редактирования ломанных линий
//*
//****************************************************************
#ifndef GUI_GRAPHEDITOR
#define GUI_GRAPHEDITOR

#include "..\..\..\common_h\render.h"
#include "..\..\..\common_h\gui.h"

#include "fe_graphline.h"
#include "fe_linelist.h"

enum GraphEditorMode
{
  GEM_EDIT = 0,
	GEM_SCALE,
	GEM_SCALEX,
	GEM_SCALEY,
	GEM_MOVELINEY,
	GEM_SCALELINEY,
	GEM_SCALELINEX,
	GEM_MOVELINEX
};


class GUIGraphEditor : public GUIControl
{
	
	bool mbBorned;
	GUIPoint BegMousePressed;
	GUIPoint RBegMousePressed;
	bool MouseButtonPressed;
	bool RMouseButtonPressed;
	IRender* pRS;


	
protected:
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

	void DrawBackGround ();

	void DrawLine ();

	bool CtrlState;


	void ClampScale ();
	
public:


	float OffsetX;
	float OffsetY;


	GUIGraphLine* SelectedLine;

	GraphEditorMode Mode;


	bool NegativeValues;
	GUIFont* pSmallFont;

	GUILineList* Lines;

//	GUIGraphLine* GraphLine;
	
	bool LockXScale;
	bool LockYScale;

	string HorizontalAxeDesc;
	string VerticalAxeDesc;
	float GridStepX;
	float GridStepY;
	float ScaleX;
	float ScaleY;


	GUIGraphEditor (GUIControl* pParent, int Left, int Top, int Width, int Height);
	
	virtual ~GUIGraphEditor ();
	
	virtual void Draw ();

	void DrawPoint (const Vector& pos, DWORD color, bool Selected);

	void UnTransformPoint (const Vector& from, Vector& to);
	void TransformPoint (const Vector& from, Vector& to);
	void ScreenToGraph (const Vector& fromr, Vector& to);
	
	void OnMDown (int MouseKey, const GUIPoint& pt);

	virtual void MouseUp (int button, const GUIPoint& pt);
	virtual void MouseMove (int button, const GUIPoint& pt);


	virtual void FitViewWidth (float Min, float Max);
	virtual void FitViewHeight (float Min, float Max);

	virtual float GetMaxValueY ();
	virtual float GetMinValueY ();

	virtual void _cdecl OnClearYes (GUIControl* sender);
	virtual void _cdecl OnClearNo (GUIControl* sender);

	virtual void _cdecl OnDeleteYes(GUIControl *sender);
	virtual void _cdecl OnDeleteNo (GUIControl *sender);

};



#endif