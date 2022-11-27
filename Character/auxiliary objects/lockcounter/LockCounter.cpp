
#include "LockCounter.h"
#include "..\Arbiter\CharactersArbiter.h"



aiLockCounter CharsLockCounter;

aiLockCounter::aiLockCounter() : info (_FL_)
{
}

aiLockCounter::~aiLockCounter()
{
}

void aiLockCounter::AddToLock (Character* me, Character* enemy)
{
	LockInfo i;
	i.me = me;
	i.enemy = enemy;
	info.Add(i);
}

int aiLockCounter::GetLockedCount (Character* body)
{
	int Count = 0;
	for (dword i = 0; i < info.Size(); i++)
	{
		if (info[i].enemy == body) Count++;
	}

	return Count;
}

bool aiLockCounter::IsLockedToMe (Character* me, Character* enemy)
{
	for (dword i = 0; i < info.Size(); i++)
	{
		//какая то сволочь ХОЧЕТ МЕНЯ УБИТЬ !!!!!! аааааа !!!! 
		if (info[i].me == enemy && info[i].enemy == me) return true;
	}

	return false;
}

void aiLockCounter::ClearLockList (Character* me)
{
	for (dword i = 0; i < info.Size(); i++)
	{
		if (info[i].me == me)
		{
			info.DelIndex(i);
			i--;
		}
	}
}

void aiLockCounter::Clear ()
{
	info.DelAll();
}



