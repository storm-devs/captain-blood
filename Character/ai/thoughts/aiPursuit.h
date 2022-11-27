#ifndef AI_THOUGHT_PURSUIT
#define AI_THOUGHT_PURSUIT


#include "aiThought.h"
#include "..\..\..\Common_h\AI\aiPathFinder.h"

class aiThoughtPursuit : public aiThought
{
	enum State
	{
		state_stoped = 0,
		state_need_go,
		state_path_created,
		state_move
	};

	State TaskState;

	Vector vDest;

	IPath * pPath;
	Vector vAntiDir;
	long tryCount;

	IPathFinder * pFinder;

	//void MakeSmoothPath (Path &src_path, Path& dest_path);

	Vector vDestination;
	Vector vDestination1;
	Vector vDestination2;

	int AttractionPointIndex;

	float fLastDistanceToTarget;

	float fDist;

	float fTimeToCheckObstancles;
	bool    bPushMode;
	float   fPushTime;
	float   fStuckTime;
	Vector  vLastPos;

	Vector	vSkidDistance2;
	float	minSkidDistance, skidDistance2, skidTime;
	Vector  oldCurPos;

	bool    check_trg_dist;
	char    act_link[16];

	void Move(float dltTime);


public:

	aiThoughtPursuit(aiBrain* Brain);
	virtual ~aiThoughtPursuit();

	virtual void Activate(const Vector& destination, const ConstString & WayPointsName, float fMinDistance, bool chck_trg_dist, float skidDistance = -1.0f);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void ChangeParams (const Vector& destination, float fMinDistance);

	virtual void Stop ();
	
	void SetActLink(const char* act);

	int GetNumPathPoints();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Pursuit")) return true;
		return false;
	}
};

#endif