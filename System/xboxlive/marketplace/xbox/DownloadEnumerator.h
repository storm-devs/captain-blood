#ifdef _XBOX
#ifndef _DOWNLOADENUMERATOR_H_
#define _DOWNLOADENUMERATOR_H_

#include "..\icontent.h"

class XBoxLiveDownloadEnumerator : public ILiveDownloadEnumerator
{
	struct ContentInfo
	{
		XMARKETPLACE_CONTENTOFFER_INFO offerInfo;
		bool bDownloadedState;
	};

public:
	XBoxLiveDownloadEnumerator();
	virtual ~XBoxLiveDownloadEnumerator();

	virtual bool Start(DWORD dwTitleID);
	virtual bool Frame();
	virtual bool IsCompliteEnumerate();
	virtual bool CheckNewItem(ILiveContent** pContents, long nContentsQ);
	virtual bool LaunchDownloading();

protected:
	DWORD m_dwTitleID;
    HANDLE m_hEnum;
	XOVERLAPPED m_overlapped;
	array<ContentInfo> m_offerFree;
	array<ContentInfo> m_offerPaid;
	XMARKETPLACE_CONTENTOFFER_INFO m_curEnumerateInfo;

	bool IsDone();
	bool IsValideContent();
	bool IsEnd();
	void End();

	void GetNextContentData();
	void PrepareOverlapped();
	void CloseOverlapped();
};

#endif
#endif
