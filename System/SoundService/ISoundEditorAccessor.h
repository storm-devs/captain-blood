
#ifndef _ISoundEditorAccessor_h_
#define _ISoundEditorAccessor_h_

#include "SoundBankFile.h"

struct EditPatchSoundData
{
	char soundName[16];				//Имя звука	
	dword nameHash;					//Хэшь имени звука
	dword nameLen;					//Длинна имени
	dword soundId[4];				//Идентификатор звука
	dword setupId[4];				//Идентификатор базовых параметров
	dword attId[4];					//Идентификатор графика затухания
	long priority;					//Приоритет по отношению к остальным звукам
	long maxCount;					//Максимальное количество одновременно проигрываемых данных звуков
	float c[4];						//Коэфициенты графика затухания
	float minDist2;					//Минимальныя дистанция в квадрате
	float maxDist2;					//Максимальныя дистанция в квадрате
	float kNorm2;					//Коэфициент нормализации в квадрате 1.0/(maxDist2 - minDist2)
	dword wavesCount;				//Количество волн в звуке
	struct WaveParams
	{
		dword waveId[4];			//Идентификатор волны, на которую ссылаеться звук
		float volume;				//Громкость с какой проигрывать волну
		float probability;			//Вероятность выбора волны (нормализованное значение)
		float playTime;				//Время активности волны (время проигрывание волны или тишины)
	} waves[256];
};


struct EditPrewiewWaveParams
{
	byte * data;			//Указатель на данные
	dword dataSize;			//Размер данные в байтах
	dword sampleRate;		//Частота сэмплирования
	bool isStereo;			//Стерео или моно
};

class ISoundEditorAccessor : public Service
{
public:
	//Разрешить-запретить создавать звуки с временных данных
	virtual void EditEnablePreview(bool isEnable) = null;
	//Зарегистрировать звук для предпрослушивания
	virtual void EditRegistrySound(SoundBankFileSound * sbfs) = null;
	//Отменить регистрацию звука для предпрослушивания
	virtual bool EditUnregistrySound(SoundBankFileSound * sbfs) = null;
	//Обновить параметры звука
	virtual bool EditPatchSound(EditPatchSoundData & data, bool isCheckWaves) = null;
	//Остановить проигрывание всех звуков
	virtual void EditStopAllSounds(const char * sceneOvner) = null;

	//Запустить волну на проигрывание 16бит (для звукового редактора)
	virtual bool EditPrewiewPlay(const dword userId[4], EditPrewiewWaveParams * mainWave, EditPrewiewWaveParams * mirrorWave, bool isMirror) = null;
	//Переключить волну на зеркальную или оригинальную
	virtual bool EditPrewiewSwitch(const dword userId[4], bool isMirror) = null;
	//Играет ли волна на прослушивании (для звукового редактора), userId[4] == 0 игнорировать его
	virtual bool EditPrewiewIsPlay(const dword userId[4], dword * samplesCount = null, bool * playWithMirror = null) = null;
	//Остановить прослушивание волны, userId[4] == 0 игнорировать его
	virtual void EditPrewiewStop(const dword userId[4]) = null;
	//Установить громкость прослушиваемой волны, userId[4] == 0 игнорировать его
	virtual void EditPrewiewSetVolume(float volume, const dword userId[4]) = null;

};

#endif

