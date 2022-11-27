

#ifndef _ActorTimeLine_h_
#define _ActorTimeLine_h_


#include "..\..\Character\CharacterController.h"

class ActorTimeLine : public CharacterControllerParams
{
	enum Type
	{
		t_unknow = 0,
		t_wait,
		t_arrive,
		t_move,
		t_change,
		t_goto,
	};

	class MiniConstString
	{
	public:
		__forceinline MiniConstString & operator = (const ConstString & constString)
		{
			str = constString.c_str();
			return *this;
		}
		__forceinline bool NotEmpty() const
		{
			return string::NotEmpty(str);
		}

		__forceinline const char * c_str() const
		{
			return str;
		}

	private:
		const char * str;
	};

	struct TimePoint
	{
		MiniConstString linkName;
		MiniConstString node;
		float blendTime;
		Type type;
		float time;
		
		//Matrix init_transform;		
		Vector init_pos;
		float init_ay;

		Vector pos;
		float ay;

		word gotoPoint;
		bool isEnableGoto;
		bool isPreview;
		MissionTrigger event;

		Matrix & GetInitTransform(Matrix & mtx)
		{
			mtx.BuildRotateY(init_ay);
			mtx.pos = init_pos;
			return mtx;
		}
	};

public:
	struct ActorData
	{
	public:
		friend class ActorTimeLine;
		ActorData();
		long GetCurrentPosition()
		{
			return long(currentPosition);
		}
	public:
		IN MissionObject * chr;		//Текущий персонаж
		IN IAnimation * ani;		//Анимация актёра
		IN Vector playerPosition;	//Позиция персонажа
		IN float playerAngle;		//Угол персонажа		
		OUT Vector pos;				//Текущая мировая позиция
		OUT float ay;				//Текущий угол
	
		float currentPosition;		//Текущая позиция на таймлайне
		const char * currentNode;	//Текущий анимационный нод	
		Matrix transform;
	};


	const char* actor_init_node;

public:
	ActorTimeLine();
	virtual ~ActorTimeLine();

private:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update(MOPReader & reader);
	//Указать поддержку заданного типа 
	virtual bool IsControllerSupport(const ConstString & controllerClassName);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	//Получить следующую точку относительно текущей
	long GetNextPoint(long current);

	Vector GetPos(ActorData & data,TimePoint& pt);
	float  GetAy(ActorData & data,TimePoint& pt);
	void   GetTransform(Matrix& mat,ActorData & data,TimePoint& pt);

public:
	//Установить персонажа в исходную позицию
	void Reset(ActorData & data, Matrix& mat);
	//Переместить персонажа
	void Move(ActorData & data, float dltTime);
	//Перейти на новую точку таймлайна
	void Goto(ActorData & data, long pointIndex);
	//Установить персонажа для отрисовки
	void SetDrawCharacter(Character * chr);
	//Проверка заданного типа
	MO_IS_FUNCTION(ActorTimeLine, CharacterControllerParams);


private:
	//Рисование точек
	void _cdecl Draw(float dltTime, long level);

private:
	array<TimePoint> timeLine;
	Character*    drawCharacter;
	MOSafePointer drawCharacterPtr;
	bool isMovePoint;
	bool teleportToChar;

	float cur_rot_time;
	float rot_time;

public:
	static const char * comment;

	bool IsTimeLineFinished(ActorData & data);
};



#endif

