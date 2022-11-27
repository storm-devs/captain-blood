#ifndef _CONTENTTEST_H_
#define _CONTENTTEST_H_

#include "..\..\..\Common_h\Mission.h"

class IContentManager;
class XContentGUIUpdater;

class ContentTest : public MissionObject
{
public:
	ContentTest();
	~ContentTest();

	bool Create(MOPReader & reader);

	void _cdecl Realize(float fDeltaTime, long level);

	virtual void Command(const char * id, dword numParams, const char ** params);

protected:
	void PlugContent(long nContentIndex, bool bPlugIn);
	void SetContentData(long nContentIndex);
	void ResetContentData(long nContentIndex);

	XContentGUIUpdater* FindUpdater(long nContentIndex);

	IContentManager* m_pContentManager;
	const char* m_pcWidgetPicture;
	const char* m_pcWidgetDescribe;

	array<XContentGUIUpdater*> m_aGUIUpdater;
};

#endif
