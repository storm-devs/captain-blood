//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxToolListBox	
//============================================================================================
			

#include "AnxToolListBox.h"
#include "..\AnxProject.h"
#include "..\Graph\GraphNodeBase.h"

//============================================================================================

AnxToolListBox::AnxToolListBox(int Left, int Top, int Width, int Height) : GUIWindow(null, Left, Top, Width, Height)
{
	listBox = NEW LB(this, Width, Height);
	bPopupStyle = true;
	bAlwaysOnTop = true;
	UpdatePopupStyle();
}

AnxToolListBox::~AnxToolListBox()
{

}


//============================================================================================

void AnxToolListBox::LB::Draw()
{
	if (Visible)
	{
		//Прямоугольник окна
		GUIRectangle rect = GetDrawRect();
		ClientToScreen(rect);
		//Рисуем панель
		GUIHelper::Draw2DRectAlpha(rect.x + 4, rect.y + 4, rect.w, rect.h, 0x1f000000);
		GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0xcfe0e0f0);
		GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	}
	//Кнопки
	GUIListBox::Draw();
}

void AnxToolListBox::Draw()
{
	//Кнопки
	GUIControl::Draw();
}

void AnxToolListBox::MouseDown(int button, const GUIPoint& pt)
{
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	if(!rect.Inside(pt))
	{
		Close(this);
	}
}

void _cdecl AnxToolListBox::FillNodes(GraphNodeBase * node, bool isRec, AnxNodeTypes mask)
{
	Assert(node);	
	listBox->Hint = " ";
	listBox->hintObj = this;
	listBox->setHint = &AnxToolListBox::SetHint;
	for(long i = 0; i < node->child; i++)
	{
		if(mask == anxnt_error || mask == node->child[i]->GetType())
		{
			listBox->Items.Add(node->child[i]->name);
		}
		if(isRec) FillNodes(node->child[i], isRec, mask);
	}
	listBox->Items.Sort(ABSort);
}

bool _cdecl AnxToolListBox::ABSort(const string & s1, const string & s2)
{
	return s1 < s2;
}


void _cdecl AnxToolListBox::SetHint()
{
	//GUIPoint pnt = Application->GetCursor()->GetPosition();
	listBox->Hint = "";
}




