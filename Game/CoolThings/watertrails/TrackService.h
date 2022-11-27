#ifndef WATER_TRACKS_SERIVCE
#define WATER_TRACKS_SERIVCE

#include "..\..\..\Common_h\core.h"
#include "..\..\..\Common_h\Mission.h"


struct TrackVertex
{
	Vector pos;
	dword color;
	float u;
	float v;
};

/*
class WaterTrackService : public Service
{

	IVBuffer * pVTmpBuffer;
	IIBuffer * pITmpBuffer;


public:

	WaterTrackService ();
	virtual ~WaterTrackService ();

	virtual bool Init();


	virtual TrackVertex* LockDrawBuffer ();
	virtual void UnlockDrawBuffer();



};
*/

//////////////////////////////

class TrackManager : public MissionObject
{
public:

	 TrackManager();
	~TrackManager();

public:

	bool Create(MOPReader &reader);

	MO_IS_FUNCTION(TrackManager, MissionObject);

public:

	enum {drop_count = 128*4};

	struct Vertex
	{
		Vector p; dword cl;

		float tu;
		float tv;

		Vector q;
	};

	Vertex *LockBuffer()
	{
		AssertCoreThread

		return (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);
	}

	void UnlockBuffer()
	{
		pVBuffer->Unlock();
	}

	void Prepare()
	{
		Render().SetStreamSource(0,pVBuffer);
		Render().SetIndices(pIBuffer,0);
	}

private:

	void CreateBuffers();

private:

	IVBuffer *pVBuffer;
	IIBuffer *pIBuffer;

};

//////////////////////////////


#endif