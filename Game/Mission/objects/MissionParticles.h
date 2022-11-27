//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Mission objects
//============================================================================================
// MissionParticles	
//============================================================================================

#ifndef _MissionParticles_h_
#define _MissionParticles_h_

#include "..\..\..\Common_h\mission\mission.h"
#include "MissionSoundBase.h"


class MissionParticles : public MissionSoundBase
{
//--------------------------------------------------------------------------------------------
public:
	MissionParticles();
	virtual ~MissionParticles();

//--------------------------------------------------------------------
//Создание, обновление
//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
#ifndef MIS_STOP_EDIT_FUNCS
	//Перевести объект в спящий режим
	virtual void EditMode_Sleep(bool isSleep);
	//Показать или скрыть объект в редакторе
	virtual void EditMode_Visible(bool isVisible);
#endif
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Работа партиклов в режиме игры
	void _cdecl Work(float dltTime, long level);
	//Работа партиклов в режиме редактирования
	void _cdecl EditModeWork(float dltTime, long level);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	MO_IS_FUNCTION(MissionParticles, MissionSoundBase)

//--------------------------------------------------------------------------------------------
private:
	//Название партикловой системы
	ConstString name;
	//Позиция партикловой системы
	Matrix matrix;
	//Объект к которому прицепляемся
	MOSafePointer objectPtr;
	ConstString connectToObject;
	//
	bool needUpdate;
	//Не качать партиклы
	bool noSwing;
	//Партикловая система
	IParticleSystem * ps;
	//Моделька отображающая партиклы
	IGMXScene * model;

public:
	static const char * comment;
};

#endif

