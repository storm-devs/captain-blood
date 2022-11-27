
#ifndef _SoundService_h_
#define _SoundService_h_

#include "ISoundEditorAccessor.h"
#include "SoundScene.h"
#include "SoundOff.h"

class SoundsEngine;
class SoundBanks;



class SoundService : public ISoundService
{
	enum States
	{
		s_needUpdateVolumes = 0x1000000,
	};

public:
	SoundService();
	virtual ~SoundService();

	//Инициализвация
	virtual bool Init();
	//Исполнение
	virtual void EndFrame(float dltTime);

	//Создать звуковую сцену
	virtual ISoundScene * CreateScene(const char * ovnerName, const char * cppFile, long cppLine);

	//Загрузить звуковой банк
	virtual bool LoadSoundBank(const char * path);
	//Удалить звуковой банк
	virtual void ReleaseSoundBank(const char * path);

	//Установить общую громкость
	virtual void SetGlobalVolume(float volume);
	//Получить общую громкость
	virtual float GetGlobalVolume();

	//Установить громкость звуков
	virtual void SetSoundsVolume(float volume);
	//Получить громкость звуков
	virtual float GetSoundsVolume();

	//Установить громкость музыки
	virtual void SetMusicVolume(float volume);
	//Получить громкость музыки
	virtual float GetMusicVolume();

	//Установка отладочных параметров
	virtual void SetDebugLevel(DebugLevel dl);
	virtual DebugLevel GetDebugLevel();
	virtual void SetDebugView(dword dv);
	virtual dword GetDebugView();

	//Нарисовать отладочную информацию
	virtual void DebugDraw();

#ifdef _XBOX
	virtual void * GetXAudio2();
#endif

public:
	//Найти звук по имени
	SoundBankFileSound * FindSound(const ConstString & name, const char ** bankName);

	//Разрешён ли вывод в лог
	bool IsLogOut();
	//Писать в лог сообщения о созданных звуках (медленно)
	bool IsTraceCreates();
	//Разрешён ли рисование отладочной информации
	bool IsDebugDraw();
	//Получить отладочные состояния
	dword GetDebugStates();

	//Получить текущую громкость звуков
	float GetCurrentSoundsVolume();
	//Получить текущую громкость музыки
	float GetCurrentMusicVolume();

	//Получить доступ к движку
	SoundsEngine & Engine();
	//Удалить сцену из списка
	void RemoveScene(SoundScene * scene);

	//Получить количество звуков ожидающих продолжение
	dword GetContinueWaiting();

public:
#ifndef NO_TOOLS
	//Разрешить-запретить создавать звуки с временных данных
	void EditEnablePreview(bool isEnable);
	//Зарегистрировать звук для предпрослушивания
	void EditRegistrySound(SoundBankFileSound * sbfs);
	//Отменить регистрацию звука для предпрослушивания
	bool EditUnregistrySound(SoundBankFileSound * sbfs);
	//Обновить параметры звука
	bool EditPatchSound(EditPatchSoundData & data, bool isCheckWaves);
	//Остановить проигрывание всех звуков
	void EditStopAllSounds(const char * sceneOvner);

#endif

private:
	array<SoundScene *> scenes;			//Созданные звуковые сцены
	SoundsEngine * engine;				//Звуковая игралка
	SoundBanks * soundBanks;			//Доступные звуковые банки
	float globalVolume;					//Общая громкость
	ICoreStorageFloat * stGlobalVolume;	//Запись общей громкости в базе ядра
	float soundsVolume;					//Громкость звуковых эффектов
	ICoreStorageFloat * stSoundsVolume;	//Запись громкости звуковых эффектов в базе ядра
	float musicVolume;					//Громкость музыки
	ICoreStorageFloat * stMusicVolume;	//Запись громкости музыки в базе ядра
	float curSoundsVolume;				//Кромкость звуковых эффектов
	float curMusicVolume;				//Громкость музыки
	SoundSceneOff sceneOff;				//Сцена для выключенного состояния
	dword states;						//Состояния сервиса	
	
	

#ifndef NO_TOOLS
private:
	array<SoundBankFileSound *> previewSounds;
	bool isEnablePreview;
#endif
};

//Разрешён ли вывод в лог
__forceinline bool SoundService::IsLogOut()
{
	return (states & SoundsEngine::dbg_logout) != 0;
}

//Писать в лог сообщения о созданных звуках (медленно)
__forceinline bool SoundService::IsTraceCreates()
{
	return (states & SoundsEngine::dbg_tracecreates) != 0;
}

//Разрешён ли рисование отладочной информации
__forceinline bool SoundService::IsDebugDraw()
{
	return (states & (SoundsEngine::dbg_draw2d | SoundsEngine::dbg_draw3d)) != 0;
}

//Получить отладочные состояния
__forceinline dword SoundService::GetDebugStates()
{
	return states;
}

//Получить текущую громкость звуков
__forceinline float SoundService::GetCurrentSoundsVolume()
{
	return curSoundsVolume;
}

//Получить текущую громкость музыки
__forceinline float SoundService::GetCurrentMusicVolume()
{
	return curMusicVolume;
}

//Получить доступ к движку
__forceinline SoundsEngine & SoundService::Engine()
{
	return *engine;
}

//Удалить сцену из списка
__forceinline void SoundService::RemoveScene(SoundScene * scene)
{
	scenes.Del(scene);
}

#endif
