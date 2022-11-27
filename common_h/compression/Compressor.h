//Spirenkov Max, 2008

#include "..\core.h"

class Compressor
{
public:
	enum CompressMethod
	{
		cm_maxcheck = 0,		//Максимально детальный просмотр файла, самый медленный (использует ~5Mb)
		cm_skip = 1,			//Пропуск некоторых данных, средняя скорость (использует ~5Mb)
		cm_fast = 2,			//Максимально быстрый но чаще с самым маленьким коэфициентом (использует ~20kb)
		cm_nousememory = 3,		//Не быстрый и аналогичный по степени сжатия cm_fast, не требует дополнительной памяти
	};


	enum DecReturn
	{
		dr_ok = 0,			//Всё в порядке
		dr_buffer_is_small = 1,		//Недостаточный размер буфера для распакованных данных
		dr_no_source = 2,		//Нет входных данных
		dr_damage_source = 3,		//Повреждены входные данные
	};

private:
	enum Constants
	{
		offset_mask_far = 0xfffff,
		offset_mask_near = 0xfff,
		chain_mask = 0xfff,
		hash_mask = 0xfff,
		unpack_len = 15,

		window_size_far = offset_mask_far + 1,
		window_size_near = offset_mask_near + 1,
		chain_len = chain_mask + 6,
		hash_size = hash_mask + 1
	};

	struct Hash
	{
		friend class Compressor;	//Для обращения к константам

		Hash(dword winSize);
		~Hash();

		//Подготовить таблицу
		void Prepare();
		//Добавить в текущую позицию хэшь
		void Add(dword curPos, dword hash);
		//Найти первый элемент в списке
		dword GetFirst(dword curPos, dword hash);
		//Получить следующий элемент в списке
		dword GetNext(dword curPos, dword address);

	private:
		dword entry[Compressor::hash_size];		//Входная таблица
		dword * window;					//Окно
		const dword windowSize;				//Размер окна
		const dword windowMask;				//Маска окна
	};

public:
	Compressor();
	~Compressor();

//-----------------------------------------------------------------
//Упаковка
//-----------------------------------------------------------------
public:	
	//Упаковать исходные данные
	void __declspec(dllexport) Compress(const byte * dataPtr, dword dataSize, CompressMethod method = cm_skip);
	//Получить указатель на буфер с данными
	const byte * Buffer();
	//Получить размер данных
	dword Size();

//-----------------------------------------------------------------
//Распаковка
//-----------------------------------------------------------------
public:
	//Распаковать данные
	static DecReturn Decompress(const byte * dataPtr, dword dataSize, byte * bufferPrt, dword & bufferSize);


//-----------------------------------------------------------------
//Утилитные
//-----------------------------------------------------------------
public:
	//Подготовить таблицы для данного метода сжатия
	void Prepare(CompressMethod method = cm_skip, bool isReleaseBuffers = false);
	//Удалить все используемые ресурсы
	void Free();


//-----------------------------------------------------------------
private:
	//Кодировать текущую информацию исходя из найденных цепочек
	void Encode(const bool updateHash);
	//Зарезервировать место в буфере если нужно
	void ReserveBuffer();
	//Просмотр цепочек через хэшь
	void FindChainUseHash(Hash * hashTable, dword hashValue);
	//Просмотр цепочек
	void FindChainWithoutHash();
	//Пропустить столько байт, занеся их в хэшь-таблицы
	void SkipBytes(dword count);
	//Расчитать индекс для вхождения в хэшь таблицу
	static dword CalcHash(const byte * ptr, dword count, dword hash = 1315423911);
	//Выяснить сколько байт равно и вернуть соответствующие количество (dword не равны)
	static dword CompareResidue(const byte * s1, const byte * s2, dword ofs);
	//Вычислить длинну совпадений
	static dword CompareChains(const byte * s1, const byte * s2, const byte * s2end);
	//Вычислить длинну совпадений при условии что в запасе есть 16 байт
	static dword CompareChains16(const byte * s1, const byte * s2, const byte * s2end);

private:
	byte * buffer;						//Буфер, в который складываем архивированное
	dword size;						//Текущий размер буфера
	dword streamPtr;					//Текущий битовый указатель
	Hash * hashNear;					//Хэшь-таблица для быстрогно поиска в ближней области
	Hash * hashFar;						//Хэшь-таблица для быстрогно поиска в дальней области

private:
	const byte * cur;					//Текущая позиция компрессии
	const byte * end;					//Указатель конца исходных данных
	const byte * endNearChains;				//Указатель конца исходных данных, когда ближние цепочки искать нет смысла
	const byte * endFarChains;				//Указатель конца исходных данных, когда дальние цепочки искать нет смысла
	const byte * optimalOffsetLong;				//Позиция оптимальной длинной цепочки
	dword optimalLenLong;					//Длинна оптимальной длинной цепочки
	const byte * optimalOffsetNear;				//Позиция оптимальной цепочки в ближней области
	dword optimalLenNear;					//Длинна оптимальной цепочки в ближней области
	dword unpackBookmark;					//Индекс счётчика несжатой цепочки в выходном потоке
	const byte * src;					//Начало исходных данных
};
