
#pragma once

#ifndef _SoundScene_h_
#define _SoundScene_h_

#include "..\..\common_h\Sound.h"
#include "SoundBankFile.h"
#include "SoundsEngine.h"

class Sound;
class SoundService;

class SoundScene : public ISoundScene
{
	struct CntQueue
	{
		Sound * snd;
		float tailTime;
		dword priority;
		dword isLoop;
	};

public:
	SoundScene(SoundService & service, const char * ovnerName, const char * cppFile, long cppLine);
	virtual ~SoundScene();


//------------------------------------------------------------------------------------------
//ISoundScene
//------------------------------------------------------------------------------------------
public:
	//Удалить сцену
	virtual void Release();

	//Создать непозиционированный звук, если звук не создался - вернётся null
	virtual ISound * Create(const ConstString & soundName, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, float vol);
	//Создать позиционированный в 3D звук, если звук не создался - вернётся null
	virtual ISound3D * Create3D(const ConstString & soundName, const Vector & worldPos, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, float vol);

private:
	//Создать звук
	Sound * CreateSound(const ConstString & soundName, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, const Vector * worldPos, float vol);
public:

	//Продолжить звучание сцены
	virtual void Resume();
	//Приостановить звучание сцены
	virtual void Pause();
	//Узнать текущее состояние сцены
	virtual bool IsPause();

	//Установить общую громкость звучание всей сцены
	virtual void SetVolume(float volume);
	//Получить общую громкость звучание всей сцены
	virtual float GetVolume();

	//Установить позицию слушателя
	virtual void SetListenerMatrix(const Matrix & mtx);
	//Получить позицию слушателя
	virtual void GetListenerMatrix(Matrix & mtx);

	//Установить текущую окружающую сцену с временем перехода в секундах
	virtual void SetSoundEnvironmentScene(const Enveronment * envPeset, float blendTime);
	//Получить текущую окружающую сцену
	virtual void GetSoundEnvironmentScene(Enveronment & envPeset);

	//Переименовать владельца сцены
	virtual void ModifyOvnerName(const char * ovnerName);


//------------------------------------------------------------------------------------------
//SoundService
//------------------------------------------------------------------------------------------
public:
	//Обновить сцену
	void Update(float dltTime);
	//Обновить громкости
	void UpdateVolumes();
	//Остановить проигрывание всех звуков
	void StopAllSounds(const char * sceneOvner);
	//Проверить на используемость
	bool IsUse(SoundBankFileSound & sbfs);
	//Путь удаляемого звукового банка
	void CheckForDelete(const char * soundBankName);
	//Нарисовать отладочную информацию
	void DebugDraw(dword debugStates, long & line);
	//Написать в лог сообщение об неудалённой сцене
	void UnReleaseMessage();

private:
	static bool CompareCntQueue(const CntQueue & q1, const CntQueue & q2);

//------------------------------------------------------------------------------------------
//Sound
//------------------------------------------------------------------------------------------
public:
	//Исполнить внутреннюю команду звука перед обновлением
	void ExecuteSoundCommand(Sound * snd);
	//Поставить звук на обновление
	void StartUpdate(Sound * snd);
	//Прекратить обновлять звук
	void StopUpdate(Sound * snd);

	//Поставить звук в очередь на доигрывание
	void SetToContinueQueue(Sound * snd, float tailTime, dword priority, bool isLoop);
	//Удалить звук из очереди на доигрывание
	void RemoveFromContinueQueue(Sound * snd);
	//Получить размер очереди на доигрывание
	dword GetCouninueQueueSize();
	
public:
	//Получить громкость для звуков
	float GetVolumeSounds();
	//Получить громкость для музыки
	float GetVolumeMusic();

	//Преобразовать позицию из мира в систему слушателя проверив максимальную дистанцию
	bool ListenerTransfromPosition(const Vector & glb, float maxDist2, Vector & loc, float & dist2);

	//Удалить звук
	void ReleaseSound(Sound * snd);	

	//Получить мастер сцены
	SoundsEngine::SceneMaster & GetSceneMaster();

	//Получить звуковой движёк	
	SoundsEngine & GetSoundsEngine();

//------------------------------------------------------------------------------------------
//Пулы
//------------------------------------------------------------------------------------------
private:

	//Выделить из пула звук
	static Sound * SoundNew(SoundScene & scene, bool is3D, bool isAutodelete, SoundBankFileSound & sbfs, const char * bankName, const char * cppFile, long cppLine);
	//Освободить в пуле звук
	static void SoundDelete(Sound * ptr);

public:
	//Очистить все пулы
	static void ClearPulls();

//------------------------------------------------------------------------------------------
private:
	SoundService & soundService;	//Ссылка на сервис звука
	array<Sound *> sounds;			//Заведёные звуки сцены
	array<Sound *> soundCommands;	//Список звуков ждущих команды	
	array<Sound *> soundUpdates;	//Список звуков ждущих команды
	array<CntQueue> continueQueue;	//Очередь звуков требующих продолжения
	Matrix listener;				//Матрица позиции слушателя
	Matrix inverseListener;			//Обратная матрица позиции слушателя
	float volume;					//Громкость сцены
	float volumeSounds;				//Громкость звуков
	float volumeMusic;				//Громкость музыки
	SoundsEngine::SceneMaster * sm;	//Сюда идут звуки сцены	на которые действуют эффекты окружения
	bool isPause;					//Состояние
	bool isQueueSorted;				//Очередь продолжений сортированна
	bool isEvtBlend;				//Блендить параметры сдеры окружения
	string ovnerNameSaved;			//Сохранёное имя владельца сцены
	string cppFileSaved;			//Сохранёное имя файла, откуда создавалась сцена
	long cppLineSaved;				//Сохранёное строка файла, откуда создавалась сцена
	FxScene::EnvParams envCurrent;	//Текущие параметры среды окружения
	FxScene::EnvParams envSet;		//Требуемые параметры среды окружения
	float envBlendWeight;			//Вес с которым действуют повые параметры среды окружения
	float envBlendSpeed;			//Скорость, с которой происходит изменение параметров среды окружения
};


//Исполнить внутреннюю команду звука перед обновлением
__forceinline void SoundScene::ExecuteSoundCommand(Sound * snd)
{
	soundCommands.Add(snd);
}

//Получить громкость для звуков
__forceinline float SoundScene::GetVolumeSounds()
{
	return volumeSounds;
}

//Получить громкость для музыки
__forceinline float SoundScene::GetVolumeMusic()
{
	return volumeMusic;
}

//Преобразовать позицию из мира в систему слушателя
__forceinline bool SoundScene::ListenerTransfromPosition(const Vector & glb, float maxDist2, Vector & loc, float & dist2)
{
	dist2 = (glb - listener.pos).GetLength2();
	if(dist2 < maxDist2)
	{
		loc = inverseListener*glb;
		return true;
	}
	return false;	
}


//Получить мастер сцены
__forceinline SoundsEngine::SceneMaster & SoundScene::GetSceneMaster()
{
	Assert(sm);
	return *sm;
}

//Получить размер очереди на доигрывание
__forceinline dword SoundScene::GetCouninueQueueSize()
{
	return continueQueue.Size();
}


#endif
