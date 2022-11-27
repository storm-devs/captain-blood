//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*  description: Линия для рисования в графедиторе
//*
//****************************************************************
#ifndef GUI_GRAPH_LINE
#define GUI_GRAPH_LINE

#include "..\..\..\common_h\gui.h"


class GUIGraphLine
{

	array<Vector> Points;
public:

	bool bNegative;
	int LastMinimalPoint;
	int SelectedPoint;
	DWORD color;
	bool bActive;


	void systemClear ();

	GUIGraphLine (const Vector& begin, const Vector& end);
	virtual ~GUIGraphLine ();


	void Clear ();
	void Copy (GUIGraphLine* src);

	void Insert (int num, const Vector& newpoint);

	void Add (const Vector& newpoint);

	int GetCount ();

	void Remove (int num);


	void Change (int num, const Vector& newpoint);

	const Vector& GetPoint (int num);

	array<Vector> &GetPoints()
	{
		return Points;
	}

	void Zero ();


	void SetSize (DWORD count);


};



#endif