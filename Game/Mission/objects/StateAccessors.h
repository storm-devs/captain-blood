//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// Mission objects
//============================================================================================
// StateAccessors
//============================================================================================

#ifndef _StateAccessors_h_
#define _StateAccessors_h_

#include "..\..\..\Common_h\mission\Mission.h"


//============================================================================================
//StateAccessor
//============================================================================================

class StateDynamicName;

class StateAccessor : public MissionObject
{
public:
	StateAccessor();
	virtual ~StateAccessor(){};

	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader){ return true; };
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader){ return true; };
	//Пересоздать объект
	virtual void Restart(){ ReCreate(); };
	//Получить текущие имя стейта
	const char * GetName(const char * name, bool useAccessFilter);

protected:
	const char * name;

private:
	string tempName;
};

//============================================================================================
//StateGetParameter
//============================================================================================

class StateVariable : public StateAccessor
{
	enum Mode
	{
		tm_param,
		tm_string,
		tm_int,
		tm_float_X_1,
		tm_float_X_2,
		tm_float_X_3,
		tm_float_X_4,
		tm_case_id,
		tm_case_string,		
		tm_count,
		tm_force = 0x7fffffff
	};

	struct EnumParams
	{
		Mode mode;
		ConstString name;
	};

public:
	StateVariable();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Проверить тип
	MO_IS_FUNCTION(StateVariable, StateAccessor)
	
	//Получить параметер
	const char * GetParameter();

private:
	Mode mode;					//Режим объясняющий как работать с именем
	const char * param;			//Имя параметра
	array<const char *> table;	//Таблица строк или идентификаторов
	string parameter;			//Прочитаный параметр

public:
	static const char * comment;
	static const EnumParams tms[tm_count];
};


//============================================================================================
//StateEmpty
//============================================================================================

class StateEmpty : public StateAccessor
{
public:	

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	
};

//============================================================================================
//StateCopy
//============================================================================================

class StateCopy : public StateAccessor
{
public:	
	StateCopy();
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

protected:
	const char * name2;
};

//============================================================================================
//StateStringSet, StateStringCheck
//============================================================================================

class StateStringSet : public StateAccessor
{
public:	

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	const char * initState;
};



class StateStringCheck : public StateAccessor
{
public:	

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	const char * value;
	MissionTrigger triggerEq;
	MissionTrigger triggerNEq;
	bool inActiveProcess;
};


//============================================================================================
//StateNumberSet, StateNumberCheck
//============================================================================================

class StateNumberSet : public StateAccessor
{
public:	

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	float initState;

};


class StateNumberCheck : public StateAccessor
{
public:
	enum Condition
	{
		c_less = 0,		
		c_less_eq = 1,
		c_eq = 2,
		c_above_eq = 3,
		c_above = 4,		
		c_size = 5
	};

	struct EnumParams
	{
		Condition cnd;
		ConstString name;
	};

public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	float value;
	const char * valueName;
	Condition cnd;
	MissionTrigger trigger;
	bool inActiveProcess;

public:
	static const char * comment;
	static const EnumParams cnds[c_size];
};

class StateNumberOperation : public StateAccessor
{
	enum Operation
	{
		o_add = 0,
		o_sub_s_v = 1,
		o_sub_v_s = 2,
		o_mul = 3,
		o_div_s_v = 4,
		o_div_v_s = 5,
		o_size = 6
	};

	struct EnumParams
	{
		Operation op;
		ConstString name;
	};

public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	const char * name2;
	float value;
	const char * valueName;
	float minResult;
	float maxResult;
	bool useLimits;	
	Operation op;

public:
	static const char * comment;
	static const EnumParams ops[o_size];
};


class StateNumberGet : public StateAccessor
{
public:
	struct Param
	{		
		const char * str;		//Параметр
		bool needTranslate;		//Нужно или нет преобразовывать
	};

public:
	StateNumberGet();
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	MOSafePointer mo;
	const char * command;
	long numParams;
	Param params[8];
	array<char> buffer;
	ConstString objName;

public:
	static const char * comment;
};


//============================================================================================
//StateAccessor
//============================================================================================

class MissionTestConfigurator : public MissionObject
{
public:
	MissionTestConfigurator();
	virtual ~MissionTestConfigurator();
private:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

private:
	static long isCreated;
};


#endif
