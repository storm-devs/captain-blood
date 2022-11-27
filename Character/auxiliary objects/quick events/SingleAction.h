#ifndef SingleAction_H
#define SingleAction_H

#include "QuickEvent.h"

class SingleAction : public QuickEvent
{
public:

	Vector orientpoint;	
	bool   only_once;	

	virtual bool EditMode_Update(MOPReader & reader);	

	virtual void BeginQuickEvent();	
	virtual void QuickEventUpdate(float dltTime);	
	virtual void Interupt(bool win_game);
	virtual void AnimEvent(const char * param);	

	static const char * comment;

	virtual const char* GetName() { return "SingleAction";};
};

#endif