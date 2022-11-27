#ifndef AI_BombardeerPlaceBomb
#define AI_BombardeerPlaceBomb


#include "..\aiThought.h"

class Character;

class aiBombardeerPlaceBomb : public aiThought
{	
	bool bStartPlaceBomb;

public:

	aiBombardeerPlaceBomb(aiBrain* Brain);
	virtual ~aiBombardeerPlaceBomb();

	virtual void Activate();

	virtual bool Process (float fDeltaTime);	

	virtual void Stop();	
};

#endif