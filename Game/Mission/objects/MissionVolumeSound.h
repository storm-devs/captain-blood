
#ifndef _MissionVolumeSound_h_
#define _MissionVolumeSound_h_

#include "MissionSoundBase.h"

class MissionVolumeSound : public MissionSoundBase
{
	enum Flags
	{
		f_init = 0,
		f_is3D = 1,				//3D звук
		f_isSphere = 2,			//Sphere/box
		f_isManageByDist = 4,	//Менеджировать 3D звук по дистанции или оставить работу сервису		
	};

	struct Debug
	{
		float curVolume;	//Текущая громкость
		float dist;			//Дистанция
		Vector sndPos;		//Позиция звука
		Vector listPos;		//Позиция слушателя
	};
public:
	MissionVolumeSound();
	virtual ~MissionVolumeSound();

	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

	//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Работа
	virtual void _cdecl Work(float dltTime, long level);
	//Рисование в режиме редактирования
	void _cdecl EditModeDraw(float dltTime, long level);

private:
	Matrix matrix;		//Матрица позиции ящика
	float radius;		//Радиус сферы
	Vector size;		//Половинный размер ящика
	float fadeDist2;	//Дистанция затухания для 2D звуков в ^2
	float kFadeDist;	//Обратная величина от дистанции затухания
	Debug * debug;		//Структура для отладки
	dword flags;	
	//Объект к которому прицепляемся
	MOSafePointer objectPtr;
	ConstString connectToObject;
	
};


#endif

