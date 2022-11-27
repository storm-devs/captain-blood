
#ifndef _Tips_h_
#define _Tips_h_


#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\ITips.h"

class TipsManager;

class Tip: public ITip
{	
	TipsManager* manager;

	Vector pos;
	float alpha;
	ITip::TState state;
	int tip_ref;

	float sec_alpha;

	bool bActive;

	void* ptr;

public:

	Tip(TipsManager* manager, int tip_ref,void* _ptr);

	virtual void Activate(bool bActive);
	virtual bool IsActive();

	virtual void SetPos(Vector pos);
	virtual Vector GetPos();

	virtual void  SetAlpha(float _alpha);
	virtual float GetAlpha();

	virtual void         SetState(ITip::TState _state);
	virtual ITip::TState GetState();	

	virtual void* GetObjPtr() { return ptr; };

	virtual int  GetTipRef() { return tip_ref; };
	
	virtual void Release();	

	void Update(float dltTime);
};

#endif











