//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Character
//===========================================================================================================================
// Controller
//============================================================================================

#ifndef _Controller_h_
#define _Controller_h_

#include "..\..\Common_h\mission.h"
#include "..\Auxiliary objects\Arbiter\CharactersArbiter.h"

class Character;
class CharacterControllerParams;

//Max. Много где встречаеться в ai
#define RETURN_CONSTSTRING(s) static const ConstString strId(s); return strId;
#define CHECK_CONTROLLERSUPPORT(s) static const ConstString strId(s); return strId == controllerClassName;

class CharacterController : public Object
{
public:	
	enum SlowmoType
	{
		slowmo_circle = 0,
		slowmo_sector,
		slowmo_forcedword = 0x7FFFFFFF
	};

	struct SlowmoParams
	{
		SlowmoType type;
		char locator[48];
		char timeScale[16];
		float sectorDirection;
		float sectorWidth;
		float radius;
		float damage;
		float duration;
		float probability;
		float delay;
	};

	CharacterController(Character & ch, const char * _name) : chr(ch){ name = _name; };
	virtual ~CharacterController(){};

	virtual void Init() {};	

	//Установить параметры для контроллера
	virtual void SetParams(CharacterControllerParams * params) {};

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset() {};	
	
	//Персонаж атаковал врага
	virtual void Attack(Character* _target) {};

	//Персонаж атаковал врага
	virtual void DrawHUD(float dltTime) {};

	//Персонаж получил люлей
	virtual void Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source) {};
	//момент выстрела 
	virtual void Shoot(float dmd) {};
	//Взрыв брошеной персонажем бомбы
	virtual void Boom(const Vector & pos) {};
	//Персонаж умирает
	virtual void Death() {};

	virtual void MoveAway(Vector pos) {};

	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime) {};
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick() {};	

	//Получить имя контроллера
	const char * Name() { return name; }
	
	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("AIParams"); }


	virtual const ConstString & GetWayPointsName(){ return ConstString::EmptyObject(); };
	virtual void SetWayPointsName(const ConstString & _WayPointsName) {};

	virtual const ConstString & GetAIZoneName(){ return ConstString::EmptyObject(); };
	virtual void SetAIZoneName(const ConstString & _AIZoneName){};

	virtual const ConstString & GetTarget(){ return ConstString::EmptyObject(); };
	virtual void SetTarget(const ConstString & _Target){};

	virtual const ConstString & GetControllerState() { return ConstString::EmptyObject(); };

	virtual void ShowDebugInfo() {};

// Добавление виртуальных функций для защиты разных версий
#ifndef GAME_DEMO
	bool allowToBeHitted;
	virtual void AllowToBeHitted(bool allow) { allowToBeHitted = allow; }
#endif
#ifdef GAME_RUSSIAN
	virtual const char * GetKillerUID(dword source, float dmg) { return "noname"; }
	#ifndef GAME_DEMO
		virtual ICharacter * GetEnemyFriendTarget(ICharacter * character) { return null; }
	#endif
#endif
// Добавление виртуальных функций для защиты разных версий

	virtual int  GetNumAtackers() { return 0; };
	virtual void SetNumAtackers(int _NumAtackers) {};

	virtual Character* GetChrTarget() { return NULL; };
	virtual void SetChrTarget(Character* _pChrTarget) {};

	virtual bool AllowRestartAnim() { return true; };
	virtual void PlayerAchtung() {};

	virtual bool AllowToBeBlowed() { return true; };

	virtual bool AllowToBeMoved() { return true; };

	virtual bool IsStatist() { return false; };	
	
	virtual void Command(dword numParams, const char ** params) {};

	virtual void Show(bool show){};

	virtual void NotifyAboutAttack(Character* attacker,float damage) {};

	virtual bool StopWhenDebug() { return false; };
	virtual void SetStopWhenDebug(bool enable) {};

	virtual bool CanTakeWeapon() { return false; };

protected:
	Character & chr;
	const char * name;	
};

class CharacterControllerDeclarant
{
public:
	CharacterControllerDeclarant(long lvl)
	{
		level = lvl;
		if(first)
		{
			if(first->level > level)
			{
				next = first;
				first = this;
			}else{
				for(CharacterControllerDeclarant * d = first; d->next; d = d->next)
				{
					if(d->next->level > level)
					{
						next = d->next;
						d->next = this;
						return;
					}
				}
				d->next = this;
				next = null;
			}				
		}else{
			first = this;
			next = null;
		}		
	};
	virtual CharacterController * Create(Character & ch) = null;
	inline const ConstString & Name()
	{
		return controllerName;
	};
	inline static CharacterControllerDeclarant * GetFirst()
	{
		return first;
	};
	inline CharacterControllerDeclarant * GetNext()
	{
		return next;
	};
protected:
	static CharacterControllerDeclarant * first;
	CharacterControllerDeclarant * next;
	ConstString controllerName;
	long level;
};

template<class T> class CharacterControllerDeclarantTmpl : public CharacterControllerDeclarant
{
public:
	CharacterControllerDeclarantTmpl(const char * name, long lvl) : CharacterControllerDeclarant(lvl)
	{
		controllerName.Set(name);
	}
	virtual CharacterController * Create(Character & ch)
	{
		return NEW T(ch, controllerName.c_str());
	}
};

#define DeclareCharacterController(className, viewName, level)		CharacterControllerDeclarantTmpl<className> className##_object_CharacterControllerDeclarantTmpl(viewName, level);

class CharacterControllerParams : public MissionObject
{
public:

	//Указать поддержку заданного типа 
	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		return false;
	}

	MO_IS_FUNCTION(CharacterControllerParams, MissionObject);

};


#endif

