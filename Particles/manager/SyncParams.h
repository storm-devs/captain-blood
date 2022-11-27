#ifndef ___PARTICLES_SYNC_PARAMS___H___
#define ___PARTICLES_SYNC_PARAMS___H___

#include "..\..\common_h\templates.h"
#include "..\system\particlesystem\particlesystem.h"
#include "..\system\particlesystem\systemproxy.h"


struct SyncParams
{
	Plane planes[32];
	Matrix mView;
	float fDeltaTime;

	ParticleVB * updateData;

	//Настоящие IParticles которые колбасит из потока
	array<ParticleSystem*> SystemsForThread;

	//Частицы которые завели из основного потока, и надо перебросить их в SystemsForThread
	array<ParticleSystem*> SystemsCreated;

	//Частицы которые убили из основного потока и надо удалить их из SystemsForThread
	array<ParticleSystem*> SystemsDeleted;

	//Прокси класс, который потом (в конце кадра) пересылает изменения в SystemsForThread
	array<ParticleSystemProxy*> SystemsProxy;

	SyncParams() : SystemsCreated(_FL_, 2048),
		SystemsDeleted(_FL_, 2048),
		SystemsProxy(_FL_, 2048),
		SystemsForThread(_FL_, 2048)
	{
		updateData = NULL;
	}

};



#endif