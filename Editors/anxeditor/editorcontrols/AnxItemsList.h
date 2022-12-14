//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxItemsList	
//============================================================================================

#ifndef _AnxItemsList_h_
#define _AnxItemsList_h_

#include "..\AnxBase.h"

class AnxItemsList : public GUIControl
{
//--------------------------------------------------------------------------------------------
public:
	AnxItemsList(GUIControl * p, long x, long y, long w, long h, long _itemHeight = 64);
	virtual ~AnxItemsList();

//--------------------------------------------------------------------------------------------
public:
	array<GUIControl *> items;	

protected:
	//Рисование
	virtual void Draw();

protected:
	long itemHeight;
	GUIScrollBar * scroll;
	long visibleItems;
};

#endif

