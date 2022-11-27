


#ifndef _SoundParams_h_
#define _SoundParams_h_


#include "..\SndBase.h"


enum SoundBaseParamConsts
{
	sbpc_db_min = volume_db_min,
	sbpc_db_max = volume_db_max,
	sbpc_db_init = 0,
	sbpc_count_max = 64,
	sbpc_count_inf = -1,
	sbpc_count_init = 16,
	sbpc_priority_min = 0,
	sbpc_priority_norm = 5,
	sbpc_priority_max = 10,
	sbpc_priority_init = sbpc_priority_norm,
	//Методы выбора волны
	sbpc_select_rnd = 0,		//Выбирать волны случайным образом
	sbpc_select_queue = 1,		//Выбирать волны случайно, избегая повторений с учётом весов
	sbpc_select_sequence = 2,	//Выбирать волны последовательно как они заданны
	sbpc_select_init = sbpc_select_rnd,
	sbpc_select_min = sbpc_select_rnd,
	sbpc_select_max = sbpc_select_sequence,
	//Зацикленные звуки
	sbpc_loop_disable = 0,		//Звук не зациклен
	sbpc_loop_one_wave = 1,		//Отыгрывать зацикленно звук с 1й волной
	//		sbpc_loop_sel_wave_rnd = 2,	//Отыгрывать зацикленно звук каждый раз выбирая новую волну как в mode_select_rnd
	//		sbpc_loop_sel_wave_que = 3,//Отыгрывать зацикленно звук каждый раз выбирая новую волну как в mode_select_queue
	sbpc_loop_min = sbpc_loop_disable,
	//		sbpc_loop_max = sbpc_loop_sel_wave_que,
	sbpc_loop_max = sbpc_loop_one_wave,
	//Какие эффекты применять 
	sbpc_fx_full = 0,		//Применять к звуку все эффекты
	sbpc_fx_premaster = 1,	//Пропусить эфект окружающей среды
	sbpc_fx_master = 2,		//Пропусить эфект окружающей среды и премастера
	sbpc_fx_music = 3,		//Музыка
	sbpc_fx_init = sbpc_fx_full,
	sbpc_fx_min = sbpc_fx_full,
	sbpc_fx_max = sbpc_fx_music,
	//Модификаторы
	sbpc_mod_phoneme = 1,		//Генерить для волн фонемы
	sbpc_mod_init = 0,
};


class SoundBaseParams
{
public:
	SoundBaseParams();
	//Сохранить параметры в ини файле
	void SaveToIni(IEditableIniFile * ini);
	//Загрузить параметры из ини файла
	void LoadFromIni(IIniFile * ini);


public:
	//Получить текстовое описание режима выбора волн
	static const char * GetSelTextDesc(long select);
	//Получить текстовое описание режима зацикленного проигрывания
	static const char * GetLoopTextDesc(long loop);
	//Получить текстовое описание режима эффектов
	static const char * GetFxTextDesc(long fx);

public:
	long priority;				//Приоритет по отношению к остальным звукам
	long maxCount;				//Максимальное количество одновременно проигрываемых данных звуков
	float volume;				//Общая громкость звука
	long selectMethod;			//Метод выбора волн
	long loopMode;				//Режим зацикленного проигрывания
	long fx;					//Какие эффекты применять
	long modifiers;				//Модификаторы
};


class SoundAttGraph
{
public:
	struct Range
	{
		float minValue;
		float maxValue;
		float initValue;
	};

public:
	SoundAttGraph();
	//Сохранить параметры в ини файле
	void SaveToIni(IEditableIniFile * ini);
	//Загрузить параметры из ини файла
	void LoadFromIni(IIniFile * ini);
	//Проверить и исправить значения
	void CheckAndFix();

public:
	float minDist;				//Минимальныя дистанция
	float maxDist;				//Максимальныя дистанция
	float c[4];					//Коэфициенты графика затухания

public:
	static const Range minD;	//Диапазон минимальной дистанции
	static const Range maxD;	//Диапазон максимальной дистанции
	static const float mmDist;	//Минимальное расстояние между максимальной и минимальной дистанциями
	static const Range cR[4];	//Диапазон коэфициентов кривой
};



#endif

