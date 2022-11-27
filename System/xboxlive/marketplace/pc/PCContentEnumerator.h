#ifndef _XBOX
#ifndef _PCCONTENTENUMERATOR_H_
#define _PCCONTENTENUMERATOR_H_

#include "..\icontent.h"
#include "..\..\..\..\common_h\ILiveService.h"

class PCLiveContentEnumerator : public ILiveContentEnumerator
{
public:
	PCLiveContentEnumerator(ILiveService* pLiveService);
	virtual ~PCLiveContentEnumerator();

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
	//XCONTENT_DATA m_content;
	//XOVERLAPPED m_overlapped;
	char m_pcContentName[1024];

	void GetNextContentData();

	ILiveService* m_pLiveService;
};

#endif
#endif
