

#include "..\..\..\Common_h\mission.h"

class Character;

class aiLockCounter
{
	struct LockInfo
	{
		Character* me;
		Character* enemy;
	};


	array<LockInfo> info;



public:
	aiLockCounter();
	virtual ~aiLockCounter();


	virtual void AddToLock (Character* me, Character* enemy);

	virtual int GetLockedCount (Character* body);

	virtual bool IsLockedToMe (Character* me, Character* enemy);

	virtual void ClearLockList (Character* me);

	virtual void Clear ();
};

