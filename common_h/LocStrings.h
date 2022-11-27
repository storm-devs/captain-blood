//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// ILocStrings
//============================================================================================

#ifndef _ILocStrings_h_
#define _ILocStrings_h_


#include "core.h"

#define ILocStrings_StorageLocalizationPath		"Profile.Global.Localization"
#define ILocStrings_LocalizationDefaultValue	"eng"

class ILocStrings : public Service
{
public:
	static const long invalidateId = -1;

public:
	//Получить строку по идентификатору
	virtual const char * GetString(long id) = null;

public:
	//Загрузить все таблицы локализации очистив перед этим ресурсы
	virtual void LoadAll() = null;

	//Получить идентификатор языка
	virtual const char * GetLocId() = null;

	//Вывести в лог список неиспользуемых строк
	virtual void TraceUnuse() = null;


public:
	//Получить количество строк
	virtual dword GetStringsCount() = null;
	//Получить строку по индексу
	virtual const char * GetStringByIndex(dword index) = null;
	//Получить идентификатор по индексу
	virtual long GetIdByIndex(dword index) = null;

};


#endif

