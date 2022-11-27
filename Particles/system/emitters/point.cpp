#include "point.h"

// Конструктор / деструктор
PointEmitter::PointEmitter(ParticleSystem* pSystem, BillBoardProcessor* processor) : BaseEmitter(pSystem, processor)
{
	sizeX = 0.0f;
	sizeY = 0.0f;
	sizeZ = 0.0f;
}

PointEmitter::~PointEmitter()
{
}

 
void PointEmitter::Editor_UpdateCachedData ()
{
	sizeX = pEmitter->Fields.GetFloat(GUID_EMITTER_SIZEX);
	sizeY = pEmitter->Fields.GetFloat(GUID_EMITTER_SIZEY);
	sizeZ = pEmitter->Fields.GetFloat(GUID_EMITTER_SIZEZ);

	BaseEmitter::Editor_UpdateCachedData();
}


//Получить позицию для рождения новых партиклов  
Vector PointEmitter::GetNewParticlePosition (float DeltaTime)
{
	//TODO : Надо рождать на начальной позиции трэка...
	v.Rand(-Vector(sizeX, sizeY, sizeZ), Vector(sizeX, sizeY, sizeZ));

	return v;
}

 
  
  
