#ifndef _XBOX

#ifndef COOL_THINGS_VIDEO_PLAYER_WMVCODEC
#define COOL_THINGS_VIDEO_PLAYER_WMVCODEC

#include "..\..\..\common_h\IVideoCodec.h"
#include "wmvinterfaces.h"

class WMVTexture;

class WMVCodec : public IVideoCodec
{
	struct VideoVertex
	{
		Vector	vPos;
		float	tu, tv;
	};

public:
	WMVCodec(IRender* _pRender);
	virtual ~WMVCodec();

	virtual bool OpenFile(const char* pcVideoFile,bool bLoopPlay);
	virtual bool Frame(bool bPause);
	virtual void CloseFile();
	virtual void SetViewPosition(float fx, float fy, float fw, float fh, bool bModifyAspect);

	virtual void Start() {Play(); IVideoCodec::Start();}
	virtual void Stop();
	virtual void Pause(bool bPause);

	void SetWMVTexture(WMVTexture* pTex);

protected:
	void Play();

private:
	ShaderId shaderID;
	IVariable* textureVar;

	bool bMakeUninitializeDD;
	bool Init();
	void Release();

	IWMVGraph* m_pGraph;

	WMVTexture* pWMVTexture;
	VideoVertex pVideoVertex[4];
	long CalculateCurrentFrame();

	bool m_bLoopPlay;
	bool bPause;
	float fCurFrame;
	float fFrameSpeed;
	long nCurFrame;
	long nEndFrame;
};

#endif
#endif
