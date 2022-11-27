//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormSounds
//============================================================================================


#include "FormSounds.h"
#include "Lists\FormListSounds.h"
#include "Lists\FormSoundWaves.h"
#include "Sliders\NumberSlider.h"
#include "ComboBox\FormComboBox.h"
#include "Sliders\FormScrollBar.h"
#include "FormButton.h"
#include "FormEditName.h"
#include "FormAttGraph.h"
#include "FormSndBaseParams.h"
#include "FormExportSoundBank.h"
#include "FormSoundBankOptions.h"
#include "FormMessageBox.h"


FormSounds::FormSounds(GUIWindow * parent, GUIRectangle & rect) : GUIControl(parent)
{
#ifndef NO_TOOLS
	options->sa->EditEnablePreview(true);
#endif
	GUIRectangle r, r1;
	isInitState = false;
	previewScene = options->soundService->CreateScene(SoundSceneName, _FL_);
	Assert(previewScene);
	previewSound = null;
	listOfSoundWaves = null;
	soundBaseParams = null;
	attenuationGraph = null;
	SetClientRect(rect);
	SetDrawRect(rect);
	divX = (rect.w*c_soundslist_width_in_percents)/100;
	//Панель со звуками
	r.x = 0;
	r.y = 1;
	r.w = divX;
	r.h = rect.h - c_bank_space - 1;
	listOfSounds = NEW FormListSounds(options->formOptions.formSounds.sounds, this, r);
	listOfSounds->onChangeFocus.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnChangeSound);
	//Кнопки менеджмента звуков
	buttonDelete = listOfSounds->ButtonAdd();
	buttonDelete->image = &options->imageDelete;
	buttonDelete->onUp.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundDeleteSelectItem);
	buttonDelete->Hint = options->GetString(SndOptions::s_hint_sound_delete);
	buttonRename = listOfSounds->ButtonAdd();
	buttonRename->image = &options->imageRename;
	buttonRename->onUp.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundRenameSelectItem);	
	buttonRename->Hint = options->GetString(SndOptions::s_hint_sound_rename);
	buttonCreateFolder = listOfSounds->ButtonAdd();	
	buttonCreateFolder->onUp.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundAddNewFolder);	
	buttonCreateFolder->image = &options->imageAddFolder;
	buttonCreateFolder->Hint = options->GetString(SndOptions::s_hint_sound_add_folder);
	buttonCreateSound = listOfSounds->ButtonAdd();
	buttonCreateSound->image = &options->imageAddSound;
	buttonCreateSound->onUp.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundAddNewSound);
	buttonCreateSound->Hint = options->GetString(SndOptions::s_hint_sound_add_new);
	buttonSoundBankOptions = listOfSounds->ButtonAdd();
	buttonSoundBankOptions->image = &options->imageSoundBankOpts;
	buttonSoundBankOptions->onUp.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundSoundBankOptions);
	buttonSoundBankOptions->Hint = options->GetString(SndOptions::s_hint_sound_sb_options);
	buttonSoundBankExport = listOfSounds->ButtonAdd();
	buttonSoundBankExport->image = &options->imageSoundBankExp;
	buttonSoundBankExport->onUp.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundSoundBankExport);
	buttonSoundBankExport->Hint = options->GetString(SndOptions::s_hint_sound_sb_export);
	OnChangeSound(null);
}

FormSounds::~FormSounds()
{
	OnSoundStop(this);
	if(previewScene)
	{
		previewScene->Release();
		previewScene = null;
	}
}

//Рисование
void FormSounds::Draw()
{
	//Прямоугольник контрола
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIControl::Draw();
	GUIHelper::Draw2DLine(divX, r.y, divX, r.y + r.h - c_bank_space, options->black2Bkg[2]);
//	GUIHelper::Draw2DLine(divX, r.y + r.h - c_bank_space, divX, r.y + r.h, options->black2Bkg[6]);
	GUIHelper::Draw2DLine(r.x, r.y + r.h - c_bank_space, r.x + r.w, r.y + r.h - c_bank_space, options->black2Bkg[2]);
}

//Был изменён текущий звук
void FormSounds::ChangeSound(const UniqId & newSoundId)
{
	currentSoundId.Reset();
	ProjectSound * snd = project->SoundGet(newSoundId);
	GUIRectangle r;
	if(snd)
	{
		currentSoundId = newSoundId;
		isInitState = true;
		//Список волн
		if(!listOfSoundWaves)
		{
			r = GetDrawRect();
			long height = ((r.h - c_bank_space)*c_swlist_heigth_in_percents)/100;
			r.w = r.w - divX - 1;
			r.x = divX + 1;
			r.y = r.h - c_bank_space;
			r.h = FormSoundWaves::FindListHeight(height);			
			r.y -= r.h;
			listOfSoundWaves = NEW FormSoundWaves(options->formOptions.formSounds.soundWaves, this, r, false);
			listOfSoundWaves->onChangeFocus.SetHandlerEx(this, &FormSounds::OnSoundChangeWave);			
		}
		listOfSoundWaves->SetNewSound(currentSoundId);
		//Слайдеры
		r = GetDrawRect();
		long controlHeight = (r.h*c_graphpart_y)/100 - c_title_h - c_controls_space - c_soundsbtn_border*2;
		if(!soundBaseParams)
		{
			r = GetDrawRect();
			r.x = divX;			
			r.y = 1;
			r.w -= r.x;
			r.h = (r.h*c_graphpart_y)/100 - c_soundsbtn_border - 1;
			soundBaseParams = NEW FormSndBaseParams(this, r, false);
			soundBaseParams->onSoundPlay.SetHandlerEx(this, &FormSounds::OnSoundPlay);
			soundBaseParams->onSoundStop.SetHandlerEx(this, &FormSounds::OnSoundStop);
		}
		if(!attenuationGraph)
		{
			r = GetDrawRect();
			r.x += divX;
			r.y = (r.h*c_graphpart_y)/100;
			r.w -= r.x;
			r.h = listOfSoundWaves->GetDrawRect().y - r.y - c_soundsbtn_border;			
			attenuationGraph = NEW FormAttGraph(this, r, false);
		}
		soundBaseParams->SetCurrentSound(currentSoundId);
		attenuationGraph->SetCurrentSound(currentSoundId);
		attenuationGraph->SetCurrentWave(listOfSoundWaves->GetProjectWaveIdFormFocus());
		isInitState = false;
	}else{
		//Список волн
		DELETE(listOfSoundWaves);
		DELETE(soundBaseParams);
		DELETE(attenuationGraph);		
	}
	//Обновим состояние кнопок
	const UniqId & parentFolder = listOfSounds->GetParentFolder();
	if(parentFolder != project->FolderSounds())
	{
		buttonCreateFolder->image = &options->imageAddFolder;
		buttonCreateFolder->Hint = options->GetString(SndOptions::s_hint_sound_add_folder);
		buttonCreateSound->Enabled = true;
		buttonCreateSound->Visible = true;
		buttonSoundBankOptions->Enabled = false;
		buttonSoundBankOptions->Visible = false;
		buttonSoundBankExport->Enabled = false;
		buttonSoundBankExport->Visible = false;
	}else{
		buttonCreateFolder->image = &options->imageAddNewBank;
		buttonCreateFolder->Hint = options->GetString(SndOptions::s_snd_new_bank);
		buttonCreateSound->Enabled = false;
		buttonCreateSound->Visible = false;
		buttonSoundBankOptions->Enabled = true;
		buttonSoundBankOptions->Visible = true;
		buttonSoundBankExport->Enabled = true;
		buttonSoundBankExport->Visible = true;
	}

}

void _cdecl FormSounds::OnChangeSound(GUIControl* sender)
{
	const UniqId & soundId = listOfSounds->GetFocusSound();
	ChangeSound(soundId);
}

void _cdecl  FormSounds::OnSoundAddNewSound(GUIControl* sender)
{
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder == project->FolderSounds())
	{
		return;
	}
	GUIRectangle r = buttonCreateSound->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);
	fen->Caption = options->GetString(SndOptions::s_snd_enter_new_sound);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundAddNewSound_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnAddNewSoundBank_Ok);
	options->gui_manager->ShowModal(fen); 	
}

void _cdecl  FormSounds::OnSoundAddNewSound_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * nameEditable = fen->GetText();
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(!project->SoundCheckUniqeName(curFolder, nameEditable))
	{
		fen->MarkTextAsIncorrect();
	}
}

void _cdecl  FormSounds::OnAddNewSoundBank_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder == project->FolderSounds())
	{
		return;
	}
	project->SoundAdd(curFolder, name);
	listOfSounds->InitList();
}

void _cdecl FormSounds::OnSoundAddNewFolder(GUIControl* sender)
{
	GUIRectangle r = buttonCreateFolder->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder != project->FolderSounds())
	{
		fen->Caption = options->GetString(SndOptions::s_snd_enter_new_sound_folder);
	}else{
		fen->Caption = options->GetString(SndOptions::s_snd_enter_new_bank);
	}	
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundAddNewFolder_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundAddNewFolder_Ok);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormSounds::OnSoundAddNewFolder_Check(GUIControl* sender)
{
	//Имя
	FormEditName * fen = (FormEditName *)sender;
	const char * nameEditable = fen->GetText();
	//Определимся что именно добавляем
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder == project->FolderSounds())
	{
		//Папка звукового банка
		if(!project->SoundBankCheckUniqeName(nameEditable))
		{
			fen->MarkTextAsIncorrect();
		}
	}else{
		//Обычная папка
		if(!project->FolderCheckUniqeName(curFolder, nameEditable))
		{
			fen->MarkTextAsIncorrect();
		}
	}
}

void _cdecl FormSounds::OnSoundAddNewFolder_Ok(GUIControl* sender)
{
	//Имя
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	//Определимся что именно добавляем
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder == project->FolderSounds())
	{
		project->SoundBankAdd(name);
	}else{
		project->FolderAdd(curFolder, name);
	}
	listOfSounds->InitList();
	listOfSounds->SetFocus(name);
}

void _cdecl FormSounds::OnSoundRenameSelectItem(GUIControl* sender)
{
	const char * text = null;
	const char * cap = null;
	ProjectSound * ps = project->SoundGet(listOfSounds->GetFocusSound());
	if(ps)
	{
		cap = options->GetString(SndOptions::s_snd_rename_sound);
		text = ps->GetName().str;
	}else{
		const UniqId & folder = listOfSounds->GetFocusFolder();
		const ExtName * name = project->FolderGetName(folder);
		if(!name)
		{
			return;
		}
		text = name->str;
		if(!project->SoundBankGetByFolder(folder))
		{
			cap = options->GetString(SndOptions::s_snd_rename_folder);
		}else{
			cap = options->GetString(SndOptions::s_snd_rename_sbank);
		}
	}
	GUIRectangle r = buttonCreateFolder->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);	
	fen->Caption = cap;
	fen->SetText(text);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundRenameSelectItem_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundRenameSelectItem_Ok);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormSounds::OnSoundRenameSelectItem_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	ProjectSound * ps = project->SoundGet(listOfSounds->GetFocusSound());
	if(ps)
	{
		Assert(ps->GetFolder() == listOfSounds->GetParentFolder());
		if(!project->SoundCheckUniqeName(ps->GetFolder(), name, &ps->GetId()))
		{
			fen->MarkTextAsIncorrect();
		}
	}else{
		const UniqId & folder = listOfSounds->GetFocusFolder();
		UniqId parentId;
		Verify(project->FolderGetParent(folder, parentId));
		Assert(parentId == listOfSounds->GetParentFolder());
		ProjectSoundBank * psb = project->SoundBankGetByFolder(folder);
		if(psb)
		{
			if(!project->SoundBankCheckUniqeName(name, &psb->GetId()))
			{
				fen->MarkTextAsIncorrect();
			}
		}else{
			if(!project->FolderCheckUniqeName(parentId, name, &folder))
			{
				fen->MarkTextAsIncorrect();
			}
		}
	}
}

void _cdecl FormSounds::OnSoundRenameSelectItem_Ok(GUIControl* sender)
{
	bool isRenameOk = true;
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	ProjectSound * ps = project->SoundGet(listOfSounds->GetFocusSound());
	if(ps)
	{
		Assert(ps->GetFolder() == listOfSounds->GetParentFolder());
		isRenameOk = project->SoundRename(ps->GetFolder(), name);
	}else{
		const UniqId & folder = listOfSounds->GetFocusFolder();
		UniqId parentId;
		Verify(project->FolderGetParent(folder, parentId));
		Assert(parentId == listOfSounds->GetParentFolder());
		ProjectSoundBank * psb = project->SoundBankGetByFolder(folder);
		if(psb)
		{
			isRenameOk = project->SoundBankRename(folder, name);
		}else{
			isRenameOk = project->FolderRename(folder, name).IsOk();
		}
	}
	if(isRenameOk)
	{
		listOfSounds->InitList();
		listOfSounds->SetFocus(name);
	}else{
		FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_error_op),
			options->GetString(SndOptions::s_form_err_rename), FormMessageBox::m_ok);
		options->gui_manager->ShowModal(mb);
	}
}

void _cdecl FormSounds::OnSoundChangeWave(GUIControl* sender)
{
	UniqId id;
	if(listOfSoundWaves)
	{
		id = listOfSoundWaves->GetProjectWaveIdFormFocus();
	}
	if(attenuationGraph)
	{
		attenuationGraph->SetCurrentWave(id);
	}
}

void _cdecl FormSounds::OnSoundDeleteSelectItem(GUIControl* sender)
{
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
		options->GetString(SndOptions::s_snd_delete_items), FormMessageBox::m_yesno);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormSounds::OnSoundDeleteSelectItem_Process);
	options->gui_manager->ShowModal(mb);
}

void _cdecl FormSounds::OnSoundDeleteSelectItem_Process(GUIControl* sender)
{
	long focusIndex = listOfSounds->GetFocus() - 1;
	if(focusIndex < 0) focusIndex = 0;
	bool isDeleteOk = true;
	//Получаем список выделеных объектов
	array<UniqId> selectItems(_FL_, 256);
	listOfSounds->GetSelectItems(selectItems);
	//Удаляем объекты, проходя по списку
	for(dword i = 0; i < selectItems.Size(); i++)
	{
		ProjectSound * ps = project->SoundGet(selectItems[i]);
		if(ps)
		{
			ErrorId err = project->SoundDelete(ps->GetId());
			if(err.IsError())
			{
				isDeleteOk =  false;
			}
		}else{
			UniqId parentId;
			if(project->FolderGetParent(selectItems[i], parentId))
			{
				Assert(parentId == listOfSounds->GetParentFolder());
				ProjectSoundBank * psb = project->SoundBankGetByFolder(selectItems[i]);
				if(psb)
				{
					isDeleteOk &= project->SoundBankDelete(selectItems[i]);
				}else{
					isDeleteOk &= project->FolderDelete(selectItems[i]).IsOk();
				}
			}
		}
	}
	if(isDeleteOk)
	{
		listOfSounds->InitList();
		listOfSounds->SetFocus(focusIndex);
	}else{
		FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_error_op),
			options->GetString(SndOptions::s_form_err_delete), FormMessageBox::m_ok);
		options->gui_manager->ShowModal(mb);
	}
}


void _cdecl FormSounds::OnSoundSoundBankOptions(GUIControl* sender)
{
	//Проверяем, что у нас корневая папка
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder != project->FolderSounds())
	{
		return;
	}
	//Получаем звуковой банк
	const UniqId & sbFolder = listOfSounds->GetFocusFolder();
	if(!sbFolder.IsValidate())
	{
		return;
	}
	ProjectSoundBank * psb = project->SoundBankGetByFolder(sbFolder);
	if(psb)
	{
		GUIPoint pos = buttonSoundBankOptions->GetDrawRect().pos;
		pos.x += buttonSoundBankOptions->GetDrawRect().w;
		pos.y += buttonSoundBankOptions->GetDrawRect().h;
		FormSoundBankOptions * fsbo = NEW FormSoundBankOptions(this, pos, psb);
		options->gui_manager->ShowModal(fsbo);
	}
}

void _cdecl FormSounds::OnSoundSoundBankExport(GUIControl* sender)
{
	//Проверяем, что у нас корневая папка
	const UniqId & curFolder = listOfSounds->GetParentFolder();
	if(curFolder != project->FolderSounds())
	{
		return;
	}	
	//Собираем список выделенных папок
	array<UniqId> foldersId(_FL_);
	listOfSounds->GetSelectItems(foldersId);
	//Ищим какие звуковые банки добавить на экспорт
	array<UniqId> sbuids(_FL_);
	sbuids.Reserve(foldersId.Size());
	for(dword i = 0; i < foldersId.Size(); i++)
	{
		ProjectSoundBank * psb = project->SoundBankGetByFolder(foldersId[i]);
		if(psb)
		{
			sbuids.Add(psb->GetId());
		}
	}
	//Запускаем выгрузку
	FormExportSoundBank * fesb = NEW FormExportSoundBank(this, sbuids.GetBuffer(), sbuids.Size());
	options->kbFocus = fesb;
	options->dndControl = null;
	options->gui_manager->ShowModal(fesb);
}

void _cdecl FormSounds::OnSoundPlay(GUIControl* sender)
{
	OnSoundStop(this);
	ProjectSound * sound = project->SoundGet(currentSoundId);
	if(!sound)
	{
		return;
	}
	if(!sound->MakePreview())
	{
		FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_cantdoit),
			options->GetString(SndOptions::s_snd_cant_preview), FormMessageBox::m_ok);
		options->gui_manager->ShowModal(mb);
		return;
	}
	previewSoundId = currentSoundId;
	Assert(previewScene);
	previewSound = previewScene->Create(sound->GetName().str, _FL_, true, false);
	Assert(previewSound);
	previewSound->Play();
}

void _cdecl FormSounds::OnSoundStop(GUIControl* sender)
{
	if(previewSound)
	{
		previewSound->Release();
		previewSound = null;
	}
	ProjectSound * sound = project->SoundGet(previewSoundId);
	if(sound)
	{
		sound->StopPreview();
	}
	previewSoundId.Reset();
	options->WavePreviewStop();
}
