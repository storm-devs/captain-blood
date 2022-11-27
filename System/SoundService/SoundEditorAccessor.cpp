
#include "SoundEditorAccessor.h"

#ifndef _XBOX
#ifndef NO_TOOLS


CREATE_SERVICE_NAMED("SoundEditorAccessor", SoundEditorAccessor, 1000)


SoundEditorAccessor::SoundEditorAccessor()
{
	soundService = null;
}

SoundEditorAccessor::~SoundEditorAccessor()
{

}

bool SoundEditorAccessor::Init()
{
	soundService = (SoundService *)api->GetService("SoundService");
	return (soundService != null);
}

//Разрешить-запретить создавать звуки с временных данных
void SoundEditorAccessor::EditEnablePreview(bool isEnable)
{
	soundService->EditEnablePreview(isEnable);
}
//Зарегистрировать звук для предпрослушивания
void SoundEditorAccessor::EditRegistrySound(SoundBankFileSound * sbfs)
{
	soundService->EditRegistrySound(sbfs);
}
//Отменить регистрацию звука для предпрослушивания
bool SoundEditorAccessor::EditUnregistrySound(SoundBankFileSound * sbfs)
{
	return soundService->EditUnregistrySound(sbfs);
}
//Обновить параметры звука
bool SoundEditorAccessor::EditPatchSound(EditPatchSoundData & data, bool isCheckWaves)
{
	return soundService->EditPatchSound(data, isCheckWaves);
}
//Остановить проигрывание всех звуков
void SoundEditorAccessor::EditStopAllSounds(const char * sceneOvner)
{
	soundService->EditStopAllSounds(sceneOvner);
}

//Запустить волну на проигрывание 16бит (для звукового редактора)
bool SoundEditorAccessor::EditPrewiewPlay(const dword userId[4], EditPrewiewWaveParams * mainWave, EditPrewiewWaveParams * mirrorWave, bool isMirror)
{
	return soundService->Engine().EditPrewiewPlay(userId, mainWave, mirrorWave, isMirror);
}
//Переключить волну на зеркальную или оригинальную
bool SoundEditorAccessor::EditPrewiewSwitch(const dword userId[4], bool isMirror)
{
	return soundService->Engine().EditPrewiewSwitch(userId, isMirror);
}
//Играет ли волна на прослушивании (для звукового редактора), userId[4] == 0 игнорировать его
bool SoundEditorAccessor::EditPrewiewIsPlay(const dword userId[4], dword * samplesCount, bool * playWithMirror)
{
	return soundService->Engine().EditPrewiewIsPlay(userId, samplesCount, playWithMirror);
}
//Остановить прослушивание волны, userId[4] == 0 игнорировать его
void SoundEditorAccessor::EditPrewiewStop(const dword userId[4])
{
	soundService->Engine().EditPrewiewStop(userId);
}
//Установить громкость прослушиваемой волны, userId[4] == 0 игнорировать его
void SoundEditorAccessor::EditPrewiewSetVolume(float volume, const dword userId[4])
{
	soundService->Engine().EditPrewiewSetVolume(volume, userId);
}



#endif
#endif