//===============================================================================================================================
// Spirenkov Maxim, 2009
//===============================================================================================================================

#include "XCompressor.h"

#ifdef GAME_RUSSIAN
#define XCompressor_USE_Difference_Codes
#pragma message("XCompressor_USE_Difference_Codes mode activate")
#endif

XCompressor::Chains::Chains()
{
	chains = null;
	windowMask = 0;
}

XCompressor::Chains::~Chains()
{
	if(chains) delete chains;
}

void XCompressor::Chains::Prepare(dword windowSizePow2)
{
	//Формируем маску по размеру
	XCompressorAssert((windowSizePow2 & (windowSizePow2 - 1)) == 0);
	dword m = windowSizePow2 - 1;
	//Выделяем буфер
	if(m > windowMask)
	{
		windowMask = m;
		if(chains)
		{
			delete chains;
			chains = null;
		}
	}
	if(!chains)
	{
		chains = NEW long[windowMask + 1];
	}
	for(dword i = 0; i <= windowMask; i++)
	{
		chains[i] = -1;
	}
	const dword entrySize = sizeof(entry)/sizeof(entry[0]);
	for(dword i = 0; i < entrySize; i++)
	{
		entry[i] = -1;
	}
}

dword XCompressor::Chains::CalcPow2WindowSize(dword size)
{
	if(size < 16) size = 16;
	if((size & (size - 1)) == 0) return size;
	dword m = 0x80000000;
	for(m; (m & size) == 0; m >>= 1);
	XCompressorAssert(m <= 0x1000000);
	XCompressorAssert(m >= 16);
	if(size & (m - 1)) m <<= 1;
	return m;
}



XCompressor::XCompressor()
{
	buffer = null;
	chains = null;
	size = 0;
	streamPtr = 0;
}


XCompressor::~XCompressor()
{
	if(buffer)
	{
		delete buffer;
		buffer = null;
	}
	if(chains)
	{
		delete chains;
		chains = null;
	}
}

/*

Несжатая запись
0000.LLLL дальше идут несжатые байты  1-16+, если 16 то есть продолжение
[code] [1]...[16] [count1 == 255] [1]..[255] [count2 == 255] [1]..[255] .... [countN < 255] [1]..[countN]

Кодируем длинную запись
0001.LLTT TTTT.TTTT TTTT.TTTT, длинна 3-6+ и смещение 4096 + 0..0x3ffff, если 6 то есть продолжение
[code1] [code2] [code3] [count1 == 255]..[countN < 255]

Кодируем короткую запись
LLLL.TTTT. TTTT.TTTT, длинна 2-15+ и смещение 1-4096, если 15 то есть продолжение
[code1] [code2] [count1 == 255]..[countN < 255]

Если есть продолжение, то после кода идёт байт с длинной следующей цепочки, если байт 0, то цепочка обрываеться, если 255, 
то будет следующий байт в цепочке с количеством.

*/

bool XCompressor::Compress(const byte * src, dword srcSize)
{
#ifndef STOP_DEBUG
	streamPtr = 0;
	//Если размер слишком маленький, непакуем
	if(srcSize < 4)
	{
		return false;
	}
	//Подготавливаем буфера
	if(size < srcSize)
	{
		delete buffer;
		buffer = null;
		size = srcSize + size/8 + 1024;
	}	
	if(!buffer)
	{
		buffer = NEW byte[size];
	}
	if(!chains)
	{
		chains = NEW Chains();
	}
	long maxOffset = 0;
	if(srcSize > c_smallOffsets)
	{
		static const dword windowSize = Chains::CalcPow2WindowSize(c_largeOffsets);
		maxOffset = c_largeOffsets;
		chains->Prepare(windowSize);
	}else{
		maxOffset = c_smallOffsets;
		chains->Prepare(c_smallOffsets);
	}
	//Цикл упаковки
	byte maxCount = 0;
	long unpackPoint = -1;
	long nextPosition = -1;
	long windowMask = chains->windowMask;
	dword i = 0;
	while(i < srcSize - 8)
	{
		if(streamPtr > srcSize)
		{
			streamPtr = 0;
			return false;
		}
		//Формирование цепочек хэша
		word index = *(word *)(src + i);
		long & entry = chains->entry[index];
		long & link = chains->chains[i & windowMask];
		link = entry;
		entry = (long)i;
		if((long)i < nextPosition)
		{
			//Данные упакованны, поэтому только заполненяем цепочки
			i++;
			continue;
		}
		//Проходим по доступтым цепочкам, выискивая самую длинную
		long chainLen = 2;
		long offset = -1;
		for(long pos = link, steps = 1; pos >= 0 && steps < c_maxLookupCount; pos = chains->chains[pos & windowMask], steps++)
		{
			//Если расстояние слишком большое, больше ничего не смотрим
			long ofs = i - pos;
			if(ofs > maxOffset) break;
			//Ищим 3х байтную цепочку
			if(src[i + 2] != src[pos + 2]) continue;
			chainLen = 3;
			offset = ofs;
			goto ContinueCheck4;
			for(; pos >= 0 && steps < c_maxLookupCount; pos = chains->chains[pos & windowMask], steps++)
			{
				//Если расстояние слишком большое, больше ничего не смотрим
				ofs = i - pos;
				if(ofs > maxOffset) break;
				//Ищим 4х байтную цепочку
ContinueCheck4:
				if((dword &)src[i] != (dword &)src[pos]) continue;
				chainLen = 4;
				offset = ofs;
				goto ContinueCheck5;
				for(; pos >= 0 && steps < c_maxLookupCount; pos = chains->chains[pos & windowMask], steps++)
				{
					//Если расстояние слишком большое, больше ничего не смотрим
					ofs = i - pos;
					if(ofs > maxOffset) break;
					//Если следующий байт за пределами цепочки другой, нет смысла чтото делать дальше
					if(src[i + chainLen] != src[pos + chainLen]) continue;
					if((dword &)src[i] != (dword &)src[pos]) continue;
ContinueCheck5:
					if(src[i + 4] != src[pos + 4]) continue;
					long clen;
					if(src[i + 5] != src[pos + 5]){ clen = 5; goto SkipCheckLoop; }
					if(src[i + 6] != src[pos + 6]){ clen = 6; goto SkipCheckLoop; }
					if(src[i + 7] != src[pos + 7]){ clen = 7; goto SkipCheckLoop; }
					clen = 6;
					dword cur = i + clen;
					dword from = pos + clen;
					while(cur < srcSize)
					{
						if(src[cur] != src[from])
						{
							break;
						}
						clen++;
						cur++;
						from++;
					}
SkipCheckLoop:
					if(chainLen < clen)
					{
						//Если цепочка длинней, то сохраняем новую
						chainLen = clen;
						offset = ofs;
						XCompressorAssert(i + chainLen <= srcSize);
						if(i + chainLen == srcSize)
						{
							break;
						}
					}
				}
			}
			break;
		}		
		//Отсекаем невыгодные решения
		if(offset <= c_smallOffsets)
		{
			if(chainLen <= 2)
			{
				while(true)
				{
					if(unpackPoint >= 0)
					{
						if(buffer[unpackPoint] != maxCount - 1)
						{
							chainLen = 0;
							break;
						}
					}
					if(offset < 0)
					{
						if(link >= 0)
						{
							offset = i - link;
							if(offset > c_smallOffsets)
							{
								chainLen = 0;
							}
						}else{
							chainLen = 0;
						}
					}
					break;
				}
			}
			if(chainLen)
			{
				//Сжатая цепочка с коротким кодом
				unpackPoint = -1;
				offset -= 1;
#ifndef XCompressor_USE_Difference_Codes
				//LLLL.TTTT. TTTT.TTTT, длинна 2-15+ и смещение 1-4096, если 15 то есть продолжение
				nextPosition = i + chainLen;
				if(chainLen < 15)
				{
					buffer[streamPtr++] = (byte)(((offset >> 8) & 0xf) | (chainLen << 4));
					buffer[streamPtr++] = (byte)(offset & 0xff);
				}else{
					buffer[streamPtr++] = (byte)((offset >> 8) & 0xf) | 0xf0;
					buffer[streamPtr++] = (byte)(offset & 0xff);
					chainLen -= 15;
					while(true)
					{
						if(chainLen < 255)
						{
							buffer[streamPtr++] = (byte)chainLen;
							if(streamPtr > srcSize)
							{
								streamPtr = 0;
								return false;
							}
							break;
						}else{
							buffer[streamPtr++] = 255;
							chainLen -= 255;
						}
					}
				}
#else
				//LLLL.TTTT. TTTT.TTTT, длинна 2-15 и смещение 1-4096
				if(chainLen > 15) chainLen = 15;
				nextPosition = i + chainLen;
				buffer[streamPtr++] = (byte)(((offset >> 8) & 0xf) | (chainLen << 4));
				buffer[streamPtr++] = (byte)(offset & 0xff);
#endif
				i++;
				continue;
			}
		}else{
			if(chainLen <= 2)
			{
				chainLen = 0;
			}else
			if(chainLen <= 3)
			{
				if(unpackPoint >= 0)
				{
					if(buffer[unpackPoint] != maxCount - 1)
					{
						chainLen = 0;
					}
				}
			}
			if(chainLen)
			{
				//Сжатая цепочка с длинным кодом
				unpackPoint = -1;
				offset -= c_smallOffsets;
#ifndef XCompressor_USE_Difference_Codes
				//0001.LLTT TTTT.TTTT TTTT.TTTT, длинна 3-6+ и смещение 4096 + 0..0x3ffff, если 6 то есть продолжение
				nextPosition = i + chainLen;
				if(chainLen < 6)
				{
					buffer[streamPtr++] = (byte)(0x10 | ((offset >> 16) & 0x3) | ((chainLen - 3) << 2));
					buffer[streamPtr++] = (byte)((offset >> 8) & 0xff);
					buffer[streamPtr++] = (byte)((offset >> 0) & 0xff);
				}else{
					buffer[streamPtr++] = (byte)(0x1c | ((offset >> 16) & 0x3));
					buffer[streamPtr++] = (byte)((offset >> 8) & 0xff);
					buffer[streamPtr++] = (byte)((offset >> 0) & 0xff);
					chainLen -= 6;
					while(true)
					{
						if(chainLen < 255)
						{
							buffer[streamPtr++] = (byte)chainLen;
							if(streamPtr > srcSize)
							{
								streamPtr = 0;
								return false;
							}
							break;
						}else{
							buffer[streamPtr++] = 255;
							chainLen -= 255;
						}
					}
				}
#else
				//0001.LLTT TTTT.TTTT TTTT.TTTT, длинна 3-6 и смещение 4096 + 0..0x3ffff
				if(chainLen > 6) chainLen = 6;
				nextPosition = i + chainLen;
				buffer[streamPtr++] = (byte)(0x10 | ((offset >> 16) & 0x3) | ((chainLen - 3) << 2));
				buffer[streamPtr++] = (byte)((offset >> 8) & 0xff);
				buffer[streamPtr++] = (byte)((offset >> 0) & 0xff);
#endif
				i++;
				continue;
			}
		}
		//Несжатая цепочка
		//0000.LLLL дальше идут несжатые байты  1-16+, если 16 то есть продолжение
		if(unpackPoint >= 0)
		{
			buffer[unpackPoint]++;
			buffer[streamPtr++] = src[i++];
			if(buffer[unpackPoint] == maxCount)
			{
				maxCount = 0xff;
				unpackPoint = streamPtr;
				buffer[streamPtr++] = 0;
			}
		}else{
			maxCount = 0x0f;
			unpackPoint = streamPtr;
			buffer[streamPtr++] = 0;
			buffer[streamPtr++] = src[i++];
		}
	}
	while(i < srcSize)
	{
		if((long)i < nextPosition)
		{
			i++;
			continue;
		}
		//Несжатая цепочка
		if(unpackPoint >= 0)
		{
			buffer[unpackPoint]++;
			buffer[streamPtr++] = src[i++];
			if(buffer[unpackPoint] == maxCount)
			{
				maxCount = 0xff;
				unpackPoint = streamPtr;
				buffer[streamPtr++] = 0;
			}
		}else{
			maxCount = 0x0f;
			unpackPoint = streamPtr;
			buffer[streamPtr++] = 0;
			buffer[streamPtr++] = src[i++];
		}
	}
	XCompressorAssert(i == srcSize);
	if(streamPtr < srcSize)
	{
		return true;
	}
	streamPtr = 0;
	return false;
#else
	streamPtr = 0;
	return false;
#endif
}


bool XCompressor::Decompress(const byte * src, dword srcSize, dword dstSize)
{
	if(size < dstSize)
	{
		delete buffer;
		buffer = null;
		size = dstSize;
	}
	streamPtr = 0;
	if(!buffer)
	{
		buffer = NEW byte[size];
	}
	if(Decompress(src, srcSize, buffer, dstSize))
	{
		streamPtr = dstSize;
		return true;
	}
	return false;
}

//Распаковать данные в данный буфер (рандомные чтения из буфера)
bool XCompressor::Decompress(const byte * src, dword srcSize, byte * dst, dword dstSize)
{
	if(srcSize < 2)
	{
		return false;	
	}
	const byte * s = src;
	const byte * stopSrc = src + srcSize;
	byte * startDst = dst;
	byte * stopDst = dst + dstSize;	
	while(s < stopSrc)
	{
		byte b = *s++;
		byte code = b & 0xf0;
		if(code == 0)
		{
			//Неупакованная цепочка
			//0000.LLLL дальше идут несжатые байты  1-16+, если 16 то есть продолжение
			dword count = (b & 0x0f) + 1;
			dword maxCount = 16;
			while(true)
			{
				if(dst + count > stopDst || s + count > stopSrc)
				{
					return false;
				}
				for(dword j = 0; j < count; j++)
				{
					*dst++ = *s++;
				}
				if(count != maxCount)
				{
					break;
				}else{
					count = *s++;
					maxCount = 255;
				}
			}
		}else{
			//Высчитываем смещение и количество байт
			dword offset = 0;
			dword count = 0;
			if(code != 0x10)
			{
				//Короткая цепочка
				//LLLL.TTTT. TTTT.TTTT, длинна 2-15+ и смещение 1-4096, если 15 то есть продолжение
				s++;
				if(s > stopSrc)
				{
					return false;
				}
				offset = ((((dword)b & 0x0f) << 8) | s[-1]) + 1;
				count = ((dword)b >> 4);
#ifndef XCompressor_USE_Difference_Codes
				if(count == 15)
				{
					while(true)
					{
						if(s >= stopSrc)
						{
							return false;
						}
						byte add = *s++;
						count += (dword)add;
						if(add != 255) break;
					}
				}
#endif
			}else{
				//Длинная цепочка
				//0001.LLTT TTTT.TTTT TTTT.TTTT, длинна 3-6+ и смещение 4096 + 0..0x3ffff, если 6 то есть продолжение
				s += 2;
				if(s > stopSrc) return false;
				offset = ((dword)b & 0x03) << 16;
				offset |= ((dword)s[-2]) << 8;
				offset |= ((dword)s[-1]) << 0;
				offset += c_smallOffsets;
				count = ((dword)(b & 0x0c) >> 2) + 3;
#ifndef XCompressor_USE_Difference_Codes
				if(count == 6)
				{
					while(true)
					{
						if(s >= stopSrc)
						{
							return false;
						}
						byte add = *s++;
						count += add;
						if(add != 255) break;
					}
				}
#endif
			}
			//Копируем необходимое количество байт
			const byte * from = dst - offset;
			if(from < startDst || dst + count > stopDst)
			{
				return false;
			}
			for(dword j = 0; j < count; j++)
			{
				*dst++ = *from++;
			}
		}
	}
	return true;
}

//Получить буффер
const byte * XCompressor::Buffer() const
{
	return buffer;
}

//Получить размер буфера
dword XCompressor::Size() const
{
	return streamPtr;
}


XStreamDecompressor::XStreamDecompressor()
{
	s = null;
	stopSrc = null;
	startSrc = null;
	destSize = 0;
	streamPtr = 0;
}

XStreamDecompressor::~XStreamDecompressor()
{
}

//Подготовиться к потоковой распаковке
bool XStreamDecompressor::Prepare(const byte * src, dword srcSize, dword dstSize)
{
	if(srcSize < 2)
	{
		return false;
	}
	s = src;
	stopSrc = src + srcSize;
	startSrc = src;
	destSize = dstSize;
	streamPtr = 0;
	continueMode = mode_start;
	return true;
}

//Потоковый распаковщик
bool XStreamDecompressor::Process(byte * dst, dword bytesForRead, dword & readCount)
{
	if(!dst || !bytesForRead) return 0;
	//Объём, который будем читать
	if(streamPtr + bytesForRead > destSize) bytesForRead = destSize - streamPtr;
	//Настраиваем продолжение распаковки
	dword startPtr = streamPtr;
	byte * stopDst = dst + bytesForRead;
	dword count = 0;
	dword maxCount = 0;
	dword from = 0;
	//Определяем точку входа
	if(continueMode == mode_unpack)
	{
		count = continueCount;
		maxCount = continueTmp;
		goto LabelContinueUnpack;
	}else
	if(continueMode == mode_pack)
	{
		count = continueCount;
		from = continueTmp;
		goto LabelContinuePack;
	}else
	if(continueMode != mode_start)
	{
		XCompressorAssert(false);
	}
	//Цикл распаковки
	while(s < stopSrc)
	{
		byte b = *s++;
		byte code = b & 0xf0;
		if(code == 0)
		{
			//Неупакованная цепочка
			//0000.LLLL дальше идут несжатые байты  1-16+, если 16 то есть продолжение
			count = (b & 0x0f) + 1;
			maxCount = 16;
			while(true)
			{
				if(streamPtr + count > destSize || s + count > stopSrc)
				{
					s = stopSrc;
					readCount = streamPtr - startPtr;
					return false;
				}
LabelContinueUnpack:
				for(dword j = 0; j < count; j++)
				{
					if(dst >= stopDst)
					{
						XCompressorAssert(dst == stopDst);
						XCompressorAssert(streamPtr - startPtr == bytesForRead);
						//Сохраняем контекст
						continueMode = mode_unpack;
						continueCount = count - j;
						continueTmp = count != maxCount ? maxCount : continueCount;
						readCount = bytesForRead;
						return true;
					}
					byte v = *s++;
					window[streamPtr & c_windowMask] = v;
					streamPtr++;
					*dst++ = v;
				}
				if(count != maxCount)
				{
					break;
				}else{
					count = *s++;
					maxCount = 255;
				}
			}
		}else{
			//Высчитываем смещение и количество байт
			dword offset = 0;
			count = 0;
			if(code != 0x10)
			{
				//Короткая цепочка
				//LLLL.TTTT. TTTT.TTTT, длинна 2-15+ и смещение 1-4096, если 15 то есть продолжение
				s++;
				if(s > stopSrc)
				{
					s = stopSrc;
					readCount = streamPtr - startPtr;
					return false;
				}
				offset = ((((dword)b & 0x0f) << 8) | s[-1]) + 1;
				count = ((dword)b >> 4);
#ifndef XCompressor_USE_Difference_Codes
				if(count == 15)
				{
					while(true)
					{
						if(s >= stopSrc)
						{
							s = stopSrc;
							readCount = streamPtr - startPtr;
							return false;
						}
						byte add = *s++;
						count += (dword)add;
						if(add != 255) break;
					}
				}
#endif
			}else{
				//Длинная цепочка
				//0001.LLTT TTTT.TTTT TTTT.TTTT, длинна 3-6+ и смещение 4096 + 0..0x3ffff, если 6 то есть продолжение
				s += 2;
				if(s > stopSrc)
				{
					s = stopSrc;
					readCount = streamPtr - startPtr;
					return false;
				}
				offset = ((dword)b & 0x03) << 16;
				offset |= ((dword)s[-2]) << 8;
				offset |= ((dword)s[-1]) << 0;
				offset += c_smallOffsets;
				count = ((dword)(b & 0x0c) >> 2) + 3;
#ifndef XCompressor_USE_Difference_Codes
				if(count == 6)
				{
					while(true)
					{
						if(s >= stopSrc)
						{
							s = stopSrc;
							readCount = streamPtr - startPtr;
							return false;
						}
						byte add = *s++;
						count += add;
						if(add != 255) break;
					}
				}
#endif
			}
			//Копируем необходимое количество байт
			from = streamPtr - offset;
			if((long)from < 0 || streamPtr + count > destSize)
			{
				s = stopSrc;
				readCount = streamPtr - startPtr;
				return false;
			}
LabelContinuePack:
			for(dword j = 0; j < count; j++)
			{
				if(dst >= stopDst)
				{
					XCompressorAssert(dst == stopDst);
					XCompressorAssert(streamPtr - startPtr == bytesForRead);
					//Сохраняем контекст
					continueMode = mode_pack;
					continueCount = count - j;
					continueTmp = from;
					readCount = bytesForRead;
					return true;
				}
				byte v = window[from & c_windowMask];
				window[streamPtr & c_windowMask] = v;
				from++;
				streamPtr++;
				*dst++ = v;
			}
		}
	}
	readCount = streamPtr - startPtr;
	return true;
}
