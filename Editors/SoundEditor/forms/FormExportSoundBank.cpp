//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormExportSoundBank
//============================================================================================


#include "FormExportSoundBank.h"
#include "FormButton.h"
#include "..\SndExporter.h"
#include "lists\FormListBase.h"



class FormLogListElement : public FormListElement
{
public:
	FormLogListElement(FormListBase & _list);
	virtual ~FormLogListElement();
	//Нарисовать линию
	virtual void Draw(const GUIRectangle & rect);
	//Если параметры линия должна быть выше чем данная то вернуть true
	virtual bool IsLessThen(const FormListElement * el) const;
	//Проверить на совпадение итема
	virtual bool IsThis(const char * str, dword hash, dword len) const;
	//Проверить попадание по префиксу
	virtual bool IsPreficsEqual(const char * pref) const;
	//Если на данном щелчке можно проводить операции селекта, вернуть true
	virtual bool MouseClick(const GUIRectangle & rect, const GUIPoint & pos);

public:
	ExtNameStr error;
	GUIImage * image;
};

class FormLog : public FormListBase
{
public:
	FormLog(GUIControl * parent, GUIRectangle rect);
	virtual ~FormLog();

	void ResetErrorsPosition();
	void AddErrors(array<SndExporter::Error> & err);
	void AddMessage(GUIImage * image, const char * text);
	ExtNameStr * GetOutput();
	void Done();

private:
	virtual void DoInitList();
	virtual void Draw();

public:
	bool isError;
private:
	array<FormLogListElement *> errors;
	FormLogListElement * dynamic;
	dword errorsPosition;
	string pathBuffer;
};


FormLogListElement::FormLogListElement(FormListBase & _list) : FormListElement(_list)
{
	image = null;
}

FormLogListElement::~FormLogListElement()
{
}

//Нарисовать линию
void FormLogListElement::Draw(const GUIRectangle & rect)
{
	DrawCursor(rect);
	if(image)
	{
		DrawIcon(rect, image);
	}
	DrawText(rect, &error, true);
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormLogListElement::IsLessThen(const FormListElement * el) const
{
	return true;
}

//Проверить на совпадение итема
bool FormLogListElement::IsThis(const char * str, dword hash, dword len) const
{
	if(hash == error.hash || len == error.len)
	{
		return string::IsEqual(str, error.str);
	}
	return false;
}

//Проверить попадание по префиксу
bool FormLogListElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(error.str, pref);
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormLogListElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	SetFocus();
	return false;
}

FormLog::FormLog(GUIControl * parent, GUIRectangle rect) : FormListBase(parent, rect, true), errors(_FL_)
{
	isError = false;
	ResetErrorsPosition();
	SetFrame(true);
	dynamic = NEW FormLogListElement(*this);
	dynamic->image = &options->imageLineArrow;
	SetCaption(options->GetString(SndOptions::s_export_caption), null);
}

FormLog::~FormLog()
{
	for(dword i = 0; i < errors.Size(); i++)
	{
		delete errors[i];
	}
	if(dynamic)
	{
		delete dynamic;
	}
}

void FormLog::DoInitList()
{
	for(dword i = 0; i < errors.Size(); i++)
	{
		AddElement(errors[i]);
	}
	if(dynamic)
	{
		AddElement(dynamic);
	}
}

void FormLog::Draw()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);	
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg2White[3]);
	FormListBase::Draw();
}

void FormLog::ResetErrorsPosition()
{
	errorsPosition = 0;
}

void FormLog::AddErrors(array<SndExporter::Error> & err)
{
	for(; errorsPosition < err.Size(); errorsPosition++)
	{
		isError = true;
		//Кодовое описание ошибки
		SndExporter::Error & errorDesc = err[errorsPosition];
		//Элемент описания кода ошибки
		FormLogListElement * element = NEW FormLogListElement(*this);
		element->image = &options->imageExportError;
		enum MessageType
		{
			mt_unknown,
			mt_wave,
			mt_sound,
		};
		MessageType mtype = mt_unknown;
		switch(errorDesc.code)
		{
		case SndExporter::ec_nowaves:
			element->error.SetString(options->GetString(SndOptions::s_export_err_nowaves));
			mtype = mt_sound;
			break;
		case SndExporter::ec_nobaseparams:
			element->error.SetString(options->GetString(SndOptions::s_export_err_nobaseparams));
			mtype = mt_sound;
			break;
		case SndExporter::ec_noattenuation:
			element->error.SetString(options->GetString(SndOptions::s_export_err_noattenuation));
			mtype = mt_sound;
			break;
		case SndExporter::ec_filenotopen:
			element->error.SetString(options->GetString(SndOptions::s_export_err_filenotopen));
			mtype = mt_sound;
			break;
		case SndExporter::ec_nomemory:
			element->error.SetString(options->GetString(SndOptions::s_export_err_nomemory));
			mtype = mt_sound;
			break;
		case SndExporter::ec_cantwrite:
			element->error.SetString(options->GetString(SndOptions::s_export_err_cantwrite));
			mtype = mt_sound;
			break;
		case SndExporter::ec_errorwaveexport:
			element->error.SetString(options->GetString(SndOptions::s_export_err_errorwaveexport));
			mtype = mt_wave;
			break;
		case SndExporter::ec_wavenotfound:
			element->error.SetString(options->GetString(SndOptions::s_export_err_wavenotfound));
			mtype = mt_sound;
			break;
		case SndExporter::ec_reimportwave:
			element->error.SetString(options->GetString(SndOptions::s_export_err_reimportwave));
			mtype = mt_wave;
			break;
		default:
			element->error.SetString(options->GetString(SndOptions::s_export_err_unknown));
		}
		errors.Add(element);
		//Элемент указывающий место ошибки
		if(errorDesc.id.IsValidate())
		{			
			if(mtype == mt_sound)
			{			
				FormLogListElement * info = NEW FormLogListElement(*this);
				ProjectSound * sound = project->SoundGet(errorDesc.id);
				const UniqId & folder = sound->GetFolder();
				info->error.data = "    ";
				info->error.data += project->FolderGetPath(folder, &project->FolderSounds());
				info->error.data += "\\";
				info->error.data += sound->GetName().str;
				info->error.Init(info->error.data.c_str());
				errors.Add(info);
			}else
			if(mtype == mt_wave)
			{
				FormLogListElement * info = NEW FormLogListElement(*this);
				ProjectWave * wave = project->WaveGet(errorDesc.id);
				const UniqId & folder = wave->GetFolder();
				info->error.data = "    ";
				info->error.data += project->FolderGetPath(folder, &project->FolderWaves());
				info->error.data += "\\";
				info->error.data += wave->GetName().str;
				info->error.Init(info->error.data.c_str());
				errors.Add(info);				
				ErrorId errorCode = wave->GetErrorCode();
				if(errorCode.IsError())
				{
					info = NEW FormLogListElement(*this);
					options->ErrorGet(errorCode, info->error.data, false);
					info->error.data.Insert(0, "    ");
					info->error.Init(info->error.data.c_str());
					errors.Add(info);
				}
			}
		}
	}
	InitList();
	SetFocus(errors.Size() - 1);
}

void FormLog::AddMessage(GUIImage * image, const char * text)
{
	FormLogListElement * element = NEW FormLogListElement(*this);
	element->image = image;
	element->error.SetString(text);
	errors.Add(element);
	InitList();
	SetFocus(errors.Size() - 1);
}

ExtNameStr * FormLog::GetOutput()
{
	return &dynamic->error;
}

void FormLog::Done()
{
	if(dynamic)
	{
		delete dynamic;
		dynamic = null;
	}
	InitList();
}


FormExportSoundBank::FormExportSoundBank(GUIControl * parent, UniqId * banks, long count) : GUIWindow(parent, 0, 0, 10, 10),
																						exportTable(_FL_)
{
	sndExporter = null;
	bPopupStyle = true;
	bSystemButton = false;
	GUIRectangle rect = GetParent()->GetClientRect();
	rect.x = (rect.w - c_width)/2;
	rect.y = (rect.h - c_height)/2;
	rect.w = c_width;
	rect.h = c_height;
	SetClientRect(rect);
	SetDrawRect(rect);
	GUIWindow::Draw();
	GUIRectangle r = GetClientRect();
	r.x = c_logspace;
	r.y = c_logspace;
	r.w -= c_logspace*2;
	r.h -= c_logspace*2 + c_cancel_button_y;
	exportLog = NEW FormLog(this, r);
	r = GetClientRect();
	r.x = (r.w - c_cancel_button_w)/2;
	r.y = r.h - c_cancel_button_y;
	r.w = c_cancel_button_w;
	r.h = c_cancel_button_h;
	buttonCancel = NEW FormButton(this, r);
	buttonCancel->text.SetString(options->GetString(SndOptions::s_form_cancel));
	buttonCancel->onUp.SetHandlerEx(this, &FormExportSoundBank::StopExport);
	currentExportIndex = 0;
	for(long i = 0; i < count; i++)
	{
		ProjectSoundBank * b = project->SoundBankGet(banks[i]);
		if(b)
		{
			exportTable.Add(b);
		}		
	}
	if(options->leakWavesMode != 0)
	{
		//Удаляем все файлы из интересуемой папки
		IFinder * finder = options->fileService->CreateFinder(options->pathLeakWaves.c_str(), "*.*", find_all_files_no_mirrors | find_no_recursive, _FL_);
		Assert(finder);
		for(dword i = 0; i < finder->Count(); i++)
		{
			options->fileService->Delete(finder->FilePath(i));
		}
		RELEASE(finder);
		string translates;
		translates.Reserve(32768);
		string path;
		path.Reserve(4096);
		array<ProjectWave *> leakWaves(_FL_, 1024);
		leakWaves.Reserve(project->WaveArray().Size() + 1);
		const array<ProjectWave *> & waves = project->WaveArray();
		if(options->leakWavesMode != 1)
		{
			
			leakWaves.Empty();
			leakWaves.AddElements(waves.Size());
			for(dword i = 0; i < waves.Size(); i++)
			{
				leakWaves[i] = waves[i];
			}
		}else{
			for(long n = 0; n < count; n++)
			{				
				ProjectSoundBank * psb = project->SoundBankGet(banks[n]);
				if(!psb) continue;
				const UniqId & sbFolder = psb->GetSBFolderId();
				const array<ProjectSound *> & sounds = project->SoundArray();
				for(dword i = 0; i < sounds.Size(); i++)
				{
					ProjectSound * snd = sounds[i];
					if(!project->FolderIsChild(sbFolder, snd->GetFolder()))
					{
						continue;
					}
					for(long j = 0; j < snd->GetWavesCount(); j++)
					{
						const UniqId & waveId = snd->GetWave(j).waveId;
						ProjectWave * wave = project->WaveGet(waveId);
						if(wave)
						{
							for(dword c = 0; c < leakWaves.Size(); c++)
							{
								if(leakWaves[c] == wave) break;								
							}
							if(c == leakWaves.Size())
							{
								leakWaves.Add(wave);
							}
						}
					}
				}
			}			
		}
		//Проходим по всем волнам проекта, сохраняя их
		translates.Empty();
		path.Empty();
		for(dword i = 0; i < leakWaves.Size(); i++)
		{
			ProjectWave * wave = leakWaves[i];
			if(!wave) continue;
			path = options->pathLeakWaves;
			path += wave->GetName().str;
			path += ".wav";
			wave->SaveWaveToWAV(path.c_str());
			translates += '"';
			translates += wave->GetName().str;
			const char * txtId = wave->GetId().ToString();
			translates += "\" = \"";
			translates += txtId;
			translates += "\"\r\n";
		}
		translates.Empty();
		path = options->pathLeakWaves;
		path += "  Translate Wave Names To Ids  .txt";
		options->fileService->SaveData(path.c_str(), translates.c_str(), translates.Len());
		for(dword i = 0, cnt = 0; i < waves.Size(); i++)
		{
			ProjectWave * wave = waves[i];
			for(dword j = 0; j < leakWaves.Size(); j++)
			{
				if(leakWaves[j] == wave)
				{
					break;
				}
			}
			if(j == leakWaves.Size())
			{
				char buf[16];
				cnt++;
				crt_snprintf(buf, sizeof(buf), "%.4u: ", cnt);				
				translates += buf;
				translates += wave->GetName().str;
				translates += "\r\n";
			}
		}
		if(cnt > 0)
		{
			path = options->pathLeakWaves;
			path += "  Unuse waves in exported banks  .txt";
			options->fileService->SaveData(path.c_str(), translates.c_str(), translates.Len());
		}

	}
}

FormExportSoundBank::~FormExportSoundBank()
{
	if(sndExporter)
	{
		delete sndExporter;
	}
}

//Рисование
void FormExportSoundBank::Draw()
{
	if(currentExportIndex == exportTable.Size())
	{
		Assert(!sndExporter);
		currentExportIndex++;
		buttonCancel->text.SetString(options->GetString(SndOptions::s_form_close));
		exportLog->Done();
	}
	//Рисуем
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[6]);	
	if(currentExportIndex < exportTable.Size())
	{
		//Процесс экспорта
		if(!sndExporter)
		{
			exportLog->ResetErrorsPosition();
			sndExporter = NEW SndExporter();
			sndExporter->progressOutput = exportLog->GetOutput();
			options->formExportSoundBankDrawTmp = options->GetString(SndOptions::s_export_start_bank);
			const ExtName * folderName = project->FolderGetName(exportTable[currentExportIndex]->GetSBFolderId());
			Assert(folderName);
			options->formExportSoundBankDrawTmp += "\"";
			options->formExportSoundBankDrawTmp += folderName->str;
			options->formExportSoundBankDrawTmp += "\"";
			exportLog->AddMessage(&options->imageSoundBank, options->formExportSoundBankDrawTmp.c_str());
			sndExporter->ExportPrepareBank(exportTable[currentExportIndex]->GetId());
		}else{
			if(sndExporter->ExportStep())
			{	
				exportLog->AddErrors(sndExporter->errors);
				sndExporter->progressOutput = null;
				dword xbsize = sndExporter->GetXboxBankSize();
				dword pcsize = sndExporter->GetPCBankSize();
				bool isErrors = (sndExporter->errors.Size() > 0);
				delete sndExporter;
				sndExporter = null;
				options->formExportSoundBankDrawTmp = options->GetString(SndOptions::s_export_done_bank);
				const ExtName * folderName = project->FolderGetName(exportTable[currentExportIndex]->GetSBFolderId());
				Assert(folderName);
				options->formExportSoundBankDrawTmp += "\"";
				options->formExportSoundBankDrawTmp += folderName->str;
				options->formExportSoundBankDrawTmp += "\"";
				if(!isErrors)
				{
					options->formExportSoundBankDrawTmp += options->GetString(SndOptions::s_export_done_bank_end);
					exportLog->AddMessage(&options->imageSoundBank, options->formExportSoundBankDrawTmp.c_str());
					options->formExportSoundBankDrawTmp = "Xbox part size: ";
					options->formExportSoundBankDrawTmp += xbsize/(1024*1024);
					options->formExportSoundBankDrawTmp += " Mb";
					exportLog->AddMessage(null, options->formExportSoundBankDrawTmp.c_str());
					options->formExportSoundBankDrawTmp = "PC part size: ";
					options->formExportSoundBankDrawTmp += pcsize/(1024*1024);
					options->formExportSoundBankDrawTmp += " Mb";
					exportLog->AddMessage(null, options->formExportSoundBankDrawTmp.c_str());
				}else{
					options->formExportSoundBankDrawTmp += options->GetString(SndOptions::s_export_done_bank_errors);
					exportLog->AddMessage(&options->imageExportError, options->formExportSoundBankDrawTmp.c_str());
				}				
				currentExportIndex++;
				delete sndExporter;
				sndExporter = null;
			}else{
				exportLog->AddErrors(sndExporter->errors);
			}
		}				
	}else{
		Sleep(5);
	}
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
	GUIControl::Draw();
}


void _cdecl FormExportSoundBank::StopExport(GUIControl * sender)
{
	if(sender == buttonCancel)
	{			
		options->gui_manager->Close(this);
		for(GUIControl * ctrl = GetParent(); ctrl; ctrl = ctrl->GetParent())
		{
			ctrl->SendMessage(GUIMSG_FORCE_DWORD, sndmsg_stopmodal, 0);
		}
		options->kbFocus = null;
	}
}





