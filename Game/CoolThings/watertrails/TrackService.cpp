#include "trackService.h"
/*
CREATE_SERVICE(WaterTrackService, 220)

WaterTrackService::WaterTrackService ()
{
	pVTmpBuffer = NULL;
	pITmpBuffer = NULL;
}

WaterTrackService::~WaterTrackService ()
{
	RELEASE(pVTmpBuffer);
	RELEASE(pITmpBuffer);
}

bool WaterTrackService::Init()
{
	return true;
}


TrackVertex* WaterTrackService::LockDrawBuffer ()
{
	return NULL;
}

void WaterTrackService::UnlockDrawBuffer()
{

}
*/

/////////////////////////////////

TrackManager:: TrackManager()
{
	pVBuffer = null;
	pIBuffer = null;
}

TrackManager::~TrackManager()
{
	RELEASE(pVBuffer)
	RELEASE(pIBuffer)
}

bool TrackManager::Create(MOPReader &reader)
{
	CreateBuffers();

	return true;
}

void TrackManager::CreateBuffers()
{
	pVBuffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*drop_count*4,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVBuffer)

	pIBuffer = Render().CreateIndexBuffer(
		sizeof(WORD)  *drop_count*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(pIBuffer)

	WORD *p = (WORD*)pIBuffer->Lock();
	Assert(p)

	for( long i = 0 ; i < drop_count ; i++ )
	{
		p[i*6 + 0] = WORD(i*4 + 0);
		p[i*6 + 1] = WORD(i*4 + 1);
		p[i*6 + 2] = WORD(i*4 + 2);
		p[i*6 + 3] = WORD(i*4 + 0);
		p[i*6 + 4] = WORD(i*4 + 2);
		p[i*6 + 5] = WORD(i*4 + 3);
	}

	pIBuffer->Unlock();
}

MOP_BEGINLIST(TrackManager, "", '1.00', 100)
MOP_ENDLIST(TrackManager)
