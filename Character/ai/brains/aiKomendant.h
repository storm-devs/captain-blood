#ifndef AI_Komendant
#define AI_Komendant

#include "aiBrain.h"
 
class  aiKomendant: public aiBrain
{		
	class AnimListener : public IAnimationListener
	{
	public:
		aiKomendant* pKomendant;

		AnimListener()
		{
			pKomendant = NULL;
		}

		void _cdecl SplashDamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<2)
			{
				return;
			}

			if(pKomendant && string::IsEqual(params[0],"Komendant splash"))
			{
				pKomendant->SplashDamage(params[1]);
			}
		};
	};

	AnimListener animListener;	

	Vector pray_pos;

public:

	aiKomendant(Character & ch, const char* szName);
	virtual ~aiKomendant();

	virtual void SetParams(CharacterControllerParams * params);	

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime);

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual bool AttackCharacter();	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("KomendantParams"); }

	virtual bool AllowToBeBlowed() { return false; };

	virtual void SplashDamage(const char* locator);

	virtual void Boom(const Vector & pos);	
};


#endif


