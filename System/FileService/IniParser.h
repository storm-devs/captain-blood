
#ifndef _IniParser_h_
#define _IniParser_h_

#include "..\..\common_h\core.h"

class IniParser
{
	struct Token
	{
		long start;		//Начало токена
		long size;		//Размер токена
	};

	struct Key
	{
		Token name;		//Имя ключа
		Token value;	//Значение ключа
		long startLine;	//Начало строки с ключём
		long endLine;	//Окончание строки с ключём
	};

	struct Section
	{
		Token name;		//Имя секции
		long start;		//Индекс начального ключа
		long size;		//Количество ключей
		long startLine;	//Начало строки с названием секции
		long endLine;	//Окончание строки с названием секции
	};


//--------------------------------------------------------------------------------------------------
public:
	IniParser();
	~IniParser();

//--------------------------------------------------------------------------------------------------
//Инициализация
//--------------------------------------------------------------------------------------------------
public:
	//Выделить буфер для внешнего заполнения
	void * Reserved(long _size);
	//Разобрать файл на токены для дальнейшего использования
	void Parse();

//--------------------------------------------------------------------------------------------------
//Работа с ini
//--------------------------------------------------------------------------------------------------
public:
	//Получить все секции
	void GetSections(array<string> & sects);
	//Найти секцию
	long FindSection(const char * section);
	//Добавить новую секцию
	long AddSection(const char * section);
	//Удалить секцию
	void DelSection(long sectionIndex);
	//Найти ключ в секции начиная с current
	long FindKey(long sectionIndex, const char * name, long nameLen, long keyIndex = -1);
	//Установить новое значение ключу
	void SetKey(long keyIndex, const char * value);
	//Получить значение ключа
	const char * GetKey(long keyIndex);
	//Добавить ключ
	long AddKey(long sectionIndex, const char * name, const char * value);
	//Удалить ключ
	void DelKey(long keyIndex);

	//Получить буфер
	const void * GetBuffer(dword & size);

//--------------------------------------------------------------------------------------------------
//Вывод сообщений об ошибке
//--------------------------------------------------------------------------------------------------
protected:
	//Добавить сообщение об ошибке
	void _cdecl AddError(const char * format, ...);

//--------------------------------------------------------------------------------------------------
//Системное
//--------------------------------------------------------------------------------------------------
private:
	//Найти начало токена
	long ParseFindStartToken(long i);
	//Пропускаем коментарий
	long ParseSkipComment(long i);
	//Зачитываем название секции
	long ParseGetSection(long i);
	//Зачитываем ключ
	long ParseGetKey(long i);
	//Сравнить 2 строки
	bool ParseEqual(long start1, long size1, long start2, long size2);
	//Сравнить 2 строки
	bool Equal(const Token & tk, const char * str, long len);
	//Заполнить строку
	void FillString(const Token & tk, string & str);
	//Привести символ к нижнему регистру
	static char ToLower(char c);
	//Заменить часть данных на новые
	void ChangeData(long start, long size, const char * str);
	//Поправить начало строки
	static void Modify(long start, long dlt, long & current);

//--------------------------------------------------------------------------------------------------
public:
	string fileName;
private:
	array<byte> data;
	array<Key> keys;
	array<Section> sections;
	string temp;
	static long startLine;
	static long line;
};


#endif
