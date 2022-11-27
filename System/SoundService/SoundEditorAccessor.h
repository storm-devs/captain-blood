
#ifndef _SoundEditorAccessor_h_
#define _SoundEditorAccessor_h_

#ifndef _XBOX
#ifndef NO_TOOLS

#include "ISoundEditorAccessor.h"
#include "SoundService.h"

class SoundEditorAccessor : public ISoundEditorAccessor
{
public:
	SoundEditorAccessor();
	virtual ~SoundEditorAccessor();
	virtual bool Init();

public:
	//Разрешить-запретить создавать звуки с временных данных
	virtual void EditEnablePreview(bool isEnable);
	//Зарегистрировать звук для предпрослушивания
	virtual void EditRegistrySound(SoundBankFileSound * sbfs);
	//Отменить регистрацию звука для предпрослушивания
	virtual bool EditUnregistrySound(SoundBankFileSound * sbfs);
	//Обновить параметры звука
	virtual bool EditPatchSound(EditPatchSoundData & data, bool isCheckWaves);
	//Остановить проигрывание всех звуков
	virtual void EditStopAllSounds(const char * sceneOvner);

	//Запустить волну на проигрывание 16бит (для звукового редактора)
	virtual bool EditPrewiewPlay(const dword userId[4], EditPrewiewWaveParams * mainWave, EditPrewiewWaveParams * mirrorWave, bool isMirror);
	//Переключить волну на зеркальную или оригинальную
	virtual bool EditPrewiewSwitch(const dword userId[4], bool isMirror);
	//Играет ли волна на прослушивании (для звукового редактора), userId[4] == 0 игнорировать его
	virtual bool EditPrewiewIsPlay(const dword userId[4], dword * samplesCount, bool * playWithMirror);
	//Остановить прослушивание волны, userId[4] == 0 игнорировать его
	virtual void EditPrewiewStop(const dword userId[4]);
	//Установить громкость прослушиваемой волны, userId[4] == 0 игнорировать его
	virtual void EditPrewiewSetVolume(float volume, const dword userId[4]);


private:
	SoundService * soundService;
};


#endif
#endif
#endif

