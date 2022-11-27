//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormSoundWaves
//============================================================================================

#include "FormSoundWaves.h"
#include "..\FormAddWaveToSound.h"


ExtName FormSndWaveElement::volText;
ExtName FormSndWaveElement::wgtText;
long FormSndWaveElement::playWaveIndex = -1;

FormSndWaveElement::FormSndWaveElement(FormListBase & _list, ProjectSound * s, long idx) : FormListElement(_list)
{
	sound = s;
	Assert(sound);
	waveIndex = idx;	
	pw = project->WaveGet(sound->GetWave(waveIndex).waveId);
	if(pw)
	{
		const char * str = pw->GetName().str;
		crt_snprintf(nameData, sizeof(nameData) - 1, "%.2u: %s", waveIndex + 1, str);
	}else{
		if(!sound->GetWave(waveIndex).waveId.IsValidate())
		{
			crt_snprintf(nameData, sizeof(nameData) - 1, "%.2u: %s", waveIndex + 1, "--");
		}else{
			crt_snprintf(nameData, sizeof(nameData) - 1, "%.2u: %s", waveIndex + 1, "- Wave is delete from project! -");
		}
	}
	name.Init(nameData);
	if(!volText.str)
	{
		volText.Init(options->GetString(SndOptions::s_snd_waves_vol));
	}
	if(!wgtText.str)
	{
		wgtText.Init(options->GetString(SndOptions::s_snd_waves_wgt));
	}
	playHighlightVol = 0.0f;
	playHighlight = 0.0f;
	stopHighlight = 0.0f;
}

FormSndWaveElement::~FormSndWaveElement()
{

}

//Получить индекс волны в таблице звука
long FormSndWaveElement::GetWaveIndex()
{
	return waveIndex;
}

//Получить индекс волны
const UniqId & FormSndWaveElement::GetProjectWaveId()
{
	if(!sound) return UniqId::zeroId;
	ProjectSound::Wave & wave = sound->GetWave(waveIndex);
	return wave.waveId;
}

//Нарисовать линию
void FormSndWaveElement::Draw(const GUIRectangle & rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	DrawSelect(rect);
	//Текст	
	DrawText(rect, &name, false);
	long controlWidth = c_controlarea_base;
	if(((FormSoundWaves &)List()).IsShortVersion())
	{		
		controlWidth = rect.w - c_shortname;
	}
	//Рисуем параметры отступая от конца
	if(rect.w >= controlWidth)
	{
		ProjectSound::Wave & wave = sound->GetWave(waveIndex);
		long x = rect.x + rect.w - controlWidth;		
		GUIHelper::Draw2DRectHorizGradient(x - c_enter_area, rect.y, c_enter_area, rect.h, ((options->bkg & 0x00ffffff) | 0x00000000), ((options->bkg & 0x00ffffff) | 0xff000000));
		GUIHelper::Draw2DRect(x, rect.y, controlWidth, rect.h, ((options->bkg & 0x00ffffff) | 0xff000000));
		if(pw)
		{
			dword rectColor = options->black2Bkg[7];
			dword imageColor = 0xa0ffffff;
			if(pw->IsPlay())
			{
				rectColor = options->black2Bkg[5];
				imageColor = 0xffffffff;
			}
			long bx = x + c_controlspace/2;
			long y = rect.y + (rect.h - c_buttonsize)/2;			
			//Кнопки проигрывания
			if(playHighlightVol > 0.0f)
			{
				dword alpha = dword(playHighlightVol*255.0f) << 24;
				GUIHelper::Draw2DRect(bx, y, c_buttonsize, c_buttonsize, alpha | (options->bkg2White[7] & 0x00ffffff));
			}
			GUIHelper::DrawSprite(bx, y, c_buttonsize, c_buttonsize, &options->imagePlayVol, imageColor);
			GUIHelper::DrawWireRect(bx, y, c_buttonsize, c_buttonsize, rectColor);
			bx += c_buttonsize + c_buttonspace;
			if(playHighlight > 0.0f)
			{
				dword alpha = dword(playHighlight*255.0f) << 24;
				GUIHelper::Draw2DRect(bx, y, c_buttonsize, c_buttonsize, alpha | (options->bkg2White[7] & 0x00ffffff));
			}
			GUIHelper::DrawSprite(bx, y, c_buttonsize, c_buttonsize, &options->imagePlay, imageColor);
			GUIHelper::DrawWireRect(bx, y, c_buttonsize, c_buttonsize, rectColor);
			bx += c_buttonsize + c_buttonspace;
			if(stopHighlight > 0.0f)
			{
				dword alpha = dword(stopHighlight*255.0f) << 24;
				GUIHelper::Draw2DRect(bx, y, c_buttonsize, c_buttonsize, alpha | (options->bkg2White[7] & 0x00ffffff));
			}
			GUIHelper::DrawSprite(bx, y, c_buttonsize, c_buttonsize, &options->imageStop, imageColor);
			GUIHelper::DrawWireRect(bx, y, c_buttonsize, c_buttonsize, rectColor);
			//Бегунок громкости
			DrawSlider(rect, x + c_buttonsarea + c_controlspace, wave.volume, c_volwidth, volText);
		}else{
			options->render->Print(float(x + c_buttonsarea + c_controlspace), float(rect.y), options->colorTextLo, volText.str);
			options->render->Print(float(x + volText.w + c_buttonsarea + c_controlspace), float(rect.y), options->colorTextLo, options->GetString(SndOptions::s_snd_waves_silence));
		}
		x += c_buttonsarea + c_controlspace + c_volwidth + c_controlspace;
		//Бегунок веса
		DrawSlider(rect, x, wave.weight, c_weightwidth, wgtText);
		x += c_weightwidth + c_controlspace;		
		//Картинка времени
		long imgh = 16;
		GUIHelper::DrawSprite(x, rect.y + (rect.h - imgh)/2, imgh, imgh, &options->imageChanalTime);
		x += rect.h + c_ctrlhspace;
		//Текст времени использования канала
		float pwPlayTime = -1.0f;
		if(pw)
		{
			pwPlayTime = pw->GetPlayTime();
		}
		if(wave.time > 0.0f && wave.time > pwPlayTime)
		{
			//Пишем текст только если он актуален (заданное время больше чем время волны)
			char buf[32];
			crt_snprintf(buf, sizeof(buf) - 1, "%.3f%s", wave.time, options->GetString(SndOptions::s_units_time_sec));
			buf[sizeof(buf) - 1] = 0;
			float tmh = options->uiFont->GetHeight(buf) + 0.4999f;
			float y = rect.y + (rect.h - tmh)*0.5f;
			options->render->Print(float(x), y, options->colorTextLo, buf);
		}
	}
	if(playHighlightVol > 0.0f)
	{
		playHighlightVol -= api->GetDeltaTime()*3.0f;
	}
	if(playHighlight > 0.0f)
	{
		playHighlight -= api->GetDeltaTime()*3.0f;
	}
	if(stopHighlight > 0.0f)
	{
		stopHighlight -= api->GetDeltaTime()*3.0f;
	}
	//Обновляем громкость, если есть проигрываемая волна
	if(waveIndex == playWaveIndex)
	{
		ProjectSound::Wave & wave = sound->GetWave(waveIndex);
		float vol = wave.volume;
		const SoundBaseParams * params = sound->GetCurrentBaseParams();
		if(params)
		{
			vol *= params->volume;
		}
		pw->PlayVolume(vol);
	}
	//GUIHelper::DrawWireRect(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
}

//Нарисовать бегунок
__forceinline void FormSndWaveElement::DrawSlider(const GUIRectangle & rect, long x, float normValue, long w, ExtName & label)
{
	//Пишем текст
	long y = rect.y + (rect.h - label.h)/2;
	options->render->Print(float(x), float(y), options->colorTextLo, label.str);
	x += label.w;
	w -= label.w;
	//Положение бегунка
	long eadge = long(w*Clampf(normValue));
	//Рисуем бегунок
	GUIHelper::Draw2DRect(x, rect.y + c_ctrlhspace, eadge, rect.h - c_ctrlhspace*2, options->bkg2White[2]);
	GUIHelper::Draw2DRect(x + eadge, rect.y + c_ctrlhspace, w - eadge, rect.h - 10, options->black2Bkg[6]);
	GUIHelper::DrawWireRect(x, rect.y + c_ctrlhspace, w, rect.h - c_ctrlhspace*2, 0xff000000);
	GUIHelper::Draw2DRect(x + eadge - c_controlbar/2, rect.y + 2,  c_controlbar, rect.h - 4, 0xff000000);
}



//Если параметры линия должна быть выше чем данная то вернуть true
bool FormSndWaveElement::IsLessThen(const FormListElement * el) const
{
	return true;
}

//Проверить на совпадение итема
bool FormSndWaveElement::IsThis(const char * str, dword hash, dword len) const
{
	return false;
}

//Проверить попадание по префиксу
bool FormSndWaveElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);	
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormSndWaveElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	long controlWidth = c_controlarea_base;
	if(((FormSoundWaves &)List()).IsShortVersion())
	{		
		controlWidth = rect.w - c_shortname;
	}
	long x = rect.x + rect.w - controlWidth;
	if(pos.x < x)
	{
		return true;
	}
	ProjectSound::Wave & wave = sound->GetWave(waveIndex);
	if(pw)
	{
		Assert(wave.waveId == pw->GetId());
		long bx = x + c_controlspace/2;
		//Проверим на попадание по кнопкам
		if(pos.x < bx + c_buttonsize)
		{
			playWaveIndex = waveIndex;
			pw->Play();
			float vol = wave.volume;
			const SoundBaseParams * params = sound->GetCurrentBaseParams();
			if(params)
			{
				vol *= params->volume;
			}
			pw->PlayVolume(vol);
			playHighlightVol = 1.0f;
			return false;
		}
		bx += c_buttonsize + c_buttonspace;
		if(pos.x >= bx && pos.x <= bx + c_buttonsize)
		{
			pw->Play();
			playHighlight = 1.0f;
			return false;
		}
		bx += c_buttonsize + c_buttonspace;
		if(pos.x >= bx && pos.x <= bx + c_buttonsize)
		{
			options->WavePreviewStop();
			stopHighlight = 1.0f;
			return false;
		}
	}
	x += c_buttonsarea + c_controlspace;
	//Бегунки
	FloatParam evtParam;	
	if(pw)
	{		
		//Бегунок громкости
		if(pos.x >= x + volText.w - c_mousesafrarea && pos.x <= x + c_volwidth + c_mousesafrarea)
		{
			//Новое значение громкости
			evtParam.id = evt_set_new_volume;
			evtParam.param = Clampf((pos.x - x - volText.w)/float(c_volwidth - volText.w), 0.0f, 1.0f);
			if(IsSelect())
			{
				SendEvent(evtParam);
			}else{
				ProjectSound::Wave & wave = sound->GetWave(waveIndex);
				wave.volume = evtParam.param;				
			}
			sound->SetToSave();
			return false;
		}
	}
	x += c_volwidth + c_controlspace;
	//Бегунок веса		
	if(pos.x >= x + wgtText.w - c_mousesafrarea && pos.x <= x + c_weightwidth + c_mousesafrarea)
	{		
		//Новое значение веса			
		evtParam.id = evt_set_new_weight;
		evtParam.param = Clampf((pos.x - x - wgtText.w)/float(c_weightwidth - wgtText.w), 0.0f, 1.0f);
		if(IsSelect())
		{
			SendEvent(evtParam);
		}else{
			ProjectSound::Wave & wave = sound->GetWave(waveIndex);
			wave.weight = evtParam.param;
		}
		sound->SetToSave();
		return false;
	}
	x += c_weightwidth + c_controlspace;	
	//Время волны
	if(pos.x >= x && pos.x <= x + rect.h)
	{
		GUIPoint p;
		p.x = x + rect.h/2;
		p.y = rect.y + rect.h/2;
		List().ScreenToClient(p);
		FormEditTime * fep = NEW FormEditTime(&List(), p, sound, waveIndex);
		options->gui_manager->ShowModal(fep);
		return false;
	}
	return false;
}

//Эвенты
void FormSndWaveElement::Event(const FormListEventData & data)
{
	if(data.id == evt_set_new_volume && IsSelect())
	{
		ProjectSound::Wave & wave = sound->GetWave(waveIndex);
		wave.volume = ((FloatParam &)data).param;
	}else
	if(data.id == evt_set_new_weight && IsSelect())
	{
		ProjectSound::Wave & wave = sound->GetWave(waveIndex);
		wave.weight = ((FloatParam &)data).param;
	}
}

//Активировать элемент
void FormSndWaveElement::OnAction()
{
}


FormSoundWaves::FormSoundWaves(FormSoundWavesOptions & opt, GUIControl * parent, const GUIRectangle & rect, bool isLightVersion) 
: FormListBase(parent, rect),
formOptions(opt)
{
	sound = null;
	//Добавляем кнопки для добавления-удаления волн
	if(!isLightVersion)
	{
		buttonDelete = ButtonAdd();
		buttonDelete->image = &options->imageDelete;
		buttonDelete->onUp.SetHandler(this, (CONTROL_EVENT)&FormSoundWaves::OnWavesDel);
		buttonDelete->Hint = options->GetString(SndOptions::s_hint_sound_remove_wave);	
		buttonAddSilence = ButtonAdd();
		buttonAddSilence->image = &options->imageAddSilence;
		buttonAddSilence->onUp.SetHandler(this, (CONTROL_EVENT)&FormSoundWaves::OnWavesAddSilence);
		buttonAddSilence->Hint = options->GetString(SndOptions::s_hint_sound_add_silence);	
		buttonAdd = ButtonAdd();
		buttonAdd->image = &options->imageAddWave;
		buttonAdd->onUp.SetHandler(this, (CONTROL_EVENT)&FormSoundWaves::OnWavesAdd);
		buttonAdd->Hint = options->GetString(SndOptions::s_hint_sound_add_wave);
	}else{		
		buttonAdd = null;
		buttonAddSilence = null;
		buttonDelete = null;
	}
	isShortIems = false;
//	isShortIems = isLightVersion;
	formOptions.currentSound.Reset();
	//Заполняем список
	InitList();	
}

FormSoundWaves::~FormSoundWaves()
{
	
}

//Инициализировать лист
void FormSoundWaves::DoInitList()
{
	//Перебираем волны в звуке
	if(!formOptions.currentSound.IsValidate())
	{
		return;
	}
	sound = project->SoundGet(formOptions.currentSound);
	if(!sound)
	{
		if(buttonAdd)
		{
			buttonAdd->Enabled = false;
			buttonAddSilence->Enabled = false;
			buttonDelete->Enabled = false;
		}
		formOptions.currentSound.Reset();
		return;
	}
	if(buttonAdd)
	{
		buttonAdd->Enabled = true;
		buttonAddSilence->Enabled = true;
		buttonDelete->Enabled = true;
	}
	dword count = sound->GetWavesCount();
	for(dword i = 0; i < count; i++)
	{
		AddElement(NEW FormSndWaveElement(*this, sound, i));
	}
	SetCaption("List of waves for current sound", null);
}

//Лист был обновлён
void FormSoundWaves::OnListUpdated()
{
	if(sound && formOptions.currentWaveElement >= 0)
	{
		if(formOptions.currentWaveElement < sound->GetWavesCount())
		{
			FormSndWaveElement::RestoreFocus rf;
			rf.id = FormSndWaveElement::evt_restore_focus;
			rf.waveIndex = formOptions.currentWaveElement;
			SendEvent(rf);
			formOptions.currentWaveElement = -1;
		}
	}
}

//Установить новый звук
void FormSoundWaves::SetNewSound(const UniqId & id)
{
	formOptions.currentSound = id;
	formOptions.currentWaveElement = -1;
	InitList();
}

//Получить идентификатор текущей волны
const UniqId & FormSoundWaves::GetProjectWaveIdFormFocus()
{
	FormSndWaveElement * line = (FormSndWaveElement *)GetLine(GetFocus());
	if(!line) return UniqId::zeroId;
	return line->GetProjectWaveId();
}

//Короткая версия
bool FormSoundWaves::IsShortVersion()
{
	return isShortIems;
}

void _cdecl FormSoundWaves::OnWavesAdd(GUIControl* sender)
{
	if(!buttonAdd) return;
	GUIRectangle r = buttonAdd->GetDrawRect();
	r.x -= c_waves_list_width - c_border - c_path_height;
	r.y -= c_waves_list_height;
	r.w = c_waves_list_width;
	r.h = c_waves_list_height;
	FormAddWaveToSound * faw = NEW FormAddWaveToSound(options->formOptions.formSounds.addWave, this, r);
	faw->Caption = options->GetString(SndOptions::s_snd_add_wave_hdr);
	faw->onOk.SetHandler(this, (CONTROL_EVENT)&FormSoundWaves::OnChooseWavesOk);
	options->gui_manager->ShowModal(faw);
}

void _cdecl FormSoundWaves::OnChooseWavesOk(GUIControl* sender)
{
	//Получаем список выделенных звуков
	array<UniqId> chooseWaves(_FL_);
	((FormAddWaveToSound *)sender)->GetWaves(chooseWaves);
	//Добавляем волны к текущему звуку
	if(!sound->AddWaves(chooseWaves.GetBuffer(), chooseWaves.Size()))
	{
		//MessageBox(Не все волны поместились в звуке)
	}
	InitList();
	sound->SetToSave();
}

void _cdecl FormSoundWaves::OnWavesAddSilence(GUIControl* sender)
{
	UniqId silence;
	//Добавляем волны к текущему звуку
	if(!sound->AddWaves(&silence, 1))
	{
		//MessageBox(Не все волны поместились в звуке)
	}
	InitList();
	sound->SetToSave();
}

void _cdecl FormSoundWaves::OnWavesDel(GUIControl* sender)
{
	array<FormListElement *> selected(_FL_);
	array<long> deleteWaves(_FL_);
	FillSelectList(selected);
	deleteWaves.AddElements(selected.Size());
	for(dword i = 0; i < selected.Size(); i++)
	{
		FormSndWaveElement * line = (FormSndWaveElement *)selected[i];
		deleteWaves[i] = line->GetWaveIndex();
	}
	sound->DeleteWaves(deleteWaves.GetBuffer(), deleteWaves.Size());
	InitList();
	sound->SetToSave();
}


FormEditTime::FormEditTime(GUIControl * parent, const GUIPoint & p, ProjectSound * s, long wi) : GUIWindow(parent, 0, 0, 1, 1)
{
	//Параметры которые редактируем и показываем
	sound = s;
	waveIndex = wi;
	ProjectSound::Wave & wave = sound->GetWave(waveIndex);
	ProjectWave * pw = project->WaveGet(wave.waveId);
	if(pw)
	{
		waveTime = pw->GetPlayTime();
	}else{
		waveTime = 0.0f;
	}	
	activeTime = wave.time;
	//Настроим окно
	GUIRectangle r;
	r.x = p.x - c_width;
	r.y = p.y - c_height;
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
	buttonCancel->onUp.SetHandler(this, (CONTROL_EVENT)&FormEditTime::OnCancelEdit);
	buttonCancel->text.SetString(options->GetString(SndOptions::s_form_cancel));
	r.x = c_width/2 + c_border;
	buttonOk = NEW FormButton(this, r);
	buttonOk->text.SetString(options->GetString(SndOptions::s_form_ok));
	buttonOk->onUp.SetHandler(this, (CONTROL_EVENT)&FormEditTime::OnOkEdit);
	//Заводим контрол редактирования числа
	actLabel.Init(options->GetString(SndOptions::s_snd_waves_atime));
	long fontHeight = long(options->uiFont->GetHeight());
	r.x = c_border + fontHeight + c_space + actLabel.w;
	r.y = c_border + c_barheight + c_space + fontHeight + c_space - 3;
	r.w = c_edit_w;
	r.h = fontHeight + 6;
	edit = NEW FormEdit(this, r);
	edit->onChange.SetHandler(this, (CONTROL_EVENT)&FormEditTime::OnEditChange);
	edit->onAccept.SetHandler(this, (CONTROL_EVENT)&FormEditTime::OnEditAccept);
	edit->floatValue = &activeTime;
	edit->min = 0.0f;
	edit->max = 60.0f*60.0f*24.0f;
	edit->isLimitedNumber = true;
	edit->UpdateText();
	options->kbFocus = edit;
	options->gui_manager->SetKeyboardFocus(edit);
	GUIWindow::Draw();
}

FormEditTime::~FormEditTime()
{
}

//Рисование
void FormEditTime::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	//Полоска времени волны
	long x = r.x + c_border;
	long w = c_width - c_border*2;
	long y = r.y + c_border;
	dword lightColor = options->bkg2White[4];
	dword darkColor = options->black2Bkg[5];
	if(waveTime > activeTime)
	{
		GUIHelper::Draw2DRect(x, y, w, c_barheight/2, lightColor);
		GUIHelper::Draw2DRect(x, y + c_barheight/2, coremax(long(w*activeTime/waveTime), 2), c_barheight/2, darkColor);
	}else{
		if(activeTime > 0.0f)
		{
			GUIHelper::Draw2DRect(x, y, w, c_barheight/2, darkColor);
			GUIHelper::Draw2DRect(x, y + c_barheight/2, coremax(long(w*waveTime/activeTime), 2), c_barheight/2, lightColor);
		}		
	}
	GUIHelper::DrawWireRect(x, y, w, c_barheight, options->black2Bkg[0]);
	y += c_barheight + c_space;
	long fontHeight = long(options->uiFont->GetHeight());
	GUIHelper::Draw2DRect(x, y, fontHeight, fontHeight, lightColor);
	GUIHelper::DrawWireRect(x, y, fontHeight, fontHeight, options->black2Bkg[0]);
	options->render->Print(float(x + fontHeight + c_space), float(y), options->colorTextLo, "%s%.3f%s", options->GetString(SndOptions::s_snd_waves_wtime), waveTime, options->GetString(SndOptions::s_units_time_sec));
	y += fontHeight + c_space;
	GUIHelper::Draw2DRect(x, y, fontHeight, fontHeight, darkColor);
	GUIHelper::DrawWireRect(x, y, fontHeight, fontHeight, options->black2Bkg[0]);
	options->render->Print(float(x + fontHeight + c_space), float(y), options->colorTextLo, actLabel.str);
	const char * format = (activeTime > waveTime) ? "%s" : "%s, %s";
	options->render->Print(float(x + fontHeight + c_space + actLabel.w + c_edit_w + 3), float(y), options->colorTextLo, format, options->GetString(SndOptions::s_units_time_sec), options->GetString(SndOptions::s_snd_waves_notemploy));
	GUIControl::Draw();
}


void _cdecl FormEditTime::OnOkEdit(GUIControl* sender)
{
	options->gui_manager->Close(this);
	ProjectSound::Wave & wave = sound->GetWave(waveIndex);
	wave.time = activeTime;
	sound->SetToSave();
}

void _cdecl FormEditTime::OnCancelEdit(GUIControl* sender)
{
	options->gui_manager->Close(this);
}

void _cdecl FormEditTime::OnEditChange(GUIControl* sender)
{
	buttonOk->Enabled = false;
}

void _cdecl FormEditTime::OnEditAccept(GUIControl* sender)
{
	buttonOk->Enabled = true;
}