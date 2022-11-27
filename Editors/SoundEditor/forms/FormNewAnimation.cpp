

#include "FormNewAnimation.h"
#include "FormButton.h"
#include "FormEdit.h"
#include "FormMessageBox.h"


FormNewAnimation::FormNewAnimation(GUIControl * parent, const GUIPoint & p) : GUIWindow(parent, 0, 0, 1, 1)
{
	//Настраиваем и показываем окно
	GUIRectangle r;
	r.x = p.x;
	r.y = p.y + c_border;
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
	buttonCancel->onUp.SetHandler(this, (CONTROL_EVENT)&FormNewAnimation::OnCancelEdit);
	buttonCancel->text.SetString(options->GetString(SndOptions::s_form_cancel));
	r.x = c_width/2 + c_border;
	buttonOk = NEW FormButton(this, r);
	buttonOk->text.SetString(options->GetString(SndOptions::s_form_ok));
	buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormNewAnimation::OnOkEdit);
	//buttonOk->Enabled = false;
	//Контрол редактирования пути
	r.x = c_space;
	r.y = c_border + c_header_height + c_space + c_edit_height + c_border;
	r.w = c_width - c_space*2 - c_nav_button_w - c_border;
	r.h = c_edit_height;
	anxEdit = NEW FormEdit(this, r);
	anxEdit->onChange.SetHandler(this, (CONTROL_EVENT)&FormNewAnimation::OnEditChange);
	anxEdit->onAccept.SetHandler(this, (CONTROL_EVENT)&FormNewAnimation::OnEditAccept);	
	anxEdit->UpdateText();
	//Кнопка выбора пути
	r.x += r.w + c_border;
	r.w = c_nav_button_w;
	buttonAnxPath = NEW FormButton(this, r);
	buttonAnxPath->onUp.SetHandler(this, (CONTROL_EVENT)&FormNewAnimation::OnAnxChoisePath);
	buttonAnxPath->text.SetString(options->GetString(SndOptions::s_snd_sbo_navbutton));
	//Фокус
	anxEdit->SetFocus();
	options->kbFocus = anxEdit;	
	options->gui_manager->SetFocus(anxEdit);
	options->gui_manager->SetKeyboardFocus(anxEdit);
	//Обновим содержимое
	GUIWindow::Draw();	
}

FormNewAnimation::~FormNewAnimation()
{

}

//Получить имя анимации, которую добавили
const char * FormNewAnimation::GetAnimationName()
{
	return anxName.c_str();
}

//Рисование
void FormNewAnimation::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, c_header_height, options->bkg2White[2]);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	options->render->Print(float(r.x + c_border), float(r.y + c_border), options->colorTextLo, options->GetString(SndOptions::s_ani_add_animation));
	options->render->Print(float(r.x + c_space), float(r.y + c_border + c_header_height + c_space), options->colorTextLo, options->GetString(SndOptions::s_ani_add_animation_anx));

	GUIControl::Draw();
}

void _cdecl FormNewAnimation::OnOkEdit(GUIControl* sender)
{
	string animName;
	animName.GetFileTitle(anxEdit->Text);
	ErrorId err = project->AnimationAdd(anxEdit->Text.c_str());
	if(err.IsError())
	{
		string error;
		options->ErrorGet(err, error, true);
		error += ". Animation file: ";
		error += animName;
		error += ".anx";
		FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_ani_error_caption),
			error.c_str(), FormMessageBox::m_ok);
		mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormNewAnimation::OnMessageBoxOk);
		options->gui_manager->ShowModal(mb);
		return;
	}
	anxName = anxEdit->Text;
	onOk.Execute(this);
	options->gui_manager->Close(this);	
}

void _cdecl FormNewAnimation::OnMessageBoxOk(GUIControl* sender)
{
	anxEdit->isAccept = false;
	options->kbFocus = anxEdit;
	options->gui_manager->SetKeyboardFocus(anxEdit);


	anxEdit->isAccept = false;
	anxEdit->SetFocus();
	options->kbFocus = anxEdit;	
	options->gui_manager->SetFocus(anxEdit);
	options->gui_manager->SetKeyboardFocus(anxEdit);	
	anxEdit->OnMDown(0, GUIPoint(0,0));
	
}

void _cdecl FormNewAnimation::OnCancelEdit(GUIControl* sender)
{
	options->gui_manager->Close(this);
}

void _cdecl FormNewAnimation::OnEditChange(GUIControl* sender)
{
	buttonOk->Enabled = false;
}

void _cdecl FormNewAnimation::OnEditAccept(GUIControl* sender)
{
	if(anxEdit->isAccept)
	{
		buttonOk->Enabled = true;
	}	
}

void _cdecl FormNewAnimation::OnAnxChoisePath(GUIControl* sender)
{

}




