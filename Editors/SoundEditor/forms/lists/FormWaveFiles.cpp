//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaveFiles
//============================================================================================

#include "FormWaveFiles.h"
#include "..\FormImportWaves.h"

FormLWFElement::FormLWFElement(FormListBase & _list, bool isFld, bool isDots, const char * path) : FormListElement(_list)
{	
	fullPath = path;
	nameData.GetFileName(fullPath);
	name.Init(nameData.c_str());
	isFolder = isFld;
	isUp = isDots;
}

FormLWFElement::~FormLWFElement()
{
}

//Можно ли войти в эту папку
void FormLWFElement::CheckEnterTo()
{
	Assert(isFolder);
	Assert(!isUp)
}

//Получить полный путь
const char * FormLWFElement::GetFullPath()
{
	return fullPath.c_str();
}

//Получить имя
const char * FormLWFElement::GetName()
{
	return nameData.c_str();
}

//Файл ли
bool FormLWFElement::IsFile()
{
	return !isFolder;
}

//Нарисовать линию
void FormLWFElement::Draw(const GUIRectangle & rect)
{
	GUIImage * image = null;
	if(!isFolder)
	{
		image = &options->imageWave;
	}else{
		if(!isUp)
		{
			image = &options->imageCloseFolder;
		}else{
			image = &options->imageUp;
		}
	}
	//Фон выделения	и фокус
	DrawCursor(rect);
	DrawSelect(rect);
	//Параметры волны
/*	if(wave)
	{
		dword infoColor = options->black2Bkg[4];
		//Рисуем волну
		if(rect.w >= c_buttonsarea + c_wavearea + c_timearea + c_namearea)
		{
			long x = rect.x + rect.w - c_buttonsarea - c_timearea - c_wavearea;
			long y = rect.y + (rect.h - 16)/2;
			GUIHelper::DrawLines(wave->GetHotPreview(), wave->GetHotPreviewCount(), infoColor, x, y);
			GUIHelper::DrawWireRect(x, y, 64, 16, options->black2Bkg[7]);
		}
		//Пишем время
		if(rect.w >= c_buttonsarea + c_timearea + c_namearea)
		{
			const ExtName & name = wave->GetPlayTimeName();
			long x = rect.x + rect.w - c_buttonsarea - c_timearea;
			long y = rect.y + (rect.h - name.h)/2;
			options->render->Print(float(x), float(y), infoColor, name.str);
		}
	}*/
	//Иконка объекта
	DrawIcon(rect, image);
	//Текст
	DrawText(rect, &name);
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormLWFElement::IsLessThen(const FormListElement * el) const
{
	FormLWFElement & el2 = *(FormLWFElement *)el;
	//Папки
	if(isFolder)
	{
		if(!el2.isFolder)
		{
			return true;
		}
		if(isUp)
		{
			return true;
		}
		if(el2.isUp)
		{
			return false;
		}
		if(string::Compare(nameData.c_str(), el2.nameData.c_str()) > 0)
		{
			return false;
		}
		return true;
	}
	if(el2.isFolder)
	{
		return false;
	}
	//Файлы
	if(string::Compare(nameData.c_str(), el2.nameData.c_str()) > 0)
	{
		return false;
	}
	return true;		
}

//Проверить на совпадение итема
bool FormLWFElement::IsThis(const char * str, dword hash, dword len) const
{
	if(hash == name.hash && len == name.len)
	{
		if(string::IsEqual(str, name.str))
		{
			return true;
		}
	}
	return false;
}

//Проверить попадание по префиксу
bool FormLWFElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormLWFElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	//Проверим на попадание по кнопкам

	return true;
}

//Эвенты
void FormLWFElement::Event(const FormListEventData & data)
{
}

//Активировать элемент
void FormLWFElement::OnAction()
{
	if(isFolder)
	{
		//Переход по иерархии
		if(!isUp)
		{
			Enter();
		}else{
			UpByHerarchy();
		}		
	}else{
		//Проиграть/остановить звук

	}
}


FormWaveFiles::FormWaveFiles(FormWaveFilesOptions & opt, GUIControl * parent, const GUIRectangle & rect)
: FormListBase(parent, rect), 
formOptions(opt)
{
	buttonImport = ButtonAdd();
	buttonImport->image = &options->imageImportWave;
	buttonImport->onUp.SetHandler(this, (CONTROL_EVENT)&FormWaveFiles::OnImport);
	buttonImport->Hint = options->GetString(SndOptions::s_hint_waves_import);
	InitList();
}

FormWaveFiles::~FormWaveFiles()
{
	FormLWFElement * element = (FormLWFElement *)GetLine(GetFocus());
	if(element)
	{
		formOptions.setCursorOnItem = element->GetName();
	}else{
		formOptions.setCursorOnItem.Empty();
	}

}

//Инициализировать лист
void FormWaveFiles::DoInitList()
{
	const char * fileExt = "wav";
	//Нормализуем путь
	formOptions.currentFolder += '\\';
	formOptions.currentFolder.CheckPath();
	//Перебераем папки и файлы на данном уровне
	bool haveDots = false;
	dword flags = find_no_mirror_files | find_no_recursive | find_folders | find_dots | find_no_files_from_packs;
	IFinder * finder = options->fileService->CreateFinder(formOptions.currentFolder.c_str(), "*.*", flags, _FL_);
	if(finder)
	{
		for(dword i = 0; i < finder->Count(); i++)
		{
			if(finder->IsFolder(i))
			{
				if(!finder->IsDot(i))
				{
					AddElement(NEW FormLWFElement(*this, true, false, finder->FilePath(i)));
				}else{
					if(!haveDots)
					{
						AddElement(NEW FormLWFElement(*this, true, true, options->GetString(SndOptions::s_folder_up)));
						haveDots = true;
					}
				}
			}else{
				if(string::IsEqual(finder->Extension(i), fileExt))
				{
					AddElement(NEW FormLWFElement(*this, false, false, finder->FilePath(i)));
				}
			}
		}
		finder->Release();
	}
	//Формируем заголовок
	GUIImage * imagePath = &options->imageComputer;
	if(haveDots)
	{
		imagePath = &options->imageOpenFolder;
	}
	string & pathBuffer = options->formWaveFilesDoInitListTmp;
	pathBuffer = formOptions.currentFolder;
	if(pathBuffer.Len() > 0)
	{
		if(pathBuffer.Last() != '\\')
		{
			pathBuffer += '\\';
		}
	}
	pathBuffer += "*.";
	pathBuffer += fileExt;
	SetCaption(pathBuffer.c_str(), imagePath);
}

//Подняться по иерархии
void FormWaveFiles::DoUpByHerarchy()
{
	//Получим имя текущей папки
	string & pathBuffer = options->formWaveFilesDoUpByHerarchyTmp;
	pathBuffer = formOptions.currentFolder;
	pathBuffer.CheckPath();
	if(pathBuffer.Len() > 0 && pathBuffer.Last() == '\\')
	{		
		pathBuffer.Delete(pathBuffer.Len() - 1, 1);
	}
	formOptions.setCursorOnItem.GetFileName(pathBuffer);
	//Поднимаемся выше по файловой иерархии
	pathBuffer = formOptions.currentFolder;
	pathBuffer += "\\..\\";
	dword flags = find_no_mirror_files | find_no_recursive | find_folders | find_dots | find_no_files_from_packs;	
	IFinder * finder = options->fileService->CreateFinder(pathBuffer.c_str(), "*.*", flags, _FL_);
	if(finder && finder->Count() > 0)
	{
		for(dword i = 0; i < finder->Count(); i++)
		{
			if(string::IsEqual(finder->Name(i), "."))
			{
				formOptions.currentFolder = finder->Path(i);
				break;
			}
		}
		if(i >= finder->Count())
		{
			formOptions.currentFolder = finder->Path(0);
			formOptions.currentFolder.CheckPath();
			long index = formOptions.currentFolder.LastChar('\\');
			formOptions.currentFolder.Delete(index, formOptions.currentFolder.Len() - index);
		}
		while(formOptions.currentFolder.Last() == '.') formOptions.currentFolder.Delete(formOptions.currentFolder.Len() - 1, 1);
		finder->Release();
	}
	DoInitList();	
}

//Опуститься по иерархии ниже
void FormWaveFiles::DoEnterTo(FormListElement * element)
{	
	FormLWFElement * wfElement = (FormLWFElement *)element;
	wfElement->CheckEnterTo();
	formOptions.currentFolder = wfElement->GetFullPath();
	DoInitList();
}

//Лист был обновлён
void FormWaveFiles::OnListUpdated()
{
	if(formOptions.setCursorOnItem.Len() > 0)
	{
		SetFocus(formOptions.setCursorOnItem.c_str());
		formOptions.setCursorOnItem.Empty();
	}
}

//Получить нажатую кнопку
void FormWaveFiles::OnKeyPressed(dword code, dword isSys)
{
	if(code == VK_F5 && isSys)
	{
		ImportFiles();
	}
}

//Импортировать выделенные файлы в проект
void FormWaveFiles::ImportFiles()
{
	//Получаем список выделенных элементов
	array<FormListElement *> selected(_FL_, 256);
	FillSelectList(selected);
	if(selected.Size() == 0)
	{
		return;
	}
	FormImportWaves * import = NEW FormImportWaves(GetParent());
	options->kbFocus = null;
	options->gui_manager->ShowModal(import);
	//Импортируем файлы в проект
	import->files.Reserve(selected.Size());
	for(long i = 0; i < selected; i++)
	{
		FormLWFElement * element = (FormLWFElement *)selected[i];
		if(element->IsFile())
		{
			import->files[import->files.Add()] = element->GetFullPath();
		}		
	}
}

void _cdecl FormWaveFiles::OnImport(GUIControl* sender)
{
	ImportFiles();
}

//Пришёл клавиатурный фокус
void FormWaveFiles::OnKbFocus()
{
	options->formOptions.formWaves.isFocusLeftPanel = false;
}
