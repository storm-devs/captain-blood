//Спиренков Максим, 2009

#pragma once

#include "AnxBase.h"

class ClipsRepository
{
	struct AntArchive
	{
		dword size;			//Размер компрессированых данных
		dword checkSum;		//Контрольная сумма файла для принятия решения о полном сравнении
		long refCount;		//Счётчик ссылок использующих данынй файл
		byte data[1];		//Данные файла
	};

public:
	struct Translate
	{
		long oldIndex;
		long newIndex;
	};

public:
	ClipsRepository();
	~ClipsRepository();
	//Удалить всё
	void Release();

	//Добавить компресированный ант на хранение
	long Add(byte * compressedAnt, dword size);
	//Удалить ант из хранилища
	void Del(long id);
	//Получить ант для использования
	byte * Get(long id, dword & size);


	//Записать данные в поток
	void Write(AnxStream & stream);
	//Прочитать данные из потока
	void Read(AnxStream & stream);
	//Импортировать данные из потока
	void Import(AnxStream & stream);

	//Подготовить таблицу индексов
	void BeginTranslates();
	//Закончить трансляцию
	void EndTranslates();
	//Преобразовать старый индекс в новый
	long TranslateIndex(long oldIndex);

	//Экспортировать ант
	void ExportAnt(long index, AnxStream & stream);
	//Импортировать ант
	long ImportAnt(AnxStream & stream);

private:
	//Все загруженые ант-файлы
	array<AntArchive *> ants;
	//Таблица трансляции
	array<Translate> translateTable;
};