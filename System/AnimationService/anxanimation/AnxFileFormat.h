//===========================================================================================================================
// Spirenkov Maxim, 2004, 2008
//===========================================================================================================================
// AnxAnimation system
//===========================================================================================================================
// Anx file format
//===========================================================================================================================

#ifndef _Anx_FileFormat_
#define _Anx_FileFormat_


#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\data_swizzle.h"


//===========================================================================================================================
//
//  !!! Порядок байт в файле задан от старшего к младшему !!!
//
//---------------------------------------------------------------------------------------------------------------------------
//
//	***	AnxHeaderId										Идентификатор файла
//	***	AnxHeader                           			Заголовок файла с указателями таблиц
//	***	AnxBone[AnxHeader.numBones]						Таблица костей (скелет)
//	***	AnxNode[AnxHeader.numNodes]						Ноды анимационного графа
//	***	AnxLink[AnxHeader.numLinks]						Линки анимационного графа
//	***	AnxConst[AnxHeader.numConsts]					Константы нодов
//	***	AnxEvent[AnxHeader.numEvents]					События клипов
//	***	AnxFndName[AnxHeader.numNames]					Элементы для поиска имени
//  *	AnxClip[AnxHeader.numClips]						Клипы
//	*	AnxMvTrackHeader[AnxHeader.numTrackHeaders]		Заголовки треков
//	*	AnxTrackHeader[AnxHeader.numTrackHeaders]		Заголовки треков
//	*	AnxPointer<AnxBone>[AnxHeader.htBonesSize]		Входная таблица для поиска костей по полному имени
//	*	AnxPointer<AnxBone>[AnxHeader.htShortBonesSize]	Входная таблица для поиска костей по короткому имени
//	*	AnxPointer<AnxFndName>[AnxHeader.htNamesSize]	Входная таблица для поиска именён
//	*	const char *[AnxHeader.numEvtPrm]				Таблица указателей, используемая в графе
//	*	const char []									Таблица строк
//      tracks data                                     Упакованные данные треков
//      byte[4..7] = 0                                  Буферный масив для безопасного чтения из треков 4х байт за раз
//
//  *	выравнено по 4 байт границе
//  ***	выравнено по 16 байт границе
// 
//
//===========================================================================================================================

//Структуры анимационного файла
struct AnxHeaderId;
struct AnxHeader;
struct AnxBone;
struct AnxNode;
struct AnxLink;
struct AnxConst;
struct AnxTrackHeader;
struct AnxClip;
struct AnxEvent;
struct AnxEventName;

__forceinline void AnxGlobal_4byteAlignedSwizzler(void * ptr, dword bytes)
{
	//Порядок байт в файле задан от старшего к младшему
	if(__SwizzleDetect()) return;
	Assert((bytes & 3) == 0);
	dword * p = (dword *)ptr;
	for(dword * pe = p + (bytes >> 2); p < pe; p++)
	{
		__RefDataSwizzler(*p);
	}
}

//Преобразуемый указатель
template<class T> struct AnxPointer
{
	//Востановить указатель
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		if(!__SwizzleDetect())
		{
			__RefDataSwizzler(offset);
		}
		return RestoreNoSwizzle(dataStart, size);
	}

	//Востановить указатель
	__forceinline bool RestoreNoSwizzle(byte * dataStart, dword size)
	{
		if(offset < size)
		{
			if(offset)
			{
				((byte * &)ptr) = dataStart + offset;
			}else{
				ptr = null;
			}
			return true;
		}
		return false;
	}

	//Подготовить указатель для сохранения файла
	__forceinline void Prepare(byte * dataStart)
	{
		PrepareNoSwizzle(dataStart);
		if(!__SwizzleDetect())
		{
			__RefDataSwizzler(offset);
		}
	}

	//Подготовить указатель для сохранения файла
	__forceinline void PrepareNoSwizzle(byte * dataStart)
	{
		if(ptr)
		{
			offset = ((byte *)ptr) - dataStart;
		}else{
			offset = 0;
		}		
	}

	union
	{
		T * ptr;						//Указатель на данные
		dword offset;					//Относительное смещение от начала файла
	};
};

//Описание кости
struct AnxBone
{
	long parent;						//Индекс на родительскую кость
	dword flags;						//Флаги кости
	AnxPointer<const char> name;		//Полное имя кости
	dword hash;							//Хэшь полного имени
	AnxPointer<AnxBone> nextName;		//Следующая кость с похожим кэшем полного имени
	AnxPointer<const char> shortName;	//Короткое имя кости
	dword shortHash;					//Хэшь короткого имени
	AnxPointer<AnxBone> nextShortName;	//Следующая кость с похожим кэшем короткого имени

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxBone));
		if(!name.RestoreNoSwizzle(dataStart, size)) return false;
		if(!nextName.RestoreNoSwizzle(dataStart, size)) return false;
		if(!shortName.RestoreNoSwizzle(dataStart, size)) return false;
		if(!nextShortName.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{		
		name.PrepareNoSwizzle(dataStart);
		nextName.PrepareNoSwizzle(dataStart);
		shortName.PrepareNoSwizzle(dataStart);
		nextShortName.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxBone));
	}
};

//Описание линка
struct AnxLink
{
	enum Consts
	{
		invalidate_range_value = 0xffffffff,
	};


	float kBlendTime;					//Коэфициент время блендинга между нодами 1/t, если <= 0 то мгновенно переключаться
	float syncPos;						//Относительная синхронизация нодов	
	AnxPointer<AnxNode> toNode;			//Нод, на который переходит линк
	AnxPointer<const char> name;		//Имя линка
	dword arange[2];					//Диапазон активации по кадрам
	dword mrange[2];					//Диапазон перехода по кадрам

	bool IsActivateRange()
	{
		return arange[0] != invalidate_range_value;
	}

	bool IsMoveRange()
	{
		return mrange[0] != invalidate_range_value;
	}
	
	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxLink));
		if(!toNode.RestoreNoSwizzle(dataStart, size)) return false;
		if(!name.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		toNode.PrepareNoSwizzle(dataStart);
		name.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxLink));
	}
};


//Описание события
struct AnxEvent
{
	enum Flags
	{
		flg_isNoBuffered = 1,			//Не буферизировать эвент
	};

	AnxPointer<const char> name;		//Имя линка
	dword frame;						//Кадр срабатывания события
	AnxPointer<const char *> params;	//Список параметров
	word numParams;						//Начало списка строк
	word flags;							//Флаги

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxEventSwizzle();
		if(!name.RestoreNoSwizzle(dataStart, size)) return false;
		if(!params.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		name.PrepareNoSwizzle(dataStart);
		params.PrepareNoSwizzle(dataStart);
		AnxEventSwizzle();
	}

private:
	__forceinline void AnxEventSwizzle()
	{
		if(__SwizzleDetect()) return;
		__RefDataSwizzler(name);
		__RefDataSwizzler(frame);
		__RefDataSwizzler(params);
		__RefDataSwizzler(numParams);
		__RefDataSwizzler(flags);
	}
};

//Запись для поиска имени
struct AnxFndName
{
	enum Flags
	{
		f_empty = 0,					//Внутреннее имя
		f_event = 1,					//Есть эвенты с таким эвентом
		f_node = 2,						//Есть ноды с таким эвентом
		f_link = 4,						//Есть линки с таким эвентом
		f_const = 8,					//Есть константы с таким эвентом
		f_any = 0xffffff				//Искать среди всех имён
	};

	AnxPointer<const char> name;		//Имя эвента
	dword hash;							//Хэшь
	AnxPointer<AnxFndName> next;		//Следующий с похожим хэшем
	dword type;							//Комбинация флагов, указывающих кем используеться имя

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxFndName));
		if(!name.RestoreNoSwizzle(dataStart, size)) return false;
		if(!next.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		name.PrepareNoSwizzle(dataStart);
		next.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxFndName));
	}
};


//Описание константы нода
struct AnxConst
{
	//Типы констант
	enum Type
	{
		ct_string = 0,
		ct_float = 1,
		ct_blend = 2,
	};

	AnxPointer<const char> name;		//Имя константы
	dword type;							//Тип константы
	AnxPointer<const char> svalue;		//Строковое значение константы
	float fvalue;						//Числовое значение константы

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxConst));
		if(!name.RestoreNoSwizzle(dataStart, size)) return false;
		if(!svalue.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		name.PrepareNoSwizzle(dataStart);
		svalue.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxConst));
	}

};

//Заголовок трека
struct AnxTrackHeaderBase
{
	enum Flags
	{
		//Маска размера элемента
		size_mask = 255,
		//Маски кодов треков
		code_const = 0,				//Некомпрессированное значение float min
		code_ui8 = 1,				//Упакованный трек из беззнаковых 8 bit значений min + delta*track[frame])
		code_ui16 = 2,				//Упакованный трек из беззнаковых 16 bit значений min + delta*track[frame])
		code_ui24 = 3,				//Упакованный трек из беззнаковых 24 bit значений min + delta*track[frame])
		code_float = 4,				//Некомпрессированный трек из float значений
		code_mask = 7,				//Маска для получения типа трека
		//Сдвиг для получения соответствующего кода
		code_shift = 3,				//Велечина сдвига для последовательного считывания
		code_shift_start = 8,		//Начальный сдвиг с которого можно перебирать элементы
		code_shift_qx = 8,			//Велечина сдвига при чтении маски qx
		code_shift_qy = 11,			//Велечина сдвига при чтении маски qy
		code_shift_qz = 14,			//Велечина сдвига при чтении маски qz
		code_shift_qw = 17,			//Велечина сдвига при чтении маски qw
		code_shift_px = 20,			//Велечина сдвига при чтении маски px
		code_shift_py = 23,			//Велечина сдвига при чтении маски py
		code_shift_pz = 26,			//Велечина сдвига при чтении маски pz
		code_shift_dx = 8,			//Велечина сдвига при чтении маски dx
		code_shift_dz = 11,			//Велечина сдвига при чтении маски dz
		//Маска для определения типа позиции в треки
		flag_global_pos =0x40000000,//Флаг означающий что позиция дана в глобальной системе координат
		//Индексы range для каждого значения
		index_qx = 0,				//qx
		index_qy = 1,				//qy
		index_qz = 2,				//qz
		index_qw = 3,				//qw
		index_px = 4,				//px
		index_py = 5,				//py
		index_pz = 6,				//pz
		index_dx = 0,				//dx
		index_dz = 1,				//dz
		
	};


	//Диапазон трека
	struct Range
	{
		float min;						//Минимальное значение трека
		float delta;					//Диапазон значений
	};

	dword desc;							//Тип трека, размер одного элемента и остальное
	AnxPointer<byte> data;				//Указатель на данные трека

protected:
	__forceinline void SwizzleTrack(dword numFrames, dword numElements)
	{
		byte * track = data.ptr;
		byte tmp, tmp1;
		for(dword i = 0; i < numFrames; i++)
		{
			dword f = desc >> code_shift_start;
			for(dword j = 0; j < numElements; j++, f >>= code_shift)
			{
				switch(f & code_mask)
				{
				case code_const:					
					break;
				case code_ui8:
					track += 1;
					break;
				case code_ui16:
					tmp = track[0];
					track[0] = track[1];
					track[1] = tmp;
					track += 2;
					break;
				case code_ui24:
					tmp = track[0];
					track[0] = track[2];
					track[2] = tmp;
					track += 3;
					break;
				case code_float:
					tmp = track[0];
					tmp1 = track[1];
					track[0] = track[3];
					track[1] = track[2];
					track[3] = tmp;
					track[2] = tmp1;
					track += 4;
					break;
				}
			}
		}
	}

	__forceinline void ReadValue(float & value, byte * & track, dword code, Range & range)
	{		
		switch(code & code_mask)
		{
		case code_const:
			(dword &)value = (dword &)range.min;
			break;
		case code_ui8:
			value = range.min + (*(core_unaligned byte *)track)*range.delta;
			track += 1;
			break;
		case code_ui16:
			value = range.min + (*(core_unaligned word *)track)*range.delta;
			track += 2;
			break;
		case code_ui24:
#ifdef _XBOX
			value = range.min + (*(core_unaligned dword *)(track - 1) & 0xffffff)*range.delta;
#else
			value = range.min + (*(core_unaligned dword *)track & 0xffffff)*range.delta;
#endif
			track += 3;
			break;
		case code_float:
			(dword &)value = *(core_unaligned dword *)track;
			track += 4;
			break;
		}
	}

};	

//Описание трека перемещения
struct AnxMvTrackHeader : public AnxTrackHeaderBase
{
	Range range[2];		//Параметры масштабирования перемещения за кадр

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxMvTrackHeader));
		if(!data.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		data.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxMvTrackHeader));
	}

	__forceinline void SwizzleTrack(dword numFrames)
	{
		AnxTrackHeaderBase::SwizzleTrack(numFrames, 2);
	}

	__forceinline void GetMovement(dword frame, Vector & movment)
	{
		byte * track =  data.ptr + (desc & size_mask)*frame;
		ReadValue(movment.x, track, (desc >> code_shift_dx) & code_mask, range[index_dx]);
		movment.y = 0.0f;
		ReadValue(movment.z, track, (desc >> code_shift_dz) & code_mask, range[index_dz]);
	}

};

//Описание трека кости
struct AnxTrackHeader : public AnxTrackHeaderBase
{
	Range range[7];	//Параметры масштабирования кватерниона

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxTrackHeader));
		if(!data.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		data.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxTrackHeader));
	}

	__forceinline void SwizzleTrack(dword numFrames)
	{
		AnxTrackHeaderBase::SwizzleTrack(numFrames, 7);
	}

	__forceinline bool GetFrame(dword frame, Quaternion & q, Vector & p)
	{
		byte * track =  data.ptr + (desc & size_mask)*frame;
		ReadValue(q.x, track, (desc >> code_shift_qx) & code_mask, range[index_qx]);
		ReadValue(q.y, track, (desc >> code_shift_qy) & code_mask, range[index_qy]);
		ReadValue(q.z, track, (desc >> code_shift_qz) & code_mask, range[index_qz]);
		ReadValue(q.w, track, (desc >> code_shift_qw) & code_mask, range[index_qw]);
		ReadValue(p.x, track, (desc >> code_shift_px) & code_mask, range[index_px]);
		ReadValue(p.y, track, (desc >> code_shift_py) & code_mask, range[index_py]);
		ReadValue(p.z, track, (desc >> code_shift_pz) & code_mask, range[index_pz]);
		return (desc & flag_global_pos) != 0;
	}
};

//Описание клипа
struct AnxClip
{		
	AnxPointer<AnxTrackHeader> btracks;		//Таблица описания треков костей
	dword frames;							//Размер клипа в кадрах
	float fps;								//Скорость проигрывания анимации
	AnxPointer<AnxMvTrackHeader> mtrack;	//Таблица описания треков перемещения
	float probability;						//Нормализованная вероятность выбора клипа в ноде
	AnxPointer<AnxEvent> events;			//Указание первого эвента, относящегося к клипу
	dword numEvents;						//Количество событий

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxClip));
		if(!btracks.RestoreNoSwizzle(dataStart, size)) return false;
		if(!mtrack.RestoreNoSwizzle(dataStart, size)) return false;
		if(!events.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		btracks.PrepareNoSwizzle(dataStart);
		mtrack.PrepareNoSwizzle(dataStart);
		events.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxClip));
	}

	//Получить значение кости на заданном кадре
	__forceinline bool GetFrame(dword boneIndex, dword frame, Quaternion & q, Vector & p)
	{
		return btracks.ptr[boneIndex].GetFrame(frame, q, p);
	}

	//Получить перемещение на заданном кадре
	__forceinline void GetMovement(dword frame, Vector & movment)
	{
		if(mtrack.ptr)
		{
			mtrack.ptr->GetMovement(frame, movment);
		}else{
			movment = Vector(0.0f);
		}
	}
};


//Информация о ноде
struct AnxNode
{
	enum Flags
	{
		isLoop = 1,
		isChange = 2,
		isStop = 4,
	};

	dword flags;						//Флаги нода
	AnxPointer<AnxClip> clips;			//Массив клипов
	dword numClips;						//Количество клипов в ноде
	AnxPointer<AnxLink> links;			//Массив линков
	word numLinks;						//Количество линков
	word defLink;						//Линк по которому переходить в случае отсутствия активных линков
	AnxPointer<AnxConst> consts;		//Массив констант
	dword numConsts;					//Количество констант
	AnxPointer<const char> name;		//Имя нода

	//Подготовить данные для использования
	__forceinline bool Restore(byte * dataStart, dword size)
	{
		AnxNodeSwizzle();
		if(!clips.RestoreNoSwizzle(dataStart, size)) return false;
		if(!links.RestoreNoSwizzle(dataStart, size)) return false;
		if(!consts.RestoreNoSwizzle(dataStart, size)) return false;
		if(!name.RestoreNoSwizzle(dataStart, size)) return false;
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void Prepare(byte * dataStart)
	{
		clips.PrepareNoSwizzle(dataStart);
		links.PrepareNoSwizzle(dataStart);
		consts.PrepareNoSwizzle(dataStart);
		name.PrepareNoSwizzle(dataStart);
		AnxNodeSwizzle();
	}

	//Найти константу
	__forceinline AnxConst * FindConst(const char * name)
	{
		for(dword i = 0; i < numConsts; i++)
		{
			AnxConst & c = consts.ptr[i];
			if(string::IsEqual(c.name.ptr, name))
			{
				return &consts.ptr[i];
			}
		}
		return null;
	}


private:
	__forceinline void AnxNodeSwizzle()
	{
		if(__SwizzleDetect()) return;
		__RefDataSwizzler(flags);
		__RefDataSwizzler(clips);
		__RefDataSwizzler(numClips);
		__RefDataSwizzler(links);
		__RefDataSwizzler(numLinks);
		__RefDataSwizzler(defLink);
		__RefDataSwizzler(consts);
		__RefDataSwizzler(numConsts);
		__RefDataSwizzler(name);
	}
};

//Заголовок данных анимации
struct AnxHeader
{
	AnxPointer<AnxBone> bones;						//Массив костей
	dword numBones;									//Количество костей в анимации
	AnxPointer<AnxNode> nodes;						//Массив нодов
	dword numNodes;									//Количество нодов
	AnxPointer<AnxLink> links;						//Массив линков
	dword numLinks;									//Количество линков
	AnxPointer<AnxConst> consts;					//Массив констант
	dword numConsts;								//Количество констант нода
	AnxPointer<AnxEvent> events;					//Массив эвентов
	dword numEvents;								//Количество событий
	AnxPointer<AnxFndName> fndNames;				//Элементы для поиска имён
	dword numNames;									//Количество элементов для поиска имён
	AnxPointer<AnxClip> clips;						//Массив клипов
	dword numClips;									//Количество клипов
	AnxPointer<AnxMvTrackHeader> movmentTracks;		//Массив заголовков треков для перемещения
	dword numMovmentTracks;							//Количество треков	для перемещения
	AnxPointer<AnxTrackHeader> bonesTracks;			//Массив заголовков треков костей
	dword numBonesTracks;							//Количество треков	костей
	AnxPointer<AnxPointer<AnxBone>> htBones;		//Входная таблица для поиска кости по полному имени
	dword htBonesMask;								//Маска для для поиска костей по полному имени
	AnxPointer<AnxPointer<AnxBone>> htShortBones;	//Входная таблица для поиска кости по короткому имени
	dword htShortBonesMask;							//Маска для для поиска костей по короткому имени
	AnxPointer<AnxPointer<AnxFndName>> htNames;		//Входная таблица для поиска имён
	dword htNamesMask;								//Маска для входной таблицы для поиска имён
	AnxPointer<AnxPointer<const char>> evtPrms;		//Массив указателей на параметры событий
	dword numEvtPrms;								//Количество параметров событий
	AnxPointer<AnxNode> startNode;					//Стартовый нод
	dword flags;									//Флаги

	//Подготовить данные для использования
	__forceinline bool RestoreAnimation(byte * dataStart, dword size)
	{
		//Заголовок
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxHeader));
		if(!bones.RestoreNoSwizzle(dataStart, size)) return false;
		if(!nodes.RestoreNoSwizzle(dataStart, size)) return false;
		if(!links.RestoreNoSwizzle(dataStart, size)) return false;
		if(!consts.RestoreNoSwizzle(dataStart, size)) return false;
		if(!events.RestoreNoSwizzle(dataStart, size)) return false;		
		if(!fndNames.RestoreNoSwizzle(dataStart, size)) return false;
		if(!clips.RestoreNoSwizzle(dataStart, size)) return false;
		if(!movmentTracks.RestoreNoSwizzle(dataStart, size)) return false;
		if(!bonesTracks.RestoreNoSwizzle(dataStart, size)) return false;		
		if(!htBones.RestoreNoSwizzle(dataStart, size)) return false;
		if(!htShortBones.RestoreNoSwizzle(dataStart, size)) return false;
		if(!htNames.RestoreNoSwizzle(dataStart, size)) return false;
		if(!evtPrms.RestoreNoSwizzle(dataStart, size)) return false;
		if(!startNode.RestoreNoSwizzle(dataStart, size)) return false;
		//Структуры данных
		if(!RestoreArray(bones, numBones, dataStart, size)) return false;
		if(!RestoreArray(nodes, numNodes, dataStart, size)) return false;
		if(!RestoreArray(links, numLinks, dataStart, size)) return false;
		if(!RestoreArray(consts, numConsts, dataStart, size)) return false;
		if(!RestoreArray(events, numEvents, dataStart, size)) return false;
		if(!RestoreArray(fndNames, numNames, dataStart, size)) return false;
		if(!RestoreArray(clips, numClips, dataStart, size)) return false;
		if(!RestoreArray(movmentTracks, numMovmentTracks, dataStart, size)) return false;
		if(!RestoreArray(bonesTracks, numBonesTracks, dataStart, size)) return false;		
		if(!RestoreArray(htBones, htBonesMask + 1, dataStart, size)) return false;
		if(!RestoreArray(htShortBones, htShortBonesMask + 1, dataStart, size)) return false;
		if(!RestoreArray(htNames, htNamesMask + 1, dataStart, size)) return false;
		if(!RestoreArray(evtPrms, numEvtPrms, dataStart, size)) return false;
		//Треки
		SwizzleTracks();
		return true;
	}

	//Упаковать данные для сохранения
	__forceinline void PrepareAnimation(byte * dataStart)
	{
		//Треки
		SwizzleTracks();
		//Структуры данных		
		PrepareArray(evtPrms, numEvtPrms, dataStart);
		PrepareArray(htNames, htNamesMask + 1, dataStart);
		PrepareArray(htShortBones, htShortBonesMask + 1, dataStart);
		PrepareArray(htBones, htBonesMask + 1, dataStart);		
		PrepareArray(clips, numClips, dataStart);
		PrepareArray(bonesTracks, numBonesTracks, dataStart);
		PrepareArray(movmentTracks, numMovmentTracks, dataStart);
		PrepareArray(consts, numConsts, dataStart);
		PrepareArray(fndNames, numNames, dataStart);
		PrepareArray(events, numEvents, dataStart);
		PrepareArray(links, numLinks, dataStart);
		PrepareArray(nodes, numNodes, dataStart);
		PrepareArray(bones, numBones, dataStart);
		//Заголовок
		bones.PrepareNoSwizzle(dataStart);
		nodes.PrepareNoSwizzle(dataStart);
		links.PrepareNoSwizzle(dataStart);
		events.PrepareNoSwizzle(dataStart);		
		consts.PrepareNoSwizzle(dataStart);
		bonesTracks.PrepareNoSwizzle(dataStart);
		movmentTracks.PrepareNoSwizzle(dataStart);
		clips.PrepareNoSwizzle(dataStart);
		fndNames.PrepareNoSwizzle(dataStart);
		startNode.PrepareNoSwizzle(dataStart);
		htBones.PrepareNoSwizzle(dataStart);
		htShortBones.PrepareNoSwizzle(dataStart);
		htNames.PrepareNoSwizzle(dataStart);
		evtPrms.PrepareNoSwizzle(dataStart);
		AnxGlobal_4byteAlignedSwizzler(this, sizeof(AnxHeader));
	}

	//Найти по имени кость
	__forceinline long FindBone(const char * boneName, dword hash)
	{
		AnxBone * bone = htBones.ptr[hash & htBonesMask].ptr;
		while(bone)
		{
			if(bone->hash == hash)
			{
				if(string::IsEqual(bone->name.ptr, boneName))
				{
					return bone - bones.ptr;
				}				
			}
			bone = bone->nextName.ptr;
		}
		return -1;
	}

	//Найти по короткому имени кость
	__forceinline long FindBoneShort(const char * boneName, dword hash)
	{
		AnxBone * bone = htShortBones.ptr[hash & htShortBonesMask].ptr;
		while(bone)
		{
			if(bone->shortHash == hash)
			{
				if(string::IsEqual(bone->shortName.ptr, boneName))
				{
					return bone - bones.ptr;
				}				
			}
			bone = bone->nextShortName.ptr;
		}
		return -1;
	}

	//Найти по имени нод
	__forceinline long FindNode(const char * nodeName)
	{
		const char * nativeName = FindName(nodeName, AnxFndName::f_node);
		if(nativeName)
		{
			for(dword i = 0; i < numNodes; i++)
			{
				if(nodes.ptr[i].name.ptr == nativeName)
				{
					return i;
				}
			}
		}
		return -1;
	}

	//Найти имя (уникальный внутренний указатель)
	__forceinline const char * FindName(const char * name, dword mask = AnxFndName::f_any)
	{
		dword hash = string::HashNoCase(name);
		AnxFndName * fname = htNames.ptr[hash & htNamesMask].ptr;
		while(fname)
		{
			if(fname->hash == hash && (fname->type & mask) != 0)
			{
				if(string::IsEqual(fname->name.ptr, name))
				{
					return fname->name.ptr;
				}				
			}
			fname = fname->next.ptr;
		}
		return null;
	}


private:
	template<class T> __forceinline bool RestoreArray(AnxPointer<T> & aptr, dword count, byte * dataStart, dword size)
	{
		for(dword i = 0; i < count; i++)
		{
			if(!aptr.ptr[i].Restore(dataStart, size)) return false;
		}
		return true;
	}
	
	template<class T> __forceinline void PrepareArray(AnxPointer<T> & aptr, dword count, byte * dataStart)
	{
		for(dword i = 0; i < count; i++)
		{
			aptr.ptr[i].Prepare(dataStart);
		}
	}

	__forceinline void SwizzleTracks()
	{
		if(!__SwizzleDetect())
		{
			for(dword i = 0; i < numClips; i++)
			{
				AnxClip & clip = clips.ptr[i];
				if(clip.mtrack.ptr)
				{
					clip.mtrack.ptr->SwizzleTrack(clip.frames);
				}
				for(dword j = 0; j < numBones; j++)
				{
					clip.btracks.ptr[j].SwizzleTrack(clip.frames);
				}
			}
		}
	}

};

//Идентификатор файла
struct AnxHeaderId
{
	enum Consts
	{
		current_version = 1,
	};


	//Получить идентификатор файла
	inline static const byte * GetId()
	{
		static const char * id = "ANX ";
		return (const byte *)id;
	};

	//Получить версию текущего описания файла
	inline static const byte * GetVer(long verIndex = current_version)
	{
		static const char * ver = "2.0 ";
		static const char * ver1 = "2.1 ";
		if(verIndex == 0)
		{
			return (const byte *)ver;
		}
		if(verIndex == 1)
		{
			return (const byte *)ver1;
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
		Assert(sizeof(AnxPointer<byte>) == 4);
		Assert(sizeof(AnxHeaderId) == 16);
		Assert(sizeof(AnxHeader) == 7*16);
		Assert(sizeof(Matrix) == 4*16);
		Assert(sizeof(AnxBone) == 2*16);
		Assert(sizeof(AnxNode) == 2*16);
		Assert(sizeof(AnxLink) == 2*16);
		Assert(sizeof(AnxConst) == 16);
		Assert(sizeof(AnxTrackHeader) == 4*16);
		Assert(sizeof(AnxMvTrackHeader) == 4*6);
		Assert(sizeof(AnxClip) == 4*7);
		Assert(sizeof(AnxEvent) == 16);
	};


	inline void Init()
	{
		for(long i = 0; i < 4; i++)
		{
			id[i] = GetId()[i];
			ver[i] = GetVer()[i];
			resered[i] = 0;
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

	byte id[4];							//Идентификатор файла
	byte ver[4];						//Версия файла
	byte checkHash[4];					//Хэшь для проверки
	byte resered[4];					//Зарезервировано для будующего использования
};



#endif




