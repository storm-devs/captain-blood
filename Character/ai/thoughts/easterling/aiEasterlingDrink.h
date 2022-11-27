#ifndef AI_EasterlingDrink
#define AI_EasterlingDrink


#include "..\aiThought.h"

class Character;

class aiEasterlingDrink : public aiThought
{	
	bool  bStartDrink;
	float fDrinkHealth;

public:

	aiEasterlingDrink(aiBrain* Brain);
	virtual ~aiEasterlingDrink();

	virtual void Activate(float health);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop();
};

#endif