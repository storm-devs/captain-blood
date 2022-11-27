#ifndef INTERFACE_VIDEO_CODEC
#define INTERFACE_VIDEO_CODEC

#include "Mission.h"

class IVideoCodec
{
public:
	IVideoCodec(IRender* _pRender)
	{
		pRender = _pRender;
		bIsPaused = false;
	}
	virtual ~IVideoCodec() {}

	virtual bool OpenFile(const char* pcVideoFile,bool bLoopPlay=false) {sCurVideoFile=pcVideoFile; return true;}
	virtual bool Frame(bool bPause) {return true;}
	virtual void CloseFile() {sCurVideoFile="";}
	virtual void SetViewPosition(float fx, float fy, float fw, float fh, bool bModifyAspect=false) {}

	virtual bool IsCurrentFile(const char* pcVideoFile) {return sCurVideoFile==pcVideoFile;}
	virtual bool IsProcessed() {return bProcessed;}

	virtual void Start() {bProcessed=true;}
	virtual void Stop() {bProcessed=false;}
	virtual void Pause(bool bPause) {bIsPaused=bPause;}

	IRender* GetRender() {return pRender;}
	bool IsSystemPause() {return !api->IsActive(); }
	bool IsPause() {return (bIsPaused || IsSystemPause());}
	bool IsGamePause() {return bIsPaused;}

protected:
	string sCurVideoFile;
	bool bProcessed;
	IRender* pRender;
	bool bIsPaused;
};

#endif
