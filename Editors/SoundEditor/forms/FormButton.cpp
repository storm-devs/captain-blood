//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormButton
//============================================================================================

#include "FormButton.h"


FormButton::FormButton(GUIControl * parent, const GUIRectangle & r) : GUIControl(parent)
{
	SetClientRect(r);
	SetDrawRect(r);
	image = null;
	glyphFunc = null;
	glyphControl = null;
	imageW = imageH = 16;
}

FormButton::~FormButton()
{
	if(options->dndControl == this)
	{
		options->dndControl = null;
	}
}

//Получить состояние кнопки
bool FormButton::IsPressed()
{
	return (options->dndControl == this);
}

//Рисование
void FormButton::Draw()
{
	if(!Visible)
	{
		if(options->dndControl == this)
		{
			options->dndControl = null;
		}
		return;
	}
	if(!Enabled)
	{
		if(options->dndControl == this)
		{
			options->dndControl = null;
		}
	}
	bool isPressed = IsPressed();
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	dword colorBkg = options->bkg;
	dword colorFrame = options->black2Bkg[6];
	GUIPoint offset(0, 0);
	if(isPressed)
	{
		offset.x = 1;
		offset.y = 1;
		colorBkg = options->bkg2White[1];
		colorFrame = options->black2Bkg[3];
	}else{
		if(!options->dndControl)
		{
			if(options->IsHighlight(this))
			{
				colorBkg = options->bkg2White[2];
				colorFrame = options->black2Bkg[0];
			}
		}
	}
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, colorBkg);
	if(!glyphControl)
	{
		if(!image)
		{
			if(text.str)
			{
				long x = r.x + (r.w - text.w)/2 + offset.x;
				long y = r.y + (r.h - text.h)/2 + offset.y;
				options->render->Print(float(x), float(y), options->colorTextLo, text.str);
			}
		}else{
			long x = r.x + (r.w - imageW)/2 + offset.x;
			long y = r.y + (r.h - imageH)/2 + offset.y;
			GUIHelper::DrawSprite(x, y, imageW, imageH, image);
		}
	}else{
		GUIRectangle r1(r.x + offset.x, r.y + offset.y, r.w, r.h);
		(glyphControl->*glyphFunc)(this, r1);
	}
	if(!Enabled)
	{
		GUIHelper::Draw2DRectAlpha(r.x, r.y, r.w, r.h, 0x80ffffff);
	}
	if(!isPressed)
	{
		GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, colorFrame);
	}else{
		GUIHelper::DrawWireRect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, colorFrame);
		GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	}
	GUIControl::Draw();
}

//Сообщения
bool FormButton::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if(!Visible || !Enabled)
	{
		if(options->dndControl == this)
		{
			options->dndControl = null;
		}
		return true;
	}
	if(message == GUIMSG_LMB_DOWN && !options->dndControl && Visible)
	{
		GUIPoint cursor_pos;
		GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
		GUIRectangle r = GetClientRect();	
		ClientToScreen(r);
		if(r.Inside(cursor_pos) && Enabled)
		{
			options->dndControl = this;
			OnDown();
		}
	}	
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Нажатие кнопки мыши
void FormButton::MouseUp(int button, const GUIPoint& pt)
{
	if(!Visible)
	{
		return;
	}
	if(!IsPressed())
	{
		return;
	}
	options->dndControl = null;
	GUIRectangle r = GetClientRect();	
	ClientToScreen(r);
	if(r.Inside(pt) && Enabled)
	{
		OnUp();
	}	
}

//Событие нажатия кнопки
void FormButton::OnDown()
{
	onDown.Execute(this);
}

//Событие отпускания кнопки
void FormButton::OnUp()
{
	onUp.Execute(this);
}
