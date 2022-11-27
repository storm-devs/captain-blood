

#include "IniFile.h"
#include "FileService.h"


//---------------------------------------------------------------------------------------------
//IniFile
//---------------------------------------------------------------------------------------------


IniFile::IniFile(const char * _path, bool _isReadOnly, const char * _cppFileName, long _cppFileLine)
{
	path = _path;
	parser.fileName = path;
	isChangeData = false;
	isReadOnly = _isReadOnly;
	lockRelease = false;
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
}

IniFile::~IniFile()
{
}


//Прочитать файл
bool IniFile::Open(IFileService_DFOpenMode mode)
{
	Assert(isReadOnly);
	IDataFile * file =  FileService::object->OpenDataFile(path, mode, _FL_);
	if(file)
	{
		dword size = file->Size();
		void * ptr = parser.Reserved(size);
		if(file->Read(ptr, size) != size)
		{
#ifndef STOP_DEBUG
			api->Error("FileService error: Can't read from ini file \"%s\" [read only mode] (file: %s, line: %i -> IniFile::Flush)", path.c_str(), cppFileName, cppFileLine);
#endif
			return false;
		}
		file->Release();
	}else{
#ifndef STOP_DEBUG
		api->Error("FileService error: Can't open ini file \"%s\" [read only mode] (file: %s, line: %i -> IniFile::Flush)", path.c_str(), cppFileName, cppFileLine);
#endif
		return false;
	}
	parser.Parse();	
	return true;
}

#ifndef _XBOX
//Открыть файл
bool IniFile::Open(IFileService_OpenMode mode)
{
	Assert(!isReadOnly);
	IFile * file =  FileService::object->OpenFile(path, mode, _FL_);
	if(file)
	{
		dword size = file->Size();
		void * ptr = parser.Reserved(size);
		if(file->Read(ptr, size) != size)
		{
#ifndef STOP_DEBUG
			api->Error("FileService error: Can't read from ini file \"%s\" [editable mode] (file: %s, line: %i -> IniFile::Flush)", path.c_str(), cppFileName, cppFileLine);
#endif
			return false;
		}
		file->Release();
	}else{
#ifndef STOP_DEBUG
		api->Error("FileService error: Can't open ini file \"%s\" [editable mode] (file: %s, line: %i -> IniFile::Flush)", path.c_str(), cppFileName, cppFileLine);
#endif
		return false;
	}
	if(mode == file_open_existing_for_read)
	{
		isReadOnly = true;
	}
	parser.Parse();	
	return true;
}
#endif

//Закрыть файл сообщив об ошибке
void IniFile::ErrorRelease()
{
#ifndef STOP_DEBUG
	api->Error("FileService error: IIniFile file \"%s\" not release (file: %s, line %i)", path.c_str(), cppFileName, cppFileLine);
#endif
	delete this;
}

//Блокировать возможность удалить файл
void IniFile::LockRelease(bool isLock)
{
	lockRelease = isLock;
}

//Закрыть файл
void IniFile::Release()
{
	if(lockRelease)
	{
		return;
	}
	if(!isReadOnly)
	{
		Flush();
	}
	SingleExClassThread(FileService::object)
	FileService::object->DeleteIniFile(this);
	delete this;
}

//Поулчить путь до файла
const char * IniFile::GetPath() const
{
	return path;
}

//Получить список секций
void IniFile::GetSections(array<string> & sections)
{
	sections.DelAll();
	parser.GetSections(sections);
}

//Добавить секцию
void IniFile::AddSection(const char * section)
{
	Assert(!isReadOnly);
	parser.AddSection(section);
	isChangeData = true;
}

//Удалить секцию с ключами
void IniFile::DelSection(const char * section)
{
	Assert(!isReadOnly);
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	if(sectionIndex >= 0)
	{
		parser.DelSection(sectionIndex);
		isChangeData = true;
	}
}

//Проверить наличие секции
bool IniFile::IsSectionCreated(const char * section)
{
	return (parser.FindSection(section) >= 0);
}

//Получить количество ключей
dword IniFile::GetKeysCount(const char * section, const char * name)
{
	//Подготовим имя ключа
	if(!name || !name[0]) return 0;
	long nameLen = strlen(name);
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	//Перебираем все одноимённые ключи в секции
	for(long keyIndex = -1, count = 0; true; count++)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0) break;
	}
	return count;
}

//Проверить наличие ключа в секции
bool IniFile::IsKeyCreated(const char * section, const char * name, long index)
{
	if(!name || !name[0]) return false;
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	if(sectionIndex < 0)
	{
		return false;
	}	
	long nameLen = strlen(name);
	return (parser.FindKey(sectionIndex, name, nameLen, index) >= 0);
}

//Удалить ключ
void IniFile::DelKey(const char * section, const char * name, long index)
{
	Assert(!isReadOnly);
	if(!name || !name[0]) return;
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	if(sectionIndex < 0)
	{
		return;
	}
	long nameLen = strlen(name);
	for(long keyIndex = -1; true; index--)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0) break;
		if(index <= 0)
		{
			parser.DelKey(keyIndex);
			isChangeData = true;
		}
	}
}

//Установить значение ключа как строку
void IniFile::SetString(const char * section, const char * name, const char * value, long index)
{
	Assert(!isReadOnly);
	//Подготовим имя ключа
	if(!name || !name[0]) return;
	long nameLen = strlen(name);
	//Находим индекс секции}
	long sectionIndex = parser.AddSection(section);
	Assert(sectionIndex >= 0);
	//Ищим ключ
	for(long keyIndex = -1; true; index--)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0)
		{
			isChangeData = true;
			if(index <= 0)
			{
				keyIndex = parser.AddKey(sectionIndex, name, value);
				return;
			}else{
				keyIndex = parser.AddKey(sectionIndex, name, "");
			}
		}
		if(index <= 0)
		{
			isChangeData = true;
			parser.SetKey(keyIndex, value);
			return;
		}
	}
}

//Получить значение ключа как строку
const char * IniFile::GetString(const char * section, const char * name, const char * defValue, long index)
{
	if(!name || !name[0]) return defValue;
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	if(sectionIndex < 0)
	{
		return defValue;
	}
	//Ищим ключ
	long nameLen = strlen(name);
	for(long keyIndex = -1; true; index--)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0)
		{
			return defValue;
		}
		if(index <= 0)
		{
			return parser.GetKey(keyIndex);
		}
	}
	return defValue;
}

//Получить все значения ключа как строки
void IniFile::GetStrings(const char * section, const char * name, array<string> & value)
{
	value.DelAll();
	//Подготовим имя ключа
	if(!name || !name[0]) return;
	long nameLen = strlen(name);
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	//Перебираем все одноимённые ключи в секции
	long keyIndex = -1;
	while(true)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0) return;
		value.Add(parser.GetKey(keyIndex));
	}
}

//Установить значение ключа как long
void IniFile::SetLong(const char * section, const char * name, long value, long index)
{
	char buf[128];
	crt_snprintf(buf, sizeof(buf), "%i", value);
	SetString(section, name, buf, index);
}

//Получить значение ключа как long
long IniFile::GetLong(const char * section, const char * name, long defValue, long index)
{
	const char * v = GetString(section, name, null, index);
	if(!v) return defValue;
	char * stop;
	return strtol(v, &stop, 10);
}

//Получить все значения ключа как long
void IniFile::GetLongs(const char * section, const char * name, array<long> & value)
{
	//Подготовим имя ключа
	if(!name || !name[0]) return;
	long nameLen = strlen(name);
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	//Перебираем все одноимённые ключи в секции
	long keyIndex = -1;
	while(true)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0) return;
		const char * v = parser.GetKey(keyIndex);
		char * stop;
		long vl = strtol(v, &stop, 10);
		value.Add(vl);
	}
}

//Установить значение ключа как float
void IniFile::SetFloat(const char * section, const char * name, float value, long index)
{
	SetDouble(section, name, value, index);
}

//Получить значение ключа как float
float IniFile::GetFloat(const char * section, const char * name, float defValue, long index)
{
	return (float)GetDouble(section, name, defValue, index);
}

//Получить все значения ключа как float
void IniFile::GetFloats(const char * section, const char * name, array<float> & value)
{
	value.DelAll();
	//Подготовим имя ключа
	if(!name || !name[0]) return;
	long nameLen = strlen(name);
	
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	//Перебираем все одноимённые ключи в секции
	long keyIndex = -1;
	while(true)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0) return;
		const char * v = parser.GetKey(keyIndex);
		char * stop;
		float vl = (float)strtod(v, &stop);
		value.Add(vl);
	}
}

//Установить значение ключа как double
void IniFile::SetDouble(const char * section, const char * name, double value, long index)
{
	char buf[128];
	crt_snprintf(buf, sizeof(buf), "%f", value);
	SetString(section, name, buf, index);
}

//Получить значение ключа как double
double IniFile::GetDouble(const char * section, const char * name, double defValue, long index)
{
	const char * v = GetString(section, name, null, index);
	if(!v) return defValue;
	char * stop;
	return strtod(v, &stop);
}

//Получить все значения ключа как double
void IniFile::GetDoubles(const char * section, const char * name, array<double> & value)
{
	value.DelAll();
	//Подготовим имя ключа
	if(!name || !name[0]) return;
	long nameLen = strlen(name);
	//Находим индекс секции
	long sectionIndex = parser.FindSection(section);
	//Перебираем все одноимённые ключи в секции
	long keyIndex = -1;
	while(true)
	{
		keyIndex = parser.FindKey(sectionIndex, name, nameLen, keyIndex);
		if(keyIndex < 0) return;
		const char * v = parser.GetKey(keyIndex);
		char * stop;
		double vl = strtod(v, &stop);
		value.Add(vl);
	}
}


//Сохранить изменения на диск немедленно
void IniFile::Flush()
{
#ifndef _XBOX
	Assert(!isReadOnly);
	dword size = 0;
	const void * data = parser.GetBuffer(size);
	IFile * file = FileService::object->OpenFile(path, file_create_always, _FL_);
	if(file)
	{
		if(file->Write(data, size) != size)
		{
#ifndef STOP_DEBUG
			api->Error("FileService error: Can't write to file \"%s\" (file: %s, line: %i -> IniFile::Flush)", path.c_str(), cppFileName, cppFileLine);
#endif
		}
		file->Release();
	}else{
#ifndef STOP_DEBUG
		api->Error("FileService error: Can't open to write ini file \"%s\" (file: %s, line: %i -> IniFile::Flush)", path.c_str(), cppFileName, cppFileLine);
#endif
	}
#endif
}

