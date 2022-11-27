#ifndef _XBOX

#include "gui_control.h"
#include "gui_helper.h"
#include "..\..\core.h"


dword GUIControl::controlsCounter = 0;
GUIFont * GUIControl::stdHintFont = null;

GUIControl::GUIControl (GUIControl* parent) : Childs (_FL_, 128)
{
	if(!controlsCounter)
	{
		stdHintFont = NEW GUIFont ("arialcyrsmall");
	}
	controlsCounter++;

	OnHintShow_RetValue = true;
	//t_OnBeforeHintShow = NEW GUIEventHandler;
	t_OnBeforeHintShow = &_eventBeforeHintShow;
	Hint = "";
	Enabled = true;
	bDragAndDrop = false;
	bAlwaysOnTop = false;
	UniqueAllocID = 0;
	draw_level = 0;
	
	if (!parent)
		Application = NULL;
	else
		Application = parent->Application;
	
	
	mouse_inside = false;
	DrawRect.Left  = 0;
	DrawRect.Width = 0;
	DrawRect.Top   = 0;
	DrawRect.Height = 0;
	
	Caption = "";
	
	pFont = NULL;
	
	Visible = true;
	
	this->parent = parent;
	
	Childs.DelAll();
	if (parent) parent->AddChild (this);

	pHintFont = stdHintFont;
}

GUIControl::~GUIControl ()
{
	IGUIManager* app = GetMainControl()->Application;
	if (app) app->DeleteControl (this);
	if (pHintFont && pHintFont != stdHintFont) delete pHintFont;
	pHintFont = NULL;

	//if (t_OnBeforeHintShow) delete t_OnBeforeHintShow;
	t_OnBeforeHintShow = NULL;
	
	if (parent)
	{
		for (DWORD n = 0; n < parent->Childs.Size(); n++)
		{
			if (parent->Childs[n] == this)
			{
				parent->Childs.DelIndex(n);
				break;
			}
		}
	}


	unsigned long ch_count = Childs.Size();
	for (unsigned long t = 0; t < ch_count; t++)
	{
		GUIControl* del_control = Childs[0];
		if (del_control)
		{
			delete del_control;
		}
	}
	
	Childs.DelAll();

	
	controlsCounter--;
	if(!controlsCounter)
	{
		delete stdHintFont;
		stdHintFont = null;
	}
}

//Элемент на дереве этого элемента (текущий или потомок)
bool GUIControl::InTree(GUIControl * ctrl)
{
	if(ctrl == this) return true;

	for(dword i = 0; i < Childs.Size(); i++)
	{
		GUIControl * child = Childs[i];
		if(child && child->InTree(ctrl))
		{
			return true;
		}
	}
	return false;
}




void GUIControl::AddChild (GUIControl* pChild)
{

	// Если уже есть парент, то распарентить...
	if (pChild->parent)
	{
		pChild->parent->RemoveChild(pChild);
	}

	// Добавить нового ребенка
	GUIControl** pNewChild = &Childs[Childs.Add ()];
	*(pNewChild) = pChild;

	// Парента прописать...
	pChild->parent = this;
}

void GUIControl::RemoveChild (GUIControl* pChild)
{
	for (unsigned long t = 0; t < Childs.Size(); t++)
	{
		if (Childs[t] == pChild)
		{
			Childs.DelIndex (t);
			return;
		}
	}
}



const GUIRectangle& GUIControl::GetDrawRect ()
{
	return DrawRect;
}

void GUIControl::SetDrawRect (const GUIRectangle& rect)
{
	DrawRect = rect;
}

const GUIRectangle& GUIControl::GetClientRect ()
{
	return ClientRect;
}

void GUIControl::SetClientRect (const GUIRectangle& rect)
{
	ClientRect = rect;
}


GUIControl* GUIControl::GetMainControl ()
{
	GUIControl* top = this;
	while (top->parent) top = top->parent;
	return top;
}

void GUIControl::Draw ()
{
	if (Visible == false) return;

	for (unsigned long n = 0; n < Childs.Size(); n++)
	{
		GUIControl** wnd = &Childs [n];
		if ((wnd) && (!(*wnd)->bAlwaysOnTop))
		{
			GUIControl* pWnd = *(wnd);
			pWnd->Draw ();
		}
	}

	for (n = 0; n < Childs.Size(); n++)
	{
		GUIControl** wnd = &Childs [n];
		if ((wnd) && ((*wnd)->bAlwaysOnTop))
		{
			GUIControl* pWnd = *(wnd);
			pWnd->Draw ();
		}
	}

}


bool GUIControl::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (!Enabled) return false;
 	if (Visible == false) return false;

	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
	int under_cursor_only = GUIHelper::MessageSpreadOnlyForUnderCursorControls(message);
	
	
	unsigned long  total_childs = Childs.Size();
	for (unsigned long n = 0; n < total_childs; n++)
	{
		GUIControl** wnd = &Childs [total_childs-1-n];
		if (wnd)
		{
			GUIControl* pWnd = *(wnd);
			if (!pWnd->Enabled) continue;
			if (under_cursor_only)
			{
				const GUIRectangle& rct = pWnd->GetScreenRect ();
				if (GUIHelper::PointInRect(cursor_pos, rct) == false) continue;
			}
			bool res = pWnd->ProcessMessages (message, lparam, hparam);
			if (res) return res;
		}
	}
	
	return false;
	
}



const GUIRectangle& GUIControl::GetScreenRect ()
{
	//TmpScreenRect
	int AddX = 0;
	int AddY = 0;
	
	GUIControl* top = this;
	while (top->parent) 
	{
		top = top->parent;
		
		GUIRectangle r = top->GetClientRect ();
		AddX += r.Left;
		AddY += r.Top;
	}
	
	
	TmpScreenRect = DrawRect;
	TmpScreenRect.Left += AddX;
	TmpScreenRect.Top += AddY;
	return TmpScreenRect;
}


bool GUIControl::SendMessage (GUIMessage message, DWORD lparam, DWORD hparam)
{
	return ProcessMessages (message, lparam, hparam);
}


void GUIControl::ClientToScreen (GUIPoint& p1)
{
	GUIControl* lpRoot = this->parent;
	while (lpRoot)
	{
		GUIRectangle tR = lpRoot->GetClientRect ();
		p1.X  += tR.Left;
		p1.Y  += tR.Top;
		lpRoot = lpRoot->parent;
	}
}


void GUIControl::ScreenToClient (GUIPoint& p1)
{
	GUIControl* lpRoot = this;
	while (lpRoot)
	{
		GUIRectangle tR = lpRoot->GetClientRect ();
		p1.X  -= tR.Left;
		p1.Y  -= tR.Top;
		lpRoot = lpRoot->parent;
	}
}


GUIControl* GUIControl::GetElementUnderCursor (const GUIPoint& pt)
{
	// Получить прямоугольник в который отрисовывается элемент
	const GUIRectangle& rct = GetScreenRect ();
	
	// Если курсор мыши лежит в прямоугольнике
	if (GUIHelper::PointInRect(pt, rct)) 
	{
		// Для всех детей
		for (unsigned long m = 0; m < Childs.Size(); m++)
		{
			// Получаем ребенка
			GUIControl* pChild = Childs[m];
			
			//Спросить у ребенка может он под курсором
			GUIControl* pElement = pChild->GetElementUnderCursor (pt);
			
			// Если да, то вернуть ребенка
			if (pElement && pElement->Visible) return pElement;
		}
		
		// Никто из детей не под курсором, вернем сами себя :)
		return this;
	}
	
	// Если курсор не попал в элемент вернем NULL
	return NULL;
}


void GUIControl::ScreenToClient (GUIRectangle& p1)
{
	GUIControl* lpRoot = this->parent;
	while (lpRoot)
	{
		GUIRectangle tR = lpRoot->GetClientRect ();
		p1.Left  -= tR.Left;
		p1.Top  -= tR.Top;
		lpRoot = lpRoot->parent;
	}
}

void GUIControl::ClientToScreen (GUIRectangle& p1)
{
	GUIControl* lpRoot = this->parent;
	while (lpRoot)
	{
		GUIRectangle tR = lpRoot->GetClientRect ();
		p1.Left  += tR.Left;
		p1.Top  += tR.Top;
		lpRoot = lpRoot->parent;
	}
}


void GUIControl::MouseUp (int button, const GUIPoint& pt)
{
	
	for (unsigned long n = 0; n < Childs.Size(); n++)
	{
		GUIControl** wnd = &Childs [n];
		if (wnd)
		{
			GUIControl* pWnd = *(wnd);
			pWnd->MouseUp (button, pt);
		}
	}
	
	
}

void GUIControl::MouseDown (int button, const GUIPoint& pt)
{
	
	for (unsigned long n = 0; n < Childs.Size(); n++)
	{
		GUIControl** wnd = &Childs [n];
		if (wnd)
		{
			GUIControl* pWnd = *(wnd);
			pWnd->MouseDown (button, pt);
		}
	}
}

void GUIControl::KeyPressed (int key, bool bSysKey)
{
	for (unsigned long n = 0; n < Childs.Size(); n++)
	{
		GUIControl** wnd = &Childs [n];
		if (wnd)
		{
			GUIControl* pWnd = *(wnd);
			pWnd->KeyPressed (key, bSysKey);
		}
	}

}

void GUIControl::MouseMove (int button, const GUIPoint& pt)
{
	
	for (unsigned long n = 0; n < Childs.Size(); n++)
	{
		GUIControl** wnd = &Childs [n];
		if (wnd)
		{
			GUIControl* pWnd = *(wnd);
			pWnd->MouseMove (button, pt);
		}
	}
}


void GUIControl::SetAllocID (int id)
{
	UniqueAllocID = id;
}

int GUIControl::GetAllocID ()
{
	return UniqueAllocID;
}


IRender* GUIControl::GetRender ()
{
	IRender* rs = (IRender*)api->GetService("DX9Render");
	return rs;
}

DWORD GUIControl::GetDrawLevel () const
{
	DWORD real_draw_level = draw_level;
	if (bAlwaysOnTop == true) real_draw_level += 0xFFFFFF;
	return real_draw_level;
};

GUIControl* GUIControl::GetParent ()
{
	return parent;
}

void GUIControl::SetFocus ()
{
	GUIControl* main = GetMainControl ();
	IGUIManager* manager = main->Application;
	manager->SetFocus (this);
}

void GUIControl::OnCreate ()
{
}

void GUIControl::sysOnCreate ()
{
	if (parent)	Application = parent->Application;
}



void GUIControl::DrawHint (int pX, int pY, const string& _Hint)
{
	//Max
	if(!pHintFont) return;
	long w = pHintFont->GetWidth(_Hint.c_str()) + 10;
	long h = pHintFont->GetHeight(_Hint.c_str());
	if(pX < 0) pX = 0;
	if(pY < 0) pY = 0;
	const RENDERSCREEN & rscr = GUIHelper::GetRender()->GetScreenInfo3D();
	if(pX + w + 1 > (long)rscr.dwWidth) pX = (long)rscr.dwWidth - w - 1;
	if(pY + h + 1 > (long)rscr.dwHeight) pY = (long)rscr.dwHeight - h - 1;
	GUIHelper::Draw2DRect(pX-1, pY-1, w+2, h+2, 0xFF000000);
	GUIHelper::Draw2DRect(pX, pY, w, h, 0xFFFFFFE1);
	pHintFont->Print(pX + 3, pY, 0xFF000000, _Hint.c_str());
	return;

	//FIXME !!!
	static array<int> str_lines(_FL_);
	string Hint = _Hint;
	str_lines.DelAll();
	if (!pHintFont) return;
	int Height = pHintFont->GetHeight();

	int lines = 1;
	str_lines.Add(0);
	for (DWORD n =0; n < Hint.Size(); n++)
	{
		if (Hint[n] == '\n')
		{
			if ((n+1) < Hint.Size()) str_lines.Add((n+1));
			Hint[n] = 0;
			lines++;
		}
	}
	if (lines == 0) lines = 1;

	Height*=lines;
	Height++;


	int Width = 0;
	for (int i = 0; i < lines; i++)
	{
		int tWidth = pHintFont->GetWidth(Hint.GetBuffer() + str_lines[i]);
		if (tWidth > Width) Width = tWidth; 
	}



	Width+= 4;

	GUIHelper::Draw2DRect(pX-1, pY-1, Width+2, Height+2, 0xFF000000);
	GUIHelper::Draw2DRect(pX, pY, Width, Height, 0xFFFFFFE1);

	for ( i = 0; i < lines; i++)
	{
		pHintFont->Print(pX, pY+(pHintFont->GetHeight()*i), 0xFF000000, (Hint.GetBuffer() + str_lines[i]));
	}
	
	
}


bool GUIControl::BeforeHintShow ()
{
	OnHintShow_RetValue = true;
	t_OnBeforeHintShow->Execute(this);
	return OnHintShow_RetValue;
}

#endif