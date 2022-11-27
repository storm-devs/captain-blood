#ifndef _ITips_h_
#define _ITips_h_

#include "Mission.h"

class ITip
{	
public:

	enum TState
	{
		inactive=0,
		active
	};

	virtual void Activate(bool) = null;
	virtual void SetPos(Vector pos) = null;
	virtual void SetAlpha(float _alpha) = null;
	virtual void SetState(TState state) = null;
	virtual void Release() = null;
};

class ITipsManager : public MissionObject
{
public:
	static ITipsManager * GetManager(IMission* mission)
	{
		ITipsManager * manager = NULL;
		MGIterator & iter = mission->GroupIterator(TipsManagerGroup(), _FL_);
		if (!iter.IsDone())
		{		
			manager = (ITipsManager *)iter.Get();
		}
		iter.Release();
		return manager;
	};

	static GroupId TipsManagerGroup()
	{
		static GroupId tipsId('t','i','p','s');
		return tipsId;
	}

public:
	virtual ITip* CreateTip(const ConstString & id, Vector pos, void* ptr) = null;	
	virtual void  SetTipState(ITip::TState _state, void* ptr) = null;	
	virtual void  Enable(bool enable) = null;		

	virtual bool  IsAnyActiveTip() = null;
};

#endif











