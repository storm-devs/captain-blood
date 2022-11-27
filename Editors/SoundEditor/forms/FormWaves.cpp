//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaves
//============================================================================================

#include "FormWaves.h"
#include "Lists\FormWaveSelector.h"
#include "Lists\FormWaveFiles.h"
#include "FormEditName.h"
#include "FormMessageBox.h"
#include "Combobox\FormCombobox.h"
#include "FormLabel.h"
#include "Sliders\NumberSlider.h"
#include "FormEdit.h"
#include "FormCheckBox.h"


FormWaves::FormWaves(GUIWindow * parent, GUIRectangle & rect) : GUIControl(parent)
{
#ifndef NO_TOOLS
	options->sa->EditEnablePreview(true);
#endif
	waveDecomposer = null;
	pcFormat = null;
	xboxFormat = null;
	SetClientRect(rect);
	SetDrawRect(rect);
	long divX = rect.w/2;
	long divY = rect.h - rect.h/4;
	GUIRectangle r;
	r.x = 0;
	r.y = 1;
	r.w = divX - c_dividerwidth/2;
	r.h = divY - c_dividerwidth/2;
	listOfWaves = NEW FormWaveSelector(options->formOptions.formWaves.leftPanel, this, r);
	listOfWaves->onChangeFocus.SetHandlerEx(this, &FormWaves::OnWavesSelectorChangeFocus);
	r.x = divX + c_dividerwidth/2;
	r.y = 1;
	r.w = rect.w - r.x - 1;
	r.h = divY - c_dividerwidth/2 - 1;
	listOfFiles = NEW FormWaveFiles(options->formOptions.formWaves.rightPanel, this, r);
	//Добавляем кнопки для добавления-удаления волн
	listOfWavesButtonDelete = listOfWaves->ButtonAdd();
	listOfWavesButtonDelete->image = &options->imageDelete;
	listOfWavesButtonDelete->onUp.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorDelete);
	listOfWavesButtonDelete->Hint = options->GetString(SndOptions::s_hint_waves_remove);
	listOfWavesButtonRename = listOfWaves->ButtonAdd();
	listOfWavesButtonRename->image = &options->imageRename;
	listOfWavesButtonRename->onUp.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorRename);
	listOfWavesButtonRename->Hint = options->GetString(SndOptions::s_hint_waves_rename);
	listOfWavesButtonAddFolder = listOfWaves->ButtonAdd();
	listOfWavesButtonAddFolder->image = &options->imageAddFolder;
	listOfWavesButtonAddFolder->onUp.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorAddFolder);
	listOfWavesButtonAddFolder->Hint = options->GetString(SndOptions::s_hint_waves_add_folder);
	if(options->isDeveloperMode)
	{
		listOfWavesButtonAddFolder = listOfWaves->ButtonAdd();
		listOfWavesButtonAddFolder->image = &options->imageWaveRootFolder;
		listOfWavesButtonAddFolder->onUp.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorHideDecompose);
		listOfWavesButtonAddFolder->Hint = "Developer: hide decompose focuse wave";
		listOfWavesButtonAddFolder = listOfWaves->ButtonAdd();
		listOfWavesButtonAddFolder->image = &options->imageWaveOpenFolder;
		listOfWavesButtonAddFolder->onUp.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorDecomposeWave);
		listOfWavesButtonAddFolder->Hint = "Developer: decompose focuse wave";
	}
	//Параметры волн	
	r.x = c_leftBorder;
	r.y = rect.y + rect.h - rect.h/4 + c_dividerwidth/2 + c_topBorder;
	r.w = c_labelsWidth;
	r.h = c_lineHeight;
	GUIRectangle r1 = r;
	r1.x += c_labelsWidth;
	r1.w = c_listsWidth;
	GUIRectangle rbBase = r1;
	rbBase.x = r1.x + r1.w + 20;
	rbBase.w = c_previewButtons_s;
	rbBase.h = c_previewButtons_s;
	rbBase.y += (r1.h - rbBase.h)/2;	
	//Имя волны
	currentWaveLabel = NEW FormLabel(this, r);
	currentWaveLabel->text.SetString(options->GetString(SndOptions::s_waves_sel_wave));
	currentWaveLabel->image = &options->imageWave;
	currentWave = NEW FormLabel(this, r1);
	r.y += c_y_stepSpace + c_lineHeight;
	r1.y += c_y_stepSpace + c_lineHeight;
	rbBase.y += c_y_stepSpace + c_lineHeight;
	//Строка с заметкой волны
	currentWaveNotesLabel = NEW FormLabel(this, r);
	currentWaveNotesLabel->text.SetString(options->GetString(SndOptions::s_waves_wave_notes));
	currentWaveEdit = NEW FormEdit(this, GUIRectangle(r1.x, r1.y, rect.w - r1.x - r1.x, r1.h));
	currentWaveEdit->onAccept.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnChangeWaveNotes);
	r.y += c_y_stepSpace + c_lineHeight;
	r1.y += c_y_stepSpace + c_lineHeight;
	rbBase.y += c_y_stepSpace + c_lineHeight;
	//Формат PC
	pcFormatLabel = NEW FormLabel(this, r);
	pcFormatLabel->text.SetString(options->GetString(SndOptions::s_waves_pc_format));
	pcFormatLabel->image = &options->imageComputer;
	GUIRectangle r2 = r1;
	r2.y -= c_listsHeightPC;
	r2.h = c_listsHeightPC;
	pcFormat = NEW FormComboBox(this, r1, r2, false);
	pcFormat->onChange.SetHandlerEx(this, &FormWaves::OnChangeWaveExportParams);
	pcFormat->SetTitle(null, options->GetString(SndOptions::s_waves_sel_exp_format_pc));	
	GUIRectangle rb = rbBase;
	pcPlay = NEW FormButton(this, rb);
	pcPlay->image = &options->imagePlay;
	pcPlay->Hint = options->GetString(SndOptions::s_hint_waves_pc_play);
	pcPlay->onUp.SetHandlerEx(this, &FormWaves::OnPreviewPlayWavePC);
	rb.x += rb.w + c_dividerwidth;
	GUIPoint point = rb.pos;	
	rb.x += rb.w + c_dividerwidth;
	r.y += c_y_stepSpace + c_lineHeight;
	r1.y += c_y_stepSpace + c_lineHeight;
	rbBase.y += c_y_stepSpace + c_lineHeight;
	//Формат XBOX
	xboxFormatLabel = NEW FormLabel(this, r);
	xboxFormatLabel->text.SetString(options->GetString(SndOptions::s_waves_xbox_format));
	xboxFormatLabel->image = &options->imageXbox;
	r2 = r1;
	r2.y -= c_listsHeightXBOX;
	r2.h = c_listsHeightXBOX;
	xboxFormat = NEW FormComboBox(this, r1, r2, false);
	xboxFormat->onChange.SetHandlerEx(this, &FormWaves::OnChangeWaveExportParams);
	xboxFormat->SetTitle(null, options->GetString(SndOptions::s_waves_sel_exp_format_xbox));
	rb = rbBase;	
	xboxPlay = NEW FormButton(this, rb);
	xboxPlay->image = &options->imagePlay;
	xboxPlay->Hint = options->GetString(SndOptions::s_hint_waves_xbox_play);
	xboxPlay->onUp.SetHandlerEx(this, &FormWaves::OnPreviewPlayWaveXbox);
	point.x += rb.x + rb.w + c_dividerwidth;
	point.y += rb.y + rb.h;
	GUIPoint bottomPoint = rb.pos;
	r.y += c_y_stepSpace + c_lineHeight;
	r1.y += c_y_stepSpace + c_lineHeight;
	rbBase.y += c_y_stepSpace + c_lineHeight;
	//Галки, которые позволяет игнорировать дефекты
	if(options->policy == policy_designer || options->isDeveloperMode)
	{
		GUIPoint cbPos = r.pos;
		cbPos.y += c_lineHeight;	
		ignorePACheckBox = NEW FormCheckBox(this, cbPos);
		ignorePACheckBox->SetText(options->GetString(SndOptions::s_waves_ignore_pa));
		ignorePACheckBox->onChange.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnChangeIgnorePA);
		ignorePACheckBox->Hint = options->GetString(SndOptions::s_hint_waves_ignore_pa);
		cbPos.x += c_checkBoxWidth;
		ignoreMACheckBox = NEW FormCheckBox(this, cbPos);
		ignoreMACheckBox->SetText(options->GetString(SndOptions::s_waves_ignore_ma));
		ignoreMACheckBox->onChange.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnChangeIgnoreMA);
		ignoreMACheckBox->Hint = options->GetString(SndOptions::s_hint_waves_ignore_ma);
		cbPos.x += c_checkBoxWidth;
		ignoreClkCheckBox = NEW FormCheckBox(this, cbPos);
		ignoreClkCheckBox->SetText(options->GetString(SndOptions::s_waves_ignore_clk));
		ignoreClkCheckBox->onChange.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnChangeIgnoreClk);
		ignoreClkCheckBox->Hint = options->GetString(SndOptions::s_hint_waves_ignore_clk);
		cbPos.x += c_checkBoxWidth;
		ignoreSlnCheckBox = NEW FormCheckBox(this, cbPos);
		ignoreSlnCheckBox->SetText(options->GetString(SndOptions::s_waves_ignore_sln));
		ignoreSlnCheckBox->onChange.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnChangeIgnoreSln);
		ignoreSlnCheckBox->Hint = options->GetString(SndOptions::s_hint_waves_ignore_sln);
		r.y += c_y_stepSpace + c_lineHeight;
		r1.y += c_y_stepSpace + c_lineHeight;
		rbBase.y += c_y_stepSpace + c_lineHeight;
		r.y += c_y_stepSpace + c_lineHeight;
		r1.y += c_y_stepSpace + c_lineHeight;
		rbBase.y += c_y_stepSpace + c_lineHeight;
	}else{
		ignorePACheckBox = null;
		ignoreMACheckBox = null;
		ignoreSlnCheckBox = null;
		ignoreClkCheckBox = null;
	}
	//Контролы находящиеся по середине опций волны
	point.x = point.x/2 + c_dividerwidth;
	point.y /= 2;
	rb.x = point.x;
	rb.y = point.y - c_previewButtons_s/2;
	rb.size = rbBase.size;
	stop = NEW FormButton(this, rb);
	stop->image = &options->imageStop;
	stop->Hint = options->GetString(SndOptions::s_hint_waves_stop);
	stop->onUp.SetHandlerEx(this, &FormWaves::OnStop);
	rb.x += rb.w + c_dividerwidth;
	rb.y -= c_previewButtons_s/2;
	rb.h += c_previewButtons_s;
	rb.w += c_previewButtons_s/2;
	waveMirror = NEW NumberSlider(this, rb);
	waveMirror->SetRange(0, 1);
	waveMirror->SetPageSize(1);
	waveMirror->SetPosition(1);
	waveMirror->SetInverse(true);
	waveMirror->SetSnapSlider(true);
	waveMirror->DisableMarkers(true);
	waveMirror->DisableDrawText(true);
	rb.x += rb.w + c_dividerwidth;
	rb.w = 200;
	waveMirrorLabel = NEW FormLabel(this, rb);
	waveMirror->onUpdate.SetHandlerEx(this, &FormWaves::OnPreviewChangeWave);
	OnPreviewChangeWave(null);

	UpdateFocus();
	OnWavesSelectorChangeFocus(null);
}

FormWaves::~FormWaves()
{
	if(waveDecomposer) delete waveDecomposer;
}

//Сообщения
bool FormWaves::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if(message == GUIMSG_FORCE_DWORD)
	{
		if(lparam == sndmsg_switchfocus)
		{
			options->formOptions.formWaves.isFocusLeftPanel = !options->formOptions.formWaves.isFocusLeftPanel;
			UpdateFocus();
		}else
		if(lparam == sndmsg_stopmodal)
		{
			listOfWaves->InitList();
			listOfFiles->InitList();
			UpdateFocus();
		}
	}
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Обновить фокус
void FormWaves::UpdateFocus()
{
	if(options->formOptions.formWaves.isFocusLeftPanel)
	{
		options->kbFocus = listOfWaves;
		options->gui_manager->SetKeyboardFocus(listOfWaves);
	}else{
		options->kbFocus = listOfFiles;
		options->gui_manager->SetKeyboardFocus(listOfFiles);
	}
}

//Рисование
void FormWaves::Draw()
{
	listOfWavesButtonRename->Enabled = listOfWaves->IsCanProcess();
	listOfWavesButtonDelete->Enabled = listOfWaves->IsCanProcess();
	waveMirrorLabel->Enabled = false;	
	waveMirrorLabel->image = null;
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	bool isPlayMirror = false;
	if(wave)
	{
		if(wave->IsPlayMirror())
		{
			waveMirrorLabel->Enabled = waveMirrorLabel->Visible;
			isPlayMirror = true;
		}
	}
	if(!isPlayMirror)
	{
		pcPlay->image = &options->imagePlay;
		xboxPlay->image = &options->imagePlay;
	}
	GUIControl::Draw();
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	long divX = r.x + r.w/2;
	long divY = r.y + r.h - r.h/4;	
	GUIHelper::Draw2DLine(divX, r.y, divX, r.y + divY, options->black2Bkg[0]);
	GUIHelper::Draw2DLine(r.x, r.y + divY, r.x + r.w, r.y + divY, options->black2Bkg[0]);
	if(waveDecomposer)
	{
		waveDecomposer->Draw(r.x, r.y + divY, r.w, r.y + r.h - divY);
	}
}

void _cdecl FormWaves::OnWavesSelectorAddFolder(GUIControl* sender)
{
	GUIRectangle r = listOfWavesButtonAddFolder->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);
	fen->Caption = options->GetString(SndOptions::s_snd_enter_new_sound_folder);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWaveAddNewFolder_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWaveAddNewFolder_Ok);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormWaves::OnWaveAddNewFolder_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * nameEditable = fen->GetText();
	//Проверяем имя на длинну
	if(string::IsEmpty(nameEditable))
	{
		fen->MarkTextAsIncorrect();
		return;
	}
	const UniqId & currentFolder = listOfWaves->GetCurrentFolder();
	if(!project->FolderCheckUniqeName(currentFolder, fen->GetText()))
	{
		fen->MarkTextAsIncorrect();
	}
}

void _cdecl FormWaves::OnWaveAddNewFolder_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	const UniqId & currentFolder = listOfWaves->GetCurrentFolder();
	project->FolderAdd(currentFolder, name);
	listOfWaves->InitList();
}


void _cdecl FormWaves::OnWavesSelectorRename(GUIControl* sender)
{
	GUIRectangle r = listOfWavesButtonRename->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_right_top);
	ProjectWave * pw = project->WaveGet(listOfWaves->GetFocusWave());
	const ExtName * fn = project->FolderGetName(listOfWaves->GetFocusFolder());
	if(pw)
	{
		fen->Caption = options->GetString(SndOptions::s_snd_rename_wave);
		fen->SetText(pw->GetName().str);
	}else
	if(fn)
	{
		fen->Caption = options->GetString(SndOptions::s_snd_rename_folder);
		fen->SetText(fn->str);
	}else{
		delete fen;
		return;
	}
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorRename_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorRename_Ok);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormWaves::OnWavesSelectorRename_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	if(string::IsEmpty(name))
	{
		fen->MarkTextAsIncorrect();
	}
	const UniqId & currentFolder = listOfWaves->GetCurrentFolder();
	const UniqId & waveId = listOfWaves->GetFocusWave();
	ProjectWave * pw = project->WaveGet(waveId);
	if(pw)
	{
		Assert(currentFolder == pw->GetFolder());
		if(project->WaveCheckUniqeName(pw->GetFolder(), name, &pw->GetId()))
		{
			return;
		}
	}
	const UniqId & focusFolder = listOfWaves->GetFocusFolder();
	if(project->FolderCheckUniqeName(currentFolder, name, &focusFolder))
	{
		UniqId parentId;
		Verify(project->FolderGetParent(focusFolder, parentId));
		Assert(parentId == currentFolder);
		return;
	}
	fen->MarkTextAsIncorrect();
}


void _cdecl FormWaves::OnWavesSelectorRename_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	if(!string::IsEmpty(name))
	{
		const UniqId & waveId = listOfWaves->GetFocusWave();
		ProjectWave * pw = project->WaveGet(waveId);
		if(pw)
		{
			project->WaveRename(waveId, name);
		}else{
			project->FolderRename(listOfWaves->GetFocusWave(), name);
		}
		listOfWaves->InitList();
	}
}


void _cdecl FormWaves::OnWavesSelectorDelete(GUIControl* sender)
{
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
		                                      options->GetString(SndOptions::s_snd_delete_items), FormMessageBox::m_yesno);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormWaves::OnWavesSelectorDelete_Process);
	options->gui_manager->ShowModal(mb);
}

void _cdecl FormWaves::OnWavesSelectorDelete_Process(GUIControl* sender)
{
	array<UniqId> selElements(_FL_, 256);
	listOfWaves->GetSelectElements(selElements);
	for(dword i = 0; i < selElements.Size(); i++)
	{
		const UniqId & id = selElements[i];
		ProjectWave * pw = project->WaveGet(id);
		if(pw)
		{
			//Удаляем волну
			project->WaveDelete(id);
		}else
		if(project->FolderGetName(id) != null)
		{
			//Удаляем папку
			project->FolderDelete(id);
		}		
	}
	//Строим список заново
	listOfWaves->InitList();	
}

void _cdecl FormWaves::OnWavesSelectorChangeFocus(GUIControl* sender)
{	
	exportedWaveSize.Empty();
	if(!pcFormat || !xboxFormat) return;
	OnStop(null);
	Assert(listOfWaves);
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	currentWaveEdit->Text = "";	
	pcFormat->Clear();
	xboxFormat->Clear();
	if(!wave)
	{
		currentWaveLabel->Visible = false;
		currentWave->Visible = false;
		currentWaveNotesLabel->Visible = false;
		currentWaveEdit->Visible = false;
		currentWaveEdit->Enabled = false;
		currentWaveEdit->UpdateText();
		pcFormatLabel->Visible = false;
		xboxFormatLabel->Visible = false;
		pcFormat->Visible = false;
		pcFormat->Enabled = false;
		pcPlay->Visible = false;
		pcPlay->Enabled = false;
		stop->Visible = false;
		stop->Enabled = false;
		waveMirror->Visible = false;
		waveMirror->Enabled = false;
		waveMirrorLabel->Visible = false;
		waveMirrorLabel->Enabled = false;
		xboxFormat->Visible = false;
		xboxFormat->Enabled = false;
		xboxPlay->Visible = false;
		xboxPlay->Enabled = false;
		if(ignorePACheckBox)
		{
			ignorePACheckBox->Visible = false;
			ignorePACheckBox->Enabled = false;
			ignoreMACheckBox->Visible = false;
			ignoreMACheckBox->Enabled = false;
			ignoreSlnCheckBox->Visible = false;
			ignoreSlnCheckBox->Enabled = false;
			ignoreClkCheckBox->Visible = false;
			ignoreClkCheckBox->Enabled = false;
		}
		return;
	}
	//Настроим контролы
	currentWaveLabel->Visible = true;
	currentWave->Visible = true;
	currentWaveNotesLabel->Visible = true;
	currentWaveEdit->Visible = true;
	currentWaveEdit->Enabled = true;
	currentWaveEdit->Text = wave->GetWaveNotes();
	pcFormatLabel->Visible = true;
	xboxFormatLabel->Visible = true;
	pcFormat->Visible = true;
	pcFormat->Enabled = true;
	pcPlay->Visible = true;
	pcPlay->Enabled = true;
	stop->Visible = true;
	stop->Enabled = true;
	waveMirror->Visible = true;
	waveMirror->Enabled = true;
	waveMirrorLabel->Visible = true;
	waveMirrorLabel->Enabled = true;
	xboxFormat->Visible = true;
	xboxFormat->Enabled = true;
	xboxPlay->Visible = true;
	xboxPlay->Enabled = true;
	if(ignorePACheckBox)
	{
		ignorePACheckBox->Visible = true;
		ignorePACheckBox->Enabled = true;	
		ignoreMACheckBox->Visible = true;
		ignoreMACheckBox->Enabled = true;
		ignoreSlnCheckBox->Visible = true;
		ignoreSlnCheckBox->Enabled = true;
		ignoreClkCheckBox->Visible = true;
		ignoreClkCheckBox->Enabled = true;
	}
	dword sampleRates[] = {44100, 22050, 11025};
	dword qualites[] = {ProjectWave::wce_compression_max, ProjectWave::wce_compression_med, ProjectWave::wce_compression_min};
	const char * qualitesName[] = {options->GetString(SndOptions::s_quality_best),
							       options->GetString(SndOptions::s_quality_medium), 
	                               options->GetString(SndOptions::s_quality_low)};
	dword pcSetFormat = 0;
	dword xboxSetFormat = 0;
	char buffer[256];
	//XMA
	for(dword i = 0; i < ARRSIZE(sampleRates); i++)
	{
		Assert(ARRSIZE(qualites) == ARRSIZE(qualitesName));
		for(dword j = 0; j < ARRSIZE(qualites); j++)
		{
			crt_snprintf(buffer, ARRSIZE(buffer), "XMA %u, %s", sampleRates[i], qualitesName[j]);
			xboxFormat->AddItem(&options->imageXma, buffer, c_format_xma | (qualites[j] << c_format_quality_shift) | sampleRates[i]);
		}
	}
	//PCM
	for(dword i = 0; i < ARRSIZE(sampleRates); i++)
	{
		crt_snprintf(buffer, ARRSIZE(buffer), "PCM %u", sampleRates[i]);
		pcFormat->AddItem(&options->imagePCM, buffer, c_format_pcm | sampleRates[i]);
		xboxFormat->AddItem(&options->imagePCM, buffer, c_format_pcm | sampleRates[i]);
	}
	//xWMA
	for(dword i = 0; i < ARRSIZE(sampleRates); i++)
	{
		if(wave->IsCanXWmaEncode(sampleRates[i]))
		{
			crt_snprintf(buffer, ARRSIZE(buffer), "xWMA %u", sampleRates[i]);
			pcFormat->AddItem(&options->imageXWma, buffer, c_format_xwma | sampleRates[i]);
			xboxFormat->AddItem(&options->imageXWma, buffer, c_format_xwma | sampleRates[i]);
		}
	}	
	//Установим текущие параметры		
	wave->GetDescription(currentWave->text.data, currentWave->Hint);
	currentWave->text.Init(currentWave->text.data.c_str());
	const ProjectWave::WaveOptions & pcOptions = wave->GetExportOptions(ProjectWave::wce_platform_pc);
	const ProjectWave::WaveOptions & xboxOptions = wave->GetExportOptions(ProjectWave::wce_platform_xbox);
	//Преобразуем характеристики в задаваемые
	long minDistPC = 0x7fffffff;
	long bestRatePC = sampleRates[0];
	long minDistXbox = 0x7fffffff;
	long bestRateXbox = sampleRates[0];
	for(dword i = 0; i < ARRSIZE(sampleRates); i++)
	{
		long d = abs(long(sampleRates[i] - pcOptions.samplerate));
		if(d < minDistPC)
		{
			bestRatePC = sampleRates[i];
			minDistPC = d;
		}
		d = abs(long(sampleRates[i] - xboxOptions.samplerate));
		if(d < minDistXbox)
		{
			bestRateXbox = sampleRates[i];
			minDistXbox = d;
		}
	}	
	minDistPC = 0x7fffffff;	
	long bestQualityPC = qualites[0];
	minDistXbox = 0x7fffffff;
	long bestQualityXbox = qualites[0];
	for(dword i = 0; i < ARRSIZE(qualites); i++)
	{
		long d = abs(long(qualites[i] - pcOptions.compression));
		if(d < minDistPC)
		{
			bestQualityPC = qualites[i];
			minDistPC = d;
		}
		d = abs(long(qualites[i] - xboxOptions.compression));
		if(d < minDistXbox)
		{
			bestQualityXbox = qualites[i];
			minDistXbox = d;
		}
	}
	//Формат
	dword bestPCFormat = c_format_pcm;
	bestQualityPC = 0;
	if(pcOptions.format == ProjectWave::wce_format_xwma && wave->IsCanXWmaEncode(pcOptions.samplerate))
	{
		bestPCFormat = c_format_xwma;
	}
	dword bestXboxFormat = c_format_xma;
	switch(xboxOptions.format)
	{
	case ProjectWave::wce_format_16bit:
		bestQualityXbox = 0;
		bestXboxFormat = c_format_pcm;
		break;
	case ProjectWave::wce_format_xma:
		break;
	case ProjectWave::wce_format_xwma:
		bestQualityXbox = 0;
		bestXboxFormat = c_format_xwma;
		break;
	default:
		Assert(false);
	}
	//Устанавливаем текущее значение
	Assert((bestRatePC & c_format_rate_mask) == bestRatePC);
	Assert((bestRateXbox & c_format_rate_mask) == bestRateXbox);
	Assert((bestQualityPC & c_format_quality_mask) == bestQualityPC);
	Assert((bestQualityXbox & c_format_quality_mask) == bestQualityXbox);
	bestQualityPC <<= c_format_quality_shift;
	bestQualityXbox <<= c_format_quality_shift;
	pcFormat->SelectItemById(bestPCFormat | bestQualityPC | bestRatePC);
	xboxFormat->SelectItemById(bestXboxFormat | bestQualityXbox | bestRateXbox);
	if(ignorePACheckBox)
	{
		ignorePACheckBox->SetCheck(wave->GetDefectIgnoreAmp());
		ignoreMACheckBox->SetCheck(wave->GetDefectIgnoreMid());
		ignoreSlnCheckBox->SetCheck(wave->GetDefectIgnoreSln());
		ignoreClkCheckBox->SetCheck(wave->GetDefectIgnoreClk());
	}
	currentWaveEdit->UpdateText();
	OnPreviewChangeWave(null);
}

void _cdecl FormWaves::OnChangeWaveNotes(GUIControl* sender)
{
	if(!currentWaveEdit->Visible) return;
	ProjectWave * pwave = project->WaveGet(listOfWaves->GetFocusWave());	
	if(pwave)
	{
		pwave->SetWaveNotes(currentWaveEdit->Text.c_str());
		currentWaveEdit->Text = pwave->GetWaveNotes();
		currentWaveEdit->Visible = false;
		currentWaveEdit->UpdateText();
		OnUpdateWaveNotesHint(null);
		currentWaveEdit->Visible = true;
	}
}

void _cdecl FormWaves::OnUpdateWaveNotesHint(GUIControl* sender)
{
	dword len = currentWaveEdit->Text.Len();	
	currentWaveEdit->Hint.Reserve(len + 64);
	currentWaveEdit->Hint.Empty();
	for(dword i = 0, cnt = 0; i < len; i++, cnt++)
	{
		char c = currentWaveEdit->Text[i];
		currentWaveEdit->Hint += c;
		if((cnt > 40 && c == ' ') || cnt > 60)
		{
			cnt = 0;	
			currentWaveEdit->Hint += '\n';
		}
	}
}

void _cdecl FormWaves::OnChangeWaveExportParams(GUIControl* sender)
{
	//Получаем волну, которую будем модифицировать
	if(!pcFormat || !xboxFormat) return;
	Assert(sender == pcFormat || sender == xboxFormat);
	Assert(listOfWaves);
	array<UniqId> elements(_FL_, 256);
	listOfWaves->GetSelectElements(elements);
	for(dword i = 0; i < elements.Size(); i++)
	{
		ProjectWave * wave = project->WaveGet(elements[i]);
		bool needWaveReExportPC = false;
		bool needWaveReExportXbox = false;
		ProjectWave::WaveOptions pcOptions = wave->GetExportOptions(ProjectWave::wce_platform_pc);
		ProjectWave::WaveOptions xboxOptions = wave->GetExportOptions(ProjectWave::wce_platform_xbox);
		//Переносим параметры PC
		if(sender == pcFormat)
		{			
			dword formatValue = pcFormat->GetCurrentItemId();
			needWaveReExportPC = ApplyChangeWaveExportParams(pcOptions, formatValue, wave->IsCanXWmaEncode(pcOptions.samplerate));
		}
		//Переносим параметры Xbox
		if(sender == xboxFormat)
		{			
			dword formatValue = xboxFormat->GetCurrentItemId();
			needWaveReExportXbox = ApplyChangeWaveExportParams(xboxOptions, formatValue, wave->IsCanXWmaEncode(xboxOptions.samplerate));
		}
		//Удаляем экспортные данные, если нужно
		if(needWaveReExportPC || needWaveReExportXbox)
		{
			wave->DeleteExportData();
			if(needWaveReExportPC)
			{
				wave->SetExportOptions(ProjectWave::wce_platform_pc, pcOptions);
			}
			if(needWaveReExportXbox)
			{
				wave->SetExportOptions(ProjectWave::wce_platform_xbox, xboxOptions);
			}
			wave->SetToSave();
		}
	}
}

bool FormWaves::ApplyChangeWaveExportParams(ProjectWave::WaveOptions & opts, dword formatValue, bool isCanXwma)
{
	bool isChange = false;
	if(formatValue != -1)
	{		
		dword rate = formatValue & c_format_rate_mask;
		dword format = 0;
		switch(formatValue & c_format_mask)
		{
		case c_format_pcm:
			format = ProjectWave::wce_format_16bit;
			break;
		case c_format_xwma:
			if(!isCanXwma) return false;
			format = ProjectWave::wce_format_xwma;
			break;
		case c_format_xma:
			format = ProjectWave::wce_format_xma;
			break;
		default:
			Assert(false);
		}		
		dword quality = (formatValue >> c_format_quality_shift) & c_format_quality_mask;
		if(opts.samplerate != rate)
		{
			opts.samplerate = rate;
			isChange = true;
		}
		if(opts.format != format)
		{
			opts.format = format;
			isChange = true;
		}
		switch(opts.format)
		{
		case ProjectWave::wce_format_xma:
		//case ProjectWave::wce_format_xwma:
			if(opts.compression != quality)
			{
				opts.compression = quality;
				isChange = true;
			}
			break;
		}
	}
	return isChange;
}

void _cdecl FormWaves::OnPreviewPlayWavePC(GUIControl* sender)
{
	OnStop(null);
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	ErrorId errorCode;
	if(!wave->PlayExported(ProjectWave::wce_platform_pc, waveMirror->GetPosition() == 0, errorCode))
	{
		if(errorCode.IsError())
		{
			string error;
			options->ErrorGet(errorCode, error, true);
			FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_cantdoit), error.c_str(), FormMessageBox::m_ok);
			options->gui_manager->ShowModal(mb);
		}
	}
	exportedWaveSize = ", PC: ";
	exportedWaveSize += wave->GetExportedWaveSize(ProjectWave::wce_platform_pc)/1024;
	exportedWaveSize += "kb";
	pcPlay->image = &options->imageSound;
	xboxPlay->image = &options->imagePlay;
	OnPreviewChangeWave(null);
}

void _cdecl FormWaves::OnPreviewPlayWaveXbox(GUIControl* sender)
{
	OnStop(null);
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	ErrorId errorCode;
	if(!wave->PlayExported(ProjectWave::wce_platform_xbox, waveMirror->GetPosition() == 0, errorCode))
	{
		if(errorCode.IsError())
		{
			string error;
			options->ErrorGet(errorCode, error, true);
			FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_cantdoit), error.c_str(), FormMessageBox::m_ok);
			options->gui_manager->ShowModal(mb);
		}
	}
	exportedWaveSize = ", Xbox: ";
	exportedWaveSize += wave->GetExportedWaveSize(ProjectWave::wce_platform_xbox)/1024;
	exportedWaveSize += "kb";
	pcPlay->image = &options->imagePlay;
	xboxPlay->image = &options->imageSound;
	OnPreviewChangeWave(null);
}


void _cdecl FormWaves::OnPreviewChangeWave(GUIControl* sender)
{
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	wave->PlaySwitch(waveMirror->GetPosition() == 0);
	if(waveMirror->GetPosition() == 0)
	{
		waveMirrorLabel->text.data = options->GetString(SndOptions::s_waves_preview_original);		
		waveMirrorLabel->text.data += ", ";
		waveMirrorLabel->text.data += (long)(wave->GetWaveDataSize()/1024);
		waveMirrorLabel->text.data += "kb";
	}else{
		waveMirrorLabel->text.data = options->GetString(SndOptions::s_waves_preview_exported);
		if(exportedWaveSize.NotEmpty())
		{
			waveMirrorLabel->text.data += exportedWaveSize;
		}
	}
	waveMirrorLabel->text.Init(waveMirrorLabel->text.data.c_str());
	OnUpdateWaveNotesHint(null);
}

void _cdecl FormWaves::OnChangeIgnorePA(GUIControl* sender)
{
	if(!ignorePACheckBox) return;
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	wave->SetDefectIgnoreAmp(ignorePACheckBox->IsChecked());	
	wave->GetDescription(currentWave->text.data, currentWave->Hint);
	currentWave->text.Init(currentWave->text.data.c_str());	
}

void _cdecl FormWaves::OnChangeIgnoreMA(GUIControl* sender)
{
	if(!ignoreMACheckBox) return;
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	wave->SetDefectIgnoreMid(ignoreMACheckBox->IsChecked());
	wave->GetDescription(currentWave->text.data, currentWave->Hint);
	currentWave->text.Init(currentWave->text.data.c_str());	
}

void _cdecl FormWaves::OnChangeIgnoreSln(GUIControl* sender)
{
	if(!ignoreSlnCheckBox) return;
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	wave->SetDefectIgnoreSln(ignoreSlnCheckBox->IsChecked());
	wave->GetDescription(currentWave->text.data, currentWave->Hint);
	currentWave->text.Init(currentWave->text.data.c_str());	
}

void _cdecl FormWaves::OnChangeIgnoreClk(GUIControl* sender)
{
	if(!ignoreClkCheckBox) return;
	ProjectWave * wave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!wave) return;
	wave->SetDefectIgnoreClk(ignoreClkCheckBox->IsChecked());
	wave->GetDescription(currentWave->text.data, currentWave->Hint);
	currentWave->text.Init(currentWave->text.data.c_str());	
}

void _cdecl FormWaves::OnStop(GUIControl* sender)
{
	options->WavePreviewStop();
}

//===============================================
//Developer mode extentions
//===============================================


void _cdecl FormWaves::OnWavesSelectorDecomposeWave(GUIControl* sender)
{
	if(waveDecomposer) delete waveDecomposer;
	waveDecomposer = null;
	if(!listOfWaves) return;
	ProjectWave * projectWave = project->WaveGet(listOfWaves->GetFocusWave());
	if(!projectWave) return;
	projectWave->LoadWaveData(ProjectWaveDebugId_DecomposeTest);
	dword waveSize = 0;
	const short * waveData = (const short *)projectWave->GetWaveData(waveSize);
	bool isStereo = (projectWave->GetFormat().nChannels > 1);
	dword samplesCount = projectWave->GetSamplesCount();
	Assert(samplesCount*(isStereo ? 2 : 1)*sizeof(short) <= waveSize);
	waveDecomposer = NEW WaveDecomposer(waveData, samplesCount, isStereo, projectWave->GetName().str);
	projectWave->UnloadWaveData(ProjectWaveDebugId_DecomposeTest);
}

void _cdecl FormWaves::OnWavesSelectorHideDecompose(GUIControl* sender)
{
	if(waveDecomposer) delete waveDecomposer;
	waveDecomposer = null;
}
