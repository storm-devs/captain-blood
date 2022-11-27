

#ifndef _MissionRandomSound_h_
#define _MissionRandomSound_h_


#include "MissionSoundBase.h"


class MissionRandomSound : public MissionSoundBase
{
	struct SoundVolume
	{
		Vector size;
		Matrix mtx;
	};

	//--------------------------------------------------------------------------------------------
public:
	MissionRandomSound();
	virtual ~MissionRandomSound();

	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Рисование модельки в режиме редактирования
	void _cdecl EditModeDraw(float dltTime, long level);
	//Работа
	virtual void _cdecl Work(float dltTime, long level);

private:
	//Найти произвольную позицию
	Vector GetRandomPosition();

public:
	array<SoundVolume> volumes;
};

#endif
