

#pragma once



#include "..\..\Common_h\math3d.h"
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\sound.h"
#include "..\..\Common_h\render.h"
#include "..\..\Common_h\gmx.h"
#include "..\..\Common_h\gui.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\Common_h\controls.h"
#include "..\..\Common_h\Animation.h"
#include "..\..\Common_h\corecmds.h"

#include "..\..\Common_h\Templates\Array.h"
#include "..\..\Common_h\Templates\Stack.h"
#include "..\..\Common_h\Templates\String.h"

#include "..\..\System\SoundService\ISoundEditorAccessor.h"

#define SoundSceneName "__SoundEditorScene__"

class BaseSlider;
class NumberSlider;
class FormScrollBar;
class FormButton;
class FormEdit;
class FormComboBox;
class FormListBase;
class FormCheckBox;


enum SndGlobalConsts
{
	sgc_header_h = 24,
	sgc_header_left_space = 5,
	sgc_header_controls_h = 18,
	sgc_header_controls_space = 6,
	sgc_header_buttons_w = sgc_header_controls_h,
	sgc_header_combo_w = 200,
	sgc_header_combo_list_h = 120,
	sgc_header_image_size = 16,	


	volume_db_min = -60,
	volume_db_max = 24,

	//Права, с которыми работает пользователь
	policy_sounder = 0,					//Доступ на уровне звуковака
	policy_designer = 1,				//Доступ на уровне дизайнера

	policy_magic_designer = 7354028,	//Магическое число для включения режима дизайнера
	policy_magic_developer_d = 4212541,	//Магическое число для включения режима разработки (только в дебаге)
	policy_magic_developer_s = 4212542,	//Магическое число для включения режима разработки (только в дебаге)

	//Состояние работы над клипом (один элемент озвучки)
	movie_stage_movie = 1,		//Состояние редактирования клипа
	movie_stage_events = 2,		//Состояние расстановки событий
	movie_stage_approval = 3,	//Состояние приёмки
	movie_stage_done = 4,		//Законченный клип
	movie_stage_check = 5,		//Были изменения, требуеться проверка
	movie_stage_damage = 6,		//Были разрушены данные в файле, требуеться детальное рассмотрение
	movie_stage_first = movie_stage_movie,
	movie_stage_last = movie_stage_damage,
};

#define SndExporter_MinExportVolume SndDbToVol(volume_db_min)
#define SndExporter_MaxExportVolume SndDbToVol(volume_db_max)

struct ErrorId
{
	__forceinline ErrorId()
	{
		SetOk();
	}
	
	__forceinline bool operator == (const ErrorId & eid) const
	{
		return id == eid.id;
	}

	__forceinline bool operator != (const ErrorId & eid) const
	{
		return id != eid.id;
	}

	__forceinline bool IsOk()
	{
		return id == -1;
	}

	__forceinline bool IsError()
	{
		return id != -1;
	}

	__forceinline void SetOk()
	{
		id = -1;
	}

	long id;
	static ErrorId ok;
};

//Расширенное имя для редактора
struct ExtName
{
	const char * str;		//Строка
	dword len;				//Длина имени в символах
	dword hash;				//Хэшь имени
	long w;					//Ширина имени в пикселях для текущего шрифта
	long h;					//Высота строки в пикселях для текущего шрифта


	inline ExtName()
	{
		str = null;
		len = 0;
		hash = 0;
		w = 0;
		h = 0;
	}

	inline void Init(const char * n)
	{
		str = n;
		UpdateParams();
	};

	__forceinline bool operator == (const ExtName & name) const
	{
		if(name.hash == hash && name.len == len)
		{
			return string::IsEqual(name.str, str);
		}
		return false;
	}

	void UpdateParams();
};

struct ExtNameStr : public ExtName
{
	ExtNameStr()
	{
		str = null;
	}

	inline void SetString(const char * n)
	{
		if(!string::IsEmpty(n))
		{
			data = n;
			str = data.c_str();			
		}else{
			str = null;
			data.Empty();
		}
		UpdateParams();
	};

	string data;
};


__forceinline float SndDbToVol(float db)
{
	float volume = db >= 0.0f ? powf(10.0f, db/20.0f) : 1.0f/powf(10.0f, db/-20.0f);
	return volume;
}

__forceinline float SndVolToDb(float vol)
{
	float curDb = 20.0f*log10f(vol);
	return curDb;
}


//Уникальный идентификатор элемента проекта
struct UniqId
{
	__forceinline UniqId()
	{ 
		Reset();
	}

	//Обнулить идентификатор
	__forceinline void Reset()
	{ 
		data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0; 
	}

	//Сравнить идентификаторы
	__forceinline bool operator == (const UniqId & uid) const
	{
		dword value = (uid.data[0] ^ data[0]) | (uid.data[1] ^ data[1]) | (uid.data[2] ^ data[2]) | (uid.data[3] ^ data[3]);
		return value == 0;
	}

	//Сравнить идентификаторы
	__forceinline bool operator != (const UniqId & uid) const
	{
		dword value = (uid.data[0] ^ data[0]) | (uid.data[1] ^ data[1]) | (uid.data[2] ^ data[2]) | (uid.data[3] ^ data[3]);
		return value != 0;
	}

	__forceinline bool IsValidate() const
	{
		return (data[0] | data[1] | data[2] | data[3]) != 0;
	}

	//Сгенерировать уникальный идентификатор
	void Build();
	//Сконвертировать идентификатор в строку, оканчивающуюся на 0 (забирать сразу в свой буфер)
	const char * ToString() const;	
	//Востановить идентификатор из строки
	bool FromString(const char * s);

	dword data[4];

	static UniqId zeroId;
};


