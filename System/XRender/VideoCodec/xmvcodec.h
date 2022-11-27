#ifdef _XBOX

#ifndef COOL_THINGS_VIDEO_PLAYER_XMVCODEC
#define COOL_THINGS_VIDEO_PLAYER_XMVCODEC

#include "..\..\..\common_h\IVideoCodec.h"

class XMVCodec : public IVideoCodec
{
public:
	XMVCodec(IRender* _pRender,bool _useStreaming);
	virtual ~XMVCodec();

	virtual bool OpenFile(const char* pcVideoFile,bool bLoopPlay);
	virtual bool Frame(bool bPause);
	virtual void CloseFile();
	virtual void SetViewPosition(float fx, float fy, float fw, float fh, bool bModifyAspect);

protected:
	IMediaPlayer* pPlayer;
	struct {
		float left;
		float top;
		float right;
		float bottom;
	} viewPos;
	bool useStreaming;
	bool bPausePlay;

	void UpdateViewPosition();
	virtual void Stop();
};

#endif
#endif
