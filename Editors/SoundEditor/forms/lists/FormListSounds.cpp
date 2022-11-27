//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListSounds
//============================================================================================

#include "FormListSounds.h"

FormLSndElement::FormLSndElement(FormListBase & _list, const UniqId & s, const UniqId & f) : FormListElement(_list)
{
	sound = s;
	folder = f;
	if(folder.IsValidate())
	{
		if(folder != project->FolderUp())
		{
			UniqId parentId;
			Verify(project->FolderGetParent(folder, parentId));
			if(parentId == project->FolderSounds())
			{
				image = &options->imageSoundBank;
			}else{
				image = &options->imageCloseFolder;
			}
			const ExtName * nm = project->FolderGetName(folder);
			crt_strncpy(nameData, sizeof(nameData), nm ? nm->str : "<folder not found>", sizeof(nameData));
		}else{
			image = &options->imageUp;
			crt_strncpy(nameData, sizeof(nameData), options->GetString(SndOptions::s_folder_up), sizeof(nameData));
		}
	}else{
		ProjectSound * ps = project->SoundGet(s);
		if(ps)
		{
			image = &options->imageSound;
			crt_strncpy(nameData, sizeof(nameData), ps->GetName().str, sizeof(nameData));
		}else{
			image = &options->imageExportError;
			crt_strncpy(nameData, sizeof(nameData), "<sound not found>", sizeof(nameData));
		}
	}
	name.Init(nameData);
}

FormLSndElement::~FormLSndElement()
{

}

//Получить папку
const UniqId & FormLSndElement::GetFolder()
{
	return folder;
}

//Получить звук
const UniqId & FormLSndElement::GetSound()
{
	return sound;
}

//Нарисовать линию
void FormLSndElement::Draw(const GUIRectangle & rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	DrawSelect(rect);
	//Иконка объекта
	DrawIcon(rect, image);
	//Текст
	DrawText(rect, &name);
	//GUIHelper::DrawWireRect(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
}

//Если линия должна быть выше чем в параметре то вернуть true
bool FormLSndElement::IsLessThen(const FormListElement * el) const
{
	FormLSndElement & el2 = *(FormLSndElement *)el;
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
		const char * name1 = nameData;
		const char * name2 = el2.nameData;
		if(string::Compare(name1, name2) > 0)
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
	const char * name1 = nameData;
	const char * name2 = el2.nameData;
	if(string::Compare(name1, name2) > 0)
	{
		return false;
	}
	return true;		
}

//Проверить на совпадение итема
bool FormLSndElement::IsThis(const char * str, dword hash, dword len) const
{
	if(name.hash == hash && name.len == len)
	{
		if(string::IsEqual(str, name.str))
		{
			return true;
		}
	}
	return false;
}

//Проверить попадание по префиксу
bool FormLSndElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormLSndElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	//Проверим на попадание по кнопкам

	return true;
}

//Эвенты
void FormLSndElement::Event(const FormListEventData & data)
{
}

//Активировать элемент
void FormLSndElement::OnAction()
{
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
		Assert(sound.IsValidate());
		//Проиграть/остановить звук

	}
}



FormListSounds::FormListSounds(FormListSoundsOptions & opt, GUIControl * parent, const GUIRectangle & rect) 
: FormListBase(parent, rect),
formOptions(opt)
{
	InitList();
}

FormListSounds::~FormListSounds()
{
	
}

//Установить новую родительскую папку
void FormListSounds::SetParentFolder(const UniqId & parentFolderId)
{
	formOptions.currentFolder = parentFolderId;
	formOptions.setCursorOnItem.Empty();
	InitList();
}


//Получить текущую родительскую папку
const UniqId & FormListSounds::GetParentFolder()
{
	return formOptions.currentFolder;
}

//Можно ли что-то сделать с выделенным элементом
bool FormListSounds::IsCanProcess()
{
	FormLSndElement * zeroLine = (FormLSndElement *)GetLine(0);
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

//Получить индекс текущего выделенного фокусом звука
const UniqId & FormListSounds::GetFocusSound()
{
	FormLSndElement * le = (FormLSndElement *)GetLine(GetFocus());
	if(!le) return UniqId::zeroId;
	return le->GetSound();
}

//Получить индекс текущей выделенной фокусом папки
const UniqId & FormListSounds::GetFocusFolder()
{
	FormLSndElement * le = (FormLSndElement *)GetLine(GetFocus());
	if(!le) return UniqId::zeroId;
	return le->GetFolder();
}

//Получить список выделеных элементов
void FormListSounds::GetSelectItems(array<UniqId> & selectItems)
{
	selectItems.Empty();
	array<FormListElement *> selected(_FL_, 128);
	FillSelectList(selected);
	selectItems.Reserve(selected.Size());
	for(dword i = 0; i < selected.Size(); i++)
	{
		FormLSndElement * line = (FormLSndElement *)selected[i];
		if(line->GetSound().IsValidate())
		{
			selectItems.Add(line->GetSound());
		}else
		if(line->GetFolder().IsValidate())
		{
			selectItems.Add(line->GetFolder());
		}
	}
}

//Инициализировать лист
void FormListSounds::DoInitList()
{
	if(!project->FolderIsChild(project->FolderSounds(), formOptions.currentFolder))
	{
		formOptions.currentFolder = project->FolderSounds();
	}
	//Добавляем папки на текущем уровне
	if(formOptions.currentFolder != project->FolderSounds())
	{
		AddElement(NEW FormLSndElement(*this, UniqId::zeroId, project->FolderUp()));
		SetCaption(project->FolderGetPath(formOptions.currentFolder, &project->FolderSounds()), &options->imageSoundBank);
	}else{
		SetCaption(options->GetString(SndOptions::s_snd_banks), &options->imageSoundBanks);
	}	
	const array<UniqId> & chld = project->FolderGetChild(formOptions.currentFolder);
	for(long i = 0; i < chld; i++)
	{
		AddElement(NEW FormLSndElement(*this, UniqId::zeroId, chld[i]));
	}
	//Добавляем волны с текущей папкой
	const array<UniqId> & objects = project->FolderObjects(formOptions.currentFolder);
	for(dword i = 0; i < objects.Size(); i++)
	{
		ProjectSound * snd = project->SoundGet(objects[i]);
		if(snd)
		{
			AddElement(NEW FormLSndElement(*this, snd->GetId(), UniqId::zeroId));
		}
	}	
}

//Подняться по иерархии
void FormListSounds::DoUpByHerarchy()
{
	//Сохраним имя родительской папки
	const ExtName * curName = project->FolderGetName(formOptions.currentFolder);
	formOptions.setCursorOnItem = curName ? curName->str : "";
	//Пытаемся подняться выше по иерархии
	UniqId parentId;
	project->FolderGetParent(formOptions.currentFolder, parentId);
	formOptions.currentFolder = parentId;
	if(!project->FolderIsChild(project->FolderSounds(), formOptions.currentFolder))
	{
		formOptions.currentFolder = project->FolderSounds();
	}
	//Строим лист
	DoInitList();
}

//Опуститься по иерархии ниже
void FormListSounds::DoEnterTo(FormListElement * element)
{	
	//Папка в которую надо перейти
	UniqId id = ((FormLSndElement *)element)->GetFolder();
	UniqId parentId;
	Verify(project->FolderGetParent(id, parentId));
	Assert(parentId == formOptions.currentFolder);
	formOptions.currentFolder = id;
	formOptions.setCursorOnItem.Empty();
	//Строим лист
	DoInitList();
}

//Лист был обновлён
void FormListSounds::OnListUpdated()
{
	if(formOptions.setCursorOnItem.Len() > 0)
	{
		SetFocus(formOptions.setCursorOnItem.c_str());
		formOptions.setCursorOnItem.Empty();
	}
}

