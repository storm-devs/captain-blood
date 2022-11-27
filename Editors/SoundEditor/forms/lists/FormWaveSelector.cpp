//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaveSelector
//============================================================================================

#include "FormWaveSelector.h"

FormLWSElement::FormLWSElement(FormListBase & _list, const UniqId & w, const UniqId & f) : FormListElement(_list)
{
	waveId = w;
	folder = f;
	playHighlight = 0.0f;
	stopHighlight = 0.0f;
	wave = null;
	if(folder.IsValidate())
	{
		waveId.Reset();
		if(folder != project->FolderUp())
		{
			image = &options->imageCloseFolder;
			const ExtName * nm = project->FolderGetName(folder);
			crt_strncpy(nameData, sizeof(nameData), nm ? nm->str : "<folder not found>", sizeof(nameData));
		}else{
			image = &options->imageUp;
			crt_strncpy(nameData, sizeof(nameData), options->GetString(SndOptions::s_folder_up), sizeof(nameData));
		}
	}else{
		Assert(waveId.IsValidate())
		wave = project->WaveGet(waveId);
		if(wave)
		{
			image = &options->imageWave;
			crt_strncpy(nameData, sizeof(nameData), wave->GetName().str, sizeof(nameData));
		}else{
			image = &options->imageExportError;
			crt_strncpy(nameData, sizeof(nameData), "<wave not found>", sizeof(nameData));
		}
	}
	waveIsDefect = false;
	if(wave)
	{
		waveIsDefect = !wave->IsNoWaveDefects();
	}
	name.Init(nameData);
}

FormLWSElement::~FormLWSElement()
{

}

bool FormLWSElement::IsThisFolder(const UniqId & f)
{
	if(!folder.IsValidate())
	{
		return false;
	}
	return folder == f;
}

const UniqId & FormLWSElement::GetWave()
{
	return waveId;
}

const UniqId & FormLWSElement::GetFolder()
{
	return folder;
}

//Нарисовать линию
void FormLWSElement::Draw(const GUIRectangle & rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	DrawSelect(rect);
	//Параметры волны
	long controlsAreaW = c_buttonsarea;
	dword infoColor = options->black2Bkg[4];
	dword rectColor = options->black2Bkg[7];
	dword imageColor = 0xa0ffffff;
	dword lineColor = 0;
	if(wave)
	{
		float playPosition = 0.0f;
		if(wave->IsPlay(playPosition))
		{
			infoColor = options->black2Bkg[2];
			rectColor = options->black2Bkg[5];
			lineColor = options->black2Bkg[1];
			imageColor = 0xffffffff;
		}
		//Рисуем волну
		if(rect.w >= c_buttonsarea + c_wavearea + c_timearea + c_namearea)
		{
			controlsAreaW += c_wavearea;
			long x = rect.x + rect.w - c_timearea - c_wavearea;
			long y = rect.y + (rect.h - 16)/2;
			GUIHelper::DrawLines(wave->GetHotPreview(), wave->GetHotPreviewCount(), infoColor, x, y);
			GUIHelper::DrawWireRect(x, y, 64, 16, rectColor);
			if(lineColor)
			{
				long pos = x + long(playPosition*64.0f);
				GUIHelper::Draw2DLine(pos, y, pos, y + 16, options->black2Bkg[0]);	
			}
		}
		//Пишем время
		if(rect.w >= c_buttonsarea + c_timearea + c_namearea)
		{
			controlsAreaW += c_timearea;
			const ExtName & name = wave->GetPlayTimeName();
			long x = rect.x + rect.w - c_timearea;
			long y = rect.y + (rect.h - name.h)/2;
			options->render->Print(float(x), float(y), infoColor, name.str);
		}
	}
	//Иконка объекта
	DrawIcon(rect, image);
	if(wave && IsFocus())
	{
		waveIsDefect = !wave->IsNoWaveDefects();
	}
	if(waveIsDefect)
	{
		DrawIcon(rect, &options->imageExportError);
	}
	//Текст
	DrawText(rect, &name);
	//Кнопки волны
	if(wave)
	{
		long x = rect.x + rect.w - controlsAreaW + c_buttonborder;
		long y = rect.y + (rect.h - c_buttonsize)/2;
		//Играть
		if(playHighlight > 0.0f)
		{
			dword alpha = dword(playHighlight*255.0f) << 24;
			GUIHelper::Draw2DRect(x, y, c_buttonsize, c_buttonsize, alpha | (options->bkg2White[7] & 0x00ffffff));
		}
		GUIHelper::DrawSprite(x, y, c_buttonsize, c_buttonsize, &options->imagePlay, imageColor);
		GUIHelper::DrawWireRect(x, y, c_buttonsize, c_buttonsize, rectColor);
		x += c_buttonsize + c_buttonspace;
		//Стоп
		if(stopHighlight > 0.0f)
		{
			dword alpha = dword(stopHighlight*255.0f) << 24;
			GUIHelper::Draw2DRect(x, y, c_buttonsize, c_buttonsize, alpha | (options->bkg2White[7] & 0x00ffffff));
		}
		GUIHelper::DrawSprite(x, y, c_buttonsize, c_buttonsize, &options->imageStop, imageColor);
		GUIHelper::DrawWireRect(x, y, c_buttonsize, c_buttonsize, rectColor);
	}
	if(playHighlight > 0.0f)
	{
		playHighlight -= api->GetDeltaTime()*3.0f;
	}
	if(stopHighlight)
	{
		stopHighlight -= api->GetDeltaTime()*3.0f;
	}
	//GUIHelper::DrawWireRect(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormLWSElement::IsLessThen(const FormListElement * el) const
{
	FormLWSElement & el2 = *(FormLWSElement *)el;
	//Папки
	if(folder.IsValidate())
	{
		if(!el2.folder.IsValidate())
		{
			return true;
		}
		if(folder == project->FolderUp())
		{
			return true;
		}
		if(el2.folder == project->FolderUp())
		{
			return false;
		}		
		const ExtName * name1 = project->FolderGetName(folder);
		const ExtName * name2 = project->FolderGetName(el2.folder);
		Assert(name1);
		Assert(name2);
		if(string::Compare(name1->str, name2->str) > 0)
		{
			return false;
		}
		return true;
	}
	if(el2.folder.IsValidate())
	{
		return false;
	}
	//Волны
	Assert(wave);
	Assert(el2.wave);
	if(string::Compare(wave->GetName().str, el2.wave->GetName().str) > 0)
	{
		return false;
	}
	return true;		
}

//Проверить на совпадение итема
bool FormLWSElement::IsThis(const char * str, dword hash, dword len) const
{
	if(name.hash == hash && name.len == len)
	{
		if(string::IsEqual(name.str, str))
		{
			return true;
		}
	}
	return false;
}


//Проверить попадание по префиксу
bool FormLWSElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormLWSElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	if(folder.IsValidate())
	{
		return true;
	}
	Assert(wave);
	//Проверим на попадание по кнопкам
	long controlsAreaW = c_buttonsarea;
	if(rect.w >= c_buttonsarea + c_wavearea + c_timearea + c_namearea)
	{
		controlsAreaW += c_wavearea;
	}
	if(rect.w >= c_buttonsarea + c_timearea + c_namearea)
	{
		controlsAreaW += c_timearea;
	}
	//Кнопки волны
	GUIRectangle r;
	r.x = rect.x + rect.w - controlsAreaW + c_buttonborder;
	r.y = rect.y + (rect.h - c_buttonsize)/2;
	r.w = r.h = c_buttonsize;
	if(r.Inside(pos))
	{
		//Запустить проигрывание звука
		playHighlight = 1.0f;
		wave->Play();
		return false;
	}
	r.x += c_buttonsize + c_buttonspace;
	if(r.Inside(pos))
	{
		//Остановить проигрывание звука
		stopHighlight = 1.0f;
		options->WavePreviewStop();
		return false;
	}
	return true;
}

//Эвенты
void FormLWSElement::Event(const FormListEventData & data)
{
}

//Активировать элемент
void FormLWSElement::OnAction()
{
	long controlsAreaW = c_buttonsarea;
	if(folder.IsValidate())
	{
		//Переход по иерархии
		if(folder != project->FolderUp())
		{
			Enter();
		}else{
			UpByHerarchy();
		}		
	}else{
		Assert(wave);
		//Проиграть/остановить звук
		if(!wave->IsPlay())
		{
			playHighlight = 1.0f;
			wave->Play();			
		}else{
			stopHighlight = 1.0f;
			options->WavePreviewStop();
		}
	}
}


FormWaveSelector::FormWaveSelector(FormWaveSelectorOptions & opt, GUIControl * parent, const GUIRectangle & rect) 
: FormListBase(parent, rect),
formOptions(opt)
{
	//Заполняем список
	InitList();
}

FormWaveSelector::~FormWaveSelector()
{
	
}

//Инициализировать лист
void FormWaveSelector::DoInitList()
{
	//Проверяем папку
	if(!project->FolderIsChild(project->FolderWaves(), formOptions.currentFolder))
	{
		formOptions.currentFolder = project->FolderWaves();
	}
	//Добавляем папки на текущем уровне
	GUIImage * imagePath = &options->imageWaveRootFolder;
	if(formOptions.currentFolder != project->FolderWaves())
	{
		AddElement(NEW FormLWSElement(*this, UniqId::zeroId, project->FolderUp()));
		imagePath = &options->imageWaveOpenFolder;
	}
	SetCaption(project->FolderGetPath(formOptions.currentFolder, &project->FolderWaves()), imagePath);
	const array<UniqId> & chld = project->FolderGetChild(formOptions.currentFolder);
	for(long i = 0; i < chld; i++)
	{
		AddElement(NEW FormLWSElement(*this, UniqId::zeroId, chld[i]));
	}
	//Добавляем волны с текущей папкой
	const array<UniqId> & objects = project->FolderObjects(formOptions.currentFolder);
	for(dword i = 0; i < objects.Size(); i++)
	{
		ProjectWave * pw = project->WaveGet(objects[i]);
		if(pw)
		{
			AddElement(NEW FormLWSElement(*this, pw->GetId(), UniqId::zeroId));
		}
	}	
}

//Подняться по иерархии
void FormWaveSelector::DoUpByHerarchy()
{	
	//Получаем имя, на которое установить потом курсор
	formOptions.setCursorOnItem.Empty();
	const ExtName * fn = project->FolderGetName(formOptions.currentFolder);
	if(fn) formOptions.setCursorOnItem = fn->str;
	//Переходим на родительскую папку
	UniqId parentId;
	project->FolderGetParent(formOptions.currentFolder, parentId);
	formOptions.currentFolder = parentId;
	//Проверяем папку
	if(!project->FolderIsChild(project->FolderWaves(), formOptions.currentFolder))
	{
		formOptions.currentFolder = project->FolderWaves();
		formOptions.setCursorOnItem.Empty();
	}
	//Строим заново список
	DoInitList();
}

//Опуститься по иерархии ниже
void FormWaveSelector::DoEnterTo(FormListElement * element)
{	
	//Папка в которую переходим
	const UniqId & fldId = ((FormLWSElement *)element)->GetFolder();
	//Проверяем родительскую связь
	UniqId parentId;
	Verify(project->FolderGetParent(fldId, parentId));
	Assert(parentId == formOptions.currentFolder);
	//Переходим
	formOptions.currentFolder = fldId;
	formOptions.setCursorOnItem.Empty();
	//Строим заново список
	DoInitList();
}

//Лист был обновлён
void FormWaveSelector::OnListUpdated()
{
	if(formOptions.setCursorOnItem.Len() > 0)
	{
		SetFocus(formOptions.setCursorOnItem.c_str());
		formOptions.setCursorOnItem.Empty();
	}
}

//Пришёл клавиатурный фокус
void FormWaveSelector::OnKbFocus()
{
	options->formOptions.formWaves.isFocusLeftPanel = true;
}

//Получить текущую папку
const UniqId & FormWaveSelector::GetCurrentFolder()
{
	return formOptions.currentFolder;
}

//Получить волну в фокусе
const UniqId & FormWaveSelector::GetFocusWave()
{
	FormLWSElement * line = (FormLWSElement *)GetLine(GetFocus());
	if(!line) return UniqId::zeroId;
	return line->GetWave();
}

//Получить папку в фокусе
const UniqId & FormWaveSelector::GetFocusFolder()
{
	FormLWSElement * line = (FormLWSElement *)GetLine(GetFocus());
	if(!line) return UniqId::zeroId;
	return line->GetFolder();
}

//Можно ли что-то сделать с выделенным элементом
bool FormWaveSelector::IsCanProcess()
{
	FormLWSElement * zeroLine = (FormLWSElement *)GetLine(0);
	if(zeroLine)
	{
		if(zeroLine->IsFocus() || zeroLine->IsSelect())
		{
			if(zeroLine->GetFolder() == project->FolderUp())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

//Получить выделенные элементы
void FormWaveSelector::GetSelectElements(array<UniqId> & elements)
{
	elements.Empty();
	array<FormListElement *> selected(_FL_, 128);
	FillSelectList(selected);
	elements.Reserve(selected.Size());
	for(dword i = 0; i < selected.Size(); i++)
	{
		FormLWSElement * line = (FormLWSElement *)selected[i];
		if(line->GetWave().IsValidate())
		{
			elements.Add(line->GetWave());
		}else
		if(line->GetFolder().IsValidate())
		{	
			elements.Add(line->GetFolder());
		}		
	}
}

