#ifndef _CONTENTMANAGER_H_
#define _CONTENTMANAGER_H_

#include "icontent.h"
#include "ILiveService.h"
class ILiveService;

class ContentManager : public IContentManager
{
private:
	enum PlugedState
	{
		ps_Pluged,
		ps_Unpluged,
		ps_Unassigned
	};

public:
	ContentManager();
	~ContentManager();

	//Инициализация
	virtual bool Init();
	void StartFrame(float dltTime);

	virtual IContentManager::ContentStatus GetStatus();

	virtual void UpdateDownloadList();
	virtual void UpdateContentList();

	virtual long GetContentQuantity() {return m_ContentsList.Size();}
	virtual bool GetContentDescribe(long nContent,IContentManager::ContentDescribe& descr);
	virtual bool PlugContent(long nContent);
	virtual bool UnplugContent(long nContent);

	ILiveContent* GetContentByIndex(long n) {if(n<0 || n>=m_ContentsList) return NULL; return m_ContentsList[n];}

protected:
	void ReleaseContents();

	void Enumerate_ContentFrame();
	void Enumerate_PrepareContents();
	void Enumerate_AddContent(const XContentData& cd);
	void Enumerate_ProcessingContents();

	void UpdatePlugedStateForContent(ILiveContent* pCnt);
	PlugedState CheckPlugedState(ILiveContent* pCnt);
	void SetPlugedStateForConetnt(ILiveContent* pCnt,bool bPlugState);

	array<ILiveContent*> m_ContentsList;
	ILiveContentEnumerator* m_pContentEnumerator;
	ILiveDownloadEnumerator* m_pDownloadEnumerator;

	DWORD m_dwTitleID;
	IContentManager::ContentStatus m_curStatus;

	ILiveService * m_pLiveService;
};

#endif
