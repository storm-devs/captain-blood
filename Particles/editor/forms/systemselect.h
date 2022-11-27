//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************
#ifndef PSYS_SELECTOR_BROWSER
#define PSYS_SELECTOR_BROWSER

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\particles.h"

class IFileService;

class TSystemSelector : public GUIWindow
{
	IFileService* pFS;
	GUIListBox* pSystemsList;
	GUIButton* pOKButton;
  GUIButton* pCancelButton;


public:

	bool OkPressed;
	string SystemName;
	
	TSystemSelector ();
	~TSystemSelector ();

	virtual void Draw ();

	virtual void BuildSystemsList ();

//===================================
public:
	virtual void _cdecl OnAddSystem ();
	virtual void _cdecl OnCancelSelect ();




  
};



#endif