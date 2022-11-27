//===============================================================================================================================
// Spirenkov Maxim, 2009
//===============================================================================================================================

#include "..\core.h"

#define XCompressorAssert(exp) Assert(exp)
//#define XCompressorAssert(exp) {0;};

class XCompressor
{
	enum Consts
	{
		c_smallOffsets = 0x1000,
		c_largeOffsets = 0x3ffff + c_smallOffsets,
		c_maxLookupCount = 20,
	};

private:
	struct Chains
	{
		Chains();
		~Chains();

		//Выделить память под окно
		void Prepare(dword windowSizePow2);
		//Расчитать размер окна для произвольного размера
		static dword CalcPow2WindowSize(dword size);

		long * chains;			//Буфер связанных списков одинаковых word
		dword windowMask;		//Маска окна, кратная степени 2
		long entry[65536];		//Последний word в цепочке
	};

public:
	XCompressor();
	~XCompressor();

public:	
	//Если возвращает false, то значит данные сжимаються в больший размер и буфер несформирован
	bool Compress(const byte * src, dword srcSize);
	//Распаковать данные во внутренний буфер
	bool Decompress(const byte * src, dword srcSize, dword dstSize);
	//Распаковать данные в данный буфер (выполняються рандомные чтения из буфера dst)
	static bool Decompress(const byte * src, dword srcSize, byte * dst, dword dstSize);

public:
	//Получить буффер
	const byte * Buffer() const;
	//Получить размер буфера
	dword Size() const;

private:
	byte * buffer;			//Буфер, в который складываем данные
	dword size;			//Текущий размер буфера
	dword streamPtr;		//Текущий битовый указатель
	Chains * chains;		//Цепочки word
};

//Потоковый распаковщик
class XStreamDecompressor
{
	enum Consts
	{
		c_windowSize = 0x80000,
		c_windowMask = c_windowSize - 1,
		c_smallOffsets = 0x1000,
		
		mode_start = 0,
		mode_unpack = 1,
		mode_pack = 2,
		mode_work = 3,
	};
public:
	XStreamDecompressor();
	~XStreamDecompressor();

	//Подготовиться к потоковой распаковке
	bool Prepare(const byte * src, dword srcSize, dword dstSize);
	//Потоковый распаковщик
	bool Process(byte * dst, dword bytesForRead, dword & readCount);

private:
	const byte * s;				//Текущая позиция источника
	const byte * stopSrc;			//Указатель, когда останавливаем распаковку
	const byte * startSrc;			//Источник данных для потоковой распаковки
	dword streamPtr;			//Текущее положение указателя в выходном потоке
	dword destSize;				//Количество байт в распакованном файле для потоковой распаковки	
	dword continueMode;
	dword continueCount;
	dword continueTmp;
	byte window[c_windowSize];		//Окно
};
