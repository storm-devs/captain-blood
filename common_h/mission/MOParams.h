
#ifndef _MissionObjectParams_h_
#define _MissionObjectParams_h_

#include "..\data_swizzle.h"

/*

Объявление списка параметров:
	MOP_BEGINLIST(Entity, name, ver, level)
		Начало списка параметров
		MOP_...
		...
		MOP_...
		Окончание списка параметров
	MOP_ENDLIST(Entity)

Типы:
	//bool
	MOP_BOOL(name, def)
	//long
	MOP_LONG(name, def)
	MOP_LONGEX(name, def, min, max) Ограниченный диапазон
	//float
	MOP_FLOAT(name)
	MOP_FLOATEX(name, def, min, max) Ограниченный диапазон
	//string
	MOP_STRING(name, def)
	MOP_STRINGEX(name, def, min, max) Ограничение по длинне
	MOP_STRINGEXLO(name, def, min, max) Ограничение по длинне + только маленькие буквы
	MOP_STRINGEXHI(name, def, min, max) Ограничение по длинне + только большие буквы
	//position
	MOP_POSITION(name, def)
	MOP_POSITIONEX(name, def, min, max) Ограниченный диапазон
	//angle
	MOP_ANGLES(name)
	MOP_ANGLESEX(name, def, min, max) Ограниченный диапазон
	//color
	MOP_COLOR(name, def)
	MOP_COLOREX(name, def, min, max) Ограниченный диапазон
	//enum
	MOP_ENUM(type, name)


Объявление массивов:
	MOP_ARRAYBEG(name, min, max)
		//Начало элементи массива
		MOP_...
		...
		MOP_...
		//Окончание элементи массива
	MOP_ARRAYEND


Объявление enum:
	MOP_ENUMBEG(type)
		MOP_ENUMELEMENT(value)
		...
		MOP_ENUMELEMENT(value)
	MOP_ENUMEND



Примеры:

	//class MyEntityDetectors : public MissionObject
	MOP_BEGINLIST(MyEntityDetectors, "Super-puper detectors" '1.00', 1000)
		MOP_ENUMBEG("events")
			MOP_ENUMELEMENT("start")
			MOP_ENUMELEMENT("stop")
			MOP_ENUMELEMENT("reset")
		MOP_ENUMEND
		
		MOP_ARRAYBEG("Point detectors", 1, 50)
			MOP_POSITION("position", Vector(0.0f, 1.0f, 2.0f))
			MOP_FLOATEX("radius", 1.0f, 0.01, 100)
			MOP_ENUM("events", "send event");
			MOP_ARRAYBEG("event params", 0, 5)
				MOP_STRING("param", "Hello");
			MOP_ARRAYEND
		MOP_ARRAYEND

		MOP_ARRAYBEG("Quad detectors", 1, 50)
			MOP_POSITION("position", Vector(0.0f, 1.0f, 2.0f))
			MOP_ANGLES("angles", Vector(0.0f, 1.0f, 2.0f))
			MOP_FLOATEX("width", 10.0f, 0.01, 100)
			MOP_FLOATEX("height", 10.0f, 0.01, 100)
			MOP_ENUM("events", "send event");
			MOP_ARRAYBEG("event params", 0, 5)
				MOP_STRING("param", "Bla-bla");
			MOP_ARRAYEND
		MOP_ARRAYEND

	MOP_ENDLIST

*/






#include "..\core.h"
#include "..\LocStrings.h"
#include "..\Templates\Array.h"
#include "..\Templates\Stack.h"
#include "..\Templates\String.h"
#include "MissionFile.h"
class MissionObject;

//Описание параметров миссионого объекта
class IMOParams : public RegObject
{
//--------------------------------------------------------------------------------------------
//Варианты типов
//--------------------------------------------------------------------------------------------
public:

	enum Type
	{
		t_bool = 0,
		t_long,
		t_float,
		t_string,
		t_locstring,
		t_position,
		t_angles,
		t_color,
		t_array,
		t_enum,
		t_group,
	};

//--------------------------------------------------------------------------------------------

#pragma pack(push, 1)
//--------------------------------------------------------------------------------------------
//Описание типов
//--------------------------------------------------------------------------------------------
public:

	//Базовые параметры
	struct Param
	{
		virtual ~Param(){};

		const char * name;
		const char * comment;
		Type type;
		bool isLimit;
		bool isHide;
	};

	//Булевский параметр
	struct Bool : public Param
	{
		bool def;
	};

	//Целочисленный параметр
	struct Long : public Param
	{
		long def;
		long min;
		long max;
	};

	//Численный параметр
	struct Float : public Param
	{
		float def;
		float min;
		float max;
	};

	//Строковой параметр
	struct String : public Param
	{
		const char * def;
		long minChars;				//Минимальная длинна строки
		long maxChars;				//Максимальная длинна строки
		union
		{
			dword flags;			//Флаги
			struct
			{
				dword onlyLo : 1;	//Только маленькие буквы
				dword onlyHi : 1;	//Только большие буквы
			};
		};
	};

	//Позиция
	struct Position : public Param
	{
		Vector def;
		Vector min;
		Vector max;
	};

	//Углы
	struct Angles : public Param
	{
		Vector def;
		Vector min;
		Vector max;
	};

	//Цвет
	struct Colors : public Param
	{
		Color def;
		Color min;
		Color max;
	};

	//Массив
	struct Array : public Param
	{
		long min;				//Минимальное количество элементов в массиве
		long max;				//Максимальное количество элементов в массиве
		array<Param *> element;	//Описание элемента массива

		Array() : element(_FL_)
		{
		};

		~Array()
		{
			for(long i = 0; i < element; i++)
			{
				delete element[i];
			}
			element.Empty();
		}
	};

	//Список перечислений (сток)
	struct Enum : public Param
	{
		Enum() : element(_FL_){};
		array<const char *> element;	//Описание элемента массива
	};

	//Массив
	struct Group : public Param
	{
		array<Param *> element;	//Описание элемента массива

		Group() : element(_FL_){};

		~Group()
		{
			for(long i = 0; i < element; i++)
			{
				delete element[i];
			}
			element.Empty();
		}

	};

	//Численный параметр
	struct LocString : public Param
	{
	};

#pragma pack(pop)


//--------------------------------------------------------------------------------------------	
//Доступ к типам
//--------------------------------------------------------------------------------------------
public:	
	//Получить entity объекта
	const char * _fastcall GetEntity() const { return type; };
	//Получить имя объекта, который описавает данный класс
	const char * _fastcall GetName() const { return name; };
	//Получить версию
	dword _fastcall GetVersion(){ return version; };
	//Получить уровень инициализации
	long _fastcall GetLevel(){ return level; };
	//Получить количество параметров
	long _fastcall GetNumParams() const { return params; };
	//Найти параметр по индексу
	const Param * _fastcall FindParam(const char * _name) const { return FindParamByName(_name, params.GetBuffer(), params.Size()); };
	//Получить параметр
	const Param * _fastcall GetParam(long index) const { return params[index]; };
	//Получить коментарий
	const char * _fastcall GetComment(){ return comment; };
	//Получить имя группы
	const char * _fastcall GetGroup(){ return group; };
	
//--------------------------------------------------------------------------------------------
//Внутреннее
//--------------------------------------------------------------------------------------------
protected:
	
	void SetType(const char * _type)
	{
		type = _type;
	};

	void SetName(const char * _name)
	{
		name = _name;
	};

	void SetVersion(dword v)
	{
		version = v;
	};

	void SetLevel(long _level)
	{
		level = _level;
	};

	void SetComment(const char * _comment)
	{
		comment = _comment;
	};

	void SetGroup(const char * _group)
	{
		group = _group;
	};

	const Param * _fastcall FindParamByName(const char * _name, const Param * const * prm, dword sz) const
	{
		for(dword i = 0; i < sz; i++)
		{
			if(string::IsEqual(prm[i]->name, _name))
			{
				return prm[i];
			}
			if(prm[i]->type == t_array)
			{
				const Array * arr = (const Array *)prm[i];
				const Param * p = FindParamByName(_name, arr->element.GetBuffer(), arr->element.Size());
				if(p) return p;
			}else
			if(prm[i]->type == t_group)
			{
				const Group * grp = (const Group *)prm[i];
				const Param * p = FindParamByName(_name, grp->element.GetBuffer(), grp->element.Size());
				if(p) return p;				
			}
		}
		return null;
	}
	

	void AddBool(const char * name, bool def, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Bool * v = NEW Bool();
		v->type = t_bool;
		v->name = name;
		v->isLimit = false;
		v->isHide = hide;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddLong(const char * name, long def, bool isLim, long min, long max, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Long * v = NEW Long();
		v->type = t_long;
		v->name = name;
		v->isLimit = isLim;
		v->isHide = hide;
		v->min = min;
		v->max = max;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddFloat(const char * name, float def, bool isLim, float min, float max, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Float * v = NEW Float();
		v->type = t_float;
		v->name = name;
		v->isLimit = isLim;
		v->isHide = hide;
		v->min = min;
		v->max = max;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddString(const char * name, const char * def, bool isLim, long min, long max, bool isLo, bool isHi, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		String * v = NEW String();
		v->type = t_string;
		v->name = name;
		v->isLimit = isLim;
		v->isHide = hide;
		v->minChars = min;
		v->maxChars = max;
		v->onlyLo = isLo ? 1 : 0;
		v->onlyHi = isHi ? 1 : 0;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddPosition(const char * name, Vector def, bool isLim, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Position * v = NEW Position();
		v->type = t_position;
		v->name = name;
		v->isLimit = isLim;
		v->isHide = hide;
		v->min.x = minX;
		v->min.y = minY;
		v->min.z = minZ;
		v->max.x = maxX;
		v->max.y = maxY;
		v->max.z = maxZ;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddAngles(const char * name, Vector def, bool isLim, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Angles * v = NEW Angles();
		v->type = t_angles;
		v->name = name;
		v->isLimit = isLim;
		v->isHide = hide;
		v->min.x = minX;
		v->min.y = minY;
		v->min.z = minZ;
		v->max.x = maxX;
		v->max.y = maxY;
		v->max.z = maxZ;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddColor(const char * name, Color def, bool isLim, float minR, float minG, float minB, float minA, float maxR, float maxG, float maxB, float maxA, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Colors * v = NEW Colors();
		v->type = t_color;
		v->name = name;
		v->isLimit = isLim;
		v->isHide = hide;
		v->min.r = minR;
		v->min.g = minG;
		v->min.b = minB;
		v->min.a = minA;
		v->max.r = maxR;
		v->max.g = maxG;
		v->max.b = maxB;
		v->max.a = maxA;
		v->def = def;
		v->comment = comment;
		params.Add(v);
	};

	void AddArrayBeg(const char * name, long min, long max, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Info * inf = NEW Info();
		inf->name = name;
		inf->min = min;
		inf->max = max;
		inf->hide = hide;
		inf->comment = comment;
		inf->start = params;
		inf->cur = (groups > 0) ? groups.Top()->name : null;
		arrays.Push(inf);
	};

	void AddArrayEnd()
	{
		Assert(enumName == 0);
		Info * inf = arrays.Top(); arrays.Pop();
		//Массив закончен за пределами начатой группы
		if(inf->cur)
		{
			Assert(groups > 0);//^
			Assert(groups.Top()->name == inf->cur);//^
		}else{
			Assert(groups == 0);//^
		}
		Array * v = NEW Array();
		v->type = t_array;
		v->name = inf->name;
		v->isLimit = true;
		v->isHide = inf->hide;
		v->comment = inf->comment;
		v->min = inf->min;
		v->max = inf->max;
		for(long i = inf->start; i < params; i++)
		{
			v->element.Add(params[i]);
		}
		while(params > inf->start) params.DelIndex(params - 1);
		params.Add(v);
		delete inf;
	};

	void SetEnumBeg(const char * type)
	{
		Assert(enumName == null);
		Assert(type != null);
		enumName = type;
		enumList.Empty();
	};

	void EnumElement(const char * value)
	{
		Assert(enumName != null);
		Assert(value != null);
		enumList.Add(value);
	};

	void SetEnumEnd()
	{
		Assert(enumName != null);
		Info * ei = NEW Info();
		ei->name = enumName;
		for(long i = 0; i < enumList; i++) ei->list.Add(enumList[i]);
		enuminfos.Add(ei);
		enumName = null;
		enumList.Empty();
	};

	void AddEnum(const char * name, const char * type, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Assert(type != 0);
		for(long i = 0; i < enuminfos; i++)
		{
			if(string::IsEqual(enuminfos[i]->name, type)) break;
		}
		Assert(i < enuminfos);
		Enum * v = NEW Enum();
		v->type = t_enum;
		v->name = name;
		v->isLimit = false;
		v->isHide = hide;
		v->comment = comment;
		for(long j = 0; j < enuminfos[i]->list; j++) v->element.Add(enuminfos[i]->list[j]);
		params.Add(v);
	};

	void GroupBeg(const char * name, const char * comment, bool hide)
	{
		Assert(enumName == 0);
		Assert(name != 0);
		Info * inf = NEW Info();
		inf->name = name;
		inf->min = 0;
		inf->max = 0;
		inf->hide = hide;
		inf->comment = comment;
		inf->start = params;
		inf->cur = (arrays > 0) ? arrays.Top()->name : null;
		groups.Push(inf);
	};

	void GroupEnd()
	{
		Assert(enumName == 0);
		Info * inf = groups.Top(); groups.Pop();
		//Массив закончен за пределами начатой группы
		if(inf->cur)
		{
			Assert(arrays > 0);//^
			Assert(arrays.Top()->name == inf->cur);//^
		}else{
			Assert(arrays == 0);//^
		}
		Group * v = NEW Group();
		v->type = t_group;
		v->name = inf->name;
		v->isLimit = false;
		v->isHide = inf->hide;
		v->comment = inf->comment;
		for(long i = inf->start; i < params; i++)
		{
			v->element.Add(params[i]);
		}
		while(params > inf->start) params.DelIndex(params - 1);
		params.Add(v);
		delete inf;
	};

	void AddLocString(const char * name, const char * comment, bool hide)
	{
		Assert(name != 0);
		LocString * v = NEW LocString();
		v->type = t_locstring;
		v->name = name;
		v->isLimit = false;
		v->isHide = hide;
		v->comment = comment;
		params.Add(v);
	};


	void EndCreation()
	{
		Assert(arrays.Size() == 0);
		for(long i = 0; i < enuminfos; i++) delete enuminfos[i];
		enuminfos.Empty();
		enumList.Empty();		
	};


	struct Info
	{
		Info() : list(_FL_){ name = null; cur = null; comment = null; };

		const char * name;
		array<const char *> list;
		long min;
		long max;
		bool hide;
		long start;
		const char * cur;
		const char * comment;
	};

	IMOParams() : params(_FL_), 
		          enuminfos(_FL_),
				  enumList(_FL_),
				  arrays(_FL_),
				  groups(_FL_)
	{ 
		name = 0; 
		enumName = 0;
		curArray = null;
		curGroup = null;
		comment = null;
		group = null;
		//Пришлось сделать хак, чтобы тригера нормально работали
		SetEnumBeg("__TripleLogic__");
		EnumElement("ignore");
		EnumElement("true");
		EnumElement("false");
		SetEnumEnd();
	};	
	virtual bool Init(){ return true; };
	virtual ~IMOParams()
	{
		for(long i = 0; i < params; i++)
		{
			delete params[i];
			params[i] = 0;
		}
		params.Empty();
	};

private:
	const char * type;				//Имя entity объекта
	const char * name;				//Имя объекта
	dword version;					//Версия объекта
	long level;						//Уровень инициализации
	array<Param *> params;			//Параметры
	array<Info *> enuminfos;		//Списки перечислений
	const char * enumName;			//Имя перечисления
	array<const char *> enumList;	//Список перечислений
	stack<Info *> arrays;			//Текущие массивы
	const char * curArray;			//Текущий массив
	stack<Info *> groups;			//Текущие группы
	const char * curGroup;			//Текущиая группа
	const char * comment;			//Коментарий к объекту
	const char * group;				//Имя группы
};


//Идентификатор Entity описывающего объект и его параметры
#define MOP_ID	"__IMOParams__"

//Начало списка параметров
#define MOP_BEGINLIST(Entity, Name, ver, level)   class __IMOParams__##Entity : public IMOParams { public: __IMOParams__##Entity (){ SetType(#Entity); SetName(Name); SetVersion(ver); SetLevel(level);
#define MOP_BEGINLISTC(Entity, Name, ver, level, comment)   class __IMOParams__##Entity : public IMOParams { public: __IMOParams__##Entity (){ SetType(#Entity); SetName(Name); SetVersion(ver); SetLevel(level); SetComment(comment);
#define MOP_BEGINLISTG(Entity, Name, ver, level, group)   class __IMOParams__##Entity : public IMOParams { public: __IMOParams__##Entity (){ SetType(#Entity); SetName(Name); SetVersion(ver); SetLevel(level); SetGroup(group);
#define MOP_BEGINLISTCG(Entity, Name, ver, level, comment, group)   class __IMOParams__##Entity : public IMOParams { public: __IMOParams__##Entity (){ SetType(#Entity); SetName(Name); SetVersion(ver); SetLevel(level); SetComment(comment); SetGroup(group);

//Постфикс EX означает наличие параметров ограничения
//Постфикс C означает наличие коментария
//Постфикс H означает что параметр не отображаеться в редакторе 
//и всегда инициализирован значением по умолчанию

//bool
#define MOP_BOOL(name, def)															AddBool(name, def, "", false);
#define MOP_BOOLC(name, def, comment)												AddBool(name, def, comment, false);
#define MOP_BOOLH(name, def)														AddBool(name, def, "", true);
//long
#define MOP_LONG(name, def)															AddLong(name, def, false, 0, 0, "", false);
#define MOP_LONGC(name, def, comment)												AddLong(name, def, false, 0, 0, comment, false);
#define MOP_LONGEX(name, def, min, max)												AddLong(name, def, true, long(min), long(max), "", false);
#define MOP_LONGEXC(name, def, min, max, comment)									AddLong(name, def, true, long(min), long(max), comment, false);
#define MOP_LONGH(name, def)														AddLong(name, def, false, 0, 0, "", true);
//float
#define MOP_FLOAT(name, def)														AddFloat(name, def, false, 0.0f, 0.0f, "", false);
#define MOP_FLOATC(name, def, comment)												AddFloat(name, def, false, 0.0f, 0.0f, comment, false);
#define MOP_FLOATEX(name, def, min, max)											AddFloat(name, def, true, float(min), float(max), "", false);
#define MOP_FLOATEXC(name, def, min, max, comment)									AddFloat(name, def, true, float(min), float(max), comment, false);
#define MOP_FLOATH(name, def)														AddFloat(name, def, false, 0.0f, 0.0f, "", true);
//string
#define MOP_STRING(name, def)														AddString(name, def, false, 0, 0, false, false, "", false);
#define MOP_STRINGC(name, def, comment)												AddString(name, def, false, 0, 0, false, false, comment, false);
#define MOP_STRINGEX(name, def, min, max)											AddString(name, def, true, long(min), long(max), false, false, "", false);
#define MOP_STRINGEXC(name, def, min, max, comment)									AddString(name, def, true, long(min), long(max), false, false, comment, false);
#define MOP_STRINGH(name, def)														AddString(name, def, false, 0, 0, false, false, true);
//locstring
#define MOP_LOCSTRING(name)															AddLocString(name, "", false);
#define MOP_LOCSTRINGC(name, comment)												AddLocString(name, comment, false);
#define MOP_LOCSTRINGH(name)														AddLocString(name, "", true);
//position
#define MOP_POSITION(name, def)														AddPosition(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, "", false);
#define MOP_POSITIONC(name, def, comment)											AddPosition(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comment, false);
#define MOP_POSITIONEX(name, def, min, max)											AddPosition(name, def, true, float(min.x), float(min.y), float(min.z), float(max.x), float(max.y), float(max.z), "", false);
#define MOP_POSITIONEXC(name, def, min, max, comment)								AddPosition(name, def, true, float(min.x), float(min.y), float(min.z), float(max.x), float(max.y), float(max.z), comment, false);
#define MOP_POSITIONH(name, def)													AddPosition(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, "", true);
//angle
#define MOP_ANGLES(name, def)														AddAngles(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, "", false);
#define MOP_ANGLESC(name, def, comment)												AddAngles(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comment, false);
#define MOP_ANGLESEX(name, def, min, max)											AddAngles(name, def, true, float(min.x), float(min.y), float(min.z), float(max.x), float(max.y), float(max.z), "", false);
#define MOP_ANGLESEXC(name, def, min, max, comment)									AddAngles(name, def, true, float(min.x), float(min.y), float(min.z), float(max.x), float(max.y), float(max.z), comment, false);
#define MOP_ANGLESH(name, def)														AddAngles(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, "", true);
//color
#define MOP_COLOR(name, def)														AddColor(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, "", false);
#define MOP_COLORC(name, def, comment)												AddColor(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comment, false);
#define MOP_COLOREX(name, def, min, max)											AddColor(name, def, true, float(min.r), float(min.g), float(min.b), float(min.a), float(max.r), float(max.g), float(max.b), float(max.a), "", false);
#define MOP_COLOREXC(name, def, min, max, comment)									AddColor(name, def, true, float(min.r), float(min.g), float(min.b), float(min.a), float(max.r), float(max.g), float(max.b), float(max.a), comment, false);
#define MOP_COLORH(name, def)														AddColor(name, def, false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, "", true);
//start array
#define MOP_ARRAYBEG(name, min, max)												AddArrayBeg(name, long(min), long(max), "", false);
#define MOP_ARRAYBEGC(name, min, max, comment)										AddArrayBeg(name, long(min), long(max), comment, false);
#define MOP_ARRAYBEGH(name, min, max)												AddArrayBeg(name, long(min), long(max), "", true);
//end array
#define MOP_ARRAYEND																AddArrayEnd();
//start enum
#define MOP_ENUMBEG(type)															SetEnumBeg(type);
//enum element
#define MOP_ENUMELEMENT(value)														EnumElement(value);
//end enum
#define MOP_ENUMEND																	SetEnumEnd();
//enum
#define MOP_ENUM(type, name)														AddEnum(name, type, "", false);
#define MOP_ENUMC(type, name, comment)												AddEnum(name, type, comment, false);
#define MOP_ENUMH(type, name)														AddEnum(name, type, "", true);
#define MOP_ENUMTL(name)															AddEnum(name, "__TripleLogic__", "Select logic state or ignore this parameter", false);
//start group
#define MOP_GROUPBEG(name)															GroupBeg(name, "", false);
#define MOP_GROUPBEGC(name, comment)												GroupBeg(name, comment, false);
#define MOP_GROUPBEGH(name)															GroupBeg(name, "", true);
//end group
#define MOP_GROUPEND()																GroupEnd();


//Окончание списка параметров
#define MOP_ENDLIST(Entity)	EndCreation(); }; };	CREATE_CLASS(__IMOParams__##Entity) CREATE_CLASS(Entity)



//Класс для чтения параметров из области памяти
class MOPReader
{
/*
	Необходимое расположение данных в памяти:
	IMOParams::Type data IMOParams::Type data...

	bool sizeof == 1
	long sizeof == 4
	float sizeof == 4
	string stringdata
	position sizeof == 3*4
	angles sizeof == 3*4
	array: numelements(sizeof == 4) numparams(sizeof == 4) element{IMOParams::Type data...} element{IMOParams::Type data...}...
	enum: len(sizeof == 4) stringdata
	binary
*/
public:
	MOPReader(const void * data, long size);

	//Получить текущию версию
	dword GetVersion();
	//Получить идентификатор объекта
	const char * GetObjectID();

//-------------------------------------------------------------
//Функции последовательного доступа
//-------------------------------------------------------------
public:
	//Прочитать bool
	bool Bool();
	//Прочитать long
	long Long();
	//Прочитать float
	float Float();
	//Прочитать string
	ConstString String();
	//Прочитать locstring
	const char * LocString();
	//Прочитать позицию
	Vector Position();
	//Прочитать углы
	Vector Angles();
	//Прочитать цвет
	Color Colors();
	//Получить количество элементов в массиве, далее читать данные элементов
	long Array();
	//Получить элемент списка перечислений
	ConstString Enum();
	//Получить элемент тройной логики если вернёт true, то можно использовать state
	bool TripleLogic(bool & state);
	

	//Создать копию данных, перезаписав текущие
	void * ReplaseData(dword & dataSize);


private:
	const char * sysReadConstString(dword & len, dword & hash);
	const char * sysReadString();
	bool Check(IMOParams::Type type);
	void Error(const char * request);
	const char * GetParamsByType(long pnt, long * size);

private:
	const char * data;
	long p;
	long size;
	ILocStrings * locStrings;
};

inline MOPReader::MOPReader(const void * data, long size)
{
	this->data = (const char *)data;
	this->size = size;
	p = 4;
	sysReadString();
	Assert(p <= size);
	locStrings = (ILocStrings *)api->GetService("LocStrings");
	Assert(locStrings);
}

//Получить текущию версию
inline dword MOPReader::GetVersion()
{
	return SwizzleDWord(*(dword *)(this->data));
}
//Получить идентификатор объекта
inline const char * MOPReader::GetObjectID()
{
	long curp = p;
	p = 4;
	const char * id = sysReadString();
	p = curp;
	return id;
}

//Прочитать bool
inline bool MOPReader::Bool()
{
	if(Check(IMOParams::t_bool))
	{
		bool b = data[p++] != 0;
		Assert(p <= size);
		return b;
	}
	return false;
}

//Прочитать long
inline long MOPReader::Long()
{
	if(Check(IMOParams::t_long))
	{
		long v = SwizzleLong(*(long *)(data + p));
		p += sizeof(long);
		Assert(p <= size);
		return v;
	}
	return 0;
}

//Прочитать float
inline float MOPReader::Float()
{	
	if(Check(IMOParams::t_float))
	{
		float v = SwizzleFloat(*(float *)(data + p));
		p += sizeof(float);
		Assert(p <= size);
		return v;
	}
	return 0.0f;
}

//Прочитать string
inline ConstString MOPReader::String()
{
	if(Check(IMOParams::t_string))
	{
		long ptr = p;
		dword len = 0, hash = 0;
		const char * str = sysReadConstString(len, hash);
		if(str)
		{
			return ConstString(str, hash, len);
		}
		p = ptr;
		Error("<- invalidate mis file data");
		p = size;
	}
	return ConstString();
}

//Прочитать locstring
inline const char * MOPReader::LocString()
{
	if(Check(IMOParams::t_locstring))
	{
		long id = SwizzleLong(*(long *)(data + p));
		p += sizeof(long);
		Assert(p <= size);
		const char * str = locStrings->GetString(id);
		if(str)
		{
			return str;
		}
	}
	return "";
}

//Прочитать позицию
inline Vector MOPReader::Position()
{
	if(Check(IMOParams::t_position))
	{
		Vector * pv = (Vector *)(data + p);
		p += sizeof(Vector);
		Assert(p <= size);
		Vector v;
		v.x = SwizzleFloat(pv->x);
		v.y = SwizzleFloat(pv->y);
		v.z = SwizzleFloat(pv->z);
		return v;
	}
	return Vector(0.0f);
}

//Прочитать углы
inline Vector MOPReader::Angles()
{
	if(Check(IMOParams::t_angles))
	{
		Vector * pv = (Vector *)(data + p);
		p += sizeof(Vector);
		Assert(p <= size);
		Vector v;
		v.x = SwizzleFloat(pv->x);
		v.y = SwizzleFloat(pv->y);
		v.z = SwizzleFloat(pv->z);
		return v;
	}
	return Vector(0.0f);
}

//Прочитать цвет
inline Color MOPReader::Colors()
{
	if(Check(IMOParams::t_color))
	{
		Color * pv = (Color *)(data + p);
		p += sizeof(Color);
		Assert(p <= size);
		Color v;
		v.r = SwizzleFloat(pv->r);
		v.g = SwizzleFloat(pv->g);
		v.b = SwizzleFloat(pv->b);
		v.a = SwizzleFloat(pv->a);
		return v;
	}
	return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

//Получить количество элементов в массиве, далее читать данные элементов
inline long MOPReader::Array()
{
	if(Check(IMOParams::t_array))
	{
		long v = SwizzleLong(*(long *)(data + p));
		p += sizeof(long);
		Assert(p <= size);
		return v;
	}
	return 1;
}

//Получить элемент списка перечислений
inline ConstString MOPReader::Enum()
{
	if(Check(IMOParams::t_enum))
	{
		long ptr = p;
		dword len = 0, hash = 0;
		const char * str = sysReadConstString(len, hash);
		if(str)
		{
			return ConstString(str, hash, len);
		}
		p = ptr;
		Error("<- invalidate mis file data");
		p = size;
	}
	return ConstString();
}

//Получить элемент тройной логики если вернёт true, то можно использовать state
inline bool MOPReader::TripleLogic(bool & state)
{
	const char * str = Enum().c_str();
	switch(str[0])
	{
	case 'i':
	case 'I':
		state = false;
		return false;
	case 't':
	case 'T':
		state = true;
		return true;
	case 'f':
	case 'F':
		state = false;
		return true;
	}
	Assert(false);
	return false;
}

//Создать копию данных, перезаписав текущие
inline void * MOPReader::ReplaseData(dword & dataSize)
{
	char * buffer = NEW char[size];
	dataSize = size;
	for(long i = 0; i < size; i++)
	{
		buffer[i] = data[i];
	}
	data = buffer;
	return buffer;
}

inline const char * MOPReader::sysReadConstString(dword & len, dword & hash)
{
	len = SwizzleDWord(*(dword *)(data + p));
	p += sizeof(dword);
	Assert(p < size);
	hash = SwizzleDWord(*(dword *)(data + p));
	p += sizeof(dword);
	Assert(p < size);		
	const char * str = sysReadString();
	Assert(p <= size);
	return str;
}

inline const char * MOPReader::sysReadString()
{
	if(p >= size) return null;
	const char * str = &data[p];
	for(; data[p]; p++)
	{		
		if(p >= size) return null;
	}
	p++;
	Assert(p <= size);
	return str;
}

inline bool MOPReader::Check(IMOParams::Type type)
{
	if(data[p] == type)
	{
		p++;
		return true;
	}
	Error(GetParamsByType(type, null));
	p += size;
	return false;
}

inline void MOPReader::Error(const char * request)
{
	api->Trace("Invalidate mis file!");
	long ptr = p;
	//Пропускаем имя
	for(ptr = 4; true; ptr++)
	{
		if(ptr >= size) break;
		if(!data[ptr])
		{
			ptr++;
			break;
		}
	}
	//Сканируем все параметры и выводим в лог
	api->Trace("===========================================");
	while(ptr < size)
	{
		long size = 1;
		const char * current = GetParamsByType(ptr, &size);
		if(ptr != p)
		{
			api->Trace("%s", current);
		}else{
			api->Trace("%s <- read as %s", current, request);
		}
		ptr += size;
	}
	p = ptr;
	api->Trace("===========================================");
	Assert(false);
}

inline const char * MOPReader::GetParamsByType(long pnt, long * size)
{
	const char * typeName = "Error type";
	long ptr = p;
	p = pnt;
	switch(size ? data[p] : pnt)
	{
	case IMOParams::t_bool:
		typeName = "bool";
		p += 2;
		break;
	case IMOParams::t_long:
		typeName = "long";
		p += 1 + sizeof(long);
		break;
	case IMOParams::t_float:
		typeName = "float";
		p += 1 + sizeof(float);
		break;
	case IMOParams::t_string:
		typeName = "string";
		p++;
		sysReadString();
		break;
	case IMOParams::t_locstring:
		typeName = "locstring";
		p += 1 + sizeof(long);
		break;
	case IMOParams::t_position:
		typeName = "position";
		p += 1 + sizeof(float)*3;
		break;
	case IMOParams::t_angles:
		typeName = "angles";
		p += 1 + sizeof(float)*3;
		break;
	case IMOParams::t_color:
		typeName = "color";
		p += 1 + sizeof(float)*4;
		break;
	case IMOParams::t_array:
		typeName = "array";
		p += 1 + sizeof(long);
		break;
	case IMOParams::t_enum:
		typeName = "enum";
		p++;
		sysReadString();
		break;
	default:
		p++;
	}
	if(size) *size = p - pnt;
	p = ptr;
	return typeName;
}



//Класс для накопления параметров в области памяти
class MOPWriter
{
public:
	MOPWriter(dword ver = '1.00', const char * id = null, const char * entityName = null);

	//Добавить bool
	void AddBool(bool v);
	//Добавить long
	void AddLong(long v);
	//Добавить float
	void AddFloat(float v);
	//Добавить string
	void AddString(const char * v);
	//Добавить locstring
	void AddLocString(long id);
	//Добавить позицию
	void AddPosition(const Vector & v);
	//Добавить углы
	void AddAngles(const Vector & v);
	//Добавить цвет
	void AddColor(const Color & v);
	//Установить количество элементов в массиве, далее добавлять данные элементов
	void AddArray(long size);
	//Добавить бинарный блок данных
	//void AddBinary(const void * data, long size);
	//Добавить элемент списка перечислений
	void AddEnum(const char * v);
	//Получить читалку
	MOPReader Reader();
	//Получить накопленные даныне
	dword GetData(void * & data);

private:
	void sysAddDword(dword d);
	void sysAddFloat(float f);
	void sysAddConstString(const char * str);
	void sysAddString(const char * str);
	void sysUpdateSize();

private:
	array<byte> data;
	long sizepos;
};


inline MOPWriter::MOPWriter(dword ver, const char * id, const char * entityName) : data(_FL_, 256)
{
	if(entityName != null)
	{
		Assert(entityName[0] != 0);
		sysAddString(entityName);
		sizepos = data;
		sysAddDword(0);
	}else sizepos = -1;
	sysAddDword(ver);
	sysAddString(id);
	sysUpdateSize();
}

//Добавить bool
inline void MOPWriter::AddBool(bool v)
{
	data.Add(IMOParams::t_bool);
	data.Add(v ? 1 : 0);
	sysUpdateSize();
}

//Добавить long
inline void MOPWriter::AddLong(long v)
{
	data.Add(IMOParams::t_long);
	sysAddDword(dword(v));
	sysUpdateSize();
}

//Добавить float
inline void MOPWriter::AddFloat(float v)
{
	data.Add(IMOParams::t_float);
	sysAddFloat(v);
	sysUpdateSize();
}

//Добавить string
inline void MOPWriter::AddString(const char * v)
{
	data.Add(IMOParams::t_string);
	sysAddConstString(v);
	sysUpdateSize();
}

//Добавить locstring
inline void MOPWriter::AddLocString(long id)
{
	data.Add(IMOParams::t_locstring);
	sysAddDword(dword(id));
	sysUpdateSize();
}

//Добавить позицию
inline void MOPWriter::AddPosition(const Vector & v)
{
	data.Add(IMOParams::t_position);
	sysAddFloat(v.x);
	sysAddFloat(v.y);
	sysAddFloat(v.z);
	sysUpdateSize();
}

//Добавить углы
inline void MOPWriter::AddAngles(const Vector & v)
{
	data.Add(IMOParams::t_angles);
	sysAddFloat(v.x);
	sysAddFloat(v.y);
	sysAddFloat(v.z);
	sysUpdateSize();
}

//Добавить цвет
inline void MOPWriter::AddColor(const Color & v)
{
	data.Add(IMOParams::t_color);
	sysAddFloat(v.r);
	sysAddFloat(v.g);
	sysAddFloat(v.b);
	sysAddFloat(v.a);
	sysUpdateSize();
}

//Установить количество элементов в массиве, далее добавлять данные элементов
inline void MOPWriter::AddArray(long size)
{
	Assert(size >= 0);
	data.Add(IMOParams::t_array);
	sysAddDword(dword(size));
	sysUpdateSize();
}

//Добавить элемент списка перечислений
inline void MOPWriter::AddEnum(const char * v)
{
	data.Add(IMOParams::t_enum);
	sysAddConstString(v);
	sysUpdateSize();
}

//Получить читалку
inline MOPReader MOPWriter::Reader()
{
	Assert(sizepos < 0);
	return MOPReader(&data[0], data);
}

//Получить накопленные даныне
inline dword MOPWriter::GetData(void * & data)
{
	data = &this->data[0];
	return this->data;
}

inline void MOPWriter::sysAddDword(dword d)
{
	d = SwizzleDWord(d);
	data.Add(byte(d >> 0));
	data.Add(byte(d >> 8));
	data.Add(byte(d >> 16));
	data.Add(byte(d >> 24));
}

inline void MOPWriter::sysAddFloat(float f)
{
	f = SwizzleFloat(f);
	sysAddDword(*(dword *)&f);
}

inline void MOPWriter::sysAddConstString(const char * str)
{
	dword len = 0;
	dword hash = string::HashNoCase(str, len);
	sysAddDword(len);
	sysAddDword(hash);
	sysAddString(str);
}

inline void MOPWriter::sysAddString(const char * str)
{
	if(!str) str = "";
	while(*str) data.Add(byte(*str++));
	data.Add(0);
}

inline void MOPWriter::sysUpdateSize()
{
	if(sizepos < 0) return;
	long size = data - sizepos - 4;
	Assert(size >= 0);
	size = SwizzleLong(size);
	data[sizepos + 0] = byte(size >> 0);
	data[sizepos + 1] = byte(size >> 8);
	data[sizepos + 2] = byte(size >> 16);
	data[sizepos + 3] = byte(size >> 24);
}

//Класс для сбора данных о миссии
class MOPCollector
{
	struct Data
	{
		byte * data;
		dword size;
		long level;
	};


public:
	MOPCollector() : objects(_FL_)
	{
		fullSize = sizeof(MissionFileId);
	}

	~MOPCollector()
	{
		for(long i = 0; i < objects; i++) delete objects[i].data;
	}

	//Добавить в коллекцию объектов данные об объекте
	void Add(long level, MOPWriter & writer)
	{
		for(long i = 0; i < objects; i++) if(objects[i].level > level) break;
		Data obj;
		obj.level = level;
		void * data = null;
		obj.size = writer.GetData(data);
		obj.data = NEW byte[obj.size];
		memcpy(obj.data, data, obj.size);
		objects.Insert(obj, i);
		fullSize += obj.size;
	}

	//Получить результирующие данные
	void GetResultData(void * & data, dword & size)
	{
		size = fullSize;
		byte * d = NEW byte[fullSize];
		data = d;
		//Идентификатор
		MissionFileId * id = (MissionFileId *)data;
		const char * cd = MISSION_FILE_ID;		
		for(long i = 0; i < sizeof(id->id); i++) id->id[i] = cd[i];
		id->ver = SwizzleLong(MISSION_FILE_VER);
		id->objectsCount = SwizzleLong((long)objects.Size());
		dword p = sizeof(MissionFileId);
		for(i = 0; i < objects; i++)
		{
			memcpy(d + p, objects[i].data, objects[i].size);
			p += objects[i].size;
		}
		Assert(size == p);
	}

private:
	array<Data> objects;
	long fullSize;
};

#endif
