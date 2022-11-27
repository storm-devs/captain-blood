//*
//****************************************************************
#ifndef GUI_GRAPH_LINE_LIST
#define GUI_GRAPH_LINE_LIST

#include "..\..\..\common_h\gui.h"
#include "gui_graphline.h"

class GUILineList
{

	array<GUIGraphLine*> Lines;
public:

	GUILineList ();
	
	virtual ~GUILineList ();

	void Add (GUIGraphLine* line);

	int GetCount ();

	void Remove (int num);

	GUIGraphLine* GetLine (int num);

	void SortByActive ();

};



#endif