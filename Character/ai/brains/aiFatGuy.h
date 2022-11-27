#ifndef AI_FAT_GUY
#define AI_FAT_GUY

#include "aiBrain.h"
#include "../Thoughts/FatGuy/aiFatGuyBlocking.h"


class aiFatGuy : public aiBrain
{
public:

	class AnimListener : public IAnimationListener
	{
	public:
		aiFatGuy* pFatGuy;

		AnimListener()
		{
			pFatGuy = NULL;
		}

		void _cdecl SplashDamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<1)
			{
				return;
			}

			if(pFatGuy && string::IsEqual(params[0],"FatGuy splash"))
			{
				pFatGuy->SplashDamage();
			}
		};
	};

	AnimListener animListener;
		
	aiFatGuy(Character & ch, const char* szName);


	virtual void TickWhenEmpty();

	virtual void ActivateTargetPairStateThought();

	//Персонаж получил люлей
	virtual void Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	//virtual bool CheckCharState(Character* target);

	virtual void NotifyAboutAttack(Character* attacker,float damage);	

	virtual void SplashDamage();
};


#endif


