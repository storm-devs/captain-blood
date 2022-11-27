#ifndef POINT_EMITTER_H
#define POINT_EMITTER_H

#include "base.h"


class PointEmitter : public BaseEmitter
{

protected:

	Vector v;
 
	float sizeX;
	float sizeY;
	float sizeZ;


//Получить позицию для рождения новых партиклов  
  virtual Vector GetNewParticlePosition (float DeltaTime);

public:
 
	// Конструктор / деструктор
  PointEmitter(ParticleSystem* pSystem, BillBoardProcessor* processor);
  virtual ~PointEmitter();


	void Editor_UpdateCachedData ();
  
  


};

#endif