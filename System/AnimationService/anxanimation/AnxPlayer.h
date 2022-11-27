//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxAnimation system
//============================================================================================
// AnxPlayer
//============================================================================================

#ifndef _AnxPlayer_h_
#define _AnxPlayer_h_
	
#include "AnxData.h"

class AnxAnimation;

class AnxPlayer
{
	struct Frame
	{
		AnxClip * clip;		//Клип, который проигрываем
		dword pos;			//Позиция проигрывания	
	};

	struct Cache
	{
		Quaternion currQ;
		Vector currP;
		union
		{
			dword globalPos;
			float globalPosFlt;
		};
		Quaternion nextQ;
		Vector nextP;
		dword needUpdate;
	};

//--------------------------------------------------------------------------------------------
public:
	AnxPlayer();
	~AnxPlayer();
	//Выделить память под кэшь
	static dword GetCacheBuffersSize(dword players, dword bonesCount);
	//Инициализировать плеер
	void Init(AnxAnimation * _ani, byte * buffer, dword playerIndex, dword bonesCount);
	//Запустить плеер
	void Start(AnxLink * _link, long _clip = -1, float position = -1.0f);
	//Запустить плеер
	void Start(AnxNode * _node, long _clip = -1, float position = -1.0f);
	//Остоновить плеер
	void Stop();
	//Свободен ли плеер
	bool IsFree();
	//Обновить состояние
	bool Update(float dltTime);
	//Применить анимацию текущего кадра
	void Apply(Quaternion & q, Vector & p, long index, bool isBlend);
	//Получить текущий нод
	AnxNode * Node();

private:
	//Получить значение трека на заданном кадре
	bool GetFrame(AnxTrackHeader * th, dword frame, Quaternion & q, Vector & p);
	//Преобразовать позицию из локальной системы в глобальную
	void ConvertToGlobal(long index, Frame & frame, Vector & position);
	//Преобразовать позицию из глобальной системы коордитнат в локальную
	void ConvertToLocal(long index, Vector & position);

public:
	//Применить анимацию текущего кадра
	void Movement(Vector & deltaPos);
	//Установить выбранный линк
	bool SelectLink(AnxLink * link, bool forceApply, bool onlyCheck);
	//Получить активированный линк
	AnxLink * GetLink();

	//Установить случайную позицию
	void RandPosition();
	//Получить текущую отновительную позицию
	float GetPosition();

	//Установить текущий кадр
	void SetCurrentFrame(float frame);
	//Получить текущий кадр
	float GetCurrentFrame();

	//Описание текущего кадра
	const Frame & CurrentFrame();
	//Коэфициент действия
	float GetKBlend();

	//Являеться ли текущий нод стоповым
	bool CurrentNodeIsStop();

	//Перебить скорость проигрывания в текущем клипе (редактор)
	void SetFPS(float fps);

	

//--------------------------------------------------------------------------------------------
private:
	//Выбрать следующий кадр
	void NextFrame();
	//Испачкать кэшь
	void DirtyCache();
	//Выбрать клип в соответствии с вероятностями
	static long SelectClip(AnxNode * node);

//--------------------------------------------------------------------------------------------
private:
	AnxAnimation * ani;	//Анимация
	AnxNode * node;		//Нод, который проигрываем
	Frame currFrame;	//Текущий кадр
	Frame nextFrame;	//Следующий кадр
	float fposition;	//Межкадровая позиция проигрывания
	AnxLink * link;		//Линк по которому переходим
	float kBlend;		//Коэфициент влияния
	bool isActive;		//Активирован ли линк перехода
	bool isSendEvent;	//Разрешено отправлять события
	bool isNextFrame;	//Был ли переход на следующий кадр
	AnxLink * selSink;	//Линк по которому перейдём с нода
	Vector frameDltPos;	//Изменение позиции за кадр анимации
	Vector curDltPos;	//Текущая позиция в клипе анимации
	Vector oldDltPos;	//Учтенная позиция в клипе анимации
	float eventTime;	//Текущее время
	Cache * cacheBones;	//Буфер кэширования костей
	dword cacheCounter;	//Счётчик порчи кэша
};

//Получить текущий нод
inline AnxNode * AnxPlayer::Node()
{
	return node;
}

//Свободен ли плеер
inline bool AnxPlayer::IsFree()
{
	return node == null;
}

//Описание текущего кадра
inline const AnxPlayer::Frame & AnxPlayer::CurrentFrame()
{
	return currFrame;
}

//Являеться ли текущий нод стоповым
inline bool AnxPlayer::CurrentNodeIsStop()
{
	if(!node) return true;
	return (node->flags & AnxNode::isStop) != 0;
}

//Коэфициент действия
inline float AnxPlayer::GetKBlend()
{
	return kBlend;
}

//Перебить скорость проигрывания в текущем клипе (редактор)
inline void AnxPlayer::SetFPS(float fps)
{
	if(currFrame.clip)
	{
		currFrame.clip->fps = fps;
	}
}


#endif

