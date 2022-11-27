#ifndef AI_ChainMan_BRAIN
#define AI_ChainMan_BRAIN

#include "aiBrain.h"
#include "../Thoughts/Chainman/aiChainmanStanding.h"

class aiChainman: public aiBrain
{	
	class AnimListener : public IAnimationListener
	{
	public:
		aiChainman* pChainman;

		AnimListener()
		{
			pChainman = NULL;
		}

		void _cdecl SplashDamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<1)
			{
				return;
			}

			if(pChainman && string::IsEqual(params[0],"Chainman splash"))
			{
				pChainman->SplashDamage();
			}
		};
	};

	AnimListener animListener;	

public:

	aiChainman(Character & ch, const char* szName);
	virtual ~aiChainman();	

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();	

	virtual bool FilterFindedTarget(Character* pTarget, bool statistSearch);

	virtual void ActivateTargetPairStateThought();

	virtual void TickWhenEmpty();
	
	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual void Death();

	virtual bool AttackCharacter();	

	virtual void SplashDamage();
	
	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("aiParams"); }
};


#endif


