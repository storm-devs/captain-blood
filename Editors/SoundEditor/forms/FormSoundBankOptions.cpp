

#include "FormSoundBankOptions.h"
#include "FormButton.h"
#include "FormEdit.h"

FormSoundBankOptions::FormSoundBankOptions(GUIControl * parent, const GUIPoint & p, ProjectSoundBank * bank) : GUIWindow(parent, 0, 0, 1, 1)
{
	bankId = bank->GetId();
	//Настраиваем и показываем окно
	GUIRectangle r;
	r.x = p.x;
	r.y = p.y;// - c_height - c_border;
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
	buttonCancel->onUp.SetHandler(this, (CONTROL_EVENT)&FormSoundBankOptions::OnCancelEdit);
	buttonCancel->text.SetString(options->GetString(SndOptions::s_form_cancel));
	r.x = c_width/2 + c_border;
	buttonOk = NEW FormButton(this, r);
	buttonOk->text.SetString(options->GetString(SndOptions::s_form_ok));
	buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormSoundBankOptions::OnOkEdit);
	//Контрол редактирования пути
	r.x = c_space;
	r.y = c_border + c_header_height + c_space + c_edit_height + c_border;
	r.w = c_width - c_space*2 - c_nav_button_w - c_border;
	r.h = c_edit_height;
	edit = NEW FormEdit(this, r);
	edit->onChange.SetHandler(this, (CONTROL_EVENT)&FormSoundBankOptions::OnEditChange);
	edit->onAccept.SetHandler(this, (CONTROL_EVENT)&FormSoundBankOptions::OnEditAccept);	
	edit->Text = bank->exportPath;
	edit->UpdateText();
	options->kbFocus = edit;
	options->gui_manager->SetKeyboardFocus(edit);
	//Кнопка выбора пути, куда экспортить
	r.x += r.w + c_border;
	r.w = c_nav_button_w;
	buttonExpPath = NEW FormButton(this, r);
	buttonExpPath->onUp.SetHandler(this, (CONTROL_EVENT)&FormSoundBankOptions::OnChoisePath);
	buttonExpPath->text.SetString(options->GetString(SndOptions::s_snd_sbo_navbutton));
	buttonExpPath->Enabled = false;
	GUIWindow::Draw();	
}

FormSoundBankOptions::~FormSoundBankOptions()
{
}


//Рисование
void FormSoundBankOptions::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	options->render->Print(float(r.x + c_border), float(r.y + c_border), options->colorTextLo, options->GetString(SndOptions::s_snd_soundbank_options));
	options->render->Print(float(r.x + c_space), float(r.y + c_border + c_header_height + c_space), options->colorTextLo, options->GetString(SndOptions::s_snd_soundbank_options_exppath));	

	GUIControl::Draw();
}

void _cdecl FormSoundBankOptions::OnOkEdit(GUIControl* sender)
{
	ProjectSoundBank * sb = project->SoundBankGet(bankId);
	if(sb)
	{
		sb->exportPath = edit->Text;
		sb->SetToSave();
	}
	options->gui_manager->Close(this);	
}

void _cdecl FormSoundBankOptions::OnCancelEdit(GUIControl* sender)
{
	options->gui_manager->Close(this);
}

void _cdecl FormSoundBankOptions::OnEditChange(GUIControl* sender)
{
	buttonOk->Enabled = false;
}

void _cdecl FormSoundBankOptions::OnEditAccept(GUIControl* sender)
{
	buttonOk->Enabled = true;

}

void _cdecl FormSoundBankOptions::OnChoisePath(GUIControl* sender)
{

}


