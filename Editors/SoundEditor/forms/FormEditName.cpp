

#include "FormEditName.h"
#include "FormButton.h"
#include "FormEdit.h"

FormEditName::FormEditName(GUIControl * parent, const GUIPoint & p, PointPivot pp) : GUIWindow(parent, 0, 0, 1, 1)
{
	//Настраиваем и показываем окно
	GUIRectangle r;
	switch(pp)
	{
	case pp_left_top:
		r.x = p.x;
		r.y = p.y;
		break;
	case pp_left_bottom:
		r.x = p.x;
		r.y = p.y - c_height;
		break;
	case pp_right_top:
		r.x = p.x - c_width;
		r.y = p.y;
		break;
	case pp_right_bottom:
		r.x = p.x - c_width;
		r.y = p.y - c_height;
		break;
	}
	r.w = c_width;
	r.h = c_height;
	bPopupStyle = true;
	bSystemButton = false;
	SetClientRect(r);
	SetDrawRect(r);
	GUIWindow::Draw();
	//Заводим кнопки
	r.x = c_width/2 - c_border - c_button_width;
	r.y = c_height - c_border - c_button_height;
	r.w = c_button_width;
	r.h = c_button_height;
	buttonCancel = NEW FormButton(this, r);
	buttonCancel->onUp.SetHandler(this, (CONTROL_EVENT)&FormEditName::OnCancelEdit);
	buttonCancel->text.SetString(options->GetString(SndOptions::s_form_cancel));
	r.x = c_width/2 + c_border;
	buttonOk = NEW FormButton(this, r);
	buttonOk->text.SetString(options->GetString(SndOptions::s_form_ok));
	buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormEditName::OnOkEdit);
	buttonOk->Enabled = false;
	//Контрол редактирования
	r.x = c_border;
	r.y = c_border + c_header_height + c_border;
	r.w = c_width - c_border*2;
	r.h = c_edit_height;
	edit = NEW FormEdit(this, r);
	edit->onChange.SetHandler(this, (CONTROL_EVENT)&FormEditName::OnEditChange);
	edit->onAccept.SetHandler(this, (CONTROL_EVENT)&FormEditName::OnEditAccept);
	GUIWindow::Draw();	
}

FormEditName::~FormEditName()
{
}

void FormEditName::OnCreate()
{
	options->kbFocus = edit;
	options->gui_manager->SetKeyboardFocus(edit);
	edit->SetFocus();	
	edit->CursorToEnd();
	OnEditAccept(this);
}

//Установить текст
void FormEditName::SetText(const char * str)
{
	edit->Text = str;
}

//Получить текст
const char * FormEditName::GetText()
{
	return edit->Text.c_str();
}

//Текущий текст негодиться
void FormEditName::MarkTextAsIncorrect()
{
	buttonOk->Enabled = false;
	edit->isAccept = false;
}

//Рисование
void FormEditName::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	if(Caption.Len() > 0)
	{
		options->render->Print(float(r.x + c_border), float(r.y + c_border), options->colorTextLo, Caption.c_str());
	}
	GUIControl::Draw();
}


void _cdecl FormEditName::OnOkEdit(GUIControl* sender)
{
	options->gui_manager->Close(this);
	onOk.Execute(this);
}

void _cdecl FormEditName::OnCancelEdit(GUIControl* sender)
{
	options->gui_manager->Close(this);
	onCancel.Execute(this);
}

void _cdecl FormEditName::OnEditChange(GUIControl* sender)
{
	buttonOk->Enabled = false;
}

void _cdecl FormEditName::OnEditAccept(GUIControl* sender)
{
	buttonOk->Enabled = true;
	onCheck.Execute(this);
}
