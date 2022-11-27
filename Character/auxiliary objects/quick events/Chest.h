#ifndef Chest_H
#define Chest_H

#include "QuickEvent.h"
#include "..\..\..\common_h\AnimationStdEvents.h"
#include "ChestPattern.h"
#include "..\..\utils\SafePointerEx.h"

#define CHEST_OBJECT_GROUP GroupId('C','H','E','S')

class Chest : public QuickEvent
{
private:
	bool statWasOpened, statIsActivated;

public:
	Chest();
	
	ConstString bonusTable;	
	
	bool allowRestart;

	ConstString pattern_name;
	MOSafePointerTypeEx<ChestPattern> pattern_ptr;
	ChestPattern* pattern;
	Vector dropPosition;
	float dropAngle;
	
	virtual void InitData();

	bool InitFromPattern();	

	virtual bool EditMode_Update(MOPReader & reader);	
	virtual void _cdecl EditMode_Work(float dltTime, long level);

	virtual void BeginQuickEvent();	
	virtual void QuickEventUpdate(float dltTime);	
	virtual void Interupt(bool win_game);
	virtual void AnimEvent(const char * param);		
	virtual void Restart();
	virtual void Activate(bool isActive);

	virtual const char* GetName() { return "Chest";};

	void SaveState();
	bool AllowRestart();

	void _cdecl DebugDraw(float dltTime, long level);
	

	static const char * comment;
};

#endif