#ifndef Rotor_H
#define Rotor_H

#include "QuickEvent.h"

class Rotor : public QuickEvent
{
public:
					
	Matrix center_matrix;
	Vector pos;	

	float   fTimeToSpin;
	float   fCurTimeToSpin;
	float   angle;
	float   angle_speed;
	bool    begin_spin;
	float   fDamageRadius;

	virtual bool EditMode_Update(MOPReader & reader);		

	virtual void BeginQuickEvent();	
	virtual void QuickEventUpdate(float dltTime);	
	virtual void Interupt(bool win_game);
	virtual void AnimEvent(const char * param);	

	virtual const char* GetName() { return "Rotor";};

	static const char * comment;
};

#endif