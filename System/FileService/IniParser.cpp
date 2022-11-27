

#include "IniParser.h"


long IniParser::startLine = 0;
long IniParser::line = 1;


IniParser::IniParser() : data(_FL_, 256),
							keys(_FL_),
							sections(_FL_)
{
}

IniParser::~IniParser()
{
}

//Выделить буфер для внешнего заполнения
void * IniParser::Reserved(long _size)
{
	if(_size <= 0) return 0;
	data.Empty();
	data.AddElements(_size);
	return &data[0];
}

//Разобрать файл на токены для дальнейшего использования
void IniParser::Parse()
{
	//Удаляем текущее состояние
	keys.Empty();
	sections.Empty();
	//Текущая строка
	line = 1;
	//Начало текущей строки
	startLine = 0;
	//Добавляем первую пустую секцию
	Section & s = sections[sections.Add()];
	s.name.start = 0;
	s.name.size = 0;
	s.start = 0;
	s.size = 0;
	//Разбираем данные
	for(long i = 0; i < data; )
	{
		//Ищем начало токена на строке
		i = ParseFindStartToken(i);
		if(i >= data) break;
		//Определяем тип
		switch(data[i])
		{
		//Перевод строки
		case '\n':
			line++;
			startLine = i + 1;
		case '\r':
			i++;
			break;
		//Коментарий
		case ';':
			i = ParseSkipComment(i);
			break;
		//Секция
		case '[':
			i = ParseGetSection(i);
			break;
		//Ключ
		default:
			i = ParseGetKey(i);
		}
	}
	//Завершение последний секции
	sections[sections - 1].size = keys - sections[sections - 1].start;
	//Вывод отладочной информации
	if(false)
	{
		string value;
		AddError("--------------------------IniParser log--------------------------\nFile:\"%s\"\n", fileName.GetBuffer());
		for(long i = 0; i < sections; i++)
		{
			FillString(sections[i].name, temp);
			AddError("S: %4i    [\"%s\"]", i, temp.GetBuffer());
			long start = sections[i].start;
			long size = sections[i].size;
			for(long j = 0; j < size; j++)
			{
				FillString(keys[start + j].name, temp);
				FillString(keys[start + j].value, value);
				AddError("K: %4i    \"%s\" = \"%s\"", start + j, temp.GetBuffer(), value.GetBuffer());
			}
		}
		AddError("\n");
	}

}

//Получить все секции
void IniParser::GetSections(array<string> & sects)
{
	sects.DelAll();
	for(long i = 0; i < sections; i++)
	{
		string & s = sects[sects.Add()];
		FillString(sections[i].name, s);
	}
}

//Найти секцию
long IniParser::FindSection(const char * section)
{
	if(!section || !section[0])
	{
		Assert(sections[0].name.size == 0);
		return 0;
	}
	long len = strlen(section);
	for(long i = 0; i < sections; i++)
	{
		if(Equal(sections[i].name, section, len))
		{
			return i;
		}
	}
	return -1;
}

//Добавить новую секцию
long IniParser::AddSection(const char * section)
{	
	long i = FindSection(section);
	if(i >= 0) return i;
	i = sections[sections.Size() - 1].endLine;
	if(keys > 0)
	{
		i = keys[keys - 1].endLine;
	}
	string str = "\r\n\r\n[";
	str += section;
	str += "]\r\n\r\n";
	ChangeData(i, 0, str);
	long index = sections.Add();
	Section & s = sections[index];
	s.name.start = i + 5;
	s.name.size = strlen(section);
	s.start = keys;
	s.size = 0;
	s.startLine = i + 4;
	s.endLine = s.startLine + s.name.size + 2;
	return index;
}

//Удалить секцию
void IniParser::DelSection(long sectionIndex)
{
	Section & s = sections[sectionIndex];
	//Удаляем все ключи в секции
	while(s.size)
	{
		DelKey(s.start);
	}
	//Удаляем данные секции
	ChangeData(s.startLine, s.endLine - s.startLine + 1, "");
	//Удаляем секцию
	sections.Extract(sectionIndex);
}

//Найти ключ в секции
long IniParser::FindKey(long sectionIndex, const char * name, long nameLen, long keyIndex)
{
	//Имя ключа
	if(sectionIndex < 0) return -1;
	Assert(name);
	Assert(nameLen > 0);
	//Ключи в секции
	long i = keyIndex < 0 ? sections[sectionIndex].start : keyIndex + 1;
	Assert(i >= sections[sectionIndex].start);
	long end = sections[sectionIndex].start + sections[sectionIndex].size;
	//Перебираем ключи
	for(; i < end; i++)
	{
		if(Equal(keys[i].name, name, nameLen))
		{
			return i;
		}
	}
	return -1;
}

//Установить новое значение ключу
void IniParser::SetKey(long keyIndex, const char * value)
{
	if(!value) value = "";
	Key & key = keys[keyIndex];
	ChangeData(key.value.start, key.value.size, value);
	key.value.size = strlen(value);
	if(key.endLine < key.value.start + key.value.size) key.endLine = key.value.start + key.value.size;
	Assert((dword)key.endLine <= data.Size());
}

//Получить значение ключа
const char * IniParser::GetKey(long keyIndex)
{
	Assert(keyIndex >= 0);
	FillString(keys[keyIndex].value, temp);
	return temp;
}

//Добавить ключ
long IniParser::AddKey(long sectionIndex, const char * name, const char * value)
{
	if(!value) value = "";
	//Если ключи у секции есть то дописываемся за последним, иначе за секцией
	long keyNameStart = -1;
	if(sections[sectionIndex].size > 0)
	{
		long lastKey = sections[sectionIndex].start + sections[sectionIndex].size - 1;
		keyNameStart = keys[lastKey].endLine;
	}else{
		keyNameStart = sections[sectionIndex].endLine;
	}
	//Добавляем запись секции в файл
	temp = "\r\n";
	temp += name;
	temp += " = ";
	long keyValueStart = keyNameStart + temp.Len();
	temp += value;
	ChangeData(keyNameStart, 0, temp);
	//Добавляем описание ключа
	long index = sections[sectionIndex].start + sections[sectionIndex].size;
	keys.Insert(index);
	Key & key = keys[index];
	sections[sectionIndex].size++;
	for(long j = sectionIndex + 1; j < sections; j++)
	{
		sections[j].start++;
	}
	key.name.start = keyNameStart + 2;
	key.name.size = strlen(name);
	key.value.start = keyValueStart;
	key.value.size = strlen(value);
	key.startLine = key.name.start;
	key.endLine = key.value.start + key.value.size;
	Assert((dword)key.endLine <= data.Size());
	return index;
}

//Удалить ключ
void IniParser::DelKey(long keyIndex)
{
	//Запомним строку, которую надо удалить
	long start = keys[keyIndex].startLine;
	long size = keys[keyIndex].endLine - keys[keyIndex].startLine + 1;
	//Удалим запись ключа
	for(long i = 0; i < sections; i++)
	{	
		if(sections[i].start <= keyIndex)
		{
			//В своей секции уменьшим количество ключей
			long n = sections[i].start + sections[i].size;
			if(keyIndex < n)
			{
				sections[i].size--;
			}			
		}else{
			//В дальней секции уменьшим начальный индекс
			sections[i].start--;
		}		
	}
	keys.Extract(keyIndex);
	//Удалим данные ключа
	ChangeData(start, size, "");
}

//Получить буфер
const void * IniParser::GetBuffer(dword & size)
{
	size = data;
	return data.GetBuffer();
}

//Добавить сообщение об ошибке
void _cdecl IniParser::AddError(const char * format, ...)
{
	api->TraceData(format, (&format) + 1);
}

//Найти начало токена
__forceinline long IniParser::ParseFindStartToken(long i)
{
	for(; i < data; i++)
	{
		switch(data[i])
		{
		case ' ':
		case '\t':
			break;
		default:
			return i;
		}
	}
	return i;
}

//Пропускаем коментарий
__forceinline long IniParser::ParseSkipComment(long i)
{
	for(; i < data; i++)
	{
		switch(data[i])
		{
		case '\n':
		case '\r':
			return i;
		}
	}
	return i;
}

//Зачитываем название секции
__forceinline long IniParser::ParseGetSection(long i)
{
	for(long start = ++i, size; i < data; i++)
	{
		switch(data[i])
		{
		case ']':
			//Если имя 0 длинны пропускаем эту секцию
			if(i == start)
			{
				AddError("Ini file error (file \"%s\", line %i): Section without name.", fileName.GetBuffer(), line);
				return i + 1;
			}
			size = i++ - start;
			goto addSection;
		case '\n':
		case '\r':
			goto stopSection;
		}
	}
stopSection:
	size = i - start;
	AddError("Ini file error (file \"%s\", line %i): Section not closed.", fileName.GetBuffer(), line);
addSection:
	//Ищем окончание строки
	for(; i < data && data[i] != '\n'; i++);
	//Поищим имя среди добавленных секций
	for(long n = 0; n < sections; n++)
	{
		if(ParseEqual(sections[n].name.start, sections[n].name.size, start, size))
		{
			AddError("Ini file error (file \"%s\", line %i): Repeat name section.", fileName.GetBuffer(), line);
			return i;
		}
	}
	//Отметим размер предыдущий секции
	sections[sections - 1].size = keys - sections[sections - 1].start;
	//Добавляем секцию
	Section & s = sections[sections.Add()];
	s.name.start = start;
	s.name.size = size;
	s.start = keys;
	s.size = 0;
	s.startLine = startLine;
	s.endLine = i < data ? i : data - 1;
	return i;
}

//Зачитываем ключ
__forceinline long IniParser::ParseGetKey(long i)
{
	//Ищем окончание названия ключа
	for(long start = i; i < data; i++)
	{
		bool isBreak = false;
		switch(data[i])
		{
		case '=':
		case '\r':
		case '\n':
			isBreak = true;
			break;
		}
		if(isBreak) break;
	}
	if(i >= data || data[i] != '=')
	{
		AddError("Ini file error (file \"%s\", line %i): Invalidate key syntactic, not found '='", fileName.GetBuffer(), line);
		return ParseSkipComment(i);
	}
	//Ищем реальное окончание ключа
	for(long end = i - 1; data[end] == ' ' || data[end] == '\t'; end--);
	//Пропускаем пробелы после '='
	for(i++; i < data && (data[i] == ' ' || data[i] == '\t'); i++);
	//Ищим окончание строки
	for(long vstart = i; i < data; i++)
	{
		bool isBreak = false;
		switch(data[i])
		{
		case '\r':
		case '\n':
			isBreak = true;
			break;
		}
		if(isBreak) break;
	}
	//Ищим окончание значения
	long vend = i - 1;
	if(vstart < data)
	{
		for(; data[vend] == ' ' || data[vend] == '\t'; vend--);
	}
	//Добавляем запись ключа в массив
	Key & key = keys[keys.Add()];
	key.name.start = start;
	key.name.size = end - start + 1;
	key.value.start = vstart;
	key.value.size = vend - vstart + 1;
	key.startLine = startLine;
	//Уточняем окончание строки
	for(; i < data && data[i] != '\n' && data[i] != '\r'; i++);
	key.endLine = i < data ? i : data - 1;
	return i;
}

//Сравнить 2 строки
__forceinline bool IniParser::ParseEqual(long start1, long size1, long start2, long size2)
{
	if(size1 != size2) return false;
	const byte * pnt1 = &data[start1];
	const byte * pnt2 = &data[start2];
	for(long i = 0; i < size1; i++)
	{
		if(ToLower(*pnt1++) != ToLower(*pnt2++)) return false;
	}
	return true;
}

//Сравнить 2 строки
__forceinline bool IniParser::Equal(const Token & tk, const char * str, long len)
{
	if(tk.size != len || len <= 0) return false;
	const byte * pnt = &data[tk.start];
	for(long i = 0; i < len; i++)
	{
		if(ToLower(*pnt++) != ToLower(*str++)) return false;
	}
	return true;
}

//Заполнить строку
__forceinline void IniParser::FillString(const Token & tk, string & str)
{
	str.Empty();
	for(long c = tk.size, i = tk.start; c > 0; c--)
	{
		str += (char)data[i++];
	}
}

//Привести символ к нижнему регистру
__forceinline char IniParser::ToLower(char c)
{
	if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
	return c;
}

//Заменить часть данных на новые
void IniParser::ChangeData(long start, long size, const char * str)
{
	//Необходимо сделать систему изменения данных с поправкой токенов
	if(!str) str = "";
	long len = strlen(str);
	long dsize = len - size;
	//Поправляем все ключи
	for(long i = 0; i < keys; i++)
	{
		Modify(start, dsize, keys[i].name.start);
		Modify(start, dsize, keys[i].value.start);
		Modify(start, dsize, keys[i].startLine);
		Modify(start, dsize, keys[i].endLine);
	}
	//Поправляем все секции
	for(long i = 0; i < sections; i++)
	{
		Modify(start, dsize, sections[i].name.start);
		Modify(start, dsize, sections[i].startLine);
		Modify(start, dsize, sections[i].endLine);
	}
	//Модифицируем данные
	if(dsize > 0)
	{
		for(long i = 0; i < dsize; i++)
		{
			data.Insert(start);
		}
	}else
	if(dsize < 0)
	{
		dsize = -dsize;
		for(long i = 0; i < dsize; i++)
		{
			data.Extract(start);
		}
	}
	for(long i = 0; i < len; i++)
	{
		data[start + i] = str[i];
	}
}

//Поправить начало строки
__forceinline void IniParser::Modify(long start, long dlt, long & current)
{
	if(current > start)
	{
		current += dlt;
	}
}

