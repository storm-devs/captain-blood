#ifndef _ICharacter_h_
#define _ICharacter_h_

#include "Mission.h"

#ifndef _XBOX
	#ifdef GAME_RUSSIAN

		#define CHARACTER_STARFORCE
		//#define CHARACTER_STARFORCE_ASSERTS

		#ifdef CHARACTER_STARFORCE_ASSERTS
			#define	AssertStarForce(x) Assert(x)
		#else
			#define	AssertStarForce(x)
		#endif

	#endif
#endif


class ICharacter : public DamageReceiver
{	
public:
	virtual IAnimation* GetAnimation() = 0;

// Добавление виртуальных функций для защиты разных версий
#ifndef GAME_DEMO
	virtual bool CheckHitted(dword source, float dmg) { return true; }
#endif
#ifdef GAME_RUSSIAN
	virtual ICharacter * GetKiller(dword source, float dmg) { return null; }
	#ifndef GAME_DEMO
		virtual bool InspectArmor(ICharacter * character) { return true; }
	#endif
#endif

	virtual Matrix & GetColiderMatrix(Matrix & mtx) = 0;
	virtual void Hit(dword source, float dmg, const char * reaction, float block_dmg, const char * blockReaction, const char * deathLink) = 0;

	virtual const ConstString & GetKillerId() = 0;

	__forceinline static const ConstString & GetTypeId()
	{
		static const ConstString typeId("Character");
		return typeId;
	};
};


#endif










