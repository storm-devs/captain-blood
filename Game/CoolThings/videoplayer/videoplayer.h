#ifndef COOL_THINGS_VIDEO_PLAYER
#define COOL_THINGS_VIDEO_PLAYER

#include "..\..\..\Common_h\IVideoCodec.h"

class VideoPlayer : public MissionObject
{
protected:
	IVideoCodec* pCodec;

	float xpos,ypos,width,height;
	const char* sFileName;
	bool bEditModePause;
	MissionTrigger triggerEnd;
	bool bLoopPlay;
	bool bStartUpActivate;
	bool bPlayByRestore;
	bool bPause;
	bool bFirstFrame;
	bool bBeforeGeom;

public:

	VideoPlayer();
	~VideoPlayer();

	void _cdecl Realize(float fDeltaTime, long level);

	void _cdecl ActivateEvent(const char * group, MissionObject * sender);
	void _cdecl DeactivateEvent(const char * group, MissionObject * sender);

	virtual void Activate(bool isActive);
	virtual void Show(bool isShow);

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	bool EditMode_Update(MOPReader & reader);

private:
	void StartVideo();
	void FinishVideo();
};

#endif