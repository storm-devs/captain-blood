#ifndef _XBOX

#ifndef VIDEO_PLAYER_WMVINTERFACES
#define VIDEO_PLAYER_WMVINTERFACES

class IWMVMemoryManager
{
public:
	static void * _cdecl CoreAlloc(size_t size);
	static void _cdecl CoreFree(void * ptr);

	static void LogTrace(const char* pcText);

	static float GetVolume();
};

class IWMVTexture
{
public:
	static IWMVTexture* Create(unsigned int& w, unsigned int& h);
	virtual void Release() = 0;
	virtual bool Lock(void* &pTxtBuffer, int &lTxtPitch) = 0;
	virtual void Unlock() = 0;
protected:
	virtual ~IWMVTexture() {};
};

class IWMVGraph
{
public:
	static IWMVGraph* CreateWMVGraphInstance();

	IWMVGraph() {}
	virtual ~IWMVGraph() {}

	virtual bool OpenFile(const char* pcVideoFile) = 0;
	virtual void CloseFile() = 0;

	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual void Restart() = 0;
	virtual void Pause(bool bPause) = 0;

	virtual void SoundOff(bool bSoundOff) = 0;

	virtual bool IsFinish() = 0;
};

#endif
#endif
