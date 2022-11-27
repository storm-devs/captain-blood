//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxItemsList	
//============================================================================================
			

#include "AnxItemsList.h"

//============================================================================================

#define AIL_SWIDTH	20

//============================================================================================

AnxItemsList::AnxItemsList(GUIControl * p, long x, long y, long w, long h, long _itemHeight) : GUIControl(p), items(_FL_)
{
	//Прямоугольник окна
	DrawRect.x = x;
	DrawRect.y = y;
	DrawRect.w = (w < 50) ? 50 : w;
	DrawRect.h = (h < 20) ? 20 : h;
	//Прямоугольник для детей
	ClientRect = DrawRect;
	//Высота элемента
	itemHeight = _itemHeight < 10 ? 10 : _itemHeight;
	//Полоса прокрутки
	scroll = NEW GUIScrollBar(this, GUISBKIND_Vertical, DrawRect.w - AIL_SWIDTH + 1, 0, AIL_SWIDTH, DrawRect.h + 1);
	//Количество итемов в окне
	visibleItems = (DrawRect.h + itemHeight - 1)/itemHeight + 1;
	scroll->PageSize = visibleItems - 1;
}

AnxItemsList::~AnxItemsList()
{

}

//============================================================================================

//Рисование
void AnxItemsList::Draw()
{
	//Текущее состояние полосы прокрутки
	scroll->PageSize = itemHeight;
	scroll->Min = 0;
	scroll->Max = items - visibleItems/2;
	if(scroll->Max < 0) scroll->Max = 0;
	scroll->Max *= itemHeight + 2;
	if(scroll->Position < scroll->Min) scroll->Position = scroll->Min;
	if(scroll->Position > scroll->Max) scroll->Position = scroll->Max;
	//Рисуем итемы
	GUIRectangle rect;
	cliper.Push();
	rect.x = DrawRect.x + 2;
	rect.y = DrawRect.y + 2;
	rect.w = ClientRect.w - AIL_SWIDTH - 1;
	rect.h = ClientRect.h - 6;
	ClientToScreen(rect);
	cliper.SetRectangle(rect);
	for(long i = 0; i < items; i++)
	{
		rect.x = 2;
		rect.y = i*(itemHeight + 2) - scroll->Position + 4;
		rect.w = ClientRect.w - AIL_SWIDTH - 3;
		rect.h = itemHeight;
		items[i]->SetDrawRect(rect);
		items[i]->SetClientRect(rect);
		items[i]->Draw();
	}
	cliper.Pop();
	//Рисуем полосу прокрутки
	scroll->Draw();
	//Рисуем рамку
	rect = DrawRect;
	ClientToScreen(rect);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
}


