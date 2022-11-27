#ifndef AI_BOMBARDEER_BRAIN
#define AI_BOMBARDEER_BRAIN

#include "aiBrain.h"
#include "../Thoughts/Bombardeer/aiBombardeerDropBomb.h"
#include "../Thoughts/Bombardeer/aiBombardeerPlaceBomb.h"


class aiBombardeer : public aiBrain
{
		
public:

	bool isGuard;
	float suicideRushDist;

	aiBombardeerDropBomb*  pBombardeerDropBomb;	
	aiBombardeerPlaceBomb* pBombardeerPlaceBomb;	

	class AnimListener : public IAnimationListener
	{
	public:
		aiBombardeer* pBombardeer;

		AnimListener()
		{
			pBombardeer = NULL;
		}

		void _cdecl Suicide(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<3)
			{
				return;
			}
			
			if(pBombardeer && string::IsEqual(params[0],"Bomber Suicide"))
			{
				float dmg = 50.0f;
				float radius = 3.0f;			
	
				char* str = null;
				if (!string::IsEmpty(params[1]))
				{
					radius = (float)strtod(params[1], &str);
				}
	
				if (!string::IsEmpty(params[2]))
				{
					dmg = (float)strtod(params[2], &str);
				}

				pBombardeer->Suicide(dmg,radius);
			}
		};
	};

	AnimListener animListener;

	float fDistToShoot;
	
	float fMinTimeToShoot;
	float fMaxTimeToShoot;

	bool  bAngleHasBeenInit;
	float fInitialAngle;
	float fShootSektor;
			
	bool  in_rush;

	float curTimeToSuicide; 
	float timeToSuicide; 	

	bool  bAutoSwitchFromSentinel;
	float fRaduisWhenSwitchFromSentinel;

	bool  in_suicide;

	aiBombardeer(Character & ch, const char* szName);
	virtual ~aiBombardeer();

	virtual void SetParams(CharacterControllerParams * params);	
	
	virtual void Init();

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();

	virtual void Update(float dltTime);

	virtual void GuardTick();	
	virtual void ActivateTargetPairStateThought();
	
	virtual void TickWhenEmpty();

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	void Boom(const Vector & pos);	

	void SuicideThought();

	void Suicide(float dmg, float radius);
	
	virtual bool AttackCharacter();

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("BombardeerParams"); }	

	virtual bool AllowToBeMoved() { return !isGuard; };

	virtual float PursuitDist() { return fDistToShoot * 0.8f; };

	
};


#endif


