

#ifndef _MissionSoundBase_h_
#define _MissionSoundBase_h_


#include "..\..\..\Common_h\mission\mission.h"


#define MISSION_SOUND_PARAMS	MOP_ARRAYBEG("Sounds", 0, 1000)															\
									MOP_STRING("Sound name", "sound")													\
									MOP_FLOATEX("Volume", 1.0f, 0.0f, 10.0f)											\
									MOP_FLOATEX("Min period time", 4.0f, 0.0f, 10000.0f)								\
									MOP_FLOATEX("Max period time", 20.0f, 0.0f, 10000.0f)								\
									MOP_BOOL("Periodical sound", true)													\
									MOP_BOOL("Sound on", true)															\
								MOP_ARRAYEND																			\
								MOP_FLOATEX("Global volume", 1.0f, 0.0f, 10.0f)											\
								MOP_FLOATEXC("Fade in", 0.0f, 0.0f, 10.0f, "Fade in time for activate sound (sec)")		\
								MOP_FLOATEXC("Fade out", 0.0f, 0.0f, 10.0f, "Fade out time for deactivate sound (sec)")	\
								MOP_BOOL("Active", false)


class MissionSoundBase : public MissionObject
{
public:
	enum State
	{
		s_periodical = 1,
		s_stoped = 2,
	};

	struct SoundElement
	{		
		ISound * sound;			//Звук
		float waitTime;			//Время ожидания до следующего запуска
		float minTime;			//Минимальное значение периода повторения		
		float maxTime;			//Максимальное значение периода повторения
		float volume;			//Громкость звука
		dword states;			//Флаги состояния
	};


	//--------------------------------------------------------------------------------------------
protected:
	MissionSoundBase();
	virtual ~MissionSoundBase();



	//--------------------------------------------------------------------------------------------
public:
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Пересоздать объект
	virtual void Restart();

	//--------------------------------------------------------------------
#ifndef MIS_STOP_EDIT_FUNCS
public:
	//Перевести объект в спящий режим
	virtual void EditMode_Sleep(bool isSleep);
	//Получить размеры описывающего ящика
	virtual void EditMode_Select(bool isSelect);
#endif

	//--------------------------------------------------------------------
public:
	//Работа
	virtual void _cdecl Work(float dltTime, long level);

	//--------------------------------------------------------------------
protected:
	//Инициализировать объект
	bool CreateSounds(MOPReader & reader, const Vector * pos);
	//Обновить состояние счётчиков и звуков
	void Update(float dltTime);
	//Обновить позицию (можно вызывать только если был указан pos при создании)
	void UpdatePosition(const Vector & pos);
	//Обновить громкость
	void UpdateVolumes(float vol);
	//Запустить последовательное проигрывание звуков
	void PreviewStart();
	//Остановить проигрывание звуков
	void Stop();
	//Приостановить проигрывание звуков с фэйдом
	void Fadeout();
	//Удалить все звуки
	void Release();

protected:
	array<SoundElement> sounds;
	float fadeInTime;
	float fadeOutTime;
};

#endif
