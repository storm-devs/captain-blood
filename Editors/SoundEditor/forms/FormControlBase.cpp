

#include "FormControlBase.h"


FormControlBase::FormControlBase(GUIControl * parent, const GUIRectangle & rect) : GUIControl(parent)
{
	SetClientRect(rect);
	SetDrawRect(rect);
	isBlockWeel = false;
}

FormControlBase::~FormControlBase()
{
	if(options->dndControl == this)
	{
		options->dndControl = null;
	}
}

//Рисование
void FormControlBase::Draw()
{
	//Прямоугольник контрола в экранных координатах
	if(!Visible) return;
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	OnDraw(r);
	if(!Enabled)
	{
		GUIHelper::Draw2DRectAlpha(r.x, r.y, r.w, r.h, 0x80ffffff);
	}
	GUIControl::Draw();
	isBlockWeel = false;
}

//Сообщения
bool FormControlBase::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
	if(message == GUIMSG_LMB_DOWN)
	{
		if(MouseDownL(GUIPoint(cursor_pos)))
		{
			return true;
		}		
	}else
	if(message == GUIMSG_RMB_DOWN)
	{
		if(MouseDownR(GUIPoint(cursor_pos)))
		{
			return true;
		}		
	}else
	if(message == GUIMSG_WHEEL_UP)
	{
		MouseWeel(cursor_pos, 1);
	}else
	if(message == GUIMSG_WHEEL_DOWN)
	{
		MouseWeel(cursor_pos, -1);
	}
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Нажатие кнопки мыши
bool FormControlBase::MouseDownL(const GUIPoint& pt)
{
	if(options->dndControl)
	{
		return false;
	}
	GUIRectangle r = GetClientRect();
	ClientToScreen(r);
	if(!r.Inside(pt))
	{
		return false;
	}
	GUIPoint mousePoint(pt.x - r.x, pt.y - r.y);
	GUIPoint pivot(0, 0);
	if(OnDragBegin(mousePoint, pivot))
	{
		deltaDrag.x = mousePoint.x - pivot.x;
		deltaDrag.y = mousePoint.y - pivot.y;
		options->dndControl = this;
		return true;
	}
	OnMouseLeftClick(mousePoint);
	return true;
}

//Нажатие правой кнопкой мыши
bool FormControlBase::MouseDownR(const GUIPoint& pt)
{
	if(options->dndControl)
	{
		return false;
	}
	GUIRectangle r = GetClientRect();
	ClientToScreen(r);
	if(!r.Inside(pt))
	{
		return false;
	}
	GUIPoint mousePoint(pt.x - r.x, pt.y - r.y);
	OnMouseRightClick(mousePoint);
	return false;
}


//Нажатие кнопки мыши
void FormControlBase::MouseUp(int button, const GUIPoint& pt)
{
	if(options->dndControl != this)
	{
		return;
	}
	GUIRectangle r = GetClientRect();
	ClientToScreen(r);
	GUIPoint mousePoint(pt.x - r.x - deltaDrag.x, pt.y - r.y - deltaDrag.y);
	OnDragMoveTo(mousePoint);
	OnDragEnd();
	options->dndControl = null;
}

//Перемещение мыши
void FormControlBase::MouseMove(int button, const GUIPoint& pt)
{
	if(options->dndControl != this)
	{
		return;
	}
	GUIRectangle r = GetClientRect();
	ClientToScreen(r);
	GUIPoint mousePoint(pt.x - r.x - deltaDrag.x, pt.y - r.y - deltaDrag.y);
	OnDragMoveTo(mousePoint);
}

//Дельта от колеса
void FormControlBase::MouseWeel(const GUIPoint& pt, long delta)
{
	if(!isBlockWeel)
	{
		isBlockWeel = true;
		GUIRectangle r = GetClientRect();
		ClientToScreen(r);
		GUIPoint mousePoint(pt.x - r.x, pt.y - r.y);
		OnMouseWeel(mousePoint, delta);
	}
}
