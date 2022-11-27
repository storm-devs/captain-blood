

#include "ProjectObject.h"
#include "..\SndOptions.h"


ProjectObject::ProjectObject(const UniqId & folderId, dword maxNameLength, const char * fileExtantion, bool isBinaryFile)
{
	Assert(maxNameLength < sizeof(nameData));
	id.Build();	
	fldId = folderId;
	memset(nameData, 0, sizeof(nameData));
	maxNameLen = maxNameLength;
	updateCounter = 1;
	ext = fileExtantion;
	isBinFile = isBinaryFile;
	Verify(project->FolderObjectAdd(fldId, id));
}

ProjectObject::~ProjectObject()
{
	project->RemoveFromSave(this);
	Verify(project->FolderObjectDel(fldId, id));
}

//Установить объект в очередь на сохранение
void ProjectObject::SetToSave()
{
	project->AddToSave(this);
}

//Создать полный путь файла для сохранения этого объекта
void ProjectObject::BuildFilePath(string & path, const char * otherName)
{
	//Получаем путь папки
	path = project->FolderGetPath(fldId);
	//Добавляем имя
	if(!otherName)
	{
		path += name.str;
	}else{
		path += otherName;
	}
	//Добавим расширение
	path += ext;
}

//Перезаписать идентификатор объекта
void ProjectObject::ReplaceId(const UniqId & newId)
{
	Assert(newId.IsValidate());
	Verify(project->FolderObjectDel(fldId, id));
	id = newId;
	Verify(project->FolderObjectAdd(fldId, id));
}

//Событие удаления объекта из проекта
void ProjectObject::OnDeleteObject()
{
}

//Инициализация объекта
ProjectObject::ErrorCode ProjectObject::Init(const char * newName)
{
	//Ставим имя, проверяя его
	ErrorCode retCode = SetName(newName);
	if(retCode != ec_ok)
	{
		return retCode;
	}
	//Путь к файлу
	BuildFilePath(options->tmp_projectObject_Init);
	//Пробуем создать файл
	if(!isBinFile)
	{
		IEditableIniFile * ini = project->FileOpenIni(options->tmp_projectObject_Init.c_str(), true, false, _FL_);
		if(!ini) return ec_cant_create_file;
		project->FileCloseIni(ini);
	}else{
		IFile * file = project->FileOpenBin(options->tmp_projectObject_Init.c_str(), SndProject::poff_isCreateNewFile, null, _FL_, 101);
		if(!file) return ec_cant_create_file;
		project->FileCloseBin(file, 101);
	}
	//Для объекта создали пустой файл
	return ec_ok;
}

//Сохранить в файл под стандартным именем
ErrorId ProjectObject::SaveProcess()
{
	updateCounter++;
	BuildFilePath(options->tmp_projectObject_SaveProcess);
	return OnSaveObject(options->tmp_projectObject_SaveProcess.c_str());
}

//Первоначальная загрузка при старте редактора
ErrorId ProjectObject::LoadProcess()
{
	BuildFilePath(options->tmp_projectObject_LoadProcess);
	return OnLoadObject(options->tmp_projectObject_LoadProcess.c_str());
}

//Переименовать объект
ProjectObject::ErrorCode ProjectObject::Rename(const char * newName)
{
	//Сохраняем старое имя
	char backup[sizeof(nameData)];
	memcpy(backup, nameData, sizeof(nameData));
	//Пути до текущего файла	
	BuildFilePath(options->tmp_projectObject_Rename_Cur);
	//Принудительно закрываем файл проекта (если это возможно)
	if(isBinFile)
	{
		if(!project->FileForceReleaseBin(options->tmp_projectObject_Rename_Cur.c_str()))
		{
			return ec_cant_closefilenow;
		}
	}else{
		if(!project->FileForceReleaseIni(options->tmp_projectObject_Rename_Cur.c_str()))
		{
			return ec_cant_closefilenow;
		}
	}
	//Ставим новое имя и получаем путь
	ErrorCode retCode = SetName(newName);
	if(retCode != ec_ok)
	{
		return retCode;
	}
	BuildFilePath(options->tmp_projectObject_Rename_New);
	//Переименовываем файл
	if(!options->fileService->Rename(options->tmp_projectObject_Rename_Cur.c_str(), options->tmp_projectObject_Rename_New.c_str()))
	{
		//Неудалось переименовать файл, похоже такое имя файла уже существует
		Verify(SetName(backup) == ec_ok);
		return ec_cant_rename;
	}
	return ec_ok;
}

//Можно ли переименовать объект в данное имя
ProjectObject::ErrorCode ProjectObject::IsCanRename(const char * newName)
{
	ErrorCode retCode = SetName(newName, false);
	BuildFilePath(options->tmp_projectObject_IsCanRename, newName);
	if(options->fileService->IsExist(options->tmp_projectObject_IsCanRename.c_str()))
	{
		return ec_cant_rename;
	}
	return ec_ok;
}

//Удаление объекта
ProjectObject::ErrorCode ProjectObject::Delete()
{
	//Путь до файла
	BuildFilePath(options->tmp_projectObject_Delete);
	//Принудительно закрываем файл проекта (если это возможно)
	if(isBinFile)
	{
		if(!project->FileForceReleaseBin(options->tmp_projectObject_Rename_Cur.c_str()))
		{
			return ec_cant_closefilenow;
		}
	}else{
		if(!project->FileForceReleaseIni(options->tmp_projectObject_Rename_Cur.c_str()))
		{
			return ec_cant_closefilenow;
		}
	}
	//Удаляем файл с диска
	options->fileService->Delete(options->tmp_projectObject_Delete.c_str());
	//Сообщаем объекту об удалении
	OnDeleteObject();
	return ec_ok;
}

//Установить имя
ProjectObject::ErrorCode ProjectObject::SetName(const char * n, bool isSet)
{
	//Проверяем имя на длинну
	dword len = string::Len(n);
	if(!len)
	{
		return ec_no_name;
	}
	if(len >= maxNameLen)
	{
		return ec_name_to_long;
	}
	//Копируем строку к себе
	if(isSet)
	{	
		memcpy(nameData, n, len);
		nameData[len] = 0;
		name.Init(nameData);
	}
	return ec_ok;
}











