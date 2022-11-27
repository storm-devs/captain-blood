//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// LocStrings
//============================================================================================

#include "LocStrings.h"
#include "..\..\common_h\data_swizzle.h"
#include "..\..\common_h\StringUtils\Win1250.h"
#include "..\..\common_h\StringUtils\Win1251.h"
#include "..\..\common_h\StringUtils\Win1252.h"
#include "..\..\common_h\sound.h"

#ifdef _XBOX
CREATE_SERVICE(LocStrings, 23)
#else
CREATE_SERVICE(LocStrings, 3)
#endif

#define DELIMETER '\t'

LocStrings::LocStrings() : strings(_FL_, 256)
{
	strings.Reserve(1024);
}

LocStrings::~LocStrings()
{
}


//Инициализация
bool LocStrings::Init()
{
	LoadAll();
	return true;
}

//Получить строку по идентификатору
const char * LocStrings::GetString(long id)
{
	long eindex = id & (ARRSIZE(entryTable) - 1);
	for(long i = entryTable[eindex]; i >= 0; i = strings[i].next)
	{
		if(strings[i].id == id)
		{
			strings[i].flags |= f_use;
			return strings[i].str;
		}
	}
	return null;
}

//Вывести в лог список неиспользуемых строк
void LocStrings::TraceUnuse()
{
	api->Trace("-------------------------------------------\nUnuse strings table\n-------------------------------------------\n\n");
	for(dword i = 0; i < strings.Size(); i++)
	{
		StringElement & se = strings[i];
		if((se.flags & f_use) == 0)
		{
			api->Trace("%.5i  :   %s", se.id, se.str.c_str());
		}
	}
	api->Trace("-------------------------------------------\n\n");
}


void LocStrings::LoadTable(IFileService * fs, const char * szFileName)
{
	//Загружаем файл
	Assert(fs);
	ILoadBuffer * loadBuffer = fs->LoadData(szFileName, _FL_);
	if(!loadBuffer)
	{
		api->Trace("LocStrings ERROR: Can't find or open language file '%s' !!!", szFileName);
		return;
	}
		
	api->Trace("LocStrings message: Load language file '%s'", szFileName);


	dword dwFileSize = loadBuffer->Size();

	word* header = (word *)loadBuffer->Buffer();


	LPCWSTR unicodeData = (LPCWSTR)(loadBuffer->Buffer() + sizeof(word));
	
	//Верно только для UTF-16 UCS2
	dword unicodeCharCount = (dwFileSize - sizeof(word)) >> 1;

	if (*header == 0xFFFE)
	{
		//inverse byte order unicode (swizzle need)
		short* swizzleArray = (short*)unicodeData;
		for (dword i = 0; i < unicodeCharCount; i++)
		{
			swizzleArray[i] = __DataSwizzler(swizzleArray[i]);
		}

	} else
	{
		if (*header == 0xFEFF)
		{
			//platform byte order unicode

		} else
		{
			api->Trace("LocStrings ERROR: Language is not unicode file !!!");
			return;
		}
	}


	dword ansiDataSize = dwFileSize + 1;
	array<char> ansiTextData(_FL_);
	ansiTextData.AddElements(ansiDataSize);
	char * ansiText = ansiTextData.GetBuffer();
	memset (ansiText, 0, ansiDataSize);
		
	int ansiCharsResult = ConvertUnicodeToCP1251(unicodeData, ansiText, ansiDataSize);
	if (ansiCharsResult == 0)
	{
		api->Trace("LocStrings ERROR: Can't convert localization text from UNICODE to ANSI !!");
		return;
	}
	
	//Разбираем текст
	StringElement se;
	const char * text = (const char *)ansiText;
	const char * end = (const char *)(ansiText + ansiCharsResult);

	//Пропускаем первую строчку, там шапка с названиями языков...
	for(; text < end; text++)
	{
		if(*text == '\r' || *text == '\n') break;
	}

	//ansiText ... text - тут шапка с названиям языков, что бы выбрать колонку правильную
	//text ... end - тут csv таблица, которую надо парсить

	//Строим список языков, список что бы потом можно было к примеру наружу этот список отдать
	//Если нужно будет...
	array<string> languages(_FL_);
	for (dword lang = 0; lang < 512; lang++)
	{
		const char * headerText = (const char *)ansiText;
		bool parseResult = ParseString(headerText, text, se, lang);
		if(parseResult)
		{
			//api->Trace("LocStrings message: Found language : '%s'", se.str.c_str());
			languages.Add(se.str);
		} else
		{
			//Строчка не парсится - используем по умолчанию, первый столбец
			break;
		}
	}

	if (languages.Size() <= 0)
	{
		api->Trace("LocStrings ERROR: No languages file found !!!");
		return;
	}

	long localizationColumn = 0;
	for (dword lang = 0; lang < languages.Size(); lang++)
	{
		if (locId == languages[lang])
		{
			//Нашли нужный язык...
			localizationColumn = lang;
			break;
		}
	}

	//api->Trace("LocStrings message: Selected language : '%s'", languages[localizationColumn].c_str());
	while(text < end)
	{
		//Выделяем строку
		const char * str = text;
		const char * str1 = text;
		//Ищем окончание строки
		for(; text < end; text++)
		{
			if(*text == '\r' || *text == '\n') break;
		}
		//Разбираем строку
		if(ParseString(str, text++, se, localizationColumn))
		{
			if(se.id >= 0)
			{
				AddString(se);
			}else{
				api->Trace("LocStrings ERROR: Found line without identificator!!! -> %s", str1);
			}			
		}
	}
	loadBuffer->Release();
}


void LocStrings::LoadAll()
{
	//Очищаем текущее содержимое
	for(long i = 0; i < ARRSIZE(entryTable); i++)
	{
		entryTable[i] = -1;
	}
	strings.DelAll();
	//Очищаем идентификатор языка
	locId.Empty();
	//Файловый сервис
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	//Получаем текущий язык...
#ifndef GAME_RUSSIAN
	const char* currentLanguage = api->Storage().GetString(ILocStrings_StorageLocalizationPath, null);
	if(!currentLanguage)
	{
		IIniFile * iniFile = fs->SystemIni();
		const char * loc = iniFile->GetString(null, "localization", null);
		if(!string::IsEmpty(loc))
		{
			currentLanguage = loc;
		}else{
			currentLanguage = ILocStrings_LocalizationDefaultValue;
		}
	}
	locId = currentLanguage;
#else
	locId = "";
	locId += "r";
	locId += "";
	locId += "us";
#endif
	
	//Перебераем локализационные файлы
#ifndef _XBOX
	IPackFile * pack = fs->LoadPack("Resource\\text.pkx", _FL_);
#else
	IPackFile * pack = fs->LoadPack("text.pkx", _FL_);
#endif
	IFinder* fileFinder = fs->CreateFinder("Resource\\Text\\", "*.csv", find_all_files, _FL_);
	for (dword i = 0; i < fileFinder->Count(); i++)
	{
		const char * fileName = fileFinder->FilePath(i);
		LoadTable(fs, fileName);
	}	
	fileFinder->Release();	
	//Загружаем звуковые банки
	RELEASE(pack);
}

//Получить идентификатор языка
const char * LocStrings::GetLocId()
{
	return locId.c_str();
}


//Получить количество строк
dword LocStrings::GetStringsCount()
{
	return strings.Size();
}

//Получить строку по индексу
const char * LocStrings::GetStringByIndex(dword index)
{
	return strings[index].str.c_str();
}

//Получить идентификатор по индексу
long LocStrings::GetIdByIndex(dword index)
{
	return strings[index].id;
}


//Разбираем строку
bool LocStrings::ParseString(const char * & str, const char * end, StringElement & se, long column)
{
	se.id = -1;
	se.next = -1;
	se.str.Empty();
	//Пропускаем свободное место
	for(; str < end; str++)
	{
		if(*str == DELIMETER) return false;
		if(*str >= '0' || *str <= '9') break;
	}
	if(str >= end) return false;
	//Получаем идентификатор
	bool isFindNumber = false;
	for(long id = 0, count = 0; str < end && count < 10; str++, count++)
	{
		if(*str < '0' || *str > '9') break;
		isFindNumber = true;
		id = id*10 + *str - '0';
	}
	if(!isFindNumber)
	{
		id = -1;
	}

	//Ищем разделитель (пропускаем, столько разделителей, сколько заданно в column)
	long skipCount = column;
	for(; str < end; str++)
	{
		//Такого столбца нет !!!
		if(*str == '\r' || *str == '\n')
		{
			return false;
		}

		if(*str == DELIMETER)
		{
			if (skipCount <= 0)
			{
				break;
			} else
			{
				skipCount--;
			}
		}
	}
	if(str >= end) return false;
	//Теперь начало столбца в str



	const char * columnEnd = str;
	columnEnd++;
	for(; columnEnd < end; columnEnd++)
	{
		if(*columnEnd == '\r' || *columnEnd == '\n')
		{
			columnEnd--;
			break;
		}

		if(*columnEnd == DELIMETER)
		{
			break;
		}
	}

	//columnEnd теперь это последний символ в столбце...

	//Если конец столбца раньше начала, он пустой к примеру, то на выход
	if(columnEnd < str) return false;

	end = columnEnd;


	//Ищем начало строки
	for(str++; str < end; str++)
	{
		if(*str != ' ') break;
	}
	if(str >= end) return false;

	//Модифицируем конец строки
	for(end--; end >= str; end--)
	{
		if(*end != ' ') break;
	}
	//Добавляем запись	
	se.id = id;
	for(; str <= end; str++)
	{
		se.str += *str;
	}

	str++;

	return true;
}


//Добавить запись
void LocStrings::AddString(StringElement & se)
{
	//api->Trace("%d = '%s'", se.id, se.str.c_str());

	const char* szStringExist = GetString(se.id);
	if(szStringExist)
	{
		api->Trace("string with id = %d (in table: \"%s\", add: \"%s\") have duplicated !!\n", se.id, szStringExist, se.str.c_str());
		szStringExist = null;
	}

	Assert(se.id >= 0);
	//Assert(szStringExist == null);
	Assert(strings.Size() < 0x7fff);

	dword index = strings.Add(se);
	StringElement & st = strings[index];
	st.next = -1;
	st.flags = 0;
	long eindex = se.id & (ARRSIZE(entryTable) - 1);	
	if(entryTable[eindex] >= 0)
	{
		for(long i = entryTable[eindex]; strings[i].next >= 0; i = strings[i].next);
		strings[i].next = index;
	}else{
		entryTable[eindex] = index;
	}

	//Фиксим кавычки
	char * src = st.str.GetDataBuffer();
	if(src)
	{
		char * dst = src;
		while(*src)
		{
			if(*src != '"')
			{
				*dst++ = *src++;
			}else{
				if(src[1] != '"')
				{
					src++;
				}else{
					*dst++ = '"';
					for(src += 2; *src == '"'; src++);
				}
			}
		}
		*dst++ = 0;
		st.str.SetDataSize(dst - st.str.GetDataBuffer());
	}

	//api->Trace("string %s", st.str.c_str());
}


