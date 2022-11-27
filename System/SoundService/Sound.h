
#pragma once

#include "..\..\common_h\Sound.h"
#include "SoundBankFile.h"
#include "SoundsEngine.h"

class SoundScene;
class SoundService;

class Sound : public ISound3D
{
	enum States
	{
		//Состояния ожидания
		cmd_none = 0,
		cmd_play = 1,					//Был выбран старт, в конце кадра надо запустить проигрывание
		cmd_stop = 2,					//Ожидание остановки звука
		cmd_fadein = 3,					//Начать проигрывать с увеличивающейся громкостью
		cmd_fadeout = 4,				//Остановить проигрывание с плавно уменьшающейся громкостью
		cmd_mask = 0xff,				//Маска для изъятия команды
		
		//Текущие состояния
		s_play = 0x10000,				//В текущий момент проигрываеться волна
		s_pause = 0x20000,				//Звук находиться на паузе
		s_outofrange = 0x40000,			//3D звук за пределами
		s_mask = 0xff0000,				//Маска состояний

		//Константные состояния
		att_isAutoDelete = 0x20000000,	//Удалить звук при остановке
		att_isLoop = 0x40000000,		//Зацикленный звук
		att_is3D = 0x80000000,			//Звук используеться как 3D
		
	};

	enum Const
	{
		c_time_to_skip_lost_ms = 1500,	//Время остатка, при котором потерянный звук уже не востанавливаеться
		c_fade_time_to_restore = 500,	//Время фэйда востанавливающегося звука

	};

public:
	Sound(SoundScene & scene, bool is3D, bool isAutodelete, SoundBankFileSound & snd, const char * bankName, const char * _cppFile, long _cppLine);
	virtual ~Sound();

public:
	//Удалить звук
	virtual void Release();

	//Проиграть
	virtual void Play();
	//Остановить
	virtual void Stop();
	//Узнать текущее состояние
	virtual bool IsPlay();

	//Установить громкость звука
	virtual void SetVolume(float volume);
	//Получить громкость звука
	virtual float GetVolume();

	//Получить имя звука
	virtual const char * GetName();
	//Получить имя звукового банка
	virtual const char * GetSoundBankName();

	//Запустить звук проигрываться и плавно увеличить громкость за time секунд
	virtual void FadeIn(float time);
	//Плавно уменьшить громкость до 0 за time секунд и остановить проигрывание звука
	virtual void FadeOut(float time);

	//Узнать, зациклен звук или нет
	virtual bool IsLoop();

	//Вернуть идентификатор фонемы. Если отрицательный результат - фонемы нет, 0 - тишина
	virtual long GetPhonemeId(ErrorCode * ecode);


public:
	//Установить позицию источника звука
	virtual void SetPosition(const Vector & position);
	//Получить позицию источника звука
	virtual Vector GetPosition();

public:
	//Приостоновить звук
	void ScenePause();
	//Продолжить проигрывать звук
	void SceneResume();
	//Продолжить проигрывание звука с потерянным каналом
	bool ContinueLost();

	//Установить код для пула 
	void SetPullCode(dword code);
	//Получить код пула
	dword GetPullCode();



public:
	//Исполнить команду
	void ExecuteCommand();
	//Обновить звук (если true, то звук выкинуть из списка обновления)
	void Update(float dltTime);
	//Написать в лог сообщение об неудалённом звуке
	void UnReleaseMessage(const char * sceneOvner);
	//Отобразить отладочную информацию
	void DebugDraw(dword debugStates, long & line, const char * sceneOvner);
	//Нарисовать слушателя
	static void DebugDrawListener(dword debugStates, const Matrix & listener, const char * sceneOvner);
	//Написать о параметрах эффекта среды окружения
	static void DebugDrawEnvironment(dword debugStates, long & line, const char * sceneOvner, const FxScene::EnvParams & env);
	//Вывести глобальные параметры
	static void DebugDrawGlobals(dword debugStates, SoundService & service);
	//Этот ли звук
	bool IsThis(SoundBankFileSound & sbfs);

private:
	//Установить команду на исполнение
	void SetCommand(States cmd);
	//Запустить звук на проигрывание немедленно
	void PlayNow(bool isFadeIn);
	//Мэнеджмент канала и волн
	void ProcessChannel(long count = 4);
	//Остановить звук немедленно
	void StopNow(bool noRelease = false);
	//Заместить этот звук новым
	void Replase();
	//Обновить громкость звука
	void UpdateVolume();
	//Обновить позицию звука
	float Update3DPosition(bool isUpdateChannel);

#ifndef STOP_DEBUG
	//Вывести отладочную информацию для 3D звука
	void DebugDraw3D(dword drawLevel, const char * sceneOvner);
	//Вывести отладочную информацию для 2D звука
	void DebugDraw2D(dword drawLevel, long & line, const char * sceneOvner);
#endif

private:
	SoundsEngine::SoundChannel * channel;		//Используемый канал для проигрывания звука
	SoundBankFileSound & sbfSound;				//Данные звука
	const char * soundBankName;					//Имя звукового банка, которому принадлежит звук
	dword states;								//Текущие состояние звука
	float currentTime;							//Текущие время проигрывания звука
	SoundBankFileWaveInfo * wave;				//Текущая проигрываемая волна
	SoundBankFilePhonemes * phonemes;			//Фонемы
	dword phonemesFrame;						//Последняя позиция с которой были считаны фонемы
	SoundScene & soundScene;					//Сцена, которой принадлежим
	float currentVolume;						//Текущая громкость звука
	Vector globalPos;							//Позиция в глобальной системе	
	float fadeTime;								//Время фэйд-эффекта
	dword pausePosition;						//Позиция паузы
	Sound * playPrev;							//Предыдущий в списке проигрываемых
	Sound * playNext;							//Следующий в списке проигрываемых
	dword pullCode;								//Код текущего пула памяти
	const char * cppFile;						//Файл из которого был создан звук
	long cppLine;								//Строка с которой был создан звук
};

//Установить код для пула 
__forceinline void Sound::SetPullCode(dword code)
{
	pullCode = code;
}

//Получить код пула
__forceinline dword Sound::GetPullCode()
{
	return pullCode;
}

//Этот ли звук
__forceinline bool Sound::IsThis(SoundBankFileSound & sbfs)
{
	return (&sbfs == &sbfSound);
}
