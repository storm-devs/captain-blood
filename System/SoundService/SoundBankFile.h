
#ifndef _SoundBankFile_h_
#define _SoundBankFile_h_

#include "..\..\common_h\core.h"
#include "..\..\Common_h\data_swizzle.h"

#ifdef _XBOX
#include <Audiodefs.h>
#endif

//===========================================================================================================================
//
//	SoundBankId
//  xbox part---------------------------
//	SoundBankFileHeader
//	SoundBankFileSound[SoundBankFileHeader.soundsCount]
//	SoundBankFileWaveInfo[SoundBankFileHeader.winfosCount]
//  SoundBankFileWave[SoundBankFileHeader.wavesCount]
//	SoundBankFileSound * entry[SoundBankFileHeader.mask + 1]
//	SoundBankFileExtra[SoundBankFileHeader.extrasCount]
//  SoundBankFileWaveQueue data
//	Names binary data
//	Extra binary data
//	WAVEFORMATEX data
//  [UINT32]
//	Waves binary data
//  ---------------------------
//  pc part-----------------------------
//	тоже что и в xbox только в порядке от меньшего к большему и волны в формате для РС
//
//	SoundBankFileId[SoundBankFileHeader.idsCount] (только PC)
//===========================================================================================================================


__forceinline void SoundBankFile_Prepare4byteAlignedSwizzler(void * ptr, dword bytes, bool isNeedSwizzle)
{
	//Порядок байт в файле задан от старшего к младшему
	if(!isNeedSwizzle) return;
	Assert((bytes & 3) == 0);
	dword * p = (dword *)ptr;
	for(dword * pe = p + (bytes >> 2); p < pe; p++)
	{
		__RefDataSwizzler(*p);
	}
}

template<class T> __forceinline bool SoundBankFile_RestorePointer(T * & aptr, byte * dataStart, dword size)
{
	if(aptr)
	{
		if((((byte *)aptr) - ((byte *)null)) >= (long)size)
		{
			return false;
		}
		aptr = (T *)((((byte *)aptr) - ((byte *)null)) + dataStart);
	}
	return true;
}


template<class T> __forceinline void SoundBankFile_PreparePointer(T * & aptr, byte * dataStart)
{
	if(aptr)
	{
		aptr = (T *)(((byte *)aptr) - dataStart);
	}
}

//Идентификаторы типа расширенных данных
enum SoundBankFileExtraId
{
	sbf_extra_phonemes = 1,		   //Фонемы
};

//Заголовок указывающий на расширенные данные
struct SoundBankFileExtra
{
	dword id;						//Идентификатор расширенных данных (SoundBankFileExtraId)
	byte * data;					//Указатель где данные распологаються

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		if(!SoundBankFile_RestorePointer(data, dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart, bool isNeedSwizzle)
	{
		SoundBankFile_PreparePointer(data, dataStart);
		SoundBankFile_Prepare4byteAlignedSwizzler(this, sizeof(SoundBankFileExtra), isNeedSwizzle);
	}
};

//Уникальный идентификатор для настройки (есть только в PC версии)
struct SoundBankFileObjectId
{
	byte * objectPtr;		//Объект, которому принадлежит идентификатор
	dword data[4];			//Идентификатор объекта
	

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		if(!SoundBankFile_RestorePointer(objectPtr, dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart, bool isNeedSwizzle)
	{
		SoundBankFile_PreparePointer(objectPtr, dataStart);
		Assert(!isNeedSwizzle);
	}

	bool IsEqual(dword id[4])
	{
		dword res = (id[0] ^ data[0]) | (id[1] ^ data[1]) | (id[2] ^ data[2]) | (id[3] ^ data[3]);
		return res == 0;
	}
};

//Базовые параметры звука
struct SoundBankFileSetup
{
	enum Modes
	{
		//Приоритет
		mode_priority_base = 0x1000,		//Базовый приоритет для редактора
		mode_priority_range = 0x2000,		//Диапазон приоритетов для редактора
		mode_priority_mask = 0xffff,		//Приоритет по отношению к остальным звукам, больше число - выше приоритет
		//Методы выбора волны
		mode_select_rnd = 0x0000,			//Выбирать волны случайным образом
		mode_select_queue = 0x10000,		//Выбирать волны случайно, избегая повторений с учётом весов
		mode_select_sequence = 0x20000,		//Выбирать волны последовательно как они заданны
		mode_select_mask = 0x30000,
		//Зацикленные звуки
		mode_loop_diasble = 0x0000,			//Звук не зациклен
		mode_loop_one_wave = 0x40000,		//Отыгрывать зацикленно звук с 1й волной
//		mode_loop_sel_wave_rnd = 0x80000,	//Отыгрывать зацикленно звук каждый раз выбирая новую волну как в mode_select_rnd
//		mode_loop_sel_wave_que = 0xc0000,	//Отыгрывать зацикленно звук каждый раз выбирая новую волну как в mode_select_queue
		mode_loop_mask = 0xc0000,
		//Какие эффекты применять к звуку
		mode_fx_full = 0x000000,			//Применять все эффекты
		mode_fx_premaster = 0x100000,		//Пропусить эфект окружающей среды
		mode_fx_master = 0x200000,			//Пропусить эфект окружающей среды и премастера
		mode_fx_music = 0x300000,			//Музыка
		mode_fx_mask = 0x300000,
	};

#ifndef GAME_RUSSIAN
	dword maxCount;					//Максимальное количество одновременно проигрываемых данных звуков, 0 неограничено
	dword mode;						//Режим использования звука
#else
	dword mode;						//Режим использования звука
	dword maxCount;					//Максимальное количество одновременно проигрываемых данных звуков, 0 неограничено	
#endif

	//Получить приоритет звука
	__forceinline dword GetPriority()
	{
		return mode & mode_priority_mask;
	}

	//Получить режим выбора волны
	__forceinline dword GetModeSelect()
	{
		return mode & mode_select_mask;
	}

	//Получить режим выбора волны
	__forceinline dword GetModeLoop()
	{
		return mode & mode_loop_mask;
	}

	//Определить какаие эффекты применять для звука
	__forceinline dword GetFxMode()
	{
		return mode & mode_fx_mask;
	}

};

//Параметры затухания звука от дистанции
struct SoundBankFileAttenuation
{
	float c[4];						//Коэфициенты графика затухания
#ifndef GAME_RUSSIAN
	float minDist2;					//Минимальныя дистанция в квадрате
	float maxDist2;					//Максимальныя дистанция в квадрате
	float kNorm2;					//Коэфициент нормализации в квадрате 1.0/(maxDist2 - minDist2)
#else
#ifndef GAME_DEMO
	float kNorm2;					//Коэфициент нормализации в квадрате 1.0/(maxDist2 - minDist2)
	float minDist2;					//Минимальныя дистанция в квадрате	
	float maxDist2;					//Максимальныя дистанция в квадрате
#else
	float minDist2;					//Минимальныя дистанция в квадрате
	float kNorm2;					//Коэфициент нормализации в квадрате 1.0/(maxDist2 - minDist2)
	float maxDist2;					//Максимальныя дистанция в квадрате
#endif
#endif

	//Расчитать затухание для звука в локальной системе слушателя
	__forceinline float Attenuation(float dist2)
	{
		if(dist2 >= minDist2)
		{
			if(dist2 < maxDist2)
			{
				float k = (dist2 - minDist2)*kNorm2;
				float att = AttenuationBySpline(k, c[0], c[1], c[2], c[3]);
				if(att < 0.0f) att = 0.0f;
				if(att > 1.0f) att = 1.0f;
				return att;
			}
			return 0.0f;
		}
		return 1.0f;
	}
};

//Физическое представление волны
struct SoundBankFileWave
{
	enum Format
	{
		f_format_error = 0x00000000,//Такого быть не должно
		f_freq_mask =    0x0001ffff,//Маска для описания частоты дискретизации		
		f_format_pcm =   0x00100000,//Не кодированый формат, 16 бит
		f_format_xma =   0x00200000,//Архивированый формат xma
		f_format_xvma =  0x00300000,//Архивированый формат xvma
		f_format_mask =  0x00700000,//Маска для получения формата кодирования
		f_quality_mask = 0x3f000000,//Маска для получения качества сжатия 0-минимальное качество, 63-максимальное качество
		f_quality_shift = 24,		//Сдвиг для получения качества сжатия
		f_stereo =       0x00020000,//Стерео или моно
		f_tmp_music =    0x80000000,//Это музыка, не сохраняемый в файле флаг (дублирует mode_fx_music)
	};

	struct XWMAWAVEFORMAT
	{
		WAVEFORMATEX format;
		dword tableCount;
		dword table[1];
	};

#ifndef GAME_RUSSIAN
	const byte * data;				//Звуковые данные волны
	dword dataSize;					//Размер звуковых данных в байтах
	dword samplesCount;				//Количество сэмплов в волне	
	dword format;					//Внутреннее описание формата волн
	float unimportantTime;			//Время малозначительного остатка, когда звук можно проигнорировать
	float maxNormalizedAmp;			//Максимальная нормализованная амплитуда от 0 до 1
	const byte * waveFormatInfo;	//Структура описывающая формат волны WAVEFORMATEX или XMA2WAVEFORMAT или XWMAWAVEFORMAT
	SoundBankFileExtra * extraData;	//Расширенные данные в таблице расширений
	dword extraCount;				//Количество расширенных данных
#else
#ifndef GAME_DEMO
	const byte * data;				//Звуковые данные волны
	dword dataSize;					//Размер звуковых данных в файтах
	dword samplesCount;				//Количество сэмплов в волне	
	dword format;					//Внутреннее описание формата волн
	const byte * waveFormatInfo;	//Структура описывающая формат волны WAVEFORMATEX или XMA2WAVEFORMAT или XWMAWAVEFORMAT	
	float maxNormalizedAmp;			//Максимальная нормализованная амплитуда от 0 до 1	
	float unimportantTime;			//Время малозначительного остатка, когда звук можно проигнорировать	
	SoundBankFileExtra * extraData;	//Расширенные данные в таблице расширений
	dword extraCount;				//Количество расширенных данных	
#else
	dword format;					//Внутреннее описание формата волн	
	dword dataSize;					//Размер звуковых данных в файтах
	dword samplesCount;				//Количество сэмплов в волне	
	const byte * waveFormatInfo;	//Структура описывающая формат волны WAVEFORMATEX или XMA2WAVEFORMAT или XWMAWAVEFORMAT
	SoundBankFileExtra * extraData;	//Расширенные данные в таблице расширений	
	float maxNormalizedAmp;			//Максимальная нормализованная амплитуда от 0 до 1
	const byte * data;				//Звуковые данные волны
	float unimportantTime;			//Время малозначительного остатка, когда звук можно проигнорировать
	dword extraCount;				//Количество расширенных данных
#endif
#endif
	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{		
		if(!SoundBankFile_RestorePointer(data, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(waveFormatInfo, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(extraData, dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart, bool isNeedSwizzle)
	{
		SoundBankFile_PreparePointer(data, dataStart);
		SoundBankFile_PreparePointer(waveFormatInfo, dataStart);
		SoundBankFile_PreparePointer(extraData, dataStart);
		SoundBankFile_Prepare4byteAlignedSwizzler(this, sizeof(SoundBankFileWave), isNeedSwizzle);
	}

#ifdef GAME_RUSSIAN


	//Кодирование волн. Если будут проблемы, то закаментировать содержимое

	__forceinline void EncodeWaveData()
	{
		if(data)
		{
			dword magic = 0;
			dword count = 0;
			byte xcode = 0;
			EncodeInit(magic, count, xcode);
			byte * d = (byte *)data;
			for(dword i = 0; i < dataSize; i++)
			{
				d[i] ^= xcode;
				if(count == 0)
				{
					if(d[i] == 0)
					{
						EncodeStep(magic, count, xcode);
					}
				}else{
					count--;
				}
			}
		}
	}


#pragma optimize("", off)
	void DecodeWaveData()
	{
		if(data)
		{
			dword magic = *(dword *)api->Storage().GetString("system.core.id", "    ");
			dword count = dataSize;
			byte xcode = 13;
			byte * d = (byte *)data;
			EncodeInit(magic, count, xcode);
			for(dword i = 0; i < dataSize; i++)
			{
				d[i] ^= xcode;
				if(count == 0)
				{
					if(d[i] == xcode)
					{
						EncodeStep(magic, count, xcode);
					}
				}else{
					count--;
				}
			}
		}
	}

	void EncodeInit(dword & magic, dword & count, byte & xcode)
	{
		magic = 18532;
		count = 341;
		xcode = byte(magic);
	}


	void EncodeStep(dword & magic, dword & count, byte & xcode)
	{
		dword cur = magic;
		magic = ((cur + 1253947)*112492*cur + 84723) >> 5;
		xcode = byte(magic);
		count = ((magic + 43)*(magic + 11)) & 1023;
		if(count > 923) count = 1374;
		if(count < 157) count = 232;
	}
#pragma optimize("", on)
#endif
};

//Описание волны принадлежащей звуку
struct SoundBankFileWaveInfo
{
#ifndef GAME_RUSSIAN
	float volume;					//Громкость с какой проигрывать волну
	float probability;				//Вероятность выбора волны
	float playTime;					//Время активности волны (время проигрывание волны или тишины)
	SoundBankFileWave * wave;		//Данные волны
#else
#ifndef GAME_DEMO
	SoundBankFileWave * wave;		//Данные волны
	float volume;					//Громкость с какой проигрывать волну
	float probability;				//Вероятность выбора волны
	float playTime;					//Время активности волны (время проигрывание волны или тишины)	
#else
	float volume;					//Громкость с какой проигрывать волну
	float probability;				//Вероятность выбора волны
	SoundBankFileWave * wave;		//Данные волны
	float playTime;					//Время активности волны (время проигрывание волны или тишины)	
#endif
#endif
	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		if(!SoundBankFile_RestorePointer(wave, dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart, bool isNeedSwizzle)
	{
		SoundBankFile_PreparePointer(wave, dataStart);
		SoundBankFile_Prepare4byteAlignedSwizzler(this, sizeof(SoundBankFileWaveInfo), isNeedSwizzle);
	}
};

//Очередь для выбора волн
struct SoundBankFileWaveQueue
{
	struct Wave
	{
		float weight;		//Текущий вес волны
	};

	static __forceinline dword GetSize(dword wavesCount)
	{
		return sizeof(SoundBankFileWaveQueue) + sizeof(Wave)*(wavesCount - 1);
	}

	//Сделать выборку волны из очереди
	__forceinline dword Select(SoundBankFileWaveInfo * waves, dword wavesCount)
	{
		//Считаем сумарный вес
		float totalWeight = 0.0f;
		for(dword i = 0; i < wavesCount; i++)
		{
			if(wave[i].weight <= 0.0f) continue;
			totalWeight += wave[i].weight;
		}
		//Если необходимо, обнавляем волны
		while(totalWeight <= 0.0001f)
		{
			//Инициализируем пустую таблицу			
			totalWeight = 0.0f;
			for(dword i = 0; i < wavesCount; i++)
			{
				wave[i].weight += waves[i].probability;
				totalWeight += wave[i].weight;
			}
		}
		Assert(totalWeight > 0.0f);
		long last = -1;
		float rnd = Rnd(totalWeight);
		float cur = 0.0f;		
		for(dword i = 0; i < wavesCount; i++)
		{
			Wave & w = wave[i];
			if(w.weight <= 0.0f)
			{
				//Пропускаем выпавшие из очереди волны
				continue;
			}
			last = i;
			cur += w.weight;
			if(cur >= rnd)
			{
				//Правим веса
				w.weight -= minWeight;
				return i;
			}
		}
		return 0;
	}

	dword count;			//Текущее количество элементов в массиве (максимум SoundBankFileSound::wavesCount)
	float minWeight;		//Минимальный вес в таблице волн
	float totalWeight;		//Сумарный вес
	Wave wave[1];			//Масив элементов для выбора волны
};

//Описание звука
struct SoundBankFileSound
{
#ifndef GAME_RUSSIAN
	SoundBankFileWaveInfo * waves;	//Индекс начала таблицы волн этого звука
	dword wavesCount;				//Количество волн приписаных звуку
	dword playSoundsCount;			//Количество звуков проигрываемых в текущий момент
	void * playSoundsList;			//Вхождение в список проигрываемых звуков
	union
	{
	SoundBankFileWaveQueue * squeue;//Очередь для случайного выбора волн при проигрывании звука
	dword selectSequenceCount;		//Счётчик для последовательного выбора волн
	};
//	SoundBankFileWaveQueue * lqueue;//Очередь для случайного выбора волн зацикленного звука
	const char * name;				//Имя звука
	dword nameHash;					//Хэшь значение имени
	dword nameLen;					//Длинна имени
	SoundBankFileSound * next;		//Следующий в цепочке поиска
	SoundBankFileExtra * extraData;	//Расширенные данные в таблице расширений
	dword extraCount;				//Количество расширенных данных
	SoundBankFileSetup setup;		//Базовые настройки звука
	SoundBankFileAttenuation att;	//Параметры для 3D звука
#else
#ifndef GAME_DEMO
	void * playSoundsList;			//Вхождение в список проигрываемых звуков
	dword playSoundsCount;			//Количество звуков проигрываемых в текущий момент	
	SoundBankFileWaveInfo * waves;	//Индекс начала таблицы волн этого звука
	dword wavesCount;				//Количество волн приписаных звуку
	union
	{
		SoundBankFileWaveQueue * squeue;//Очередь для случайного выбора волн при проигрывании звука
		dword selectSequenceCount;		//Счётчик для последовательного выбора волн
	};
	//	SoundBankFileWaveQueue * lqueue;//Очередь для случайного выбора волн зацикленного звука
	dword nameHash;					//Хэшь значение имени
	dword nameLen;					//Длинна имени
	SoundBankFileSound * next;		//Следующий в цепочке поиска
	const char * name;				//Имя звука	
	SoundBankFileExtra * extraData;	//Расширенные данные в таблице расширений
	dword extraCount;				//Количество расширенных данных
	SoundBankFileAttenuation att;	//Параметры для 3D звука
	SoundBankFileSetup setup;		//Базовые настройки звука	
#else
	const char * name;				//Имя звука
	dword nameHash;					//Хэшь значение имени
	dword nameLen;					//Длинна имени
	SoundBankFileSetup setup;		//Базовые настройки звука
	SoundBankFileAttenuation att;	//Параметры для 3D звука
	SoundBankFileWaveInfo * waves;	//Индекс начала таблицы волн этого звука
	dword wavesCount;				//Количество волн приписаных звуку
	dword playSoundsCount;			//Количество звуков проигрываемых в текущий момент
	void * playSoundsList;			//Вхождение в список проигрываемых звуков
	union
	{
		SoundBankFileWaveQueue * squeue;//Очередь для случайного выбора волн при проигрывании звука
		dword selectSequenceCount;		//Счётчик для последовательного выбора волн
	};
	//	SoundBankFileWaveQueue * lqueue;//Очередь для случайного выбора волн зацикленного звука
	SoundBankFileSound * next;		//Следующий в цепочке поиска
	SoundBankFileExtra * extraData;	//Расширенные данные в таблице расширений
	dword extraCount;				//Количество расширенных данных

#endif
#endif

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		if(!SoundBankFile_RestorePointer(waves, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(name, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(next, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(extraData, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(squeue, dataStart, size)) return false;
		playSoundsCount = 0;
		playSoundsList = null;
		InitSelectors();
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart, bool isNeedSwizzle)
	{
		if(wavesCount > 1)
		{
			
			dword selMode = setup.GetModeSelect();
			switch(selMode)
			{
			case SoundBankFileSetup::mode_select_rnd:				
				squeue = null;
				break;
			case SoundBankFileSetup::mode_select_queue:
				memset(squeue, 0, SoundBankFileWaveQueue::GetSize(wavesCount));
				SoundBankFile_PreparePointer(squeue, dataStart);
				break;
			case SoundBankFileSetup::mode_select_sequence:
				squeue = null;
				break;
			};
/*			dword loopMode = setup->GetModeLoop();
			if(loopMode == SoundBankFileSetup::mode_loop_sel_wave_que)
			{
				memset(lqueue, 0, SoundBankFileWaveQueue::GetSize(wavesCount));
				SoundBankFile_PreparePointer(lqueue, dataStart);
			}else{
				lqueue = null;
			}
*/
		}else{
			squeue = null;
//			lqueue = null;
		}
		playSoundsCount = 0;
		playSoundsList = null;		
		SoundBankFile_PreparePointer(waves, dataStart);
		SoundBankFile_PreparePointer(name, dataStart);
		SoundBankFile_PreparePointer(next, dataStart);
		SoundBankFile_PreparePointer(extraData, dataStart);
		SoundBankFile_Prepare4byteAlignedSwizzler(this, sizeof(SoundBankFileSound), isNeedSwizzle);
	}

	//Инициализация параметров выбора волн
	__forceinline void InitSelectors()
	{
		if(wavesCount > 1)
		{
			dword selMode = setup.GetModeSelect();
			switch(selMode)
			{
			case SoundBankFileSetup::mode_select_rnd:
				squeue = null;
				break;
			case SoundBankFileSetup::mode_select_queue:
				{
				//Ищем минимальное значение, чтобы определить количество экземпляров в очереди
				float minValue = waves[0].probability;
				for(dword i = 1; i < wavesCount; i++)
				{
					if(minValue < waves[i].probability)
					{
						minValue = waves[i].probability;
					}
				}
				//Не более 100 проигрывания на звук из таблицы
				squeue->minWeight = coremax(minValue, 0.01f);
				//Таблица в начале пустая
				squeue->count = 0;
				}
				break;
			case SoundBankFileSetup::mode_select_sequence:
				selectSequenceCount = 0;
				break;
			};
			/*			dword loopMode = setup->GetModeLoop();
			if(loopMode == SoundBankFileSetup::mode_loop_sel_wave_que)
			{
			if(!SoundBankFile_RestorePointer(lqueue, dataStart, size)) return false;
			lqueue->minWeight = 100.0f;
			lqueue->count = 0;
			}else{				
			lqueue = null;
			}
			*/
		}else{
			squeue = null;
//			lqueue = null;
		}
	}

	//Выбрать волну для воспроизведения
	__forceinline dword SelectWaveIndex()
	{
		if(wavesCount == 1)
		{
			return 0;
		}
		dword selMode = setup.GetModeSelect();
		switch(selMode)
		{
		case SoundBankFileSetup::mode_select_rnd:
			return GetRandomWaveIndex();
		case SoundBankFileSetup::mode_select_queue:
			return squeue->Select(waves, wavesCount);
		case SoundBankFileSetup::mode_select_sequence:
			{
				if(selectSequenceCount >= wavesCount)
				{
					selectSequenceCount = 0;
				}
				dword index = selectSequenceCount;
				selectSequenceCount++;
				return index;
			}
		};
		Assert(false);
		return 0;
	}
/*
	//Выбрать волну для повторения
	__forceinline long GetLoopWaveIndex(long currentIndex)
	{
		if(wavesCount == 1)
		{
			return currentIndex;
		}
		dword loopMode = setup->GetModeLoop();
		switch(loopMode)
		{
		case SoundBankFileSetup::mode_loop_diasble:
			Assert(false);
			return currentIndex;
		case SoundBankFileSetup::mode_loop_one_wave:
			return currentIndex;
		case SoundBankFileSetup::mode_loop_sel_wave_rnd:
			return GetRandomWaveIndex();
		case SoundBankFileSetup::mode_loop_sel_wave_que:
			return lqueue->Select(waves, wavesCount);
		}
		Assert(false);
		return currentIndex;
	}
*/
private:
	dword GetRandomWaveIndex()
	{
		float rnd = Rnd();
		float cur = 0.0f;
		for(dword i = 0; i < wavesCount; i++)
		{
			cur += waves[i].probability;
			if(cur >= rnd)
			{
				return i;
			}
		}
		return wavesCount - 1;
	}
};


//Описание фонем прикреплёных к волне
struct SoundBankFilePhonemes
{
	enum Consts
	{
		currentVersion = 1,
		phoneme_id_silence = 0,	//Фонема тишины
		phoneme_id_max = 254,	//Последний занят для корректного сравнения времени
	};

	dword version;		//Текущая версия данных
	dword framesCount;	//Количество кадров	
	dword phoneme[1];	//В старших 3х байтах находиться время в милисекундах, в младшем байте идентификатор фонемы

	static __forceinline dword GetSize(dword framesCount)
	{
		return sizeof(SoundBankFilePhonemes) + sizeof(dword)*(framesCount - 1);
	}

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart, bool isNeedSwizzle)
	{
		SoundBankFile_Prepare4byteAlignedSwizzler(this, sizeof(SoundBankFilePhonemes) + sizeof(dword)*(framesCount - 1), isNeedSwizzle);
	}

	//Найти кадр текущей фонемы зная время в милисекундах и последний кадр
	__forceinline dword FindFrame(dword currentTimeInMs, dword lookFromFrame = 0)
	{
		dword time = (currentTimeInMs <= 0xffffff) ? ((currentTimeInMs << 8) | 0xff) : 0xffffffff;
		for(dword i = lookFromFrame + 1; i < framesCount; i++)
		{
			if(time < phoneme[i])
			{
				return i - 1;
			}
		}
		return framesCount - 1;
	}

	//Получить идентификатор фонемы из запакованного значения
	__forceinline dword GetPhonemeId(dword frame)
	{
		return phoneme[frame] & 0xff;
	}

	//Запаковать фонему имея время в милисекундах и идентификатор
	static __forceinline dword PackPhoneme(dword timeInMs, byte id)
	{
		Assert(timeInMs <= 0xffffff);
		Assert(id < phoneme_id_max);
		return (timeInMs << 8) | dword(id);
	}

};

//Заголовок файла
struct SoundBankFileHeader
{
	dword uniqueId[4];				//Уникальный идентификатор звукового банка
	SoundBankFileSound * sounds;	//Звуки
	dword soundsCount;				//Количество звуков
	SoundBankFileWaveInfo * winfos;	//Описание волн подвязаных к звукам
	dword winfosCount;				//Размер таблицы волн
	SoundBankFileWave * waves;		//Волны, используещиеся в банке
	dword wavesCount;				//Количество волн используемых в банке
	SoundBankFileSound ** entry;	//Входная таблица поиска звуков по имени
	dword mask;						//Маска входа через таблицу поиска
	SoundBankFileObjectId * ids;	//Таблица идентификаторов, используемых для отладки
	dword idsCount;					//Количество идентификаторов
	SoundBankFileExtra * extras;	//Таблица расширений
	dword extrasCount;				//Количество расширений

	//Подготовить данные для использования
	__forceinline bool Restore(dword size)
	{
		//Указатель к которому приводим данные
		byte * dataStart = (byte *)this;
		//Себя
		if(!SoundBankFile_RestorePointer(sounds, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(winfos, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(waves, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(entry, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(ids, dataStart, size)) return false;
		if(!SoundBankFile_RestorePointer(extras, dataStart, size)) return false;
		//Таблица расширений
		if(!RestoreArray(extras, extrasCount, dataStart, size)) return false;
		//Входная таблица поиска звуков по имени
		for(dword i = 0; i <= mask; i++)
		{
			if(!SoundBankFile_RestorePointer(entry[i], dataStart, size)) return false;
		}
		//Волны, используещиеся в банке
		if(!RestoreArray(waves, wavesCount, dataStart, size)) return false;
		//Описание волн подвязаных к звукам
		if(!RestoreArray(winfos, winfosCount, dataStart, size)) return false;
		//Звуки
		if(!RestoreArray(sounds, soundsCount, dataStart, size)) return false;
		//Идентификаторы
		if(!RestoreArray(ids, idsCount, dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(bool isNeedSwizzle)
	{
		//Указатель к которому приводим данные
		byte * dataStart = (byte *)this;
		//Идентификаторы
		if(!isNeedSwizzle && ids)
		{
			PrepareArray(ids, idsCount, dataStart, false);
		}else{
			ids = null;
			idsCount = 0;
		}
		//Звуки
		PrepareArray(sounds, soundsCount, dataStart, isNeedSwizzle);
		//Описание волн подвязаных к звукам
		PrepareArray(winfos, winfosCount, dataStart, isNeedSwizzle);
		//Волны, используещиеся в банке
		PrepareArray(waves, wavesCount, dataStart, isNeedSwizzle);
		//Входная таблица поиска звуков по имени
		for(dword i = 0; i <= mask; i++)
		{
			SoundBankFile_PreparePointer(entry[i], dataStart);
		}
		SoundBankFile_Prepare4byteAlignedSwizzler(entry, (mask + 1)*sizeof(SoundBankFileSound *), isNeedSwizzle);
		//Таблица расширений
		for(dword i = 0; i <= extrasCount; i++)
		{
			switch(extras[i].id)
			{
			case sbf_extra_phonemes:
				((SoundBankFilePhonemes *)extras[i].data)->Prepare(dataStart, isNeedSwizzle);
				break;				
//			default:
				//Непрописанный тип. Надо внести в таблицу.
//				Assert(false);
			}
		}
		PrepareArray(extras, extrasCount, dataStart, isNeedSwizzle);
		//Себя
		SoundBankFile_PreparePointer(ids, dataStart);
		SoundBankFile_PreparePointer(sounds, dataStart);
		SoundBankFile_PreparePointer(winfos, dataStart);
		SoundBankFile_PreparePointer(waves, dataStart);
		SoundBankFile_PreparePointer(entry, dataStart);
		SoundBankFile_PreparePointer(extras, dataStart);
		SoundBankFile_Prepare4byteAlignedSwizzler(this, sizeof(SoundBankFileHeader), isNeedSwizzle);
	}

private:
	template<class T> __forceinline bool RestoreArray(T * aptr, dword count, byte * dataStart, dword size)
	{
		for(dword i = 0; i < count; i++)
		{
			if(!aptr[i].Restore(dataStart, size)) return false;
		}
		return true;
	}

	template<class T> __forceinline void PrepareArray(T * aptr, dword count, byte * dataStart, bool isNeedSwizzle)
	{
		for(dword i = 0; i < count; i++)
		{
			aptr[i].Prepare(dataStart, isNeedSwizzle);
		}
	}

};

//Идентификатор файла
struct SoundBankFileId
{
	enum Consts
	{
		current_version = 4,
	};

	//Получить идентификатор файла
	inline static const byte * GetId()
	{
		static const char * id = "SBF ";
		return (const byte *)id;
	};

	//Получить версию текущего описания файла
	inline static const byte * GetVer(long verIndex = current_version)
	{
		static const char * ver = "1.2 ";
		if(verIndex == 4)
		{
			return (const byte *)ver;
		}
		return null;
	};

	//Получить значение хэшь функции заданной для теста
	inline static void GetTestStringHash(byte v[4])
	{
		static const dword hashValue = string::Hash("This Is String For Check Hash");
		v[0] = (hashValue >> 0) & 0xff;
		v[1] = (hashValue >> 8) & 0xff;
		v[2] = (hashValue >> 16) & 0xff;
		v[3] = (hashValue >> 24) & 0xff;
	};

	inline static void CheckMachine()
	{
		Assert(sizeof(word) == 2);
		Assert(sizeof(dword) == 4);
		Assert(sizeof(const char *) == 4);
		Assert(sizeof(SoundBankFileId) == 16);
	};


	inline void Init(dword pcOffset)
	{
		for(long i = 0; i < 4; i++)
		{
			id[i] = GetId()[i];
			ver[i] = GetVer()[i];
			dword shift = i << 3;
			pcoffset[i] = byte(pcOffset >> shift);			
		}
		GetTestStringHash(checkHash);		
	}

	inline bool CheckId()
	{
		const byte * sid = GetId();
		for(long i = 0; i < 4; i++)
		{
			if(id[i] != sid[i])
			{
				return false;
			}
		}
		byte tHash[4];
		GetTestStringHash(tHash);
		for(long i = 0; i < 4; i++)
		{
			if(tHash[i] != checkHash[i])
			{
				return false;
			}
		}
		return true;
	}

	inline bool CheckVer(long verIndex = current_version)
	{
		const byte * sver = GetVer(verIndex);
		if(!sver)
		{
			return false;
		}
		for(long i = 0; i < 4; i++)
		{
			if(ver[i] != sver[i])
			{
				return false;
			}
		}
		return true;
	}

	inline dword GetXboxChankOffset()
	{
		return sizeof(SoundBankFileId);
	}

	inline dword GetXboxChankSize()
	{
		return GetPCChankOffset() - GetXboxChankOffset();
	}

	inline dword GetPCChankOffset()
	{
		dword size = pcoffset[0];
		size |= dword(pcoffset[1]) << 8;
		size |= dword(pcoffset[2]) << 16;
		size |= dword(pcoffset[3]) << 24;
		return size;
	}

	inline dword GetPCChankSize(dword totalFileSize)
	{
		dword size = totalFileSize - GetPCChankOffset();
		return size;
	}


	byte id[4];							//Идентификатор файла
	byte ver[4];						//Версия файла
	byte checkHash[4];					//Хэшь для проверки
	byte pcoffset[4];					//Положение чанка данных для pc
};


#endif
