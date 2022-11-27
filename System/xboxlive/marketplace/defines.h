#ifndef _DEFINES_H_
#define _DEFINES_H_

class ITexture;

struct XContentData
{
#ifdef _XBOX
	XCONTENT_DATA dat;
#endif
};

class XBoxLiveEntityIterator
{
public:
	virtual ~XBoxLiveEntityIterator() {}

	void MarkAsUnchecked() {bChecked=false;}
	void DoCheck() {bChecked=true;}
	bool IsChecked() {return bChecked;}

private:
	bool bChecked;
};

class ILiveContent : public XBoxLiveEntityIterator
{
public:
	virtual ~ILiveContent() {}

	virtual bool IsPluged() = 0;
	virtual bool PlugContent() = 0;
	virtual void UnplugContent() = 0;

	virtual const char* GetContentName() = 0;
	virtual const char* GetDisplayName() = 0;

	virtual ITexture* GetPicTexture() = 0;
	virtual bool IsPicTextureGetProcessing() = 0;
	virtual void ReleasePicTexture() = 0;

	virtual bool IsContentMatching(const BYTE * pbContentID) = 0;
};

class ILiveContentEnumerator
{
public:
	virtual ~ILiveContentEnumerator() {}

	virtual bool Start() = 0;
	virtual bool IsDone() = 0;
	virtual bool IsValideContent() = 0;
	virtual bool IsEnd() = 0;
	virtual void End() = 0;
	virtual void Next() = 0;

	virtual const char* GetCurContentName() = 0;
	virtual void GetContentData(XContentData& cd) = 0;
};

class ILiveDownloadEnumerator
{
public:
	virtual ~ILiveDownloadEnumerator() {}

	virtual bool Start(DWORD dwTitleID) = 0;
	virtual bool Frame() = 0;
	virtual bool IsCompliteEnumerate() = 0;
	virtual bool CheckNewItem(ILiveContent** pContents, long nContentsQ) = 0;
	virtual bool LaunchDownloading() = 0;
};

#endif
