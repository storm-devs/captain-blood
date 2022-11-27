
#ifndef _IniFile_h_
#define _IniFile_h_

#include "..\..\common_h\FileService.h"
#include "IniParser.h"


//Интерфейс на ini-файл
class IniFile : public IEditableIniFile
{
public:
	IniFile(const char * _path, bool _isReadOnly, const char * _cppFileName, long _cppFileLine);
	virtual ~IniFile();
	//Прочитать файл
	bool Open(IFileService_DFOpenMode mode);
#ifndef _XBOX
	//Открыть файл
	bool Open(IFileService_OpenMode mode);
#endif
	//Закрыть файл сообщив об ошибке
	void ErrorRelease();
	//Блокировать возможность удалить файл
	void LockRelease(bool isLock);


public:
	//Закрыть файл
	virtual void Release();

	//Поулчить путь до файла
	virtual const char * GetPath() const;

	//Получить список секций
	virtual void GetSections(array<string> & sections);
	//Добавить секцию
	virtual void AddSection(const char * section);
	//Удалить секцию с ключами
	virtual void DelSection(const char * section);
	//Проверить наличие секции
	virtual bool IsSectionCreated(const char * section);

	//Получить количество ключей
	virtual dword GetKeysCount(const char * section, const char * name);
	//Проверить наличие ключа в секции
	virtual bool IsKeyCreated(const char * section, const char * name, long index = 0);
	//Удалить ключ
	virtual void DelKey(const char * section, const char * name, long index = 0);

	//Установить значение ключа как строку
	virtual void SetString(const char * section, const char * name, const char * value, long index = 0);
	//Получить значение ключа как строку
	virtual const char * GetString(const char * section, const char * name, const char * defValue = "", long index = 0);
	//Получить все значения ключа как строки
	virtual void GetStrings(const char * section, const char * name, array<string> & value);

	//Установить значение ключа как long
	virtual void SetLong(const char * section, const char * name, long value, long index = 0);
	//Получить значение ключа как long
	virtual long GetLong(const char * section, const char * name, long defValue = 0, long index = 0);
	//Получить все значения ключа как long
	virtual void GetLongs(const char * section, const char * name, array<long> & value);

	//Установить значение ключа как float
	virtual void SetFloat(const char * section, const char * name, float value, long index = 0);
	//Получить значение ключа как float
	virtual float GetFloat(const char * section, const char * name, float defValue = 0.0, long index = 0);
	//Получить все значения ключа как float
	virtual void GetFloats(const char * section, const char * name, array<float> & value);

	//Установить значение ключа как double
	virtual void SetDouble(const char * section, const char * name, double value, long index = 0);
	//Получить значение ключа как double
	virtual double GetDouble(const char * section, const char * name, double defValue = 0.0, long index = 0);
	//Получить все значения ключа как double
	virtual void GetDoubles(const char * section, const char * name, array<double> & value);
	//Сохранить изменения на диск немедленно
	virtual void Flush();


protected:
	string path;
	bool isReadOnly;
	bool isChangeData;
	bool lockRelease;
	IniParser parser;
#ifndef STOP_DEBUG
	const char * cppFileName;
	long cppFileLine;
#endif
};


#endif
