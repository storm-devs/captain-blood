
#include "FormSndBaseParams.h"
#include "..\SndOptions.h"
#include "Sliders\NumberSlider.h"
#include "ComboBox\FormComboBox.h"
#include "Lists\FormListBase.h"
#include "FormCheckBox.h"
#include "FormEditName.h"
#include "FormButton.h"
#include "FormMessageBox.h"


FormSndBaseParams::FormSndBaseParams(GUIControl * parent, const GUIRectangle & rect, bool isLightVersion) : GUIControl(parent)
{
	currentSound = null;
	isInitState = true;
	SetClientRect(rect);
	SetDrawRect(rect);
	correctVolume = 3.0f;
	title.Init(options->GetString(SndOptions::s_snd_base_params));
	//Слайдер громкости	
	GUIRectangle r = GetDrawRect();
	long controlY = long(options->uiFont->GetHeight()) + 4;
	long controlHeight = r.h - c_title_h - c_space*2 - controlY*2;
	controlY = c_title_h + c_space + controlY;
	r.x = c_space + c_slider_space;
	r.y = controlY;
	r.w = c_slider_width;
	r.h = controlHeight;
	volumeSlider = NEW NumberSlider(this, r);
	volumeSlider->SetRange(long(sbpc_db_min*1000), long(sbpc_db_max*1000));
	volumeSlider->SetVertical();
	volumeSlider->SetSnapSlider(false);			
	volumeSlider->SetPageSize(10000);
	volumeSlider->SetName(options->GetString(SndOptions::s_snd_sld_volume));
	volumeSlider->onChangeText.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangeVolume);
	volumeSlider->SetInverse(true);
	volumeSlider->SetPosition(sbpc_db_init*1000, true);
	//Приоритет звука
	r.x = c_space + c_slider_space*2 + c_slider_width;
	r.y = controlY;
	r.w = c_slider_width;
	r.h = controlHeight;
	prioritySlider = NEW NumberSlider(this, r);
	prioritySlider->SetVertical();
	prioritySlider->SetSnapSlider(true);
	prioritySlider->SetRange(sbpc_priority_min, sbpc_priority_max);
	prioritySlider->SetPageSize(1);
	prioritySlider->SetName(options->GetString(SndOptions::s_snd_sld_priority));
	prioritySlider->onChangeText.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangePriority);
	prioritySlider->SetInverse(true);
	prioritySlider->SetPosition(sbpc_priority_init, true);
	//Количество проигрываемых звуков
	r.x = c_space + c_slider_space*3 + c_slider_width*2;
	r.y = controlY;
	r.w = c_slider_width;
	r.h = controlHeight;
	counterSlider = NEW NumberSlider(this, r);
	counterSlider->SetVertical();
	counterSlider->SetSnapSlider(false);
	counterSlider->SetRange(0, sbpc_count_max);
	counterSlider->SetPageSize(4);
	counterSlider->SetName(options->GetString(SndOptions::s_snd_sld_count));
	counterSlider->onChangeText.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangeCount);
	counterSlider->SetInverse(true);
	counterSlider->SetPosition(sbpc_count_init - 1, true);
	//Режим выбора волн
	GUIRectangle cr, r2;
	cr.x = c_space + c_slider_space*4 + c_slider_width*3 + 50;
	cr.y = controlY;
	cr.w = c_combo_w;
	cr.h = c_combo_h;
	r2.x = cr.x;
	r2.y = cr.y + cr.h + 1;
	r2.w = c_combo_list_w;
	r2.h = FormListBase::FindListHeight(c_combo_list_h);
	if(!isLightVersion)
	{
		selectList = NEW FormComboBox(this, cr, r2, false);
		selectList->SetTitle(null, options->GetString(SndOptions::s_snd_select_title));
		selectList->Hint = options->GetString(SndOptions::s_snd_select_hint);
		selectList->onChange.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangeSelect);
		selectList->Caption = options->GetString(SndOptions::s_snd_select_name);
		for(long i = sbpc_select_min; i <= sbpc_select_max; i++)
		{
			selectList->AddItem(null, SoundBaseParams::GetSelTextDesc(i), i);
		}
	}else{
		selectList = null;
	}
	//Выбор эффектов
	if(!isLightVersion)
	{
		cr.y += c_combo_step_y;
		cr.w = c_combo_w;
		cr.h = c_combo_h;
		fxList = NEW FormComboBox(this, cr, r2, false);
		fxList->SetTitle(null, options->GetString(SndOptions::s_snd_fx_title));
		fxList->Hint = options->GetString(SndOptions::s_snd_fx_hint);
		fxList->onChange.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangeFx);
		fxList->Caption = options->GetString(SndOptions::s_snd_fx_name);
		for(long i = sbpc_fx_min; i <= sbpc_fx_max; i++)
		{
			fxList->AddItem(null, SoundBaseParams::GetFxTextDesc(i), i);
		}
		//Режим зацикленного проигрывания
		cr.y += c_combo_step_y - 20;
		cr.h = c_check_step_y;
		loopCheckBox = NEW FormCheckBox(this, cr.pos);
		loopCheckBox->SetText(options->GetString(SndOptions::s_snd_loop_title));
		loopCheckBox->onChange.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangeLoop);
		loopCheckBox->Hint = options->GetString(SndOptions::s_snd_loop_hint);
		//Разрешены ли фонемы	
		cr.y += c_check_step_y;
		phonemesCheckBox = NEW FormCheckBox(this, cr.pos);
		phonemesCheckBox->SetText(options->GetString(SndOptions::s_snd_phoneme_title));
		phonemesCheckBox->onChange.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangePhonemes);
		phonemesCheckBox->Hint = options->GetString(SndOptions::s_snd_phoneme_hint);
		//Кнопки проигрывания
		cr.y += c_check_step_y + c_space;
		cr.w = 32;
		cr.h = 32;
		buttonPlay = NEW FormButton(this, cr);
		buttonPlay->image = &options->imagePlay;
		buttonPlay->onUp.SetHandlerEx(this, &FormSndBaseParams::OnPlaySound);
		buttonPlay->Hint = options->GetString(SndOptions::s_hint_sound_play);
		buttonPlay->imageW = 32;
		buttonPlay->imageH = 32;
		cr.x += cr.w + c_space;
		buttonStop = NEW FormButton(this, cr);
		buttonStop->image = &options->imageStop;
		buttonStop->onUp.SetHandlerEx(this, &FormSndBaseParams::OnStopSound);
		buttonStop->Hint = options->GetString(SndOptions::s_hint_sound_stop);
		buttonStop->imageW = 32;
		buttonStop->imageH = 32;
	}else{
		fxList = null;
		loopCheckBox = null;
		phonemesCheckBox = null;
		buttonPlay = null;
		buttonStop = null;
	}
	//Выбор доступных опций
	r = GetDrawRect();	
	cr.x = r.w - c_combo_w - (c_button_w + sgc_header_controls_space)*3 - c_space;
	cr.y = (c_title_h - c_combo_h - 1)/2 - 1;
	cr.w = c_combo_w;
	cr.h = c_combo_h;
	r2.x = cr.x;
	r2.y = cr.y + cr.h + 1;
	r2.w = c_combo_list_w;
	r2.h = FormListBase::FindListHeight(c_combo_list_h);
	setupsList = NEW FormComboBox(this, cr, r2);
	setupsList->SetTitle(&options->imageSoundSetup, options->GetString(SndOptions::s_snd_sel_base_params));
	setupsList->Hint = options->GetString(SndOptions::s_hint_sound_base_params);
	setupsList->onChange.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnChangeSetup);
	FillPresetsList();
	//Кнопки управления сетапами
	r.x = cr.x + cr.w + sgc_header_controls_space;
	r.y = cr.y;
	r.w = c_button_w;
	r.h = c_combo_h;
	buttonAdd = NEW FormButton(this, r);
	buttonAdd->image = &options->imageAddSetup;
	buttonAdd->onUp.SetHandlerEx(this, &FormSndBaseParams::OnSetupAdd);
	buttonAdd->Hint = options->GetString(SndOptions::s_hint_sound_bp_add);
	r.x += c_button_w + sgc_header_controls_space;
	buttonRename = NEW FormButton(this, r);
	buttonRename->image = &options->imageRename;
	buttonRename->onUp.SetHandlerEx(this, &FormSndBaseParams::OnSetupRename);
	buttonRename->Hint = options->GetString(SndOptions::s_hint_sound_bp_rename);
	r.x += c_button_w + sgc_header_controls_space;
	if(options->policy == policy_designer)
	{
		buttonDelete = NEW FormButton(this, r);
		buttonDelete->image = &options->imageDelete;
		buttonDelete->onUp.SetHandlerEx(this, &FormSndBaseParams::OnSetupDelete);
		buttonDelete->Hint = options->GetString(SndOptions::s_hint_sound_bp_add);
	}else{
		buttonDelete = null;
	}
	isInitState = false;
	OnChangeSetup(null);
}

FormSndBaseParams::~FormSndBaseParams()
{

}

//Установить текущий звук, в который прописывать выбранные параметры
void FormSndBaseParams::SetCurrentSound(const UniqId & sndId)
{
	currentSound = project->SoundGet(sndId);
	OnRestoreSelect(null);	
}

//Рисование
void FormSndBaseParams::Draw()
{
	//Рисуем заголовок
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, c_title_h, options->bkg2White[1]);
	GUIHelper::Draw2DLine(r.x, r.y + c_title_h - 1, r.x + r.w, r.y + c_title_h - 1, options->black2Bkg[4]);
	if(buttonPlay)
	{
		options->uiFont->SetColor(options->colorTextLo);
		options->uiFont->Print(float(r.x + c_title_space), float(r.y + (c_title_h - title.h)/2), title.str);
	}
	GUIControl::Draw();
	if(!currentSound || !currentSound->GetCurrentBaseParams())
	{
		dword c = (options->bkg & 0x00ffffff) | 0xc0000000;
		GUIHelper::Draw2DRectAlpha(r.x, r.y + c_title_h + 1, r.w, r.h, c);
	}
}


void _cdecl FormSndBaseParams::OnChangeVolume(GUIControl* sender)
{
	float curDb = Clampf(volumeSlider->GetPosition()*0.001f, sbpc_db_min, sbpc_db_max);
	char buf[32];
	if(curDb < 0.0f)
	{
		curDb = powf(curDb/sbpc_db_min, correctVolume)*sbpc_db_min;
	}else{
		curDb = powf(curDb/sbpc_db_max, correctVolume)*sbpc_db_max;
	}
	float volume = SndDbToVol(curDb);
	//crt_snprintf(buf, sizeof(buf) - 1, "%2.2fdB\n(%g)", curDb, volume);
	if(fabsf(volume - 1.0f) > 1/65536.0f)
	{
		crt_snprintf(buf, sizeof(buf) - 1, "%2.2g%s", curDb, options->GetString(SndOptions::s_snd_vol_db));
	}else{
		curDb = 0.0f;
		volume = 1.0f;
		crt_snprintf(buf, sizeof(buf) - 1, "* %2.2g%s", curDb, options->GetString(SndOptions::s_snd_vol_db));
	}
	volumeSlider->posString.SetString(buf);
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		if(fabsf(sbp->volume - volume) > 1e-10f)
		{
			sbp->volume = volume;
			if(saveObject) saveObject->SetToSave();
			if(currentSound) currentSound->UpdateExportParams();
		}
	}
}

void _cdecl FormSndBaseParams::OnChangePriority(GUIControl* sender)
{
	long priority = prioritySlider->GetPosition();
	prioritySlider->posString.data = options->GetString(SndOptions::s_snd_priority_normal);
	if(priority > sbpc_priority_norm)
	{
		if(priority < sbpc_priority_max)
		{
			prioritySlider->posString.data += options->GetString(SndOptions::s_snd_priority_inc);
			prioritySlider->posString.data += (priority - sbpc_priority_norm);
		}else{
			prioritySlider->posString.data = options->GetString(SndOptions::s_snd_priority_max);
		}
	}else{
		if(priority != sbpc_priority_norm)
		{
			if(priority > sbpc_priority_min)
			{
				prioritySlider->posString.data += options->GetString(SndOptions::s_snd_priority_dec);
				prioritySlider->posString.data += (sbpc_priority_norm - priority);
			}else{
				prioritySlider->posString.data = options->GetString(SndOptions::s_snd_priority_min);
			}
		}
	}
	prioritySlider->posString.Init(prioritySlider->posString.data.c_str());
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		if(sbp->priority != priority)
		{
			sbp->priority = priority;
			if(saveObject) saveObject->SetToSave();
			if(currentSound) currentSound->UpdateExportParams();
		}
	}
}

void _cdecl FormSndBaseParams::OnChangeCount(GUIControl* sender)
{
	long count = counterSlider->GetPosition() + 1;
	counterSlider->posString.data.Empty();
	if(count <= sbpc_count_max)
	{
		counterSlider->posString.data += count;		
	}else{
		counterSlider->posString.data = options->GetString(SndOptions::s_snd_count_inf);
		count = sbpc_count_inf;
	}
	counterSlider->posString.Init(counterSlider->posString.data.c_str());
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		if(sbp->maxCount != count)
		{
			sbp->maxCount = count;			
			if(saveObject) saveObject->SetToSave();
			if(currentSound) currentSound->UpdateExportParams();
		}	
	}
}

void _cdecl FormSndBaseParams::OnChangeSelect(GUIControl* sender)
{
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		long select = selectList->GetCurrentItemId();
		Assert(select >= sbpc_select_min);
		Assert(select <= sbpc_select_max);
		if(sbp->selectMethod != select)
		{
			sbp->selectMethod = select;
			if(saveObject) saveObject->SetToSave();
		}
	}
}

void _cdecl FormSndBaseParams::OnChangeLoop(GUIControl* sender)
{
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		bool isLoop = loopCheckBox->IsChecked();
		long loop = isLoop ? sbpc_loop_one_wave : sbpc_loop_disable;
		Assert(loop >= sbpc_loop_min);
		Assert(loop <= sbpc_loop_max);		
		if(sbp->loopMode != loop)
		{
			sbp->loopMode = loop;
			if(saveObject) saveObject->SetToSave();
		}
	}
}

void _cdecl FormSndBaseParams::OnChangeFx(GUIControl* sender)
{	
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		long fx = fxList->GetCurrentItemId();
		Assert(fx >= sbpc_fx_min);
		Assert(fx <= sbpc_fx_max);
		if(fx != sbp->fx)
		{			
			sbp->fx = fx;
			if(saveObject) saveObject->SetToSave();
		}
	}
}

void _cdecl FormSndBaseParams::OnChangePhonemes(GUIControl* sender)
{
	ProjectObject * saveObject;
	SoundBaseParams * sbp = GetCurrentObject(saveObject);
	if(sbp)
	{
		bool isEnable = phonemesCheckBox->IsChecked();
		if(isEnable)
		{
			if(sbp->modifiers & sbpc_mod_phoneme)
			{
				return;
			}
			sbp->modifiers |= sbpc_mod_phoneme;
		}else{
			if(!(sbp->modifiers & sbpc_mod_phoneme))
			{
				return;
			}
			sbp->modifiers &= ~sbpc_mod_phoneme;
		}		
		if(saveObject) saveObject->SetToSave();
	}
}

void _cdecl FormSndBaseParams::OnChangeSetup(GUIControl* sender)
{
	const UniqId * selUid = null;
	if(setupsList)
	{
		selUid = setupsList->GetCurrentItemUId();
	}
	if(selUid)
	{
		if(selUid->IsValidate())
		{
			if(!project->SndBaseParamsGet(*selUid))
			{			
				//Ненашли при попытки установки глобальный пакаметр, значит переключаемся на локальные
				selUid = null;
			}
		}
	}
	isInitState = true;
	bool needUpdate = false;
	ProjectObject * saveObject = null;
	if(selUid)
	{		
		//Ставим звуку новые параметры
		currentSound->SetGlobalBaseParams(*selUid);
		//Получаем указатель на текущие параметры
		SoundBaseParams * sbp = GetCurrentObject(saveObject);
		Assert(sbp);
		//Громкость
		volumeSlider->Enabled = true;
		float curDb = SndVolToDb(sbp->volume);
		if(curDb < sbpc_db_min)
		{
			curDb = sbpc_db_min;
			sbp->volume = SndDbToVol(curDb);
			needUpdate = true;
		}
		if(curDb > sbpc_db_max)
		{
			curDb = sbpc_db_max;
			sbp->volume = SndDbToVol(curDb);
			needUpdate = true;
		}
		if(curDb < 0)
		{
			curDb = powf(curDb/sbpc_db_min, 1.0f/correctVolume)*sbpc_db_min;
		}else{
			curDb = powf(curDb/sbpc_db_max, 1.0f/correctVolume)*sbpc_db_max;
		}
		volumeSlider->SetPosition(long(curDb*1000.0f));
		//Приоритет звука
		prioritySlider->Enabled = true;
		if(sbp->priority < sbpc_priority_min)
		{
			sbp->priority = sbpc_priority_min;
			needUpdate = true;
		}
		if(sbp->priority > sbpc_priority_max)
		{
			sbp->priority = sbpc_priority_max;
			needUpdate = true;
		}
		prioritySlider->SetPosition(sbp->priority);
		//Количество звуков
		counterSlider->Enabled = true;
		if(sbp->maxCount != sbpc_count_inf)
		{
			if(sbp->maxCount > sbpc_count_max)
			{
				sbp->maxCount = sbpc_count_max;
				needUpdate = true;
			}
			counterSlider->SetPosition(sbp->maxCount - 1);
		}else{
			counterSlider->SetPosition(sbpc_count_max);
		}
		//Режим выбора волн
		if(selectList)
		{
			selectList->Enabled = true;
			if(sbp->selectMethod < sbpc_select_min)
			{
				sbp->selectMethod = sbpc_select_min;
				needUpdate = true;
			}
			if(sbp->selectMethod > sbpc_select_max)
			{
				sbp->selectMethod = sbpc_select_max;
				needUpdate = true;
			}
			selectList->SelectItemById(sbp->selectMethod);
		}
		//Режим зацикленного проигрывания
		if(loopCheckBox)
		{
			loopCheckBox->Enabled = true;
			if(sbp->loopMode < sbpc_loop_min)
			{
				sbp->loopMode = sbpc_loop_min;
				needUpdate = true;
			}
			if(sbp->loopMode > sbpc_loop_max)
			{
				sbp->loopMode = sbpc_loop_max;
				needUpdate = true;
			}
			loopCheckBox->SetCheck(sbp->loopMode != sbpc_loop_disable);
		}
		//Выбор эффектов
		if(fxList)
		{
			fxList->Enabled = true;
			if(sbp->fx < sbpc_fx_min)
			{
				sbp->fx = sbpc_fx_min;
				needUpdate = true;
			}
			if(sbp->fx > sbpc_fx_max)
			{
				sbp->fx = sbpc_fx_max;
				needUpdate = true;
			}
			fxList->SelectItemById(sbp->fx);
		}
		//Разрешены ли фонемы
		if(phonemesCheckBox)
		{
			phonemesCheckBox->Enabled = true;
			phonemesCheckBox->SetCheck((sbp->modifiers & sbpc_mod_phoneme) != 0);
		}
		if(buttonPlay) buttonPlay->Enabled = true;
		if(buttonStop) buttonStop->Enabled = true;
	}else{
		volumeSlider->Enabled = false;
		prioritySlider->Enabled = false;
		counterSlider->Enabled = false;
		if(selectList) selectList->Enabled = false;
		if(loopCheckBox) loopCheckBox->Enabled = false;
		if(fxList) fxList->Enabled = false;
		if(phonemesCheckBox) phonemesCheckBox->Enabled = false;
		if(buttonPlay) buttonPlay->Enabled = false;
		if(buttonStop) buttonStop->Enabled = false;
	}
	if(needUpdate)
	{
		if(saveObject) saveObject->SetToSave();		
	}
	if(currentSound) currentSound->UpdateExportParams();
	isInitState = false;
	setupsList->SelectItemByUId(UniqId::zeroId);
}

void _cdecl FormSndBaseParams::OnSetupAdd(GUIControl* sender)
{
	//Создаём новый график
	GUIPoint pfen = buttonAdd->GetDrawRect().pos;
	pfen.x += buttonAdd->GetDrawRect().w;
	pfen.y += buttonAdd->GetDrawRect().h;
	FormEditName * fen = NEW FormEditName(this, pfen, FormEditName::pp_right_top);
	fen->Caption = options->GetString(SndOptions::s_snd_enter_new_sbp);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnCheckSetupName);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnAddSetupNew);
	fen->onCancel.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnRestoreSelect);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormSndBaseParams::OnCheckSetupName(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	if(!project->SndBaseParamsCheckUniqeName(name))
	{
		fen->MarkTextAsIncorrect();
	}
}

void _cdecl FormSndBaseParams::OnAddSetupNew(GUIControl* sender)
{
	if(!currentSound)
	{
		return;
	}
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	UniqId id;
	if(project->SndBaseParamsAdd(name, &id))
	{
		if(currentSound) currentSound->SetGlobalBaseParams(id);
		setupsList->AddItem(null, name, -1, id);
		setupsList->SelectItemByUId(id);
		OnChangeSetup(this);
	}
}

void _cdecl FormSndBaseParams::OnRestoreSelect(GUIControl* sender)
{
	if(!setupsList) return;
	if(currentSound)
	{
		setupsList->SelectItemByUId(currentSound->GetGlobalBaseParams());
	}
	OnChangeSetup(this);
}

void _cdecl FormSndBaseParams::OnSetupRename(GUIControl* sender)
{
	if(!buttonRename || !currentSound) return;
	ProjectSoundBaseParams * psbp = project->SndBaseParamsGet(currentSound->GetGlobalBaseParams());
	if(!psbp) return;
	GUIPoint pfen = buttonRename->GetDrawRect().pos;
	pfen.x += buttonRename->GetDrawRect().w;
	pfen.y += buttonRename->GetDrawRect().h;
	FormEditName * fen = NEW FormEditName(this, pfen, FormEditName::pp_right_top);
	fen->Caption = options->GetString(SndOptions::s_snd_rename_sbp);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnCheckSetupName);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnSetupRenameProcess);
	fen->onCancel.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnRestoreSelect);
	fen->SetText(psbp->GetName().str);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormSndBaseParams::OnSetupRenameProcess(GUIControl* sender)
{
	if(!setupsList) return;
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	const UniqId & id = currentSound->GetGlobalBaseParams();
	project->SndBaseParamsRename(id, name);
	FillPresetsList();
	setupsList->SelectItemByUId(id);
}

void _cdecl FormSndBaseParams::OnSetupDelete(GUIControl* sender)
{
	if(!buttonDelete) return;
	GUIRectangle r = buttonDelete->GetDrawRect();
	r.y += r.h;
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
				options->GetString(SndOptions::s_snd_delete_base_params), FormMessageBox::m_yesno, this, 
				FormMessageBox::pp_right_top, &r.pos);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormSndBaseParams::OnSetupDeleteProcess);
	options->gui_manager->ShowModal(mb);	
}

void _cdecl FormSndBaseParams::OnSetupDeleteProcess(GUIControl* sender)
{	
	if(!setupsList) return;
	const UniqId * curId = setupsList->GetCurrentItemUId();
	if(!curId || !curId->IsValidate()) return;
	project->SndBaseParamsDelete(*curId);
	if(currentSound)
	{
		if(currentSound->GetGlobalBaseParams() == *curId)
		{
			currentSound->SetGlobalBaseParams(UniqId::zeroId);
		}
	}
	setupsList->SelectItem(-1);
	FillPresetsList();
	OnChangeSetup(null);
}

void _cdecl FormSndBaseParams::OnPlaySound(GUIControl* sender)
{
	onSoundPlay.Execute(this);
}

void _cdecl FormSndBaseParams::OnStopSound(GUIControl* sender)
{
	onSoundStop.Execute(this);
}

//Получить текущую структуру для модификации и текущий объект для сохранения
SoundBaseParams * FormSndBaseParams::GetCurrentObject(ProjectObject * & saveObject)
{
	SoundBaseParams * sbp = null;
	saveObject = null;
	if(currentSound)
	{
		const UniqId & globalSbp = currentSound->GetGlobalBaseParams();
		if(globalSbp.IsValidate())
		{
			ProjectSoundBaseParams * sbpd = project->SndBaseParamsGet(globalSbp);
			if(sbpd)
			{
				sbp = &sbpd->params;
				saveObject = sbpd;
			}
		}else{
			sbp = &currentSound->GetLocalBaseParams();
			saveObject = currentSound;
		}		
	}
	if(isInitState)
	{
		saveObject = null;
	}
	return sbp;
}

//Заполнить список пресэтов
void FormSndBaseParams::FillPresetsList()
{
	if(!setupsList) return;
	setupsList->Clear();
	setupsList->AddItem(null, " Select preset ", -1, UniqId::zeroId);
	const array<ProjectSoundBaseParams *> & sbps = project->SndBaseParamsArray();
	for(dword i = 0; i < sbps.Size(); i++)
	{
		const ExtName & ename = sbps[i]->GetName();
		setupsList->AddItem(null, ename.str, -1, sbps[i]->GetId());
	}
}

