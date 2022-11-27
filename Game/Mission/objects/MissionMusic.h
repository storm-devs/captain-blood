
#ifndef _MissionMusic_h_
#define _MissionMusic_h_

#include "..\..\..\Common_h\Mission.h"


class MissionMusic : public MissionObject
{
	enum Action
	{
		act_error,
		act_play_mission,
		act_play_movie_p,
		act_play_movie_s,
		act_pause_movie,
		act_continue_movie,
		act_play_global,
		act_stop_all,
	};

	struct Translate
	{
		dword id;
		ConstString str;
	};

public:
	enum Slot
	{
		s_mission = 0,
		s_movie = 1,
		s_global = 2,
		max_slots_count = 3,
	};
	//--------------------------------------------------------------------------------------------
public:
	MissionMusic();
	virtual ~MissionMusic();

	//--------------------------------------------------------------------------------------------
protected:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	//--------------------------------------------------------------------
protected:
#ifndef MIS_STOP_EDIT_FUNCS
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Update(MOPReader & reader);
	//Выделить объект
	virtual void EditMode_Select(bool isSelect);
	//Перевести объект в спящий режим
	virtual void EditMode_Sleep(bool isSleep);
#endif

protected:
	const char * musicName;
	Action action;
	float fadeIn;
	float fadeOut;
	float volume;	
	const char * activateMessage;
public:
	static Translate actions[];
};


class MusicService : public Service
{
	struct Slot
	{
		ISoundScene * musicScene;	//Сцена, в которой живёт музыка игры
		ISound * music;				//Текущая активная музыка
		ISound * fadeoutPlay;		//Доигрывающая музыка
		const char * name;			//Имя слота
	};

public:
	MusicService();
	virtual ~MusicService();
	virtual bool Init();
	virtual void EndFrame(float dltTime);

public:
	//Начать играть музыку
	bool PlayMusic(long slotIndex, const char * name, float volume, float fadeInTime, float fadeOutTime, bool isCheckGlobal);
	//Пауза слота
	void PauseMusic(long slotIndex, bool isPause);
	//Остановить музыку
	void StopMusic(long slotIndex, float fadeOutTime);

private:
	void StopSlot(long slotIndex, float fadeOutTime);

private:
	Slot slots[MissionMusic::max_slots_count];

public:
	static MusicService * ptr;	//Указатель на объект
};


#endif


































