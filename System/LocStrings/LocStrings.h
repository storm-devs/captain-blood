//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// LocStrings
//============================================================================================

#ifndef _LocStrings_h_
#define _LocStrings_h_

#include "..\..\common_h\LocStrings.h"
#include "..\..\common_h\fileservice.h"


class LocStrings : public ILocStrings
{
	enum Flags
	{
		f_use = 1,
	};

	struct StringElement
	{
		long id;
		short next;
		word flags;
		string str;
	};

	//Локализационный звуковой банк миссии
	struct MissionSoundBank
	{
		string missionName;
		long refCounter;
	};

public:
	LocStrings();
	~LocStrings();

	//Инициализация
	virtual bool Init();

public:
	//Получить строку по идентификатору
	virtual const char * GetString(long id);
	//Вывести в лог список неиспользуемых строк
	virtual void TraceUnuse();

public:	
	//Загрузить все таблицы локализации очистив перед этим ресурсы
	void LoadAll();

public:
	//Получить идентификатор языка
	virtual const char * GetLocId();

public:	
	//Получить количество строк
	virtual dword GetStringsCount();
	//Получить строку по индексу
	virtual const char * GetStringByIndex(dword index);
	//Получить идентификатор по индексу
	virtual long GetIdByIndex(dword index);

private:


	//Загрузить таблицу локализации
	void LoadTable (IFileService * fs, const char * szFileName);

	//Разбираем строку
	bool ParseString(const char * & str, const char * end, StringElement & se, long column);
	//Добавить запись
	void AddString(StringElement & se);

private:
	array<StringElement> strings;
	long entryTable[1024];	
	string locId;	
	string tmpBuffer;

};



#endif

