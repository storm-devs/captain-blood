//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaves
//============================================================================================

#include "FormImportWaves.h"
#include "FormButton.h"
#include "FormEditName.h"


FormImportWaves::FormImportWaves(GUIControl * parent) : GUIWindow(parent, 0, 0, 10, 10), files(_FL_, 256)
{		
	bPopupStyle = true;
	bSystemButton = false;
	GUIRectangle rect = GetParent()->GetClientRect();
	rect.x = (rect.w - c_width)/2;
	rect.y = (rect.h - c_height)/2;
	rect.w = c_width;
	rect.h = c_height;
	SetClientRect(rect);
	SetDrawRect(rect);
	importIndex = 0;
	stage = stage_select_file;
	isReplaceMode = false;
	GUIWindow::Draw();
	//Создаём кнопки
	CreateButtons(c_bt_importBeg, c_bt_importEnd);
	CreateButtons(c_bt_waitBeg, c_bt_waitEnd);
	CreateButtons(c_bt_errorBeg, c_bt_errorEnd);
	//Настраиваем индивидуальные параметры
	buttons[c_bt_importStop]->text.SetString(options->GetString(SndOptions::s_form_stop));
	buttons[c_bt_importStop]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnImportStop);
	buttons[c_bt_waitSkip]->text.SetString(options->GetString(SndOptions::s_form_skip));
	buttons[c_bt_waitSkip]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnWaitSkip);
	buttons[c_bt_waitReplace]->text.SetString(options->GetString(SndOptions::s_form_replace));
	buttons[c_bt_waitReplace]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnWaitReplace);
	buttons[c_bt_waitRename]->text.SetString(options->GetString(SndOptions::s_form_rename));
	buttons[c_bt_waitRename]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnWaitRename);
	buttons[c_bt_waitStop]->text.SetString(options->GetString(SndOptions::s_form_stop));
	buttons[c_bt_waitStop]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnWaitStop);
	buttons[c_bt_errorContinue]->text.SetString(options->GetString(SndOptions::s_form_continue));
	buttons[c_bt_errorContinue]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnErrorContinue);
	buttons[c_bt_errorStop]->text.SetString(options->GetString(SndOptions::s_form_stop));
	buttons[c_bt_errorStop]->onUp.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnErrorStop);

	ShowButtons(c_bt_importBeg, c_bt_importEnd);
	GUIWindow::Draw();
}

FormImportWaves::~FormImportWaves()
{

}

//Разместить набор кнопок
void FormImportWaves::CreateButtons(long from, long to)
{
	Assert(from <= to);
	Assert(from >= 0);
	Assert(to < c_bt_count);
	long count = to - from + 1;
	long width = c_button_space*(count - 1) + c_button_width*count;
	GUIRectangle rect;
	rect.x = (c_width - width)/2;
	rect.y = c_height - c_button_height - 5;
	rect.w = c_button_width;
	rect.h = c_button_height;
	for(long i = from; i <= to; i++)
	{
		buttons[i] = NEW FormButton(this, rect);
		buttons[i]->Visible = false;
		rect.x += c_button_space + c_button_width;
	}	
}

//Показать группу кнопок
void FormImportWaves::ShowButtons(long from, long to)
{
	Assert(from <= to);
	Assert(from >= 0);
	Assert(to < c_bt_count);
	for(long i = 0; i < c_bt_count; i++)
	{
		buttons[i]->Visible = (from <= i && i <= to);
		buttons[i]->Enabled = buttons[i]->Visible;
	}
}

//Рисование
void FormImportWaves::Draw()
{
	if(stage == stage_import_file)
	{
		//Указываем что с этим файлом разбираемся
		importIndex++;
		//При режиме замены надо найти волну которую заменяем
		ErrorId importResult;
		if(!isReplaceMode)
		{
			importResult = project->WaveImport(currentFolder, currentPath.c_str(), currentName.c_str(), UniqId::zeroId);
		}else{
			isReplaceMode = false;
			dword strLen = 0;
			dword strHash = string::HashNoCase(currentName.c_str(), strLen);			
			const array<ProjectWave *> &  waves = project->WaveArray();
			for(dword i = 0; i < waves.Size(); i++)
			{
				const ExtName & wn = waves[i]->GetName();
				if(wn.hash == strHash && wn.len == strLen)
				{
					if(string::IsEqual(wn.str, currentName.c_str()))
					{
						UniqId replaceId = waves[i]->GetId();
						importResult = project->WaveImport(currentFolder, currentPath.c_str(), currentName.c_str(), replaceId);
						break;
					}
				}
			}
			Assert(i < waves.Size());
		}
		//Загрузка файла		
		if(importResult.IsOk())
		{
			stage = stage_select_file;	
		}else{
			options->ErrorGet(importResult, errorCode, true);
			dword step = c_max_string_len;
			for(dword i = step; i < errorCode.Len(); i += step)
			{
				errorCode.Insert(i, "\n");
			}
			stage = stage_error_file;
			ShowButtons(c_bt_errorBeg, c_bt_errorEnd);
		}		
	}
	if(stage == stage_select_file)
	{
		//Выбор файла из списка для загрузки на следующем кадре
		if(importIndex >= files)
		{
			currentPath.Empty();
			currentName.Empty();
			stage = stage_no_files;
			for(GUIControl * ctrl = GetParent(); ctrl; ctrl = ctrl->GetParent())
			{
				ctrl->SendMessage(GUIMSG_FORCE_DWORD, sndmsg_stopmodal, 0);
			}			
			options->gui_manager->Close(this);
			return;
		}
		currentPath = files[importIndex];
		currentName.GetFileTitle(currentPath);
		if(currentName.Len() > ProjectWave::c_namelen)
		{
			dword delCount = currentName.Len() - ProjectWave::c_namelen;
			dword delPos = 4;
			Assert(delPos + delCount < currentName.Len());
			currentName.Delete(4, delCount);
		}
		currentFolder = options->formOptions.formWaves.leftPanel.currentFolder;
		currentDrawPath = currentPath;
		dword maxPathLen = 74;
		if(currentDrawPath.Len() > maxPathLen)
		{			
			const char * filler = options->GetString(SndOptions::s_filler);
			long delCount = currentDrawPath.Len() - maxPathLen + string::Len(filler);
			currentDrawPath.Delete(maxPathLen/3, delCount);
			currentDrawPath.Insert(maxPathLen/3, filler);
		}
		//Проверить на допустимость имени
		if(project->WaveCheckUniqeName(currentFolder, currentName.c_str()))
		{
			stage = stage_import_file;
			ShowButtons(c_bt_importBeg, c_bt_importEnd);
		}else{
			stage = stage_wait_file;
			ShowButtons(c_bt_waitBeg, c_bt_waitEnd);
			if(!project->FolderCheckUniqeName(currentFolder, currentName.c_str()))
			{
				//Папку нельзя перезаписать
				buttons[c_bt_waitReplace]->Enabled = false;
			}
		}
	}
	//Рисуем
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, options->bkg2White[1]);	
	long x = rect.x + c_textleft;
	long y = rect.y + c_texttop;
	ExtName headerText;
	if(stage != stage_wait_file)
	{
		if(stage != stage_error_file)
		{
			headerText.Init(options->GetString(SndOptions::s_form_import));
		}else{
			headerText.Init(options->GetString(SndOptions::s_form_import_error));
		}		
	}else{
		options->formImportWavesDrawTmp = options->GetString(SndOptions::s_form_replace_wave1);
		options->formImportWavesDrawTmp += currentName;
		options->formImportWavesDrawTmp += options->GetString(SndOptions::s_form_replace_wave2);
		headerText.Init(options->formImportWavesDrawTmp.c_str());
	}	
	options->render->Print(float(x), float(y), options->colorTextLo, headerText.str);
	y += 4 + headerText.h + headerText.h/4;	
	if(stage != stage_error_file)
	{
		options->render->Print(float(x), float(y), options->colorTextLo, currentDrawPath.c_str());	
	}else{
		options->render->Print(float(x), float(y), options->colorTextLo, errorCode.c_str());
	}	
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
	GUIControl::Draw();
}

void _cdecl FormImportWaves::OnImportStop(GUIControl* sender)
{
	stage = stage_select_file;
	importIndex = files.Size();
	currentPath.Empty();
	currentName.Empty();
}

void _cdecl FormImportWaves::OnWaitStop(GUIControl* sender)
{
	OnImportStop(sender);
}

void _cdecl FormImportWaves::OnWaitReplace(GUIControl* sender)
{
	stage = stage_import_file;
	isReplaceMode = true;
}

void _cdecl FormImportWaves::OnWaitRename(GUIControl* sender)
{
	GUIRectangle r = buttons[c_bt_waitRename]->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_right_bottom);
	fen->Caption = options->GetString(SndOptions::s_snd_enter_new_sound);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnWaitRename_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormImportWaves::OnWaitRename_Ok);
	fen->SetText(currentName.c_str());
	fen->MarkTextAsIncorrect();
	options->gui_manager->ShowModal(fen); 	
	
}

void _cdecl FormImportWaves::OnWaitRename_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * newName = fen->GetText();
	if(!project->WaveCheckUniqeName(currentFolder, newName))
	{
		fen->MarkTextAsIncorrect();
	}	
}

void _cdecl FormImportWaves::OnWaitRename_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	currentName = fen->GetText();
	stage = stage_import_file;
}

void _cdecl FormImportWaves::OnWaitSkip(GUIControl* sender)
{
	stage = stage_select_file;
	importIndex++;	
}

void _cdecl FormImportWaves::OnErrorStop(GUIControl* sender)
{
	OnImportStop(sender);
}

void _cdecl FormImportWaves::OnErrorContinue(GUIControl* sender)
{
	stage = stage_select_file;
}




