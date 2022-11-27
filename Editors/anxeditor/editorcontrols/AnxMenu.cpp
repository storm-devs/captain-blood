//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxMenu	
//============================================================================================
			

#include "AnxMenu.h"
#include "AnxToolButton.h"

//============================================================================================

AnxMenu::AnxMenu(long x, long y, long w) : GUIWindow(null, x, y, w, 8)
{
	offset = 0;
	bheight = 32;
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
}

AnxMenu::~AnxMenu()
{

}

//============================================================================================

void AnxMenu::AddItem(Command * cmd, const char * imageName, const char * text, const char * hint)
{
	if(!hint) hint = "";
	GUIRectangle r = DrawRect;
	ClientToScreen(r);
	offset += 3;
	AnxToolButton * b = NEW AnxToolButton(cmd, this, 4, 4 + offset, r.w - 8, bheight, this);
	b->Hint = hint;
	offset += bheight;
	b->Caption = text;
	b->Glyph->Load(imageName);
	b->Layout = GUIBUTTONLAYOUT_GlyphLeft;
	SetHeight(offset + 8);
	UpdatePopupStyle();
}

//============================================================================================

void AnxMenu::Draw()
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xcfe0e0f0);
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	//Кнопки
	GUIControl::Draw();
	//Рисуем раздулительные линии
	for(long i = bheight; i < offset - bheight; i += bheight + 3)
	{
		long ly = 8 + rect.y + i;
		GUIHelper::Draw2DLine(rect.x + 4, ly, rect.x + rect.w - 4, ly, 0xff808080);
	}
}

void AnxMenu::MouseDown(int button, const GUIPoint& pt)
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	if(!rect.Inside(pt)) Close(this);
}


