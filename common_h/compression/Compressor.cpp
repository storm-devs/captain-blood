//Spirenkov Max, 2008

#include "Compressor.h"


Compressor::Hash::Hash(dword winSize) : windowSize(winSize), windowMask(winSize - 1)
{
	window = NEW dword[windowSize];
}

Compressor::Hash::~Hash()
{
	delete window;
}

//Подготовить таблицу
__forceinline void Compressor::Hash::Prepare()
{
	dword initial = dword(-(long)windowSize);
	for(dword i = 0; i < Compressor::hash_size; i++)
	{
		entry[i] = initial;
	}
	for(dword i = 0; i < windowSize; i++)
	{
		window[i] = initial;
	}
}

//Добавить в текущую позицию хэшь
__forceinline void Compressor::Hash::Add(dword curPos, dword hash)
{
	dword & eh = entry[hash & Compressor::hash_mask];
	window[curPos & windowMask] = eh;
	eh = curPos;
}

//Найти первый элемент в списке
__forceinline dword Compressor::Hash::GetFirst(dword curPos, dword hash)
{
	dword address = entry[hash & Compressor::hash_mask];
	if(curPos - address > windowSize)
	{
		return -1;
	}
	return address;
}

//Получить следующий элемент в списке
__forceinline dword Compressor::Hash::GetNext(dword curPos, dword address)
{
	address = window[address & windowMask];
	if(curPos - address > windowSize)
	{
		return -1;
	}
	return address;
}


Compressor::Compressor()
{
	buffer = null;
	size = 0;
	streamPtr = 0;
	hashNear = null;
	hashFar = null;
}

Compressor::~Compressor()
{
	Free();
}


//Запокавать исходные данные
//Без хэшь-таблицы пакуеться упрощёным методом и не используеться дополнительной памяти
// Nikita: добавил объявление так что бы можно было защитить старфорсом (сказывается только на system.dll,
// поэтому будут "тормозить" с защитой только запись сейва
void __declspec(dllexport) Compressor::Compress(const byte * dataPtr, dword dataSize, CompressMethod method)
{
	//Подготавливаем буфер для данных
	streamPtr = 0;
	if(!dataSize)
	{
		return;
	}
	if(size < dataSize + dataSize/32)
	{
		if(buffer)
		{
			delete buffer;
		}
		size = (dataSize + dataSize/32 + 15) & ~15;
		buffer = NEW byte[size];
	}
	//Подготавливаем таблицы
	Prepare(method, false);
	if(hashNear)
	{
		hashNear->Prepare();
	}
	if(hashFar)
	{
		hashFar->Prepare();
	}
	//Подготавливаем поля
	cur = dataPtr;
	end = dataPtr + dataSize;
	endNearChains = end - 1;
	endFarChains = end - 6;
	unpackBookmark = -1;
	optimalLenLong = 0;
	optimalLenNear = 0;
	src = dataPtr;
	if(method == cm_maxcheck)
	{
		Encode(true);
		while(cur < end)
		{
			optimalLenLong = 0;
			optimalLenNear = 0;
			if(cur < endNearChains)
			{
				dword hash = CalcHash(cur, 2);
				FindChainUseHash(hashNear, hash);
				optimalLenNear = optimalLenLong;
				optimalOffsetNear = optimalOffsetLong;
				if(optimalLenLong < chain_len && cur < endFarChains)
				{
					hash = CalcHash(cur + 2, 4, hash);
					FindChainUseHash(hashFar, hash);
				}
			}
			Encode(true);
			ReserveBuffer();
		}
	}else
	if(method == cm_skip)
	{
		Encode(false);
		while(cur < end)
		{
			optimalLenLong = 0;
			optimalLenNear = 0;
			if(cur < endNearChains)
			{
				dword hash = CalcHash(cur, 2);
				FindChainUseHash(hashNear, hash);
				optimalLenNear = optimalLenLong;
				optimalOffsetNear = optimalOffsetLong;
				if(optimalLenLong < chain_len && cur < endFarChains)
				{
					hash = CalcHash(cur + 2, 4, hash);
					FindChainUseHash(hashFar, hash);
				}
			}
			Encode(false);
			ReserveBuffer();
		}
	}else
	if(method == cm_fast)
	{
		Encode(true);
		while(cur < end)
		{
			optimalLenLong = 0;
			optimalLenNear = 0;
			if(cur < endNearChains)
			{
				dword hash = CalcHash(cur, 2);
				FindChainUseHash(hashNear, hash);
				optimalLenNear = optimalLenLong;
				optimalOffsetNear = optimalOffsetLong;
			}
			Encode(true);
			ReserveBuffer();
		}
	}else
	if(method == cm_nousememory)
	{
		Encode(false);
		while(cur < end)
		{
			optimalLenLong = 0;
			optimalLenNear = 0;
			FindChainWithoutHash();
			Encode(false);
			ReserveBuffer();
		}
	}
}

//Получить указатель на буфер с данными
const byte * Compressor::Buffer()
{
	return buffer;
}

//Получить размер данных
dword Compressor::Size()
{
	return streamPtr;
}

//Распаковать данные
Compressor::DecReturn Compressor::Decompress(const byte * src, dword ssize, byte * dst, dword & dsize)
{
	//Проверяем входные данные
	if(!src || !ssize)
	{
		dsize = 0;
		return dr_no_source;
	}
	if(!dst || !dsize)
	{
		dsize = 0;
		return dr_buffer_is_small;
	}
	//Начинаем распаковку
	byte * start = dst;
	byte * endOfBuffer = dst + dsize;
	const byte * endOfData = src + ssize;
	dword count, offset;
	while(src < endOfData)
	{
		if(*src)
		{
			if((*src & 0xf0) == 0)
			{
				//Копируем символы
				long count = (*src++ & 0xf);
				byte * eob = dst + count;
				if(src + count <= endOfData && eob <= endOfBuffer)
				{
					//Всё нормально, просто копируем
					do{ *dst++ = *src++; }while(dst < eob);
				}else{
					//Что то не так
					while(true)
					{
						if(src >= endOfData)
						{
							//Ошибка исходного файла
							dsize = dst - start;
							return dr_damage_source;
						}
						if(dst >= endOfBuffer)
						{
							//Закончилось место
							dsize = dst - start;
							return dr_buffer_is_small;
						}
						*dst++ = *src++;
					}
				}
				continue;
			}else{
				//Вставляем цепочку читая короткую запись
				if(src + 1 > endOfData)
				{
					//Ошибка исходного файла
					dsize = dst - start;
					return dr_damage_source;
				}
				count = ((src[0] & 0xf0) >> 4) + 1;
				offset = (src[0] & 0x0f) | (((dword)src[1]) << 4);
				src += 2;
			}
		}else{
			//Вставляем цепочку читая длинную запись
			if(src + 5 > endOfData)
			{
				//Ошибка исходного файла
				dsize = dst - start;
				return dr_damage_source;
			}
			count = (src[1] | (dword(src[2] & 0x0f) << 8)) + 6;
			offset = (src[2] >> 4) & 0x0f;
			offset |= dword(src[3]) << 4;
			offset |= dword(src[4]) << 12;
			src += 5;
		}
		byte * chain = dst - 1 - offset;
		if(chain < start)
		{
			//Ошибка исходного файла
			dsize = dst - start;
			return dr_damage_source;
		}
		byte * eob = dst + count;
		if(eob <= endOfBuffer)
		{
			do{ *dst++ = *chain++; }while(dst < eob);
		}else{
			//Закончилось место
			while(dst < endOfBuffer){ *dst++ = *chain++; }
			dsize = dst - start;
			return dr_buffer_is_small;
		}
	}
	dsize = dst - start;
	return dr_ok;
}

//Подготовить таблицы для данного метода сжатия
void Compressor::Prepare(CompressMethod method, bool isReleaseBuffers)
{
	switch(method)
	{
	case cm_maxcheck:
	case cm_skip:
		if(!hashNear)
		{
			hashNear = NEW Hash(window_size_near);
		}
		if(!hashFar)
		{
			hashFar = NEW Hash(window_size_far);
		}
		break;
	case cm_fast:
		if(!hashNear)
		{
			hashNear = NEW Hash(window_size_near);
		}
		if(isReleaseBuffers)
		{
			if(hashFar)
			{
				delete hashFar;
				hashFar = null;
			}			
		}
		break;
	case cm_nousememory:
		if(isReleaseBuffers)
		{
			if(hashNear)
			{
				delete hashNear;
				hashNear = null;
			}
			if(hashFar)
			{
				delete hashFar;
				hashFar = null;
			}
		}
		break;
	}
}

//Удалить все используемые ресурсы
void Compressor::Free()
{
	Prepare(cm_nousememory, true);
	if(buffer)
	{
		delete buffer;
		buffer = null;
	}
	size = 0;
	streamPtr = 0;
}

//Кодировать текущую информацию исходя из найденных цепочек
__forceinline void Compressor::Encode(const bool updateHash)
{
	if(optimalLenLong < 6) optimalLenLong = 0;
	if(optimalLenNear < 2) optimalLenNear = 0;
	if(optimalLenNear | optimalLenLong)
	{
		//Оцениваем как кодировать цепочку
		if(optimalLenNear > 16) optimalLenNear = 16;
		if(optimalLenLong > chain_len) optimalLenLong = chain_len;
		if(optimalLenNear*5 >= 2*optimalLenLong)
		{
			if(optimalLenNear > 2 || (unpackBookmark != -1 && buffer[unpackBookmark] >= 14))
			{
				//Кодируем короткую запись
				// LLLL.TTTT. TTTT.TTTT, длинна 1-15 (2-16) и смещение 0-4095 (1-4096)
				dword offset = cur - optimalOffsetNear - 1;
				dword len = optimalLenNear - 1;
				buffer[streamPtr + 0] = byte((len << 4) & 0xf0) | byte(offset & 0x0f);
				buffer[streamPtr + 1] = byte(offset >> 4);
				streamPtr += 2;
				//Смещаем текущий указатель на данное количество байт
				if(updateHash)
				{
					SkipBytes(optimalLenNear);
				}else{
					cur += optimalLenNear;
				}
				//Прекращаем непрерывную цепочку несжатых символов
				unpackBookmark = -1;
				return;
			}
		}else{
			//Кодируем длинную запись
			// 0000.0000 LLLL.LLLL LLLL.TTTT TTTT.TTTT TTTT.TTTT
			// длинна 0-4095 (6-4101) и смещение 0-1048575 (1-1048576)
			dword offset = cur - optimalOffsetLong - 1;
			dword len = optimalLenLong - 6;
			buffer[streamPtr + 0] = 0;
			buffer[streamPtr + 1] = byte(len);
			buffer[streamPtr + 2] = byte((offset << 4) & 0xf0) | byte((len >> 8) & 0x0f);
			buffer[streamPtr + 3] = byte(offset >> 4);
			buffer[streamPtr + 4] = byte(offset >> 12);
			streamPtr += 5;
			//Смещаем текущий указатель на данное количество байт
			if(updateHash)
			{
				SkipBytes(optimalLenLong);
			}else{
				cur += optimalLenLong;
			}
			//Прекращаем непрерывную цепочку несжатых символов
			unpackBookmark = -1;
			return;
		}
	}
	//Добавляем оригинальный символ
	if(unpackBookmark != -1)
	{
		if(buffer[unpackBookmark] < 15)
		{
			buffer[unpackBookmark]++;
		}else{
			unpackBookmark = streamPtr++;
			buffer[unpackBookmark] = 1;
		}
	}else{
		unpackBookmark = streamPtr++;
		buffer[unpackBookmark] = 1;
	}
	buffer[streamPtr++] = *cur++;
}

//Зарезервировать место в буфере если нужно
__forceinline void Compressor::ReserveBuffer()
{
	//Проверяем что достаточно места в буфере
	if(size - streamPtr < 32)
	{
		size = (size + size/8 + 1024 + 15) & ~15;
		byte * b = NEW byte[size];
		memcpy(b, buffer, streamPtr);
		delete buffer;
		buffer = b;
	}
}


//Просмотр цепочек
__forceinline void Compressor::FindChainUseHash(Hash * hashTable, dword hashValue)
{
	dword curPos = cur - src;
	dword chainBase = hashTable->GetFirst(curPos, hashValue);
	if(end - cur >= 16)
	{
		while(chainBase != -1)
		{
			//Определяем длинну совпадающей цепочки
			const byte * chain = src + chainBase;
			const byte * hotTest = cur + optimalLenLong;
			if(hotTest < end && *hotTest == chain[optimalLenLong])
			{
				dword len = CompareChains16(chain, cur, end);
				//Определяем оптимальность цепочки
				if(optimalLenLong < len)
				{
					optimalLenLong = len;
					optimalOffsetLong = chain;
					if(len >= chain_len)
					{
						break;
					}
				}
			}
			chainBase = hashTable->GetNext(curPos, chainBase);
		}
	}else{
		while(chainBase != -1)
		{
			//Определяем длинну совпадающей цепочки
			const byte * chain = src + chainBase;
			const byte * hotTest = cur + optimalLenLong;
			if(hotTest < end && *hotTest == chain[optimalLenLong])
			{
				dword len = CompareChains(chain, cur, end);
				//Определяем оптимальность цепочки
				if(optimalLenLong < len)
				{
					optimalLenLong = len;
					optimalOffsetLong = chain;
					if(len >= chain_len)
					{
						break;
					}
				}
			}
			chainBase = hashTable->GetNext(curPos, chainBase);
		}
	}
	hashTable->Add(curPos, hashValue);
}

//Просмотр цепочек прямым перебором
__forceinline void Compressor::FindChainWithoutHash()
{
	//Кодируем только короткие цепочки в маленьком окне
	bool isUse16Test = (end - cur >= 16);
	const byte * window = cur - window_size_near;
	if(window < src) window = src;
	//Необходимо чтобы текущая позиция уже не была равна начальной (src)
	for(const byte * chain = cur - 1; chain >= window; chain--)
	{
		dword len = isUse16Test ? CompareChains16(chain, cur, end) : CompareChains(chain, cur, end);
		if(optimalLenLong < len)
		{
			optimalLenLong = len;
			optimalOffsetLong = chain;
		}
	}
	optimalLenNear = optimalLenLong;
	optimalOffsetNear = optimalOffsetLong;
}

//Пропустить столько байт, занеся их в хэшь-таблицу
__forceinline void Compressor::SkipBytes(dword count)
{
	for(cur++, count--; count != 0; count--, cur++)
	{
		dword curPos = cur - src;
		dword hash = CalcHash(cur, 2);
		hashNear->Add(curPos, hash);
		if(hashFar)
		{
			hash = CalcHash(cur + 2, 4, hash);
			hashFar->Add(curPos, hash);
		}
	}
}


//Расчитать индекс для вхождения в хэшь таблицу
__forceinline dword Compressor::CalcHash(const byte * ptr, dword count, dword hash)
{
	for(; count; count--, ptr++)
	{
		hash ^= ((hash << 5) + *ptr + (hash >> 2));
	}
	return hash;
}

//Выяснить сколько байт равно и вернуть соответствующие количество (dword неравны)
__forceinline dword Compressor::CompareResidue(const byte * s1, const byte * s2, dword ofs)
{
	if((__int16 &)s1[ofs] != (__int16 &)s2[ofs])
	{
		if(s1[ofs] != s2[ofs])
		{
			return ofs + 0;
		}else{
			return ofs + 1;
		}
	}else{
		if(s1[ofs + 2] != s2[ofs + 2])
		{
			return ofs + 2;
		}else{
			return ofs + 3;
		}
	}
}

//Вычислить длинну совпадений
__forceinline dword Compressor::CompareChains(const byte * s1, const byte * s2, const byte * s2end)
{
	const byte * start = s2;
	const byte * end = s2 + ((s2end - s2) & ~3);
	for(; s2 < end; s1 += 4, s2 += 4)
	{
		if(*(__int32 *)s1 != *(__int32 *)s2)
		{
			return CompareResidue(s1, s2, 0) + (s2 - start);
		}
	}
	for(; s2 < s2end; s1++, s2++)
	{
		if(*s1 != *s2)
		{
			break;
		}
	}
	return s2 - start;
}

//Вычислить длинну совпадений при условии что в запасе есть 16 байт
__forceinline dword Compressor::CompareChains16(const byte * s1, const byte * s2, const byte * s2end)
{
	//Проверяем первые 16 байт
	if((__int32 &)s1[0*4] == (__int32 &)s2[0*4])
	{			
		if((__int32 &)s1[1*4] == (__int32 &)s2[1*4])
		{
			if((__int32 &)s1[2*4] == (__int32 &)s2[2*4])
			{
				if((__int32 &)s1[3*4] == (__int32 &)s2[3*4])
				{
					//Допроверяем оставшийся хвост
					return CompareChains(s1 + 16, s2 + 16, s2end) + 16;
				}else{
					return CompareResidue(s1, s2, 3*4);
				}
			}else{
				return CompareResidue(s1, s2, 2*4);
			}
		}else{
			return CompareResidue(s1, s2, 1*4);
		}
	}
	return CompareResidue(s1, s2, 0*4);
}
