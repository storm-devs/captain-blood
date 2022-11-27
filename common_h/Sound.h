#ifndef _Sound_h_
#define _Sound_h_

#include "core.h"
#include "math3d.h"

/*
	Все громкости задаются в диапазоне 0..1
	Имена звуков являются ссылками на описания

	Описание звука содержит следующие параметры:
		- имена файлов с весами выбора
		- громкость
		- зациклен или нет
		- дистанция начала затухания громкости *
		- дистанция окончания затухания громкости, когда звук неслышно *

		* параметры используемые только 3D звуками
*/


//Глобальный звуковой банк, загружаемый сервисом
#define SoundService_GlobalSoundBank	"Globals"


//Звук
class ISound
{
public:
	enum ErrorCode
	{
		ec_ok,
		ec_phonemes_no_data,
		ec_phonemes_data_is_empty,
		ec_phonemes_sound_not_play,
	};

protected:
	virtual ~ISound(){}

public:
	//Удалить звук
	virtual void Release() = null;

	//Проиграть
	virtual void Play() = null;
	//Остановить
	virtual void Stop() = null;
	//Узнать текущее состояние
	virtual bool IsPlay() = null;

	//Установить громкость звука
	virtual void SetVolume(float volume) = null;
	//Получить громкость звука
	virtual float GetVolume() = null;

	//Получить имя звука
	virtual const char * GetName() = null;
	//Получить имя звукового банка
	virtual const char * GetSoundBankName() = null;

	//Запустить звук проигрываться и плавно увеличить громкость за time секунд
	virtual void FadeIn(float time) = null;
	//Плавно уменьшить громкость до 0 за time секунд и остановить проигрывание звука
	virtual void FadeOut(float time) = null;

	//Узнать, зациклен звук или нет
	virtual bool IsLoop() = null;

	//Вернуть идентификатор фонемы. Если отрицательный результат - фонемы нет, 0 - тишина
	virtual long GetPhonemeId(ErrorCode * ecode = null) = null;
};

//Звук, позиционируемый в пространстве
class ISound3D : public ISound
{
protected:
	virtual ~ISound3D(){}

public:
	//Установить позицию источника звука
	virtual void SetPosition(const Vector & position) = null;
	//Получить позицию источника звука
	virtual Vector GetPosition() = null;
};

//Звуковая сцена
class ISoundScene
{
public:
	
	//Параметры среды
	struct Enveronment
	{
		union
		{
			struct
			{
				float predelayTime;			//Задержка до первого отражённого звука 0..500мс
				float earlyTime;			//Время всех вторичных отражений 0..1000мс
				float earlyAttenuation;		//Коэфициент затухания первичных отражений
				float damping;				//Поглощение средой высокочастотной составляющей (захламлёность)
				float dispersion;			//Степень рассеивания (мелкие предметы)
				float wet;					//Какая часть рассеяного звука попадает в выходной сигнал (0..1)
				float dry;					//Какая часть исходного звука попадает в выходной сигнал (0..1)
				float unuse;				//Не используеться
			};
			float val[8];					//Представление в виде набора чисел
		};
	};


protected:
	virtual ~ISoundScene(){}

public:
	//Удалить сцену
	virtual void Release() = null;

	//Создать непозиционированный звук, если звук не создался - вернётся null
	inline ISound * Create(const char * soundName, const char * _cppFile, long _cppLine, bool isStartPlay = true, bool autoDelete = true)
	{
		return Create(ConstString(soundName), _cppFile, _cppLine, isStartPlay, autoDelete);
	}

	//Создать позиционированный в 3D звук, если звук не создался - вернётся null
	inline ISound3D * Create3D(const char * soundName, const Vector & worldPos, const char * _cppFile, long _cppLine, bool isStartPlay = true, bool autoDelete = true)
	{
		return Create3D(ConstString(soundName), worldPos, _cppFile, _cppLine, isStartPlay, autoDelete);
	}

	//Создать непозиционированный звук, если звук не создался - вернётся null
	virtual ISound * Create(const ConstString & soundName, const char * _cppFile, long _cppLine, bool isStartPlay = true, bool autoDelete = true, float vol = 1.0f) = null;
	//Создать позиционированный в 3D звук, если звук не создался - вернётся null
	virtual ISound3D * Create3D(const ConstString & soundName, const Vector & worldPos, const char * _cppFile, long _cppLine, bool isStartPlay = true, bool autoDelete = true, float vol = 1.0f) = null;

	//Продолжить звучание сцены
	virtual void Resume() = null;
	//Приостановить звучание сцены
	virtual void Pause() = null;
	//Узнать текущее состояние сцены
	virtual bool IsPause() = null;
	
	//Установить общую громкость звучание всей сцены
	virtual void SetVolume(float volume) = null;
	//Получить общую громкость звучание всей сцены
	virtual float GetVolume() = null;

	//Установить позицию слушателя
	virtual void SetListenerMatrix(const Matrix & mtx) = null;
	//Получить позицию слушателя
	virtual void GetListenerMatrix(Matrix & mtx) = null;

	//Установить текущую окружающую сцену с временем перехода в секундах
	virtual void SetSoundEnvironmentScene(const Enveronment * envPeset, float blendTime = 2.0f) = null;
	//Получить текущую окружающую сцену
	virtual void GetSoundEnvironmentScene(Enveronment & envPeset) = null;

	//Переименовать владельца сцены
	virtual void ModifyOvnerName(const char * ovnerName) = null;

};


//Звуковой сервис
class ISoundService : public Service
{
public:

	enum DebugLevel
	{
		dl_minimal = 1,
		dl_maximum = 2
	};

	enum DebugView
	{
		dv_none = 0,
		dv_2d = 1,
		dv_3d = 2,
		dv_listener = 4,
		dv_listener3d = dv_3d | dv_listener,
		dv_all = dv_2d | dv_3d | dv_listener,
	};

public:
	//Загрузить звуковой банк
	virtual bool LoadSoundBank(const char * path) = null;
	//Удалить звуковой банк
	virtual void ReleaseSoundBank(const char * path) = null;

	//Создать звуковую сцену
	virtual ISoundScene * CreateScene(const char * ovnerName, const char * cppFile, long cppLine) = null;

	//Установить общую громкость
	virtual void SetGlobalVolume(float volume) = null;
	//Получить общую громкость
	virtual float GetGlobalVolume() = null;

	//Установить громкость звуков
	virtual void SetSoundsVolume(float volume) = null;
	//Получить громкость звуков
	virtual float GetSoundsVolume() = null;

	//Установить громкость музыки
	virtual void SetMusicVolume(float volume) = null;
	//Получить громкость музыки
	virtual float GetMusicVolume() = null;

	//Установка отладочных параметров
	virtual void SetDebugLevel(DebugLevel dl) = null;
	virtual DebugLevel GetDebugLevel() = null;
	virtual void SetDebugView(dword dv) = null;
	virtual dword GetDebugView() = null;

	//Нарисовать отладочную информацию
	virtual void DebugDraw() = null;

#ifdef _XBOX
	virtual void * GetXAudio2() = null;
#endif
};


#endif

