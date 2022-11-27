#ifdef _XBOX
#ifndef _CONTENTENUMERATOR_H_
#define _CONTENTENUMERATOR_H_

#include "..\icontent.h"

class XBoxLiveContentEnumerator : public ILiveContentEnumerator
{
public:
	XBoxLiveContentEnumerator();
	virtual ~XBoxLiveContentEnumerator();

	virtual bool Start();
	virtual bool IsDone();
	virtual bool IsValideContent();
	virtual bool IsEnd();
	virtual void End();
	virtual void Next();

	virtual const char* GetCurContentName();
	virtual void GetContentData(XContentData& cd);

protected:
    HANDLE m_hEnum;
	XCONTENT_DATA m_content;
	XOVERLAPPED m_overlapped;
	char m_pcContentName[XCONTENT_MAX_FILENAME_LENGTH+1];

	void GetNextContentData();
	void PrepareOverlapped();
	void CloseOverlapped();
};

#endif
#endif
