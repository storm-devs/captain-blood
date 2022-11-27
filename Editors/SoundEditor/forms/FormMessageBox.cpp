

#include "FormMessageBox.h"
#include "FormButton.h"


FormMessageBox::FormMessageBox(const char * caption, const char * message, Mode mode, GUIControl * parent, PointPivot pp, const GUIPoint * p) : GUIWindow(parent, 0, 0, 1, 1)
{
	//Устанавливаем текст
	captionText.SetString(caption);
	messageText.SetString(message);
	//Определяем ширину
	dword w = coremax(captionText.w, messageText.w) + c_space + c_space + 40;
	w = coremax(coremin(w, c_width), (c_button_width + c_space)*2);
	//Настраиваем и показываем окно
	GUIRectangle r;
	r.w = w;
	r.h = c_height + messageText.h;
	if(p)
	{
		switch(pp)
		{
		case pp_left_top:
			r.x = p->x;
			r.y = p->y;
			break;
		case pp_left_bottom:
			r.x = p->x;
			r.y = p->y - r.h;
			break;
		case pp_right_top:
			r.x = p->x - r.w;
			r.y = p->y;
			break;
		case pp_right_bottom:
			r.x = p->x - r.w;
			r.y = p->y - r.h;
			break;
		}
	}else{		
		long w = options->screenWidth;
		long h = options->screenHeight;
		r.x = (w - r.w)/2;
		r.y = (h - r.h)/2;
	}
	if(r.x < 0) r.x = 0;
	if(r.y < 0) r.y = 0;
	if(r.x + r.w > (long)options->screenWidth) r.x = (long)options->screenWidth - r.w - 1;
	if(r.y + r.h > (long)options->screenWidth) r.y = (long)options->screenWidth - r.h - 1;

	bPopupStyle = true;
	bSystemButton = false;
	SetClientRect(r);
	SetDrawRect(r);
	GUIWindow::Draw();
	//Заголовок
	captionPos.x = c_space;
	captionPos.y = (c_header_height - captionText.h)/2;
	messagePos.x = c_space + c_space;
	messagePos.y = c_header_height + c_space;
	//Заводим кнопки
	if(mode != m_ok)
	{
		r.x = w/2 - c_space - c_button_width;
		r.y = r.h - c_space - c_button_height;
		r.w = c_button_width;
		r.h = c_button_height;
		buttonOk = NEW FormButton(this, r);
		buttonOk->text.SetString(options->GetString(mode == m_yesno ? SndOptions::s_form_yes : SndOptions::s_form_ok));
		buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormMessageBox::DoOk);
		r.x = w/2 + c_space;
		buttonCancel = NEW FormButton(this, r);
		buttonCancel->onUp.SetHandler(this, (CONTROL_EVENT)&FormMessageBox::DoCancel);
		buttonCancel->text.SetString(options->GetString(mode == m_yesno ? SndOptions::s_form_no : SndOptions::s_form_cancel));
	}else{
		r.x = w/2 - c_button_width/2;
		r.y = r.h - c_space - c_button_height;
		r.w = c_button_width;
		r.h = c_button_height;
		buttonOk = NEW FormButton(this, r);
		buttonOk->text.SetString(options->GetString(SndOptions::s_form_ok));
		buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormMessageBox::DoOk);
	}
	headerColor = options->bkg | 0xfff00000;
}

FormMessageBox::~FormMessageBox()
{

}

//Рисование
void FormMessageBox::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg2White[2]);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, c_header_height, headerColor);
	options->render->Print(float(r.x + captionPos.x), float(r.y + captionPos.y), options->colorTextLo, captionText.str);
	options->render->Print(float(r.x + messagePos.x), float(r.y + messagePos.y), options->colorTextLo, messageText.str);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	GUIControl::Draw();
}

void _cdecl FormMessageBox::DoOk(GUIControl* sender)
{
	options->gui_manager->Close(this);
	onOk.Execute(this);
}

void _cdecl FormMessageBox::DoCancel(GUIControl* sender)
{
	options->gui_manager->Close(this);
	onCancel.Execute(this);
}