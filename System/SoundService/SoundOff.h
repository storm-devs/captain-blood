
#ifndef _SoundOff_h_
#define _SoundOff_h_

#include "..\..\common_h\Sound.h"


class SoundSceneOff;

//Звук
class SoundOff : public ISound3D
{
public:

	SoundOff(SoundSceneOff & s, const char * n, const char * cppf, long cppl) : scene(s)
	{
		name = n;
		cppFile = cppf;
		cppLine = cppl;
		pos = vel = 0.0f;
	}
	virtual ~SoundOff(){ RemoveFromList(); }


	//Удалить звук
	virtual void Release(){ delete this; };

	//Проиграть
	virtual void Play(){};
	//Остановить
	virtual void Stop(){};
	//Узнать текущее состояние
	virtual bool IsPlay(){ return false; };

	//Установить громкость звука
	virtual void SetVolume(float volume){};
	//Получить громкость звука
	virtual float GetVolume(){ return 0.0f; };

	//Получить имя звука
	virtual const char * GetName(){ return name.c_str(); };
	//Получить имя звукового банка
	virtual const char * GetSoundBankName(){ return SoundService_GlobalSoundBank; };

	//Установить позицию источника звука
	virtual void SetPosition(const Vector & position){ pos = position; };
	//Получить позицию источника звука
	virtual Vector GetPosition(){ return pos; };
	//Установить скорость источника звука
	virtual void SetVelocity(const Vector & velocity){ vel = velocity; };
	//Получить скорость источника звука
	virtual Vector GetVelocity(){ return vel; };
	//Запустить звук проигрываться и плавно увеличить громкость за time секунд
	virtual void FadeIn(float time){};
	//Плавно уменьшить громкость до 0 за time секунд и остановить проигрывание звука
	virtual void FadeOut(float time){};

	//Узнать, зациклен звук или нет
	virtual bool IsLoop(){ return false; };

	//Вернуть идентификатор фонемы. Если отрицательный результат - фонемы нет
	virtual long GetPhonemeId(ErrorCode * ecode)
	{
		if(ecode) *ecode = ec_phonemes_sound_not_play; 
		return -1; 
	};

	void RemoveFromList();

	string name;
	Vector pos;
	Vector vel;
	const char * cppFile;
	long cppLine;
	SoundSceneOff & scene;
};


//Звуковая сцена
class SoundSceneOff : public ISoundScene
{
public:

	SoundSceneOff() : sounds(_FL_, 1024){ sounds.Reserve(1024); };
	virtual ~SoundSceneOff()
	{
		while(sounds)
		{
			SoundOff * snd = sounds[sounds - 1];
			api->Trace("Ureleased sound found \"%s\" (cpp file: %s, line: %i)", snd->name.c_str(), snd->cppFile, snd->cppLine);
			delete snd;
		}
	}

	//Удалить сцену
	virtual void Release(){};

	//Создать непозиционированный звук, если звук не создался - вернётся null
	virtual ISound * Create(const ConstString & soundName, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, float vol)
	{
		if(!autoDelete) 
		{
			SoundOff * s = NEW SoundOff(*this, soundName.c_str(), _cppFile, _cppLine);
			sounds.Add(s);
			return s;
		}
		return null;
	}

	//Создать позиционированный в 3D звук, если звук не создался - вернётся null
	virtual ISound3D * Create3D(const ConstString & soundName, const Vector & worldPos, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, float vol)
	{
		if(!autoDelete) 
		{
			SoundOff * s = NEW SoundOff(*this, soundName.c_str(), _cppFile, _cppLine);
			s->pos = worldPos;
			sounds.Add(s);
			return s;
		}
		return null;
	}


	//Продолжить звучание сцены
	virtual void Resume(){};
	//Приостановить звучание сцены
	virtual void Pause(){};
	//Узнать текущее состояние сцены
	virtual bool IsPause(){ return false; };

	//Установить общую громкость звучание всей сцены
	virtual void SetVolume(float volume){};
	//Получить общую громкость звучание всей сцены
	virtual float GetVolume(){ return 0.0f; };

	//Установить позицию слушателя
	virtual void SetListenerMatrix(const Matrix & mtx){};
	//Получить позицию слушателя
	virtual void GetListenerMatrix(Matrix & mtx){};

	//Установить текущую окружающую сцену с временем перехода в секундах
	virtual void SetSoundEnvironmentScene(const Enveronment * envPeset, float blendTime){};
	//Получить текущую окружающую сцену
	virtual void GetSoundEnvironmentScene(Enveronment & envPeset){};

	//Переименовать владельца сцены
	virtual void ModifyOvnerName(const char * ovnerName){};	

	//Нарисовать отладочную информацию
	virtual void DebugDraw(){};

	array<SoundOff *> sounds;

};


__forceinline void SoundOff::RemoveFromList()
{
	scene.sounds.Del(this);
}

#endif

