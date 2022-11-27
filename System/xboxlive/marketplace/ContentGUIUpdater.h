#ifndef ContentGUIUpdater_h
#define ContentGUIUpdater_h

#include "..\..\..\Common_h\Mission.h"

class ContentManager;
class ILiveContent;

class XContentGUIUpdater
{
public:
	XContentGUIUpdater(ContentManager* pManager, ILiveContent* pContent, long nContentIndex);
	~XContentGUIUpdater();

	void Update();

	void StartUpdater(MissionObject* pPicWidget, MissionObject* pDescrWidget);

	ILiveContent* GetContentPtr() {return m_pContent;}
	long GetContentIndex() {return m_nContentIndex;}

protected:
	ContentManager * m_pManager;
	ILiveContent* m_pContent;
	long m_nContentIndex;

	bool m_bUpdate;
	MissionObject* m_pPicWidget;

	void StopPictureProcessing();
	void SetGUIPicture(ITexture* pTexture, const char* pcDefTextureName);
};

#endif
