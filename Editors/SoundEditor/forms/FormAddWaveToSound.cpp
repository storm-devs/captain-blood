


#include "FormAddWaveToSound.h"
#include "FormButton.h"
#include "lists\FormWaveSelector.h"



FormAddWaveToSound::FormAddWaveToSound(FormWaveSelectorOptions & opt, GUIControl * parent, const GUIRectangle & rect) : GUIWindow(parent, 0, 0, 1, 1)
{
	bPopupStyle = true;
	bSystemButton = false;
	SetClientRect(rect);
	SetDrawRect(rect);
	GUIWindow::Draw();
	//Заводим кнопки
	GUIRectangle r = GetDrawRect();
	r.x = r.w/2 - c_border - c_button_width;
	r.y = r.h - c_border - c_button_height;
	r.w = c_button_width;
	r.h = c_button_height;
	buttonCancel = NEW FormButton(this, r);
	buttonCancel->onUp.SetHandler(this, (CONTROL_EVENT)&FormAddWaveToSound::OnCancelAdd);
	buttonCancel->text.SetString(options->GetString(SndOptions::s_form_cancel));
	r.x = r.x + c_border + c_button_width + c_border;
	buttonOk = NEW FormButton(this, r);
	buttonOk->text.SetString(options->GetString(SndOptions::s_form_ok));
	buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormAddWaveToSound::OnOkAdd);
	buttonOk->Enabled = true;
	//Список
	r = GetDrawRect();
	long fontHeight = (long)(options->uiFont->GetHeight() + 0.9999f);
	r.x = c_border;
	r.y = c_border*3 + fontHeight;
	r.w -= c_border*2;
	r.h -= r.y + c_border*3 + c_button_height + fontHeight - 1;
	list = NEW FormWaveSelector(opt, this, r);
	list->SetFrame(true);
}

FormAddWaveToSound::~FormAddWaveToSound()
{
	options->WavePreviewStop();
}


//Получить список выделенных звуков
void FormAddWaveToSound::GetWaves(array<UniqId> & chooseWaves)
{
	chooseWaves.Empty();
	long count = list->GetNumLines();
	for(long i = 0; i < count; i++)
	{
		FormLWSElement * le = (FormLWSElement *)list->GetLine(i);
		if(le && le->IsSelect())
		{
			ProjectWave * wave = project->WaveGet(le->GetWave());
			if(wave)
			{
				chooseWaves.Add(wave->GetId());
			}
		}
	}
	if(chooseWaves.Size() == 0)
	{
		long focus = list->GetFocus();
		if(focus >= 0)
		{
			FormLWSElement * le = (FormLWSElement *)list->GetLine(focus);
			ProjectWave * wave = project->WaveGet(le->GetWave());
			if(wave)
			{
				chooseWaves.Add(wave->GetId());
			}
		}
	}
}

//Рисование
void FormAddWaveToSound::Draw()
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

void _cdecl FormAddWaveToSound::OnOkAdd(GUIControl* sender)
{
	options->gui_manager->Close(this);
	onOk.Execute(this);
}

void _cdecl FormAddWaveToSound::OnCancelAdd(GUIControl* sender)
{
	options->gui_manager->Close(this);
	onCancel.Execute(this);
}

